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
 * DESCRIPTION:
 *      Movement/collision utility functions,
 *      as used by function in p_map.c.
 *      BLOCKMAP Iterator functions,
 *      and some PIT_* functions to use for iteration.
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"

#include "globdata.h"


fixed_t _g_opentop;
fixed_t _g_openbottom;
fixed_t _g_openrange;
fixed_t _g_lowfloor;


divline_t _g_trace;


// 1/11/98 killough: Intercept limit removed
#define MAXINTERCEPTS 64
static intercept_t intercepts[MAXINTERCEPTS];
static intercept_t* intercept_p;


//
// P_AproxDistance
// Gives an estimation of distance (not exact)
//

fixed_t CONSTFUNC P_AproxDistance(fixed_t dx, fixed_t dy)
{
  dx = D_abs(dx);
  dy = D_abs(dy);
  if (dx < dy)
    return dx+dy-(dx>>1);
  return dx+dy-(dy>>1);
}

//
// P_PointOnLineSide
// Returns 0 or 1
//
// killough 5/3/98: reformatted, cleaned up

int16_t PUREFUNC P_PointOnLineSide(fixed_t x, fixed_t y, const line_t __far* line)
{
  return
    !line->dx ? x <= (fixed_t)line->v1.x<<FRACBITS ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= (fixed_t)line->v1.y<<FRACBITS ? line->dx < 0 : line->dx > 0 :
    //FixedMul(y-((fixed_t)line->v1.y<<FRACBITS), line->dx>>FRACBITS) >= FixedMul(line->dy>>FRACBITS, x-((fixed_t)line->v1.x<<FRACBITS));
    ((y - ((fixed_t)line->v1.y<<FRACBITS) >> 8)) * line->dx >= line->dy * ((x - ((fixed_t)line->v1.x<<FRACBITS)) >> 8);
}

//
// P_BoxOnLineSide
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
//
// killough 5/3/98: reformatted, cleaned up

int16_t PUREFUNC P_BoxOnLineSide(const fixed_t *tmbox, const line_t __far* ld)
{
    int16_t p;
    switch (ld->slopetype)
    {

    default: // shut up compiler warnings -- killough
    case ST_HORIZONTAL:
        return
                (tmbox[BOXBOTTOM] > (fixed_t)ld->v1.y<<FRACBITS) == (p = tmbox[BOXTOP] > (fixed_t)ld->v1.y<<FRACBITS) ?
                    p ^ (ld->dx < 0) : -1;
    case ST_VERTICAL:
        return
                (tmbox[BOXLEFT] < (fixed_t)ld->v1.x<<FRACBITS) == (p = tmbox[BOXRIGHT] < (fixed_t)ld->v1.x<<FRACBITS) ?
                    p ^ (ld->dy < 0) : -1;
    case ST_POSITIVE:
        return
                P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld) ==
                (p = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld)) ? p : -1;
    case ST_NEGATIVE:
        return
                (P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld)) ==
                (p = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld)) ? p : -1;
    }
}

//
// P_PointOnDivlineSide
// Returns 0 or 1.
//
// killough 5/3/98: reformatted, cleaned up

static int16_t PUREFUNC P_PointOnDivlineSide(fixed_t x, fixed_t y, const divline_t *line)
{
  return
    !line->dx ? x <= line->x ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= line->y ? line->dx < 0 : line->dx > 0 :
    (line->dy^line->dx^(x -= line->x)^(y -= line->y)) < 0 ? (line->dy^x) < 0 :
    //FixedMul(y>>8, line->dx>>8) >= FixedMul(line->dy>>8, x>>8);
    (y >> 8) * (line->dx >> 16) >= (line->dy >> 16) * (x >> 8);
}

//
// P_MakeDivline
//

static void P_MakeDivline(const line_t __far* li, divline_t *dl)
{
  dl->x = (fixed_t)li->v1.x<<FRACBITS;
  dl->y = (fixed_t)li->v1.y<<FRACBITS;
  dl->dx = (fixed_t)li->dx<<FRACBITS;
  dl->dy = (fixed_t)li->dy<<FRACBITS;
}


