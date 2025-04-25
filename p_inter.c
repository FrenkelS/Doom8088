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
 *      Handling interactions (i.e., collisions).
 *
 *-----------------------------------------------------------------------------*/

#include "d_player.h"
#include "d_englsh.h"
#include "m_random.h"
#include "am_map.h"
#include "r_main.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_tick.h"
#include "i_system.h"

#include "p_inter.h"
#include "p_enemy.h"

#include "globdata.h"

#include "p_inter.h"

#define BONUSADD        6

// Ty 03/07/98 - add deh externals
// Maximums and such were hardcoded values.  Need to externalize those for
// dehacked support (and future flexibility).  Most var names came from the key
// strings used in dehacked.

       const int16_t initial_health    = 100;
       const int16_t initial_bullets   = 50;
static const int16_t maxhealth         = 100; // was MAXHEALTH as a #define, used only in this module
static const int16_t max_armor         = 200;
static const int16_t green_armor_class = 1;  // these are involved with armortype below
static const int16_t blue_armor_class  = 2;
static const int16_t max_soul          = 200;
static const int16_t soul_health       = 100;
       const int16_t god_health        = 100;   // these are used in cheats (see st_stuff.c)
       const int16_t idfa_armor        = 200;
       const int16_t idfa_armor_class  = 2;

       const int16_t bfgcells          = 40;      // used in p_pspr.c
// Ty 03/07/98 - end deh externals

// a weapon is found with two clip loads,
// a big item has five clip loads
       const int16_t maxammo[NUMAMMO] = {200, 50, 50, 300};
static const int8_t clipammo[NUMAMMO] = { 10,  4,  1,  20};

//
// GET STUFF
//

//
// P_GiveAmmo
// Num is the number of clip loads,
// not the individual count (0= 1/2 clip).
// Returns false if the ammo can't be picked up at all
//

static boolean P_GiveAmmo(player_t *player, ammotype_t ammo, int16_t num)
{
  int16_t oldammo;

  if (ammo == am_noammo)
    return false;

#ifdef RANGECHECK
  if (ammo < 0 || ammo > NUMAMMO)
    I_Error ("P_GiveAmmo: bad type %i", ammo);
#endif

  if ( player->ammo[ammo] == player->maxammo[ammo]  )
    return false;

  if (num)
    num *= clipammo[ammo];
  else
    num = clipammo[ammo]/2;

  // give double ammo in trainer mode, you'll need in nightmare
  if (_g_gameskill == sk_baby || _g_gameskill == sk_nightmare)
    num <<= 1;

  oldammo = player->ammo[ammo];
  player->ammo[ammo] += num;

  if (player->ammo[ammo] > player->maxammo[ammo])
    player->ammo[ammo] = player->maxammo[ammo];

  // If non zero ammo, don't change up weapons, player was lower on purpose.
  if (oldammo)
    return true;

  // We were down to zero, so select a new weapon.
  // Preferences are not user selectable.

  switch (ammo)
    {
    case am_clip:
      if (player->readyweapon == wp_fist) {
        if (player->weaponowned[wp_chaingun])
          player->pendingweapon = wp_chaingun;
        else
          player->pendingweapon = wp_pistol;
      }
      break;

    case am_shell:
      if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
        if (player->weaponowned[wp_shotgun])
          player->pendingweapon = wp_shotgun;
      break;

      case am_cell:
        if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
          if (player->weaponowned[wp_plasma])
            player->pendingweapon = wp_plasma;
      break;

      case am_misl:
        if (player->readyweapon == wp_fist)
          if (player->weaponowned[wp_missile])
            player->pendingweapon = wp_missile;
    default:
      break;
    }
  return true;
}

//
// P_GiveWeapon
// The weapon name may have a MF_DROPPED flag ored in.
//

static boolean P_GiveWeapon(player_t *player, weapontype_t weapon, boolean dropped)
{
  boolean gaveammo;
  boolean gaveweapon;

  if (weaponinfo[weapon].ammo != am_noammo)
    {
      // give one clip with a dropped weapon,
      // two clips with a found weapon
      gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, dropped ? 1 : 2);
    }
  else
    gaveammo = false;

  if (player->weaponowned[weapon])
    gaveweapon = false;
  else
    {
      gaveweapon = true;
      player->weaponowned[weapon] = true;
      player->pendingweapon = weapon;
    }
  return gaveweapon || gaveammo;
}

//
// P_GiveBody
// Returns false if the body isn't needed at all
//

static boolean P_GiveBody(player_t *player, int16_t num)
{
  if (player->health >= maxhealth)
    return false; // Ty 03/09/98 externalized MAXHEALTH to maxhealth
  player->health += num;
  if (player->health > maxhealth)
    player->health = maxhealth;
  player->mo->health = player->health;
  return true;
}

