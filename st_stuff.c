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
 *      Status bar code.
 *      Does the face/direction indicator animation.
 *      Does palette indicators as well (red pain/berserk, bright pickup)
 *
 *-----------------------------------------------------------------------------*/

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


#define DISABLE_STATUS_BAR


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
#define ST_AMMO2Y               (ST_Y+23)

#define ST_AMMO3WIDTH           ST_AMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_AMMO3X               (ST_X+220)
#define ST_AMMO3Y               (ST_Y+17)


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
#define ST_MAXAMMO2Y            (ST_Y+23)

#define ST_MAXAMMO3WIDTH        ST_MAXAMMO0WIDTH
// proff 08/18/98: Changed for high-res
#define ST_MAXAMMO3X            (ST_X+238)
#define ST_MAXAMMO3Y            (ST_Y+17)


// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13


// used to use appopriately pained face
static int32_t      st_oldhealth = -1;


//
// STATUS BAR CODE
//

static void ST_Stop(void);

// Respond to keyboard input events,
//  intercept cheats.
boolean ST_Responder(const event_t *ev)
{
  // Filter automap on/off.
  if (ev->type == ev_keyup && (ev->data1 & 0xffff0000) == AM_MSGHEADER)
    {
      switch(ev->data1)
        {
        case AM_MSGENTERED:
          break;

        case AM_MSGEXITED:
          break;
        }
    }

  return false;
}

static int16_t ST_calcPainOffset(void)
{
  static int16_t lastcalc;
  static int16_t oldhealth = -1;
  int16_t health = _g->player.health > 100 ? 100 : _g->player.health;

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
        if (!_g->player.health)
        {
            priority = 9;
            _g->st_faceindex = ST_DEADFACE;
            _g->st_facecount = 1;
        }
    }

    if (priority < 9)
    {
        if (_g->player.bonuscount)
        {
            // picking up bonus
            doevilgrin = false;

            for (i=0;i<NUMWEAPONS;i++)
            {
                if (_g->oldweaponsowned[i] != _g->player.weaponowned[i])
                {
                    doevilgrin = true;
                    _g->oldweaponsowned[i] = _g->player.weaponowned[i];
                }
            }
            if (doevilgrin)
            {
                // evil grin if just picked up weapon
                priority = 8;
                _g->st_facecount = ST_EVILGRINCOUNT;
                _g->st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
            }
        }

    }
	
	//Restore the face looking at enemies direction in this SVN... Cause it's handy! ~Kippykip
	if (priority < 8)
    {
        if (_g->player.damagecount && _g->player.attacker && _g->player.attacker != _g->player.mo)
		{
			// being attacked
			priority = 7;

			// haleyjd 10/12/03: classic DOOM problem of missing OUCH face
			// was due to inversion of this test:
			// if(plyr->health - st_oldhealth > ST_MUCHPAIN)
            if(st_oldhealth - _g->player.health > ST_MUCHPAIN)
			{
				_g->st_facecount = ST_TURNCOUNT;
				_g->st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
			}
			else
			{
                badguyangle = R_PointToAngle2(_g->player.mo->x,
                _g->player.mo->y,
                _g->player.attacker->x,
                _g->player.attacker->y);

                if (badguyangle > _g->player.mo->angle)
				{
					// whether right or left
                    diffang = badguyangle - _g->player.mo->angle;
					i = diffang > ANG180;
				}
				else
				{
					// whether left or right
                    diffang = _g->player.mo->angle - badguyangle;
					i = diffang <= ANG180;
				} // confusing, aint it?


				_g->st_facecount = ST_TURNCOUNT;
				_g->st_faceindex = ST_calcPainOffset();

				if (diffang < ANG45)
				{
					// head-on
					_g->st_faceindex += ST_RAMPAGEOFFSET;
				}
				else if (i)
				{
					// turn face right
					_g->st_faceindex += ST_TURNOFFSET;
				}
				else
				{
					// turn face left
					_g->st_faceindex += ST_TURNOFFSET+1;
				}
			}
		}
    }

    if (priority < 7)
    {
        if (_g->player.damagecount)
        {
            // haleyjd 10/12/03: classic DOOM problem of missing OUCH face
            // was due to inversion of this test:
            // if(plyr->health - st_oldhealth > ST_MUCHPAIN)
            if(st_oldhealth - _g->player.health > ST_MUCHPAIN)
            {
                priority = 7;
                _g->st_facecount = ST_TURNCOUNT;
                _g->st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
            else
            {
                priority = 6;
                _g->st_facecount = ST_TURNCOUNT;
                _g->st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
            }

        }
    }

    if (priority < 6)
    {
        // rapid firing
        if (_g->player.attackdown)
        {
            if (lastattackdown==-1)
                lastattackdown = ST_RAMPAGEDELAY;
            else if (!--lastattackdown)
            {
                priority = 5;
                _g->st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
                _g->st_facecount = 1;
                lastattackdown = 1;
            }
        }
        else
            lastattackdown = -1;

    }

    if (priority < 5)
    {
        // invulnerability
        if ((_g->player.cheats & CF_GODMODE)
                || _g->player.powers[pw_invulnerability])
        {
            priority = 4;

            _g->st_faceindex = ST_GODFACE;
            _g->st_facecount = 1;

        }

    }

    // look left or look right if the facecount has timed out
    if (!_g->st_facecount)
    {
        _g->st_faceindex = ST_calcPainOffset() + (_g->st_randomnumber % 3);
        _g->st_facecount = ST_STRAIGHTFACECOUNT;
        priority = 0;
    }

    _g->st_facecount--;

}


