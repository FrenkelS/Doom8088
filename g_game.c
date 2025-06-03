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

#include "compiler.h"
#include "d_player.h"
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


static const byte __far* demobuffer;   /* cph - only used for playback */
static uint16_t demolength; // check for overrun (missing DEMOMARKER)

static const byte __far* demo_p;

gameaction_t    _g_gameaction;
gamestate_t     _g_gamestate;
skill_t         _g_gameskill;

int16_t             _g_gamemap;

player_t        _g_player;

static int32_t             starttime;     // for comparative timing purposes

int32_t             _g_gametic;
int32_t             _g_basetic;       /* killough 9/29/98: for demo sync */
int32_t             _g_totalkills, _g_totallive, _g_totalitems, _g_totalsecret;    // for intermission
wbstartstruct_t _g_wminfo;               // parms for world map / intermission
static int32_t             totalleveltimes;      // CPhipps - total time for all completed levels


static boolean gamekeydown[NUMKEYS];

static skill_t d_skill;

static byte  savegameslot;         // Slot to load if gameaction == ga_loadgame


static boolean secretexit;

boolean         _g_respawnmonsters;

boolean         _g_usergame;      // ok to save / end game
boolean         _g_timingdemo;    // if true, exit with report on completion
boolean         _g_demoplayback;
boolean         _g_singledemo;           // quit after playing a demo from cmdline


//
// controls (have defaults)
//

static const int16_t key_right       = KEYD_RIGHT;
static const int16_t key_left        = KEYD_LEFT;
static const int16_t key_up          = KEYD_UP;
static const int16_t key_down        = KEYD_DOWN;
       const int16_t key_menu_right  = KEYD_RIGHT;
       const int16_t key_menu_left   = KEYD_LEFT;
       const int16_t key_menu_up     = KEYD_UP;
       const int16_t key_menu_down   = KEYD_DOWN;
       const int16_t key_menu_escape = KEYD_START;
       const int16_t key_menu_enter  = KEYD_A;
static const int16_t key_strafeleft  = KEYD_L;
static const int16_t key_straferight = KEYD_R;
       const int16_t key_fire        = KEYD_B; 
static const int16_t key_speed       = KEYD_SPEED;
static const int16_t key_strafe      = KEYD_STRAFE;
static const int16_t key_use         = KEYD_A;
       const int16_t key_escape      = KEYD_START;
       const int16_t key_map_right   = KEYD_RIGHT;
       const int16_t key_map_left    = KEYD_LEFT;
       const int16_t key_map_up      = KEYD_UP;
       const int16_t key_map_down    = KEYD_DOWN;
       const int16_t key_map         = KEYD_SELECT;
       const int16_t key_map_follow  = 'f';
       const int16_t key_map_zoomin  = KEYD_PLUS;
       const int16_t key_map_zoomout = KEYD_MINUS;
static const int16_t key_weapon_up   = KEYD_BRACKET_RIGHT;
static const int16_t key_weapon_down = KEYD_BRACKET_LEFT;


#define MAXPLMOVE   (forwardmove[1])
#define SLOWTURNTICS  6

static const int8_t forwardmove[2] = {0x19, 0x32};
static const int8_t sidemove[2]    = {0x18, 0x28};
static const int16_t angleturn[3]  = {640, 1280, 320};  // + slow turn

static void G_DoReborn (void);
static void G_DoCompleted(void);
static void G_DoWorldDone(void);
static void G_DoLoadGame(void);
static void G_DoSaveGame (void);
static void G_DoNewGame (void);
static void G_DoPlayDemo(void);
static void G_InitNew(skill_t skill, int16_t map);
static void G_ReadDemoTiccmd (void);


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
} gba_save_settings_t;

static const uint32_t settings_cookie = 0xbaddead1;

static const uint16_t settings_sram_offset = sizeof(gba_save_data_t) * 8;

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
static inline int8_t fudgef(int8_t b)
{
    static int16_t c;
    if (!b) return b;
    if (++c & 0x1f) return b;
    b |= 1; if (b>2) b-=2;
    return b;
}

static ticcmd_t netcmd;

