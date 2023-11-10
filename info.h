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
 *      Thing frame/state LUT,
 *      generated by multigen utilitiy.
 *      This one is the original DOOM version, preserved.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __INFO__
#define __INFO__

/* Needed for action function pointer handling. */
#include "d_think.h"
#include "sounds.h"


/********************************************************************
 * Sprite name enumeration - must match info.c                      *
 ********************************************************************/
typedef enum
{
  SPR_TROO,
  SPR_SHTG,
  SPR_PUNG,
  SPR_PISG,
  SPR_PISF,
  SPR_SHTF,
  SPR_CHGG,
  SPR_CHGF,
  SPR_MISG,
  SPR_MISF,
  SPR_SAWG,
  SPR_BLUD,
  SPR_PUFF,
  SPR_BAL1,
  SPR_MISL,
  SPR_TFOG,
  SPR_IFOG,
  SPR_PLAY,
  SPR_POSS,
  SPR_SPOS,
  SPR_SARG,
  SPR_BAL7,
  SPR_BOSS,
  SPR_ARM1,
  SPR_ARM2,
  SPR_BAR1,
  SPR_BEXP,
  SPR_BON1,
  SPR_BON2,
  SPR_BKEY,
  SPR_RKEY,
  SPR_YKEY,
  SPR_STIM,
  SPR_MEDI,
  SPR_SOUL,
  SPR_PINS,
  SPR_SUIT,
  SPR_PMAP,
  SPR_PVIS,
  SPR_CLIP,
  SPR_AMMO,
  SPR_ROCK,
  SPR_BROK,
  SPR_SHEL,
  SPR_SBOX,
  SPR_BPAK,
  SPR_MGUN,
  SPR_CSAW,
  SPR_LAUN,
  SPR_SHOT,
  SPR_COLU,
  SPR_POL5,
  SPR_CAND,
  SPR_CBRA,
  SPR_ELEC,
  SPR_TRED,
  NUMSPRITES  /* counter of how many there are */
} spritenum_t;

/********************************************************************
 * States (frames) enumeration -- must match info.c                 *
 ********************************************************************/

