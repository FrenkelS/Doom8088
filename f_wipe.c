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
 *      Mission begin melt/wipe screen special effect.
 *
 *-----------------------------------------------------------------------------
 */

//Most of this code is backported from https://github.com/next-hack/nRF52840Doom


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include "z_zone.h"
#include "doomdef.h"
#include "v_video.h"
#include "m_random.h"
#include "f_wipe.h"
#include "globdata.h"
#include "i_system.h"


static uint16_t __far* frontbuffer;
static  int16_t __far* wipe_y_lookup;


void wipe_StartScreen(void)
{
	frontbuffer = Z_TryMallocStatic(SCREENWIDTH * SCREENHEIGHT * sizeof(uint16_t));
	if (frontbuffer)
		I_CopyBackBufferToBuffer(frontbuffer);
}


static boolean wipe_ScreenWipe(int32_t ticks)
{
    boolean done = true;

    uint16_t* backbuffer = I_GetBackBuffer();

    while (ticks--)
    {
        I_DrawBuffer(frontbuffer);
        for (int16_t i = 0; i < SCREENWIDTH; i++)
        {
            if (wipe_y_lookup[i] < 0)
            {
                wipe_y_lookup[i]++;
                done = false;
                continue;
            }

            // scroll down columns, which are still visible
            if (wipe_y_lookup[i] < SCREENHEIGHT)
            {
                /* cph 2001/07/29 -
                 *  The original melt rate was 8 pixels/sec, i.e. 25 frames to melt
                 *  the whole screen, so make the melt rate depend on SCREENHEIGHT
                 *  so it takes no longer in high res
                 */
                int16_t dy = (wipe_y_lookup[i] < 16) ? wipe_y_lookup[i] + 1 : SCREENHEIGHT / 25;
                // At most dy shall be so that the column is shifted by SCREENHEIGHT (i.e. just
                // invisible)
                if (wipe_y_lookup[i] + dy >= SCREENHEIGHT)
                    dy = SCREENHEIGHT - wipe_y_lookup[i];

                uint16_t* s = &frontbuffer[i] + ((SCREENHEIGHT - dy - 1) * SCREENPITCH);

                uint16_t* d = &frontbuffer[i] + ((SCREENHEIGHT - 1) * SCREENPITCH);

                // scroll down the column. Of course we need to copy from the bottom... up to
                // SCREENHEIGHT - yLookup - dy

                for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
                {
                    *d = *s;
                    d += -SCREENPITCH;
                    s += -SCREENPITCH;
                }

                // copy new screen. We need to copy only between y_lookup and + dy y_lookup
                s = &backbuffer[i]  + wipe_y_lookup[i] * SCREENPITCH;
                d = &frontbuffer[i] + wipe_y_lookup[i] * SCREENPITCH;

                for (int16_t j = 0 ; j < dy; j++)
                {
                    *d = *s;
                    d += SCREENPITCH;
                    s += SCREENPITCH;
                }

                wipe_y_lookup[i] += dy;
                done = false;
            }
        }
    }

    return done;
}

static void wipe_initMelt()
{
    wipe_y_lookup = Z_MallocStatic(SCREENWIDTH * sizeof(int16_t));

    // setup initial column positions (y<0 => not ready to scroll yet)
    wipe_y_lookup[0] = -(M_Random() % 16);
    for (int8_t i = 1; i < SCREENWIDTH; i++)
    {
        int8_t r = (M_Random() % 3) - 1;

        wipe_y_lookup[i] = wipe_y_lookup[i - 1] + r;

        if (wipe_y_lookup[i] > 0)
            wipe_y_lookup[i] = 0;
        else if (wipe_y_lookup[i] == -16)
            wipe_y_lookup[i] = -15;
    }
}


//
// D_Wipe
//
// CPhipps - moved the screen wipe code from D_Display to here
// The screens to wipe between are already stored, this just does the timing
// and screen updating

void D_Wipe(void)
{
    if (!frontbuffer)
        return;

    wipe_initMelt();

    boolean done;
    int32_t wipestart = I_GetTime () - 1;

    do
    {
        int32_t nowtime, tics;
        do
        {
            nowtime = I_GetTime();
            tics = nowtime - wipestart;
        } while (!tics);

        wipestart = nowtime;
        done = wipe_ScreenWipe(tics);

        M_Drawer();                   // menu is drawn even on top of wipes

    } while (!done);

    Z_Free(frontbuffer);
    Z_Free(wipe_y_lookup);
}