//
// P_GiveArmor
// Returns false if the armor is worse
// than the current armor.
//

static boolean P_GiveArmor(player_t *player, int16_t armortype)
{
  int16_t hits = armortype*100;
  if (player->armorpoints >= hits)
    return false;   // don't pick up
  player->armortype = armortype;
  player->armorpoints = hits;
  return true;
}

//
// P_GiveCard
//

static void P_GiveCard(player_t *player, card_t card)
{
  if (player->cards[card])
    return;
  player->bonuscount = BONUSADD;
  player->cards[card] = 1;
}

//
// P_GivePower
//
// Rewritten by Lee Killough
//

boolean P_GivePower(player_t *player, powertype_t power)
{
  static const int16_t tics[NUMPOWERS] = {
    INVULNTICS, 1 /* strength */, INVISTICS,
    IRONTICS, 1 /* allmap */, INFRATICS,
   };

  switch (power)
    {
      case pw_invisibility:
        player->mo->flags |= MF_SHADOW;
        break;
      case pw_allmap:
        if (player->powers[pw_allmap])
          return false;
        break;
      case pw_strength:
        P_GiveBody(player,100);
        break;
    }

  // Unless player has infinite duration cheat, set duration (killough)

  if (player->powers[power] >= 0)
    player->powers[power] = tics[power];
  return true;
}

//
// P_TouchSpecialThing
//

void P_TouchSpecialThing(mobj_t __far* special, mobj_t __far* toucher)
{
  player_t *player;
  int16_t      i;
  sfxenum_t      sound;
  fixed_t  delta = special->z - toucher->z;

  if (delta > toucher->height || delta < -8*FRACUNIT)
    return;        // out of reach

  sound = sfx_itemup;
  player = P_MobjIsPlayer(toucher);

  // Dead thing touching.
  // Can happen with a sliding player corpse.
  if (toucher->health <= 0)
    return;

    // Identify by sprite.
  switch (special->sprite)
    {
      // armor
    case SPR_ARM1:
      if (!P_GiveArmor (player, green_armor_class))
        return;
      player->message = GOTARMOR; // Ty 03/22/98 - externalized
      break;

    case SPR_ARM2:
      if (!P_GiveArmor (player, blue_armor_class))
        return;
      player->message = GOTMEGA; // Ty 03/22/98 - externalized
      break;


        // bonus items
    case SPR_BON1:
      player->health++;               // can go over 100%
      if (player->health > (maxhealth * 2))
        player->health = (maxhealth * 2);
      player->mo->health = player->health;
      player->message = GOTHTHBONUS; // Ty 03/22/98 - externalized
      break;

    case SPR_BON2:
      player->armorpoints++;          // can go over 100%
      if (player->armorpoints > max_armor)
        player->armorpoints = max_armor;
      if (!player->armortype)
        player->armortype = green_armor_class;
      player->message = GOTARMBONUS; // Ty 03/22/98 - externalized
      break;

    case SPR_SOUL:
      player->health += soul_health;
      if (player->health > max_soul)
        player->health = max_soul;
      player->mo->health = player->health;
      player->message = GOTSUPER; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;


        // cards
        // leave cards for everyone
    case SPR_BKEY:
      if (!player->cards[it_bluecard])
        player->message = GOTBLUECARD; // Ty 03/22/98 - externalized
      P_GiveCard (player, it_bluecard);
        break;

    case SPR_YKEY:
      if (!player->cards[it_yellowcard])
        player->message = GOTYELWCARD; // Ty 03/22/98 - externalized
      P_GiveCard (player, it_yellowcard);
        break;

    case SPR_RKEY:
      if (!player->cards[it_redcard])
        player->message = GOTREDCARD; // Ty 03/22/98 - externalized
      P_GiveCard (player, it_redcard);
        break;


      // medikits, heals
    case SPR_STIM:
      if (!P_GiveBody (player, 10))
        return;
      player->message = GOTSTIM; // Ty 03/22/98 - externalized
      break;

    case SPR_MEDI:
      if (!P_GiveBody (player, 25))
        return;

      if (player->health < 50) // cph - 25 + the 25 just added, thanks to Quasar for reporting this bug
        player->message = GOTMEDINEED; // Ty 03/22/98 - externalized
      else
        player->message = GOTMEDIKIT; // Ty 03/22/98 - externalized
      break;


      // power ups
    case SPR_PINS:
      if (!P_GivePower (player, pw_invisibility))
        return;
      player->message = GOTINVIS; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_SUIT:
      if (!P_GivePower (player, pw_ironfeet))
        return;
      player->message = GOTSUIT; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_PMAP:
      if (!P_GivePower (player, pw_allmap))
        return;
      player->message = GOTMAP; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;

    case SPR_PVIS:
      if (!P_GivePower (player, pw_infrared))
        return;
      player->message = GOTVISOR; // Ty 03/22/98 - externalized
      sound = sfx_getpow;
      break;


      // ammo
    case SPR_CLIP:
      if (special->flags & MF_DROPPED)
        {
          if (!P_GiveAmmo (player,am_clip,0))
            return;
        }
      else
        {
          if (!P_GiveAmmo (player,am_clip,1))
            return;
        }
      player->message = GOTCLIP; // Ty 03/22/98 - externalized
      break;

    case SPR_AMMO:
      if (!P_GiveAmmo (player, am_clip,5))
        return;
      player->message = GOTCLIPBOX; // Ty 03/22/98 - externalized
      break;

    case SPR_ROCK:
      if (!P_GiveAmmo (player, am_misl,1))
        return;
      player->message = GOTROCKET; // Ty 03/22/98 - externalized
      break;

    case SPR_BROK:
      if (!P_GiveAmmo (player, am_misl,5))
        return;
      player->message = GOTROCKBOX; // Ty 03/22/98 - externalized
      break;

    case SPR_SHEL:
      if (!P_GiveAmmo (player, am_shell,1))
        return;
      player->message = GOTSHELLS; // Ty 03/22/98 - externalized
      break;

    case SPR_SBOX:
      if (!P_GiveAmmo (player, am_shell,5))
        return;
      player->message = GOTSHELLBOX; // Ty 03/22/98 - externalized
      break;

    case SPR_BPAK:
      if (!player->backpack)
        {
          for (i=0 ; i<NUMAMMO ; i++)
            player->maxammo[i] *= 2;
          player->backpack = true;
        }
      for (i=0 ; i<NUMAMMO ; i++)
        P_GiveAmmo (player, i, 1);
      player->message = GOTBACKPACK; // Ty 03/22/98 - externalized
      break;


        // weapons
    case SPR_MGUN:
      if (!P_GiveWeapon (player, wp_chaingun, (special->flags&MF_DROPPED)!=0) )
        return;
      player->message = GOTCHAINGUN; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_CSAW:
      if (!P_GiveWeapon (player, wp_chainsaw, false) )
        return;
      player->message = GOTCHAINSAW; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_LAUN:
      if (!P_GiveWeapon (player, wp_missile, false) )
        return;
      player->message = GOTLAUNCHER; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;

    case SPR_SHOT:
      if (!P_GiveWeapon (player, wp_shotgun, (special->flags&MF_DROPPED)!=0 ) )
        return;
      player->message = GOTSHOTGUN; // Ty 03/22/98 - externalized
      sound = sfx_wpnup;
      break;


    default:
      I_Error ("P_SpecialThing: Unknown gettable thing");
    }

  if (special->flags & MF_COUNTITEM)
    player->itemcount++;
  P_RemoveMobj (special);
  player->bonuscount += BONUSADD;

  /* cph 20028/10 - for old-school DM addicts, allow old behavior
   * where only consoleplayer's pickup sounds are heard */
  // displayplayer, not consoleplayer, for viewing multiplayer demos
  if (player == &_g_player)
    S_StartSound (player->mo, sound | PICKUP_SOUND);   // killough 4/25/98
}

