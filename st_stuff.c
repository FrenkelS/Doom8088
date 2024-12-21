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
 *      Status bar code.
 *      Does the face/direction indicator animation.
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "compiler.h"
#include "doomdef.h"
#include "d_player.h"
#include "m_random.h"
#include "i_system.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "r_main.h"
#include "am_map.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_englsh.h"

#include "globdata.h"


//
// Typedefs of widgets
//

// Number widget

typedef struct
{
  // upper right-hand corner
  //  of the number (right-justified)
  int16_t   x;
  int16_t   y;

  // max # of digits in number
  int16_t width;

  // last number value
  int16_t   oldnum;

  // pointer to current value
  int16_t*  num;

  // list of patches for 0-9
  int16_t* p;

} st_number_t;


// Multiple Icon widget
typedef struct
{
  // center-justified location of icons
  int16_t     x;
  int16_t     y;

  // last icon number
  int16_t     oldinum;

  // pointer to current icon
  int16_t*    inum;

  // list of icons
  int16_t*   p;

} st_multicon_t;


// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)


static int16_t statusbarnum;

// 0-9, tall numbers
static int16_t tallnum[10];

// 0-9, short, yellow (,different!) numbers
static int16_t shortnum[10];

static int16_t keys[NUMCARDS];

// face status patches
static int16_t faces[ST_NUMFACES];


// weapon ownership patches
static int16_t arms[6][2];

// ready-weapon widget
static st_number_t w_ready;

// health widget
static st_number_t st_health;

// weapon ownership widgets
static st_multicon_t w_arms[6];

// face status widget
static st_multicon_t w_faces;

// keycard widgets
static st_multicon_t w_keyboxes[3];

// ammo widgets
static st_number_t w_ammo[4];

// max ammo widgets
static st_number_t w_maxammo[4];

// armor widget
static st_number_t  st_armor;

// used for evil grin
static boolean  oldweaponsowned[NUMWEAPONS];

 // count until face changes
static int16_t      st_facecount;

// current face index, used by w_faces
static int16_t      st_faceindex;

// holds key-type for each key box on bar
static int16_t      keyboxes[3];

// a random number per tick
static int16_t      st_randomnumber;


// Size of statusbar.
// Now sensitive for scaling.

// proff 08/18/98: Changed for high-res
#define ST_Y      (SCREENHEIGHT - ST_HEIGHT)


//
// STATUS BAR DATA
//

// Location of status bar
#define ST_X                    0


#define ST_TURNOFFSET           (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET           (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET       (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET        (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE              (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE             (ST_GODFACE+1)

// proff 08/18/98: Changed for high-res
#define ST_FACESX               (ST_X+104)
#define ST_FACESY               (ST_Y)

#define ST_EVILGRINCOUNT        (2*TICRATE)
 //Fix Status bar face timing ~Kippykip
#define ST_STRAIGHTFACECOUNT    (18)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN             20

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?
// I dunno, why don't you go and find out!!!  killough

// AMMO number pos.
#define ST_AMMOWIDTH            3
// proff 08/18/98: Changed for high-res
#define ST_AMMOX                (ST_X+32)
#define ST_AMMOY                (ST_Y+6)

// HEALTH number pos.
#define ST_HEALTHWIDTH          3
// proff 08/18/98: Changed for high-res
#define ST_HEALTHX              (ST_X+66)
#define ST_HEALTHY              (ST_Y+6)


// Weapon pos.
// proff 08/18/98: Changed for high-res
#define ST_ARMSX                (ST_X+82)
#define ST_ARMSY                (ST_Y+4)

#define ST_ARMSXSPACE           8
#define ST_ARMSYSPACE           10


// ARMOR number pos.
#define ST_ARMORWIDTH           3
// proff 08/18/98: Changed for high-res
#define ST_ARMORX               (ST_X+166)
#define ST_ARMORY               (ST_Y+6)


// proff 08/18/98: Changed for high-res
#define ST_KEY0X                (ST_X+178)
#define ST_KEY0Y                (ST_Y+3)

// proff 08/18/98: Changed for high-res
#define ST_KEY1X                (ST_X+178)
#define ST_KEY1Y                (ST_Y+13)

// proff 08/18/98: Changed for high-res
#define ST_KEY2X                (ST_X+178)
#define ST_KEY2Y                (ST_Y+23)


