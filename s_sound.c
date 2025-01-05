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
 *  Copyright 2023-2025 by
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
 * DESCRIPTION:  Platform-independent sound code
 *
 *-----------------------------------------------------------------------------*/

// killough 3/7/98: modified to allow arbitrary listeners in spy mode
// killough 5/2/98: reindented, removed useless code, beautified

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "d_player.h"
#include "s_sound.h"
#include "i_sound.h"
#include "i_system.h"
#include "d_main.h"
#include "r_main.h"
#include "m_random.h"
#include "w_wad.h"

#include "globdata.h"


typedef struct
{
  const sfxinfo_t *sfxinfo;  // sound information (if null, channel avail.)
  int32_t tickend;
  void __far* origin;        // origin of sound
  int16_t handle;          // handle of the sound being played
  boolean is_pickup;       // whether sound is a player's weapon
} channel_t;


// the set of channels available
static channel_t *channels;

// music currently being played
static musicenum_t mus_playing;


// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST (1200L<<FRACBITS)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).

#define S_CLOSE_DIST (160L<<FRACBITS)
#define S_ATTENUATOR ((S_CLIPPING_DIST-S_CLOSE_DIST)>>FRACBITS)

// Adjustable by menu.
#define NORM_PRIORITY 64
#define NORM_SEP      128

#define S_STEREO_SWING		96


// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int16_t snd_SfxVolume = 15;

// Maximum volume of music. Useless so far.
int16_t snd_MusicVolume = 15;


// number of channels available
static const int16_t numChannels = 1;

//
// Internals.
//

static void S_StopChannel(int16_t cnum);

static void S_StopMusic(void);

static boolean S_AdjustSoundParams(mobj_t __far* listener, mobj_t __far* source, int16_t *vol, int16_t *sep);

static int16_t S_getChannel(void __far* origin, const sfxinfo_t *sfxinfo, boolean is_pickup);

// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(int16_t sfxVolume, int16_t musicVolume)
{
    //jff 1/22/98 skip sound init if sound not enabled
    if (!nosfxparm)
    {
        printf("S_Init: default sfx volume %d\n", sfxVolume);

        S_SetSfxVolume(sfxVolume);

        // Allocating the internal channels for mixing
        // (the maximum numer of sounds rendered
        // simultaneously) within zone memory.
        // CPhipps - calloc
        channels =
                (channel_t *) calloc(numChannels,sizeof(channel_t));
    }

    // CPhipps - music init reformatted
    if (!nomusicparm) {
        S_SetMusicVolume(musicVolume);
    }
}


//
// Kills all sounds
//

static void S_Stop(void)
{
    int16_t cnum;

    //jff 1/22/98 skip sound init if sound not enabled
    if (!nosfxparm)
        for (cnum=0 ; cnum<numChannels ; cnum++)
            if (channels[cnum].sfxinfo)
                S_StopChannel(cnum);
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void)
{
    musicenum_t mnum;

    // kill all playing sounds at start of level
    //  (trust me - a good idea)

    S_Stop();

    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;

    mnum = mus_e1m1 + _g_gamemap-1;

    S_ChangeMusic(mnum, true);
}

static void S_StartSoundAtVolume(mobj_t __far* origin, sfxenum_t sfx_id, int16_t volume)
{
    int16_t cnum;
    boolean is_pickup;
    const sfxinfo_t *sfx;

    int16_t sep = NORM_SEP;

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return;

    is_pickup = sfx_id & PICKUP_SOUND || sfx_id == sfx_oof || (sfx_id == sfx_noway);
    sfx_id &= ~PICKUP_SOUND;

    // check for bogus sound #
    if (!(sfx_None < sfx_id && sfx_id < NUMSFX))
        I_Error("S_StartSoundAtVolume: Bad sfx #: %d", sfx_id);

    sfx = &S_sfx[sfx_id];

    // Check to see if it is audible, modify the params

    if (!origin || origin == _g_player.mo)
    {
        volume *= 8;
    }
    else
        if (!S_AdjustSoundParams(_g_player.mo, origin, &volume, &sep))
            return;

    // kill old sound
    for (cnum=0 ; cnum<numChannels ; cnum++)
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin &&
                (channels[cnum].is_pickup == is_pickup))
        {
            S_StopChannel(cnum);
            break;
        }

    // try to find a channel
    cnum = S_getChannel(origin, sfx, is_pickup);

    if (cnum<0)
        return;

    int16_t h = I_StartSound(sfx_id, cnum, volume, sep);
    if (h != -1)
    {
        channels[cnum].handle = h;
        channels[cnum].tickend = (_g_gametic + sfx->ticks);
    }

}