//
// KillMobj
//
static void P_KillMobj(mobj_t __far* source, mobj_t __far* target)
{
  mobjtype_t item;
  mobj_t     __far* mo;

  target->flags &= ~MF_SHOOTABLE;
  target->flags &= ~MF_NOGRAVITY;
  target->flags |= MF_CORPSE|MF_DROPOFF;
  target->height >>= 2;


  if (!((target->flags ^ MF_COUNTKILL) & MF_COUNTKILL))
    _g_totallive--;

  if (source && P_MobjIsPlayer(source))
    {
      // count for intermission
      if (target->flags & MF_COUNTKILL)
        P_MobjIsPlayer(source)->killcount++;
    }
  else if (target->flags & MF_COUNTKILL)
  { /* Add to kills tally */

      // count all monster deaths,
      // even those caused by other monsters
      _g_player.killcount++;

  }

  if (P_MobjIsPlayer(target))
    {
      target->flags &= ~MF_SOLID;
      _g_player.playerstate = PST_DEAD;
      P_DropWeapon (&_g_player);

      if (automapmode & am_active)
        AM_Stop();    // don't die in auto map; switch view prior to dying
    }

  if (target->health < -mobjinfo[target->type].spawnhealth && mobjinfo[target->type].xdeathstate)
    P_SetMobjState (target, mobjinfo[target->type].xdeathstate);
  else
    P_SetMobjState (target, mobjinfo[target->type].deathstate);

  target->tics -= P_Random()&3;

  if (target->tics < 1)
    target->tics = 1;

  // Drop stuff.
  // This determines the kind of object spawned
  // during the death frame of a thing.

  if( (_g_player.cheats & CF_ENEMY_ROCKETS) && (target->type >= MT_POSSESSED) && (target->type <= MT_BRUISERSHOT) )
  {
    item = MT_MISC27; //Everyone drops a rocket launcher.
  }
  else
  {
      switch (target->type)
      {
      case MT_POSSESSED:
          item = MT_CLIP;
          break;

      case MT_SHOTGUY:
          item = MT_SHOTGUN;
          break;

      default:
          return;
      }
  }

  mo = P_SpawnMobj (target->x,target->y,ONFLOORZ, item);
  mo->flags |= MF_DROPPED;    // special versions of items
}

