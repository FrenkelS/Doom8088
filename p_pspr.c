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
 *      Weapon sprite animation, weapon objects.
 *      Action functions for weapons.
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
#include "r_main.h"
#include "p_map.h"
#include "p_inter.h"
#include "p_pspr.h"
#include "p_enemy.h"
#include "m_random.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_event.h"

#include "globdata.h"

#define LOWERSPEED   (FRACUNIT*6)
#define RAISESPEED   (FRACUNIT*6)
#define WEAPONBOTTOM (FRACUNIT*128)
#define WEAPONTOP    (FRACUNIT*32)

#define BFGCELLS bfgcells        /* Ty 03/09/98 externalized in p_inter.c */

//
// P_SetPsprite
//

static void P_SetPsprite(player_t *player, psprnum_t position, statenum_t stnum)
{
  pspdef_t *psp = &player->psprites[position];

  do
    {
      const state_t *state;

      if (!stnum)
        {
          // object removed itself
          psp->state = NULL;
          break;
        }

      state = &states[stnum];
      psp->state = state;
      psp->tics = state->tics;        // could be 0

      // Call action routine.
      // Modified handling.
      if (state->action)
        {
          state->action(player, psp);
          if (!psp->state)
            break;
        }
      stnum = psp->state->nextstate;
    }
  while (!psp->tics);     // an initial state of 0 could cycle through
}

//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//

static void P_BringUpWeapon(player_t *player)
{
  statenum_t newstate;

  if (player->pendingweapon == wp_nochange)
    player->pendingweapon = player->readyweapon;

  if (player->pendingweapon == wp_chainsaw)
    S_StartSound (player->mo, sfx_sawup);

  newstate = weaponinfo[player->pendingweapon].upstate;

  player->pendingweapon = wp_nochange;
  // killough 12/98: prevent pistol from starting visibly at bottom of screen:
  player->psprites[ps_weapon].sy =
WEAPONBOTTOM+FRACUNIT*2;

  P_SetPsprite(player, ps_weapon, newstate);
}

// The first set is where the weapon preferences from             // killough,
// default.cfg are stored. These values represent the keys used   // phares
// in DOOM2 to bring up the weapon, i.e. 6 = plasma gun. These    //    |
// are NOT the wp_* constants.                                    //    V

static const weapontype_t weapon_preferences[NUMWEAPONS+1] =
{
    6, 9, 4, 3, 2, 8, 5, 7, 1,  // !compatibility preferences
};

// P_SwitchWeapon checks current ammo levels and gives you the
// most preferred weapon with ammo. It will not pick the currently
// raised weapon. When called from P_CheckAmmo this won't matter,
// because the raised weapon has no ammo anyway. When called from
// G_BuildTiccmd you want to toggle to a different weapon regardless.

weapontype_t P_SwitchWeapon(player_t *player)
{
  const weapontype_t *prefer = &weapon_preferences[0]; // killough 3/22/98
  weapontype_t currentweapon = player->readyweapon;
  weapontype_t newweapon = currentweapon;
  int16_t i = NUMWEAPONS+1;   // killough 5/2/98

  // killough 2/8/98: follow preferences and fix BFG/SSG bugs

  do
    switch (*prefer++)
      {
      case 1:
        if (!player->powers[pw_strength])      // allow chainsaw override
          break;
      case 0:
        newweapon = wp_fist;
        break;
      case 2:
        if (player->ammo[am_clip])
          newweapon = wp_pistol;
        break;
      case 3:
        if (player->weaponowned[wp_shotgun] && player->ammo[am_shell])
          newweapon = wp_shotgun;
        break;
      case 4:
        if (player->weaponowned[wp_chaingun] && player->ammo[am_clip])
          newweapon = wp_chaingun;
        break;
      case 5:
        if (player->weaponowned[wp_missile] && player->ammo[am_misl])
          newweapon = wp_missile;
        break;
      case 8:
        if (player->weaponowned[wp_chainsaw])
          newweapon = wp_chainsaw;
        break;
      }
  while (newweapon==currentweapon && --i);          // killough 5/2/98
  return newweapon;
}