static inline fixed_t CONSTFUNC FixedDiv(fixed_t a, fixed_t b)
{
	if (a < 0)
	{
		a = -a;
		b = -b;
	}

	uint16_t ibit = 1;
	while (b < a)
	{
		b    <<= 1;
		ibit <<= 1;
	}

	int16_t ch = 0;
	for (; ibit != 0; ibit >>= 1)
	{
		if (a >= b)
		{
			a  -= b;
			ch |= ibit;
		}
		a <<= 1;
	}

	uint16_t cl = 0;
	if (a >= b) {a -= b; cl |= 0x8000;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x4000;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x2000;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x1000;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0800;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0400;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0200;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0100;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0080;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0040;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0020;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0010;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0008;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0004;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0002;} a <<= 1;
	if (a >= b) {a -= b; cl |= 0x0001;}

	return (((fixed_t)ch) << FRACBITS) | cl;
}


//
// P_InterceptVector
// Returns the fractional intercept point
// along the first divline.
// This is only called by the addthings
// and addlines traversers.
//

static fixed_t PUREFUNC P_InterceptVector3(const divline_t *v2, const divline_t *v1)
{
	fixed_t a = (v1->dy >> FRACBITS) * ((v1->x - v2->x) >> 8);
	fixed_t b = (v1->dx >> FRACBITS) * ((v2->y - v1->y) >> 8);
	fixed_t c = FixedMul(v2->dx, v1->dy >> 8);
	fixed_t d = FixedMul(v2->dy, v1->dx >> 8);

	fixed_t num = a + b;
	fixed_t den = c - d;

	if (num == 0 || den == 0)
		return 0;
	else if ((num ^ den) < 0)
		return -1;
	else
		return FixedDiv(num, den);
}

//
// P_LineOpening
// Sets opentop and openbottom to the window
// through a two sided line.
// OPTIMIZE: keep this precalculated
//

void P_LineOpening(const line_t __far* linedef)
{
    // moved front and back outside P-LineOpening and changed
    // them to these so we can pick up the new friction value
    // in PIT_CheckLine()
    sector_t __far* openfrontsector;
    sector_t __far* openbacksector;

    if (linedef->sidenum[1] == NO_INDEX)      // single sided line
    {
        _g_openrange = 0;
        return;
    }

    openfrontsector = LN_FRONTSECTOR(linedef);
    openbacksector  = LN_BACKSECTOR(linedef);

    if (openfrontsector->ceilingheight < openbacksector->ceilingheight)
        _g_opentop = openfrontsector->ceilingheight;
    else
        _g_opentop = openbacksector->ceilingheight;

    if (openfrontsector->floorheight > openbacksector->floorheight)
    {
        _g_openbottom = openfrontsector->floorheight;
        _g_lowfloor = openbacksector->floorheight;
    }
    else
    {
        _g_openbottom = openbacksector->floorheight;
        _g_lowfloor = openfrontsector->floorheight;
    }
    _g_openrange = _g_opentop - _g_openbottom;
}

//
// THING POSITION SETTING
//

//
// P_UnsetThingPosition
// Unlinks a thing from block map and sectors.
// On each position change, BLOCKMAP and other
// lookups maintaining lists ot things inside
// these structures need to be updated.
//

void P_UnsetThingPosition(mobj_t __far* thing)
{
  if (!(thing->flags & MF_NOSECTOR))
    {
      /* invisible things don't need to be in sector list
       * unlink from subsector
       *
       * killough 8/11/98: simpler scheme using pointers-to-pointers for prev
       * pointers, allows head node pointers to be treated like everything else
       */

      mobj_t __far*__far* sprev = thing->sprev;
      mobj_t __far* snext = thing->snext;
      if ((*sprev = snext))  // unlink from sector list
        snext->sprev = sprev;

        // phares 3/14/98
        //
        // Save the sector list pointed to by touching_sectorlist.
        // In P_SetThingPosition, we'll keep any nodes that represent
        // sectors the Thing still touches. We'll add new ones then, and
        // delete any nodes for sectors the Thing has vacated. Then we'll
        // put it back into touching_sectorlist. It's done this way to
        // avoid a lot of deleting/creating for nodes, when most of the
        // time you just get back what you deleted anyway.
        //
        // If this Thing is being removed entirely, then the calling
        // routine will clear out the nodes in sector_list.

      P_SetSeclist(thing->touching_sectorlist);
      thing->touching_sectorlist = NULL; //to be restored by P_SetThingPosition
    }

  if (!(thing->flags & MF_NOBLOCKMAP))
    {
      /* inert things don't need to be in blockmap
       *
       * killough 8/11/98: simpler scheme using pointers-to-pointers for prev
       * pointers, allows head node pointers to be treated like everything else
       *
       * Also more robust, since it doesn't depend on current position for
       * unlinking. Old method required computing head node based on position
       * at time of unlinking, assuming it was the same position as during
       * linking.
       */

      mobj_t __far* bnext;
      mobj_t __far*__far* bprev = thing->bprev;
      if (bprev && (*bprev = bnext = thing->bnext))  // unlink from block map
        bnext->bprev = bprev;
    }
}

