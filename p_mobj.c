/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
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
 *      Moving object handling. Spawn functions.
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "compiler.h"
#include "doomdef.h"
#include "d_player.h"
#include "m_random.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_tick.h"
#include "sounds.h"
#include "s_sound.h"
#include "info.h"
#include "g_game.h"
#include "p_inter.h"
#include "i_system.h"

#include "globdata.h"


void A_CyberAttack(mobj_t __far* actor);


//
// P_SetMobjState
// Returns true if the mobj is still present.
//

boolean P_SetMobjState(mobj_t __far* mobj, statenum_t state)
{
    const state_t*	st;

    do
    {
        if (state == S_NULL)
        {
            mobj->state = (state_t *) S_NULL;
            P_RemoveMobj (mobj);
            return false;
        }

        st = &states[state];
        mobj->state = st;
        mobj->tics = st->tics;
        mobj->sprite = st->sprite;
        mobj->frame = st->frame;

        // Modified handling.
        // Call action functions when the state is set
        if(st->action)
        {
            if(!(_g_player.cheats & CF_ENEMY_ROCKETS))
            {
                st->action(mobj);
            }
            else
            {
                if(mobjinfo[mobj->type].missilestate && (state >= mobjinfo[mobj->type].missilestate) && (state < mobjinfo[mobj->type].painstate))
                    A_CyberAttack(mobj);
                else
                    st->action(mobj);
            }
        }

        state = st->nextstate;

    } while (!mobj->tics);

    return true;
}


//
// P_ExplodeMissile
//

static void P_ExplodeMissile(mobj_t __far* mo)
  {
  mo->momx = mo->momy = mo->momz = 0;

  P_SetMobjState (mo, mobjinfo[mo->type].deathstate);

  mo->tics -= P_Random()&3;

  if (mo->tics < 1)
    mo->tics = 1;

  mo->flags &= ~MF_MISSILE;

  if (mobjinfo[mo->type].deathsound)
    S_StartSound (mo, mobjinfo[mo->type].deathsound);
  }


//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//

static fixed_t   bestslidefrac;
static const line_t __far*   bestslideline;
static mobj_t __far*   slidemo;
static fixed_t   tmxmove;
static fixed_t   tmymove;

//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
// If the floor is icy, then you can bounce off a wall.             // phares
//

static void P_HitSlideLine(const line_t __far* ld)
{
    int16_t     side;
    angle_t lineangle;
    angle_t moveangle;
    angle_t deltaangle;
    fixed_t movelen;
    fixed_t newlen;
    //   |
    // Under icy conditions, if the angle of approach to the wall     //   V
    // is more than 45 degrees, then you'll bounce and lose half
    // your momentum. If less than 45 degrees, you'll slide along
    // the wall. 45 is arbitrary and is believable.

    // Check for the special cases of horz or vert walls.

    /* killough 10/98: only bounce if hit hard (prevents wobbling)
   * cph - DEMOSYNC - should only affect players in Boom demos? */

    if (ld->dy == 0)
    {
        tmymove = 0; // no more movement in the Y direction
        return;
    }

    if (ld->dx == 0)
    {                                                          // phares
        tmxmove = 0; // no more movement in the X direction
        return;
    }

    // The wall is angled. Bounce if the angle of approach is         // phares
    // less than 45 degrees.                                          // phares

    side = P_PointOnLineSide (slidemo->x, slidemo->y, ld);

    lineangle = R_PointToAngle2 (0,0, (fixed_t)ld->dx<<FRACBITS, (fixed_t)ld->dy<<FRACBITS);
    if (side == 1)
        lineangle += ANG180;

    moveangle = R_PointToAngle2 (0,0, tmxmove, tmymove);

    // killough 3/2/98:
    // The moveangle+=10 breaks v1.9 demo compatibility in
    // some demos, so it needs demo_compatibility switch.

    moveangle += 10; // prevents sudden path reversal due to        // phares
    // rounding error                              //   |
    deltaangle = moveangle-lineangle;                                 //   V
    movelen = P_AproxDistance (tmxmove, tmymove);
    //   |
    // phares
    if (deltaangle > ANG180)
        deltaangle += ANG180;

    //  I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;
    newlen  = FixedMulAngle(movelen, finecosine(deltaangle));
    tmxmove = FixedMulAngle(newlen,  finecosine(lineangle));
    tmymove = FixedMulAngle(newlen,  finesine(  lineangle));
    // phares
}


