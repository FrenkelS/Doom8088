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
 *  System interface for sound.
 *
 *-----------------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdint.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "z_zone.h"

#include "m_swap.h"
#include "i_sound.h"
#include "w_wad.h"
#include "s_sound.h"

#include "doomdef.h"
#include "d_player.h"
#include "doomtype.h"

#include "d_main.h"

#include "m_fixed.h"

#include "globdata.h"

//#define __arm__


//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
//
static void addsfx(int32_t sfxid, int32_t channel, int32_t volume, int32_t sep)
{
	UNUSED(sfxid);
	UNUSED(channel);
	UNUSED(volume);
	UNUSED(sep);
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int32_t I_StartSound(int32_t id, int32_t channel, int32_t vol, int32_t sep)
{
	if ((channel < 0) || (channel >= MAX_CHANNELS))
		return -1;

    addsfx(id, channel, vol, sep);

	return channel;
}

//static SDL_AudioSpec audio;

void I_InitSound(void)
{
	// Finished initialization.
    printf("I_InitSound: sound ready\n");
}

void I_PlaySong(int32_t handle, int32_t looping)
{
	UNUSED(looping);

	if(handle == mus_None)
		return;
}


void I_StopSong(int32_t handle)
{
	UNUSED(handle);
}

void I_SetMusicVolume(int32_t volume)
{
	UNUSED(volume);
}
