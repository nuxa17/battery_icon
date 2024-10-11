#ifndef INCLUDED_ICON_H
#define INCLUDED_ICON_H

#include <psptypes.h>

// Icon struct for `draw_bin`. Make sure that `sizeof(*icon->msx) <= icon->w * icon->h`.
typedef struct
{
    int sx;        // Left offset
    int sy;        // Up offset
    const u8 *msx; // icon
    int w;         // width
    int h;         // height
    int fg_col;    // RGB is swapped to BGR
    int bg_col;    // RGB is swapped to BGR
} icon_t;

#endif