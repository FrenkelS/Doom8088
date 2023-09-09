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
 *  Movement, collision handling.
 *  Shooting and aiming.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "r_main.h"
#include "p_mobj.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_inter.h"
#include "m_random.h"
#include "m_bbox.h"
#include "i_system.h"

#include "globdata.h"




//
// TELEPORT MOVE
//

//
// PIT_StompThing
//


static boolean PIT_StompThing (mobj_t* thing)
  {
  fixed_t blockdist;

  // phares 9/10/98: moved this self-check to start of routine

  // don't clip against self

  if (thing == _g->tmthing)
    return true;

  if (!(thing->flags & MF_SHOOTABLE)) // Can't shoot it? Can't stomp it!
    return true;

  blockdist = thing->radius + _g->tmthing->radius;

  if (D_abs(thing->x - _g->tmx) >= blockdist || D_abs(thing->y - _g->tmy) >= blockdist)
    return true; // didn't hit it

  // monsters don't stomp things except on boss level
  if (!_g->telefrag)  // killough 8/9/98: make consistent across all levels
    return false;

  P_DamageMobj (thing, _g->tmthing, _g->tmthing, 10000); // Stomp!

  return true;
  }


//
// P_TeleportMove
//

boolean P_TeleportMove (mobj_t* thing,fixed_t x,fixed_t y, boolean boss)
  {
  int32_t     xl;
  int32_t     xh;
  int32_t     yl;
  int32_t     yh;
  int32_t     bx;
  int32_t     by;

  subsector_t*  newsubsec;

  /* killough 8/9/98: make telefragging more consistent, preserve compatibility */
  _g->telefrag = P_MobjIsPlayer(thing) || boss;

  // kill anything occupying the position

  _g->tmthing = thing;

  _g->tmx = x;
  _g->tmy = y;

  _g->tmbbox[BOXTOP] = y + _g->tmthing->radius;
  _g->tmbbox[BOXBOTTOM] = y - _g->tmthing->radius;
  _g->tmbbox[BOXRIGHT] = x + _g->tmthing->radius;
  _g->tmbbox[BOXLEFT] = x - _g->tmthing->radius;

  newsubsec = R_PointInSubsector (x,y);
  _g->ceilingline = NULL;

  // The base floor/ceiling is from the subsector
  // that contains the point.
  // Any contacted lines the step closer together
  // will adjust them.

  _g->tmfloorz = _g->tmdropoffz = newsubsec->sector->floorheight;
  _g->tmceilingz = newsubsec->sector->ceilingheight;

  _g->validcount++;
  _g->numspechit = 0;

  // stomp on any things contacted

  xl = (_g->tmbbox[BOXLEFT] - _g->bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
  xh = (_g->tmbbox[BOXRIGHT] - _g->bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
  yl = (_g->tmbbox[BOXBOTTOM] - _g->bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
  yh = (_g->tmbbox[BOXTOP] - _g->bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockThingsIterator(bx,by,PIT_StompThing))
        return false;

  // the move is ok,
  // so unlink from the old position & link into the new position

  P_UnsetThingPosition (thing);

  thing->floorz = _g->tmfloorz;
  thing->ceilingz = _g->tmceilingz;
  thing->dropoffz = _g->tmdropoffz;        // killough 11/98

  thing->x = x;
  thing->y = y;

  P_SetThingPosition (thing);

  //thing->PrevX = x;
  //thing->PrevY = y;
  //thing->PrevZ = thing->floorz;

  return true;
  }


//
// MOVEMENT ITERATOR FUNCTIONS
//


/* killough 8/1/98: used to test intersection between thing and line
 * assuming NO movement occurs -- used to avoid sticky situations.
 */

static boolean untouched(const line_t *ld)
{
  fixed_t x, y, tmbbox[4];
  return
    (tmbbox[BOXRIGHT] = (x=_g->tmthing->x)+_g->tmthing->radius) <= ld->bbox[BOXLEFT] ||
    (tmbbox[BOXLEFT] = x-_g->tmthing->radius) >= ld->bbox[BOXRIGHT] ||
    (tmbbox[BOXTOP] = (y=_g->tmthing->y)+_g->tmthing->radius) <= ld->bbox[BOXBOTTOM] ||
    (tmbbox[BOXBOTTOM] = y-_g->tmthing->radius) >= ld->bbox[BOXTOP] ||
    P_BoxOnLineSide(tmbbox, ld) != -1;
}

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//

static // killough 3/26/98: make static
boolean PIT_CheckLine (const line_t* ld)
{
  if (_g->tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
   || _g->tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
   || _g->tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
   || _g->tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
    return true; // didn't hit it

  if (P_BoxOnLineSide(_g->tmbbox, ld) != -1)
    return true; // didn't hit it

  // A line has been hit

  // The moving thing's destination position will cross the given line.
  // If this should not be allowed, return false.
  // If the line is special, keep track of it
  // to process later if the move is proven ok.
  // NOTE: specials are NOT sorted by order,
  // so two special lines that are only 8 pixels apart
  // could be crossed in either order.

  // killough 7/24/98: allow player to move out of 1s wall, to prevent sticking
  if (!LN_BACKSECTOR(ld)) // one sided line
    {
      _g->blockline = ld;
      return _g->tmunstuck && !untouched(ld) &&
  FixedMul(_g->tmx-_g->tmthing->x,ld->dy) > FixedMul(_g->tmy-_g->tmthing->y,ld->dx);
    }

  // killough 8/10/98: allow bouncing objects to pass through as missiles
  if (!(_g->tmthing->flags & (MF_MISSILE)))
    {
      if (ld->flags & ML_BLOCKING)           // explicitly blocking everything
  return _g->tmunstuck && !untouched(ld);  // killough 8/1/98: allow escape

      // killough 8/9/98: monster-blockers don't affect friends
      if (!(_g->tmthing->flags & MF_FRIEND || P_MobjIsPlayer(_g->tmthing))
    && ld->flags & ML_BLOCKMONSTERS)
  return false; // block monsters only
    }

  // set openrange, opentop, openbottom
  // these define a 'window' from one sector to another across this line

  P_LineOpening (ld);

  // adjust floor & ceiling heights

  if (_g->opentop < _g->tmceilingz)
    {
      _g->tmceilingz = _g->opentop;
      _g->ceilingline = ld;
      _g->blockline = ld;
    }

  if (_g->openbottom > _g->tmfloorz)
    {
      _g->tmfloorz = _g->openbottom;
      _g->floorline = ld;          // killough 8/1/98: remember floor linedef
      _g->blockline = ld;
    }

  if (_g->lowfloor < _g->tmdropoffz)
    _g->tmdropoffz = _g->lowfloor;

  // if contacted a special line, add it to the list

  if (LN_SPECIAL(ld))
  {
      // 1/11/98 killough: remove limit on lines hit, by array doubling
      if (_g->numspechit < 4)
      {
        _g->spechit[_g->numspechit++] = ld;
      }
  }

  return true;
}

//
// PIT_CheckThing
//

static boolean PIT_CheckThing(mobj_t *thing) // killough 3/26/98: make static
{
  fixed_t blockdist;
  int32_t damage;

  // killough 11/98: add touchy things
  if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE)))
    return true;

  blockdist = thing->radius + _g->tmthing->radius;

  if (D_abs(thing->x - _g->tmx) >= blockdist || D_abs(thing->y - _g->tmy) >= blockdist)
    return true; // didn't hit it

  // killough 11/98:
  //
  // This test has less information content (it's almost always false), so it
  // should not be moved up to first, as it adds more overhead than it removes.

  // don't clip against self

  if (thing == _g->tmthing)
    return true;

  // check for skulls slamming into things

  if (_g->tmthing->flags & MF_SKULLFLY)
    {
      // A flying skull is smacking something.
      // Determine damage amount, and the skull comes to a dead stop.

      int32_t damage = ((P_Random()%8)+1)*mobjinfo[_g->tmthing->type].damage;

      P_DamageMobj (thing, _g->tmthing, _g->tmthing, damage);

      _g->tmthing->flags &= ~MF_SKULLFLY;
      _g->tmthing->momx = _g->tmthing->momy = _g->tmthing->momz = 0;

      P_SetMobjState (_g->tmthing, mobjinfo[_g->tmthing->type].spawnstate);

      return false;   // stop moving
    }

  // missiles can hit other things
  // killough 8/10/98: bouncing non-solid things can hit other things too

  if (_g->tmthing->flags & MF_MISSILE)
    {
      // see if it went over / under

      if (_g->tmthing->z > thing->z + thing->height)
  return true;    // overhead

      if (_g->tmthing->z+_g->tmthing->height < thing->z)
  return true;    // underneath

      if (_g->tmthing->target && (_g->tmthing->target->type == thing->type))
      {
  if (thing == _g->tmthing->target)
    return true;                // Don't hit same species as originator.
  else
    if (thing->type != MT_PLAYER) // Explode, but do no damage.
        return false;         // Let players missile other players.
      }

      // killough 8/10/98: if moving thing is not a missile, no damage
      // is inflicted, and momentum is reduced if object hit is solid.

      if (!(_g->tmthing->flags & MF_MISSILE)) {
  if (!(thing->flags & MF_SOLID)) {
      return true;
  } else {
      _g->tmthing->momx = -_g->tmthing->momx;
      _g->tmthing->momy = -_g->tmthing->momy;
      if (!(_g->tmthing->flags & MF_NOGRAVITY))
        {
    _g->tmthing->momx >>= 2;
    _g->tmthing->momy >>= 2;
        }
      return false;
  }
      }

      if (!(thing->flags & MF_SHOOTABLE))
  return !(thing->flags & MF_SOLID); // didn't do any damage

      // damage / explode

      damage = ((P_Random()%8)+1)*mobjinfo[_g->tmthing->type].damage;
      P_DamageMobj (thing, _g->tmthing, _g->tmthing->target, damage);

      // don't traverse any more
      return false;
    }

  // check for special pickup

  if (thing->flags & MF_SPECIAL)
    {
      uint32_t solid = thing->flags & MF_SOLID;
      if (_g->tmthing->flags & MF_PICKUP)
  P_TouchSpecialThing(thing, _g->tmthing); // can remove thing
      return !solid;
    }

  return !(thing->flags & MF_SOLID);
}


//
// MOVEMENT CLIPPING
//

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
//
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  floorz
//  ceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//

boolean P_CheckPosition (mobj_t* thing,fixed_t x,fixed_t y)
  {
  int32_t     xl;
  int32_t     xh;
  int32_t     yl;
  int32_t     yh;
  int32_t     bx;
  int32_t     by;
  subsector_t*  newsubsec;

  _g->tmthing = thing;

  _g->tmx = x;
  _g->tmy = y;

  _g->tmbbox[BOXTOP] = y + _g->tmthing->radius;
  _g->tmbbox[BOXBOTTOM] = y - _g->tmthing->radius;
  _g->tmbbox[BOXRIGHT] = x + _g->tmthing->radius;
  _g->tmbbox[BOXLEFT] = x - _g->tmthing->radius;

  newsubsec = R_PointInSubsector (x,y);
  _g->floorline = _g->blockline = _g->ceilingline = NULL; // killough 8/1/98

  // Whether object can get out of a sticky situation:
  _g->tmunstuck = P_MobjIsPlayer(thing) &&          /* only players */
    P_MobjIsPlayer(thing)->mo == thing; /* not under old demos */

  // The base floor / ceiling is from the subsector
  // that contains the point.
  // Any contacted lines the step closer together
  // will adjust them.

  _g->tmfloorz = _g->tmdropoffz = newsubsec->sector->floorheight;
  _g->tmceilingz = newsubsec->sector->ceilingheight;
  _g->validcount++;
  _g->numspechit = 0;

  if ( _g->tmthing->flags & MF_NOCLIP )
    return true;

  // Check things first, possibly picking things up.
  // The bounding box is extended by MAXRADIUS
  // because mobj_ts are grouped into mapblocks
  // based on their origin point, and can overlap
  // into adjacent blocks by up to MAXRADIUS units.

  xl = (_g->tmbbox[BOXLEFT] - _g->bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
  xh = (_g->tmbbox[BOXRIGHT] - _g->bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
  yl = (_g->tmbbox[BOXBOTTOM] - _g->bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
  yh = (_g->tmbbox[BOXTOP] - _g->bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;


  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockThingsIterator(bx,by,PIT_CheckThing))
        return false;

  // check lines

  xl = (_g->tmbbox[BOXLEFT] - _g->bmaporgx)>>MAPBLOCKSHIFT;
  xh = (_g->tmbbox[BOXRIGHT] - _g->bmaporgx)>>MAPBLOCKSHIFT;
  yl = (_g->tmbbox[BOXBOTTOM] - _g->bmaporgy)>>MAPBLOCKSHIFT;
  yh = (_g->tmbbox[BOXTOP] - _g->bmaporgy)>>MAPBLOCKSHIFT;

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockLinesIterator (bx,by,PIT_CheckLine))
        return false; // doesn't fit

  return true;
  }


//
// P_TryMove
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
//
boolean P_TryMove(mobj_t* thing,fixed_t x,fixed_t y)
{
    fixed_t oldx;
    fixed_t oldy;

    _g->felldown = _g->floatok = false;               // killough 11/98

    if (!P_CheckPosition (thing, x, y))
        return false;   // solid wall or thing

    if ( !(thing->flags & MF_NOCLIP) )
    {
        if (_g->tmceilingz - _g->tmfloorz < thing->height)
            return false;	// doesn't fit

        _g->floatok = true;

        if ( !(thing->flags & MF_TELEPORT)
             && _g->tmceilingz - thing->z < thing->height)
            return false;	// mobj must lower itself to fit

        if ( !(thing->flags & MF_TELEPORT)
             && _g->tmfloorz - thing->z > 24*FRACUNIT )
            return false;	// too big a step up

        if ( !(thing->flags & (MF_DROPOFF|MF_FLOAT))
             && _g->tmfloorz - _g->tmdropoffz > 24*FRACUNIT )
            return false;	// don't stand over a dropoff
    }

    // the move is ok,
    // so unlink from the old position and link into the new position

    P_UnsetThingPosition (thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = _g->tmfloorz;
    thing->ceilingz = _g->tmceilingz;
    thing->dropoffz = _g->tmdropoffz;      // killough 11/98: keep track of dropoffs
    thing->x = x;
    thing->y = y;

    P_SetThingPosition (thing);

    // if any special lines were hit, do the effect

    if (! (thing->flags&(MF_TELEPORT|MF_NOCLIP)) )
        while (_g->numspechit--)
            if (LN_SPECIAL(_g->spechit[_g->numspechit]))  // see if the line was crossed
            {
                int32_t oldside;
                if ((oldside = P_PointOnLineSide(oldx, oldy, _g->spechit[_g->numspechit])) !=
                        P_PointOnLineSide(thing->x, thing->y, _g->spechit[_g->numspechit]))
                    P_CrossSpecialLine(_g->spechit[_g->numspechit], oldside, thing);
            }

    return true;
}


//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//



//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
// If the floor is icy, then you can bounce off a wall.             // phares
//

static void P_HitSlideLine (const line_t* ld)
{
    int32_t     side;
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

    if (ld->slopetype == ST_HORIZONTAL)
    {
        _g->tmymove = 0; // no more movement in the Y direction
        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {                                                          // phares
        _g->tmxmove = 0; // no more movement in the X direction
        return;
    }

    // The wall is angled. Bounce if the angle of approach is         // phares
    // less than 45 degrees.                                          // phares

    side = P_PointOnLineSide (_g->slidemo->x, _g->slidemo->y, ld);

    lineangle = R_PointToAngle2 (0,0, ld->dx, ld->dy);
    if (side == 1)
        lineangle += ANG180;

    moveangle = R_PointToAngle2 (0,0, _g->tmxmove, _g->tmymove);

    // killough 3/2/98:
    // The moveangle+=10 breaks v1.9 demo compatibility in
    // some demos, so it needs demo_compatibility switch.

    moveangle += 10; // prevents sudden path reversal due to        // phares
    // rounding error                              //   |
    deltaangle = moveangle-lineangle;                                 //   V
    movelen = P_AproxDistance (_g->tmxmove, _g->tmymove);
    //   |
    // phares
    if (deltaangle > ANG180)
        deltaangle += ANG180;

    //  I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;
    newlen = FixedMul (movelen, finecosine(deltaangle));
    _g->tmxmove = FixedMul (newlen, finecosine(lineangle));
    _g->tmymove = FixedMul (newlen, finesine(  lineangle));
    // phares
}


//
// PTR_SlideTraverse
//

static boolean PTR_SlideTraverse (intercept_t* in)
  {
  const line_t* li;

  if (!in->isaline)
    I_Error ("PTR_SlideTraverse: not a line?");

  li = in->d.line;

  if ( ! (li->flags & ML_TWOSIDED) )
    {
    if (P_PointOnLineSide (_g->slidemo->x, _g->slidemo->y, li))
      return true; // don't hit the back side
    goto isblocking;
    }

  // set openrange, opentop, openbottom.
  // These define a 'window' from one sector to another across a line

  P_LineOpening (li);

  if (_g->openrange < _g->slidemo->height)
    goto isblocking;  // doesn't fit

  if (_g->opentop - _g->slidemo->z < _g->slidemo->height)
    goto isblocking;  // mobj is too high

  if (_g->openbottom - _g->slidemo->z > 24*FRACUNIT )
    goto isblocking;  // too big a step up

  // this line doesn't block movement

  return true;

  // the line does block movement,
  // see if it is closer than best so far

isblocking:

  if (in->frac < _g->bestslidefrac)
    {
    _g->bestslidefrac = in->frac;
    _g->bestslideline = li;
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

void P_SlideMove(mobj_t *mo)
{
  int8_t hitcount = 3;

  _g->slidemo = mo; // the object that's sliding

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

      _g->bestslidefrac = FRACUNIT+1;

      P_PathTraverse(leadx, leady, leadx+mo->momx, leady+mo->momy,
         PT_ADDLINES, PTR_SlideTraverse);
      P_PathTraverse(trailx, leady, trailx+mo->momx, leady+mo->momy,
         PT_ADDLINES, PTR_SlideTraverse);
      P_PathTraverse(leadx, traily, leadx+mo->momx, traily+mo->momy,
         PT_ADDLINES, PTR_SlideTraverse);

      // move up to the wall

      if (_g->bestslidefrac == FRACUNIT+1)
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

      if ((_g->bestslidefrac -= 0x800) > 0)
  {
    fixed_t newx = FixedMul(mo->momx, _g->bestslidefrac);
    fixed_t newy = FixedMul(mo->momy, _g->bestslidefrac);

    if (!P_TryMove(mo, mo->x+newx, mo->y+newy))
      goto stairstep;
  }

      // Now continue along the wall.
      // First calculate remainder.

      _g->bestslidefrac = FRACUNIT-(_g->bestslidefrac+0x800);

      if (_g->bestslidefrac > FRACUNIT)
            _g->bestslidefrac = FRACUNIT;

      if (_g->bestslidefrac <= 0)
  break;

      _g->tmxmove = FixedMul(mo->momx, _g->bestslidefrac);
      _g->tmymove = FixedMul(mo->momy, _g->bestslidefrac);

      P_HitSlideLine(_g->bestslideline); // clip the moves

      mo->momx = _g->tmxmove;
      mo->momy = _g->tmymove;

      /* killough 10/98: affect the bobbing the same way (but not voodoo dolls)
       * cph - DEMOSYNC? */
  if (P_MobjIsPlayer(mo) && P_MobjIsPlayer(mo)->mo == mo)
  {
    if (D_abs(P_MobjIsPlayer(mo)->momx) > D_abs(_g->tmxmove))
      P_MobjIsPlayer(mo)->momx = _g->tmxmove;
    if (D_abs(P_MobjIsPlayer(mo)->momy) > D_abs(_g->tmymove))
      P_MobjIsPlayer(mo)->momy = _g->tmymove;
  }
    }
  while (!P_TryMove(mo, mo->x+_g->tmxmove, mo->y+_g->tmymove));
}

//
// P_LineAttack
//



//
// PTR_AimTraverse
// Sets linetaget and aimslope when a target is aimed at.
//
static boolean PTR_AimTraverse (intercept_t* in)
{
    const line_t* li;
    mobj_t* th;
    fixed_t slope;
    fixed_t thingtopslope;
    fixed_t thingbottomslope;
    fixed_t dist;

    if (in->isaline)
    {
        li = in->d.line;

        if ( !(li->flags & ML_TWOSIDED) )
            return false;   // stop

        // Crosses a two sided line.
        // A two sided line will restrict
        // the possible target ranges.

        P_LineOpening (li);

        if (_g->openbottom >= _g->opentop)
            return false;   // stop

        dist = FixedMul(_g->attackrange, in->frac);

        if (LN_FRONTSECTOR(li)->floorheight != LN_BACKSECTOR(li)->floorheight)
        {
            slope = FixedDiv (_g->openbottom - _g->shootz , dist);

            if (slope > _g->bottomslope)
                _g->bottomslope = slope;
        }

        if (LN_FRONTSECTOR(li)->ceilingheight != LN_BACKSECTOR(li)->ceilingheight)
        {
            slope = FixedDiv (_g->opentop - _g->shootz , dist);
            if (slope < _g->topslope)
                _g->topslope = slope;
        }

        if (_g->topslope <= _g->bottomslope)
            return false;   // stop

        return true;    // shot continues
    }

    // shoot a thing

    th = in->d.thing;
    if (th == _g->shootthing)
        return true;    // can't shoot self

    if (!(th->flags&MF_SHOOTABLE))
        return true;    // corpse or something

    /* killough 7/19/98, 8/2/98:
   * friends don't aim at friends (except players), at least not first
   */
    if (th->flags & _g->shootthing->flags & _g->aim_flags_mask && !P_MobjIsPlayer(th))
        return true;

    // check angles to see if the thing can be aimed at

    dist = FixedMul (_g->attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - _g->shootz , dist);

    if (thingtopslope < _g->bottomslope)
        return true;    // shot over the thing

    thingbottomslope = FixedDiv (th->z - _g->shootz, dist);

    if (thingbottomslope > _g->topslope)
        return true;    // shot under the thing

    // this thing can be hit!

    if (thingtopslope > _g->topslope)
        thingtopslope = _g->topslope;

    if (thingbottomslope < _g->bottomslope)
        thingbottomslope = _g->bottomslope;

    _g->aimslope = (thingtopslope+thingbottomslope)/2;
    _g->linetarget = th;

    return false;   // don't go any farther
}


//
// PTR_ShootTraverse
//
static boolean PTR_ShootTraverse (intercept_t* in)
  {
  fixed_t x;
  fixed_t y;
  fixed_t z;
  fixed_t frac;

  mobj_t* th;

  fixed_t slope;
  fixed_t dist;
  fixed_t thingtopslope;
  fixed_t thingbottomslope;

  if (in->isaline)
    {
    const line_t *li = in->d.line;

    if (LN_SPECIAL(li))
      P_ShootSpecialLine (_g->shootthing, li);

      if (li->flags & ML_TWOSIDED)
  {  // crosses a two sided (really 2s) line
    P_LineOpening (li);
    dist = FixedMul(_g->attackrange, in->frac);

    // killough 11/98: simplify

    if ((LN_FRONTSECTOR(li)->floorheight==LN_BACKSECTOR(li)->floorheight ||
         (slope = FixedDiv(_g->openbottom - _g->shootz , dist)) <= _g->aimslope) &&
        (LN_FRONTSECTOR(li)->ceilingheight==LN_BACKSECTOR(li)->ceilingheight ||
         (slope = FixedDiv (_g->opentop - _g->shootz , dist)) >= _g->aimslope))
      return true;      // shot continues
  }

    // hit line
    // position a bit closer

    frac = in->frac - FixedDiv (4*FRACUNIT,_g->attackrange);
    x = _g->trace.x + FixedMul (_g->trace.dx, frac);
    y = _g->trace.y + FixedMul (_g->trace.dy, frac);
    z = _g->shootz + FixedMul (_g->aimslope, FixedMul(frac, _g->attackrange));

    if (LN_FRONTSECTOR(li)->ceilingpic == _g->skyflatnum)
      {
      // don't shoot the sky!

      if (z > LN_FRONTSECTOR(li)->ceilingheight)
        return false;

      // it's a sky hack wall

      if  (LN_BACKSECTOR(li) && LN_BACKSECTOR(li)->ceilingpic == _g->skyflatnum)

        // fix bullet-eaters -- killough:
        // WARNING: Almost all demos will lose sync without this
        // demo_compatibility flag check!!! killough 1/18/98
      if (LN_BACKSECTOR(li)->ceilingheight < z)
        return false;
      }

    // Spawn bullet puffs.

    P_SpawnPuff (x,y,z);

    // don't go any farther

    return false;
    }

  // shoot a thing

  th = in->d.thing;
  if (th == _g->shootthing)
    return true;  // can't shoot self

  if (!(th->flags&MF_SHOOTABLE))
    return true;  // corpse or something

  // check angles to see if the thing can be aimed at

  dist = FixedMul (_g->attackrange, in->frac);
  thingtopslope = FixedDiv (th->z+th->height - _g->shootz , dist);

  if (thingtopslope < _g->aimslope)
    return true;  // shot over the thing

  thingbottomslope = FixedDiv (th->z - _g->shootz, dist);

  if (thingbottomslope > _g->aimslope)
    return true;  // shot under the thing

  // hit thing
  // position a bit closer

  frac = in->frac - FixedDiv (10*FRACUNIT,_g->attackrange);

  x = _g->trace.x + FixedMul (_g->trace.dx, frac);
  y = _g->trace.y + FixedMul (_g->trace.dy, frac);
  z = _g->shootz + FixedMul (_g->aimslope, FixedMul(frac, _g->attackrange));

  // Spawn bullet puffs or blod spots,
  // depending on target type.
  if (in->d.thing->flags & MF_NOBLOOD)
    P_SpawnPuff (x,y,z);
  else
    P_SpawnBlood (x,y,z, _g->la_damage);

  if (_g->la_damage)
    P_DamageMobj (th, _g->shootthing, _g->shootthing, _g->la_damage);

  // don't go any farther
  return false;
  }


//
// P_AimLineAttack
//
fixed_t P_AimLineAttack(mobj_t* t1,angle_t angle,fixed_t distance, uint64_t mask)
  {
  fixed_t x2;
  fixed_t y2;

  angle >>= ANGLETOFINESHIFT;
  _g->shootthing = t1;

  x2 = t1->x + (distance>>FRACBITS)*finecosine(angle);
  y2 = t1->y + (distance>>FRACBITS)*finesine(  angle);
  _g->shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

  // can't shoot outside view angles

  _g->topslope = 100*FRACUNIT/160;
  _g->bottomslope = -100*FRACUNIT/160;

  _g->attackrange = distance;
  _g->linetarget = NULL;

  /* killough 8/2/98: prevent friends from aiming at friends */
  _g->aim_flags_mask = mask;

  P_PathTraverse(t1->x,t1->y,x2,y2,PT_ADDLINES|PT_ADDTHINGS,PTR_AimTraverse);

  if (_g->linetarget)
    return _g->aimslope;

  return 0;
  }


//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
//

void P_LineAttack
(mobj_t* t1,
 angle_t angle,
 fixed_t distance,
 fixed_t slope,
 int32_t     damage)
  {
  fixed_t x2;
  fixed_t y2;

  angle >>= ANGLETOFINESHIFT;
  _g->shootthing = t1;
  _g->la_damage = damage;
  x2 = t1->x + (distance>>FRACBITS)*finecosine(angle);
  y2 = t1->y + (distance>>FRACBITS)*finesine(  angle);
  _g->shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
  _g->attackrange = distance;
  _g->aimslope = slope;

  P_PathTraverse(t1->x,t1->y,x2,y2,PT_ADDLINES|PT_ADDTHINGS,PTR_ShootTraverse);
  }


//
// USE LINES
//


static boolean PTR_UseTraverse (intercept_t* in)
  {
  int32_t side;



  if (!LN_SPECIAL(in->d.line))
    {
    P_LineOpening (in->d.line);
    if (_g->openrange <= 0)
      {
      S_StartSound (_g->usething, sfx_noway);

      // can't use through a wall
      return false;
      }

    // not a special line, but keep checking

    return true;
    }

  side = 0;
  if (P_PointOnLineSide (_g->usething->x, _g->usething->y, in->d.line) == 1)
    side = 1;

  //  return false;   // don't use back side

  P_UseSpecialLine (_g->usething, in->d.line, side);

  //WAS can't use for than one special line in a row
  //jff 3/21/98 NOW multiple use allowed with enabling line flag

  return ((in->d.line->flags&ML_PASSUSE))?
          true : false;
}

// Returns false if a "oof" sound should be made because of a blocking
// linedef. Makes 2s middles which are impassable, as well as 2s uppers
// and lowers which block the player, cause the sound effect when the
// player tries to activate them. Specials are excluded, although it is
// assumed that all special linedefs within reach have been considered
// and rejected already (see P_UseLines).
//
// by Lee Killough
//

static boolean PTR_NoWayTraverse(intercept_t* in)
  {
  const line_t *ld = in->d.line;
                                           // This linedef
  return LN_SPECIAL(ld) || !(                 // Ignore specials
   ld->flags & ML_BLOCKING || (            // Always blocking
   P_LineOpening(ld),                      // Find openings
   _g->openrange <= 0 ||                       // No opening
   _g->openbottom > _g->usething->z+24*FRACUNIT || // Too high it blocks
   _g->opentop < _g->usething->z+_g->usething->height  // Too low it blocks
  )
  );
  }

//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines (player_t*  player)
  {
  int32_t     angle;
  fixed_t x1;
  fixed_t y1;
  fixed_t x2;
  fixed_t y2;

  _g->usething = player->mo;

  angle = player->mo->angle >> ANGLETOFINESHIFT;

  x1 = player->mo->x;
  y1 = player->mo->y;
  x2 = x1 + (USERANGE>>FRACBITS)*finecosine(angle);
  y2 = y1 + (USERANGE>>FRACBITS)*finesine(  angle);

  // old code:
  //
  // P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse );
  //
  // This added test makes the "oof" sound work on 2s lines -- killough:

  if (P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse ))
    if (!P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_NoWayTraverse ))
      S_StartSound (_g->usething, sfx_noway);
  }


//
// RADIUS ATTACK
//




//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//

static boolean PIT_RadiusAttack (mobj_t* thing)
  {
  fixed_t dx;
  fixed_t dy;
  fixed_t dist;

  /* killough 8/20/98: allow bouncers to take damage
   * (missile bouncers are already excluded with MF_NOBLOCKMAP)
   */

  if (!(thing->flags & MF_SHOOTABLE))
    return true;

  dx = D_abs(thing->x - _g->bombspot->x);
  dy = D_abs(thing->y - _g->bombspot->y);

  dist = dx>dy ? dx : dy;
  dist = (dist - thing->radius) >> FRACBITS;

  if (dist < 0)
  dist = 0;

  if (dist >= _g->bombdamage)
    return true;  // out of range

  if ( P_CheckSight (thing, _g->bombspot) )
    {
    // must be in direct path
    P_DamageMobj (thing, _g->bombspot, _g->bombsource, _g->bombdamage - dist);
    }

  return true;
  }


//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void P_RadiusAttack(mobj_t* spot,mobj_t* source,int32_t damage)
  {
  int32_t x;
  int32_t y;

  int32_t xl;
  int32_t xh;
  int32_t yl;
  int32_t yh;

  fixed_t dist;

  dist = (damage+MAXRADIUS)<<FRACBITS;
  yh = (spot->y + dist - _g->bmaporgy)>>MAPBLOCKSHIFT;
  yl = (spot->y - dist - _g->bmaporgy)>>MAPBLOCKSHIFT;
  xh = (spot->x + dist - _g->bmaporgx)>>MAPBLOCKSHIFT;
  xl = (spot->x - dist - _g->bmaporgx)>>MAPBLOCKSHIFT;
  _g->bombspot = spot;
  _g->bombsource = source;
  _g->bombdamage = damage;

  for (y=yl ; y<=yh ; y++)
    for (x=xl ; x<=xh ; x++)
      P_BlockThingsIterator (x, y, PIT_RadiusAttack );
  }


// CPhipps -
// Use block memory allocator here

#include "z_bmallo.h"

static struct block_memory_alloc_s secnodezone = { NULL, sizeof(msecnode_t), 32 };

void P_SetSecnodeFirstpoolToNull(void)
{
	secnodezone.firstpool = NULL;
}


inline static msecnode_t* P_GetSecnode(void)
{
  return (msecnode_t*)Z_BMalloc(&secnodezone);
}

// P_PutSecnode() returns a node to the freelist.

inline static void P_PutSecnode(msecnode_t* node)
{
  Z_BFree(&secnodezone, node);
}

// phares 3/16/98
//
// P_AddSecnode() searches the current list to see if this sector is
// already there. If not, it adds a sector node at the head of the list of
// sectors this object appears in. This is called when creating a list of
// nodes that will get linked in later.

static void P_AddSecnode(sector_t* s, mobj_t* thing)
  {
  msecnode_t* node;

  node = _g->sector_list;
  while (node)
    {
    if (node->m_sector == s)   // Already have a node for this sector?
      {
      node->m_thing = thing; // Yes. Setting m_thing says 'keep it'.
      return;
      }
    node = node->m_tnext;
    }

  // Couldn't find an existing node for this sector. Add one at the head
  // of the list.

  node = P_GetSecnode();

  // killough 4/4/98, 4/7/98: mark new nodes unvisited.
  node->visited = false;

  node->m_sector = s;       // sector
  node->m_thing  = thing;     // mobj
  node->m_tprev  = NULL;    // prev node on Thing thread
  node->m_tnext  = _g->sector_list;  // next node on Thing thread
  if (_g->sector_list)
    _g->sector_list->m_tprev = node; // set back link on Thing

  // Add new node at head of sector thread starting at s->touching_thinglist

  node->m_sprev  = NULL;    // prev node on sector thread
  node->m_snext  = s->touching_thinglist; // next node on sector thread
  if (s->touching_thinglist)
    node->m_snext->m_sprev = node;
  s->touching_thinglist = node;
  _g->sector_list = node;
  }


// P_DelSecnode() deletes a sector node from the list of
// sectors this object appears in. Returns a pointer to the next node
// on the linked list, or NULL.

static msecnode_t* P_DelSecnode(msecnode_t* node)
  {
  msecnode_t* tp;  // prev node on thing thread
  msecnode_t* tn;  // next node on thing thread
  msecnode_t* sp;  // prev node on sector thread
  msecnode_t* sn;  // next node on sector thread

  if (node)
    {

    // Unlink from the Thing thread. The Thing thread begins at
    // sector_list and not from mobj_t->touching_sectorlist.

    tp = node->m_tprev;
    tn = node->m_tnext;
    if (tp)
      tp->m_tnext = tn;
    if (tn)
      tn->m_tprev = tp;

    // Unlink from the sector thread. This thread begins at
    // sector_t->touching_thinglist.

    sp = node->m_sprev;
    sn = node->m_snext;
    if (sp)
      sp->m_snext = sn;
    else
      node->m_sector->touching_thinglist = sn;
    if (sn)
      sn->m_sprev = sp;

    // Return this node to the freelist

    P_PutSecnode(node);
    return(tn);
    }
  return(NULL);
  }                             // phares 3/13/98

// Delete an entire sector list

void P_DelSeclist(void)
{
	if (_g->sector_list)
	{
		msecnode_t* node = _g->sector_list;
		while (node)
			node = P_DelSecnode(node);

		_g->sector_list = NULL;
	}
}


// phares 3/14/98
//
// PIT_GetSectors
// Locates all the sectors the object is in by looking at the lines that
// cross through it. You have already decided that the object is allowed
// at this location, so don't bother with checking impassable or
// blocking lines.

static boolean PIT_GetSectors(const line_t* ld)
  {
  if (_g->tmbbox[BOXRIGHT]  <= ld->bbox[BOXLEFT]   ||
      _g->tmbbox[BOXLEFT]   >= ld->bbox[BOXRIGHT]  ||
      _g->tmbbox[BOXTOP]    <= ld->bbox[BOXBOTTOM] ||
      _g->tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
    return true;

  if (P_BoxOnLineSide(_g->tmbbox, ld) != -1)
    return true;

  // This line crosses through the object.

  // Collect the sector(s) from the line and add to the
  // sector_list you're examining. If the Thing ends up being
  // allowed to move to this position, then the sector_list
  // will be attached to the Thing's mobj_t at touching_sectorlist.

  P_AddSecnode(LN_FRONTSECTOR(ld),_g->tmthing);

  /* Don't assume all lines are 2-sided, since some Things
   * like MT_TFOG are allowed regardless of whether their radius takes
   * them beyond an impassable linedef.
   *
   * killough 3/27/98, 4/4/98:
   * Use sidedefs instead of 2s flag to determine two-sidedness.
   * killough 8/1/98: avoid duplicate if same sector on both sides
   * cph - DEMOSYNC? */

  if (LN_BACKSECTOR(ld) && LN_BACKSECTOR(ld) != LN_FRONTSECTOR(ld))
    P_AddSecnode(LN_BACKSECTOR(ld), _g->tmthing);

  return true;
  }


// phares 3/14/98
//
// P_CreateSecNodeList alters/creates the sector_list that shows what sectors
// the object resides in.

void P_CreateSecNodeList(mobj_t* thing)
{
  mobj_t* saved_tmthing = _g->tmthing; /* cph - see comment at func end */

  // First, clear out the existing m_thing fields. As each node is
  // added or verified as needed, m_thing will be set properly. When
  // finished, delete all nodes where m_thing is still NULL. These
  // represent the sectors the Thing has vacated.

  msecnode_t* node = _g->sector_list;
  while (node)
    {
    node->m_thing = NULL;
    node = node->m_tnext;
    }

  _g->tmthing = thing;

  _g->tmx = thing->x;
  _g->tmy = thing->y;

  _g->tmbbox[BOXTOP]    = thing->y + _g->tmthing->radius;
  _g->tmbbox[BOXBOTTOM] = thing->y - _g->tmthing->radius;
  _g->tmbbox[BOXRIGHT]  = thing->x + _g->tmthing->radius;
  _g->tmbbox[BOXLEFT]   = thing->x - _g->tmthing->radius;

  _g->validcount++; // used to make sure we only process a line once

  int32_t xl = (_g->tmbbox[BOXLEFT]   - _g->bmaporgx) >> MAPBLOCKSHIFT;
  int32_t xh = (_g->tmbbox[BOXRIGHT]  - _g->bmaporgx) >> MAPBLOCKSHIFT;
  int32_t yl = (_g->tmbbox[BOXBOTTOM] - _g->bmaporgy) >> MAPBLOCKSHIFT;
  int32_t yh = (_g->tmbbox[BOXTOP]    - _g->bmaporgy) >> MAPBLOCKSHIFT;

  for (int32_t bx = xl; bx <= xh; bx++)
    for (int32_t by = yl; by <= yh; by++)
      P_BlockLinesIterator(bx,by,PIT_GetSectors);

  // Add the sector of the (x,y) point to sector_list.

  P_AddSecnode(thing->subsector->sector,thing);

  // Now delete any nodes that won't be used. These are the ones where
  // m_thing is still NULL.

  node = _g->sector_list;
  while (node)
    {
    if (node->m_thing == NULL)
      {
      if (node == _g->sector_list)
        _g->sector_list = node->m_tnext;
      node = P_DelSecnode(node);
      }
    else
      node = node->m_tnext;
    }

  /* cph -
   * This is the strife we get into for using global variables. tmthing
   *  is being used by several different functions calling
   *  P_BlockThingIterator, including functions that can be called *from*
   *  P_BlockThingIterator. Using a global tmthing is not reentrant.
   * OTOH for Boom/MBF demos we have to preserve the buggy behavior.
   *  Fun. We restore its previous value unless we're in a Boom/MBF demo.
   */
  _g->tmthing = saved_tmthing;
}

/* cphipps 2004/08/30 - 
 * Must clear tmthing at tic end, as it might contain a pointer to a removed thinker, or the level might have ended/been ended and we clear the objects it was pointing too. Hopefully we don't need to carry this between tics for sync. */
void P_MapStart(void)
{
    if (_g->tmthing) I_Error("P_MapStart: tmthing set!");
}

void P_MapEnd(void)
{
    _g->tmthing = NULL;
}