//
// PTR_SlideTraverse
//

static boolean PTR_SlideTraverse (intercept_t* in)
  {
  const line_t __far* li;

  if (!in->isaline)
    I_Error ("PTR_SlideTraverse: not a line?");

  li = in->d.line;

  if ( ! (li->flags & ML_TWOSIDED) )
    {
    if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
      return true; // don't hit the back side
    goto isblocking;
    }

  // set openrange, opentop, openbottom.
  // These define a 'window' from one sector to another across a line

  P_LineOpening (li);

  if (_g_openrange < slidemo->height)
    goto isblocking;  // doesn't fit

  if (_g_opentop - slidemo->z < slidemo->height)
    goto isblocking;  // mobj is too high

  if (_g_openbottom - slidemo->z > 24*FRACUNIT )
    goto isblocking;  // too big a step up

  // this line doesn't block movement

  return true;

  // the line does block movement,
  // see if it is closer than best so far

isblocking:

  if (in->frac < bestslidefrac)
    {
    bestslidefrac = in->frac;
    bestslideline = li;
    }

  return false; // stop
  }


//
// P_SlideMove
// The momx / momy move is bad, so try to slide
// along a wall.
// Find the first line hit, move flush to it,
// and slide along it
//
// This is a kludgy mess.
//
// killough 11/98: reformatted

static void P_SlideMove(mobj_t __far* mo)
{
  int8_t hitcount = 3;

  slidemo = mo; // the object that's sliding

  do
    {
      fixed_t leadx, leady, trailx, traily;

      if (!--hitcount)
  goto stairstep;   // don't loop forever

      // trace along the three leading corners

      if (mo->momx > 0)
  leadx = mo->x + mo->radius, trailx = mo->x - mo->radius;
      else
  leadx = mo->x - mo->radius, trailx = mo->x + mo->radius;

      if (mo->momy > 0)
  leady = mo->y + mo->radius, traily = mo->y - mo->radius;
      else
  leady = mo->y - mo->radius, traily = mo->y + mo->radius;

      bestslidefrac = FRACUNIT+1;

      P_PathTraverse(leadx, leady, leadx+mo->momx, leady+mo->momy,
         PT_ADDLINES, PTR_SlideTraverse);
      P_PathTraverse(trailx, leady, trailx+mo->momx, leady+mo->momy,
         PT_ADDLINES, PTR_SlideTraverse);
      P_PathTraverse(leadx, traily, leadx+mo->momx, traily+mo->momy,
         PT_ADDLINES, PTR_SlideTraverse);

      // move up to the wall

      if (bestslidefrac == FRACUNIT+1)
  {
    // the move must have hit the middle, so stairstep

  stairstep:

    /* phares 5/4/98: kill momentum if you can't move at all
     * This eliminates player bobbing if pressed against a wall
     * while on ice.
     *
     * killough 10/98: keep buggy code around for old Boom demos
     *
     * cph 2000/09//23: buggy code was only in Boom v2.01
     */

    if (!P_TryMove(mo, mo->x, mo->y + mo->momy))
      P_TryMove(mo, mo->x + mo->momx, mo->y);


    break;
  }

      // fudge a bit to make sure it doesn't hit

      if ((bestslidefrac -= 0x800) > 0)
  {
    fixed_t newx = FixedMul(mo->momx, bestslidefrac);
    fixed_t newy = FixedMul(mo->momy, bestslidefrac);

    if (!P_TryMove(mo, mo->x+newx, mo->y+newy))
      goto stairstep;
  }

      // Now continue along the wall.
      // First calculate remainder.

      bestslidefrac = FRACUNIT-(bestslidefrac+0x800);

      if (bestslidefrac > FRACUNIT)
            bestslidefrac = FRACUNIT;

      if (bestslidefrac <= 0)
  break;

      tmxmove = FixedMul(mo->momx, bestslidefrac);
      tmymove = FixedMul(mo->momy, bestslidefrac);

      P_HitSlideLine(bestslideline); // clip the moves

      mo->momx = tmxmove;
      mo->momy = tmymove;

      /* killough 10/98: affect the bobbing the same way (but not voodoo dolls)
       * cph - DEMOSYNC? */
  if (P_MobjIsPlayer(mo) && P_MobjIsPlayer(mo)->mo == mo)
  {
    if (D_abs(P_MobjIsPlayer(mo)->momx) > D_abs(tmxmove))
      P_MobjIsPlayer(mo)->momx = tmxmove;
    if (D_abs(P_MobjIsPlayer(mo)->momy) > D_abs(tmymove))
      P_MobjIsPlayer(mo)->momy = tmymove;
  }
    }
  while (!P_TryMove(mo, mo->x+tmxmove, mo->y+tmymove));
}


