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
 * DESCRIPTION: Main game control interface.
 *-----------------------------------------------------------------------------*/

#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "d_event.h"
#include "d_ticcmd.h"

//
// GAME
//

boolean G_Responder(event_t *ev);
void G_CheckDemoStatus(void);
void G_DeferedInitNew(skill_t skill);
void G_DeferedPlayDemo(const char *demo); // CPhipps - const
void G_LoadGame(int16_t slot); // killough 5/15/98
void G_SaveGame(int16_t slot); // Called by M_Responder.
void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_WorldDone(void);
void G_EndGame(void); /* cph - make m_menu.c call a G_* function for this */
void G_Ticker(void);
void G_ReloadDefaults(void);     // killough 3/1/98: loads game defaults
void G_Compatibility(void);
void G_PlayerReborn(void);
void G_DoVictory(void);
void G_BuildTiccmd (void);
void G_MakeSpecialEvent(buttoncode_t bc, ...); /* cph - new event stuff */

void G_UpdateSaveGameStrings();

void G_LoadSettings();
void G_SaveSettings();


// killough 5/2/98: moved from m_misc.c:

extern const int32_t  key_menu_right;                                  // phares 3/7/98
extern const int32_t  key_menu_left;                                   //     |
extern const int32_t  key_menu_up;                                     //     V
extern const int32_t  key_menu_down;
extern const int32_t  key_menu_escape;                                 //     |
extern const int32_t  key_menu_enter;                                  // phares 3/7/98
extern const int32_t  key_fire;
extern const int32_t  key_use;
extern const int32_t  key_escape;                                             // phares
extern const int32_t  key_savegame;                                           //    |
extern const int32_t  key_loadgame;                                           //    V
extern const int32_t  key_reverse;
extern const int32_t  key_zoomin;
extern const int32_t  key_zoomout;
extern const int32_t  key_chat;
extern const int32_t  key_backspace;
extern const int32_t  key_enter;
extern const int32_t  key_help;
extern const int32_t  key_soundvolume;
extern const int32_t  key_hud;
extern const int32_t  key_quicksave;
extern const int32_t  key_endgame;
extern const int32_t  key_messages;
extern const int32_t  key_quickload;
extern const int32_t  key_quit;
extern const int32_t  key_gamma;
extern const int32_t  key_spy;
extern const int32_t  key_pause;
extern const int32_t  key_setup;
extern const int32_t  key_forward;
extern const int32_t  key_backward;
extern const int32_t  key_map_right;
extern const int32_t  key_map_left;
extern const int32_t  key_map_up;
extern const int32_t  key_map_down;
extern const int32_t  key_map_zoomin;
extern const int32_t  key_map_zoomout;
extern const int32_t  key_map;
extern const int32_t  key_map_gobig;
extern const int32_t  key_map_follow;
extern const int32_t  key_map_mark;                                           //    ^
extern const int32_t  key_map_clear;                                          //    |
extern const int32_t  key_map_grid;                                           // phares


// killough 5/2/98: moved from d_deh.c:
// CPhipps - Make savedesciption visible in wider scope
#define SAVEDESCLEN 8

#define NUMKEYS   16


#endif
