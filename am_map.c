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
 *   the automap code
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>

#include "d_player.h"
#include "r_defs.h"
#include "st_stuff.h"
#include "r_main.h"
#include "p_setup.h"
#include "p_maputl.h"
#include "w_wad.h"
#include "v_video.h"
#include "p_spec.h"
#include "am_map.h"
#include "d_englsh.h"
#include "g_game.h"

#include "globdata.h"


enum automapmode_e automapmode; // Mode that the automap is in


#if NR_OF_COLORS == 1
static const uint8_t mapcolor_wall = 0xb0;
static const uint8_t mapcolor_fchg = 0xb1;
static const uint8_t mapcolor_cchg = 0xb1;
static const uint8_t mapcolor_clsd = 0xdb;
static const uint8_t mapcolor_rdor = 0xb2;
static const uint8_t mapcolor_bdor = 0xb2;
static const uint8_t mapcolor_ydor = 0xb2;
static const uint8_t mapcolor_tele = 0xb0;
static const uint8_t mapcolor_secr = 0xb0;
static const uint8_t mapcolor_unsn = 0xb0;
static const uint8_t mapcolor_sngl = 0xdb;
#elif NR_OF_COLORS == 2
#define WHITE	(1<<7)
static const uint8_t mapcolor_wall = WHITE;
static const uint8_t mapcolor_fchg = WHITE;
static const uint8_t mapcolor_cchg = WHITE;
static const uint8_t mapcolor_clsd = WHITE;
static const uint8_t mapcolor_rdor = WHITE;
static const uint8_t mapcolor_bdor = WHITE;
static const uint8_t mapcolor_ydor = WHITE;
static const uint8_t mapcolor_tele = WHITE;
static const uint8_t mapcolor_secr = WHITE;
static const uint8_t mapcolor_unsn = WHITE;
static const uint8_t mapcolor_sngl = WHITE;
#elif NR_OF_COLORS == 4
#define CYAN	(1<<6)
#define MAGENTA	(2<<6)
#define WHITE	(3<<6)
static const uint8_t mapcolor_wall = MAGENTA;
static const uint8_t mapcolor_fchg = MAGENTA;
static const uint8_t mapcolor_cchg = MAGENTA;
static const uint8_t mapcolor_clsd = WHITE;
static const uint8_t mapcolor_rdor = MAGENTA;
static const uint8_t mapcolor_bdor = CYAN;
static const uint8_t mapcolor_ydor = WHITE;
static const uint8_t mapcolor_tele = CYAN;
static const uint8_t mapcolor_secr = MAGENTA;
static const uint8_t mapcolor_unsn = WHITE;
static const uint8_t mapcolor_sngl = WHITE;
#elif NR_OF_COLORS == 16
#define BLUE			0x11
#define GREEN			0x22
#define RED				0x44
#define BROWN			0x66
#define DARK_GRAY		0x88
#define LIGHT_RED		0xcc
#define LIGHT_MAGENTA	0xdd
#define YELLOW			0xee
#define WHITE			0xff
static const uint8_t mapcolor_wall = RED;
static const uint8_t mapcolor_fchg = BROWN;
static const uint8_t mapcolor_cchg = BROWN;
static const uint8_t mapcolor_clsd = WHITE;
static const uint8_t mapcolor_rdor = LIGHT_RED;
static const uint8_t mapcolor_bdor = BLUE;
static const uint8_t mapcolor_ydor = YELLOW;
static const uint8_t mapcolor_tele = GREEN;
static const uint8_t mapcolor_secr = LIGHT_MAGENTA;
static const uint8_t mapcolor_unsn = DARK_GRAY;
static const uint8_t mapcolor_sngl = WHITE;
#else
static const uint8_t mapcolor_wall = 23;    // normal 1s wall color
static const uint8_t mapcolor_fchg = 55;    // line at floor height change color
static const uint8_t mapcolor_cchg = 215;    // line at ceiling height change color
static const uint8_t mapcolor_clsd = 208;    // line at sector with floor=ceiling color
static const uint8_t mapcolor_rdor = 175;    // red door color  (diff from keys to allow option)
static const uint8_t mapcolor_bdor = 204;    // blue door color (of enabling one but not other )
static const uint8_t mapcolor_ydor = 231;    // yellow door color
static const uint8_t mapcolor_tele = 119;    // teleporter line color
static const uint8_t mapcolor_secr = 252;    // secret sector boundary color
static const uint8_t mapcolor_unsn = 104;    // computer map unseen line color
static const uint8_t mapcolor_sngl = 208;    // single player arrow color
#endif