static fixed_t FixedMul32OrigFriction(fixed_t a)
{
	uint16_t alw = a;
	 int16_t ahw = a >> FRACBITS;

	uint32_t ll = (uint32_t) alw * ORIG_FRICTION;
	 int32_t hl = ( int32_t) ahw * ORIG_FRICTION;
	return (ll >> FRACBITS) + hl;
}


//
// P_XYMovement
//
// Attempts to move something if it has momentum.
//

#define MAXMOVE         (30*FRACUNIT)
#define STOPSPEED       (FRACUNIT/16)

static void P_XYMovement(mobj_t __far* mo)
{
    player_t *player;
    fixed_t xmove, ymove;

    if (!(mo->momx | mo->momy)) // Any momentum?
    {
        return;
    }

    player = P_MobjIsPlayer(mo);

    if (mo->momx > MAXMOVE)
        mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE)
        mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
        mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE)
        mo->momy = -MAXMOVE;

    xmove = mo->momx;
    ymove = mo->momy;

    do
    {
        fixed_t ptryx, ptryy;
        // killough 8/9/98: fix bug in original Doom source:
        // Large negative displacements were never considered.
        // This explains the tendency for Mancubus fireballs
        // to pass through walls.
        // CPhipps - compatibility optioned

        if (xmove > MAXMOVE/2 || ymove > MAXMOVE/2 || ((xmove < -MAXMOVE/2 || ymove < -MAXMOVE/2)))
        {
            ptryx = mo->x + xmove/2;
            ptryy = mo->y + ymove/2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }

        if (!P_TryMove (mo, ptryx, ptryy))
        {
            // blocked move

            if (player)   // try to slide along it
                P_SlideMove (mo);
            else
            {
                if (mo->flags & MF_MISSILE)
                {
                    // explode a missile

                    if (_g_ceilingline)
                    {
                        const sector_t __far* ceilingBackSector = LN_BACKSECTOR(_g_ceilingline);

                        if(ceilingBackSector && ceilingBackSector->ceilingpic == skyflatnum)
                        {
                            if (mo->z > ceilingBackSector->ceilingheight)
                            {
                                // Hack to prevent missiles exploding
                                // against the sky.
                                // Does not handle sky floors.

                                P_RemoveMobj (mo);
                                return;
                            }
                        }
                    }

                    P_ExplodeMissile (mo);
                }
                else // whatever else it is, it is now standing still in (x,y)
                {
                    mo->momx = mo->momy = 0;
                }
            }
        }
    } while (xmove || ymove);

    // slow down

    /* no friction for missiles or skulls ever, no friction when airborne */
    if (mo->flags & MF_MISSILE || mo->z > mo->floorz)
        return;

    /* killough 8/11/98: add bouncers
   * killough 9/15/98: add objects falling off ledges
   * killough 11/98: only include bouncers hanging off ledges
   */
    if ((mo->flags & MF_CORPSE) &&
            (mo->momx > FRACUNIT/4 || mo->momx < -FRACUNIT/4 ||
             mo->momy > FRACUNIT/4 || mo->momy < -FRACUNIT/4) &&
            mo->floorz != mo->subsector->sector->floorheight)
        return;  // do not stop sliding if halfway off a step with some momentum

    // killough 11/98:
    // Stop voodoo dolls that have come to rest, despite any
    // moving corresponding player, except in old demos:

    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED &&
            mo->momy > -STOPSPEED && mo->momy < STOPSPEED &&
            (!player || !(player->cmd.forwardmove | player->cmd.sidemove) ||
             (player->mo != mo)))
    {
        // if in a walking frame, stop moving

        // killough 10/98:
        // Don't affect main player when voodoo dolls stop, except in old demos:

        if (player && (uint16_t)(player->mo->state - states - S_PLAY_RUN1) < 4
                && (player->mo == mo))
            P_SetMobjState(player->mo, S_PLAY);

        mo->momx = mo->momy = 0;

        /* killough 10/98: kill any bobbing momentum too (except in voodoo dolls)
       * cph - DEMOSYNC - needs compatibility check?
       */
        if (player && player->mo == mo)
            player->momx = player->momy = 0;
    }
    else
    {
        /* phares 3/17/98
       *
       * Friction will have been adjusted by friction thinkers for
       * icy or muddy floors. Otherwise it was never touched and
       * remained set at ORIG_FRICTION
       *
       * killough 8/28/98: removed inefficient thinker algorithm,
       * instead using touching_sectorlist in P_GetFriction() to
       * determine friction (and thus only when it is needed).
       *
       * killough 10/98: changed to work with new bobbing method.
       * Reducing player momentum is no longer needed to reduce
       * bobbing, so ice works much better now.
       *
       * cph - DEMOSYNC - need old code for Boom demos?
       */

        mo->momx = FixedMul32OrigFriction(mo->momx);
        mo->momy = FixedMul32OrigFriction(mo->momy);

        /* killough 10/98: Always decrease player bobbing by ORIG_FRICTION.
       * This prevents problems with bobbing on ice, where it was not being
       * reduced fast enough, leading to all sorts of kludges being developed.
       */


        if (player && player->mo == mo)     /* Not voodoo dolls */
        {
            player->momx = FixedMul32OrigFriction(player->momx);
            player->momy = FixedMul32OrigFriction(player->momy);
        }
    }
}