static weapontype_t P_CheckCanSwitchWeapon(weapontype_t weapon, player_t* player)
{
    switch(weapon)
    {
        case wp_fist:
        {
            return wp_fist;
        }
        break;

        case wp_pistol:
        {
            if (player->ammo[am_clip])
                return wp_pistol;
        }
        break;

        case wp_shotgun:
        {
            if (player->ammo[am_shell])
                return wp_shotgun;
        }
        break;

        case wp_chaingun:
        {
            if (player->ammo[am_clip])
                return wp_chaingun;
        }
        break;

        case wp_missile:
        {
            if (player->ammo[am_misl])
                return wp_missile;
        }
        break;

        case wp_plasma:
        {
            if (player->ammo[am_cell])
                return wp_plasma;
        }
        break;

        case wp_chainsaw:
        {
            return wp_chainsaw;
        }
        break;
    }

    return wp_nochange;
}


weapontype_t P_WeaponCycleUp(player_t *player)
{
    weapontype_t w = player->readyweapon;

    for(int16_t i = 0; i < NUMWEAPONS; i++)
    {
        w++;
        if(w >= NUMWEAPONS)
            w = 0;
		
		//Dumb hack to fix weapon order	to be like PSXDoom ~Kippykip
		switch(w)
		{
			case wp_chaingun:
			{
				w = wp_supershotgun;
			}
			break;
			case wp_fist:
			{
				w = wp_chaingun;
			}
			break;
			case wp_chainsaw:
			{
				w = wp_fist;
			}
			break;
			case wp_pistol:
			{
				w = wp_chainsaw;
			}
			break;
			case wp_supershotgun:
			{
				w = wp_pistol;
			}
			break;
		}

        if(!player->weaponowned[w])
            continue;

        if(P_CheckCanSwitchWeapon(w, player) != wp_nochange)
            return w;

    }

    return player->readyweapon;
}

