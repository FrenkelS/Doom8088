/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000,2002 by
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
 * DESCRIPTION:
 *      Enemy thinking, AI.
 *      Action Pointer Functions
 *      that are associated with states/frames.
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
#include "i_system.h"
#include "m_random.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_inter.h"
#include "g_game.h"
#include "p_enemy.h"
#include "p_tick.h"

#include "globdata.h"


static fixed_t dropoff_deltax, dropoff_deltay, floorz;


//
// ENEMY THINKING
// Enemies are always spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//

//
// P_CheckMeleeRange
//

boolean P_CheckMeleeRange(mobj_t __far* actor)
{
  mobj_t __far* pl = actor->target;

  return
    pl &&
    (P_AproxDistance(pl->x-actor->x, pl->y-actor->y) <
     MELEERANGE - 20*FRACUNIT + mobjinfo[pl->type].radius) &&
    P_CheckSight(actor, actor->target);
}


//
// P_CheckMissileRange
//
boolean P_CheckMissileRange(mobj_t __far* actor)
{
  fixed_t dist;

  if (!P_CheckSight(actor, actor->target))
    return false;

  if (actor->flags & MF_JUSTHIT)
  {      // the target just hit the enemy, so fight back!
      actor->flags &= ~MF_JUSTHIT;

      return true;
  }

  if (actor->reactiontime)
    return false;       // do not attack yet

  // OPTIMIZE: get this from a global checksight
  dist = P_AproxDistance ( actor->x-actor->target->x,
                           actor->y-actor->target->y) - 64*FRACUNIT;

  if (!mobjinfo[actor->type].meleestate)
    dist -= 128*FRACUNIT;       // no melee attack, so fire more

  dist >>= FRACBITS;

  if (dist > 200)
    dist = 200;

  if (P_Random() < dist)
    return false;

  return true;
}


//
// P_DoNewChaseDir
//
// killough 9/8/98:
//
// Most of P_NewChaseDir(), except for what
// determines the new direction to take
//