#if !defined MAPWIDTH
#define MAPWIDTH SCREENWIDTH
#endif
static const int16_t f_w = MAPWIDTH;
static const int16_t f_h = VIEWWINDOWHEIGHT;// to allow runtime setting of width/height


typedef struct
{
 fixed_t x,y;
} mpoint_t;

typedef struct
{
    mpoint_t a, b;
} mline_t;

typedef struct
{
  int16_t x, y;
} fpoint_t;

typedef struct
{
  fpoint_t a, b;
} fline_t;


static mpoint_t m_paninc;    // how far the window pans each tic (map coords)

static fixed_t m_x,  m_y;    // LL x,y window location on the map (map coords)
static fixed_t m_x2, m_y2;   // UR x,y window location on the map (map coords)

//
// width/height of window on map (map coords)
//
static fixed_t  m_w;
static fixed_t  m_h;

// based on level size
static fixed_t  min_x;
static fixed_t  min_y;
static fixed_t  max_x;
static fixed_t  max_y;

static fixed_t  max_w;          // max_x-min_x,
static fixed_t  max_h;          // max_y-min_y

static fixed_t  min_scale_mtof; // used to tell when to stop zooming out
static fixed_t  max_scale_mtof; // used to tell when to stop zooming in

// old location used by the Follower routine
static mpoint_t f_oldloc;

// scale on entry
#define INITSCALEMTOF (.2*FRACUNIT)

// used by MTOF to scale from map-to-frame-buffer coords
static fixed_t scale_mtof = (fixed_t)INITSCALEMTOF;
// used by FTOM to scale from frame-buffer-to-map coords (=1/scale_mtof)
static fixed_t scale_ftom;

static boolean stopped = true;

static fixed_t mtof_zoommul = FRACUNIT; // how far the window zooms each tic (map coords)
static fixed_t ftom_zoommul = FRACUNIT; // how far the window zooms each tic (fb coords)

// how much the automap moves window per tic in frame-buffer coordinates
// moves 140 pixels in 1 second
#define F_PANINC  4
// how much zoom-in per tic
// goes to 2x in 1 second
#define M_ZOOMIN        ((int32_t) (1.02*FRACUNIT))
// how much zoom-out per tic
// pulls out to 0.5x in 1 second
#define M_ZOOMOUT       ((int32_t) (FRACUNIT/1.02))

#define MAPBITS 12
#define FRACTOMAPBITS (FRACBITS-MAPBITS)

#define PLAYERRADIUS    (16L*(1<<MAPBITS)) // e6y

// translates between frame-buffer and map distances
#define FTOM(x) ((x)*scale_ftom)
#define MTOF(x) (FixedMul((x),scale_mtof)>>16)
// translates between frame-buffer and map coordinates
#define CXMTOF(x)  (MTOF((x)- m_x))
#define CYMTOF(y)  ((f_h - MTOF((y)- m_y)))


//
// The vector graphics for the automap.
//  A line drawing of the player pointing right,
//   starting from the middle.
//
#define R ((8*PLAYERRADIUS)/7)
static const mline_t player_arrow[] =
{
  { { -R+R/8, 0 }, { R, 0 } }, // -----
  { { R, 0 }, { R-R/2, R/4 } },  // ----->
  { { R, 0 }, { R-R/2, -R/4 } },
  { { -R+R/8, 0 }, { -R-R/8, R/4 } }, // >---->
  { { -R+R/8, 0 }, { -R-R/8, -R/4 } },
  { { -R+3*R/8, 0 }, { -R+R/8, R/4 } }, // >>--->
  { { -R+3*R/8, 0 }, { -R+R/8, -R/4 } }
};
#undef R
#define NUMPLYRLINES (sizeof(player_arrow)/sizeof(mline_t))



