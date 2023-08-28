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
#include "r_main.h"
#include "m_bbox.h"
#include "w_wad.h"   /* needed for color translation lump lookup */
#include "v_video.h"

#include "globdata.h"


/*
 * This function draws at GBA resolution (ie. not pixel doubled)
 * so the st bar and menus don't look like garbage.
 */

static void V_DrawPatch(int32_t x, int32_t y, const patch_t* patch)
{
    y -= patch->topoffset;
    x -= patch->leftoffset;

    int32_t   col = 0;

    const int32_t   DX  = (((int32_t)240)<<FRACBITS) / 320;
    const int32_t   DXI = (((int32_t)320)<<FRACBITS) / 240;
    const int32_t   DY  = ((((int32_t)SCREENHEIGHT)<<FRACBITS)+(FRACUNIT-1)) / 200;
    const int32_t   DYI = (((int32_t)200)<<FRACBITS) / SCREENHEIGHT;

    byte* byte_topleft = (byte*)_g->screen;
    const int32_t byte_pitch = (SCREENPITCH * 2);

    const int32_t left = ( x * DX ) >> FRACBITS;
    const int32_t right =  ((x + patch->width) *  DX) >> FRACBITS;
    const int32_t bottom = ((y + patch->height) * DY) >> FRACBITS;

    for (int32_t dc_x=left; dc_x<right; dc_x++, col+=DXI)
    {
        int32_t colindex = (col>>FRACBITS);

        if(dc_x < 0)
            continue;

        const column_t* column = (const column_t *)((const byte*)patch + patch->columnofs[colindex]);

        if (dc_x >= 240)
            break;

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            const byte* source = (const byte*)column + 3;
            const int32_t topdelta = column->topdelta;

            int32_t dc_yl = (((y + topdelta) * DY) >> FRACBITS);
            int32_t dc_yh = (((y + topdelta + column->length) * DY) >> FRACBITS);

            if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
                break;

            int32_t count = (dc_yh - dc_yl);

            byte* dest = byte_topleft + (dc_yl*byte_pitch) + dc_x;

            const fixed_t fracstep = DYI;
            fixed_t frac = 0;

            // Inner loop that does the actual texture mapping,
            //  e.g. a DDA-lile scaling.
            // This is as fast as it gets.
            while (count--)
            {
                uint16_t color = source[frac >> FRACBITS];

                //The GBA must write in 16bits.
                if((uint32_t)dest & 1)
                {
                    //Odd addreses, we combine existing pixel with new one.
                    uint16_t* dest16 = (uint16_t*)(dest - 1);


                    uint16_t old = *dest16;

                    *dest16 = (old & 0xff) | (color << 8);
                }
                else
                {
                    uint16_t* dest16 = (uint16_t*)dest;

                    uint16_t old = *dest16;

                    *dest16 = ((color & 0xff) | (old & 0xff00));
                }

                dest += byte_pitch;
                frac += fracstep;
            }

            column = (const column_t *)((const byte *)column + column->length + 4 );
        }
    }
}


void V_DrawPatchNoScale(int32_t x, int32_t y, const patch_t* patch)
{
    y -= patch->topoffset;
    x -= patch->leftoffset;

    byte* desttop = (byte*)_g->screen;
    desttop += (ScreenYToOffset(y) << 1) + x;

    int16_t width = patch->width;

    for (int16_t col = 0; col < width; col++, desttop++)
    {
        const column_t* column = (const column_t*)((const byte*)patch + patch->columnofs[col]);

        uint32_t odd_addr = (uint32_t)desttop & 1;

        byte* desttop_even = (byte*)((uint32_t)desttop & ~1);

        // step through the posts in a column
        while (column->topdelta != 0xff)
        {
            const byte* source = (const byte*)column + 3;
            byte* dest = desttop_even + (ScreenYToOffset(column->topdelta) << 1);

            uint32_t count = column->length;

            while (count--)
            {
                uint32_t color = *source++;
                volatile uint16_t* dest16 = (volatile uint16_t*)dest;

                uint32_t old = *dest16;

                //The GBA must write in 16bits.
                if (odd_addr)
                    *dest16 = (old & 0xff) | (color << 8);
                else
                    *dest16 = ((color & 0xff) | (old & 0xff00));

                dest += 240;
            }

            column = (const column_t*)((const byte*)column + column->length + 4);
        }
    }
}


void V_DrawNumPatch(int32_t x, int32_t y, int16_t num)
{
	const patch_t* patch = W_GetLumpByNum(num);
	V_DrawPatch(x, y, patch);
	Z_Free(patch);
}


void V_DrawNamePatch(int32_t x, int32_t y, const char *name)
{
	const patch_t* patch = W_GetLumpByName(name);
	V_DrawPatch(x, y, patch);
	Z_Free(patch);
}


void V_DrawNumPatchNoScale(int32_t x, int32_t y, int16_t num)
{
	const patch_t* patch = W_GetLumpByNum(num);
	V_DrawPatchNoScale(x, y, patch);
	Z_Free(patch);
}
