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

void G_Responder(event_t *ev);
void G_CheckDemoStatus(void);
void G_DeferedInitNew(skill_t skill);
void G_DeferedPlayDemo(const char *demo);
void G_LoadGame(int16_t slot);
void G_SaveGame(int16_t slot); // Called by M_Responder.
void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_WorldDone(void);
void G_Ticker(void);
void G_ReloadDefaults(void);     // killough 3/1/98: loads game defaults
void G_PlayerReborn(void);
void G_BuildTiccmd (void);

void G_UpdateSaveGameStrings();

void G_LoadSettings();
void G_SaveSettings();


extern const int16_t  key_menu_right;
extern const int16_t  key_menu_left;
extern const int16_t  key_menu_up;
extern const int16_t  key_menu_down;
extern const int16_t  key_menu_escape;
extern const int16_t  key_menu_enter;
extern const int16_t  key_fire;
extern const int16_t  key_escape;
extern const int16_t  key_map_right;
extern const int16_t  key_map_left;
extern const int16_t  key_map_up;
extern const int16_t  key_map_down;
extern const int16_t  key_map_zoomin;
extern const int16_t  key_map_zoomout;
extern const int16_t  key_map;
extern const int16_t  key_map_follow;


#endif
