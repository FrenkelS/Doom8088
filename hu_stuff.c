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
 * DESCRIPTION:  Heads-up displays
 *
 *-----------------------------------------------------------------------------
 */

#include <stdint.h>
#include "d_player.h"
#include "r_defs.h"
#include "hu_stuff.h"
#include "st_stuff.h" /* jff 2/16/98 need loc of status bar */
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
  // left-justified position of scrolling text window
  int16_t   x;
  int16_t   y;

  char  l[HU_MAXLINELENGTH+1]; // line of text
  int16_t   len;                            // current line length
} hu_textline_t;


// Scrolling Text window widget
//  (child of Text Line widget)
typedef struct
{
  hu_textline_t l; // text line to draw

  // pointer to boolean stating whether to update window
  boolean*    on;
} hu_stext_t;


// widgets
static hu_textline_t  w_title;
static hu_stext_t     w_message;

static boolean    message_on;
boolean    _g_message_dontfuckwithme;


// global heads up display controls

//
// Locally used constants, shortcuts.
//
// Ty 03/28/98 -
// These shortcuts modifed to reflect char ** of mapnames[]
#define HU_TITLE  (mapnames[_g_gamemap-1])
#define HU_TITLEX 0
//jff 2/16/98 change 167 to ST_Y-1
// CPhipps - changed to ST_TY
// proff - changed to 200-ST_HEIGHT for stretching
#define HU_TITLEY ((SCREENHEIGHT - ST_HEIGHT) - 1 - HU_FONT_HEIGHT)


#define HU_MSGX         0
#define HU_MSGY         0


//
// Builtin map names.
//
// Ty 03/27/98 - externalized map name arrays - now in d_deh.c
// and converted to arrays of pointers to char *
// DOOM map names.
// CPhipps - const**const
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


static int16_t font_lump_offset;


//
// HU_Init()
//
// Initialize the heads-up display, text that overwrites the primary display
//
// Passed nothing, returns nothing
//
void HU_Init(void)
{
	font_lump_offset = W_GetNumForName(HU_FONTSTART_LUMP) - HU_FONTSTART;
}


//
// HUlib_clearTextLine()
//
// Blank the internal text line in a hu_textline_t widget
//
// Passed a hu_textline_t, returns nothing
//
static void HUlib_clearTextLine(hu_textline_t* t)
{
    t->len  = 0;
    t->l[0] = 0;
}


//
// HUlib_initTextLine()
//
// Initialize a hu_textline_t widget. Set the position.
//
// Passed a hu_textline_t, and the values used to initialize
// Returns nothing
//
static void HUlib_initTextLine(hu_textline_t* t, int16_t x, int16_t y)
{
    t->x = x;
    t->y = y;
    HUlib_clearTextLine(t);
}


//
// HUlib_initSText()
//
// Initialize a hu_stext_t widget. Set whether enabled.
//
// Passed a hu_stext_t, and the values used to initialize
// Returns nothing
//
static void HUlib_initSText(hu_stext_t* s, boolean* on)
{
	s->on = on;
	HUlib_initTextLine(&s->l, HU_MSGX, HU_MSGY);
}


//
// HUlib_addCharToTextLine()
//
// Adds a character at the end of the text line in a hu_textline_t widget
//
// Passed the hu_textline_t and the char to add
//
static void HUlib_addCharToTextLine(hu_textline_t* t,char ch)
{
	if (t->len != HU_MAXLINELENGTH)
	{
		t->l[t->len++] = ch;
		t->l[t->len]   = 0;
	}
}


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
    const char* s;

    message_on = false;
    _g_message_dontfuckwithme = false;

    // create the message widget
    // messages to player in upper-left of screen
    HUlib_initSText(&w_message, &message_on);

    //jff 2/16/98 added some HUD widgets
    // create the map title widget - map title display in lower left of automap
    HUlib_initTextLine(&w_title, HU_TITLEX, HU_TITLEY);

    // initialize the automap's level title widget
    if (_g_gamestate == GS_LEVEL)
    {
        s = HU_TITLE;
        while (*s)
            HUlib_addCharToTextLine(&w_title, *(s++));
    }
}


//
// HUlib_drawTextLine()
//
// Draws a hu_textline_t widget
//
// Passed the hu_textline_t and flag whether to draw a cursor
// Returns nothing
//
static void HUlib_drawTextLine(hu_textline_t* l)
{
	const int16_t y = l->y;

	// draw the new stuff
	int16_t x = l->x;
	for (int16_t i = 0; i < l->len; i++)
	{
		char c = toupper(l->l[i]); //jff insure were not getting a cheap toupper conv.

		if (HU_FONTSTART <= c && c <= HU_FONTEND)
		{
			const patch_t __far* patch = W_GetLumpByNum(c + font_lump_offset);
			int16_t w = patch->width;
			if (x + w > SCREENWIDTH)
			{
				Z_ChangeTagToCache(patch);
				break;
			}
			V_DrawPatchNotScaled(x, y, patch);
			Z_ChangeTagToCache(patch);
			x += w;
		}
		else
		{
			x += HU_FONT_SPACE_WIDTH;
			if (x >= SCREENWIDTH)
				break;
		}
	}
}


//
// HUlib_drawSText()
//
// Displays a hu_stext_t widget
//
// Passed a hu_stext_t
// Returns nothing
//
static void HUlib_drawSText(hu_stext_t* s)
{
	if (!*s->on)
		return; // if not on, don't draw

	// draw everything
	HUlib_drawTextLine(&s->l); // no cursor, please
}


//
// HU_Drawer()
//
// Draw all the pieces of the heads-up display
//
// Passed nothing, returns nothing
//
void HU_Drawer(void)
{
    // draw the automap widgets if automap is displayed
    if (automapmode & am_active)
    {
        // map title
        HUlib_drawTextLine(&w_title);
    }

    HUlib_drawSText(&w_message);
}


//
// HUlib_addMessageToSText()
//
// Adds a message line to a hu_stext_t widget
//
// Passed a hu_stext_t and a message string
// Returns nothing
//
static void HUlib_addMessageToSText(hu_stext_t* s, const char* msg)
{
	HUlib_clearTextLine(&s->l);

	while (*msg)
		HUlib_addCharToTextLine(&s->l, *(msg++));
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
            HUlib_addMessageToSText(&w_message, plr->message);

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
