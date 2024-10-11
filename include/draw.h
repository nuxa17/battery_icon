#ifndef INCLUDED_DRAW_H
#define INCLUDED_DRAW_H

#include <psptypes.h>
#include "icon.h"

int draw_bin(void *vram32, int bufferwidth, int pixelformat, icon_t *icon);

#endif