static int16_t largeammo = 1994; // means "n/a"

static void ST_updateWidgets(void)
{
    int8_t         i;

    if(_g->fps_show)
        _g->w_ready.num = &_g->fps_framerate;
    else if (weaponinfo[_g->player.readyweapon].ammo == am_noammo)
        _g->w_ready.num = &largeammo;
    else
        _g->w_ready.num = &_g->player.ammo[weaponinfo[_g->player.readyweapon].ammo];


    // update keycard multiple widgets
    for (i=0;i<3;i++)
    {
        _g->keyboxes[i] = _g->player.cards[i] ? i : -1;

        //jff 2/24/98 select double key
        //killough 2/28/98: preserve traditional keys by config option

        if (_g->player.cards[i+3])
            _g->keyboxes[i] = i+3;
    }

    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();
}

void ST_Ticker(void)
{
  _g->st_randomnumber = M_Random();
  ST_updateWidgets();
  st_oldhealth = _g->player.health;
}


static void ST_doPaletteStuff(void)
{
    int8_t         palette;
    int32_t cnt = _g->player.damagecount;

    if (_g->player.powers[pw_strength])
    {
        // slowly fade the berzerk out
        int32_t bzc = 12 - (_g->player.powers[pw_strength]>>6);
        if (bzc > cnt)
            cnt = bzc;
    }

    if (cnt)
    {
        palette = (cnt+7)>>3;
        if (palette >= NUMREDPALS)
            palette = NUMREDPALS-1;

        /* cph 2006/08/06 - if in the menu, reduce the red tint - navigating to
       * load a game can be tricky if the screen is all red */
        if (_g->menuactive) palette >>=1;

        palette += STARTREDPALS;
    }
    else
        if (_g->player.bonuscount)
        {
            palette = (_g->player.bonuscount+7)>>3;
            if (palette >= NUMBONUSPALS)
                palette = NUMBONUSPALS-1;
            palette += STARTBONUSPALS;
        }
        else
            if (_g->player.powers[pw_ironfeet] > 4*32 || _g->player.powers[pw_ironfeet] & 8)
                palette = RADIATIONPAL;
            else
                palette = 0;

    if (palette != _g->st_palette) {
        I_SetPalette(_g->st_palette = palette); // CPhipps - use new palette function
    }
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
		V_DrawNumPatchNoScale(mi->x, mi->y, mi->p[*mi->inum]);

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

  const int16_t   w = V_NumPatchWidth(n->p[0]);
  int16_t   x = n->x;

  // CPhipps - compact some code, use num instead of *n->num
  if ((n->oldnum = num) < 0)
  {
    if (numdigits == 2 && num < -9)
      num = -9;
    else if (numdigits == 3 && num < -99)
      num = -99;

    num = -num;
  }

  // clear the area
  x = n->x - numdigits*w;

  // if non-number, do not draw it
  if (num == largeammo)
    return;

  x = n->x;

  // in the special case of 0, you draw 0
  if (!num)
    V_DrawNumPatchNoScale(x - w, n->y, n->p[0]);

  // draw the new number
  while (num && numdigits--)
  {
    x -= w;
    V_DrawNumPatchNoScale(x, n->y, n->p[num % 10]);
    num /= 10;
  }
}