//
// P_ZMovement
//
// Attempt vertical movement.

#define GRAVITY         FRACUNIT

static void P_ZMovement(mobj_t __far* mo)
{

    // check for smooth step up

    if (P_MobjIsPlayer(mo) &&
            P_MobjIsPlayer(mo)->mo == mo &&  // killough 5/12/98: exclude voodoo dolls
            mo->z < mo->floorz)
    {
        P_MobjIsPlayer(mo)->viewheight -= mo->floorz-mo->z;
        P_MobjIsPlayer(mo)->deltaviewheight = (VIEWHEIGHT - P_MobjIsPlayer(mo)->viewheight)>>3;
    }

  // adjust altitude

  mo->z += mo->momz;

  // clip movement

  if (mo->z <= mo->floorz)
    {
    // hit the floor

    if (mo->momz < 0)
      {
    if (P_MobjIsPlayer(mo) && /* killough 5/12/98: exclude voodoo dolls */
        P_MobjIsPlayer(mo)->mo == mo && mo->momz < -GRAVITY*8)
      {
        // Squat down.
        // Decrease viewheight for a moment
        // after hitting the ground (hard),
        // and utter appropriate sound.

        P_MobjIsPlayer(mo)->deltaviewheight = mo->momz>>3;
        if (mo->health > 0) /* cph - prevent "oof" when dead */
    S_StartSound (mo, sfx_oof);
      }
  mo->momz = 0;
      }
    mo->z = mo->floorz;

    if ( (mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP) )
      {
      P_ExplodeMissile (mo);
      return;
      }
    }
  else // still above the floor                                     // phares
    if (!(mo->flags & MF_NOGRAVITY))
      {
  if (!mo->momz)
    mo->momz = -GRAVITY;
        mo->momz -= GRAVITY;
      }

  if (mo->z + mo->height > mo->ceilingz)
    {
    // hit the ceiling

    if (mo->momz > 0)
      mo->momz = 0;

    mo->z = mo->ceilingz - mo->height;

    if ( (mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP) )
      {
      P_ExplodeMissile (mo);
      return;
      }
    }
  }