weapontype_t P_WeaponCycleDown(player_t *player)
{
    weapontype_t w = player->readyweapon;

    for (int16_t i = 0; i < NUMWEAPONS; i++)
    {
        if (w == 0)
            w = NUMWEAPONS-1;
        else
            w--;

		//Dumb hack to fix weapon order	to be like PSXDoom ~Kippykip
		switch(w)
		{
			case wp_shotgun:
			{
				w = wp_supershotgun;
			}
			break;
			case wp_chainsaw:
			{
				w = wp_shotgun;
			}
			break;
			case wp_fist:
			{
				w = wp_chainsaw;
			}
			break;
			case wp_bfg:
			{
				w = wp_fist;
			}
			break;
			case wp_supershotgun:
			{
				w = wp_bfg;
			}
			break;
		}

        if(!player->weaponowned[w])
            continue;

        if(P_CheckCanSwitchWeapon(w, player) != wp_nochange)
            return w;
    }

    return player->readyweapon;
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
// (only in demo_compatibility mode -- killough 3/22/98)
//

boolean P_CheckAmmo(player_t *player)
{
  ammotype_t ammo = weaponinfo[player->readyweapon].ammo;
  int16_t count = 1;  // Regular

  if (player->readyweapon == wp_bfg)  // Minimal amount for one shot varies.
    count = BFGCELLS;
  else
    if (player->readyweapon == wp_supershotgun)        // Double barrel.
      count = 2;

  // Some do not need ammunition anyway.
  // Return if current ammunition sufficient.

  if (ammo == am_noammo || player->ammo[ammo] >= count)
    return true;

  return false;
}


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

static void P_RecursiveSound(sector_t __far* sec, int16_t soundblocks, mobj_t __far* soundtarget)
{
  int16_t i;

  // wake up all monsters in this sector
  if (sec->validcount == validcount && sec->soundtraversed <= soundblocks+1)
    return;             // already flooded

  sec->validcount     = validcount;
  sec->soundtraversed = soundblocks+1;
  sec->soundtarget    = soundtarget;

  for (i=0; i<sec->linecount; i++)
    {
      sector_t __far* other;
      const line_t __far* check = sec->lines[i];

      if (!(check->flags & ML_TWOSIDED))
        continue;

      P_LineOpening(check);

      if (_g_openrange <= 0)
        continue;       // closed door

      other=_g_sides[check->sidenum[_g_sides[check->sidenum[0]].sector==sec]].sector;

      if (!(check->flags & ML_SOUNDBLOCK))
        P_RecursiveSound(other, soundblocks, soundtarget);
      else
        if (!soundblocks)
          P_RecursiveSound(other, 1, soundtarget);
    }
}

//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
static void P_NoiseAlert(mobj_t __far* emitter)
{
  validcount++;
  P_RecursiveSound(emitter->subsector->sector, 0, emitter);
}


//
// P_FireWeapon.
//

static void P_FireWeapon(player_t *player)
{
  statenum_t newstate;

  if (!P_CheckAmmo(player))
    return;

  P_SetMobjState(player->mo, S_PLAY_ATK1);
  newstate = weaponinfo[player->readyweapon].atkstate;
  P_SetPsprite(player, ps_weapon, newstate);
  P_NoiseAlert(player->mo);
}

//
// P_DropWeapon
// Player died, so put the weapon away.
//

void P_DropWeapon(player_t *player)
{
  P_SetPsprite(player, ps_weapon, weaponinfo[player->readyweapon].downstate);
}

//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//

void A_WeaponReady(player_t *player, pspdef_t *psp)
{
  // get out of attack state
  if (player->mo->state == &states[S_PLAY_ATK1]
      || player->mo->state == &states[S_PLAY_ATK2] )
    P_SetMobjState(player->mo, S_PLAY);

  if (player->readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
    S_StartSound(player->mo, sfx_sawidl);

  // check for change
  //  if player is dead, put the weapon away

  if (player->pendingweapon != wp_nochange || !player->health)
    {
      // change weapon (pending weapon should already be validated)
      statenum_t newstate = weaponinfo[player->readyweapon].downstate;
      P_SetPsprite(player, ps_weapon, newstate);
      return;
    }

  // check for fire
  //  the missile launcher and bfg do not auto fire

  if (player->cmd.buttons & BT_ATTACK)
    {
      if (!player->attackdown || (player->readyweapon != wp_missile &&
                                  player->readyweapon != wp_bfg))
        {
          player->attackdown = true;
          P_FireWeapon(player);
          return;
        }
    }
  else
    player->attackdown = false;

  // bob the weapon based on movement speed
  {
    int16_t angle = _g_leveltime;
    angle = (angle * 128) & FINEMASK;

    int16_t ahw = player->bob >> FRACBITS;
    fixed_t cos = finecosine(angle);
    uint16_t blw = cos;
     int16_t bhw = cos >> FRACBITS;

    uint32_t hl = (uint32_t) ahw * blw;

    psp->sx = 1 + ((player->bob * bhw) >> FRACBITS) + (hl >> FRACBITS);
    angle &= FINEANGLES/2-1;
    psp->sy = WEAPONTOP + FixedMulAngle(player->bob, finesine(angle));
  }
}

//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//

void A_ReFire(player_t *player, pspdef_t *psp)
{
	UNUSED(psp);

	// check for fire
	//  (if a weaponchange is pending, let it go through instead)

	if ((player->cmd.buttons & BT_ATTACK) && player->pendingweapon == wp_nochange && player->health)
	{
		player->refire++;
		P_FireWeapon(player);
	}
	else
	{
		player->refire = 0;
		P_CheckAmmo(player);
	}
}


//
// A_Lower
// Lowers current weapon,
//  and changes weapon at bottom.
//

void A_Lower(player_t *player, pspdef_t *psp)
{
  psp->sy += LOWERSPEED;

  // Is already down.
  if (psp->sy < WEAPONBOTTOM)
    return;

  // Player is dead.
  if (player->playerstate == PST_DEAD)
    {
      psp->sy = WEAPONBOTTOM;
      return;      // don't bring weapon back up
    }

  // The old weapon has been lowered off the screen,
  // so change the weapon and start raising it

  if (!player->health)
    {      // Player is dead, so keep the weapon off screen.
      P_SetPsprite(player,  ps_weapon, S_NULL);
      return;
    }

  player->readyweapon = player->pendingweapon;

  P_BringUpWeapon(player);
}

//
// A_Raise
//

void A_Raise(player_t *player, pspdef_t *psp)
{
  statenum_t newstate;

  psp->sy -= RAISESPEED;

  if (psp->sy > WEAPONTOP)
    return;

  psp->sy = WEAPONTOP;

  // The weapon has been raised all the way,
  //  so change to the ready state.

  newstate = weaponinfo[player->readyweapon].readystate;

  P_SetPsprite(player, ps_weapon, newstate);
}


// Weapons now recoil, amount depending on the weapon.              // phares
//                                                                  //   |
// The P_SetPsprite call in each of the weapon firing routines      //   V
// was moved here so the recoil could be synched with the
// muzzle flash, rather than the pressing of the trigger.
// The BFG delay caused this to be necessary.

static void A_FireSomething(player_t* player,int16_t adder)
{
  P_SetPsprite(player, ps_flash,
               weaponinfo[player->readyweapon].flashstate+adder);
}

//
// A_GunFlash
//

void A_GunFlash(player_t *player, pspdef_t *psp)
{
	UNUSED(psp);

	P_SetMobjState(player->mo, S_PLAY_ATK2);

	A_FireSomething(player,0);                                      // phares
}

//
// WEAPON ATTACKS
//

//
// A_Punch
//

void A_Punch(player_t *player, pspdef_t *psp)
{
	angle_t	angle, t;
	fixed_t	slope;
	int16_t	damage = (P_Random()%10+1)<<1;

	UNUSED(psp);

	if (player->powers[pw_strength])
		damage *= 10;

	angle = player->mo->angle;

	// killough 5/5/98: remove dependence on order of evaluation:
	t = P_Random();
	angle += (t - P_Random())<<18;

	slope = P_AimLineAttack(player->mo, angle, MELEERANGE);

	P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);

	if (!_g_linetarget)
		return;

	S_StartSound(player->mo, sfx_punch);

	// turn to face target
	player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, _g_linetarget->x, _g_linetarget->y);
}