void G_BuildTiccmd(void)
{
    static int16_t     turnheld = 0;       // for accelerative turning

    boolean strafe;
    int16_t speed;
    int16_t tspeed;
    int16_t forward;
    int16_t side;
    weapontype_t newweapon;
    /* cphipps - remove needless I_BaseTiccmd call, just set the ticcmd to zero */
    memset(&netcmd,0,sizeof(ticcmd_t));

    strafe = gamekeydown[key_strafe];

    //Use button negates the always run setting.
    speed = (gamekeydown[key_speed] ^ _g_alwaysRun);

    forward = side = 0;

    // use two stage accelerative turning
    // on the keyboard
    if (gamekeydown[key_right] || gamekeydown[key_left])
        turnheld++;
    else
        turnheld = 0;

    if (turnheld < SLOWTURNTICS)
        tspeed = 2;             // slow turn
    else
        tspeed = speed;

    // let movement keys cancel each other out

    if (strafe)
    {
        if (gamekeydown[key_right])
            side += sidemove[speed];
        if (gamekeydown[key_left])
            side -= sidemove[speed];
    }
    else
    {
        if (gamekeydown[key_right])
            netcmd.angleturn -= angleturn[tspeed];
        if (gamekeydown[key_left])
            netcmd.angleturn += angleturn[tspeed];
    }

    if (gamekeydown[key_up])
        forward += forwardmove[speed];
    if (gamekeydown[key_down])
        forward -= forwardmove[speed];

    if (gamekeydown[key_straferight])
        side += sidemove[speed];

    if (gamekeydown[key_strafeleft])
        side -= sidemove[speed];

    if (gamekeydown[key_fire])
        netcmd.buttons |= BT_ATTACK;

    if (gamekeydown[key_use])
    {
        netcmd.buttons |= BT_USE;
    }

    if(gamekeydown[key_weapon_up])
        newweapon = P_WeaponCycleUp(&_g_player);
    else if(gamekeydown[key_weapon_down])
        newweapon = P_WeaponCycleDown(&_g_player);
    else if ((_g_player.attackdown && !P_CheckAmmo(&_g_player)))
        newweapon = P_SwitchWeapon(&_g_player);
    else
        newweapon = wp_nochange;

    if (newweapon != wp_nochange)
    {
        netcmd.buttons |= BT_CHANGE;
        netcmd.buttons |= newweapon<<BT_WEAPONSHIFT;
    }

    if (forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;
    if (side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if (side < -MAXPLMOVE)
        side = -MAXPLMOVE;

    netcmd.forwardmove += fudgef((int8_t)forward);
    netcmd.sidemove += side;
}


//
// G_DoLoadLevel
//

static void G_DoLoadLevel (void)
{
    if (wipegamestate == GS_LEVEL)
        wipegamestate = -1;             // force a wipe

    _g_gamestate = GS_LEVEL;


    if (_g_player.playerstate == PST_DEAD)
        _g_player.playerstate = PST_REBORN;


    // initialize the msecnode_t freelist.                     phares 3/25/98
    // any nodes in the freelist are gone by now, cleared
    // by Z_FreeTags() when the previous level ended or player
    // died.

    P_SetSecnodeFirstpoolToNull();


    P_SetupLevel (_g_gamemap);

    _g_gameaction = ga_nothing;
    Z_CheckHeap ();

    // clear cmd building stuff
    memset(gamekeydown, 0, sizeof(gamekeydown));

    ST_Start(); // wake up the status bar
    HU_Start(); // wake up the heads up text
}


//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//

void G_Responder (event_t* ev)
{
    // any other key pops up menu if in demos
    //
    // killough 8/2/98: enable automap in -timedemo demos
    //
    // killough 9/29/98: make any key pop up menu regardless of
    // which kind of demo, and allow other events during playback

    if (_g_gameaction == ga_nothing && (_g_demoplayback || _g_gamestate == GS_DEMOSCREEN))
    {
        // killough 10/98:
        // Don't pop up menu, if paused in middle
        // of demo playback, or if automap active.
        // Don't suck up keys, which may be cheats
        if(_g_gamestate == GS_DEMOSCREEN)
        {
            if(!(automapmode & am_active))
            {
                if(ev->type == ev_keydown)
                {
                    M_StartControlPanel();
                    return;
                }
            }
        }

        return;
    }

    switch (ev->type)
    {
        case ev_keydown:
            if (ev->data1 < NUMKEYS)
                gamekeydown[ev->data1] = true;
            return;    // eat key down events

        case ev_keyup:
            if (ev->data1 < NUMKEYS)
                gamekeydown[ev->data1] = false;
            return;   // always let key up events filter down

        default:
            break;
    }
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//

void G_Ticker (void)
{
    static gamestate_t prevgamestate = 0;

    if(_g_player.playerstate == PST_REBORN)
        G_DoReborn ();

    P_MapEnd();

    // do things to change the game state
    while (_g_gameaction != ga_nothing)
    {
        switch (_g_gameaction)
        {
        case ga_loadlevel:
            _g_player.playerstate = PST_REBORN;
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

    if (!_g_demoplayback && _g_menuactive)
        _g_basetic++;  // For revenant tracers and RNG -- we must maintain sync
    else
    {
        memcpy(&_g_player.cmd, &netcmd, sizeof(ticcmd_t));

        if (_g_demoplayback)
            G_ReadDemoTiccmd ();
    }

    // cph - if the gamestate changed, we may need to clean up the old gamestate
    if (_g_gamestate != prevgamestate)
    {
        switch (prevgamestate)
        {
        case GS_LEVEL:
            // This causes crashes at level end - Neil Stevens
            // The crash is because the sounds aren't stopped before freeing them
            // the following is a possible fix
            // This fix does avoid the crash however, with this fix in, the exit
            // switch sound is cut off
            // S_Stop();
            // Z_FreeTags(PU_LEVEL, PU_PURGELEVEL-1);
            break;
        case GS_INTERMISSION:
            WI_End();
        default:
            break;
        }
        prevgamestate = _g_gamestate;
    }

    // do main actions
    switch (_g_gamestate)
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
    player_t *p = &_g_player;
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
    int16_t i;
    int16_t killcount;
    int16_t itemcount;
    int16_t secretcount;

    killcount   = _g_player.killcount;
    itemcount   = _g_player.itemcount;
    secretcount = _g_player.secretcount;

    p = &_g_player;

    int16_t cheats = p->cheats;
    memset (p, 0, sizeof(*p));
    p->cheats = cheats;

    _g_player.killcount = killcount;
    _g_player.itemcount = itemcount;
    _g_player.secretcount = secretcount;

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

static void G_DoReborn(void)
{
    _g_gameaction = ga_loadlevel;      // reload the level from scratch
}


void G_ExitLevel (void)
{
    secretexit = false;
    _g_gameaction = ga_completed;
}

// Here's for the german edition.
// IF NO WOLF3D LEVELS, NO SECRET EXIT!

void G_SecretExitLevel (void)
{
    secretexit = true;
    _g_gameaction = ga_completed;
}


// DOOM Par Times
static const uint8_t pars[10] = {
    0,30,75,120,90,165,180,180,30,165
};


//
// G_DoCompleted
//

static void G_DoCompleted (void)
{
    _g_gameaction = ga_nothing;

    G_PlayerFinishLevel();        // take away cards and stuff

    if (automapmode & am_active)
        AM_Stop();

    if (_g_gamemap == 9) // kilough 2/7/98
        _g_player.didsecret = true;

    _g_wminfo.didsecret = _g_player.didsecret;
    _g_wminfo.last = _g_gamemap -1;

    // wminfo.next is 0 biased, unlike gamemap
    if (secretexit)
        _g_wminfo.next = 8;  // go to secret level
    else
        if (_g_gamemap == 9)
        {
            // returning from secret level
            _g_wminfo.next = 3;
        }
        else
           _g_wminfo.next = _g_gamemap;          // go to next level

    _g_wminfo.maxkills = _g_totalkills;
    _g_wminfo.maxitems = _g_totalitems;
    _g_wminfo.maxsecret = _g_totalsecret;

    _g_wminfo.partime = TICRATE*pars[_g_gamemap];


    _g_wminfo.plyr[0].skills = _g_player.killcount;
    _g_wminfo.plyr[0].sitems = _g_player.itemcount;
    _g_wminfo.plyr[0].ssecret = _g_player.secretcount;
    _g_wminfo.plyr[0].stime = _g_leveltime;

    /* cph - modified so that only whole seconds are added to the totalleveltimes
   *  value; so our total is compatible with the "naive" total of just adding
   *  the times in seconds shown for each level. Also means our total time
   *  will agree with Compet-n.
   */
    _g_wminfo.totaltimes = (totalleveltimes += (_g_leveltime - (int16_t)_g_leveltime % TICRATE));

    _g_gamestate = GS_INTERMISSION;
    automapmode &= ~am_active;

    // lmpwatch.pl engine-side demo testing support
    // print "FINISHED: <mapname>" when the player exits the current map
    if (nodrawers && (_g_demoplayback || _g_timingdemo))
    {
        printf("FINISHED: E1M%d\n", _g_gamemap);
    }

    WI_Start (&_g_wminfo);
}

//
// G_WorldDone
//

void G_WorldDone (void)
{
    _g_gameaction = ga_worlddone;

    if (secretexit)
        _g_player.didsecret = true;

    if (_g_gamemap == 8)
        _g_gameaction = ga_victory; // cph - after ExM8 summary screen, show victory stuff
}

static void G_DoWorldDone (void)
{
    _g_gamestate = GS_LEVEL;
    _g_gamemap = _g_wminfo.next+1;
    G_DoLoadLevel();
    _g_gameaction = ga_nothing;
}

// killough 2/28/98: A ridiculously large number
// of players, the most you'll ever need in a demo
// or savegame. This is used to prevent problems, in
// case more players in a game are supported later.

#define MIN_MAXPLAYERS 4


inline static void LoadSRAM(byte __far* eeprom, uint16_t size, uint16_t offset)
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
    uint16_t savebuffersize = sizeof(gba_save_data_t) * 8;


    byte __far* loadbuffer = Z_MallocStatic(savebuffersize);

    LoadSRAM(loadbuffer, savebuffersize, 0);

    gba_save_data_t __far* saveslots = (gba_save_data_t __far*)loadbuffer;

    for(int16_t i = 0; i < 8; i++)
    {
        if(saveslots[i].save_present != 1)
        {
            strcpy(_g_savegamestrings[i], "EMPTY");
        }
        else
        {
            strcpy(_g_savegamestrings[i], "E1My");

            _g_savegamestrings[i][3] = '0' + saveslots[i].gamemap;
        }
    }

    Z_Free(loadbuffer);
}

// killough 3/16/98: add slot info
void G_LoadGame(int16_t slot)
{
    savegameslot = slot;
    _g_demoplayback = false;

    G_DoLoadGame();
}

static void G_DoLoadGame(void)
{
    uint16_t savebuffersize = sizeof(gba_save_data_t) * 8;


    byte __far* loadbuffer = Z_MallocStatic(savebuffersize);

    LoadSRAM(loadbuffer, savebuffersize, 0);

    gba_save_data_t __far* saveslots = (gba_save_data_t __far*)loadbuffer;

    gba_save_data_t __far* savedata = &saveslots[savegameslot];

    if(savedata->save_present != 1)
        return;

    _g_gameskill = savedata->gameskill;
    _g_gamemap = savedata->gamemap;
	
    G_InitNew (_g_gameskill, _g_gamemap);

    totalleveltimes = savedata->totalleveltimes;
    _fmemcpy(_g_player.weaponowned, savedata->weaponowned, sizeof(savedata->weaponowned));
    _fmemcpy(_g_player.ammo, savedata->ammo, sizeof(savedata->ammo));
    _fmemcpy(_g_player.maxammo, savedata->maxammo, sizeof(savedata->maxammo));
	
    //If stored maxammo is more than no backpack ammo, player had a backpack.
    if(_g_player.maxammo[am_clip] > maxammo[am_clip])
		_g_player.backpack = true;

    Z_Free(loadbuffer);
}

//
// G_SaveGame
// Called by the menu task.
//

void G_SaveGame(int16_t slot)
{
    savegameslot = slot;
    G_DoSaveGame();
}


inline static void SaveSRAM(const byte __far* eeprom, uint16_t size, uint16_t offset)
{
	UNUSED(eeprom);
	UNUSED(size);
	UNUSED(offset);
}


static void G_DoSaveGame(void)
{
    uint16_t savebuffersize = sizeof(gba_save_data_t) * 8;

    byte __far* savebuffer = Z_MallocStatic(savebuffersize);

    LoadSRAM(savebuffer, savebuffersize, 0);

    gba_save_data_t __far* saveslots = (gba_save_data_t __far*)savebuffer;

    gba_save_data_t __far* savedata = &saveslots[savegameslot];

    savedata->save_present = 1;

    savedata->gameskill = _g_gameskill;
    savedata->gamemap = _g_gamemap;
    savedata->totalleveltimes = totalleveltimes;

    _fmemcpy(savedata->weaponowned, _g_player.weaponowned, sizeof(savedata->weaponowned));
    _fmemcpy(savedata->ammo, _g_player.ammo, sizeof(savedata->ammo));
    _fmemcpy(savedata->maxammo, _g_player.maxammo, sizeof(savedata->maxammo));

    SaveSRAM(savebuffer, savebuffersize, 0);

    Z_Free(savebuffer);

    _g_player.message = GGSAVED;

    G_UpdateSaveGameStrings();
}

void G_SaveSettings()
{
    gba_save_settings_t settings;

    settings.cookie = settings_cookie;

    settings.gamma = _g_gamma;
    settings.alwaysRun = _g_alwaysRun;

    settings.showMessages = showMessages;

    settings.musicVolume = snd_MusicVolume;
    settings.soundVolume = snd_SfxVolume;

    SaveSRAM((byte __far*)&settings, sizeof(settings), settings_sram_offset);
}

void G_LoadSettings()
{
    gba_save_settings_t settings;

    LoadSRAM((byte __far*)&settings, sizeof(settings), settings_sram_offset);

    if(settings.cookie == settings_cookie)
    {
        _g_gamma = (settings.gamma > 5) ? 5 : settings.gamma;
        _g_alwaysRun = (settings.alwaysRun > 0) ? 1 : 0;

        showMessages = (settings.showMessages > 0) ? 1 : 0;

        snd_SfxVolume   = (settings.soundVolume > 15) ? 15 : settings.soundVolume;
        snd_MusicVolume = (settings.musicVolume > 15) ? 15 : settings.musicVolume;
		
        I_SetPalette(0);

        S_SetSfxVolume(snd_SfxVolume);
        S_SetMusicVolume(snd_MusicVolume);
    }
}

void G_DeferedInitNew(skill_t skill)
{
    d_skill = skill;
    _g_gameaction = ga_newgame;
}

// killough 3/1/98: function to reload all the default parameter
// settings before a new game begins

void G_ReloadDefaults(void)
{
    // killough 3/1/98: Initialize options based on config file
    // (allows functions above to load different values for demos
    // and savegames without messing up defaults).

    _g_demoplayback = false;
    _g_singledemo = false;            // killough 9/29/98: don't stop after 1 demo
}

static void G_DoNewGame (void)
{
    G_ReloadDefaults();            // killough 3/1/98
    G_InitNew (d_skill, 1);
    _g_gameaction = ga_nothing;

    //jff 4/26/98 wake up the status bar in case were coming out of a DM demo
    ST_Start();
}

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//

static void G_InitNew(skill_t skill, int16_t map)
{
    if (skill > sk_nightmare)
        skill = sk_nightmare;

    if (map < 1)
        map = 1;
    if (map > 9)
        map = 9;

    M_ClearRandom();

    _g_respawnmonsters = skill == sk_nightmare;

    _g_player.playerstate = PST_REBORN;

    _g_usergame = true;                // will be set false if a demo
    automapmode &= ~am_active;
    _g_gamemap = map;
    _g_gameskill = skill;

    totalleveltimes = 0;

    G_DoLoadLevel ();
}

//
// DEMO RECORDING
//

#define DEMOMARKER    0x80

static void G_ReadDemoTiccmd (void)
{
    uint8_t at; // e6y: tasdoom stuff

    if (*demo_p == DEMOMARKER)
        G_CheckDemoStatus();      // end of demo data stream
    else if (_g_demoplayback && demo_p + 4 > demobuffer + demolength)
    {
        printf("G_ReadDemoTiccmd: missing DEMOMARKER\n");
        G_CheckDemoStatus();
    }
    else
    {
        ticcmd_t* cmd = &_g_player.cmd;
        cmd->forwardmove = ((int8_t)*demo_p++);
        cmd->sidemove = ((int8_t)*demo_p++);
        cmd->angleturn = ((uint8_t)(at = *demo_p++))<<8;
        cmd->buttons = (uint8_t)*demo_p++;
    }
}


//
// G_PlayDemo
//

static const char *defdemoname;

void G_DeferedPlayDemo (const char* name)
{
    defdemoname = name;
    _g_gameaction = ga_playdemo;
}


//e6y: Check for overrun
static void CheckForOverrun(const byte __far* start_p, const byte __far* current_p, size_t size)
{
    size_t pos = current_p - start_p;
    if (pos + size > demolength)
    {
        I_Error("CheckForOverrun: wrong demo header\n");
    }
}

static const byte __far* G_ReadDemoHeader(const byte __far* demo_p)
{
    skill_t skill;
    int16_t map;

    // e6y
    // The local variable should be used instead of demobuffer,
    // because demobuffer can be uninitialized
    const byte __far* header_p = demo_p;

    _g_basetic = _g_gametic;  // killough 9/29/98

    // killough 2/22/98, 2/28/98: autodetect old demos and act accordingly.
    // Old demos turn on demo_compatibility => compatibility; new demos load
    // compatibility flag, and other flags as well, as a part of the demo.

    //e6y: check for overrun
    CheckForOverrun(header_p, demo_p, 1);

    demo_p++;

    // killough 3/2/98: force these variables to be 0 in demo_compatibility

    // killough 3/6/98: rearrange to fix savegame bugs (moved fastparm,
    // respawnparm, nomonsters flags to G_LoadOptions()/G_SaveOptions())

    //e6y: check for overrun
    CheckForOverrun(header_p, demo_p, 8);

    skill = *demo_p++;
    demo_p++;
    map = *demo_p++;
    demo_p++;
    demo_p++;
    demo_p++;
    demo_p++;
    demo_p++;

    //e6y: check for overrun
    CheckForOverrun(header_p, demo_p, MAXPLAYERS);

    demo_p++;
    demo_p += MIN_MAXPLAYERS - MAXPLAYERS;


    if (_g_gameaction != ga_loadgame) { /* killough 12/98: support -loadgame */
        G_InitNew(skill, map);
    }

    _g_player.cheats = 0;

    return demo_p;
}


static void ExtractFileBase (const char *path, char *dest)
{
    const char *src = path + strlen(path) - 1;
    int16_t length;

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
    demobuffer = W_GetLumpByNum(demolumpnum);
    demolength = W_LumpLength(demolumpnum);

    demo_p = G_ReadDemoHeader(demobuffer);

    _g_gameaction = ga_nothing;
    _g_usergame = false;

    _g_demoplayback = true;

    starttime = I_GetTime();
}

/* G_CheckDemoStatus
 *
 * Called after a death or level completion to allow demos to be cleaned up
 */
void G_CheckDemoStatus (void)
{
    if (_g_timingdemo)
    {
        int32_t endtime = I_GetTime();
        // killough -- added fps information and made it work for longer demos:
        uint32_t realtics = endtime - starttime;
        uint32_t resultfps = TICRATE * 1000L * _g_gametic / realtics;
        I_Error ("Timed %lu gametics in %lu realtics = %lu.%.3lu frames per second",
                 (uint32_t) _g_gametic,realtics,
                 resultfps / 1000, resultfps % 1000);
    }

    Z_ChangeTagToCache(demobuffer);

    if (_g_demoplayback)
    {
        if (_g_singledemo)
            exit(0);  // killough

        G_ReloadDefaults();    // killough 3/1/98
        D_AdvanceDemo ();
    }
}