static void ST_drawWidgets(void)
{
    STlib_drawNum(&_g->w_ready);
	
	// Restore the ammo numbers for backpack stats I guess, etc ~Kippykip
	for (int8_t i = 0; i < 4; i++)
    {
		STlib_drawNum(&_g->w_ammo[i]);
		STlib_drawNum(&_g->w_maxammo[i]);
    }

    STlib_drawNum(&_g->st_health);
    STlib_drawNum(&_g->st_armor);

    STlib_updateMultIcon(&_g->w_faces);

    for (int8_t i = 0; i < 3 ;i++)
        STlib_updateMultIcon(&_g->w_keyboxes[i]);

    for (int8_t i = 0; i < 6; i++)
        STlib_updateMultIcon(&_g->w_arms[i]);
}


static void ST_refreshBackground(void)
{
	const uint16_t st_offset = (SCREENHEIGHT - ST_SCALED_HEIGHT) * SCREENWIDTH;
	W_ReadLumpByName("STBAR", &_g->screen[st_offset]);
}


static void ST_doRefresh(void)
{
#if !defined DISABLE_STATUS_BAR
  // draw status bar background to off-screen buff
  ST_refreshBackground();

  // and refresh all widgets
  ST_drawWidgets();
#endif
}

static boolean ST_NeedUpdate()
{
	// ready weapon ammo
	if(_g->w_ready.oldnum != *_g->w_ready.num)
        return true;
	
    if(_g->st_health.oldnum != *_g->st_health.num)
        return true;

    if(_g->st_armor.oldnum != *_g->st_armor.num)
        return true;

    if(_g->w_faces.oldinum != *_g->w_faces.inum)
        return true;
	
	// ammo
    for(int8_t i=0; i<4; i++)
    {
        if(_g->w_ammo[i].oldnum != *_g->w_ammo[i].num)
            return true;
		if(_g->w_maxammo[i].oldnum != *_g->w_maxammo[i].num)
            return true;
    }

    // weapons owned
    for(int8_t i=0; i<6; i++)
    {
        if(_g->w_arms[i].oldinum != *_g->w_arms[i].inum)
            return true;
    }

    for(int8_t i = 0; i < 3; i++)
    {
        if(_g->w_keyboxes[i].oldinum != *_g->w_keyboxes[i].inum)
            return true;
    }

    return false;
}

void ST_Drawer(void)
{
    ST_doPaletteStuff();  // Do red-/gold-shifts from damage/items

    boolean needupdate = false;

    if(ST_NeedUpdate())
    {
        needupdate = true;
        _g->st_needrefresh = 2;
    }
    else if(_g->st_needrefresh)
    {
        needupdate = true;
    }

    if(needupdate)
    {
        ST_doRefresh();

        _g->st_needrefresh--;
    }
}



//
// ST_loadGraphics
//
// CPhipps - Loads graphics needed for status bar
//
static void ST_loadGraphics(void)
{
    int8_t  i, facenum;
    char namebuf[9];

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
		sprintf(namebuf, "STGANUM%d", i); //Special GBA Doom II Red Numbers ~Kippykip
        _g->tallnum[i] = W_GetNumForName(namebuf);

        sprintf(namebuf, "STYSNUM%d", i);
        _g->shortnum[i] = W_GetNumForName(namebuf);
    }

    // key cards
    for (i=0;i<NUMCARDS;i++)
    {
        sprintf(namebuf, "STKEYS%d", i);
        _g->keys[i] = W_GetNumForName(namebuf);
    }

    // arms ownership widgets
    for (i=0;i<6;i++)
    {
        sprintf(namebuf, "STGNUM%d", i+2);

        // gray #
        _g->arms[i][0] = W_GetNumForName(namebuf);

        // yellow #
        _g->arms[i][1] = _g->shortnum[i+2];
    }

    // face states
    facenum = 0;

    for (i=0;i<ST_NUMPAINFACES;i++)
    {
        for (int8_t j=0;j<ST_NUMSTRAIGHTFACES;j++)
        {
            sprintf(namebuf, "STFST%d%d", i, j);
            _g->faces[facenum++] = W_GetNumForName(namebuf);
        }
        sprintf(namebuf, "STFTR%d0", i);	// turn right
        _g->faces[facenum++] = W_GetNumForName(namebuf);
        sprintf(namebuf, "STFTL%d0", i);	// turn left
        _g->faces[facenum++] = W_GetNumForName(namebuf);
        sprintf(namebuf, "STFOUCH%d", i);	// ouch!
        _g->faces[facenum++] = W_GetNumForName(namebuf);
        sprintf(namebuf, "STFEVL%d", i);	// evil grin ;)
        _g->faces[facenum++] = W_GetNumForName(namebuf);
        sprintf(namebuf, "STFKILL%d", i);	// pissed off
        _g->faces[facenum++] = W_GetNumForName(namebuf);
    }
    _g->faces[facenum++] = W_GetNumForName("STFGOD0");
    _g->faces[facenum++] = W_GetNumForName("STFDEAD0");
}