//
// A_Saw
//

void A_Saw(player_t *player, pspdef_t *psp)
{
	fixed_t slope;
	int16_t	damage = 2*(P_Random()%10+1);
	angle_t angle = player->mo->angle;
	// killough 5/5/98: remove dependence on order of evaluation:
	angle_t t = P_Random();
	angle += (t - P_Random())<<18;

	UNUSED(psp);

	// Use meleerange + 1 so that the puff doesn't skip the flash
	slope = P_AimLineAttack(player->mo, angle, MELEERANGE+1);

	P_LineAttack(player->mo, angle, MELEERANGE+1, slope, damage);

	if (!_g_linetarget)
	{
		S_StartSound(player->mo, sfx_sawful);
		return;
	}

	S_StartSound(player->mo, sfx_sawhit);

	// turn to face target
	angle = R_PointToAngle2(player->mo->x, player->mo->y, _g_linetarget->x, _g_linetarget->y);

	if (angle - player->mo->angle > ANG180) {
		if (angle - player->mo->angle < -ANG90/20)
			player->mo->angle = angle + ANG90/21;
		else
			player->mo->angle -= ANG90/20;
	} else {
		if (angle - player->mo->angle > ANG90/20)
			player->mo->angle = angle - ANG90/21;
		else
			player->mo->angle += ANG90/20;
	}

	player->mo->flags |= MF_JUSTATTACKED;
}

//
// A_FireMissile
//