// Ammunition counter.
#define ST_AMMO0WIDTH           3
// proff 08/18/98: Changed for high-res
#define ST_AMMO0X               (ST_X+220)
#define ST_AMMO0Y               (ST_Y+5)

#define ST_AMMO1WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO1X               (ST_X+220)
#define ST_AMMO1Y               (ST_Y+11)

#define ST_AMMO2WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO2X               (ST_X+220)
#define ST_AMMO2Y               (ST_Y+17)

#define ST_AMMO3WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO3X               (ST_X+220)
#define ST_AMMO3Y               (ST_Y+23)


// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH        3
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO0X            (ST_X+238)
#define ST_MAXAMMO0Y            (ST_Y+5)

#define ST_MAXAMMO1WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO1X            (ST_X+238)
#define ST_MAXAMMO1Y            (ST_Y+11)

#define ST_MAXAMMO2WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO2X            (ST_X+238)
#define ST_MAXAMMO2Y            (ST_Y+17)

#define ST_MAXAMMO3WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO3X            (ST_X+238)
#define ST_MAXAMMO3Y            (ST_Y+23)


// used to use appopriately pained face
static int16_t      st_oldhealth = -1;


//
// STATUS BAR CODE
//

static void ST_Stop(void);


static int16_t ST_calcPainOffset(void)
{
  static int16_t lastcalc;
  static int16_t oldhealth = -1;
  int16_t health = _g_player.health > 100 ? 100 : _g_player.health;

  if (health != oldhealth)
    {
      lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
      oldhealth = health;
    }
  return lastcalc;
}

//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//

