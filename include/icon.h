#ifndef INCLUDED_ICON_H
#define INCLUDED_ICON_H

#include <psptypes.h>

#define SX_DEFAULT 35             // Left offset
#define SY_DEFAULT 35             // Up offset
#define W_DEFAULT 29              // width
#define H_DEFAULT 16              // height
#define FG_COLOR_DEFAULT 0x0000FF // RGB is swapped to BGR
#define BG_COLOR_DEFAULT 0x000000 // RGB is swapped to BGR
#define ICON_SIZE_MIN 0
#define ICON_SIZE_MAX 10

// Battery icon; has an extra \0 (0x00) for being a string
#define MSX_DEFAULT "\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00" \
                    "\x3F\xFF\xFF\x80" \
                    "\x3F\xFF\xFF\x80" \
                    "\x30\x60\xC1\x80" \
                    "\x30\x31\x81\x80" \
                    "\x30\x1B\x01\xE0" \
                    "\x30\x0E\x01\xE0" \
                    "\x30\x0E\x01\xE0" \
                    "\x30\x1B\x01\xE0" \
                    "\x30\x31\x81\x80" \
                    "\x30\x60\xC1\x80" \
                    "\x3F\xFF\xFF\x80" \
                    "\x3F\xFF\xFF\x80" \
                    "\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00"

// Icon struct for `draw_bin`. Make sure that `sizeof(icon.msx) <= icon->w * icon->h`.
typedef struct
{
    u8 msx[sizeof(MSX_DEFAULT) * 4]; // icon
    int sx;                          // Left offset
    int sy;                          // Up offset
    int w;                           // width
    int h;                           // height
    int fg_col;                      // RGB is swapped to BGR
    int bg_col;                      // RGB is swapped to BGR
} icon_t;

typedef enum
{
    P_UPLEFT,
    P_UPRIGHT,
    P_CENTRE,
    P_DOWNLEFT,
    P_DOWNRIGHT
} icon_pos;

extern const u8 i_batt[];

void init_icon(icon_t *icon, double scale);
void fix_position(icon_t *icon, icon_pos pos);
void scaled_icon(icon_t *icon, double scale);

#endif