typedef enum
{
  S_NULL,
  S_LIGHTDONE,
  S_PUNCH,
  S_PUNCHDOWN,
  S_PUNCHUP,
  S_PUNCH1,
  S_PUNCH2,
  S_PUNCH3,
  S_PUNCH4,
  S_PUNCH5,
  S_PISTOL,
  S_PISTOLDOWN,
  S_PISTOLUP,
  S_PISTOL1,
  S_PISTOL2,
  S_PISTOL3,
  S_PISTOL4,
  S_PISTOLFLASH,
  S_SGUN,
  S_SGUNDOWN,
  S_SGUNUP,
  S_SGUN1,
  S_SGUN2,
  S_SGUN3,
  S_SGUN4,
  S_SGUN5,
  S_SGUN6,
  S_SGUN7,
  S_SGUN8,
  S_SGUN9,
  S_SGUNFLASH1,
  S_SGUNFLASH2,
  S_CHAIN,
  S_CHAINDOWN,
  S_CHAINUP,
  S_CHAIN1,
  S_CHAIN2,
  S_CHAIN3,
  S_CHAINFLASH1,
  S_CHAINFLASH2,
  S_MISSILE,
  S_MISSILEDOWN,
  S_MISSILEUP,
  S_MISSILE1,
  S_MISSILE2,
  S_MISSILE3,
  S_MISSILEFLASH1,
  S_MISSILEFLASH2,
  S_MISSILEFLASH3,
  S_MISSILEFLASH4,
  S_SAW,
  S_SAWB,
  S_SAWDOWN,
  S_SAWUP,
  S_SAW1,
  S_SAW2,
  S_SAW3,
  S_BLOOD1,
  S_BLOOD2,
  S_BLOOD3,
  S_PUFF1,
  S_PUFF2,
  S_PUFF3,
  S_PUFF4,
  S_TBALL1,
  S_TBALL2,
  S_TBALLX1,
  S_TBALLX2,
  S_TBALLX3,
  S_ROCKET,
  S_EXPLODE1,
  S_EXPLODE2,
  S_EXPLODE3,
  S_TFOG,
  S_TFOG01,
  S_TFOG02,
  S_TFOG2,
  S_TFOG3,
  S_TFOG4,
  S_TFOG5,
  S_TFOG6,
  S_TFOG7,
  S_TFOG8,
  S_TFOG9,
  S_TFOG10,
  S_IFOG,
  S_IFOG01,
  S_IFOG02,
  S_IFOG2,
  S_IFOG3,
  S_IFOG4,
  S_IFOG5,
  S_PLAY,
  S_PLAY_RUN1,
  S_PLAY_RUN2,
  S_PLAY_RUN3,
  S_PLAY_RUN4,
  S_PLAY_ATK1,
  S_PLAY_ATK2,
  S_PLAY_PAIN,
  S_PLAY_PAIN2,
  S_PLAY_DIE1,
  S_PLAY_DIE2,
  S_PLAY_DIE3,
  S_PLAY_DIE4,
  S_PLAY_DIE5,
  S_PLAY_DIE6,
  S_PLAY_DIE7,
  S_PLAY_XDIE1,
  S_PLAY_XDIE2,
  S_PLAY_XDIE3,
  S_PLAY_XDIE4,
  S_PLAY_XDIE5,
  S_PLAY_XDIE6,
  S_PLAY_XDIE7,
  S_PLAY_XDIE8,
  S_PLAY_XDIE9,
  S_POSS_STND,
  S_POSS_STND2,
  S_POSS_RUN1,
  S_POSS_RUN2,
  S_POSS_RUN3,
  S_POSS_RUN4,
  S_POSS_RUN5,
  S_POSS_RUN6,
  S_POSS_RUN7,
  S_POSS_RUN8,
  S_POSS_ATK1,
  S_POSS_ATK2,
  S_POSS_ATK3,
  S_POSS_PAIN,
  S_POSS_PAIN2,
  S_POSS_DIE1,
  S_POSS_DIE2,
  S_POSS_DIE3,
  S_POSS_DIE4,
  S_POSS_DIE5,
  S_POSS_XDIE1,
  S_POSS_XDIE2,
  S_POSS_XDIE3,
  S_POSS_XDIE4,
  S_POSS_XDIE5,
  S_POSS_XDIE6,
  S_POSS_XDIE7,
  S_POSS_XDIE8,
  S_POSS_XDIE9,
  S_SPOS_STND,
  S_SPOS_STND2,
  S_SPOS_RUN1,
  S_SPOS_RUN2,
  S_SPOS_RUN3,
  S_SPOS_RUN4,
  S_SPOS_RUN5,
  S_SPOS_RUN6,
  S_SPOS_RUN7,
  S_SPOS_RUN8,
  S_SPOS_ATK1,
  S_SPOS_ATK2,
  S_SPOS_ATK3,
  S_SPOS_PAIN,
  S_SPOS_PAIN2,
  S_SPOS_DIE1,
  S_SPOS_DIE2,
  S_SPOS_DIE3,
  S_SPOS_DIE4,
  S_SPOS_DIE5,
  S_SPOS_XDIE1,
  S_SPOS_XDIE2,
  S_SPOS_XDIE3,
  S_SPOS_XDIE4,
  S_SPOS_XDIE5,
  S_SPOS_XDIE6,
  S_SPOS_XDIE7,
  S_SPOS_XDIE8,
  S_SPOS_XDIE9,
  S_TROO_STND,
  S_TROO_STND2,
  S_TROO_RUN1,
  S_TROO_RUN2,
  S_TROO_RUN3,
  S_TROO_RUN4,
  S_TROO_RUN5,
  S_TROO_RUN6,
  S_TROO_RUN7,
  S_TROO_RUN8,
  S_TROO_ATK1,
  S_TROO_ATK2,
  S_TROO_ATK3,
  S_TROO_PAIN,
  S_TROO_PAIN2,
  S_TROO_DIE1,
  S_TROO_DIE2,
  S_TROO_DIE3,
  S_TROO_DIE4,
  S_TROO_DIE5,
  S_TROO_XDIE1,
  S_TROO_XDIE2,
  S_TROO_XDIE3,
  S_TROO_XDIE4,
  S_TROO_XDIE5,
  S_TROO_XDIE6,
  S_TROO_XDIE7,
  S_TROO_XDIE8,
  S_SARG_STND,
  S_SARG_STND2,
  S_SARG_RUN1,
  S_SARG_RUN2,
  S_SARG_RUN3,
  S_SARG_RUN4,
  S_SARG_RUN5,
  S_SARG_RUN6,
  S_SARG_RUN7,
  S_SARG_RUN8,
  S_SARG_ATK1,
  S_SARG_ATK2,
  S_SARG_ATK3,
  S_SARG_PAIN,
  S_SARG_PAIN2,
  S_SARG_DIE1,
  S_SARG_DIE2,
  S_SARG_DIE3,
  S_SARG_DIE4,
  S_SARG_DIE5,
  S_SARG_DIE6,
  S_BRBALL1,
  S_BRBALL2,
  S_BRBALLX1,
  S_BRBALLX2,
  S_BRBALLX3,
  S_BOSS_STND,
  S_BOSS_STND2,
  S_BOSS_RUN1,
  S_BOSS_RUN2,
  S_BOSS_RUN3,
  S_BOSS_RUN4,
  S_BOSS_RUN5,
  S_BOSS_RUN6,
  S_BOSS_RUN7,
  S_BOSS_RUN8,
  S_BOSS_ATK1,
  S_BOSS_ATK2,
  S_BOSS_ATK3,
  S_BOSS_PAIN,
  S_BOSS_PAIN2,
  S_BOSS_DIE1,
  S_BOSS_DIE2,
  S_BOSS_DIE3,
  S_BOSS_DIE4,
  S_BOSS_DIE5,
  S_BOSS_DIE6,
  S_BOSS_DIE7,
  S_ARM1,
  S_ARM1A,
  S_ARM2,
  S_ARM2A,
  S_BAR1,
  S_BAR2,
  S_BEXP,
  S_BEXP2,
  S_BEXP3,
  S_BEXP4,
  S_BEXP5,
  S_BON1,
  S_BON1A,
  S_BON1B,
  S_BON1C,
  S_BON1D,
  S_BON1E,
  S_BON2,
  S_BON2A,
  S_BON2B,
  S_BON2C,
  S_BON2D,
  S_BON2E,
  S_BKEY,
  S_BKEY2,
  S_RKEY,
  S_RKEY2,
  S_YKEY,
  S_YKEY2,
  S_STIM,
  S_MEDI,
  S_SOUL,
  S_SOUL2,
  S_SOUL3,
  S_SOUL4,
  S_SOUL5,
  S_SOUL6,
  S_PINS,
  S_PINS2,
  S_PINS3,
  S_PINS4,
  S_SUIT,
  S_PMAP,
  S_PMAP2,
  S_PMAP3,
  S_PMAP4,
  S_PMAP5,
  S_PMAP6,
  S_PVIS,
  S_PVIS2,
  S_CLIP,
  S_AMMO,
  S_ROCK,
  S_BROK,
  S_SHEL,
  S_SBOX,
  S_BPAK,
  S_MGUN,
  S_CSAW,
  S_LAUN,
  S_SHOT,
  S_COLU,
  S_DEADTORSO,
  S_DEADBOTTOM,
  S_GIBS,
  S_CANDLESTIK,
  S_CANDELABRA,
  S_TECHPILLAR,
  S_REDTORCH,
  S_REDTORCH2,
  S_REDTORCH3,
  S_REDTORCH4,
  NUMSTATES  /* Counter of how many there are */
} statenum_t;