//
// P_NightmareRespawn
//

static void P_NightmareRespawn(mobj_t __far* mobj)
{
    fixed_t      x;
    fixed_t      y;
    subsector_t __far* ss;
    mobj_t __far*      mo;

    /* haleyjd: stupid nightmare respawning bug fix
   *
   * 08/09/00: compatibility added, time to ramble :)
   * This fixes the notorious nightmare respawning bug that causes monsters
   * that didn't spawn at level startup to respawn at the point (0,0)
   * regardless of that point's nature. SMMU and Eternity need this for
   * script-spawned things like Halif Swordsmythe, as well.
   *
   * cph - copied from eternity, except comp_respawnfix becomes comp_respawn
   *   and the logic is reversed (i.e. like the rest of comp_ it *disables*
   *   the fix)
   */

    //ZLB: Everything respawns at its death point.
    //The spawnpoint is removed from the mobj.

    x = mobj->x;
    y = mobj->y;

    if(!x && !y)
    {
        return;
    }

    // something is occupying its position?

    if (!P_CheckPosition (mobj, x, y) )
        return; // no respwan

    // spawn a teleport fog at old spot
    // because of removal of the body?

    mo = P_SpawnMobj (mobj->x,
                      mobj->y,
                      mobj->subsector->sector->floorheight,
                      MT_TFOG);

    // initiate teleport sound

    S_StartSound (mo, sfx_telept);

    // spawn a teleport fog at the new spot

    ss = R_PointInSubsector (x,y);

    mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_TFOG);

    S_StartSound (mo, sfx_telept);

    // spawn the new monster
    // inherit attributes from deceased one

    mo = P_SpawnMobj(x, y, ONFLOORZ, mobj->type);
    mo->angle = mobj->angle;
    mo->reactiontime = 18;

    // remove the old monster,

    P_RemoveMobj (mobj);
}


void P_MobjThinker (mobj_t __far* mobj)
{
    // momentum movement
    if (mobj->momx | mobj->momy)
    {
        P_XYMovement(mobj);
        if (mobj->thinker.function != P_MobjThinker) // cph - Must've been removed
            return;       // killough - mobj was removed
    }

    if (mobj->z != mobj->floorz || mobj->momz)
    {
        P_ZMovement(mobj);
        if (mobj->thinker.function != P_MobjThinker) // cph - Must've been removed
            return;       // killough - mobj was removed
    }

    // cycle through states,
    // calling action functions at transitions

    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic

        if (!mobj->tics)
            if (!P_SetMobjState (mobj, mobj->state->nextstate) )
                return;     // freed itself
    }
    else
    {

        // check for nightmare respawn

        if (! (mobj->flags & MF_COUNTKILL) )
            return;

        if (!_g_respawnmonsters)
            return;

        mobj->movecount++;

        if (mobj->movecount < 12*35)
            return;

        if (_g_leveltime & 31)
            return;

        if (P_Random () > 4)
            return;

        P_NightmareRespawn (mobj);
    }

}