//
// AM_activateNewScale()
//
// Changes the map scale after zooming or translating
//
// Passed nothing, returns nothing
//
static void AM_activateNewScale(void)
{
    m_x += m_w/2;
    m_y += m_h/2;
    m_w = FTOM(f_w);
    m_h = FTOM(f_h);
    m_x -= m_w/2;
    m_y -= m_h/2;
    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}

//
// AM_findMinMaxBoundaries()
//
// Determines bounding box of all vertices,
// sets global variables controlling zoom range.
//
// Passed nothing, returns nothing
//
static void AM_findMinMaxBoundaries(void)
{
    int16_t i;
    fixed_t a;
    fixed_t b;
    int16_t min16_x, min16_y, max16_x, max16_y;

    min16_x = min16_y =  INT16_MAX;
    max16_x = max16_y = -INT16_MAX;

    for (i=0;i<_g_numlines;i++)
    {
        if (_g_lines[i].v1.x < min16_x)
            min16_x = _g_lines[i].v1.x;
        else if (_g_lines[i].v1.x > max16_x)
            max16_x = _g_lines[i].v1.x;

        if (_g_lines[i].v2.x < min16_x)
            min16_x = _g_lines[i].v2.x;
        else if (_g_lines[i].v2.x > max16_x)
            max16_x = _g_lines[i].v2.x;

        if (_g_lines[i].v1.y < min16_y)
            min16_y = _g_lines[i].v1.y;
        else if (_g_lines[i].v1.y > max16_y)
            max16_y = _g_lines[i].v1.y;

        if (_g_lines[i].v2.y < min16_y)
            min16_y = _g_lines[i].v2.y;
        else if (_g_lines[i].v2.y > max16_y)
            max16_y = _g_lines[i].v2.y;
    }

    min_x = (fixed_t)min16_x << MAPBITS;
    min_y = (fixed_t)min16_y << MAPBITS;
    max_x = (fixed_t)max16_x << MAPBITS;
    max_y = (fixed_t)max16_y << MAPBITS;

    max_w = max_x - min_x;
    max_h = max_y - min_y;

    a = FixedApproxDiv((int32_t)f_w<<FRACBITS, max_w);
    b = FixedApproxDiv((int32_t)f_h<<FRACBITS, max_h);

    min_scale_mtof = a < b ? a : b;
    max_scale_mtof = FixedApproxDiv((int32_t)f_h<<FRACBITS, 2*PLAYERRADIUS);
}

//
// AM_changeWindowLoc()
//
// Moves the map window by the global variables m_paninc.x, m_paninc.y
//
// Passed nothing, returns nothing
//
static void AM_changeWindowLoc(void)
{
    if (m_paninc.x || m_paninc.y)
    {
        automapmode &= ~am_follow;
        f_oldloc.x = INT32_MAX;
    }

    m_x += m_paninc.x;
    m_y += m_paninc.y;

    if (m_x + m_w/2 > max_x)
        m_x = max_x - m_w/2;
    else if (m_x + m_w/2 < min_x)
        m_x = min_x - m_w/2;

    if (m_y + m_h/2 > max_y)
        m_y = max_y - m_h/2;
    else if (m_y + m_h/2 < min_y)
        m_y = min_y - m_h/2;

    m_x2 = m_x + m_w;
    m_y2 = m_y + m_h;
}


