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
 *  Copyright 2024 by
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
 * DESCRIPTION:  Heads-up displays
 *               Text mode version
 *
 *-----------------------------------------------------------------------------
 */

#include <stdint.h>

#include "d_player.h"
#include "i_vtext.h"
#include "r_defs.h"
#include "hu_stuff.h"
#include "w_wad.h"
#include "s_sound.h"
#include "d_englsh.h"
#include "sounds.h"
#include "g_game.h"
#include "r_main.h"

#include "globdata.h"


#define HU_MAXLINELENGTH  34


/* Text Line widget
 *  (parent of Scrolling Text and Input Text widgets) */
typedef struct
{
  char  lineoftext[HU_MAXLINELENGTH+1];
  size_t   len;
} hu_textline_t;


// widgets
static hu_textline_t  w_title;
static hu_textline_t  w_message;

// boolean stating whether to update window
static boolean    message_on;
boolean    _g_message_dontfuckwithme;


// global heads up display controls

//
// Locally used constants, shortcuts.
//

#define HU_TITLEY (VIEWWINDOWHEIGHT - 1)

#define HU_MSGY         0


//
// HU_Init()
//
// Initialize the heads-up display, text that overwrites the primary display
//
// Passed nothing, returns nothing
//
void HU_Init(void)
{
	// Do nothing
}


//
// Builtin map names.
//
static const char *const mapnames[] =
{
    HUSTR_E1M1,
    HUSTR_E1M2,
    HUSTR_E1M3,
    HUSTR_E1M4,
    HUSTR_E1M5,
    HUSTR_E1M6,
    HUSTR_E1M7,
    HUSTR_E1M8,
    HUSTR_E1M9,
};


//
// HU_Start(void)
//
// Create and initialize the heads-up widgets, software machines to
// maintain, update, and display information over the primary display
//
// This routine must be called after any change to the heads up configuration
// in order for the changes to take effect in the actual displays
//
// Passed nothing, returns nothing
//
void HU_Start(void)
{
	message_on = false;
	_g_message_dontfuckwithme = false;

	// create the message widget
	// messages to player in upper-left of screen
	w_message.len           = 0;
	w_message.lineoftext[0] = 0;

	// create the map title widget - map title display in lower left of automap
	w_title.len       = strlen(mapnames[_g_gamemap - 1]);
	strcpy(w_title.lineoftext, mapnames[_g_gamemap - 1]);
}


//
// HU_Drawer()
//
// Draw all the pieces of the heads-up display
//
// Passed nothing, returns nothing
//

static int16_t message_clearer = 0;

void HU_Drawer(void)
{
    // draw the automap widgets / map title if automap is displayed
	static int16_t am_clearer = 0;

	if (automapmode & am_active)
	{
        V_DrawString(0, HU_TITLEY, 12, w_title.lineoftext);
		am_clearer = 3; // 3 screen pages
	}
	else
	{
		if (am_clearer)
		{
			am_clearer--;
			V_ClearString(HU_TITLEY, w_title.len);
		}
	}


	// message
	if (message_clearer)
	{
		message_clearer--;
		V_ClearString(HU_MSGY, HU_MAXLINELENGTH);
	}

    if (message_on)
		V_DrawString(0, HU_MSGY, 12, w_message.lineoftext);
}


//
// HU_Ticker()
//
// Update the hud displays once per frame
//
// Passed nothing, returns nothing
//

#define HU_MSGTIMEOUT   (4*TICRATE)

void HU_Ticker(void)
{
	static int16_t        message_counter = 0;

	player_t* plr = &_g_player;

	// tick down message counter if message is up
	if (message_counter && !--message_counter)
	{
		message_clearer = 3; // 3 screen pages
		message_on = false;
	}

	// if messages on, or "Messages Off" is being displayed
	// this allows the notification of turning messages off to be seen
	if (showMessages || _g_message_dontfuckwithme)
	{
		// display message if necessary
		if (plr->message)
		{
			//post the message to the message widget
			size_t newlen = strlen(plr->message);
			if (newlen < w_message.len)
				message_clearer = 3; // 3 screen pages
			w_message.len = newlen;
			strcpy(w_message.lineoftext, plr->message);

			// clear the message to avoid posting multiple times
			plr->message = NULL;
			// note a message is displayed
			message_on = true;
			// start the message persistence counter
			message_counter = HU_MSGTIMEOUT;

			// clear the flag that "Messages Off" is being posted
			_g_message_dontfuckwithme = false;
		}
	}
}