/********************************************************************
 * Definition of the state (frames) structure                       *
 ********************************************************************/

typedef struct
{
  spritenum_t sprite;       /* sprite number to show                       */
  uint16_t    frame;        /* which frame/subframe of the sprite is shown */
  int16_t     tics;         /* number of gametics this frame should last   */
  actionf_t   action;       /* code pointer to function for action if any  */
  statenum_t  nextstate;    /* linked list pointer to next state or zero   */
} state_t;

/* these are in info.c */
extern const state_t  states[NUMSTATES];
extern const char* const sprnames[]; /* 1/17/98 killough - CPhipps - const */

/********************************************************************
 * Thing enumeration -- must match info.c                           *
 ********************************************************************
 * Note that many of these are generically named for the ornamentals
 */

typedef enum {
  MT_PLAYER,
  MT_POSSESSED,
  MT_SHOTGUY,
  MT_TROOP,
  MT_SERGEANT,
  MT_SHADOWS,
  MT_BRUISER,
  MT_BRUISERSHOT,
  MT_BARREL,
  MT_TROOPSHOT,
  MT_ROCKET,
  MT_PUFF,
  MT_BLOOD,
  MT_TFOG,
  MT_IFOG,
  MT_TELEPORTMAN,
  MT_MISC0,
  MT_MISC1,
  MT_MISC2,
  MT_MISC3,
  MT_MISC4,
  MT_MISC5,
  MT_MISC6,
  MT_MISC10,
  MT_MISC11,
  MT_MISC12,
  MT_INS,
  MT_MISC14,
  MT_MISC15,
  MT_MISC16,
  MT_CLIP,
  MT_MISC17,
  MT_MISC18,
  MT_MISC19,
  MT_MISC22,
  MT_MISC23,
  MT_MISC24,
  MT_CHAINGUN,
  MT_MISC26,
  MT_MISC27,
  MT_SHOTGUN,
  MT_MISC31,
  MT_MISC43,
  MT_MISC48,
  MT_MISC49,
  MT_MISC50,
  MT_MISC62,
  MT_MISC63,
  MT_MISC64,
  MT_MISC66,
  MT_MISC67,
  MT_MISC68,
  MT_MISC69,
  MT_MISC71,
  MT_NOTHING,
  NUMMOBJTYPES  // Counter of how many there are
} mobjtype_t;

