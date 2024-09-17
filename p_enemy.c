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
 *      Enemy thinking, AI.
 *      Action Pointer Functions
 *      that are associated with states/frames.
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
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


static const fixed_t distfriend = 128L << FRACBITS;

typedef enum {
  DI_EAST,
  DI_NORTHEAST,
  DI_NORTH,
  DI_NORTHWEST,
  DI_WEST,
  DI_SOUTHWEST,
  DI_SOUTH,
  DI_SOUTHEAST,
  DI_NODIR,
  NUMDIRS
} dirtype_t;


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

static boolean P_CheckMeleeRange(mobj_t __far* actor)
{
  mobj_t __far* pl = actor->target;

  return  // killough 7/18/98: friendly monsters don't attack other friends
    pl && !(actor->flags & pl->flags & MF_FRIEND) &&
    (P_AproxDistance(pl->x-actor->x, pl->y-actor->y) <
     MELEERANGE - 20*FRACUNIT + mobjinfo[pl->type].radius) &&
    P_CheckSight(actor, actor->target);
}

//
// P_HitFriend()
//
// killough 12/98
// This function tries to prevent shooting at friends

static boolean P_HitFriend(mobj_t __far* actor)
{
  return actor->flags & MF_FRIEND && actor->target &&
    (P_AimLineAttack(actor,
         R_PointToAngle2(actor->x, actor->y,
             actor->target->x, actor->target->y),
         P_AproxDistance(actor->x-actor->target->x,
             actor->y-actor->target->y), false),
     _g_linetarget) && _g_linetarget != actor->target &&
    !((_g_linetarget->flags ^ actor->flags) & MF_FRIEND);
}

//
// P_CheckMissileRange
//
static boolean P_CheckMissileRange(mobj_t __far* actor)
{
  fixed_t dist;

  if (!P_CheckSight(actor, actor->target))
    return false;

  if (actor->flags & MF_JUSTHIT)
  {      // the target just hit the enemy, so fight back!
      actor->flags &= ~MF_JUSTHIT;

      /* killough 7/18/98: no friendly fire at corpses
       * killough 11/98: prevent too much infighting among friends
       * cph - yikes, talk about fitting everything on one line... */

      return
              !(actor->flags & MF_FRIEND) ||
              (actor->target->health > 0 &&
               (!(actor->target->flags & MF_FRIEND) ||
                (P_MobjIsPlayer(actor->target) ? true :
                     !(actor->target->flags & MF_JUSTHIT) && P_Random() >128)));
  }

  /* killough 7/18/98: friendly monsters don't attack other friendly
   * monsters or players (except when attacked, and then only once)
   */
  if (actor->flags & actor->target->flags & MF_FRIEND)
    return false;

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

  if (P_HitFriend(actor))
    return false;

  return true;
}

/*
 * P_IsOnLift
 *
 * killough 9/9/98:
 *
 * Returns true if the object is on a lift. Used for AI,
 * since it may indicate the need for crowded conditions,
 * or that a monster should stay on the lift for a while
 * while it goes up or down.
 */

static boolean P_IsOnLift(const mobj_t __far* actor)
{
  const sector_t __far* sec = actor->subsector->sector;

  // Short-circuit: it's on a lift which is active.
  if (sec->floordata && ((thinker_t __far*) sec->floordata)->function==T_PlatRaise)
    return true;

  return false;
}


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//

static const fixed_t xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
static const fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

static boolean P_Move(mobj_t __far* actor)
{
  fixed_t tryx, tryy, deltax, deltay, origx, origy;
  boolean try_ok;
  int32_t speed;

  if (actor->movedir == DI_NODIR)
    return false;

#ifdef RANGECHECK
  if (actor->movedir >= 8)
    I_Error ("P_Move: Weird actor->movedir!");
#endif

  speed = mobjinfo[actor->type].speed;

  tryx = (origx = actor->x) + (deltax = speed * xspeed[actor->movedir]);
  tryy = (origy = actor->y) + (deltay = speed * yspeed[actor->movedir]);

  try_ok = P_TryMove(actor, tryx, tryy);

  if (!try_ok)
    {      // open any specials
      if (!_g_numspechit)
        return false;

      actor->movedir = DI_NODIR;

      /* if the special is not a door that can be opened, return false
       *
       * killough 8/9/98: this is what caused monsters to get stuck in
       * doortracks, because it thought that the monster freed itself
       * by opening a door, even if it was moving towards the doortrack,
       * and not the door itself.
       *
       * killough 9/9/98: If a line blocking the monster is activated,
       * return true 90% of the time. If a line blocking the monster is
       * not activated, but some other line is, return false 90% of the
       * time. A bit of randomness is needed to ensure it's free from
       * lockups, but for most cases, it returns the correct result.
       *
       * Do NOT simply return false 1/4th of the time (causes monsters to
       * back out when they shouldn't, and creates secondary stickiness).
       */

      boolean good = false;
      for ( ; _g_numspechit--; )
        if (P_UseSpecialLine(actor, _g_spechit[_g_numspechit]))
          good = true;

      /* cph - compatibility maze here
       * Boom v2.01 and orig. Doom return "good"
       * Boom v2.02 and LxDoom return good && (P_Random(pr_trywalk)&3)
       * MBF plays even more games
       */
      return good;
    }

  /* fall more slowly, under gravity */
  actor->z = actor->floorz;

  return true;
}


