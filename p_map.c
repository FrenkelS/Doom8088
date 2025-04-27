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
#include "p_user.h"
#include "m_random.h"
#include "i_system.h"

#include "globdata.h"


static mobj_t    __far* tmthing;
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

const line_t    __far* _g_ceilingline;


// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

// 1/11/98 killough: removed limit on special lines crossed
line_t __far* _g_spechit[4];

int16_t _g_numspechit;

// Temporary holder for thing_sectorlist threads
static msecnode_t __far* _s_sector_list;


mobj_t __far*   _g_linetarget; // who got hit (or NULL)
static mobj_t __far*   shootthing;

// Height if not aiming up or down
static fixed_t   shootz;

static int16_t       la_damage;
static fixed_t   attackrange;

// slopes to top and bottom of target

static fixed_t  topslope;
static fixed_t  bottomslope;

static mobj_t __far* bombsource;
static mobj_t __far* bombspot;
static int16_t bombdamage;

static mobj_t __far*   usething;


static boolean telefrag;   /* killough 8/9/98: whether to telefrag at exit */


// MAXRADIUS is for precalculated sector block boxes
#define MAXRADIUS       (32*FRACUNIT)


boolean P_IsAttackRangeMeleeRange(void)
{
	return attackrange == MELEERANGE;
}


//
// TELEPORT MOVE
//

//
// PIT_StompThing
//


static boolean PIT_StompThing(mobj_t __far* thing)
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