//
// AM_initVariables()
//
// Initialize the variables for the automap
//
// Affects the automap global variables
// Status bar is notified that the automap has been entered
// Passed nothing, returns nothing
//
static void AM_initVariables(void)
{
    automapmode |= (am_active | am_follow);

    f_oldloc.x = INT32_MAX;

    m_paninc.x = m_paninc.y = 0;

    m_w = FTOM(f_w);
    m_h = FTOM(f_h);


    m_x = (_g_player.mo->x >> FRACTOMAPBITS) - m_w/2;//e6y
    m_y = (_g_player.mo->y >> FRACTOMAPBITS) - m_h/2;//e6y
    AM_changeWindowLoc();
}

//
// AM_LevelInit()
//
// Initialize the automap at the start of a new level
// should be called at the start of every level
//
// Passed nothing, returns nothing
// Affects automap's global variables
//
// CPhipps - get status bar height from status bar code
static void AM_LevelInit(void)
{
    AM_findMinMaxBoundaries();
    scale_mtof = FixedApproxDiv(min_scale_mtof, (int32_t) (0.7*FRACUNIT));
    if (scale_mtof > max_scale_mtof)
        scale_mtof = min_scale_mtof;
    scale_ftom = FixedReciprocal(scale_mtof);
}

//
// AM_Stop()
//
// Cease automap operations, unload patches, notify status bar
//
// Passed nothing, returns nothing
//
void AM_Stop (void)
{
    automapmode  = 0;
    stopped = true;
}

//
// AM_Start()
//
// Start up automap operations,
//  if a new level, or game start, (re)initialize level variables
//  init map variables
//  load mark patches
//
// Passed nothing, returns nothing
//
static void AM_Start(void)
{
    static int16_t lastlevel   = -1;
    static int16_t lastepisode = -1;

    if (!stopped)
        AM_Stop();

    stopped = false;
    if (lastlevel != _g_gamemap || lastepisode != 1)
    {
        AM_LevelInit();
        lastlevel = _g_gamemap;
        lastepisode = 1;
    }
    AM_initVariables();
}

//
// AM_minOutWindowScale()
//
// Set the window scale to the maximum size
//
// Passed nothing, returns nothing
//
static void AM_minOutWindowScale(void)
{
    scale_mtof = min_scale_mtof;
    scale_ftom = FixedReciprocal(scale_mtof);
    AM_activateNewScale();
}

//
// AM_maxOutWindowScale(void)
//
// Set the window scale to the minimum size
//
// Passed nothing, returns nothing
//
static void AM_maxOutWindowScale(void)
{
    scale_mtof = max_scale_mtof;
    scale_ftom = FixedReciprocal(scale_mtof);
    AM_activateNewScale();
}

//
// AM_Responder()
//
// Handle events (user inputs) in automap mode
//
// Passed an input event, returns true if its handled
//
boolean AM_Responder(event_t*  ev)
{
    boolean rc;
    int16_t ch;

    rc = false;

    if (!(automapmode & am_active))
    {
        if (ev->type == ev_keydown && ev->data1 == key_map)
        {
            AM_Start ();
            rc = true;
        }
    }
    else if (ev->type == ev_keydown)
    {
        rc = true;
        ch = ev->data1;

        if (ch == key_map_right)
            if (!(automapmode & am_follow))
                m_paninc.x = FTOM(F_PANINC);
            else
                rc = false;
        else if (ch == key_map_left)
            if (!(automapmode & am_follow))
                m_paninc.x = -FTOM(F_PANINC);
            else
                rc = false;
        else if (ch == key_map_up)
            if (!(automapmode & am_follow))
                m_paninc.y = FTOM(F_PANINC);
            else
                rc = false;
        else if (ch == key_map_down)
            if (!(automapmode & am_follow))
                m_paninc.y = -FTOM(F_PANINC);
            else
                rc = false;
        else if (ch == key_map)
        {
            if(automapmode & am_overlay)
                AM_Stop ();
            else
                automapmode |= (am_overlay | am_rotate | am_follow);
        }
        else if (ch == key_map_follow)
        {
            automapmode ^= am_follow;     // CPhipps - put all automap mode stuff into one enum
            f_oldloc.x = INT32_MAX;
            _g_player.message = (automapmode & am_follow) ? AMSTR_FOLLOWON : AMSTR_FOLLOWOFF;
        }
        else if (ch == key_map_zoomout)
        {
            mtof_zoommul = M_ZOOMOUT;
            ftom_zoommul = M_ZOOMIN;
        }
        else if (ch == key_map_zoomin)
        {
            mtof_zoommul = M_ZOOMIN;
            ftom_zoommul = M_ZOOMOUT;
        }
        else
        {
            rc = false;
        }
    }
    else if (ev->type == ev_keyup)
    {
        rc = false;
        ch = ev->data1;
        if (ch == key_map_right)
        {
            if (!(automapmode & am_follow))
                m_paninc.x = 0;
        }
        else if (ch == key_map_left)
        {
            if (!(automapmode & am_follow))
                m_paninc.x = 0;
        }
        else if (ch == key_map_up)
        {
            if (!(automapmode & am_follow))
                m_paninc.y = 0;
        }
        else if (ch == key_map_down)
        {
            if (!(automapmode & am_follow))
                m_paninc.y = 0;
        }
        else if ((ch == key_map_zoomout) || (ch == key_map_zoomin))
        {
            mtof_zoommul = FRACUNIT;
            ftom_zoommul = FRACUNIT;
        }
    }
    return rc;
}

