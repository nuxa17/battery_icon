// Code from the vshex from devhook, adapted

#include <psptypes.h>
#include <pspdisplay.h>

#include "icon.h"

// Each row begins with a new word, so icons can be cropped horizontally and vertically.
// Make sure that `sizeof(*icon->msx) <= icon->w * icon->h`.
int draw_bin(void *topaddr, int bufferwidth, int pixelformat, icon_t *icon)
{
    int x, y, p;
    int npix, offset;
    u8 word;
    const u8 *msx;
    int pwidth, pheight, unk; // bufferwidth, pixelformat,
    unsigned int *vram32 = (unsigned int *)topaddr;
    u16 *vram16;

    unsigned char in16bit = 0;

    sceDisplayGetMode(&unk, &pwidth, &pheight);

    vram16 = (u16 *)vram32;

    if (pixelformat != PSP_DISPLAY_PIXEL_FORMAT_8888)
        in16bit = 1;

    msx = icon->msx;
    offset = icon->sy * bufferwidth + icon->sx; // bufferwidth can be bigger than pwidth!
    npix = x = y = 0;

    while (npix < icon->w * icon->h)
    {
        word = *msx;
        for (p = 0; p < 8 && x < icon->w; p++)
        {
            if (in16bit)
                vram16[offset] = (word & 0x80) ? icon->fg_col : icon->bg_col;
            else
                vram32[offset] = (word & 0x80) ? icon->fg_col : icon->bg_col;
            word <<= 1;
            npix++;
            x++;
            offset++;
            if (icon->sx + x == bufferwidth)
            {
                msx += (icon->w - x) / 8; // skip remaining words from row
                x = icon->w;              // skip remaining bits
                npix = (y * icon->w) + x; // update "drawn" pixels
            }
        }

        if (x == icon->w) // next word on the next row
        {
            x = 0;
            y++;
            // move offset to next line
            offset = (y + icon->sy) * bufferwidth + icon->sx;
        }
        msx++;
    }

    return x;
}