static void ST_updateFaceWidget(void)
{
    int16_t         i;
	angle_t     badguyangle;
	angle_t     diffang;
    static int16_t  lastattackdown = -1;
    static int8_t  priority = 0;
    boolean     doevilgrin;

    if (priority < 10)
    {
        // dead
        if (!_g_player.health)
        {
            priority = 9;
            st_faceindex = ST_DEADFACE;
            st_facecount = 1;
        }
    }

    if (priority < 9)
    {
        if (_g_player.bonuscount)
        {
            // picking up bonus
            doevilgrin = false;

            for (i=0;i<NUMWEAPONS;i++)
            {
                if (oldweaponsowned[i] != _g_player.weaponowned[i])
                {
                    doevilgrin = true;
                    oldweaponsowned[i] = _g_player.weaponowned[i];
                }
            }
            if (doevilgrin)
            {
                // evil grin if just picked up weapon
                priority = 8;
                st_facecount = ST_EVILGRINCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
            }
        }

    }
	
	//Restore the face looking at enemies direction in this SVN... Cause it's handy! ~Kippykip
	if (priority < 8)
    {
        if (_g_player.damagecount && _g_player.attacker && _g_player.attacker != _g_player.mo)
		{
			// being attacked
			priority = 7;

			// haleyjd 10/12/03: classic DOOM problem of missing OUCH face
			// was due to inversion of this test:
			// if(plyr->health - st_oldhealth > ST_MUCHPAIN)
            if(st_oldhealth - _g_player.health > ST_MUCHPAIN)
			{
				st_facecount = ST_TURNCOUNT;
				st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
			}
			else
			{
                badguyangle = R_PointToAngle2(_g_player.mo->x,
                _g_player.mo->y,
                _g_player.attacker->x,
                _g_player.attacker->y);

                if (badguyangle > _g_player.mo->angle)
				{
					// whether right or left
                    diffang = badguyangle - _g_player.mo->angle;
					i = diffang > ANG180;
				}
				else
				{
					// whether left or right
                    diffang = _g_player.mo->angle - badguyangle;
					i = diffang <= ANG180;
				} // confusing, aint it?


				st_facecount = ST_TURNCOUNT;
				st_faceindex = ST_calcPainOffset();

				if (diffang < ANG45)
				{
					// head-on
					st_faceindex += ST_RAMPAGEOFFSET;
				}
				else if (i)
				{
					// turn face right
					st_faceindex += ST_TURNOFFSET;
				}
				else
				{
					// turn face left
					st_faceindex += ST_TURNOFFSET+1;
				}
			}
		}
    }

    if (priority < 7)
    {
        if (_g_player.damagecount)
        {
            // haleyjd 10/12/03: classic DOOM problem of missing OUCH face
            // was due to inversion of this test:
            // if(plyr->health - st_oldhealth > ST_MUCHPAIN)
            if(st_oldhealth - _g_player.health > ST_MUCHPAIN)
            {
                priority = 7;
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
            else
            {
                priority = 6;
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
            }

        }
    }

    if (priority < 6)
    {
        // rapid firing
        if (_g_player.attackdown)
        {
            if (lastattackdown==-1)
                lastattackdown = ST_RAMPAGEDELAY;
            else if (!--lastattackdown)
            {
                priority = 5;
                st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
                st_facecount = 1;
                lastattackdown = 1;
            }
        }
        else
            lastattackdown = -1;

    }

    if (priority < 5)
    {
        // invulnerability
        if ((_g_player.cheats & CF_GODMODE)
                || _g_player.powers[pw_invulnerability])
        {
            priority = 4;

            st_faceindex = ST_GODFACE;
            st_facecount = 1;

        }

    }

    // look left or look right if the facecount has timed out
    if (!st_facecount)
    {
        st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
        st_facecount = ST_STRAIGHTFACECOUNT;
        priority = 0;
    }

    st_facecount--;

}


static int16_t largeammo = 1994; // means "n/a"

static void ST_updateWidgets(void)
{
    int8_t         i;

    if(_g_fps_show)
        w_ready.num = &_g_fps_framerate;
    else if (weaponinfo[_g_player.readyweapon].ammo == am_noammo)
        w_ready.num = &largeammo;
    else
        w_ready.num = &_g_player.ammo[weaponinfo[_g_player.readyweapon].ammo];


    // update keycard multiple widgets
    for (i=0;i<3;i++)
    {
        keyboxes[i] = _g_player.cards[i] ? i : -1;
    }

    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();
}

void ST_Ticker(void)
{
  st_randomnumber = M_Random();
  ST_updateWidgets();
  st_oldhealth = _g_player.health;
}


//
// STlib_updateMultIcon()
//
// Draw a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys. Displays each when the control
// numbers change
//
// Passed a st_multicon_t widget
// Returns nothing.
//
static void STlib_updateMultIcon(st_multicon_t* mi)
{
    if(!mi->p)
        return;

    if (*mi->inum != -1)  // killough 2/16/98: redraw only if != -1
		V_DrawNumPatchNotScaled(mi->x, mi->y, mi->p[*mi->inum]);

    mi->oldinum = *mi->inum;

}


/*
 * STlib_drawNum()
 *
 * A fairly efficient way to draw a number based on differences from the
 * old number.
 *
 * Passed a st_number_t widget
 * Returns nothing
 *
 * jff 2/16/98 add color translation to digit output
 * cphipps 10/99 - const pointer to colour trans table, made function static
 */
static void STlib_drawNum(st_number_t* n)
{

  int16_t   numdigits = n->width;
  int16_t   num = *n->num;

  // CPhipps - compact some code, use num instead of *n->num
  if ((n->oldnum = num) < 0)
  {
    if (numdigits == 2 && num < -9)
      num = -9;
    else if (numdigits == 3 && num < -99)
      num = -99;

    num = -num;
  }

  // if non-number, do not draw it
  if (num == largeammo)
    return;

  const int16_t   w = V_NumPatchWidth(n->p[0]);
  int16_t   x = n->x;

  // in the special case of 0, you draw 0
  if (!num)
    V_DrawNumPatchNotScaled(x - w, n->y, n->p[0]);

  // draw the new number
  while (num && numdigits--)
  {
    x -= w;
    V_DrawNumPatchNotScaled(x, n->y, n->p[num % 10]);
    num /= 10;
  }
}


static void ST_drawWidgets(void)
{
    STlib_drawNum(&w_ready);
	
	// Restore the ammo numbers for backpack stats I guess, etc ~Kippykip
	for (int8_t i = 0; i < NUMAMMO; i++)
    {
		STlib_drawNum(&w_ammo[i]);
		STlib_drawNum(&w_maxammo[i]);
    }

    STlib_drawNum(&st_health);
    STlib_drawNum(&st_armor);

    STlib_updateMultIcon(&w_faces);

    for (int8_t i = 0; i < 3 ;i++)
        STlib_updateMultIcon(&w_keyboxes[i]);

    for (int8_t i = 0; i < 6; i++)
        STlib_updateMultIcon(&w_arms[i]);
}


static void ST_refreshBackground(void)
{
	static const uint16_t st_offset = (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH;
	V_DrawRaw(statusbarnum, st_offset);
}


void ST_doRefresh(void)
{
#if !defined DISABLE_STATUS_BAR
  // draw status bar background to off-screen buff
  ST_refreshBackground();

  // and refresh all widgets
  ST_drawWidgets();
#endif
}

boolean ST_NeedUpdate(void)
{
	// ready weapon ammo
	if(w_ready.oldnum != *w_ready.num)
        return true;
	
    if(st_health.oldnum != *st_health.num)
        return true;

    if(st_armor.oldnum != *st_armor.num)
        return true;

    if(w_faces.oldinum != *w_faces.inum)
        return true;
	
	// ammo
    for(int8_t i=0; i<NUMAMMO; i++)
    {
        if(w_ammo[i].oldnum != *w_ammo[i].num)
            return true;
		if(w_maxammo[i].oldnum != *w_maxammo[i].num)
            return true;
    }

    // weapons owned
    for(int8_t i=0; i<6; i++)
    {
        if(w_arms[i].oldinum != *w_arms[i].inum)
            return true;
    }

    for(int8_t i = 0; i < 3; i++)
    {
        if(w_keyboxes[i].oldinum != *w_keyboxes[i].inum)
            return true;
    }

    return false;
}


//
// ST_loadData
//
// CPhipps - Loads graphics needed for status bar
//
static void ST_loadData(void)
{
    int8_t  i, facenum;
    char namebuf[9];

    statusbarnum = W_GetNumForName("STBAR");

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
		sprintf(namebuf, "STGANUM%d", i); //Special GBA Doom II Red Numbers ~Kippykip
        tallnum[i] = W_GetNumForName(namebuf);

        sprintf(namebuf, "STYSNUM%d", i);
        shortnum[i] = W_GetNumForName(namebuf);
    }

    // key cards
    for (i=0;i<NUMCARDS;i++)
    {
        sprintf(namebuf, "STKEYS%d", i);
        keys[i] = W_GetNumForName(namebuf);
    }

    // arms ownership widgets
    for (i=0;i<6;i++)
    {
        sprintf(namebuf, "STGNUM%d", i+2);

        // gray #
        arms[i][0] = W_GetNumForName(namebuf);

        // yellow #
        arms[i][1] = shortnum[i+2];
    }

    // face states
    facenum = 0;

    for (i=0;i<ST_NUMPAINFACES;i++)
    {
        for (int8_t j=0;j<ST_NUMSTRAIGHTFACES;j++)
        {
            sprintf(namebuf, "STFST%d%d", i, j);
            faces[facenum++] = W_GetNumForName(namebuf);
        }
        sprintf(namebuf, "STFTR%d0", i);	// turn right
        faces[facenum++] = W_GetNumForName(namebuf);
        sprintf(namebuf, "STFTL%d0", i);	// turn left
        faces[facenum++] = W_GetNumForName(namebuf);
        sprintf(namebuf, "STFOUCH%d", i);	// ouch!
        faces[facenum++] = W_GetNumForName(namebuf);
        sprintf(namebuf, "STFEVL%d", i);	// evil grin ;)
        faces[facenum++] = W_GetNumForName(namebuf);
        sprintf(namebuf, "STFKILL%d", i);	// pissed off
        faces[facenum++] = W_GetNumForName(namebuf);
    }
    faces[facenum++] = W_GetNumForName("STFGOD0");
    faces[facenum++] = W_GetNumForName("STFDEAD0");
}


static void ST_initData(void)
{
    int8_t i;

    st_faceindex = 0;
    ST_initPalette();

    st_oldhealth = -1;

    for (i=0;i<NUMWEAPONS;i++)
        oldweaponsowned[i] = _g_player.weaponowned[i];

    for (i=0;i<3;i++)
        keyboxes[i] = -1;
}


//
// STlib_initMultIcon()
//
// Initialize a st_multicon_t widget, used for a multigraphic display
// like the status bar's keys.
//
// Passed a st_multicon_t widget, the position, the graphic patches, and a pointer
// to the numbers representing what to display
// Returns nothing.
//
static void STlib_initMultIcon(st_multicon_t* i, int16_t x, int16_t y, int16_t* il, int16_t* inum)
{
	i->x       = x;
	i->y       = y;
	i->oldinum = -1;
	i->inum    = inum;
	i->p       = il;
}


//
// STlib_initNum()
//
// Initializes an st_number_t widget
//
// Passed the widget, its position, the patches for the digits, a pointer
// to the value displayed, and the width
// Returns nothing
//
static void STlib_initNum(st_number_t* n, int16_t x, int16_t y, int16_t* pl, int16_t* num, int16_t width)
{
	n->x      = x;
	n->y      = y;
	n->oldnum = 0;
	n->width  = width;
	n->num    = num;
	n->p      = pl;
}


static void ST_createWidgets(void)
{
    // ready weapon ammo
    STlib_initNum(&w_ready, ST_AMMOX, ST_AMMOY, tallnum, &_g_player.ammo[weaponinfo[_g_player.readyweapon].ammo], ST_AMMOWIDTH);

    // health percentage
    STlib_initNum(&st_health, ST_HEALTHX, ST_HEALTHY, tallnum, &_g_player.health, ST_HEALTHWIDTH);
					  
    // armor percentage - should be colored later
    STlib_initNum(&st_armor, ST_ARMORX, ST_ARMORY, tallnum, &_g_player.armorpoints, ST_ARMORWIDTH);

    // weapons owned
    for(int8_t i = 0; i < 6; i++)
    {
        STlib_initMultIcon(&w_arms[i], ST_ARMSX+(i%3)*ST_ARMSXSPACE, ST_ARMSY+(i/3)*ST_ARMSYSPACE, arms[i], &_g_player.weaponowned[i+1]);
    }
	
    // keyboxes 0-2
    STlib_initMultIcon(&w_keyboxes[0], ST_KEY0X, ST_KEY0Y, keys, &keyboxes[0]);
    STlib_initMultIcon(&w_keyboxes[1], ST_KEY1X, ST_KEY1Y, keys, &keyboxes[1]);
    STlib_initMultIcon(&w_keyboxes[2], ST_KEY2X, ST_KEY2Y, keys, &keyboxes[2]);			
			
	// ammo count (all four kinds)
	STlib_initNum(&w_ammo[am_clip],  ST_AMMO0X, ST_AMMO0Y, shortnum, &_g_player.ammo[am_clip],  ST_AMMO0WIDTH);
	STlib_initNum(&w_ammo[am_shell], ST_AMMO1X, ST_AMMO1Y, shortnum, &_g_player.ammo[am_shell], ST_AMMO1WIDTH);
	STlib_initNum(&w_ammo[am_misl],  ST_AMMO2X, ST_AMMO2Y, shortnum, &_g_player.ammo[am_misl],  ST_AMMO2WIDTH);
	STlib_initNum(&w_ammo[am_cell],  ST_AMMO3X, ST_AMMO3Y, shortnum, &_g_player.ammo[am_cell],  ST_AMMO3WIDTH);

	// max ammo count (all four kinds)
	STlib_initNum(&w_maxammo[am_clip],  ST_MAXAMMO0X, ST_MAXAMMO0Y, shortnum, &_g_player.maxammo[am_clip],  ST_MAXAMMO0WIDTH);
	STlib_initNum(&w_maxammo[am_shell], ST_MAXAMMO1X, ST_MAXAMMO1Y, shortnum, &_g_player.maxammo[am_shell], ST_MAXAMMO1WIDTH);
	STlib_initNum(&w_maxammo[am_misl],  ST_MAXAMMO2X, ST_MAXAMMO2Y, shortnum, &_g_player.maxammo[am_misl],  ST_MAXAMMO2WIDTH);
	STlib_initNum(&w_maxammo[am_cell],  ST_MAXAMMO3X, ST_MAXAMMO3Y, shortnum, &_g_player.maxammo[am_cell],  ST_MAXAMMO3WIDTH);
			
    // faces
    STlib_initMultIcon(&w_faces, ST_FACESX, ST_FACESY, faces, &st_faceindex);
}

static boolean st_stopped = true;

void ST_Start(void)
{
  if (!st_stopped)
    ST_Stop();
  ST_initData();
  ST_createWidgets();
  st_stopped = false;
}

static void ST_Stop(void)
{
  if (st_stopped)
    return;
  I_SetPalette(0);
  st_stopped = true;
}

void ST_Init(void)
{
  ST_loadData();
}