//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//

static boolean P_TryWalk(mobj_t __far* actor)
{
  if (!P_Move(actor))
    return false;
  actor->movecount = P_Random()&15;
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

static boolean PIT_AvoidDropoff(const line_t __far* line)
{
  if (LN_BACKSECTOR(line)                          && // Ignore one-sided linedefs
      _g_tmbbox[BOXRIGHT]  > line->bbox[BOXLEFT]   &&
      _g_tmbbox[BOXLEFT]   < line->bbox[BOXRIGHT]  &&
      _g_tmbbox[BOXTOP]    > line->bbox[BOXBOTTOM] && // Linedef must be contacted
      _g_tmbbox[BOXBOTTOM] < line->bbox[BOXTOP]    &&
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

static void P_NewChaseDir(mobj_t __far* actor)
{
    mobj_t __far* target = actor->target;
    fixed_t deltax = target->x - actor->x;
    fixed_t deltay = target->y - actor->y;

    // killough 8/8/98: sometimes move away from target, keeping distance
    //
    // 1) Stay a certain distance away from a friend, to avoid being in their way
    // 2) Take advantage over an enemy without missiles, by keeping distance

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
    else
    {
        fixed_t dist = P_AproxDistance(deltax, deltay);

        // Move away from friends when too close, except
        // in certain situations (e.g. a crowded lift)

        if (actor->flags & target->flags & MF_FRIEND &&
                distfriend > dist &&
                !P_IsOnLift(target))
        {
            deltax = -deltax, deltay = -deltay;
        }
    }


    P_DoNewChaseDir(actor, deltax, deltay);
}

//
// P_IsVisible
//
// killough 9/9/98: whether a target is visible to a monster
//

static boolean P_IsVisible(mobj_t __far* actor, mobj_t __far* mo, boolean allaround)
{
    if (!allaround)
    {
        angle_t an = R_PointToAngle2(actor->x, actor->y, mo->x, mo->y) - actor->angle;

        if (ANG90 < an && an < ANG270 && P_AproxDistance(mo->x-actor->x, mo->y-actor->y) > MELEERANGE)
            return false;
    }
    return P_CheckSight(actor, mo);
}

//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//

static boolean P_LookForPlayers(mobj_t __far* actor, boolean allaround)
{
    player_t *player;

    if(_g_playeringame)
    {
        player = &_g_player;

        if (player->health <= 0)
            return false;               // dead

        if (!P_IsVisible(actor, player->mo, allaround))
            return false;

        actor->target = player->mo;

        /* killough 9/9/98: give monsters a threshold towards getting players
       * (we don't want it to be too easy for a player with dogs :)
       */
        actor->threshold = 60;

        return true;
    }

    return false;
}

//
// P_LookForTargets
//
// killough 9/5/98: look for targets to go after, depending on kind of monster
//

static boolean P_LookForTargets(mobj_t __far* actor, boolean allaround)
{
    return P_LookForPlayers (actor, allaround);
}



//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//

void A_Look(mobj_t __far* actor)
{
    mobj_t __far* targ = actor->subsector->sector->soundtarget;
    actor->threshold = 0; // any shot will wake up

    /* killough 7/18/98:
   * Friendly monsters go after other monsters first, but
   * also return to player, without attacking them, if they
   * cannot find any targets. A marine's best friend :)
   */
    actor->pursuecount = 0;

    boolean seen = false;

    if (targ && (targ->flags & MF_SHOOTABLE) )
    {
        actor->target = targ;

        if ( actor->flags & MF_AMBUSH )
        {
            if (P_CheckSight (actor, actor->target))
                seen = true;
        }
        else
            seen = true;
    }


    if ( (!seen) && (!P_LookForPlayers (actor, false)))
        return;



    // go into chase state

    if (mobjinfo[actor->type].seesound)
    {
        sfxenum_t sound;
        switch (mobjinfo[actor->type].seesound)
        {
        case sfx_posit1:
        case sfx_posit2:
        //case sfx_posit3:
            sound = sfx_posit1+P_Random()%3;
            break;

        case sfx_bgsit1:
        //case sfx_bgsit2:
            sound = sfx_bgsit1+P_Random()%2;
            break;

        default:
            sound = mobjinfo[actor->type].seesound;
            break;
        }
        S_StartSound(actor, sound);
    }
    P_SetMobjState(actor, mobjinfo[actor->type].seestate);
}

//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//

void A_Chase(mobj_t __far* actor)
{
    if (actor->reactiontime)
        actor->reactiontime--;

    if (actor->threshold)
    { /* modify target threshold */
        if (!actor->target || actor->target->health <= 0)
            actor->threshold = 0;
        else
            actor->threshold--;
    }

    /* turn towards movement direction if not there yet
   * killough 9/7/98: keep facing towards target if strafing or backing out
   */

    if (actor->movedir < 8)
    {
        int32_t delta = (actor->angle &= (((int32_t)7)<<29)) - (((int32_t)actor->movedir) << 29);
        if (delta > 0)
            actor->angle -= ANG90/2;
        else if (delta < 0)
            actor->angle += ANG90/2;
    }

    if (!actor->target || !(actor->target->flags&MF_SHOOTABLE))
    {
        if (!P_LookForTargets(actor,true)) // look for a new target
            P_SetMobjState(actor, mobjinfo[actor->type].spawnstate); // no new target
        return;
    }

    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        if (_g_gameskill != sk_nightmare)
            P_NewChaseDir(actor);
        return;
    }

    // check for melee attack
    if (mobjinfo[actor->type].meleestate && P_CheckMeleeRange(actor))
    {
        if (mobjinfo[actor->type].attacksound)
            S_StartSound(actor, mobjinfo[actor->type].attacksound);

        P_SetMobjState(actor, mobjinfo[actor->type].meleestate);
        /* killough 8/98: remember an attack
      * cph - DEMOSYNC? */
        if (!mobjinfo[actor->type].missilestate)
            actor->flags |= MF_JUSTHIT;
        return;
    }

    // check for missile attack
    if (mobjinfo[actor->type].missilestate)
    {
        if (!(_g_gameskill < sk_nightmare && actor->movecount))
        {
            if (P_CheckMissileRange(actor))
            {
                P_SetMobjState(actor, mobjinfo[actor->type].missilestate);
                actor->flags |= MF_JUSTATTACKED;
                return;
            }
        }
    }


    if (!actor->threshold)
    {
        if (actor->pursuecount)
            actor->pursuecount--;
        else
        {
            /* Our pursuit time has expired. We're going to think about
             * changing targets */
            actor->pursuecount = BASETHRESHOLD;

          /* Unless (we have a live target
           *         and it's not friendly
           *         and we can see it)
           *  try to find a new one; return if sucessful */

            if (!(actor->target && actor->target->health > 0 &&
                  (
                      (((actor->target->flags ^ actor->flags) & MF_FRIEND ||
                        (!(actor->flags & MF_FRIEND))) &&
                       P_CheckSight(actor, actor->target))))
                    && P_LookForTargets(actor, true))
                return;

            /* (Current target was good, or no new target was found.)
           *
           * If monster is a missile-less friend, give up pursuit and
           * return to player, if no attacks have occurred recently.
           */

            if (!mobjinfo[actor->type].missilestate && actor->flags & MF_FRIEND)
            {
                if (actor->flags & MF_JUSTHIT)          /* if recent action, */
                    actor->flags &= ~MF_JUSTHIT;          /* keep fighting */
                else if (P_LookForPlayers(actor, true)) /* else return to player */
                    return;
            }
        }
    }

    // chase towards player
    if (--actor->movecount<0 || !P_Move(actor))
        P_NewChaseDir(actor);

    // make active sound
    if (mobjinfo[actor->type].activesound && P_Random()<3)
        S_StartSound(actor, mobjinfo[actor->type].activesound);
}

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
  slope = P_AimLineAttack(actor, angle, MISSILERANGE, false);
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
  slope = P_AimLineAttack(actor, bangle, MISSILERANGE, false);
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

    if (!(_g_playeringame && _g_player.health > 0))
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
