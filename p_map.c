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

#include "d_player.h"
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
#include "i_system.h"

#include "globdata.h"


static mobj_t    *tmthing;
static fixed_t   tmx;
static fixed_t   tmy;


// The tm* items are used to hold information globally, usually for
// line or object intersection checking

fixed_t   _g_tmbbox[4];  // bounding box for line intersection checks
fixed_t   _g_tmfloorz;   // floor you'd hit if free to fall
fixed_t   _g_tmceilingz; // ceiling of sector you're in
fixed_t   _g_tmdropoffz; // dropoff on other side of line you're crossing

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls

const line_t    *_g_ceilingline;
const line_t        *_g_blockline;    /* killough 8/11/98: blocking linedef */
static int32_t         tmunstuck;     /* killough 8/1/98: whether to allow unsticking */

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

// 1/11/98 killough: removed limit on special lines crossed
const line_t *_g_spechit[4];

int32_t _g_numspechit;

// Temporary holder for thing_sectorlist threads
msecnode_t* _g_sector_list;

static fixed_t   bestslidefrac;
static const line_t*   bestslideline;
static mobj_t*   slidemo;
static fixed_t   tmxmove;
static fixed_t   tmymove;

mobj_t*   _g_linetarget; // who got hit (or NULL)
static mobj_t*   shootthing;

// Height if not aiming up or down
static fixed_t   shootz;

static int32_t       la_damage;
fixed_t   _g_attackrange;

// slopes to top and bottom of target

static fixed_t  topslope;
static fixed_t  bottomslope;

static mobj_t *bombsource, *bombspot;
static int32_t bombdamage;

static mobj_t*   usething;

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean   _g_floatok;

/* killough 11/98: if "felldown" true, object was pushed down ledge */
boolean   _g_felldown;

boolean _g_crushchange, _g_nofit;

static boolean telefrag;   /* killough 8/9/98: whether to telefrag at exit */


