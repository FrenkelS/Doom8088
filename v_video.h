/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023, 2024 by
 *  Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Gamma correction LUT.
 *  Color range translation support
 *  Functions to draw patches (by post) directly to screen.
 *  Functions to blit a block to the screen.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomtype.h"
#include "doomdef.h"
// Needed because we are refering to patches.
#include "r_data.h"

//
// VIDEO
//

void V_DrawBackground(int16_t backgroundnum);

void V_DrawRaw(int16_t num, uint16_t offset);
void V_DrawRawFullScreen(int16_t num);

// V_DrawNumPatchScaled - Draws the patch from lump num
void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch);
void V_DrawNumPatchScaled(int16_t x, int16_t y, int16_t lump);
void V_DrawNumPatchNotScaled(int16_t x, int16_t y, int16_t lump);

void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch);


// V_DrawNamePatchScaled - Draws the patch from lump "name"
#define V_DrawNamePatchScaled(x,y,name) V_DrawNumPatchScaled(x,y,W_GetNumForName(name))


void V_ClearViewWindow(void);
void V_InitDrawLine(void);
void V_ShutdownDrawLine(void);
void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);


#endif