//
// AM_rotate()
//
// Rotation in 2D.
// Used to rotate player arrow line character.
//
// Passed the coordinates of a point, and an angle
// Returns the coordinates rotated by the angle
//
// CPhipps - made static & enhanced for automap rotation

static void AM_rotate(fixed_t* x,  fixed_t* y, angle_t a, fixed_t xorig, fixed_t yorig)
{
    fixed_t tmpx;

    //e6y
    xorig>>=FRACTOMAPBITS;
    yorig>>=FRACTOMAPBITS;

    tmpx =
            FixedMulAngle(*x - xorig,finecosineapprox(a>>ANGLETOFINESHIFT)) -
            FixedMulAngle(*y - yorig,finesineapprox(  a>>ANGLETOFINESHIFT));

    *y   = yorig +
            FixedMulAngle(*x - xorig,finesineapprox(  a>>ANGLETOFINESHIFT)) +
            FixedMulAngle(*y - yorig,finecosineapprox(a>>ANGLETOFINESHIFT));

    *x = tmpx + xorig;
}

//
// AM_changeWindowScale()
//
// Automap zooming
//
// Passed nothing, returns nothing
//
static void AM_changeWindowScale(void)
{
    // Change the scaling multipliers
    scale_mtof = FixedMul(scale_mtof, mtof_zoommul);
    scale_ftom = FixedReciprocal(scale_mtof);

    if (scale_mtof < min_scale_mtof)
        AM_minOutWindowScale();
    else if (scale_mtof > max_scale_mtof)
        AM_maxOutWindowScale();
    else
        AM_activateNewScale();
}

//
// AM_doFollowPlayer()
//
// Turn on follow mode - the map scrolls opposite to player motion
//
// Passed nothing, returns nothing
//
static void AM_doFollowPlayer(void)
{
    if (f_oldloc.x != _g_player.mo->x || f_oldloc.y != _g_player.mo->y)
    {
        m_x = FTOM(MTOF(_g_player.mo->x >> FRACTOMAPBITS)) - m_w/2;//e6y
        m_y = FTOM(MTOF(_g_player.mo->y >> FRACTOMAPBITS)) - m_h/2;//e6y
        m_x2 = m_x + m_w;
        m_y2 = m_y + m_h;
        f_oldloc.x = _g_player.mo->x;
        f_oldloc.y = _g_player.mo->y;
    }
}

//
// AM_Ticker()
//
// Updates on gametic - enter follow mode, zoom, or change map location
//
// Passed nothing, returns nothing
//
void AM_Ticker (void)
{
    if (!(automapmode & am_active))
        return;

    if (automapmode & am_follow)
        AM_doFollowPlayer();

    // Change the zoom if necessary
    if (ftom_zoommul != FRACUNIT)
        AM_changeWindowScale();

    // Change x,y location
    if (m_paninc.x || m_paninc.y)
        AM_changeWindowLoc();
}