void A_FireMissile(player_t *player, pspdef_t *psp)
{
	UNUSED(psp);

	S_StartSound(player->mo, sfx_rlaunc);
	player->ammo[weaponinfo[player->readyweapon].ammo]--;
	P_SpawnPlayerMissile(player->mo);
}


static fixed_t bulletslope;


//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//
static void P_BulletSlope(mobj_t __far* mo)
{
	angle_t an = mo->angle;    // see which target is to be aimed at

	bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT);
	if (!_g_linetarget)
		bulletslope = P_AimLineAttack(mo, an += 1L << 26, 16 * 64 * FRACUNIT);
	if (!_g_linetarget)
		bulletslope = P_AimLineAttack(mo, an -= 2L << 26, 16 * 64 * FRACUNIT);
}


//
// P_GunShot
//

static void P_GunShot(mobj_t __far* mo, boolean accurate)
{
  int16_t damage = 5*(P_Random()%3+1);
  angle_t angle = mo->angle;

  if (!accurate)
    {  // killough 5/5/98: remove dependence on order of evaluation:
      angle_t t = P_Random();
      angle += (t - P_Random())<<18;
    }

  P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);
}

//
// A_FirePistol
//

void A_FirePistol(player_t *player, pspdef_t *psp)
{
	UNUSED(psp);

	S_StartSound(player->mo, sfx_pistol);

	P_SetMobjState(player->mo, S_PLAY_ATK2);
	player->ammo[weaponinfo[player->readyweapon].ammo]--;

	A_FireSomething(player,0);                                      // phares
	P_BulletSlope(player->mo);
	P_GunShot(player->mo, !player->refire);
}

//
// A_FireShotgun
//

void A_FireShotgun(player_t *player, pspdef_t *psp)
{
	int16_t i;

	UNUSED(psp);

	S_StartSound(player->mo, sfx_shotgn);
	P_SetMobjState(player->mo, S_PLAY_ATK2);

	player->ammo[weaponinfo[player->readyweapon].ammo]--;

	A_FireSomething(player,0);                                      // phares

	P_BulletSlope(player->mo);

	for (i=0; i<7; i++)
		P_GunShot(player->mo, false);
}


//
// A_FireCGun
//

void A_FireCGun(player_t *player, pspdef_t *psp)
{
  if (player->ammo[weaponinfo[player->readyweapon].ammo])
    S_StartSound(player->mo, sfx_pistol);

  if (!player->ammo[weaponinfo[player->readyweapon].ammo])
    return;

  P_SetMobjState(player->mo, S_PLAY_ATK2);
  player->ammo[weaponinfo[player->readyweapon].ammo]--;

  A_FireSomething(player,psp->state - &states[S_CHAIN1]);           // phares

  P_BulletSlope(player->mo);

  P_GunShot(player->mo, !player->refire);
}

void A_Light0(player_t *player, pspdef_t *psp)
{
	UNUSED(psp);

	player->extralight = 0;
}

void A_Light1 (player_t *player, pspdef_t *psp)
{
	UNUSED(psp);

	player->extralight = 1;
}

void A_Light2 (player_t *player, pspdef_t *psp)
{
	UNUSED(psp);

	player->extralight = 2;
}


//
// P_SetupPsprites
// Called at start of level for each player.
//

void P_SetupPsprites(player_t *player)
{
  int16_t i;

  // remove all psprites
  for (i=0; i<NUMPSPRITES; i++)
    player->psprites[i].state = NULL;

  // spawn the gun
  player->pendingweapon = player->readyweapon;
  P_BringUpWeapon(player);
}

//
// P_MovePsprites
// Called every tic by player thinking routine.
//

void P_MovePsprites(player_t *player)
{
  pspdef_t *psp = player->psprites;
  psprnum_t i;

  // a null state means not active
  // drop tic count and possibly change state
  // a -1 tic count never changes

  for (i=0; i<NUMPSPRITES; i++, psp++)
    if (psp->state && psp->tics != -1 && !--psp->tics)
      P_SetPsprite(player, i, psp->state->nextstate);

  player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
  player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}