//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//

void P_DamageMobj(mobj_t __far* target, mobj_t __far* inflictor, mobj_t __far* source, int16_t damage)
{
  player_t *player;
  boolean justhit = false;          /* killough 11/98 */

  /* killough 8/31/98: allow bouncers to take damage */
  if (!(target->flags & (MF_SHOOTABLE)))
    return; // shouldn't happen...

  if (target->health <= 0)
    return;

  player = P_MobjIsPlayer(target);
  if (player && _g_gameskill == sk_baby)
    damage >>= 1;   // take half damage in trainer mode

  // Some close combat weapons should not
  // inflict thrust and push the victim out of reach,
  // thus kick away unless using the chainsaw.

  if (inflictor && !(target->flags & MF_NOCLIP) &&
      (!source || !P_MobjIsPlayer(source) ||
       P_MobjIsPlayer(source)->readyweapon != wp_chainsaw))
    {
      angle_t ang = R_PointToAngle2 (inflictor->x, inflictor->y,
                                      target->x,    target->y);

      fixed_t thrust = damage*(FRACUNIT>>3)*100/mobjinfo[target->type].mass;

      // make fall forwards sometimes
      if ( target->health < damage && damage < 40
           && target->z - inflictor->z > 64*FRACUNIT
           && P_Random() & 1)
        {
          ang += ANG180;
          thrust *= 4;
        }

      ang >>= ANGLETOFINESHIFT;
      target->momx += FixedMulAngle(thrust, finecosine(ang));
      target->momy += FixedMulAngle(thrust, finesine(  ang));
    }

  // player specific
  if (player)
    {
      // end of game hell hack
      if (target->subsector->sector->special == 11 && damage >= target->health)
        damage = target->health - 1;

      // Below certain threshold,
      // ignore damage in GOD mode, or with INVUL power.
      // killough 3/26/98: make god mode 100% god mode in non-compat mode

      if ((damage < 1000 || ((player->cheats&CF_GODMODE))) &&
          (player->cheats&CF_GODMODE || player->powers[pw_invulnerability]))
        return;

      if (player->armortype)
        {
          int16_t saved = player->armortype == 1 ? damage/3 : damage/2;
          if (player->armorpoints <= saved)
            {
              // armor is used up
              saved = player->armorpoints;
              player->armortype = 0;
            }
          player->armorpoints -= saved;
          damage -= saved;
        }

      player->health -= damage;       // mirror mobj health here for Dave
      if (player->health < 0)
        player->health = 0;

      player->attacker = source;
      player->damagecount += damage;  // add damage after armor / invuln

      if (player->damagecount > 100)
        player->damagecount = 100;  // teleport stomp does 10k points...
    }

  // do the damage
  target->health -= damage;
  if (target->health <= 0)
    {
      P_KillMobj (source, target);
      return;
    }

      if (player)
        target->target = source;

  if (P_Random () < mobjinfo[target->type].painchance)
  {
      justhit = true;

    P_SetMobjState(target, mobjinfo[target->type].painstate);
  }

  target->reactiontime = 0;           // we're awake now...

  /* killough 9/9/98: cleaned up, made more consistent: */

  if (source && source != target && !target->threshold)
    {
      /* if not intent on another player, chase after this one
       *
       * killough 2/15/98: remember last enemy, to prevent
       * sleeping early; 2/21/98: Place priority on players
       * killough 9/9/98: cleaned up, made more consistent:
       */

      if (!target->lastenemy || target->lastenemy->health <= 0 || target->target != source) // remember last enemy - killough
       target->lastenemy = target->target;

      target->target    = source;
      target->threshold = BASETHRESHOLD;
      if (target->state == &states[mobjinfo[target->type].spawnstate]
          && mobjinfo[target->type].seestate != S_NULL)
        P_SetMobjState (target, mobjinfo[target->type].seestate);
    }

  if (justhit)
    target->flags |= MF_JUSTHIT;    // fight back!
}