//
// P_SetThingPosition
// Links a thing into both a block and a subsector
// based on it's x y.
// Sets thing->subsector properly
//

void P_SetThingPosition(mobj_t __far* thing)
{                                                      // link into subsector
  subsector_t __far* ss = thing->subsector = R_PointInSubsector(thing->x, thing->y);
  if (!(thing->flags & MF_NOSECTOR))
    {
      // invisible things don't go into the sector links

      // killough 8/11/98: simpler scheme using pointer-to-pointer prev
      // pointers, allows head nodes to be treated like everything else

      mobj_t __far*__far* link = &ss->sector->thinglist;
      mobj_t __far* snext = *link;
      if ((thing->snext = snext))
        snext->sprev = &thing->snext;
      thing->sprev = link;
      *link = thing;

      // phares 3/16/98
      //
      // If sector_list isn't NULL, it has a collection of sector
      // nodes that were just removed from this Thing.

      // Collect the sectors the object will live in by looking at
      // the existing sector_list and adding new nodes and deleting
      // obsolete ones.

      // When a node is deleted, its sector links (the links starting
      // at sector_t->touching_thinglist) are broken. When a node is
      // added, new sector links are created.

      P_CreateSecNodeList(thing);
    }

  // link into blockmap
  if (!(thing->flags & MF_NOBLOCKMAP))
    {
      // inert things don't need to be in blockmap
      int16_t blockx = (thing->x - _g_bmaporgx)>>MAPBLOCKSHIFT;
      int16_t blocky = (thing->y - _g_bmaporgy)>>MAPBLOCKSHIFT;
      if (0 <= blockx && blockx < _g_bmapwidth && 0 <= blocky && blocky < _g_bmapheight)
        {
        // killough 8/11/98: simpler scheme using pointer-to-pointer prev
        // pointers, allows head nodes to be treated like everything else

        mobj_t __far*__far* link = &_g_blocklinks[blocky*_g_bmapwidth+blockx];
        mobj_t __far* bnext = *link;
        if ((thing->bnext = bnext))
          bnext->bprev = &thing->bnext;
        thing->bprev = link;
        *link = thing;
      }
      else        // thing is off the map
        thing->bnext = NULL, thing->bprev = NULL;
    }
}

//
// BLOCK MAP ITERATORS
// For each line/thing in the given mapblock,
// call the passed PIT_* function.
// If the function returns false,
// exit with false without checking anything else.
//

//
// P_BlockLinesIterator
// The validcount flags are used to avoid checking lines
// that are marked in multiple mapblocks,
// so increment validcount before the first call
// to P_BlockLinesIterator, then make one or more calls
// to it.
//
// killough 5/3/98: reformatted, cleaned up

boolean P_BlockLinesIterator(int16_t x, int16_t y, boolean func(line_t __far*))
{

    if (!(0 <= x && x < _g_bmapwidth && 0 <= y && y <_g_bmapheight))
        return true;

    const int16_t offset = _g_blockmap[y*_g_bmapwidth+x];
    const int16_t __far* list = _g_blockmaplump+offset;     // original was reading         // phares


    // delmiting 0 as linedef 0     // phares

    // killough 1/31/98: for compatibility we need to use the old method.
    // Most demos go out of sync, and maybe other problems happen, if we
    // don't consider linedef 0. For safety this should be qualified.

    list++;     // skip 0 starting delimiter                      // phares

    const uint16_t vcount = validcount;

    for ( ; *list != -1 ; list++)                                   // phares
    {
        const int16_t lineno = *list;

        line_t __far* ld = &_g_lines[lineno];

        if (ld->validcount == vcount)
            continue;       // line has already been checked

        ld->validcount = vcount;

        if (!func(ld))
            return false;
    }

    return true;  // everything was checked
}

//
// P_BlockThingsIterator
//