/********************************************************************
 * Definition of the Thing structure
 ********************************************************************/
/* Note that these are only indices to the state, sound, etc. arrays
 * and not actual pointers.  Most can be set to zero if the action or
 * sound doesn't apply (like lamps generally don't attack or whistle).
 */

sfxenum_t getSeeSound(mobjtype_t type);
sfxenum_t getDeathSound(mobjtype_t type);

int16_t getSpawnHealth(mobjtype_t type);

typedef struct
{
  int16_t doomednum;
  /* Thing number used in id's editor, and now
       probably by every other editor too */
  statenum_t spawnstate;
  /* The state (frame) index when this Thing is
           first created */
  statenum_t seestate;     /* The state when it sees you or wakes up */
  int16_t reactiontime; /* How many tics it waits after it wakes up
           before it will start to attack, in normal
           skills (halved for nightmare) */
  statenum_t painstate;    /* The state to indicate pain */
  uint8_t painchance;   /* A number that is checked against a random
           number 0-255 to see if the Thing is supposed
           to go to its painstate or not.  Note this
           has absolutely nothing to do with the chance
           it will get hurt, just the chance of it
           reacting visibly. */
  statenum_t meleestate;   /* Melee==close attack */
  statenum_t missilestate; /* What states to use when it's in the air, if
           in fact it is ever used as a missile */
  statenum_t deathstate;   /* What state begins the death sequence */
  statenum_t xdeathstate;  /* What state begins the horrible death sequence
           like when a rocket takes out a trooper */
  int32_t speed;        /* How fast it moves.  Too fast and it can miss
           collision logic. */
  int32_t radius;       /* An often incorrect radius */
  int32_t height;       /* An often incorrect height, used only to see
           if a monster can enter a sector */
  int16_t mass;         /* How much an impact will move it.  Cacodemons
           seem to retreat when shot because they have
           very little mass and are moved by impact */
  int16_t damage;       /* If this is a missile, how much does it hurt? */
  uint32_t flags;  /* Bit masks for lots of things.  See p_mobj.h */
} mobjinfo_t;

/* See p_mobj_h for addition more technical info */
extern const mobjinfo_t mobjinfo[NUMMOBJTYPES];

#endif
