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
 *      Text mode version
 *
 *-----------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>

#include "d_englsh.h"
#include "f_lib.h"
#include "hu_stuff.h"
#include "i_vtext.h"
#include "v_video.h"

void F_TextWrite(int32_t count)
{
	// draw some of the text onto the screen
	int16_t         cx = (VIEWWINDOWWIDTH  - 32) / 2;
	int16_t         cy = (VIEWWINDOWHEIGHT - 12) / 2;
	const char* ch = E1TEXT;

	for ( ; count ; count-- )
	{
		char c = *ch++;

		if (!c)
			break;
		if (c == '\n')
		{
			cx = (VIEWWINDOWWIDTH - 32) / 2;
			cy++;
			continue;
		}

		V_DrawCharacter(cx, cy, 12, c);
		cx++;
	}
}
