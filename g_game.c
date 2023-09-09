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
 * DESCRIPTION:  none
 *  The original Doom description was none, basically because this file
 *  has everything. This ties up the game logic, linking the menu and
 *  input code to the underlying game by creating & respawning players,
 *  building game tics, calling the underlying thing logic.
 *
 *-----------------------------------------------------------------------------
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "d_net.h"
#include "f_finale.h"
#include "doomtype.h"
#include "m_menu.h"
#include "m_random.h"
#include "p_setup.h"
#include "p_tick.h"
#include "p_map.h"
#include "d_main.h"
#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "w_wad.h"
#include "r_main.h"
#include "p_map.h"
#include "s_sound.h"
#include "d_englsh.h"
#include "sounds.h"
#include "r_data.h"
#include "m_fixed.h"
#include "p_inter.h"
#include "g_game.h"
#include "i_system.h"

#include "globdata.h"


//
// controls (have defaults)
//

static const int32_t     key_right = KEYD_RIGHT;
static const int32_t     key_left = KEYD_LEFT;
static const int32_t     key_up = KEYD_UP;
static const int32_t     key_down = KEYD_DOWN;
const int32_t     key_menu_right = KEYD_RIGHT;                                      // phares 3/7/98
const int32_t     key_menu_left = KEYD_LEFT;                                       //     |
const int32_t     key_menu_up = KEYD_UP;                                         //     V
const int32_t     key_menu_down = KEYD_DOWN;
const int32_t     key_menu_escape = KEYD_START;                                     //     |
const int32_t     key_menu_enter = KEYD_A;                                      // phares 3/7/98
static const int32_t     key_strafeleft = KEYD_L;
static const int32_t     key_straferight = KEYD_R;
//Match Doom II GBA controls ~ Kippykip
const int32_t     key_fire = KEYD_B; 
const int32_t     key_use = KEYD_A;
const int32_t     key_escape = KEYD_START;                           // phares 4/13/98
const int32_t     key_enter = KEYD_A;
const int32_t     key_map_right = KEYD_RIGHT;
const int32_t     key_map_left = KEYD_LEFT;
const int32_t     key_map_up = KEYD_UP;
const int32_t     key_map_down = KEYD_DOWN;
const int32_t     key_map = KEYD_SELECT;
const int32_t     key_map_follow = KEYD_A;
const int32_t     key_map_zoomin = KEYD_R;
const int32_t     key_map_zoomout = KEYD_L;
                                          // phares

#define MAXPLMOVE   (forwardmove[1])
#define SLOWTURNTICS  6

static const fixed_t forwardmove[2] = {0x19, 0x32};
static const fixed_t sidemove[2]    = {0x18, 0x28};
static const fixed_t angleturn[3]   = {640, 1280, 320};  // + slow turn

static void G_DoReborn (void);
static void G_DoCompleted(void);
static void G_DoWorldDone(void);
static void G_DoLoadGame(void);
static void G_DoSaveGame (void);
static void G_DoNewGame (void);
static void G_DoPlayDemo(void);
static void G_InitNew(skill_t skill, int32_t map);
static void G_ReadDemoTiccmd (ticcmd_t* cmd);


typedef struct gba_save_data_t
{
    int32_t save_present;
    skill_t gameskill;
    int32_t gamemap;
    int32_t totalleveltimes;

    int32_t weaponowned[NUMWEAPONS];
    int32_t ammo[NUMAMMO];
    int32_t maxammo[NUMAMMO];
} gba_save_data_t;


typedef struct gba_save_settings_t
{
    uint32_t cookie;
    uint32_t alwaysRun;
    uint32_t gamma;
    uint32_t showMessages;
    uint32_t musicVolume;
    uint32_t soundVolume;
	uint32_t highDetail;

} gba_save_settings_t;

static const uint32_t settings_cookie = 0xbaddead1;

static const uint32_t settings_sram_offset = sizeof(gba_save_data_t) * 8;

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
static inline int8_t fudgef(int8_t b)
{
    static int32_t c;
    if (!b) return b;
    if (++c & 0x1f) return b;
    b |= 1; if (b>2) b-=2;
    return b;
}