//Thinker function for stuff that doesn't need to do anything
//interesting.
//Just cycles through the states. Allows sprite animation to work.
static void P_MobjBrainlessThinker(mobj_t __far* mobj)
{
    // cycle through states,
    // calling action functions at transitions

    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic

        if (!mobj->tics)
            P_SetMobjState (mobj, mobj->state->nextstate);
    }
}



static think_t P_ThinkerFunctionForType(mobjtype_t type, mobj_t __far* mobj)
{
    //Full thinking ability.
    if(type < MT_MISC0)
        return P_MobjThinker;

    //Just state cycles.
    if(mobj->tics != -1)
        return P_MobjBrainlessThinker;

    //No thinking at all.
    return NULL;
}

//
// P_SpawnMobj
//

static mobj_t __far* P_NewMobj()
{
    mobj_t __far* mobj = NULL;

    for(int16_t i = _g_thingPoolSize-1; i >= 0; i--)
    {
        if(_g_thingPool[i].type == MT_NOTHING)
        {
            mobj = &_g_thingPool[i];
            _fmemset (mobj, 0, sizeof (*mobj));

            mobj->flags = MF_POOLED;
            return mobj;
        }
    }

    if(mobj == NULL)
    {
        mobj = Z_MallocLevel(sizeof(*mobj), NULL);
        _fmemset (mobj, 0, sizeof (*mobj));
    }

    return mobj;
}

