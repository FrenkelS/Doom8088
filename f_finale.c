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
 *      Game completion, final screen animation.
 *
 *-----------------------------------------------------------------------------
 */

#include <stdint.h>

#include "d_player.h"
#include "d_event.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"
#include "sounds.h"
#include "f_finale.h"
#include "f_lib.h"
#include "d_englsh.h"

#include "globdata.h"


// Stage of animation:
//  false = text, true = art screen
static boolean finalestage;
static int32_t finalecount;

static boolean midstage;                 // whether we're in "mid-stage"


static int16_t help2num;
static int16_t backgroundnum;


// defines for the end mission display text                     // phares

#define TEXTSPEED    300   // original value                    // phares
#define TEXTWAIT     250   // original value                    // phares
#define NEWTEXTSPEED 1     // new value                         // phares
#define NEWTEXTWAIT  1000  // new value                         // phares

// CPhipps - removed the old finale screen text message strings;
// they were commented out for ages already
// Ty 03/22/98 - ... the new s_WHATEVER extern variables are used
// in the code below instead.

void WI_checkForAccelerate(void);

//
// F_StartFinale
//
void F_StartFinale (void)
{
    _g_gameaction = ga_nothing;
    _g_gamestate = GS_FINALE;
    automapmode &= ~am_active;

    // killough 3/28/98: clear accelerative text flags
    _g_acceleratestage = midstage = false;

    // Okay - IWAD dependend stuff.
    // This has been changed severly, and
    //  some stuff might have changed in the process.

    S_ChangeMusic(mus_victor, true);

    finalestage = false;
    finalecount = 0;
}


// Get_TextSpeed() returns the value of the text display speed  // phares
// Rewritten to allow user-directed acceleration -- killough 3/28/98

static int32_t Get_TextSpeed(void)
{
    return midstage ? NEWTEXTSPEED : (midstage=_g_acceleratestage) ?
                              _g_acceleratestage=false, NEWTEXTSPEED : TEXTSPEED;
}


//
// F_Ticker
//
// killough 3/28/98: almost totally rewritten, to use
// player-directed acceleration instead of constant delays.
// Now the player can accelerate the text display by using
// the fire/use keys while it is being printed. The delay
// automatically responds to the user, and gives enough
// time to read.
//
//

void F_Ticker(void)
{
    WI_checkForAccelerate();

    // advance animation
    finalecount++;

    if (!finalestage)
    {
        int32_t speed = Get_TextSpeed();
        /* killough 2/28/98: changed to allow acceleration */
        if (finalecount > strlen(E1TEXT)*speed/100 +
                (midstage ? NEWTEXTWAIT : TEXTWAIT) ||
                (midstage && _g_acceleratestage))
        {
       // Doom 1 end
                               // with enough time, it's automatic
            finalecount = 0;
            finalestage = true;
            wipegamestate = -1;         // force a wipe
        }
    }
}


//
// F_Drawer
//
// The TEXTSPEED constant is changed into the Get_TextSpeed
// function so that the speed of writing the text can be
// increased, and there's still time to read what's written.
//
void F_Drawer (void)
{
	if (!finalestage)
	{
		V_DrawBackground(backgroundnum);

		int32_t count = (finalecount - 10) * 100 / Get_TextSpeed();
		if (count < 0)
			count = 0;

		F_TextWrite(count);
	}
	else
		V_DrawRawFullScreen(help2num);
}


void F_Init(void)
{
	help2num      = W_GetNumForName("HELP2");
	backgroundnum = W_GetNumForName("FLOOR4_8");
}
