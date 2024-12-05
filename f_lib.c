/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2024 Frenkel Smeijers
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
 *      Game completion, final screen animation library.
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "d_englsh.h"
#include "f_lib.h"
#include "hu_stuff.h"
#include "v_video.h"
#include "w_wad.h"

//
// F_TextWrite
//
// This program displays the background and text at end-mission
// text time. It draws both repeatedly so that other displays,
// like the main menu, can be drawn over it dynamically and
// erased dynamically. The TEXTSPEED constant is changed into
// the Get_TextSpeed function so that the speed of writing the
// text can be increased, and there's still time to read what's
// written.
void F_TextWrite(int32_t finalecount, int32_t textSpeed)
{
	int16_t font_lump_offset = W_GetNumForName(HU_FONTSTART_LUMP) - HU_FONTSTART;

	// draw some of the text onto the screen
	int16_t         cx = 10;
	int16_t         cy = 10;
	const char* ch = E1TEXT;
	int32_t         count = (finalecount - 10)*100/textSpeed;

	if (count < 0)
		count = 0;

	for ( ; count ; count-- )
	{
		char c = *ch++;

		if (!c)
			break;
		if (c == '\n')
		{
			cx = 10;
			cy += 11;
			continue;
		}

		c = toupper(c);
		if (HU_FONTSTART <= c && c <= HU_FONTEND) {
			const patch_t __far* patch = W_GetLumpByNum(c + font_lump_offset);
			V_DrawPatchNotScaled(cx, cy, patch);
			cx += patch->width;
			Z_ChangeTagToCache(patch);
		} else {
			cx += HU_FONT_SPACE_WIDTH;
		}
	}
}