void S_StartSound(mobj_t __far* origin, sfxenum_t sfx_id)
{
    S_StartSoundAtVolume(origin, sfx_id, snd_SfxVolume);
}

void S_StartSound2(degenmobj_t __far* origin, sfxenum_t sfx_id)
{
    //Look at this mess.

    //Originally, the degenmobj_t had
    //a thinker_t at the start of the struct
    //so that it could be passed around and
    //cast to a mobj_t* in the sound code
    //for non-mobj sound makers like doors.

    //This also meant that each and every sector_t
    //struct has 24 bytes wasted. I can't afford
    //to waste memory like that so we have a seperate
    //function for these cases which cobbles toget a temp
    //mobj_t-like struct to pass to the sound code.


    static struct fake_mobj
    {
        thinker_t ununsed;
        degenmobj_t origin;
    } __far fm;

    fm.origin.x = origin->x;
    fm.origin.y = origin->y;

    S_StartSoundAtVolume((mobj_t __far*)&fm, sfx_id, snd_SfxVolume);
}

void S_StopSound(void __far* origin)
{
    int16_t cnum;

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return;

    for (cnum=0 ; cnum<numChannels ; cnum++)
        if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
        {
            S_StopChannel(cnum);
            break;
        }
}


static boolean S_SoundIsPlaying(int16_t cnum)
{
    const channel_t* channel = &channels[cnum];

    if(channel->sfxinfo)
    {
        int32_t ticknow = _g_gametic;

        return (channel->tickend < ticknow);
    }

    return false;
}

//
// Updates music & sounds
//
void S_UpdateSounds(void)
{
	int16_t cnum;
	
	//jff 1/22/98 return if sound is not enabled
	if (nosfxparm)
		return;
	
	for (cnum=0 ; cnum<numChannels ; cnum++)
	{
		const sfxinfo_t *sfx;
		channel_t *c = &channels[cnum];

		if ((sfx = c->sfxinfo))
		{
			if (!S_SoundIsPlaying(c->handle))
			{
				// if channel is allocated but sound has stopped, free it
				S_StopChannel(cnum);
			}
		}
	}
}

void S_SetMusicVolume(int16_t volume)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;

    if (!(0 <= volume && volume <= 15))
        I_Error("S_SetMusicVolume: Attempt to set music volume at %d", volume);

    I_SetMusicVolume(volume);
    snd_MusicVolume = volume;
}



void S_SetSfxVolume(int16_t volume)
{
    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return;

    if (!(0 <= volume && volume <= 127))
        I_Error("S_SetSfxVolume: Attempt to set sfx volume at %d", volume);

    snd_SfxVolume = volume;
}



// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(musicenum_t m_id)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;
    S_ChangeMusic(m_id, false);
}


void S_ChangeMusic(musicenum_t musicnum, boolean looping)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;

    if (!(mus_None < musicnum && musicnum < NUMMUSIC))
        I_Error("S_ChangeMusic: Bad music number %d", musicnum);

    if (mus_playing == musicnum)
        return;

    // shutdown old music
    S_StopMusic();

    // play it
    I_PlaySong(musicnum, looping);

    mus_playing = musicnum;
}


// Stops the music fer sure.
static void S_StopMusic(void)
{
    //jff 1/22/98 return if music is not enabled
    if (nomusicparm)
        return;

    if (mus_playing)
    {
        I_StopSong(mus_None);

        mus_playing = mus_None;
    }
}