mobj_t __far* P_SpawnMobj(fixed_t x,fixed_t y,fixed_t z,mobjtype_t type)
{
    const state_t*    st;
    const mobjinfo_t* info;

    mobj_t __far*     mobj = P_NewMobj();

    info = &mobjinfo[type];
    mobj->type = type;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;                                      // phares
    mobj->flags |= info->flags;
    mobj->health = info->spawnhealth;

    if (_g_gameskill != sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    P_Random(); // only to call random for compatibiltiy
    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet

    st = &states[info->spawnstate];

    mobj->state  = st;
    mobj->tics   = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame  = st->frame;
    mobj->touching_sectorlist = NULL; // NULL head of sector list // phares 3/13/98

    // set subsector and/or block links

    P_SetThingPosition (mobj);

    mobj->dropoffz =           /* killough 11/98: for tracking dropoffs */
            mobj->floorz   = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    mobj->z = z == ONFLOORZ ? mobj->floorz : z == ONCEILINGZ ?
                                  mobj->ceilingz - mobj->height : z;

    mobj->thinker.function = P_ThinkerFunctionForType(type, mobj);

    mobj->target = mobj->tracer = mobj->lastenemy = NULL;
    P_AddThinker (&mobj->thinker);
    if (!((mobj->flags ^ MF_COUNTKILL) & MF_COUNTKILL))
        _g_totallive++;
    return mobj;
}

//
// P_RemoveMobj
//

void P_RemoveMobj(mobj_t __far* mobj)
{
  P_UnsetThingPosition (mobj);

  // Delete all nodes on the current sector_list               phares 3/16/98
  P_DelSeclist();

  // stop any playing sound

  S_StopSound (mobj);

  // killough 11/98:
  //
  // Remove any references to other mobjs.
  //
  // Older demos might depend on the fields being left alone, however,
  // if multiple thinkers reference each other indirectly before the
  // end of the current tic.
  // CPhipps - only leave dead references in old demos; I hope lxdoom_1 level
  // demos are rare and don't rely on this. I hope.

  if (!_g_demoplayback)
  {
    mobj->target    = NULL;
    mobj->tracer    = NULL;
    mobj->lastenemy = NULL;
  }
  // free block

  P_RemoveThing (mobj);
}


/*
 * P_FindDoomedNum
 *
 * Finds a mobj type with a matching doomednum
 *
 */

static PUREFUNC int16_t P_FindDoomedNum(int16_t type)
{
    // find which type to spawn
    for (int16_t i = 0; i < NUMMOBJTYPES; i++)
    {
        if (type == mobjinfo[i].doomednum)
            return i;
    }

    I_Error("P_FindDoomedNum: unknown thing %i", type);
}


//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//

static void P_SpawnPlayer(int16_t playerx, int16_t playery, int8_t playerangle)
{
  player_t* p;
  mobj_t __far*   mobj;

  p = &_g_player;

  if (p->playerstate == PST_REBORN)
    G_PlayerReborn ();

  fixed_t x = ((int32_t)playerx) << FRACBITS;
  fixed_t y = ((int32_t)playery) << FRACBITS;
  fixed_t z = ONFLOORZ;
  mobj = P_SpawnMobj (x,y,z, MT_PLAYER);

  // set color translations for player sprites

  mobj->angle      = ANG45 * playerangle;
  mobj->health     = p->health;

  p->mo            = mobj;
  p->playerstate   = PST_LIVE;
  p->refire        = 0;
  p->message       = NULL;
  p->damagecount   = 0;
  p->bonuscount    = 0;
  p->extralight    = 0;
  p->fixedcolormap = 0;
  p->viewheight    = VIEWHEIGHT;

  p->momx = p->momy = 0;   // killough 10/98: initialize bobbing to 0.

  // setup gun psprite

  P_SetupPsprites (p);
}


//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
//

// These are Thing flags

// Skill flags.
#define MTF_EASY                1
#define MTF_NORMAL              2
#define MTF_HARD                4
// Deaf monsters/do not react to sound.
#define MTF_AMBUSH              8

void P_SpawnMapThing(const mapthing_t __far* mthing)
{
    int16_t     i;
    mobj_t __far* mobj;
    fixed_t x;
    fixed_t y;

    // check for players specially

    //Only care about start spot for player 1.
    if (mthing->type == 1)
    {
        P_SpawnPlayer(mthing->x, mthing->y, mthing->angle);
        return;
    }

    // check for apropriate skill level
    if (_g_gameskill == sk_baby || _g_gameskill == sk_easy ?
            !(mthing->options & MTF_EASY) :
            _g_gameskill == sk_hard || _g_gameskill == sk_nightmare ?
            !(mthing->options & MTF_HARD) : !(mthing->options & MTF_NORMAL))
        return;

    // find which type to spawn

    i = P_FindDoomedNum(mthing->type);

    x = ((int32_t)mthing->x) << FRACBITS;
    y = ((int32_t)mthing->y) << FRACBITS;

    mobj = P_SpawnMobj(x, y, ONFLOORZ, i);

    if (mobj->tics > 0)
        mobj->tics = 1 + (P_Random () % mobj->tics);

    if (!((mobj->flags ^ MF_COUNTKILL) & MF_COUNTKILL))
        _g_totalkills++;

    if (mobj->flags & MF_COUNTITEM)
        _g_totalitems++;

    mobj->angle = ANG45 * mthing->angle;
    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;
}


//
// GAME SPAWN FUNCTIONS
//

//
// P_SpawnPuff
//
void P_SpawnPuff(fixed_t x,fixed_t y,fixed_t z)
  {
  mobj_t __far* th;
  // killough 5/5/98: remove dependence on order of evaluation:
  int32_t t = P_Random();
  z += (t - P_Random())<<10;

  th = P_SpawnMobj (x,y,z, MT_PUFF);
  th->momz = FRACUNIT;
  th->tics -= P_Random()&3;

  if (th->tics < 1)
    th->tics = 1;

  // don't make punches spark on the wall

  if (P_IsAttackRangeMeleeRange())
    P_SetMobjState (th, S_PUFF3);
  }


//
// P_SpawnBlood
//
void P_SpawnBlood(fixed_t x,fixed_t y,fixed_t z,int16_t damage)
  {
  mobj_t __far* th;
  // killough 5/5/98: remove dependence on order of evaluation:
  fixed_t t = P_Random();
  z += (t - P_Random())<<10;
  th = P_SpawnMobj(x,y,z, MT_BLOOD);
  th->momz = FRACUNIT*2;
  th->tics -= P_Random()&3;

  if (th->tics < 1)
    th->tics = 1;

  if (9 <= damage && damage <= 12)
    P_SetMobjState (th,S_BLOOD2);
  else if (damage < 9)
    P_SetMobjState (th,S_BLOOD3);
  }


//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//

static void P_CheckMissileSpawn(mobj_t __far* th)
  {
  th->tics -= P_Random()&3;
  if (th->tics < 1)
    th->tics = 1;

  // move a little forward so an angle can
  // be computed if it immediately explodes

  th->x += (th->momx>>1);
  th->y += (th->momy>>1);
  th->z += (th->momz>>1);

  // killough 8/12/98: for non-missile objects (e.g. grenades)
  if (!(th->flags & MF_MISSILE))
    return;

  if (!P_TryMove (th, th->x, th->y))
    P_ExplodeMissile (th);
  }


//
// P_SpawnMissile
//

mobj_t __far* P_SpawnMissile(mobj_t __far* source, mobj_t __far* dest, mobjtype_t type)
  {
  mobj_t __far* th;
  angle_t an;
  fixed_t     dist;

  th = P_SpawnMobj (source->x,source->y,source->z + 4*8*FRACUNIT,type);

  if (mobjinfo[th->type].seesound)
    S_StartSound (th, mobjinfo[th->type].seesound);

  th->target = source;    // where it came from
  an = R_PointToAngle2 (source->x, source->y, dest->x, dest->y);

  // fuzzy player

  if (dest->flags & MF_SHADOW)
    {  // killough 5/5/98: remove dependence on order of evaluation:
    int32_t t = P_Random();
    an += (t - P_Random())<<20;
    }

  th->angle = an;
  an >>= ANGLETOFINESHIFT;
  th->momx = FixedMulAngle(mobjinfo[th->type].speed, finecosine(an));
  th->momy = FixedMulAngle(mobjinfo[th->type].speed, finesine(  an));

  dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);
  dist = dist / mobjinfo[th->type].speed;

  if (dist < 1)
    dist = 1;

  th->momz = (dest->z - source->z) / dist;
  P_CheckMissileSpawn (th);

  return th;
  }