//
// AM_clipMline()
//
// Automap clipping of lines.
//
// Based on Cohen-Sutherland clipping algorithm but with a slightly
// faster reject and precalculated slopes. If the speed is needed,
// use a hash algorithm to handle the common cases.
//
// Passed the line's coordinates on map and in the frame buffer performs
// clipping on them in the lines frame coordinates.
// Returns true if any part of line was not clipped
//
static boolean AM_clipMline(mline_t*  ml, fline_t*  fl)
{
    enum
    {
        LEFT    =1,
        RIGHT   =2,
        BOTTOM  =4,
        TOP     =8
    };

    int16_t outcode1 = 0;
    int16_t outcode2 = 0;
    int16_t outside;

    fpoint_t  tmp;
    int16_t   dx;
    int16_t   dy;


#define DOOUTCODE(oc, mx, my) \
    (oc) = 0; \
    if ((my) < 0) (oc) |= TOP; \
    else if ((my) >= f_h) (oc) |= BOTTOM; \
    if ((mx) < 0) (oc) |= LEFT; \
    else if ((mx) >= f_w) (oc) |= RIGHT;


    // do trivial rejects and outcodes
    if (ml->a.y > m_y2)
        outcode1 = TOP;
    else if (ml->a.y < m_y)
        outcode1 = BOTTOM;

    if (ml->b.y > m_y2)
        outcode2 = TOP;
    else if (ml->b.y < m_y)
        outcode2 = BOTTOM;

    if (outcode1 & outcode2)
        return false; // trivially outside

    if (ml->a.x < m_x)
        outcode1 |= LEFT;
    else if (ml->a.x > m_x2)
        outcode1 |= RIGHT;

    if (ml->b.x < m_x)
        outcode2 |= LEFT;
    else if (ml->b.x > m_x2)
        outcode2 |= RIGHT;

    if (outcode1 & outcode2)
        return false; // trivially outside

    // transform to frame-buffer coordinates.
    fl->a.x = CXMTOF(ml->a.x);
    fl->a.y = CYMTOF(ml->a.y);
    fl->b.x = CXMTOF(ml->b.x);
    fl->b.y = CYMTOF(ml->b.y);

    DOOUTCODE(outcode1, fl->a.x, fl->a.y)
            DOOUTCODE(outcode2, fl->b.x, fl->b.y)

            if (outcode1 & outcode2)
            return false;

    while (outcode1 | outcode2)
    {
        // may be partially inside box
        // find an outside point
        if (outcode1)
            outside = outcode1;
        else
            outside = outcode2;

        // clip to each side
        if (outside & TOP)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (dx*(fl->a.y))/dy;
            tmp.y = 0;
        }
        else if (outside & BOTTOM)
        {
            dy = fl->a.y - fl->b.y;
            dx = fl->b.x - fl->a.x;
            tmp.x = fl->a.x + (dx*(fl->a.y-f_h))/dy;
            tmp.y = f_h-1;
        }
        else if (outside & RIGHT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy*(f_w-1 - fl->a.x))/dx;
            tmp.x = f_w-1;
        }
        else if (outside & LEFT)
        {
            dy = fl->b.y - fl->a.y;
            dx = fl->b.x - fl->a.x;
            tmp.y = fl->a.y + (dy*(-fl->a.x))/dx;
            tmp.x = 0;
        }

        if (outside == outcode1)
        {
            fl->a = tmp;
            DOOUTCODE(outcode1, fl->a.x, fl->a.y)
        }
        else
        {
            fl->b = tmp;
            DOOUTCODE(outcode2, fl->b.x, fl->b.y)
        }

        if (outcode1 & outcode2)
            return false; // trivially outside
    }

    return true;
}
#undef DOOUTCODE