static void S_StopChannel(int16_t cnum)
{
    int16_t i;
    channel_t *c = &channels[cnum];

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return;

    if (c->sfxinfo)
    {
        // check to see
        //  if other channels are playing the sound
        for (i=0 ; i<numChannels ; i++)
            if (cnum != i && c->sfxinfo == channels[i].sfxinfo)
                break;

        // degrade usefulness of sound data
        c->sfxinfo = NULL;
        c->tickend = 0;
    }
}

//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns false.
// Otherwise, modifies parameters and returns true.
//

static boolean S_AdjustSoundParams(mobj_t __far* listener, mobj_t __far* source, int16_t *vol, int16_t *sep)
{
	fixed_t adx, ady,approx_dist;

	//jff 1/22/98 return if sound is not enabled
	if (nosfxparm)
		return false;

	// e6y
	// Fix crash when the program wants to S_AdjustSoundParams() for player
	// which is not displayplayer and displayplayer was not spawned at the moment.
	// It happens in multiplayer demos only.
	//
	// Stack trace is:
	// P_SetupLevel() , P_LoadThings() , P_SpawnMapThing() , P_SpawnPlayer(players[0]) ,
	// P_SetupPsprites() , P_BringUpWeapon() , S_StartSound(players[0]->mo, sfx_sawup) ,
	// S_StartSoundAtVolume() , S_AdjustSoundParams(players[displayplayer]->mo, ...);
	// players[displayplayer]->mo is NULL
	//
	// There is no more crash on e1cmnet3.lmp between e1m2 and e1m3
	// http://competn.doom2.net/pub/compet-n/doom/coop/movies/e1cmnet3.zip
	
	if (!listener)
		return false;

	// calculate the distance to sound origin
	// and clip it if necessary
	adx = D_abs(listener->x - source->x);
	ady = D_abs(listener->y - source->y);

	// From _GG1_ p.428. Appox. eucledian distance fast.
	approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

	if (!approx_dist)  // killough 11/98: handle zero-distance as special case
	{
        *vol = snd_SfxVolume;
		return *vol > 0;
	}
	
	if (approx_dist > S_CLIPPING_DIST)
		return false;
	

    // angle of source to listener
    angle_t angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

    if (angle <= listener->angle)
        angle += 0xffffffff;

    angle -= listener->angle;
    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    *sep = 128 - ((S_STEREO_SWING * finesineapprox(angle))>>FRACBITS);


	// volume calculation
	if (approx_dist < S_CLOSE_DIST)
        *vol = snd_SfxVolume*8;
	else
		// distance effect
        *vol = (snd_SfxVolume * ((S_CLIPPING_DIST-approx_dist)>>FRACBITS) * 8) / S_ATTENUATOR;
	
	return (*vol > 0);
}

//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//

static int16_t S_getChannel(void __far* origin, const sfxinfo_t *sfxinfo, boolean is_pickup)
{
    // channel number to use
    int16_t cnum;
    channel_t *c;

    //jff 1/22/98 return if sound is not enabled
    if (nosfxparm)
        return -1;

    // Find an open channel
    for (cnum=0; cnum<numChannels && channels[cnum].sfxinfo; cnum++)
        if (origin && channels[cnum].origin == origin &&
                channels[cnum].is_pickup == is_pickup)
        {
            S_StopChannel(cnum);
            break;
        }

    // None available
    if (cnum == numChannels)
    {      // Look for lower priority
        for (cnum=0 ; cnum<numChannels ; cnum++)
            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority)
                break;
        if (cnum == numChannels)
            return -1;                  // No lower priority.  Sorry, Charlie.
        else
            S_StopChannel(cnum);        // Otherwise, kick out lower priority.
    }

    c = &channels[cnum];              // channel is decided to be cnum.
    c->sfxinfo   = sfxinfo;
    c->origin    = origin;
    c->is_pickup = is_pickup;
    return cnum;
}
