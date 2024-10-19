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
 *      Does palette indicators (red pain/berserk, bright pickup)
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "compiler.h"
#include "doomdef.h"
#include "i_system.h"
#include "st_stuff.h"

#include "globdata.h"


static int8_t st_palette;


void ST_initPalette(void)
{
    st_palette = -1;
}


// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13


void ST_doPaletteStuff(void)
{
    int8_t  palette;
    int16_t cnt = _g_player.damagecount;

    if (_g_player.powers[pw_strength])
    {
        // slowly fade the berzerk out
        int16_t bzc = 12 - (_g_player.powers[pw_strength] >> 6);
        if (bzc > cnt)
            cnt = bzc;
    }

    if (cnt)
    {
        palette = (cnt + 7) >> 3;
        if (palette >= NUMREDPALS)
            palette = NUMREDPALS - 1;

        /* cph 2006/08/06 - if in the menu, reduce the red tint - navigating to
       * load a game can be tricky if the screen is all red */
        if (_g_menuactive)
            palette >>= 1;

        palette += STARTREDPALS;
    }
    else if (_g_player.bonuscount)
    {
        palette = (_g_player.bonuscount + 7) >> 3;
        if (palette >= NUMBONUSPALS)
            palette = NUMBONUSPALS - 1;
        palette += STARTBONUSPALS;
    }
    else if (_g_player.powers[pw_ironfeet] > 4 * 32 || _g_player.powers[pw_ironfeet] & 8)
        palette = RADIATIONPAL;
    else
        palette = 0;

    if (palette != st_palette) {
        I_SetPalette(st_palette = palette);
    }
}
