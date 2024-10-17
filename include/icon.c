#include <pspkernel.h>
#include <psptypes.h>
#include <math.h>
#include <string.h>
#include "icon.h"

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 272

// Scale values can be from 1 to 2
void init_icon(icon_t *icon, double scale)
{
    u8 *aux = (u8 *)MSX_DEFAULT;
    memmove(icon->msx, aux, sizeof(MSX_DEFAULT));

    if (scale != 1)
    {
        scaled_icon(icon, scale);
    }
    else
    {
        icon->w = W_DEFAULT;
        icon->h = H_DEFAULT;
    }
    icon->sx = SX_DEFAULT;
    icon->sy = SY_DEFAULT;
    icon->fg_col = FG_COLOR_DEFAULT;
    icon->bg_col = BG_COLOR_DEFAULT;
}

void fix_position(icon_t *icon, icon_pos pos)
{
    switch (pos)
    {
    case P_UPRIGHT:
        icon->sx = DISPLAY_WIDTH - icon->w - SX_DEFAULT;
        icon->sy = SY_DEFAULT;
        break;
    case P_CENTRE:
        icon->sx = DISPLAY_WIDTH / 2 - icon->w / 2;
        icon->sy = DISPLAY_HEIGHT / 2 - icon->h / 2;
        break;
    case P_DOWNLEFT:
        icon->sx = SX_DEFAULT;
        icon->sy = DISPLAY_HEIGHT - icon->h - SY_DEFAULT;
        break;
    case P_DOWNRIGHT:
        icon->sx = DISPLAY_WIDTH - icon->w - SX_DEFAULT;
        icon->sy = DISPLAY_HEIGHT - icon->h - SY_DEFAULT;
        break;
    case P_UPLEFT:
    default:
        icon->sx = SX_DEFAULT;
        icon->sy = SY_DEFAULT;
        break;
    }
}

// Scale value must be in the range 1-2
void scaled_icon(icon_t *i, double scale)
{
    u8 a[] = MSX_DEFAULT;
    int a_w = W_DEFAULT;
    int a_h = H_DEFAULT;

    int i_w = round(a_w * scale);
    int i_h = round(a_h * scale);

    // x and y are the current bit, word is the current word
    int a_words_row = ceil(a_w / 8.0); // Number of u8s in a row
    int a_x = -1, a_y = -1, a_word = 0;
    int i_words_row = ceil(i_w / 8.0); // Number of u8s in a row
    int i_x = 0, i_y = 0, i_word = 0;
    u8 bit = 0;

    while (i_y < i_h)
    {
        if (floor(i_y / scale) == a_y) // Same line
        {
            while (i_word < i_words_row * (i_y + 1)) // Copy it
            {
                i->msx[i_word] = i->msx[i_word - i_words_row];
                i_word++;
            }
            i_y++;
            continue;
        }
        a_x = -1;
        a_y = floor(i_y / scale);
        i_x = 0;

        while (i_x < i_w)
        {
            i->msx[i_word] = 0; // Set all bits to 0
            for (int j = 7; j >= 0 && i_x < i_w; j--)
            {
                if (floor(i_x / scale) != a_x) // Referencing different bit
                {
                    a_x = floor(i_x / scale);
                    a_word = a_y * a_words_row + a_x / 8;
                    bit = (a[a_word] >> (7 - a_x % 8)) & 1;
                }
                i->msx[i_word] |= bit << j; // Set corresponding bit
                i_x++;
            }
            i_word++;
        }
        i_y++;
    }

    i->w = i_w;
    i->h = i_h;
}