//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//

void P_SpawnPlayerMissile(mobj_t __far* source)
{
	mobj_t __far* th;
	fixed_t x, y, z, slope = 0;

	// see which target is to be aimed at

	angle_t an = source->angle;

	slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT);
	if (!_g_linetarget)
		slope = P_AimLineAttack(source, an += 1L << 26, 16 * 64 * FRACUNIT);
	if (!_g_linetarget)
		slope = P_AimLineAttack(source, an -= 2l << 26, 16 * 64 * FRACUNIT);
	if (!_g_linetarget)
		an = source->angle, slope = 0;

	x = source->x;
	y = source->y;
	z = source->z + 4 * 8 * FRACUNIT;

	th = P_SpawnMobj(x,y,z, MT_ROCKET);

	if (mobjinfo[th->type].seesound)
		S_StartSound(th, mobjinfo[th->type].seesound);

	th->target = source;
	th->angle  = an;
	th->momx   = FixedMulAngle(mobjinfo[th->type].speed,finecosine(an >> ANGLETOFINESHIFT));
	th->momy   = FixedMulAngle(mobjinfo[th->type].speed,finesine(  an >> ANGLETOFINESHIFT));
	th->momz   = FixedMul(mobjinfo[th->type].speed,slope);

	P_CheckMissileSpawn(th);
}


struct player_s* P_MobjIsPlayer(const mobj_t __far* mobj)
{
    if(mobj == _g_player.mo)
    {
        return &_g_player;
    }

    return NULL;
}
