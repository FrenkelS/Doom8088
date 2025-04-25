/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2025 Frenkel Smeijers
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
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "i_system.h"
#include "m_fixed.h"
#include "m_random.h"
#include "p_enemy.h"
#include "p_inter.h"
#include "p_map.h"

#include "globdata.h"


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//

static const fixed_t xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
static const fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

static boolean P_Move(mobj_t __far* actor)
{
  if (actor->movedir == DI_NODIR)
    return false;

#ifdef RANGECHECK
  if (actor->movedir >= 8)
    I_Error ("P_Move: Weird actor->movedir!");
#endif

  int32_t speed = mobjinfo[actor->type].speed;

  fixed_t tryx = actor->x + speed * xspeed[actor->movedir];
  fixed_t tryy = actor->y + speed * yspeed[actor->movedir];

  boolean try_ok = P_TryMove(actor, tryx, tryy);

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

boolean P_TryWalk(mobj_t __far* actor)
{
  if (!P_Move(actor))
    return false;
  actor->movecount = P_Random()&15;
  return true;
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
// A_Look
// Stay in state until a player is sighted.
//

void A_Look(mobj_t __far* actor)
{
    mobj_t __far* targ = actor->subsector->sector->soundtarget;
    actor->threshold = 0; // any shot will wake up
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
           *         and we can see it)
           *  try to find a new one; return if sucessful */

            if (!(actor->target && actor->target->health > 0 && P_CheckSight(actor, actor->target)) && P_LookForTargets(actor, true))
                return;
        }
    }

    // chase towards player
    if (--actor->movecount<0 || !P_Move(actor))
        P_NewChaseDir(actor);

    // make active sound
    if (mobjinfo[actor->type].activesound && P_Random()<3)
        S_StartSound(actor, mobjinfo[actor->type].activesound);
}