//
// AM_drawMline()
//
// Clip lines, draw visible parts of lines.
//
// Passed the map coordinates of the line, and the color to draw it
// Color -1 is special and prevents drawing. Color 247 is special and
// is translated to black, allowing Color 0 to represent feature disable
// in the defaults file.
// Returns nothing.
//
static void AM_drawMline(mline_t* ml, uint8_t color)
{
    fline_t fl;

    if (AM_clipMline(ml, &fl))
        V_DrawLine(fl.a.x, fl.a.y, fl.b.x, fl.b.y, color); // draws it on frame buffer using fb coords
}

//
// AM_DoorColor()
//
// Returns the 'color' or key needed for a door linedef type
//
// Passed the type of linedef, returns:
//   -1 if not a keyed door
//    0 if a red key required
//    1 if a blue key required
//    2 if a yellow key required
static int16_t AM_DoorColor(int16_t type)
{
    switch (type)  // closed keyed door
    {
    case 26: case 32:
        /*bluekey*/
        return 1;
    case 27: case 34:
        /*yellowkey*/
        return 2;
    case 28: case 33:
        /*redkey*/
        return 0;
    default:
        return -1; //not a keyed door
    }
}


//
// P_WasSecret()
//
// Passed a sector, returns if the sector secret type was active, i.e.
// secret type was set and the secret has been obtained already.
//
// jff 3/14/98 added to simplify checks for whether sector is secret
//  in automap and other places
//
static boolean PUREFUNC P_WasSecret(const sector_t __far* sec)
{
  return sec->oldspecial == 9;
}


//
// Determines visible lines, draws them.
// This is LineDef based, not LineSeg based.
//
// jff 1/5/98 many changes in this routine
// backward compatibility not needed, so just changes, no ifs
// addition of clauses for:
//    doors opening, keyed door id, secret sectors,
//    teleports, exit lines, key things
// ability to suppress any of added features or lines with no height changes
//
// support for gamma correction in automap abandoned
//
// jff 4/3/98 changed mapcolor_xxxx=0 as control to disable feature
// jff 4/3/98 changed mapcolor_xxxx=-1 to disable drawing line completely
//
static void AM_drawWalls(void)
{
    int16_t i;
    mline_t l;

    // draw the unclipped visible portions of all lines
    for (i=0;i<_g_numlines;i++)
    {
        l.a.x = (fixed_t)_g_lines[i].v1.x << MAPBITS;
        l.a.y = (fixed_t)_g_lines[i].v1.y << MAPBITS;
        l.b.x = (fixed_t)_g_lines[i].v2.x << MAPBITS;
        l.b.y = (fixed_t)_g_lines[i].v2.y << MAPBITS;


        const sector_t __far* backsector = LN_BACKSECTOR(&_g_lines[i]);
        const sector_t __far* frontsector = LN_FRONTSECTOR(&_g_lines[i]);

        const int16_t line_special =  LN_SPECIAL(&_g_lines[i]);

        if (automapmode & am_rotate)
        {
            AM_rotate(&l.a.x, &l.a.y, ANG90-_g_player.mo->angle, _g_player.mo->x, _g_player.mo->y);
            AM_rotate(&l.b.x, &l.b.y, ANG90-_g_player.mo->angle, _g_player.mo->x, _g_player.mo->y);
        }

        // if line has been seen or IDDT has been used
        if (_g_lines[i].r_flags & ML_MAPPED)
        {
            if (_g_lines[i].flags & ML_DONTDRAW)
                continue;
            {
                /* cph - show keyed doors and lines */
                int16_t amd;
                if (!(_g_lines[i].flags & ML_SECRET) && (amd = AM_DoorColor(line_special)) != -1)
                {
                    {
                        switch (amd) /* closed keyed door */
                        {
                        case 1:
                            /*bluekey*/
                            AM_drawMline(&l,mapcolor_bdor);
                            continue;
                        case 2:
                            /*yellowkey*/
                            AM_drawMline(&l,mapcolor_ydor);
                            continue;
                        case 0:
                            /*redkey*/
                            AM_drawMline(&l,mapcolor_rdor);
                            continue;
                        }
                    }
                }
            }

            if(!backsector)
            {
                // jff 1/10/98 add new color for 1S secret sector boundary
                if (P_WasSecret(frontsector))
                    AM_drawMline(&l, mapcolor_secr); // line bounding secret sector
                else                               //jff 2/16/98 fixed bug
                    AM_drawMline(&l, mapcolor_wall); // special was cleared
            }
            else /* now for 2S lines */
            {
                // jff 1/10/98 add color change for all teleporter types
                if (!(_g_lines[i].flags & ML_SECRET) && (line_special == 97))
                { // teleporters
                    AM_drawMline(&l, mapcolor_tele);
                }
                else if (_g_lines[i].flags & ML_SECRET)    // secret door
                {
                    AM_drawMline(&l, mapcolor_wall);      // wall color
                }
                else if
                        (
                         !(_g_lines[i].flags & ML_SECRET) &&    // non-secret closed door
                         ((backsector->floorheight==backsector->ceilingheight) ||
                          (frontsector->floorheight==frontsector->ceilingheight))
                         )
                {
                    AM_drawMline(&l, mapcolor_clsd);      // non-secret closed door
                } //jff 1/6/98 show secret sector 2S lines
                else if (P_WasSecret(frontsector) || P_WasSecret(backsector))
                {
                    AM_drawMline(&l, mapcolor_secr); // line bounding secret sector
                } //jff 1/6/98 end secret sector line change
                else if (backsector->floorheight !=
                         frontsector->floorheight)
                {
                    AM_drawMline(&l, mapcolor_fchg); // floor level change
                }
                else if (backsector->ceilingheight !=
                         frontsector->ceilingheight)
                {
                    AM_drawMline(&l, mapcolor_cchg); // ceiling level change
                }
            }
        } // now draw the lines only visible because the player has computermap
        else if (_g_player.powers[pw_allmap]) // computermap visible lines
        {
            if (!(_g_lines[i].flags & ML_DONTDRAW)) // invisible flag lines do not show
            {
                AM_drawMline(&l, mapcolor_unsn);
            }
        }
    }
}