// MAXRADIUS is for precalculated sector block boxes
#define MAXRADIUS       (32*FRACUNIT)


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

  if (thing == tmthing)
    return true;

  if (!(thing->flags & MF_SHOOTABLE)) // Can't shoot it? Can't stomp it!
    return true;

  blockdist = thing->radius + tmthing->radius;

  if (D_abs(thing->x - tmx) >= blockdist || D_abs(thing->y - tmy) >= blockdist)
    return true; // didn't hit it

  // monsters don't stomp things except on boss level
  if (!telefrag)  // killough 8/9/98: make consistent across all levels
    return false;

  P_DamageMobj (thing, tmthing, tmthing, 10000); // Stomp!

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
  telefrag = P_MobjIsPlayer(thing) || boss;

  // kill anything occupying the position

  tmthing = thing;

  tmx = x;
  tmy = y;

  _g_tmbbox[BOXTOP]    = y + tmthing->radius;
  _g_tmbbox[BOXBOTTOM] = y - tmthing->radius;
  _g_tmbbox[BOXRIGHT]  = x + tmthing->radius;
  _g_tmbbox[BOXLEFT]   = x - tmthing->radius;

  newsubsec = R_PointInSubsector (x,y);
  _g_ceilingline = NULL;

  // The base floor/ceiling is from the subsector
  // that contains the point.
  // Any contacted lines the step closer together
  // will adjust them.

  _g_tmfloorz = _g_tmdropoffz = newsubsec->sector->floorheight;
  _g_tmceilingz = newsubsec->sector->ceilingheight;

  validcount++;
  _g_numspechit = 0;

  // stomp on any things contacted

  xl = (_g_tmbbox[BOXLEFT] - _g_bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
  xh = (_g_tmbbox[BOXRIGHT] - _g_bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
  yl = (_g_tmbbox[BOXBOTTOM] - _g_bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
  yh = (_g_tmbbox[BOXTOP] - _g_bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockThingsIterator(bx,by,PIT_StompThing))
        return false;

  // the move is ok,
  // so unlink from the old position & link into the new position

  P_UnsetThingPosition (thing);

  thing->floorz = _g_tmfloorz;
  thing->ceilingz = _g_tmceilingz;
  thing->dropoffz = _g_tmdropoffz;        // killough 11/98

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
    (tmbbox[BOXRIGHT] = (x=tmthing->x)+tmthing->radius) <= ld->bbox[BOXLEFT] ||
    (tmbbox[BOXLEFT] = x-tmthing->radius) >= ld->bbox[BOXRIGHT] ||
    (tmbbox[BOXTOP] = (y=tmthing->y)+tmthing->radius) <= ld->bbox[BOXBOTTOM] ||
    (tmbbox[BOXBOTTOM] = y-tmthing->radius) >= ld->bbox[BOXTOP] ||
    P_BoxOnLineSide(tmbbox, ld) != -1;
}

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//

static boolean PIT_CheckLine (const line_t* ld)
{
  if (_g_tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
   || _g_tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
   || _g_tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
   || _g_tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
    return true; // didn't hit it

  if (P_BoxOnLineSide(_g_tmbbox, ld) != -1)
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
      _g_blockline = ld;
      return tmunstuck && !untouched(ld) &&
  FixedMul(tmx-tmthing->x,ld->dy) > FixedMul(tmy-tmthing->y,ld->dx);
    }

  // killough 8/10/98: allow bouncing objects to pass through as missiles
  if (!(tmthing->flags & (MF_MISSILE)))
    {
      if (ld->flags & ML_BLOCKING)           // explicitly blocking everything
  return tmunstuck && !untouched(ld);  // killough 8/1/98: allow escape

      // killough 8/9/98: monster-blockers don't affect friends
      if (!(tmthing->flags & MF_FRIEND || P_MobjIsPlayer(tmthing))
    && ld->flags & ML_BLOCKMONSTERS)
  return false; // block monsters only
    }

  // set openrange, opentop, openbottom
  // these define a 'window' from one sector to another across this line

  P_LineOpening (ld);

  // adjust floor & ceiling heights

  if (_g_opentop < _g_tmceilingz)
    {
      _g_tmceilingz = _g_opentop;
      _g_ceilingline = ld;
      _g_blockline = ld;
    }

  if (_g_openbottom > _g_tmfloorz)
    {
      _g_tmfloorz = _g_openbottom;
      _g_blockline = ld;
    }

  if (_g_lowfloor < _g_tmdropoffz)
    _g_tmdropoffz = _g_lowfloor;

  // if contacted a special line, add it to the list

  if (LN_SPECIAL(ld))
  {
      // 1/11/98 killough: remove limit on lines hit, by array doubling
      if (_g_numspechit < 4)
      {
        _g_spechit[_g_numspechit++] = ld;
      }
  }

  return true;
}

//
// PIT_CheckThing
//

static boolean PIT_CheckThing(mobj_t *thing)
{
  fixed_t blockdist;
  int32_t damage;

  // killough 11/98: add touchy things
  if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE)))
    return true;

  blockdist = thing->radius + tmthing->radius;

  if (D_abs(thing->x - tmx) >= blockdist || D_abs(thing->y - tmy) >= blockdist)
    return true; // didn't hit it

  // killough 11/98:
  //
  // This test has less information content (it's almost always false), so it
  // should not be moved up to first, as it adds more overhead than it removes.

  // don't clip against self

  if (thing == tmthing)
    return true;

  // check for skulls slamming into things

  if (tmthing->flags & MF_SKULLFLY)
    {
      // A flying skull is smacking something.
      // Determine damage amount, and the skull comes to a dead stop.

      int32_t damage = ((P_Random()%8)+1)*mobjinfo[tmthing->type].damage;

      P_DamageMobj (thing, tmthing, tmthing, damage);

      tmthing->flags &= ~MF_SKULLFLY;
      tmthing->momx = tmthing->momy = tmthing->momz = 0;

      P_SetMobjState (tmthing, mobjinfo[tmthing->type].spawnstate);

      return false;   // stop moving
    }

  // missiles can hit other things
  // killough 8/10/98: bouncing non-solid things can hit other things too

  if (tmthing->flags & MF_MISSILE)
    {
      // see if it went over / under

      if (tmthing->z > thing->z + thing->height)
  return true;    // overhead

      if (tmthing->z+tmthing->height < thing->z)
  return true;    // underneath

      if (tmthing->target && (tmthing->target->type == thing->type))
      {
  if (thing == tmthing->target)
    return true;                // Don't hit same species as originator.
  else
    if (thing->type != MT_PLAYER) // Explode, but do no damage.
        return false;         // Let players missile other players.
      }

      // killough 8/10/98: if moving thing is not a missile, no damage
      // is inflicted, and momentum is reduced if object hit is solid.

      if (!(tmthing->flags & MF_MISSILE)) {
  if (!(thing->flags & MF_SOLID)) {
      return true;
  } else {
      tmthing->momx = -tmthing->momx;
      tmthing->momy = -tmthing->momy;
      if (!(tmthing->flags & MF_NOGRAVITY))
        {
    tmthing->momx >>= 2;
    tmthing->momy >>= 2;
        }
      return false;
  }
      }

      if (!(thing->flags & MF_SHOOTABLE))
  return !(thing->flags & MF_SOLID); // didn't do any damage

      // damage / explode

      damage = ((P_Random()%8)+1)*mobjinfo[tmthing->type].damage;
      P_DamageMobj (thing, tmthing, tmthing->target, damage);

      // don't traverse any more
      return false;
    }

  // check for special pickup

  if (thing->flags & MF_SPECIAL)
    {
      uint32_t solid = thing->flags & MF_SOLID;
      if (tmthing->flags & MF_PICKUP)
  P_TouchSpecialThing(thing, tmthing); // can remove thing
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

  tmthing = thing;

  tmx = x;
  tmy = y;

  _g_tmbbox[BOXTOP] = y + tmthing->radius;
  _g_tmbbox[BOXBOTTOM] = y - tmthing->radius;
  _g_tmbbox[BOXRIGHT] = x + tmthing->radius;
  _g_tmbbox[BOXLEFT] = x - tmthing->radius;

  newsubsec = R_PointInSubsector (x,y);
  _g_blockline = _g_ceilingline = NULL; // killough 8/1/98

  // Whether object can get out of a sticky situation:
  tmunstuck = P_MobjIsPlayer(thing) &&          /* only players */
    P_MobjIsPlayer(thing)->mo == thing; /* not under old demos */

  // The base floor / ceiling is from the subsector
  // that contains the point.
  // Any contacted lines the step closer together
  // will adjust them.

  _g_tmfloorz = _g_tmdropoffz = newsubsec->sector->floorheight;
  _g_tmceilingz = newsubsec->sector->ceilingheight;
  validcount++;
  _g_numspechit = 0;

  if ( tmthing->flags & MF_NOCLIP )
    return true;

  // Check things first, possibly picking things up.
  // The bounding box is extended by MAXRADIUS
  // because mobj_ts are grouped into mapblocks
  // based on their origin point, and can overlap
  // into adjacent blocks by up to MAXRADIUS units.

  xl = (_g_tmbbox[BOXLEFT] - _g_bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
  xh = (_g_tmbbox[BOXRIGHT] - _g_bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
  yl = (_g_tmbbox[BOXBOTTOM] - _g_bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
  yh = (_g_tmbbox[BOXTOP] - _g_bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;


  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockThingsIterator(bx,by,PIT_CheckThing))
        return false;

  // check lines

  xl = (_g_tmbbox[BOXLEFT] - _g_bmaporgx)>>MAPBLOCKSHIFT;
  xh = (_g_tmbbox[BOXRIGHT] - _g_bmaporgx)>>MAPBLOCKSHIFT;
  yl = (_g_tmbbox[BOXBOTTOM] - _g_bmaporgy)>>MAPBLOCKSHIFT;
  yh = (_g_tmbbox[BOXTOP] - _g_bmaporgy)>>MAPBLOCKSHIFT;

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

    _g_felldown = _g_floatok = false;               // killough 11/98

    if (!P_CheckPosition (thing, x, y))
        return false;   // solid wall or thing

    if ( !(thing->flags & MF_NOCLIP) )
    {
        if (_g_tmceilingz - _g_tmfloorz < thing->height)
            return false;	// doesn't fit

        _g_floatok = true;

        if ( !(thing->flags & MF_TELEPORT)
             && _g_tmceilingz - thing->z < thing->height)
            return false;	// mobj must lower itself to fit

        if ( !(thing->flags & MF_TELEPORT)
             && _g_tmfloorz - thing->z > 24*FRACUNIT )
            return false;	// too big a step up

        if ( !(thing->flags & (MF_DROPOFF|MF_FLOAT))
             && _g_tmfloorz - _g_tmdropoffz > 24*FRACUNIT )
            return false;	// don't stand over a dropoff
    }

    // the move is ok,
    // so unlink from the old position and link into the new position

    P_UnsetThingPosition (thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = _g_tmfloorz;
    thing->ceilingz = _g_tmceilingz;
    thing->dropoffz = _g_tmdropoffz;      // killough 11/98: keep track of dropoffs
    thing->x = x;
    thing->y = y;

    P_SetThingPosition (thing);

    // if any special lines were hit, do the effect

    if (! (thing->flags&(MF_TELEPORT|MF_NOCLIP)) )
        while (_g_numspechit--)
            if (LN_SPECIAL(_g_spechit[_g_numspechit]))  // see if the line was crossed
            {
                int32_t oldside;
                if ((oldside = P_PointOnLineSide(oldx, oldy, _g_spechit[_g_numspechit])) !=
                        P_PointOnLineSide(thing->x, thing->y, _g_spechit[_g_numspechit]))
                    P_CrossSpecialLine(_g_spechit[_g_numspechit], oldside, thing);
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
        tmymove = 0; // no more movement in the Y direction
        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {                                                          // phares
        tmxmove = 0; // no more movement in the X direction
        return;
    }

    // The wall is angled. Bounce if the angle of approach is         // phares
    // less than 45 degrees.                                          // phares

    side = P_PointOnLineSide (slidemo->x, slidemo->y, ld);

    lineangle = R_PointToAngle2 (0,0, ld->dx, ld->dy);
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
    newlen = FixedMul (movelen, finecosine(deltaangle));
    tmxmove = FixedMul (newlen, finecosine(lineangle));
    tmymove = FixedMul (newlen, finesine(  lineangle));
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

void P_SlideMove(mobj_t *mo)
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


// for more intelligent autoaiming
static uint32_t aim_flags_mask;

static fixed_t  aimslope;


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

        if (_g_openbottom >= _g_opentop)
            return false;   // stop

        dist = FixedMul(_g_attackrange, in->frac);

        if (LN_FRONTSECTOR(li)->floorheight != LN_BACKSECTOR(li)->floorheight)
        {
            slope = FixedDiv (_g_openbottom - shootz , dist);

            if (slope > bottomslope)
                bottomslope = slope;
        }

        if (LN_FRONTSECTOR(li)->ceilingheight != LN_BACKSECTOR(li)->ceilingheight)
        {
            slope = FixedDiv (_g_opentop - shootz , dist);
            if (slope < topslope)
                topslope = slope;
        }

        if (topslope <= bottomslope)
            return false;   // stop

        return true;    // shot continues
    }

    // shoot a thing

    th = in->d.thing;
    if (th == shootthing)
        return true;    // can't shoot self

    if (!(th->flags&MF_SHOOTABLE))
        return true;    // corpse or something

    /* killough 7/19/98, 8/2/98:
   * friends don't aim at friends (except players), at least not first
   */
    if (th->flags & shootthing->flags & aim_flags_mask && !P_MobjIsPlayer(th))
        return true;

    // check angles to see if the thing can be aimed at

    dist = FixedMul (_g_attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    if (thingtopslope < bottomslope)
        return true;    // shot over the thing

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > topslope)
        return true;    // shot under the thing

    // this thing can be hit!

    if (thingtopslope > topslope)
        thingtopslope = topslope;

    if (thingbottomslope < bottomslope)
        thingbottomslope = bottomslope;

    aimslope = (thingtopslope + thingbottomslope) / 2;
    _g_linetarget = th;

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
      P_ShootSpecialLine (shootthing, li);

      if (li->flags & ML_TWOSIDED)
  {  // crosses a two sided (really 2s) line
    P_LineOpening (li);
    dist = FixedMul(_g_attackrange, in->frac);

    // killough 11/98: simplify

    if ((LN_FRONTSECTOR(li)->floorheight==LN_BACKSECTOR(li)->floorheight ||
         (slope = FixedDiv(_g_openbottom - shootz , dist)) <= aimslope) &&
        (LN_FRONTSECTOR(li)->ceilingheight==LN_BACKSECTOR(li)->ceilingheight ||
         (slope = FixedDiv (_g_opentop - shootz , dist)) >= aimslope))
      return true;      // shot continues
  }

    // hit line
    // position a bit closer

    frac = in->frac - FixedDiv(4 * FRACUNIT, _g_attackrange);
    x = _g_trace.x + FixedMul(_g_trace.dx, frac);
    y = _g_trace.y + FixedMul(_g_trace.dy, frac);
    z = shootz  + FixedMul(aimslope, FixedMul(frac, _g_attackrange));

    if (LN_FRONTSECTOR(li)->ceilingpic == skyflatnum)
      {
      // don't shoot the sky!

      if (z > LN_FRONTSECTOR(li)->ceilingheight)
        return false;

      // it's a sky hack wall

      if  (LN_BACKSECTOR(li) && LN_BACKSECTOR(li)->ceilingpic == skyflatnum)

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
  if (th == shootthing)
    return true;  // can't shoot self

  if (!(th->flags&MF_SHOOTABLE))
    return true;  // corpse or something

  // check angles to see if the thing can be aimed at

  dist = FixedMul (_g_attackrange, in->frac);
  thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

  if (thingtopslope < aimslope)
    return true;  // shot over the thing

  thingbottomslope = FixedDiv (th->z - shootz, dist);

  if (thingbottomslope > aimslope)
    return true;  // shot under the thing

  // hit thing
  // position a bit closer

  frac = in->frac - FixedDiv (10*FRACUNIT,_g_attackrange);

  x = _g_trace.x + FixedMul(_g_trace.dx, frac);
  y = _g_trace.y + FixedMul(_g_trace.dy, frac);
  z = shootz  + FixedMul(aimslope, FixedMul(frac, _g_attackrange));

  // Spawn bullet puffs or blod spots,
  // depending on target type.
  if (in->d.thing->flags & MF_NOBLOOD)
    P_SpawnPuff (x,y,z);
  else
    P_SpawnBlood (x,y,z, la_damage);

  if (la_damage)
    P_DamageMobj (th, shootthing, shootthing, la_damage);

  // don't go any farther
  return false;
  }


//
// P_AimLineAttack
//
fixed_t P_AimLineAttack(mobj_t* t1,angle_t angle,fixed_t distance, boolean friend)
  {
  fixed_t x2;
  fixed_t y2;

  angle >>= ANGLETOFINESHIFT;
  shootthing = t1;

  x2 = t1->x + (distance>>FRACBITS)*finecosine(angle);
  y2 = t1->y + (distance>>FRACBITS)*finesine(  angle);
  shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

  // can't shoot outside view angles

  topslope    =  100 * FRACUNIT / 160;
  bottomslope = -100 * FRACUNIT / 160;

  _g_attackrange = distance;
  _g_linetarget  = NULL;

  // prevent friends from aiming at friends
  aim_flags_mask = friend ? MF_FRIEND : 0;

  P_PathTraverse(t1->x,t1->y,x2,y2,PT_ADDLINES|PT_ADDTHINGS,PTR_AimTraverse);

  if (_g_linetarget)
    return aimslope;

  return 0;
  }


//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
//

void P_LineAttack(mobj_t* t1, angle_t angle, fixed_t distance, fixed_t slope, int32_t damage)
{
  fixed_t x2;
  fixed_t y2;

  angle >>= ANGLETOFINESHIFT;
  shootthing = t1;
  la_damage = damage;
  x2 = t1->x + (distance>>FRACBITS)*finecosine(angle);
  y2 = t1->y + (distance>>FRACBITS)*finesine(  angle);
  shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
  _g_attackrange = distance;
  aimslope = slope;

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
    if (_g_openrange <= 0)
      {
      S_StartSound (usething, sfx_noway);

      // can't use through a wall
      return false;
      }

    // not a special line, but keep checking

    return true;
    }

  side = 0;
  if (P_PointOnLineSide (usething->x, usething->y, in->d.line) == 1)
    side = 1;

  //  return false;   // don't use back side

  P_UseSpecialLine (usething, in->d.line, side);

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
   _g_openrange <= 0 ||                       // No opening
   _g_openbottom > usething->z+24*FRACUNIT || // Too high it blocks
   _g_opentop < usething->z+usething->height  // Too low it blocks
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

  usething = player->mo;

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
      S_StartSound (usething, sfx_noway);
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

  dx = D_abs(thing->x - bombspot->x);
  dy = D_abs(thing->y - bombspot->y);

  dist = dx>dy ? dx : dy;
  dist = (dist - thing->radius) >> FRACBITS;

  if (dist < 0)
  dist = 0;

  if (dist >= bombdamage)
    return true;  // out of range

  if ( P_CheckSight (thing, bombspot) )
    {
    // must be in direct path
    P_DamageMobj (thing, bombspot, bombsource, bombdamage - dist);
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
  yh = (spot->y + dist - _g_bmaporgy)>>MAPBLOCKSHIFT;
  yl = (spot->y - dist - _g_bmaporgy)>>MAPBLOCKSHIFT;
  xh = (spot->x + dist - _g_bmaporgx)>>MAPBLOCKSHIFT;
  xl = (spot->x - dist - _g_bmaporgx)>>MAPBLOCKSHIFT;
  bombspot = spot;
  bombsource = source;
  bombdamage = damage;

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

  node = _g_sector_list;
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
  node->m_tnext  = _g_sector_list;  // next node on Thing thread
  if (_g_sector_list)
    _g_sector_list->m_tprev = node; // set back link on Thing

  // Add new node at head of sector thread starting at s->touching_thinglist

  node->m_sprev  = NULL;    // prev node on sector thread
  node->m_snext  = s->touching_thinglist; // next node on sector thread
  if (s->touching_thinglist)
    node->m_snext->m_sprev = node;
  s->touching_thinglist = node;
  _g_sector_list = node;
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
	if (_g_sector_list)
	{
		msecnode_t* node = _g_sector_list;
		while (node)
			node = P_DelSecnode(node);

		_g_sector_list = NULL;
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
  if (_g_tmbbox[BOXRIGHT]  <= ld->bbox[BOXLEFT]   ||
      _g_tmbbox[BOXLEFT]   >= ld->bbox[BOXRIGHT]  ||
      _g_tmbbox[BOXTOP]    <= ld->bbox[BOXBOTTOM] ||
      _g_tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
    return true;

  if (P_BoxOnLineSide(_g_tmbbox, ld) != -1)
    return true;

  // This line crosses through the object.

  // Collect the sector(s) from the line and add to the
  // sector_list you're examining. If the Thing ends up being
  // allowed to move to this position, then the sector_list
  // will be attached to the Thing's mobj_t at touching_sectorlist.

  P_AddSecnode(LN_FRONTSECTOR(ld),tmthing);

  /* Don't assume all lines are 2-sided, since some Things
   * like MT_TFOG are allowed regardless of whether their radius takes
   * them beyond an impassable linedef.
   *
   * killough 3/27/98, 4/4/98:
   * Use sidedefs instead of 2s flag to determine two-sidedness.
   * killough 8/1/98: avoid duplicate if same sector on both sides
   * cph - DEMOSYNC? */

  if (LN_BACKSECTOR(ld) && LN_BACKSECTOR(ld) != LN_FRONTSECTOR(ld))
    P_AddSecnode(LN_BACKSECTOR(ld), tmthing);

  return true;
  }


// phares 3/14/98
//
// P_CreateSecNodeList alters/creates the sector_list that shows what sectors
// the object resides in.

void P_CreateSecNodeList(mobj_t* thing)
{
  mobj_t* saved_tmthing = tmthing; /* cph - see comment at func end */

  // First, clear out the existing m_thing fields. As each node is
  // added or verified as needed, m_thing will be set properly. When
  // finished, delete all nodes where m_thing is still NULL. These
  // represent the sectors the Thing has vacated.

  msecnode_t* node = _g_sector_list;
  while (node)
    {
    node->m_thing = NULL;
    node = node->m_tnext;
    }

  tmthing = thing;

  tmx = thing->x;
  tmy = thing->y;

  _g_tmbbox[BOXTOP]    = thing->y + tmthing->radius;
  _g_tmbbox[BOXBOTTOM] = thing->y - tmthing->radius;
  _g_tmbbox[BOXRIGHT]  = thing->x + tmthing->radius;
  _g_tmbbox[BOXLEFT]   = thing->x - tmthing->radius;

  validcount++; // used to make sure we only process a line once

  int32_t xl = (_g_tmbbox[BOXLEFT]   - _g_bmaporgx) >> MAPBLOCKSHIFT;
  int32_t xh = (_g_tmbbox[BOXRIGHT]  - _g_bmaporgx) >> MAPBLOCKSHIFT;
  int32_t yl = (_g_tmbbox[BOXBOTTOM] - _g_bmaporgy) >> MAPBLOCKSHIFT;
  int32_t yh = (_g_tmbbox[BOXTOP]    - _g_bmaporgy) >> MAPBLOCKSHIFT;

  for (int32_t bx = xl; bx <= xh; bx++)
    for (int32_t by = yl; by <= yh; by++)
      P_BlockLinesIterator(bx,by,PIT_GetSectors);

  // Add the sector of the (x,y) point to sector_list.

  P_AddSecnode(thing->subsector->sector,thing);

  // Now delete any nodes that won't be used. These are the ones where
  // m_thing is still NULL.

  node = _g_sector_list;
  while (node)
    {
    if (node->m_thing == NULL)
      {
      if (node == _g_sector_list)
        _g_sector_list = node->m_tnext;
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
  tmthing = saved_tmthing;
}

/* cphipps 2004/08/30 - 
 * Must clear tmthing at tic end, as it might contain a pointer to a removed thinker, or the level might have ended/been ended and we clear the objects it was pointing too. Hopefully we don't need to carry this between tics for sync. */
void P_MapStart(void)
{
    if (tmthing) I_Error("P_MapStart: tmthing set!");
}

void P_MapEnd(void)
{
    tmthing = NULL;
}