boolean P_TeleportMove(mobj_t __far* thing, fixed_t x, fixed_t y, boolean boss)
  {
  int16_t     xl;
  int16_t     xh;
  int16_t     yl;
  int16_t     yh;
  int16_t     bx;
  int16_t     by;

  subsector_t __far*  newsubsec;

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

  xl = (_g_tmbbox[BOXLEFT]   - _g_bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
  xh = (_g_tmbbox[BOXRIGHT]  - _g_bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
  yl = (_g_tmbbox[BOXBOTTOM] - _g_bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
  yh = (_g_tmbbox[BOXTOP]    - _g_bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

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


//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//

static boolean PIT_CheckLine (line_t __far* ld)
{
  if (_g_tmbbox[BOXRIGHT]  <= (fixed_t)ld->bbox[BOXLEFT]   << FRACBITS
   || _g_tmbbox[BOXLEFT]   >= (fixed_t)ld->bbox[BOXRIGHT]  << FRACBITS
   || _g_tmbbox[BOXTOP]    <= (fixed_t)ld->bbox[BOXBOTTOM] << FRACBITS
   || _g_tmbbox[BOXBOTTOM] >= (fixed_t)ld->bbox[BOXTOP]    << FRACBITS)
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

  if (!LN_BACKSECTOR(ld)) // one sided line
    {
      return false;
    }

  if (!(tmthing->flags & MF_MISSILE))
    {
      if (ld->flags & ML_BLOCKING)           // explicitly blocking everything
  return false;

      if (!P_MobjIsPlayer(tmthing) && ld->flags & ML_BLOCKMONSTERS)
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
    }

  if (_g_openbottom > _g_tmfloorz)
    {
      _g_tmfloorz = _g_openbottom;
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

static boolean PIT_CheckThing(mobj_t __far* thing)
{
  fixed_t blockdist;
  int16_t damage;

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
      boolean solid = thing->flags & MF_SOLID;
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

boolean P_CheckPosition(mobj_t __far* thing, fixed_t x, fixed_t y)
  {
  int16_t     xl;
  int16_t     xh;
  int16_t     yl;
  int16_t     yh;
  int16_t     bx;
  int16_t     by;
  subsector_t __far*  newsubsec;

  tmthing = thing;

  tmx = x;
  tmy = y;

  _g_tmbbox[BOXTOP] = y + tmthing->radius;
  _g_tmbbox[BOXBOTTOM] = y - tmthing->radius;
  _g_tmbbox[BOXRIGHT] = x + tmthing->radius;
  _g_tmbbox[BOXLEFT] = x - tmthing->radius;

  newsubsec = R_PointInSubsector (x,y);
  _g_ceilingline = NULL;


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

  xl = (_g_tmbbox[BOXLEFT]   - _g_bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
  xh = (_g_tmbbox[BOXRIGHT]  - _g_bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
  yl = (_g_tmbbox[BOXBOTTOM] - _g_bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
  yh = (_g_tmbbox[BOXTOP]    - _g_bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;


  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockThingsIterator(bx,by,PIT_CheckThing))
        return false;

  // check lines

  xl = (_g_tmbbox[BOXLEFT]   - _g_bmaporgx)>>MAPBLOCKSHIFT;
  xh = (_g_tmbbox[BOXRIGHT]  - _g_bmaporgx)>>MAPBLOCKSHIFT;
  yl = (_g_tmbbox[BOXBOTTOM] - _g_bmaporgy)>>MAPBLOCKSHIFT;
  yh = (_g_tmbbox[BOXTOP]    - _g_bmaporgy)>>MAPBLOCKSHIFT;

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockLinesIterator (bx,by,PIT_CheckLine))
        return false; // doesn't fit

  return true;
  }


//
// P_CrossSpecialLine - Walkover Trigger Dispatcher
//
// Called every time a thing origin is about
//  to cross a line with a non 0 special, whether a walkover type or not.
//
// jff 02/12/98 all W1 lines were fixed to check the result from the EV_
//  function before clearing the special. This avoids losing the function
//  of the line, should the sector already be active when the line is
//  crossed. Change is qualified by demo_compatibility.
//
// CPhipps - take a line_t pointer instead of a line number, as in MBF
static void P_CrossSpecialLine(line_t __far* line, int16_t side, mobj_t __far* thing)
{
  boolean         ok;

  //  Things that should never trigger lines
  if (!P_MobjIsPlayer(thing))
  {
    // Things that should NOT trigger specials...
    switch(thing->type)
    {
      case MT_ROCKET:
      case MT_TROOPSHOT:
      case MT_BRUISERSHOT:
        return;

      default: break;
    }
  }


  if (!P_MobjIsPlayer(thing))
  {
    ok = false;
    switch(LN_SPECIAL(line))
    {
      case 97:      // teleport retrigger
      case 88:      // plat down-wait-up-stay retrigger
        ok = true;
        break;
    }
    if (!ok)
      return;
  }

  if (!P_CheckTag(line))  //jff 2/27/98 disallow zero tag on some types
    return;

  // Dispatch on the line special value to the line's action routine
  // If a once only function, and successful, clear the line special

  switch (LN_SPECIAL(line))
  {
      // Regular walk once triggers

    case 2:
      // Open Door
      if (EV_DoDoor(line,dopen))
        LN_SPECIAL(line) = 0;
      break;

    case 5:
      // Raise Floor
      if (EV_DoFloor(line,raiseFloor))
        LN_SPECIAL(line) = 0;
      break;

    case 8:
      // Build Stairs
      if (EV_BuildStairs(line))
        LN_SPECIAL(line) = 0;
      break;

    case 16:
      // Close Door 30
      if (EV_DoDoor(line,close30ThenOpen))
        LN_SPECIAL(line) = 0;
      break;

    case 22:
      // Raise floor to nearest height and change texture
      if (EV_DoPlat(line,raiseToNearestAndChange))
        LN_SPECIAL(line) = 0;
      break;

    case 35:
      // Lights Very Dark
      EV_LightTurnOn(line,35);
      LN_SPECIAL(line) = 0;
      break;

    case 36:
      // Lower Floor (TURBO)
      if (EV_DoFloor(line,turboLower))
        LN_SPECIAL(line) = 0;
      break;

      // Regular walk many retriggerable

    case 76:
      // Close Door 30
      EV_DoDoor(line,close30ThenOpen);
      break;

    case 82:
      // Lower Floor To Lowest
      EV_DoFloor( line, lowerFloorToLowest );
      break;

    case 86:
      // Open Door
      EV_DoDoor(line,dopen);
      break;

    case 88:
      // PlatDownWaitUp
      EV_DoPlat(line,downWaitUpStay);
      break;

    case 90:
      // Raise Door
      EV_DoDoor(line,normal);
      break;

    case 91:
      // Raise Floor
      EV_DoFloor(line,raiseFloor);
      break;

    case 97:
      // TELEPORT!
      EV_Teleport( line, side, thing );
      break;

    case 98:
      // Lower Floor (TURBO)
      EV_DoFloor(line,turboLower);
      break;
  }
}


//
// P_TryMove
// Attempt to move to a new position,
// crossing special lines.
//
boolean P_TryMove(mobj_t __far* thing, fixed_t x, fixed_t y)
{
    fixed_t oldx;
    fixed_t oldy;

    if (!P_CheckPosition (thing, x, y))
        return false;   // solid wall or thing

    if ( !(thing->flags & MF_NOCLIP) )
    {
        if (_g_tmceilingz - _g_tmfloorz < thing->height)
            return false;	// doesn't fit

        if (_g_tmceilingz - thing->z < thing->height)
            return false;	// mobj must lower itself to fit

        if (_g_tmfloorz - thing->z > 24*FRACUNIT )
            return false;	// too big a step up

        if ( !(thing->flags & MF_DROPOFF)
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

    if (!(thing->flags & MF_NOCLIP))
        while (_g_numspechit--)
            if (LN_SPECIAL(_g_spechit[_g_numspechit]))  // see if the line was crossed
            {
                int16_t oldside;
                if ((oldside = P_PointOnLineSide(oldx, oldy, _g_spechit[_g_numspechit])) !=
                        P_PointOnLineSide(thing->x, thing->y, _g_spechit[_g_numspechit]))
                    P_CrossSpecialLine(_g_spechit[_g_numspechit], oldside, thing);
            }

    return true;
}


static fixed_t  aimslope;


//
// PTR_AimTraverse
// Sets linetaget and aimslope when a target is aimed at.
//
static boolean PTR_AimTraverse (intercept_t* in)
{
    const line_t __far* li;
    mobj_t __far* th;
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

        dist = FixedMul(attackrange, in->frac);

        if (LN_FRONTSECTOR(li)->floorheight != LN_BACKSECTOR(li)->floorheight)
        {
            slope = dist != 0 ? FixedApproxDiv(_g_openbottom - shootz, dist) : INT32_MAX;
            if (slope > bottomslope)
                bottomslope = slope;
        }

        if (LN_FRONTSECTOR(li)->ceilingheight != LN_BACKSECTOR(li)->ceilingheight)
        {
            slope = dist != 0 ? FixedApproxDiv(_g_opentop - shootz, dist) : INT32_MAX;
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

    // check angles to see if the thing can be aimed at

    dist = FixedMul (attackrange, in->frac);
    thingtopslope = dist != 0 ? FixedApproxDiv(th->z + th->height - shootz, dist) : INT32_MAX;

    if (thingtopslope < bottomslope)
        return true;    // shot over the thing

    thingbottomslope = dist != 0 ? FixedApproxDiv(th->z - shootz, dist) : INT32_MAX;

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
// P_ShootSpecialLine - Gun trigger special dispatcher
//
// Called when a thing shoots a special line with bullet, shell, saw, or fist.
//
// jff 02/12/98 all G1 lines were fixed to check the result from the EV_
// function before clearing the special. This avoids losing the function
// of the line, should the sector already be in motion when the line is
// impacted. Change is qualified by demo_compatibility.
//
static void P_ShootSpecialLine(mobj_t __far* thing, line_t __far* line)
{
  // Impacts that other things can activate.
  if (!P_MobjIsPlayer(thing))
  {
    boolean ok = false;
    switch(LN_SPECIAL(line))
    {
      case 46:
        // 46 GR Open door on impact weapon is monster activatable
        ok = true;
        break;
    }
    if (!ok)
      return;
  }

  if (!P_CheckTag(line))  //jff 2/27/98 disallow zero tag on some types
    return;

  switch(LN_SPECIAL(line))
  {
    case 46:
      // 46 GR open door, stay open
      EV_DoDoor(line,dopen);
      P_ChangeSwitchTexture(line,true);
      break;
  }
}


static fixed_t FixedMul3(fixed_t a, fixed_t b, fixed_t c)
{
	if (a == 0)
		return 0;

	fixed_t bc = FixedMul(c, b);
	return FixedMul(bc, a);
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

  mobj_t __far* th;

  fixed_t dist;
  fixed_t thingtopslope;
  fixed_t thingbottomslope;

  if (in->isaline)
  {
    line_t __far* li = in->d.line;

    if (LN_SPECIAL(li))
      P_ShootSpecialLine (shootthing, li);

    if (li->flags & ML_TWOSIDED)
    {  // crosses a two sided (really 2s) line
      P_LineOpening (li);
      fixed_t t = FixedMul3(aimslope, in->frac, attackrange) + shootz;

      if ((LN_FRONTSECTOR(li)->floorheight   == LN_BACKSECTOR(li)->floorheight   || _g_openbottom <= t) &&
          (LN_FRONTSECTOR(li)->ceilingheight == LN_BACKSECTOR(li)->ceilingheight || _g_opentop    >= t))
        return true;      // shot continues
    }

    // hit line
    // position a bit closer

    frac = in->frac - 4 * FixedReciprocal(attackrange);
    x = _g_trace.x + FixedMul(_g_trace.dx, frac);
    y = _g_trace.y + FixedMul(_g_trace.dy, frac);
    z = shootz + FixedMul3(aimslope, frac, attackrange);

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

  dist = FixedMul (attackrange, in->frac);
  thingtopslope = FixedApproxDiv (th->z+th->height - shootz , dist);

  if (thingtopslope < aimslope)
    return true;  // shot over the thing

  thingbottomslope = FixedApproxDiv (th->z - shootz, dist);

  if (thingbottomslope > aimslope)
    return true;  // shot under the thing

  // hit thing
  // position a bit closer

  frac = in->frac - 10 * FixedReciprocal(attackrange);

  x = _g_trace.x + FixedMul(_g_trace.dx, frac);
  y = _g_trace.y + FixedMul(_g_trace.dy, frac);
  z = shootz  + FixedMul3(aimslope, frac, attackrange);

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
fixed_t P_AimLineAttack(mobj_t __far* t1, angle_t angle, fixed_t distance)
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

  attackrange = distance;
  _g_linetarget  = NULL;

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

void P_LineAttack(mobj_t __far* t1, angle_t angle, fixed_t distance, fixed_t slope, int16_t damage)
{
  fixed_t x2;
  fixed_t y2;

  angle >>= ANGLETOFINESHIFT;
  shootthing = t1;
  la_damage = damage;
  x2 = t1->x + (distance>>FRACBITS)*finecosine(angle);
  y2 = t1->y + (distance>>FRACBITS)*finesine(  angle);
  shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
  attackrange = distance;
  aimslope = slope;

  P_PathTraverse(t1->x,t1->y,x2,y2,PT_ADDLINES|PT_ADDTHINGS,PTR_ShootTraverse);
}


//
// USE LINES
//


static boolean PTR_UseTraverse (intercept_t* in)
  {
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

  if (P_PointOnLineSide (usething->x, usething->y, in->d.line) != 1)
    P_UseSpecialLine (usething, in->d.line);

  return false;
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
  const line_t __far* ld = in->d.line;
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
  int16_t     angle;
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

static boolean PIT_RadiusAttack (mobj_t __far* thing)
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
void P_RadiusAttack(mobj_t __far* spot, mobj_t __far* source, int16_t damage)
  {
  int16_t x;
  int16_t y;

  int16_t xl;
  int16_t xh;
  int16_t yl;
  int16_t yh;

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


inline static msecnode_t __far* P_GetSecnode(void)
{
  return (msecnode_t __far*)Z_BMalloc(&secnodezone);
}

// P_PutSecnode() returns a node to the freelist.

inline static void P_PutSecnode(msecnode_t __far* node)
{
  Z_BFree(&secnodezone, node);
}

// phares 3/16/98
//
// P_AddSecnode() searches the current list to see if this sector is
// already there. If not, it adds a sector node at the head of the list of
// sectors this object appears in. This is called when creating a list of
// nodes that will get linked in later.

static void P_AddSecnode(sector_t __far* s, mobj_t __far* thing)
  {
  msecnode_t __far* node;

  node = _s_sector_list;
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
  node->m_tnext  = _s_sector_list;  // next node on Thing thread
  if (_s_sector_list)
    _s_sector_list->m_tprev = node; // set back link on Thing

  // Add new node at head of sector thread starting at s->touching_thinglist

  node->m_sprev  = NULL;    // prev node on sector thread
  node->m_snext  = s->touching_thinglist; // next node on sector thread
  if (s->touching_thinglist)
    node->m_snext->m_sprev = node;
  s->touching_thinglist = node;
  _s_sector_list = node;
  }


// P_DelSecnode() deletes a sector node from the list of
// sectors this object appears in. Returns a pointer to the next node
// on the linked list, or NULL.

static msecnode_t __far* P_DelSecnode(msecnode_t __far* node)
  {
  msecnode_t __far* tp;  // prev node on thing thread
  msecnode_t __far* tn;  // next node on thing thread
  msecnode_t __far* sp;  // prev node on sector thread
  msecnode_t __far* sn;  // next node on sector thread

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
	msecnode_t __far* node = _s_sector_list;
	while (node)
		node = P_DelSecnode(node);

	_s_sector_list = NULL;
}


void P_SetSeclist(msecnode_t __far* sectorList)
{
	_s_sector_list = sectorList;
}


// phares 3/14/98
//
// PIT_GetSectors
// Locates all the sectors the object is in by looking at the lines that
// cross through it. You have already decided that the object is allowed
// at this location, so don't bother with checking impassable or
// blocking lines.

static boolean PIT_GetSectors(line_t __far* ld)
  {
  if (_g_tmbbox[BOXRIGHT]  <= (fixed_t)ld->bbox[BOXLEFT]   << FRACBITS ||
      _g_tmbbox[BOXLEFT]   >= (fixed_t)ld->bbox[BOXRIGHT]  << FRACBITS ||
      _g_tmbbox[BOXTOP]    <= (fixed_t)ld->bbox[BOXBOTTOM] << FRACBITS ||
      _g_tmbbox[BOXBOTTOM] >= (fixed_t)ld->bbox[BOXTOP]    << FRACBITS)
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

void P_CreateSecNodeList(mobj_t __far* thing)
{
  mobj_t __far* saved_tmthing = tmthing; /* cph - see comment at func end */

  // First, clear out the existing m_thing fields. As each node is
  // added or verified as needed, m_thing will be set properly. When
  // finished, delete all nodes where m_thing is still NULL. These
  // represent the sectors the Thing has vacated.

  msecnode_t __far* node = _s_sector_list;
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

  int16_t xl = (_g_tmbbox[BOXLEFT]   - _g_bmaporgx) >> MAPBLOCKSHIFT;
  int16_t xh = (_g_tmbbox[BOXRIGHT]  - _g_bmaporgx) >> MAPBLOCKSHIFT;
  int16_t yl = (_g_tmbbox[BOXBOTTOM] - _g_bmaporgy) >> MAPBLOCKSHIFT;
  int16_t yh = (_g_tmbbox[BOXTOP]    - _g_bmaporgy) >> MAPBLOCKSHIFT;

  for (int16_t bx = xl; bx <= xh; bx++)
    for (int16_t by = yl; by <= yh; by++)
      P_BlockLinesIterator(bx,by,PIT_GetSectors);

  // Add the sector of the (x,y) point to sector_list.

  P_AddSecnode(thing->subsector->sector,thing);

  // Now delete any nodes that won't be used. These are the ones where
  // m_thing is still NULL.

  node = _s_sector_list;
  while (node)
    {
    if (node->m_thing == NULL)
      {
      if (node == _s_sector_list)
        _s_sector_list = node->m_tnext;
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

  thing->touching_sectorlist = _s_sector_list; // Attach to Thing's mobj_t
  _s_sector_list = NULL; // clear for next time
}

void P_MapEnd(void)
{
    tmthing = NULL;
}