boolean P_BlockThingsIterator(int16_t x, int16_t y, boolean func(mobj_t __far*))
{
  mobj_t __far* mobj;
  if (0 <= x && x < _g_bmapwidth && 0 <= y && y < _g_bmapheight)
    for (mobj = _g_blocklinks[y*_g_bmapwidth+x]; mobj; mobj = mobj->bnext)
      if (!func(mobj))
        return false;
  return true;
}

//
// INTERCEPT ROUTINES
//

// Check for limit and double size if necessary -- killough
static boolean check_intercept(void)
{
    size_t offset = intercept_p - intercepts;

    return (offset < MAXINTERCEPTS);
}


// PIT_AddLineIntercepts.
// Looks for lines in the given block
// that intercept the given trace
// to add to the intercepts list.
//
// A line is crossed if its endpoints
// are on opposite sides of the trace.
//
// killough 5/3/98: reformatted, cleaned up

static boolean PIT_AddLineIntercepts(line_t __far* ld)
{
  int16_t       s1;
  int16_t       s2;
  fixed_t   frac;
  divline_t dl;

  // avoid precision problems with two routines
  if (_g_trace.dx >  FRACUNIT*16 || _g_trace.dy >  FRACUNIT*16 ||
      _g_trace.dx < -FRACUNIT*16 || _g_trace.dy < -FRACUNIT*16)
    {
      s1 = P_PointOnDivlineSide ((fixed_t)ld->v1.x<<FRACBITS, (fixed_t)ld->v1.y<<FRACBITS, &_g_trace);
      s2 = P_PointOnDivlineSide ((fixed_t)ld->v2.x<<FRACBITS, (fixed_t)ld->v2.y<<FRACBITS, &_g_trace);
    }
  else
    {
      s1 = P_PointOnLineSide (_g_trace.x, _g_trace.y, ld);
      s2 = P_PointOnLineSide (_g_trace.x+_g_trace.dx, _g_trace.y+_g_trace.dy, ld);
    }

  if (s1 == s2)
    return true;        // line isn't crossed

  // hit the line
  P_MakeDivline(ld, &dl);
  frac = P_InterceptVector3(&_g_trace, &dl);

  if (frac < 0)
    return true;        // behind source

  if(!check_intercept())
    return false;

  intercept_p->frac = frac;
  intercept_p->isaline = true;
  intercept_p->d.line = ld;
  intercept_p++;

  return true;  // continue
}

//
// PIT_AddThingIntercepts
//
// killough 5/3/98: reformatted, cleaned up

static boolean PIT_AddThingIntercepts(mobj_t __far* thing)
{
  fixed_t   x1, y1;
  fixed_t   x2, y2;
  int16_t       s1, s2;
  divline_t dl;
  fixed_t   frac;

  // check a corner to corner crossection for hit
  if ((_g_trace.dx ^ _g_trace.dy) > 0)
    {
      x1 = thing->x - thing->radius;
      y1 = thing->y + thing->radius;
      x2 = thing->x + thing->radius;
      y2 = thing->y - thing->radius;
    }
  else
    {
      x1 = thing->x - thing->radius;
      y1 = thing->y - thing->radius;
      x2 = thing->x + thing->radius;
      y2 = thing->y + thing->radius;
    }

  s1 = P_PointOnDivlineSide (x1, y1, &_g_trace);
  s2 = P_PointOnDivlineSide (x2, y2, &_g_trace);

  if (s1 == s2)
    return true;                // line isn't crossed

  dl.x = x1;
  dl.y = y1;
  dl.dx = x2-x1;
  dl.dy = y2-y1;

  frac = P_InterceptVector3(&_g_trace, &dl);

  if (frac < 0)
    return true;                // behind source

  if(!check_intercept())
      return false;

  intercept_p->frac = frac;
  intercept_p->isaline = false;
  intercept_p->d.thing = thing;
  intercept_p++;

  return true;          // keep going
}

//
// P_TraverseIntercepts
// Returns true if the traverser function returns true
// for all lines.
//

static boolean P_TraverseIntercepts(traverser_t func)
{
  intercept_t *in = NULL;
  int16_t count = intercept_p - intercepts;
  while (count--)
    {
      fixed_t dist = INT32_MAX;
      intercept_t *scan;
      for (scan = intercepts; scan < intercept_p; scan++)
        if (scan->frac < dist)
          dist = (in=scan)->frac;
      if (dist > FRACUNIT)
        return true;    // checked everything in range
      if (!func(in))
        return false;           // don't bother going farther
      in->frac = INT32_MAX;
    }
  return true;                  // everything was traversed
}

