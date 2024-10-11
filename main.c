#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdisplay_kernel.h>
#include <psppower.h>
#include <stdio.h>
#include <pspsyscon.h>

#include "icon.h"
#include "draw.h"
#include "systemctrl.h"

#define ONE_SECOND 1000000
#define BLINK_TIME ONE_SECOND

PSP_MODULE_INFO("battery_icon", 0x1000, 1, 0);
// PSP_MAIN_THREAD_ATTR(0);

void *hooked_setframebuf_func;

int running, wait;
volatile int display;

// Battery icon, has an extra \0 (0x00) for being a string
const u8 i_batt[] =
    "\x00\x00\x00\x00"
    "\x00\x00\x00\x00"
    "\x3F\xFF\xFF\x80"
    "\x3F\xFF\xFF\x80"
    "\x30\x60\xC1\x80"
    "\x30\x31\x81\x80"
    "\x30\x1B\x01\xE0"
    "\x30\x0E\x01\xE0"
    "\x30\x0E\x01\xE0"
    "\x30\x1B\x01\xE0"
    "\x30\x31\x81\x80"
    "\x30\x60\xC1\x80"
    "\x3F\xFF\xFF\x80"
    "\x3F\xFF\xFF\x80"
    "\x00\x00\x00\x00"
    "\x00\x00\x00\x00";

icon_t icon = {35, 35, i_batt, 29, 16, 0x0000FF, 0x000000};

unsigned int handle_alarm(void *common)
{
    display = 0;
    return 0;
}

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

        if (scePowerIsLowBattery() == 1 && scePowerIsBatteryCharging() == 0) // Low battery
        {
            // show icon until alarm rings
            sceKernelSetAlarm(BLINK_TIME, alarm, NULL);
            display = 1;
        }
    }

    sceKernelExitDeleteThread(0);
    return 0;
}

int module_start(SceSize args, void *argp)
{
    SceUID thid;
    running = 0;
    thid = sceKernelCreateThread("batt_icon", main_thread, 0x18, 0x1000, 0, NULL);
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
        pspSdkReferThreadStatusByName("batt_icon", &thid, NULL);
        running = 0;
        SceUInt time = 5 * ONE_SECOND;
        int ret = sceKernelWaitThreadEnd(thid, &time);
        if (ret < 0)
            sceKernelTerminateDeleteThread(thid);
    }

    return 0;
}