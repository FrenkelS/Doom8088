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
 *  DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
 *  plus functions to
 *  parse command line parameters, configure game parameters (turbo),
 *  and call the startup functions.
 *
 *-----------------------------------------------------------------------------
 */

#include <stdint.h>

#include "doomdef.h"
#include "doomtype.h"
#include "d_player.h"
#include "d_englsh.h"
#include "sounds.h"
#include "z_zone.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"
#include "f_finale.h"
#include "f_wipe.h"
#include "m_menu.h"
#include "i_system.h"
#include "i_sound.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "p_setup.h"
#include "r_main.h"
#include "d_main.h"
#include "am_map.h"
#include "m_cheat.h"
#include "globdata.h"

static void D_DoAdvanceDemo(void);
static void D_PageDrawer(void);
static void D_UpdateFPS(void);


// CPhipps - removed wadfiles[] stuff


//jff 1/22/98 parms for disabling music and sound
      boolean nosfxparm   = false;
const boolean nomusicparm = true;

const boolean nodrawers = false;


static int32_t maketic;


static int16_t  pagetic;

static boolean singletics; // debug flag to cancel adaptiveness
static boolean advancedemo;
boolean _g_fps_show;

//fps counter stuff
int16_t _g_fps_framerate;


static int16_t titlepicnum;


/*
 * D_PostEvent - Event handling
 *
 * Called by I/O functions when an event is received.
 * Try event handlers for each code area in turn.
 * cph - in the true spirit of the Boom source, let the 
 *  short ciruit operator madness begin!
 */

void D_PostEvent(event_t *ev)
{
    /* cph - suppress all input events at game start
   * FIXME: This is a lousy kludge */
    if (_g_gametic < 3)
        return;

    if (!M_Responder(ev))
        if (!(_g_gamestate == GS_LEVEL && (C_Responder(ev) || AM_Responder(ev))))
            G_Responder(ev);
}


static void D_BuildNewTiccmds(void)
{
    static int32_t lastmadetic = 0;
    int16_t newtics = I_GetTime() - lastmadetic;
    lastmadetic += newtics;

    while (newtics--)
    {
        I_StartTic();
        if ((int16_t)(maketic - _g_gametic) > 3)
            break;

        G_BuildTiccmd();
        maketic++;
    }
}


//
// D_Display
//  draw current display, possibly wiping it from the previous
//

gamestate_t wipegamestate = GS_DEMOSCREEN; // wipegamestate can be set to -1 to force a wipe on the next draw

static void D_Display (void)
{
    static gamestate_t oldgamestate = GS_LEVEL;

    if (nodrawers)                    // for comparative timing / profiling
        return;

    // save the current screen if about to wipe
    boolean wipe = (_g_gamestate != wipegamestate);

    if (wipe)
        wipe_StartScreen();

    if (_g_gamestate != GS_LEVEL) { // Not a level
        if (oldgamestate == GS_LEVEL)
            I_SetPalette(0); // cph - use default (basic) palette

        switch (_g_gamestate)
        {
            case GS_INTERMISSION:
                WI_Drawer();
                break;
            case GS_FINALE:
                F_Drawer();
                break;
            case GS_DEMOSCREEN:
                D_PageDrawer();
                break;
            default:
                break;
        }
    }
    else if (_g_gametic != _g_basetic)
    { // In a level

        // Work out if the player view is visible, and if there is a border
        boolean viewactive = (!(automapmode & am_active) || (automapmode & am_overlay));

        // Now do the drawing
        if (viewactive)
            R_RenderPlayerView (&_g_player);

        if (automapmode & am_active)
            AM_Drawer();

        ST_doPaletteStuff();  // Do red-/gold-shifts from damage/items
        ST_Drawer();

        HU_Drawer();
    }

    oldgamestate = wipegamestate = _g_gamestate;

    // menus go directly to the screen
    M_Drawer();          // menu is drawn even on top of everything

    D_BuildNewTiccmds();

    if (wipe)
        // wipe update
        D_Wipe();
    else
        // normal update
        I_FinishUpdate ();              // page flip or blit buffer
}


//? how many ticks to run?
static void TryRunTics (void)
{
    int16_t runtics;
    int32_t entertime = I_GetTime();

    // Wait for tics to run
    while (1)
    {

        D_BuildNewTiccmds();

        runtics = maketic - _g_gametic;
        if (runtics <= 0)
        {
            if ((int16_t)(I_GetTime() - entertime) > 10)
            {
                M_Ticker();
                return;
            }
        }
        else
            break;
    }

    while (runtics-- > 0)
    {

        if (advancedemo)
            D_DoAdvanceDemo ();

        M_Ticker ();
        G_Ticker ();
        _g_gametic++;
    }
}


