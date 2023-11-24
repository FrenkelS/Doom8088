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
 *  Copyright 2023 by
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
 *  Gamma correction LUT stuff.
 *  Color range translation support
 *  Functions to draw patches (by post) directly to screen.
 *  Functions to blit a block to the screen.
 *
 *-----------------------------------------------------------------------------
 */

#include "doomdef.h"
#include "compiler.h"
#include "r_main.h"
#include "w_wad.h"   /* needed for color translation lump lookup */
#include "v_video.h"

#include "globdata.h"


/*
 * V_DrawBackground tiles a 64x64 patch over the entire screen, providing the
 * background for the Help and Setup screens, and plot text between levels.
 * cphipps - used to have M_DrawBackground, but that was used the framebuffer
 * directly, so this is my code from the equivalent function in f_finale.c
 */
void V_DrawBackground(void)
{
    /* erase the entire screen to a tiled background */
    const byte __far* src = W_GetLumpByName("FLOOR4_8");

    for(uint8_t y = 0; y < SCREENHEIGHT; y++)
    {
        for(uint16_t x = 0; x < SCREENWIDTH; x+=64)
        {
            uint8_t __far* d = &_g_screen[y * SCREENWIDTH + x];
            const byte __far* s = &src[((y&63) * 64) + (x&63)];

            uint8_t len = 64;

            if (SCREENWIDTH - x < 64)
                len = SCREENWIDTH - x;

            _fmemcpy(d, s, len);
        }
    }

    Z_ChangeTagToCache(src);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	const uint8_t __far* lump = W_TryGetLumpByNum(num);

	if (lump != NULL)
	{
		int32_t lumpLength = W_LumpLength(num);	
		_fmemcpy(&_g_screen[offset], lump, lumpLength);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, &_g_screen[offset]);
}


static void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
    static const int32_t   DX  = (((int32_t)SCREENWIDTH)<<FRACBITS) / SCREENWIDTH_VGA;
    static const int32_t   DXI = (((int32_t)SCREENWIDTH_VGA)<<FRACBITS) / SCREENWIDTH;
    static const int32_t   DY  = ((((int32_t)SCREENHEIGHT)<<FRACBITS)+(FRACUNIT-1)) / SCREENHEIGHT_VGA;
    static const int16_t   DYI = ((((int32_t)SCREENHEIGHT_VGA)<<FRACBITS) / SCREENHEIGHT) >> 8;

    y -= patch->topoffset;
    x -= patch->leftoffset;

    const int16_t left   = ( x * DX ) >> FRACBITS;
    const int16_t right  = ((x + patch->width)  * DX) >> FRACBITS;
    const int16_t bottom = ((y + patch->height) * DY) >> FRACBITS;

    int32_t   col = 0;

    for (int16_t dc_x = left; dc_x < right; dc_x++, col += DXI)
    {
        if (dc_x < 0)
            continue;
        else if (dc_x >= SCREENWIDTH)
            break;

        const column_t __far* column = (const column_t __far*)((const byte __far*)patch + patch->columnofs[col >> FRACBITS]);

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

            if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
                break;

            int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

            byte __far* dest = _g_screen + (dc_yl * SCREENWIDTH) + dc_x;

            int16_t frac = 0;

            const byte __far* source = (const byte __far*)column + 3;

            int16_t count = dc_yh - dc_yl;
            while (count--)
            {
                *dest = source[frac >> 8];
                dest += SCREENWIDTH;
                frac += DYI;
            }

            column = (const column_t __far*)((const byte __far*)column + column->length + 4);
        }
    }
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
    y -= patch->topoffset;
    x -= patch->leftoffset;

    byte __far* desttop = _g_screen + (y * SCREENWIDTH) + x;

    int16_t width = patch->width;

    for (int16_t col = 0; col < width; col++, desttop++)
    {
        const column_t __far* column = (const column_t __far*)((const byte __far*)patch + patch->columnofs[col]);

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            const byte __far* source = (const byte __far*)column + 3;
            byte __far* dest = desttop + (column->topdelta * SCREENWIDTH);

            uint16_t count = column->length;

            while (count--)
            {
                *dest = *source++;
                dest += SCREENWIDTH;
            }

            column = (const column_t __far*)((const byte __far*)column + column->length + 4);
        }
    }
}


void V_DrawNumPatchScaled(int16_t x, int16_t y, int16_t num)
{
	const patch_t __far* patch = W_GetLumpByNum(num);
	V_DrawPatchScaled(x, y, patch);
	Z_ChangeTagToCache(patch);
}


void V_DrawNumPatchNotScaled(int16_t x, int16_t y, int16_t num)
{
	const patch_t __far* patch = W_GetLumpByNum(num);
	V_DrawPatchNotScaled(x, y, patch);
	Z_ChangeTagToCache(patch);
}


//
// V_FillRect
//
void V_FillRect(byte colour)
{
	_fmemset(_g_screen, colour, SCREENWIDTH * (SCREENHEIGHT - ST_HEIGHT));
}


void V_PlotPixel(int16_t x, int16_t y, uint8_t color)
{
    _g_screen[y * SCREENWIDTH + x] = color;
}