static void P_DoNewChaseDir(mobj_t __far* actor, fixed_t deltax, fixed_t deltay)
{
  int16_t xdir, ydir, tdir;
  uint8_t olddir = actor->movedir;
  uint8_t turnaround = olddir;

  if (turnaround != DI_NODIR)         // find reverse direction
    turnaround ^= 4;

  xdir =
    deltax >  10*FRACUNIT ? DI_EAST :
    deltax < -10*FRACUNIT ? DI_WEST : DI_NODIR;

  ydir =
    deltay < -10*FRACUNIT ? DI_SOUTH :
    deltay >  10*FRACUNIT ? DI_NORTH : DI_NODIR;

  // try direct route
  if (xdir != DI_NODIR && ydir != DI_NODIR && turnaround !=
      (actor->movedir = deltay < 0 ? deltax > 0 ? DI_SOUTHEAST : DI_SOUTHWEST :
       deltax > 0 ? DI_NORTHEAST : DI_NORTHWEST) && P_TryWalk(actor))
    return;

  // try other directions
  if (P_Random() > 200 || D_abs(deltay)>D_abs(deltax))
    tdir = xdir, xdir = ydir, ydir = tdir;

  if ((xdir == turnaround ? xdir = DI_NODIR : xdir) != DI_NODIR &&
      (actor->movedir = xdir, P_TryWalk(actor)))
    return;         // either moved forward or attacked

  if ((ydir == turnaround ? ydir = DI_NODIR : ydir) != DI_NODIR &&
      (actor->movedir = ydir, P_TryWalk(actor)))
    return;

  // there is no direct path to the player, so pick another direction.
  if (olddir != DI_NODIR && (actor->movedir = olddir, P_TryWalk(actor)))
    return;

  // randomly determine direction of search
  if (P_Random() & 1)
  {
      for (tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
          if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
              return;
  }
  else
  {
      for (tdir = DI_SOUTHEAST; tdir >= DI_EAST; tdir--)
          if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
              return;
  }

  if ((actor->movedir = turnaround) != DI_NODIR && !P_TryWalk(actor))
    actor->movedir = DI_NODIR;
}

//
// killough 11/98:
//
// Monsters try to move away from tall dropoffs.
//
// In Doom, they were never allowed to hang over dropoffs,
// and would remain stuck if involuntarily forced over one.
// This logic, combined with p_map.c (P_TryMove), allows
// monsters to free themselves without making them tend to
// hang over dropoffs.

static boolean PIT_AvoidDropoff(line_t __far* line)
{
  if (LN_BACKSECTOR(line)                                               && // Ignore one-sided linedefs
      _g_tmbbox[BOXRIGHT]  > (fixed_t)line->bbox[BOXLEFT]   << FRACBITS &&
      _g_tmbbox[BOXLEFT]   < (fixed_t)line->bbox[BOXRIGHT]  << FRACBITS &&
      _g_tmbbox[BOXTOP]    > (fixed_t)line->bbox[BOXBOTTOM] << FRACBITS && // Linedef must be contacted
      _g_tmbbox[BOXBOTTOM] < (fixed_t)line->bbox[BOXTOP]    << FRACBITS &&
      P_BoxOnLineSide(_g_tmbbox, line) == -1)
    {
      fixed_t front = LN_FRONTSECTOR(line)->floorheight;
      fixed_t back  = LN_BACKSECTOR(line)->floorheight;
      angle_t angle;

      // The monster must contact one of the two floors,
      // and the other must be a tall dropoff (more than 24).

      if (back == floorz && front < floorz - FRACUNIT*24)
  angle = R_PointToAngle2(0,0,(fixed_t)line->dx<<FRACBITS,(fixed_t)line->dy<<FRACBITS);   // front side dropoff
      else
  if (front == floorz && back < floorz - FRACUNIT*24)
    angle = R_PointToAngle2((fixed_t)line->dx<<FRACBITS,(fixed_t)line->dy<<FRACBITS,0,0); // back side dropoff
  else
    return true;

      // Move away from dropoff at a standard speed.
      // Multiple contacted linedefs are cumulative (e.g. hanging over corner)
      dropoff_deltax -= finesine(  angle >> ANGLETOFINESHIFT)*32;
      dropoff_deltay += finecosine(angle >> ANGLETOFINESHIFT)*32;
    }
  return true;
}

//
// Driver for above
//

static boolean P_AvoidDropoff(mobj_t __far* actor)
{
  int16_t yh=((_g_tmbbox[BOXTOP]   = actor->y+actor->radius)-_g_bmaporgy)>>MAPBLOCKSHIFT;
  int16_t yl=((_g_tmbbox[BOXBOTTOM]= actor->y-actor->radius)-_g_bmaporgy)>>MAPBLOCKSHIFT;
  int16_t xh=((_g_tmbbox[BOXRIGHT] = actor->x+actor->radius)-_g_bmaporgx)>>MAPBLOCKSHIFT;
  int16_t xl=((_g_tmbbox[BOXLEFT]  = actor->x-actor->radius)-_g_bmaporgx)>>MAPBLOCKSHIFT;
  int16_t bx, by;

  floorz = actor->z;            // remember floor height

  dropoff_deltax = dropoff_deltay = 0;

  // check lines

  validcount++;
  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      P_BlockLinesIterator(bx, by, PIT_AvoidDropoff);  // all contacted lines

  return (dropoff_deltax | dropoff_deltay) != 0;   // Non-zero if movement prescribed
}


//
// P_NewChaseDir
//
// killough 9/8/98: Split into two functions
//

void P_NewChaseDir(mobj_t __far* actor)
{
    mobj_t __far* target = actor->target;
    fixed_t deltax = target->x - actor->x;
    fixed_t deltay = target->y - actor->y;

    // killough 8/8/98: sometimes move away from target, keeping distance
    // Take advantage over an enemy without missiles, by keeping distance

    if (actor->floorz - actor->dropoffz > FRACUNIT*24 &&
            actor->z <= actor->floorz &&
            !(actor->flags & MF_DROPOFF) &&
            P_AvoidDropoff(actor)) /* Move away from dropoff */
    {
        P_DoNewChaseDir(actor, dropoff_deltax, dropoff_deltay);

        // If moving away from dropoff, set movecount to 1 so that
        // small steps are taken to get monster away from dropoff.

        actor->movecount = 1;
        return;
    }

    P_DoNewChaseDir(actor, deltax, deltay);
}


//
// ACTION ROUTINES
//

//
// A_FaceTarget
//
void A_FaceTarget(mobj_t __far* actor)
{
  if (!actor->target)
    return;
  actor->flags &= ~MF_AMBUSH;
  actor->angle = R_PointToAngle2(actor->x, actor->y,
                                 actor->target->x, actor->target->y);
  if (actor->target->flags & MF_SHADOW)
    { // killough 5/5/98: remove dependence on order of evaluation:
      int32_t t = P_Random();
      actor->angle += (t-P_Random())<<21;
    }
}

//
// A_PosAttack
//

void A_PosAttack(mobj_t __far* actor)
{
  angle_t angle;
  int16_t damage;
  fixed_t slope;
  angle_t t;

  if (!actor->target)
    return;
  A_FaceTarget(actor);
  angle = actor->angle;
  slope = P_AimLineAttack(actor, angle, MISSILERANGE);
  S_StartSound(actor, sfx_pistol);

  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random();
  angle += (t - P_Random())<<20;
  damage = (P_Random()%5 + 1)*3;
  P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack(mobj_t __far* actor)
{
  int16_t i;
  angle_t bangle;
  fixed_t slope;

  if (!actor->target)
    return;
  S_StartSound(actor, sfx_shotgn);
  A_FaceTarget(actor);
  bangle = actor->angle;
  slope = P_AimLineAttack(actor, bangle, MISSILERANGE);
  for (i=0; i<3; i++)
    {  // killough 5/5/98: remove dependence on order of evaluation:
      angle_t t = P_Random();
      angle_t angle = bangle + ((t - P_Random())<<20);
      int16_t damage = ((P_Random()%5)+1)*3;
      P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
    }
}


//
// A_TroopAttack
//

void A_TroopAttack(mobj_t __far* actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  if (P_CheckMeleeRange(actor))
    {
      int16_t damage;
      S_StartSound(actor, sfx_claw);
      damage = (P_Random()%8+1)*3;
      P_DamageMobj(actor->target, actor, actor, damage);
      return;
    }
  P_SpawnMissile(actor, actor->target, MT_TROOPSHOT);  // launch a missile
}

void A_SargAttack(mobj_t __far* actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  if (P_CheckMeleeRange(actor))
    {
      int16_t damage = ((P_Random()%10)+1)*4;
      P_DamageMobj(actor->target, actor, actor, damage);
    }
}


void A_CyberAttack(mobj_t __far* actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  S_StartSound(actor, sfx_rlaunc);
  P_SpawnMissile(actor, actor->target, MT_ROCKET);
}

void A_BruisAttack(mobj_t __far* actor)
{
  if (!actor->target)
    return;
  if (P_CheckMeleeRange(actor))
    {
      int16_t damage;
      S_StartSound(actor, sfx_claw);
      damage = (P_Random()%8+1)*10;
      P_DamageMobj(actor->target, actor, actor, damage);
      return;
    }
  P_SpawnMissile(actor, actor->target, MT_BRUISERSHOT);  // launch a missile
}


void A_Scream(mobj_t __far* actor)
{
  sfxenum_t sound;

  switch (mobjinfo[actor->type].deathsound)
    {
    case 0:
      return;

    case sfx_podth1:
    case sfx_podth2:
    //case sfx_podth3:
      sound = sfx_podth1 + P_Random()%3;
      break;

    case sfx_bgdth1:
    //case sfx_bgdth2:
      sound = sfx_bgdth1 + P_Random()%2;
      break;

    default:
      sound = mobjinfo[actor->type].deathsound;
      break;
    }

  S_StartSound(actor, sound);
}

void A_XScream(mobj_t __far* actor)
{
  S_StartSound(actor, sfx_slop);
}

void A_Pain(mobj_t __far* actor)
{
  if (mobjinfo[actor->type].painsound)
    S_StartSound(actor, mobjinfo[actor->type].painsound);
}

void A_Fall(mobj_t __far* actor)
{
  // actor is on ground, it can be walked over
  actor->flags &= ~MF_SOLID;
}

//
// A_Explode
//
void A_Explode(mobj_t __far* thingy)
{
  P_RadiusAttack( thingy, thingy->target, 128 );
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//

void A_BossDeath(mobj_t __far* mo)
{
    thinker_t __far* th;
    line_t    junk;

    if (_g_gamemap != 8)
        return;

    if (mo->type != MT_BRUISER)
        return;

    if (_g_player.health <= 0)
        return;     // no one left alive, so do not end game

    // scan the remaining thinkers to see
    // if all bosses are dead
    for (th = _g_thinkerclasscap.next; th != &_g_thinkerclasscap; th = th->next)
        if (th->function == P_MobjThinker)
        {
            mobj_t __far* mo2 = (mobj_t __far*) th;
            if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
                return;         // other boss not dead
        }

    // victory!
    junk.tag = 666;
    EV_DoFloor(&junk, lowerFloorToLowest);
}


void A_PlayerScream(mobj_t __far* mo)
{
  sfxenum_t sound = sfx_pldeth;  // Default death sound.
  S_StartSound(mo, sound);
}
