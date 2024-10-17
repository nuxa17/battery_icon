#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdisplay_kernel.h>
#include <psppower.h>
#include <pspsyscon.h>
#include <string.h>

#include "icon.h"
#include "draw.h"
#include "systemctrl.h"
#include "libinip.h"

#define ONE_SECOND 1000000
#define BLINK_TIME ONE_SECOND
#define INI_FILE "battery_icon.ini"

#ifdef DEBUG
const char *thname = "battery_icon_debug";
PSP_MODULE_INFO("battery_icon_debug", 0x1000, 1, 1);
#else
const char *thname = "battery_icon";
PSP_MODULE_INFO("battery_icon", 0x1000, 1, 1);
#endif

// PSP_MAIN_THREAD_ATTR(0);

void *hooked_setframebuf_func;

int running, wait;
volatile int display;

icon_t icon;

unsigned int handle_alarm(void *common)
{
    display = 0;
    return 0;
}

#ifdef DEBUG
void draw_debug()
{
    SceKernelAlarmHandler alarm = handle_alarm;
    unsigned int *vram32;
    int a, b, c;
    sceKernelSetAlarm(BLINK_TIME, alarm, NULL);
    display = 1;
    while (display)
    {
        c = PSP_DISPLAY_SETBUF_IMMEDIATE; // doesn't matter
        sceDisplayGetFrameBuf((void *)&vram32, &a, &b, c);
        draw_bin(vram32, a, b, &icon);
        sceDisplayWaitVblankStart();
    }
}
#endif

/*
 * CONFIG LOADING
 */

typedef struct myconfig_
{
    int size;
    int position;
    int sx;
    int sy;
} myconfig;

void create_config()
{
    int fd = sceIoOpen(INI_FILE, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
        return;

    char msg[] = "# Size can be between 0 and 10\n"
                 "# Position can be 0(up - left), 1(up - rigth), 2(centre), 3(down - left)4(down - right) or -1(custom).\n"
                 "# If position is 5, the values of 'sx' and 'sy' are applied.\n"
                 "# (0, 0) is the top left corner of the display.\n\n";

    sceIoWrite(fd, msg, sizeof(msg) - 1); // remove ending '\0'
    sceIoClose(fd);
}

void load_config(const char *path)
{
    ILP_Key key[4];

    int size = ICON_SIZE_MIN;
    int pos = P_UPLEFT;
    int sx = SX_DEFAULT;
    int sy = SY_DEFAULT;

    ILPInitKey(key);
    ILPRegisterDec(key, "size", &size, size);
    ILPRegisterDec(key, "position", &pos, pos);
    ILPRegisterDec(key, "sx", &sx, sx);
    ILPRegisterDec(key, "sy", &sy, sy);

    char ini_path[strlen(path) + 1];
    strcpy(ini_path, path);
    char *temp = strrchr(ini_path, '/');
    *(temp + 1) = '\0';
    sceIoChdir(ini_path);

    int ret = ILPReadFromFile(key, INI_FILE);

    if (ret == ILP_ERROR_OPEN)
    {
        create_config();
        ILPWriteToFile(key, INI_FILE, "\n");
    }

    if (size < ICON_SIZE_MIN || size > ICON_SIZE_MAX)
    {
        size = ICON_SIZE_MIN;
    }

    init_icon(&icon, 1 + size / 10.0);

    if (pos == -1)
    {
        icon.sx = sx;
        icon.sy = sy;
    }
    else
    {
        fix_position(&icon, pos);
    }
}

/*
 * FUNCTION PATCHING
 */

int setFrameBuf_patch(void *topaddr, int bufferwidth, int pixelformat, int sync)
{
    if (display && !wait && bufferwidth)
    {
        wait = 1;
        draw_bin(topaddr, bufferwidth, pixelformat, &icon);
        wait = 0;
    }
    return sceDisplaySetFrameBuf(topaddr, bufferwidth, pixelformat, sync);
}

int main_thread(SceSize args, void *argp)
{
    /* Wait for the kernel to boot */
    while (sceKernelFindModuleByName("sceKernelLibrary") == NULL)
    {
        sceKernelDelayThread(10000); /* wait 10 milliseconds */
    }

    load_config(argp);

    SceKernelAlarmHandler alarm = handle_alarm;
    display = 0;
    wait = 0;

    // grab sceDisplaySetFrameBuffer from the NID
    // (https://spenon-dev.github.io/PSPLibDoc/modules/display_09g.prx_sceDisplay.html)
    // (https://psdevwiki.com/pspprxlibraries/)
    hooked_setframebuf_func = (void *)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);

    // patch it
    sctrlHENPatchSyscall(hooked_setframebuf_func, setFrameBuf_patch);

    while (running)
    {
        // suspend thread one blink cycle
        sceKernelDelayThread(BLINK_TIME * 2);

#ifndef DEBUG
        if (scePowerIsLowBattery() == 1 && scePowerIsBatteryCharging() == 0) // Low battery
#endif
        {
            // show icon until alarm rings
            sceKernelSetAlarm(BLINK_TIME, alarm, NULL);
            display = 1;
        }
    }

    sceKernelExitDeleteThread(0);
    return 0;
}

/*
 *  MODULE STUFF
 */

int module_start(SceSize args, void *argp)
{
    SceUID thid;
    running = 0;
    thid = sceKernelCreateThread(thname, main_thread, 0x18, 0x1000, 0, NULL);
    if (thid >= 0)
    {
        running = 1;
        sceKernelStartThread(thid, args, argp);
    }
    return 0;
}

int module_stop(SceSize args, void *argp)
{
    if (running)
    {
        SceUID thid;
        pspSdkReferThreadStatusByName(thname, &thid, NULL);
        running = 0;
        display = 0;
        SceUInt time = 5 * ONE_SECOND;
        int ret = sceKernelWaitThreadEnd(thid, &time);
        if (ret < 0)
            sceKernelTerminateDeleteThread(thid);
    }

    return 0;
}