//
// P_PathTraverse
// Traces a line from x1,y1 to x2,y2,
// calling the traverser function for each.
// Returns true if the traverser function returns true
// for all lines.
//

boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
                       int16_t flags, boolean trav(intercept_t *))
{
  int16_t xt1, yt1;
  int16_t xt2, yt2;
  fixed_t xstep, ystep;
  fixed_t xintercept, yintercept;
  int16_t     mapx, mapy;
  int16_t     mapxstep, mapystep;
  int16_t     count;

  validcount++;
  intercept_p = intercepts;

  if (!((x1-_g_bmaporgx)&(MAPBLOCKSIZE-1)))
    x1 += FRACUNIT;     // don't side exactly on a line

  if (!((y1-_g_bmaporgy)&(MAPBLOCKSIZE-1)))
    y1 += FRACUNIT;     // don't side exactly on a line

  _g_trace.x = x1;
  _g_trace.y = y1;
  _g_trace.dx = x2 - x1;
  _g_trace.dy = y2 - y1;

  x1 -= _g_bmaporgx;
  y1 -= _g_bmaporgy;
  xt1 = x1>>MAPBLOCKSHIFT;
  yt1 = y1>>MAPBLOCKSHIFT;

  x2 -= _g_bmaporgx;
  y2 -= _g_bmaporgy;
  xt2 = x2>>MAPBLOCKSHIFT;
  yt2 = y2>>MAPBLOCKSHIFT;

  if (xt2 > xt1)
  {
    mapxstep = 1;
    fixed_t partial = FRACUNIT - ((x1>>MAPBTOFRAC)&(FRACUNIT-1));
    ystep = FixedApproxDiv (y2-y1,x2-x1);
    yintercept = (y1>>MAPBTOFRAC) + FixedMul3216(ystep, partial);
  }
  else if (xt2 < xt1)
  {
    mapxstep = -1;
    fixed_t partial = (x1>>MAPBTOFRAC)&(FRACUNIT-1);
    ystep = FixedApproxDiv (y2-y1,x1-x2);
    yintercept = (y1>>MAPBTOFRAC) + FixedMul3216(ystep, partial);
  }
  else // xt2 == xt1
  {
    mapxstep = 0;
    ystep = 256*FRACUNIT;
    yintercept = (y1>>MAPBTOFRAC) + ystep;
  }


  if (yt2 > yt1)
  {
    mapystep = 1;
    fixed_t partial = FRACUNIT - ((y1>>MAPBTOFRAC)&(FRACUNIT-1));
    xstep = FixedApproxDiv (x2-x1,y2-y1);
    xintercept = (x1>>MAPBTOFRAC) + FixedMul3216(xstep, partial);
  }
  else if (yt2 < yt1)
  {
    mapystep = -1;
    fixed_t partial = (y1>>MAPBTOFRAC)&(FRACUNIT-1);
    xstep = FixedApproxDiv (x2-x1,y1-y2);
    xintercept = (x1>>MAPBTOFRAC) + FixedMul3216(xstep, partial);
  }
  else // yt2 == yt1
  {
    mapystep = 0;
    xstep = 256*FRACUNIT;
    xintercept = (x1>>MAPBTOFRAC) + xstep;
  }


  // Step through map blocks.
  // Count is present to prevent a round off error
  // from skipping the break.

  mapx = xt1;
  mapy = yt1;

  for (count = 0; count < 64; count++)
    {
      if (flags & PT_ADDLINES)
        if (!P_BlockLinesIterator(mapx, mapy,PIT_AddLineIntercepts))
          return false; // early out

      if (flags & PT_ADDTHINGS)
        if (!P_BlockThingsIterator(mapx, mapy,PIT_AddThingIntercepts))
          return false; // early out

      if (mapx == xt2 && mapy == yt2)
        break;

      if ((yintercept >> FRACBITS) == mapy)
        {
          yintercept += ystep;
          mapx += mapxstep;
        }
      else
        if ((xintercept >> FRACBITS) == mapx)
          {
            xintercept += xstep;
            mapy += mapystep;
          }
    }

  // go through the sorted list
  return P_TraverseIntercepts(trav);
}
