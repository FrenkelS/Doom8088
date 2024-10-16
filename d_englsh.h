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
 *  Printed strings for translation.
 *  English language support (default).
 *
 *-----------------------------------------------------------------------------*/

#ifndef __D_ENGLSH__
#define __D_ENGLSH__

/* m_menu.c */
#define PRESSKEY    "Press a key."
#define PRESSYN     "Press Y or N."
#define SAVEDEAD    "You can't save if\nyou aren't playing!\n\n"PRESSKEY

#define NIGHTMARE \
  "Are you sure? This skill level\n"\
  "isn't even remotely fair.\n\n"PRESSYN

#define MSGOFF      "Messages Off"
#define MSGON       "Messages On"

#define RUNOFF      "Take your time."
#define RUNON       "In a hurry, marine?"

#define ENDGAME     "Are you sure you want to\nend the game?\n\n"PRESSYN
#define QUITMSG	    "Are you sure you want to\nquit this great game?\n\n"PRESSYN


/* p_inter.c */
#define GOTARMOR    "Picked up the armor."
#define GOTMEGA     "Picked up the MegaArmor!"
#define GOTHTHBONUS "Picked up a health bonus."
#define GOTARMBONUS "Picked up an armor bonus."
#define GOTSTIM     "Picked up a stimpack."
#define GOTMEDIKIT  "Picked up a medikit."
//#define GOTMEDINEED "Picked up a medikit that you REALLY need!" - String is too long for GBA res
#define GOTMEDINEED  "Picked up a medikit."
#define GOTSUPER    "Supercharge!"

#define GOTBLUECARD "Picked up a blue keycard."
#define GOTYELWCARD "Picked up a yellow keycard."
#define GOTREDCARD  "Picked up a red keycard."

#define GOTINVIS    "Partial Invisibility"
#define GOTSUIT     "Radiation Shielding Suit"
#define GOTMAP      "Computer Area Map"
#define GOTVISOR    "Light Amplification Visor"

#define GOTCLIP     "Picked up a clip."
#define GOTCLIPBOX  "Picked up a box of bullets."
#define GOTROCKET   "Picked up a rocket."
#define GOTROCKBOX  "Picked up a box of rockets."
#define GOTSHELLS   "Picked up 4 shotgun shells."
#define GOTSHELLBOX "Picked up a box of shotgun shells"
#define GOTBACKPACK "Picked up a backpack full of ammo"

#define GOTCHAINGUN "You got the chaingun!"
#define GOTCHAINSAW "A chainsaw!  Find some meat!"
#define GOTLAUNCHER "You got the rocket launcher!"
#define GOTSHOTGUN  "You got the shotgun!"

/* p_doors.c */
#define PD_BLUEK    "You need a blue key to open this"
#define PD_REDK     "You need a red key to open this"
#define PD_YELLOWK  "You need a yellow key to open this"

/* g_game.c */
#define GGSAVED     "game saved."

/* hu_stuff.c */
#define HUSTR_E1M1  "E1M1: Hangar"
#define HUSTR_E1M2  "E1M2: Nuclear Plant"
#define HUSTR_E1M3  "E1M3: Toxin Refinery"
#define HUSTR_E1M4  "E1M4: Command Control"
#define HUSTR_E1M5  "E1M5: Phobos Lab"
#define HUSTR_E1M6  "E1M6: Central Processing"
#define HUSTR_E1M7  "E1M7: Computer Station"
#define HUSTR_E1M8  "E1M8: Phobos Anomaly"
#define HUSTR_E1M9  "E1M9: Military Base"


/* am_map.c */

#define AMSTR_FOLLOWON    "Follow Mode On"
#define AMSTR_FOLLOWOFF   "Follow Mode Off"

/* st_stuff.c */

#define STSTR_DQDON     "Degreelessness Mode On"
#define STSTR_DQDOFF    "Degreelessness Mode Off"

#define STSTR_KFAADDED  "Very Happy Ammo Added"
#define STSTR_FAADDED   "Ammo (no keys) Added"

#define STSTR_NCON      "No Clipping Mode On"
#define STSTR_NCOFF     "No Clipping Mode Off"

#define STSTR_CHOPPERS  "... doesn't suck - GM"

#define STSTR_ROCKETON    "Enemy Rockets On"
#define STSTR_ROCKETOFF   "Enemy Rockets Off"

#define STSTR_FPSON    "FPS Counter On"
#define STSTR_FPSOFF   "FPS Counter Off"

/* f_finale.c */

#define E1TEXT \
  "Once you beat the big badasses\n"\
  "and clean out the moon base\n"\
  "you're supposed to win?\n"\
  "Where's your ticket home?\n"\
  "What the hell is this? It's not\n"\
  "supposed to end this way!\n"\
  "\n" \
  "It stinks like rotten meat, but\n"\
  "looks like the lost Deimos base.\n"\
  "You're stuck on The Shores of\n"\
  "Hell.\n"\
  "The only way out is through."


#endif
