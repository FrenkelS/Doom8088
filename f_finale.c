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
#include "f_finale.h" // CPhipps - hmm...
#include "d_englsh.h"

#include "globdata.h"


// Stage of animation:
//  false = text, true = art screen
static boolean finalestage; // cph -
static int32_t finalecount; // made static

static boolean midstage;                 // whether we're in "mid-stage"


// defines for the end mission display text                     // phares

#define TEXTSPEED    300   // original value                    // phares
#define TEXTWAIT     250   // original value                    // phares
#define NEWTEXTSPEED 1     // new value                         // phares
#define NEWTEXTWAIT  1000  // new value                         // phares

// CPhipps - removed the old finale screen text message strings;
// they were commented out for ages already
// Ty 03/22/98 - ... the new s_WHATEVER extern variables are used
// in the code below instead.

void WI_checkForAccelerate(void);    // killough 3/28/98: used to

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
// killough 5/10/98: add back v1.9 demo compatibility
//

    void F_Ticker(void)
    {

    WI_checkForAccelerate();  // killough 3/28/98: check for acceleration

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
// F_TextWrite
//
// This program displays the background and text at end-mission     // phares
// text time. It draws both repeatedly so that other displays,      //   |
// like the main menu, can be drawn over it dynamically and         //   V
// erased dynamically. The TEXTSPEED constant is changed into
// the Get_TextSpeed function so that the speed of writing the      //   ^
// text can be increased, and there's still time to read what's     //   |
// written.                                                         // phares
// CPhipps - reformatted

#include "hu_stuff.h"

static void F_TextWrite (void)
{
	V_DrawBackground();

	int16_t font_lump_offset = W_GetNumForName(HU_FONTSTART_LUMP) - HU_FONTSTART;

	// draw some of the text onto the screen
	int16_t         cx = 10;
	int16_t         cy = 10;
	const char* ch = E1TEXT; // CPhipps - const
	int32_t         count = (finalecount - 10)*100/Get_TextSpeed(); // phares

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


//
// F_Drawer
//
void F_Drawer (void)
{
	if (!finalestage)
		F_TextWrite ();
	else
	{
		int16_t num = W_GetNumForName("HELP2");
		V_DrawRawFullScreen(num);
	}
}