static void ST_loadData(void)
{
  ST_loadGraphics();
}

static void ST_initData(void)
{
    int8_t i;

    _g->st_faceindex = 0;
    _g->st_palette = -1;

    st_oldhealth = -1;

    for (i=0;i<NUMWEAPONS;i++)
        _g->oldweaponsowned[i] = _g->player.weaponowned[i];

    for (i=0;i<3;i++)
        _g->keyboxes[i] = -1;
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
static void STlib_initMultIcon(st_multicon_t* i, int32_t x, int32_t y, int16_t* il, int32_t* inum)
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
static void STlib_initNum(st_number_t* n, int32_t x, int32_t y, int16_t* pl, int32_t* num, int32_t width)
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
    STlib_initNum(&_g->w_ready, ST_AMMOX, ST_AMMOY, _g->tallnum, &_g->player.ammo[weaponinfo[_g->player.readyweapon].ammo], ST_AMMOWIDTH);

    // health percentage
    STlib_initNum(&_g->st_health, ST_HEALTHX, ST_HEALTHY, _g->tallnum, &_g->player.health, ST_HEALTHWIDTH);
					  
    // armor percentage - should be colored later
    STlib_initNum(&_g->st_armor, ST_ARMORX, ST_ARMORY, _g->tallnum, &_g->player.armorpoints, ST_ARMORWIDTH);

    // weapons owned
    for(int8_t i = 0; i < 6; i++)
    {
        STlib_initMultIcon(&_g->w_arms[i], ST_ARMSX+(i%3)*ST_ARMSXSPACE, ST_ARMSY+(i/3)*ST_ARMSYSPACE, _g->arms[i], (int32_t*) &_g->player.weaponowned[i+1]);
    }
	
    // keyboxes 0-2
    STlib_initMultIcon(&_g->w_keyboxes[0], ST_KEY0X, ST_KEY0Y, _g->keys, &_g->keyboxes[0]);
    STlib_initMultIcon(&_g->w_keyboxes[1], ST_KEY1X, ST_KEY1Y, _g->keys, &_g->keyboxes[1]);
    STlib_initMultIcon(&_g->w_keyboxes[2], ST_KEY2X, ST_KEY2Y, _g->keys, &_g->keyboxes[2]);			
			
	// ammo count (all four kinds)
	STlib_initNum(&_g->w_ammo[0], ST_AMMO0X, ST_AMMO0Y, _g->shortnum, &_g->player.ammo[0], ST_AMMO0WIDTH);
	STlib_initNum(&_g->w_ammo[1], ST_AMMO1X, ST_AMMO1Y, _g->shortnum, &_g->player.ammo[1], ST_AMMO1WIDTH);
	STlib_initNum(&_g->w_ammo[2], ST_AMMO2X, ST_AMMO2Y, _g->shortnum, &_g->player.ammo[2], ST_AMMO2WIDTH);
	STlib_initNum(&_g->w_ammo[3], ST_AMMO3X, ST_AMMO3Y, _g->shortnum, &_g->player.ammo[3], ST_AMMO3WIDTH);

	// max ammo count (all four kinds)
	STlib_initNum(&_g->w_maxammo[0], ST_MAXAMMO0X, ST_MAXAMMO0Y, _g->shortnum, &_g->player.maxammo[0], ST_MAXAMMO0WIDTH);
	STlib_initNum(&_g->w_maxammo[1], ST_MAXAMMO1X, ST_MAXAMMO1Y, _g->shortnum, &_g->player.maxammo[1], ST_MAXAMMO1WIDTH);
	STlib_initNum(&_g->w_maxammo[2], ST_MAXAMMO2X, ST_MAXAMMO2Y, _g->shortnum, &_g->player.maxammo[2], ST_MAXAMMO2WIDTH);
	STlib_initNum(&_g->w_maxammo[3], ST_MAXAMMO3X, ST_MAXAMMO3Y, _g->shortnum, &_g->player.maxammo[3], ST_MAXAMMO3WIDTH);
			
    // faces
    STlib_initMultIcon(&_g->w_faces, ST_FACESX, ST_FACESY, _g->faces, &_g->st_faceindex);
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