//
//  D_DoomLoop()
//
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime and I_StartTic
//
_Noreturn static void D_DoomLoop(void)
{
    for (;;)
    {
        // frame syncronous IO operations

        // process one or more tics
        if (singletics)
        {
            I_StartTic ();
            G_BuildTiccmd ();

            if (advancedemo)
                D_DoAdvanceDemo ();

            M_Ticker ();
            G_Ticker ();

            _g_gametic++;
            maketic++;
        }
        else
            TryRunTics (); // will run at least one tic

        // killough 3/16/98: change consoleplayer to displayplayer
        if (_g_player.mo) // cph 2002/08/10
            S_UpdateSounds();// move positional sounds

        // Update display, next frame, with current state.
        D_Display();


        if(_g_fps_show)
        {
            D_UpdateFPS();
        }
    }
}

static void D_UpdateFPS()
{
    static uint32_t fps_frames = 0;
    static uint32_t fps_timebefore = 0;

    fps_frames++;

    uint32_t timenow = I_GetTime();
    if(timenow >= (fps_timebefore + TICRATE))
    {
        uint32_t tics_elapsed = timenow - fps_timebefore;
        fixed_t f_realfps = FixedApproxDiv((fps_frames*(TICRATE*10)) << FRACBITS, tics_elapsed <<FRACBITS);

        _g_fps_framerate = (f_realfps >> FRACBITS);

        fps_frames = 0;
        fps_timebefore = timenow;
    }
    else if(timenow < fps_timebefore)
    {
        //timer overflow.
        fps_timebefore = timenow;
        fps_frames = 0;
    }
}

//
//  DEMO LOOP
//


//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker(void)
{
    if (--pagetic < 0)
        D_AdvanceDemo();
}

//
// D_PageDrawer
//

static void D_PageDrawer(void)
{
	V_DrawRawFullScreen(titlepicnum);
}

//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    advancedemo = true;
}


static void D_DrawTitle1(const char *name)
{
	UNUSED(name);

	S_StartMusic(mus_intro);
	pagetic = (TICRATE*30);
}


static struct
{
    void (*func)(const char *);
    const char *name;
}
const demostates[] =
{
    {D_DrawTitle1, NULL},
    {G_DeferedPlayDemo, "demo3"},
    {NULL, NULL},
};

static int16_t  demosequence;

/*
 * This cycles through the demo sequences.
 */

static void D_DoAdvanceDemo(void)
{
    _g_player.playerstate = PST_LIVE;  /* not reborn */
    advancedemo = _g_usergame = false;
    _g_gameaction = ga_nothing;

    pagetic = TICRATE * 11;
    _g_gamestate = GS_DEMOSCREEN;


    if (!demostates[++demosequence].func)
        demosequence = 0;

    demostates[demosequence].func(demostates[demosequence].name);
}

//
// D_StartTitle
//
void D_StartTitle (void)
{
    _g_gameaction = ga_nothing;
    demosequence = -1;
    D_AdvanceDemo();
}


/*
=================
=
= M_CheckParm
=
= Checks for the given parameter in the program's command line arguments
=
= Returns the argument number (1 to argc - 1) or 0 if not present
=
=================
*/

static int myargc;
static const char * const * myargv;

int16_t M_CheckParm(char *check)
{
	for (int16_t i = 1; i < myargc; i++)
		if (!stricmp(check, myargv[i]))
			return i;

	return 0;
}


static void D_Init(void)
{
	titlepicnum = W_GetNumForName("TITLEPIC");
}


//
// D_DoomMainSetup
//
// CPhipps - the old contents of D_DoomMain, but moved out of the main
//  line of execution so its stack space can be freed

static void D_DoomMainSetup(void)
{
    // init subsystems
    I_InitKeyboard();

    I_InitTimer();

    I_InitSound();

    printf("Z_Init: Init zone memory allocation daemon.\n");
    Z_Init();

    G_ReloadDefaults();    // killough 3/4/98: set defaults just loaded.

    printf("W_Init: Init WADfiles.\n");
    W_Init(); // CPhipps - handling of wadfiles init changed

    I_InitSound2();

    D_Init();
    F_Init();
    WI_Init();

    printf("M_Init: Init miscellaneous info.\n");
    M_Init();

    printf("R_Init: DOOM refresh daemon - [...................]\n");
    R_Init();

    printf("P_Init: Init Playloop state.\n");
    P_Init();

    S_Init(snd_SfxVolume /* *8 */, snd_MusicVolume /* *8*/ );

    printf("HU_Init: Setting up heads up display.\n");
    HU_Init();

    printf("ST_Init: Init status bar.\n");
    ST_Init();

    G_LoadSettings();

    _g_fps_show = false;

    I_InitGraphics();

    int16_t p = M_CheckParm("-timedemo");
    if (p && p < myargc - 1)
    {
        singletics = true;
        _g_timingdemo = true;            // show stats after quit
        G_DeferedPlayDemo(myargv[p + 1]);
        _g_singledemo = true;            // quit after one demo
    }
    else
    {
        D_StartTitle();                 // start up intro loop
    }
}

//
// D_DoomMain
//

void D_DoomMain(int argc, const char * const * argv)
{
    myargc = argc;
    myargv = argv;

    D_DoomMainSetup();

    D_DoomLoop ();  // never returns
}