void G_BuildTiccmd(ticcmd_t* cmd)
{
    int32_t speed;
    int32_t tspeed;
    int32_t forward;
    int32_t side;
    int32_t newweapon;                                          // phares
    /* cphipps - remove needless I_BaseTiccmd call, just set the ticcmd to zero */
    memset(cmd,0,sizeof*cmd);

    //Use button negates the always run setting.
    speed = (_g->gamekeydown[key_use] ^ _g->alwaysRun);

    forward = side = 0;

    // use two stage accelerative turning
    // on the keyboard and joystick
    if (_g->gamekeydown[key_right] || _g->gamekeydown[key_left])
        _g->turnheld ++;
    else
        _g->turnheld = 0;

    if (_g->turnheld < SLOWTURNTICS)
        tspeed = 2;             // slow turn
    else
        tspeed = speed;

    // let movement keys cancel each other out

    if (_g->gamekeydown[key_right])
        cmd->angleturn -= angleturn[tspeed];
    if (_g->gamekeydown[key_left])
        cmd->angleturn += angleturn[tspeed];

    if (_g->gamekeydown[key_up])
        forward += forwardmove[speed];
    if (_g->gamekeydown[key_down])
        forward -= forwardmove[speed];

    if (_g->gamekeydown[key_straferight])
        side += sidemove[speed];

    if (_g->gamekeydown[key_strafeleft])
        side -= sidemove[speed];

    if (_g->gamekeydown[key_fire])
        cmd->buttons |= BT_ATTACK;

    if (_g->gamekeydown[key_use])
    {
        cmd->buttons |= BT_USE;
    }

    // Toggle between the top 2 favorite weapons.                   // phares
    // If not currently aiming one of these, switch to              // phares
    // the favorite. Only switch if you possess the weapon.         // phares

    // killough 3/22/98:
    //
    // Perform automatic weapons switch here rather than in p_pspr.c,
    // except in demo_compatibility mode.
    //
    // killough 3/26/98, 4/2/98: fix autoswitch when no weapons are left

    if(_g->gamekeydown[key_use] && _g->gamekeydown[key_straferight])
    {
        newweapon = P_WeaponCycleUp(&_g->player);
        side -= sidemove[speed]; //Hack cancel strafe.
    }

    else if(_g->gamekeydown[key_use] && _g->gamekeydown[key_strafeleft])
    {
        newweapon = P_WeaponCycleDown(&_g->player);
        side += sidemove[speed]; //Hack cancel strafe.
    }
    else if ((_g->player.attackdown && !P_CheckAmmo(&_g->player)))
        newweapon = P_SwitchWeapon(&_g->player);           // phares
    else
    {
        newweapon = wp_nochange;

        // killough 3/22/98: For network and demo consistency with the
        // new weapons preferences, we must do the weapons switches here
        // instead of in p_user.c. But for old demos we must do it in
        // p_user.c according to the old rules. Therefore demo_compatibility
        // determines where the weapons switch is made.

        // killough 2/8/98:
        // Allow user to switch to fist even if they have chainsaw.
        // Switch to fist or chainsaw based on preferences.
        // Switch to shotgun or SSG based on preferences.

        {
            const player_t *player = &_g->player;

            // only select chainsaw from '1' if it's owned, it's
            // not already in use, and the player prefers it or
            // the fist is already in use, or the player does not
            // have the berserker strength.

            if (newweapon==wp_fist && player->weaponowned[wp_chainsaw] &&
                    player->readyweapon!=wp_chainsaw &&
                    (player->readyweapon==wp_fist ||
                     !player->powers[pw_strength] ||
                     P_WeaponPreferred(wp_chainsaw, wp_fist)))
                newweapon = wp_chainsaw;
        }
        // killough 2/8/98, 3/22/98 -- end of weapon selection changes
    }

    if (newweapon != wp_nochange)
    {
        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= newweapon<<BT_WEAPONSHIFT;
    }

    if (forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;
    if (side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if (side < -MAXPLMOVE)
        side = -MAXPLMOVE;

    cmd->forwardmove += fudgef((int8_t)forward);
    cmd->sidemove += side;
}


//
// G_DoLoadLevel
//

static void G_DoLoadLevel (void)
{
    if (_g->wipegamestate == GS_LEVEL)
        _g->wipegamestate = -1;             // force a wipe

    _g->gamestate = GS_LEVEL;


    if (_g->playeringame && _g->player.playerstate == PST_DEAD)
        _g->player.playerstate = PST_REBORN;


    // initialize the msecnode_t freelist.                     phares 3/25/98
    // any nodes in the freelist are gone by now, cleared
    // by Z_FreeTags() when the previous level ended or player
    // died.

    P_SetSecnodeFirstpoolToNull();


    P_SetupLevel (_g->gamemap);

    _g->gameaction = ga_nothing;
    Z_CheckHeap ();

    // clear cmd building stuff
    memset (_g->gamekeydown, 0, sizeof(_g->gamekeydown));

    // killough 5/13/98: in case netdemo has consoleplayer other than green
    ST_Start();
    HU_Start();
}


//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//

boolean G_Responder (event_t* ev)
{
    // any other key pops up menu if in demos
    //
    // killough 8/2/98: enable automap in -timedemo demos
    //
    // killough 9/29/98: make any key pop up menu regardless of
    // which kind of demo, and allow other events during playback

    if (_g->gameaction == ga_nothing && (_g->demoplayback || _g->gamestate == GS_DEMOSCREEN))
    {
        // killough 10/98:
        // Don't pop up menu, if paused in middle
        // of demo playback, or if automap active.
        // Don't suck up keys, which may be cheats
        if(_g->gamestate == GS_DEMOSCREEN)
        {
            if(!(_g->automapmode & am_active))
            {
                if(ev->type == ev_keydown)
                {
                    M_StartControlPanel();
                    return true;
                }
            }
        }

        return false;
    }

    if (_g->gamestate == GS_FINALE && F_Responder(ev))
        return true;  // finale ate the event

    switch (ev->type)
    {
        case ev_keydown:

            if (ev->data1 <NUMKEYS)
                _g->gamekeydown[ev->data1] = true;
            return true;    // eat key down events

        case ev_keyup:
            if (ev->data1 <NUMKEYS)
                _g->gamekeydown[ev->data1] = false;
            return false;   // always let key up events filter down

        default:
            break;
    }
    return false;
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//

void G_Ticker (void)
{
    P_MapStart();

    if(_g->playeringame && _g->player.playerstate == PST_REBORN)
        G_DoReborn ();

    P_MapEnd();

    // do things to change the game state
    while (_g->gameaction != ga_nothing)
    {
        switch (_g->gameaction)
        {
        case ga_loadlevel:
            _g->player.playerstate = PST_REBORN;
            G_DoLoadLevel ();
            break;
        case ga_newgame:
            G_DoNewGame ();
            break;
        case ga_loadgame:
            G_DoLoadGame ();
            break;
        case ga_savegame:
            G_DoSaveGame ();
            break;
        case ga_playdemo:
            G_DoPlayDemo ();
            break;
        case ga_completed:
            G_DoCompleted ();
            break;
        case ga_victory:
            F_StartFinale ();
            break;
        case ga_worlddone:
            G_DoWorldDone ();
            break;
        case ga_nothing:
            break;
        }
    }

    if (!_g->demoplayback && _g->menuactive)
        _g->basetic++;  // For revenant tracers and RNG -- we must maintain sync
    else
    {
        if (_g->playeringame)
        {
            ticcmd_t *cmd = &_g->player.cmd;

            memcpy(cmd, &_g->netcmd, sizeof *cmd);

            if (_g->demoplayback)
                G_ReadDemoTiccmd (cmd);
        }
    }

    // cph - if the gamestate changed, we may need to clean up the old gamestate
    if (_g->gamestate != _g->prevgamestate)
    {
        switch (_g->prevgamestate)
        {
        case GS_LEVEL:
            // This causes crashes at level end - Neil Stevens
            // The crash is because the sounds aren't stopped before freeing them
            // the following is a possible fix
            // This fix does avoid the crash wowever, with this fix in, the exit
            // switch sound is cut off
            // S_Stop();
            // Z_FreeTags(PU_LEVEL, PU_PURGELEVEL-1);
            break;
        case GS_INTERMISSION:
            WI_End();
        default:
            break;
        }
        _g->prevgamestate = _g->gamestate;
    }

    // do main actions
    switch (_g->gamestate)
    {
    case GS_LEVEL:
        P_Ticker ();
        ST_Ticker ();
        AM_Ticker ();
        HU_Ticker ();
        break;

    case GS_INTERMISSION:
        WI_Ticker ();
        break;

    case GS_FINALE:
        F_Ticker ();
        break;

    case GS_DEMOSCREEN:
        D_PageTicker ();
        break;
    }
}

//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//

static void G_PlayerFinishLevel(void)
{
    player_t *p = &_g->player;
    memset(p->powers, 0, sizeof p->powers);
    memset(p->cards, 0, sizeof p->cards);
    p->mo = NULL;           // cph - this is allocated PU_LEVEL so it's gone
    p->extralight = 0;      // cancel gun flashes
    p->fixedcolormap = 0;   // cancel ir gogles
    p->damagecount = 0;     // no palette changes
    p->bonuscount = 0;
}

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//

void G_PlayerReborn (void)
{
    player_t *p;
    int32_t i;
    int32_t killcount;
    int32_t itemcount;
    int32_t secretcount;

    killcount = _g->player.killcount;
    itemcount = _g->player.itemcount;
    secretcount = _g->player.secretcount;

    p = &_g->player;

    int32_t cheats = p->cheats;
    memset (p, 0, sizeof(*p));
    p->cheats = cheats;

    _g->player.killcount = killcount;
    _g->player.itemcount = itemcount;
    _g->player.secretcount = secretcount;

    p->usedown = p->attackdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = initial_health;  // Ty 03/12/98 - use dehacked values
    p->readyweapon = p->pendingweapon = wp_pistol;
    p->weaponowned[wp_fist] = true;
    p->weaponowned[wp_pistol] = true;
    p->ammo[am_clip] = initial_bullets; // Ty 03/12/98 - use dehacked values

    for (i=0 ; i<NUMAMMO ; i++)
        p->maxammo[i] = maxammo[i];
}

//
// G_DoReborn
//

void G_DoReborn (void)
{
    _g->gameaction = ga_loadlevel;      // reload the level from scratch
}

// DOOM Par Times
static const int32_t pars[10] = {
    0,30,75,120,90,165,180,180,30,165
};


void G_ExitLevel (void)
{
    _g->secretexit = false;
    _g->gameaction = ga_completed;
}

// Here's for the german edition.
// IF NO WOLF3D LEVELS, NO SECRET EXIT!

void G_SecretExitLevel (void)
{
    _g->secretexit = true;
    _g->gameaction = ga_completed;
}

//
// G_DoCompleted
//

static void G_DoCompleted (void)
{
    _g->gameaction = ga_nothing;

    if (_g->playeringame)
        G_PlayerFinishLevel();        // take away cards and stuff

    if (_g->automapmode & am_active)
        AM_Stop();

    if (_g->gamemap == 9) // kilough 2/7/98
        _g->player.didsecret = true;

    _g->wminfo.didsecret = _g->player.didsecret;
    _g->wminfo.last = _g->gamemap -1;

    // wminfo.next is 0 biased, unlike gamemap
    if (_g->secretexit)
        _g->wminfo.next = 8;  // go to secret level
    else
        if (_g->gamemap == 9)
        {
            // returning from secret level
            _g->wminfo.next = 3;
        }
        else
           _g->wminfo.next = _g->gamemap;          // go to next level

    _g->wminfo.maxkills = _g->totalkills;
    _g->wminfo.maxitems = _g->totalitems;
    _g->wminfo.maxsecret = _g->totalsecret;

    _g->wminfo.partime = TICRATE*pars[_g->gamemap];

    _g->wminfo.pnum = 0;


    _g->wminfo.plyr[0].in = _g->playeringame;
    _g->wminfo.plyr[0].skills = _g->player.killcount;
    _g->wminfo.plyr[0].sitems = _g->player.itemcount;
    _g->wminfo.plyr[0].ssecret = _g->player.secretcount;
    _g->wminfo.plyr[0].stime = _g->leveltime;

    /* cph - modified so that only whole seconds are added to the totalleveltimes
   *  value; so our total is compatible with the "naive" total of just adding
   *  the times in seconds shown for each level. Also means our total time
   *  will agree with Compet-n.
   */
    _g->wminfo.totaltimes = (_g->totalleveltimes += (_g->leveltime - _g->leveltime%35));

    _g->gamestate = GS_INTERMISSION;
    _g->automapmode &= ~am_active;

    // lmpwatch.pl engine-side demo testing support
    // print "FINISHED: <mapname>" when the player exits the current map
    if (nodrawers && (_g->demoplayback || _g->timingdemo))
    {
        printf("FINISHED: E1M%ld\n", _g->gamemap);
    }

    WI_Start (&_g->wminfo);
}

//
// G_WorldDone
//

void G_WorldDone (void)
{
    _g->gameaction = ga_worlddone;

    if (_g->secretexit)
        _g->player.didsecret = true;

    if (_g->gamemap == 8)
        _g->gameaction = ga_victory; // cph - after ExM8 summary screen, show victory stuff
}

static void G_DoWorldDone (void)
{
    _g->gamestate = GS_LEVEL;
    _g->gamemap = _g->wminfo.next+1;
    G_DoLoadLevel();
    _g->gameaction = ga_nothing;
}

// killough 2/28/98: A ridiculously large number
// of players, the most you'll ever need in a demo
// or savegame. This is used to prevent problems, in
// case more players in a game are supported later.

#define MIN_MAXPLAYERS 4


inline static void LoadSRAM(byte* eeprom, uint32_t size, uint32_t offset)
{
	UNUSED(eeprom);
	UNUSED(size);
	UNUSED(offset);
}


//
// Update the strings displayed in the load-save menu.
//
void G_UpdateSaveGameStrings()
{
    uint32_t savebuffersize = sizeof(gba_save_data_t) * 8;


    byte* loadbuffer = Z_MallocStatic(savebuffersize);

    LoadSRAM(loadbuffer, savebuffersize, 0);

    gba_save_data_t* saveslots = (gba_save_data_t*)loadbuffer;

    for(int32_t i = 0; i < 8; i++)
    {
        if(saveslots[i].save_present != 1)
        {
            strcpy(_g->savegamestrings[i], "EMPTY");
        }
        else
        {
            strcpy(_g->savegamestrings[i], "E1My");

            _g->savegamestrings[i][3] = '0' + saveslots[i].gamemap;
        }
    }

    Z_Free(loadbuffer);
}

// killough 3/16/98: add slot info
void G_LoadGame(int32_t slot)
{  
    _g->savegameslot = slot;
    _g->demoplayback = false;

    G_DoLoadGame();
}

static void G_DoLoadGame(void)
{
    uint32_t savebuffersize = sizeof(gba_save_data_t) * 8;


    byte* loadbuffer = Z_MallocStatic(savebuffersize);

    LoadSRAM(loadbuffer, savebuffersize, 0);

    gba_save_data_t* saveslots = (gba_save_data_t*)loadbuffer;

    gba_save_data_t* savedata = &saveslots[_g->savegameslot];

    if(savedata->save_present != 1)
        return;

    _g->gameskill = savedata->gameskill;
    _g->gamemap = savedata->gamemap;
	
    G_InitNew (_g->gameskill, _g->gamemap);

    _g->totalleveltimes = savedata->totalleveltimes;
    memcpy(_g->player.weaponowned, savedata->weaponowned, sizeof(savedata->weaponowned));
    memcpy(_g->player.ammo, savedata->ammo, sizeof(savedata->ammo));
    memcpy(_g->player.maxammo, savedata->maxammo, sizeof(savedata->maxammo));
	
    //If stored maxammo is more than no backpack ammo, player had a backpack.
    if(_g->player.maxammo[am_clip] > maxammo[am_clip])
		_g->player.backpack = true;

    Z_Free(loadbuffer);
}

//
// G_SaveGame
// Called by the menu task.
//

void G_SaveGame(int32_t slot)
{
    _g->savegameslot = slot;
    G_DoSaveGame();
}


inline static void SaveSRAM(const byte* eeprom, uint32_t size, uint32_t offset)
{
	UNUSED(eeprom);
	UNUSED(size);
	UNUSED(offset);
}


static void G_DoSaveGame(void)
{
    uint32_t savebuffersize = sizeof(gba_save_data_t) * 8;

    byte* savebuffer = Z_MallocStatic(savebuffersize);

    LoadSRAM(savebuffer, savebuffersize, 0);

    gba_save_data_t* saveslots = (gba_save_data_t*)savebuffer;

    gba_save_data_t* savedata = &saveslots[_g->savegameslot];

    savedata->save_present = 1;

    savedata->gameskill = _g->gameskill;
    savedata->gamemap = _g->gamemap;
    savedata->totalleveltimes = _g->totalleveltimes;

    memcpy(savedata->weaponowned, _g->player.weaponowned, sizeof(savedata->weaponowned));
    memcpy(savedata->ammo, _g->player.ammo, sizeof(savedata->ammo));
    memcpy(savedata->maxammo, _g->player.maxammo, sizeof(savedata->maxammo));

    SaveSRAM(savebuffer, savebuffersize, 0);

    Z_Free(savebuffer);

    _g->player.message = GGSAVED;

    G_UpdateSaveGameStrings();
}

void G_SaveSettings()
{
    gba_save_settings_t settings;

    settings.cookie = settings_cookie;

    settings.gamma = _g->gamma;
    settings.alwaysRun = _g->alwaysRun;

    settings.showMessages = _g->showMessages;

    settings.musicVolume = _g->snd_MusicVolume;
    settings.soundVolume = _g->snd_SfxVolume;
	settings.highDetail = _g->highDetail;

    SaveSRAM((byte*)&settings, sizeof(settings), settings_sram_offset);
}

void G_LoadSettings()
{
    gba_save_settings_t settings;

    LoadSRAM((byte*)&settings, sizeof(settings), settings_sram_offset);

    if(settings.cookie == settings_cookie)
    {
        _g->gamma = (settings.gamma > 5) ? 5 : settings.gamma;
        _g->alwaysRun = (settings.alwaysRun > 0) ? 1 : 0;

        _g->showMessages = (settings.showMessages > 0) ? 1 : 0;

        _g->snd_SfxVolume = (settings.soundVolume > 15) ? 15 : settings.soundVolume;
        _g->snd_MusicVolume = (settings.musicVolume > 15) ? 15 : settings.musicVolume;
		_g->highDetail = (settings.highDetail > 0) ? 1 : 0;
		highDetail = _g->highDetail;
		
        I_SetPalette(0);

        S_SetSfxVolume(_g->snd_SfxVolume);
        S_SetMusicVolume(_g->snd_MusicVolume);
    }
}

void G_DeferedInitNew(skill_t skill)
{
    _g->d_skill = skill;
    _g->gameaction = ga_newgame;
}

// killough 3/1/98: function to reload all the default parameter
// settings before a new game begins

void G_ReloadDefaults(void)
{
    // killough 3/1/98: Initialize options based on config file
    // (allows functions above to load different values for demos
    // and savegames without messing up defaults).

    _g->demoplayback = false;
    _g->singledemo = false;            // killough 9/29/98: don't stop after 1 demo
}

static void G_DoNewGame (void)
{
    G_ReloadDefaults();            // killough 3/1/98
    G_InitNew (_g->d_skill, 1);
    _g->gameaction = ga_nothing;

    //jff 4/26/98 wake up the status bar in case were coming out of a DM demo
    ST_Start();
}

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//

static void G_InitNew(skill_t skill, int32_t map)
{
    if (skill > sk_nightmare)
        skill = sk_nightmare;

    if (map < 1)
        map = 1;
    if (map > 9)
        map = 9;

    M_ClearRandom();

    _g->respawnmonsters = skill == sk_nightmare;

    _g->player.playerstate = PST_REBORN;

    _g->usergame = true;                // will be set false if a demo
    _g->automapmode &= ~am_active;
    _g->gamemap = map;
    _g->gameskill = skill;

    _g->totalleveltimes = 0; // cph

    G_DoLoadLevel ();
}

//
// DEMO RECORDING
//

#define DEMOMARKER    0x80

static void G_ReadDemoTiccmd (ticcmd_t* cmd)
{
    uint8_t at; // e6y: tasdoom stuff

    if (*_g->demo_p == DEMOMARKER)
        G_CheckDemoStatus();      // end of demo data stream
    else if (_g->demoplayback && _g->demo_p + 4 > _g->demobuffer + _g->demolength)
    {
        printf("G_ReadDemoTiccmd: missing DEMOMARKER\n");
        G_CheckDemoStatus();
    }
    else
    {
        cmd->forwardmove = ((int8_t)*_g->demo_p++);
        cmd->sidemove = ((int8_t)*_g->demo_p++);
        cmd->angleturn = ((uint8_t)(at = *_g->demo_p++))<<8;
        cmd->buttons = (uint8_t)*_g->demo_p++;
    }
}


//
// G_PlayDemo
//

static const char *defdemoname;

void G_DeferedPlayDemo (const char* name)
{
    defdemoname = name;
    _g->gameaction = ga_playdemo;
}


//e6y: Check for overrun
static void CheckForOverrun(const byte *start_p, const byte *current_p, size_t maxsize, size_t size)
{
    size_t pos = current_p - start_p;
    if (pos + size > maxsize)
    {
        I_Error("CheckForOverrun: wrong demo header\n");
    }
}

static const byte* G_ReadDemoHeader(const byte *demo_p, size_t size)
{
    skill_t skill;
    int32_t map;

    // e6y
    // The local variable should be used instead of demobuffer,
    // because demobuffer can be uninitialized
    const byte *header_p = demo_p;

    _g->basetic = _g->gametic;  // killough 9/29/98

    // killough 2/22/98, 2/28/98: autodetect old demos and act accordingly.
    // Old demos turn on demo_compatibility => compatibility; new demos load
    // compatibility flag, and other flags as well, as a part of the demo.

    //e6y: check for overrun
    CheckForOverrun(header_p, demo_p, size, 1);

    demo_p++;

    // killough 3/2/98: force these variables to be 0 in demo_compatibility

    // killough 3/6/98: rearrange to fix savegame bugs (moved fastparm,
    // respawnparm, nomonsters flags to G_LoadOptions()/G_SaveOptions())

    //e6y: check for overrun
    CheckForOverrun(header_p, demo_p, size, 8);

    skill = *demo_p++;
    demo_p++;
    map = *demo_p++;
    demo_p++;
    demo_p++;
    demo_p++;
    demo_p++;
    demo_p++;

    //e6y: check for overrun
    CheckForOverrun(header_p, demo_p, size, MAXPLAYERS);

    _g->playeringame = *demo_p++;
    demo_p += MIN_MAXPLAYERS - MAXPLAYERS;


    if (_g->gameaction != ga_loadgame) { /* killough 12/98: support -loadgame */
        G_InitNew(skill, map);
    }

    _g->player.cheats = 0;

    return demo_p;
}


static void ExtractFileBase (const char *path, char *dest)
{
    const char *src = path + strlen(path) - 1;
    int32_t length;

    // back up until a \ or the start
    while (src != path && src[-1] != ':' // killough 3/22/98: allow c:filename
           && *(src-1) != '\\'
           && *(src-1) != '/')
    {
        src--;
    }

    // copy up to eight characters
    memset(dest,0,8);
    length = 0;

    while ((*src) && (*src != '.') && (++length<9))
    {
        *dest++ = toupper(*src);
        src++;
    }
    /* cph - length check removed, just truncate at 8 chars.
   * If there are 8 or more chars, we'll copy 8, and no zero termination
   */
}


static void G_DoPlayDemo(void)
{
    char basename[9];

    ExtractFileBase(defdemoname,basename);           // killough
    basename[8] = 0;

    /* cph - store lump number for unlocking later */
    int16_t demolumpnum = W_GetNumForName(basename);
    _g->demobuffer = W_GetLumpByNum(demolumpnum);
    _g->demolength = W_LumpLength(demolumpnum);

    _g->demo_p = G_ReadDemoHeader(_g->demobuffer, _g->demolength);

    _g->gameaction = ga_nothing;
    _g->usergame = false;

    _g->demoplayback = true;

    _g->starttime = I_GetTime();
}

/* G_CheckDemoStatus
 *
 * Called after a death or level completion to allow demos to be cleaned up
 */
void G_CheckDemoStatus (void)
{
    if (_g->timingdemo)
    {
        int32_t endtime = I_GetTime();
        // killough -- added fps information and made it work for longer demos:
        uint32_t realtics = endtime-_g->starttime;
        uint32_t resultfps = TICRATE * 1000L * _g->gametic / realtics;
        I_Error ("Timed %lu gametics in %lu realtics = %lu.%.3lu frames per second",
                 (uint32_t) _g->gametic,realtics,
                 resultfps / 1000, resultfps % 1000);
    }

    Z_Free(_g->demobuffer);

    if (_g->demoplayback)
    {
        if (_g->singledemo)
            exit(0);  // killough

        G_ReloadDefaults();    // killough 3/1/98
        D_AdvanceDemo ();
    }
}