//
// AM_drawLineCharacter()
//
// Draws a vector graphic according to numerous parameters
//
// Passed the angle to draw it at, and the map coordinates to draw it at.
// Returns nothing
//
static void AM_drawLineCharacter(angle_t angle, fixed_t x, fixed_t y)
{
    uint16_t   i;
    mline_t l;

    if (automapmode & am_rotate) angle -= _g_player.mo->angle - ANG90; // cph

    for (i=0;i<NUMPLYRLINES;i++)
    {
        l.a.x = player_arrow[i].a.x;
        l.a.y = player_arrow[i].a.y;

        if (angle)
            AM_rotate(&l.a.x, &l.a.y, angle, 0, 0);

        l.a.x += x;
        l.a.y += y;

        l.b.x = player_arrow[i].b.x;
        l.b.y = player_arrow[i].b.y;

        if (angle)
            AM_rotate(&l.b.x, &l.b.y, angle, 0, 0);

        l.b.x += x;
        l.b.y += y;

        AM_drawMline(&l, mapcolor_sngl);
    }
}

//
// AM_drawPlayers()
//
// Draws the player arrow in single player,
//
// Passed nothing, returns nothing
//
static void AM_drawPlayers(void)
{    
    AM_drawLineCharacter(_g_player.mo->angle, _g_player.mo->x >> FRACTOMAPBITS, _g_player.mo->y >> FRACTOMAPBITS);
}


//
// AM_Drawer()
//
// Draws the entire automap
//
// Passed nothing, returns nothing
//
void AM_Drawer (void)
{
    // CPhipps - all automap modes put into one enum
    if (!(automapmode & am_active)) return;

    V_InitDrawLine();

    if (!(automapmode & am_overlay)) // cph - If not overlay mode, clear background for the automap
        V_ClearViewWindow();

    AM_drawWalls();
    AM_drawPlayers();
    V_ShutdownDrawLine();
}
