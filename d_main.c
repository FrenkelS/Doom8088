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
 * DESCRIPTION:
 *  DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
 *  plus functions to
 *  parse command line parameters, configure game parameters (turbo),
 *  and call the startup functions.
 *
 *-----------------------------------------------------------------------------
 */



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include "doomdef.h"
#include "doomtype.h"
#include "doomstat.h"
#include "d_net.h"
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

static void D_PageDrawer(void);
static void D_UpdateFPS(void);


// CPhipps - removed wadfiles[] stuff


//jff 1/22/98 parms for disabling music and sound
const boolean nosfxparm = true;
const boolean nomusicparm = true;

const boolean nodrawers = false;


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
    if (_g->gametic < 3)
        return;

    M_Responder(ev) ||
            (_g->gamestate == GS_LEVEL && (
                 C_Responder(ev) ||
                 ST_Responder(ev) ||
                 AM_Responder(ev)
                 )
             ) ||
            G_Responder(ev);

}


//
// D_Display
//  draw current display, possibly wiping it from the previous
//

static void D_Display (void)
{

    boolean wipe;
    boolean viewactive = false;

    if (nodrawers)                    // for comparative timing / profiling
        return;

    I_StartDisplay();

    // save the current screen if about to wipe
    wipe = (_g->gamestate != _g->wipegamestate);

    if (wipe)
        wipe_StartScreen();

    if (_g->gamestate != GS_LEVEL) { // Not a level
        switch (_g->oldgamestate)
        {
            case -1:
            case GS_LEVEL:
                I_SetPalette(0); // cph - use default (basic) palette
            default:
                break;
        }

        switch (_g->gamestate)
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
    else if (_g->gametic != _g->basetic)
    { // In a level

        HU_Erase();

        // Work out if the player view is visible, and if there is a border
        viewactive = (!(_g->automapmode & am_active) || (_g->automapmode & am_overlay));

        // Now do the drawing
        if (viewactive)
            R_RenderPlayerView (&_g->player);

        if (_g->automapmode & am_active)
            AM_Drawer();

        ST_Drawer();

        HU_Drawer();
    }

    _g->oldgamestate = _g->wipegamestate = _g->gamestate;

    // menus go directly to the screen
    M_Drawer();          // menu is drawn even on top of everything

    D_BuildNewTiccmds();

    if (!wipe)
        // normal update
        I_FinishUpdate ();              // page flip or blit buffer
    else
        // wipe update
        D_Wipe();
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

static void D_DoomLoop(void)
{
    for (;;)
    {
        // frame syncronous IO operations

        // process one or more tics
        if (_g->singletics)
        {
            I_StartTic ();
            G_BuildTiccmd (&_g->netcmd);

            if (_g->advancedemo)
                D_DoAdvanceDemo ();

            M_Ticker ();
            G_Ticker ();

            _g->gametic++;
            _g->maketic++;
        }
        else
            TryRunTics (); // will run at least one tic

        // killough 3/16/98: change consoleplayer to displayplayer
        if (_g->player.mo) // cph 2002/08/10
            S_UpdateSounds();// move positional sounds

        // Update display, next frame, with current state.
        D_Display();


        if(_g->fps_show)
        {
            D_UpdateFPS();
        }
    }
}

static void D_UpdateFPS()
{
    _g->fps_frames++;

    uint32_t timenow = I_GetTime();
    if(timenow >= (_g->fps_timebefore + TICRATE))
    {
        uint32_t tics_elapsed = timenow - _g->fps_timebefore;
        fixed_t f_realfps = FixedDiv((_g->fps_frames*(TICRATE*10)) << FRACBITS, tics_elapsed <<FRACBITS);

        _g->fps_framerate = (f_realfps >> FRACBITS);

        _g->fps_frames = 0;
        _g->fps_timebefore = timenow;
    }
    else if(timenow < _g->fps_timebefore)
    {
        //timer overflow.
        _g->fps_timebefore = timenow;
        _g->fps_frames = 0;
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
    if (--_g->pagetic < 0)
        D_AdvanceDemo();
}

//
// D_PageDrawer
//

static void D_PageDrawer(void)
{
	W_ReadLumpByName("TITLEPIC", _g->screen);
}

//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    _g->advancedemo = true;
}

/* killough 11/98: functions to perform demo sequences
 * cphipps 10/99: constness fixes
 */

static void D_SetPageName(const char *name)
{
	UNUSED(name);
}

static void D_DrawTitle1(const char *name)
{
	UNUSED(name);

	S_StartMusic(mus_intro);
	_g->pagetic = (TICRATE*30);
}


/* killough 11/98: tabulate demo sequences
 */

static struct
{
    void (*func)(const char *);
    const char *name;
}
const demostates[] =
{
    {D_DrawTitle1, NULL},
    {G_DeferedPlayDemo, "demo1"},
    {D_SetPageName, NULL},
    {G_DeferedPlayDemo, "demo2"},
    {D_SetPageName, NULL},
    {G_DeferedPlayDemo, "demo3"},
    {NULL, NULL},
};

/*
 * This cycles through the demo sequences.
 * killough 11/98: made table-driven
 */

void D_DoAdvanceDemo(void)
{
    _g->player.playerstate = PST_LIVE;  /* not reborn */
    _g->advancedemo = _g->usergame = false;
    _g->gameaction = ga_nothing;

    _g->pagetic = TICRATE * 11;         /* killough 11/98: default behavior */
    _g->gamestate = GS_DEMOSCREEN;


    if (!demostates[++_g->demosequence].func)
        _g->demosequence = 0;

    demostates[_g->demosequence].func(demostates[_g->demosequence].name);
}

//
// D_StartTitle
//
void D_StartTitle (void)
{
    _g->gameaction = ga_nothing;
    _g->demosequence = -1;
    D_AdvanceDemo();
}


static void LoadIWAD(void)
{
	// IWAD
	_g->fileWAD = fopen("DOOM1.WAD", "rb");
	if (_g->fileWAD == NULL)
		I_Error("Can't open DOOM1.WAD.");

	// DistScale
	_g->fileDistScale = fopen("DISTSCAL.LMP", "rb");
	if (_g->fileDistScale == NULL)
		I_Error("Can't open DISTSCAL.LMP.");

	// FineSine
	_g->fileFineSine = fopen("FINESINE.LMP", "rb");
	if (_g->fileFineSine == NULL)
		I_Error("Can't open FINESINE.LMP.");

	// FineTan
	_g->fileFineTan = fopen("FINETAN.LMP", "rb");
	if (_g->fileFineTan == NULL)
		I_Error("Can't open FINETAN.LMP.");

	// TanToAngle
	_g->fileTanToAngle = fopen("TAN2ANG.LMP", "rb");
	if (_g->fileTanToAngle == NULL)
		I_Error("Can't open TAN2ANG.LMP.");

	// ViewAngleToX
	_g->fileViewAngleToX = fopen("VIEWANGX.LMP", "rb");
	if (_g->fileViewAngleToX == NULL)
		I_Error("Can't open VIEWANGX.LMP.");

	// XToViewAngle
	_g->fileXToViewAngle = fopen("XVIEWANG.LMP", "rb");
	if (_g->fileXToViewAngle == NULL)
		I_Error("Can't open XVIEWANG.LMP.");

	// YSlope
	_g->fileYSlope = fopen("YSLOPE.LMP", "rb");
	if (_g->fileYSlope == NULL)
		I_Error("Can't open YSLOPE.LMP.");
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

int myargc;
const char * const * myargv;

static int16_t M_CheckParm(char *check)
{
	for (int16_t i = 1; i < myargc; i++)
		if (!stricmp(check, myargv[i]))
			return i;

	return 0;
}


//
// D_DoomMainSetup
//
// CPhipps - the old contents of D_DoomMain, but moved out of the main
//  line of execution so its stack space can be freed

static void D_DoomMainSetup(void)
{
    LoadIWAD();

    // init subsystems

    G_ReloadDefaults();    // killough 3/4/98: set defaults just loaded.

    // CPhipps - move up netgame init
    D_InitNetGame();

    printf("W_Init: Init WADfiles.\n");
    W_Init(); // CPhipps - handling of wadfiles init changed

    printf("M_Init: Init miscellaneous info.\n");
    M_Init();

    printf("R_Init: DOOM refresh daemon - [...................]\n");
    R_Init();

    printf("P_Init: Init Playloop state.\n");
    P_Init();

    S_Init(_g->snd_SfxVolume /* *8 */, _g->snd_MusicVolume /* *8*/ );

    printf("HU_Init: Setting up heads up display.\n");
    HU_Init();

    printf("ST_Init: Init status bar.\n");
    ST_Init();

    _g->highDetail = false;

    G_LoadSettings();

    _g->fps_show = false;

    I_InitGraphics();

    int16_t p = M_CheckParm("-timedemo");
    if (p && p < myargc - 1)
    {
        _g->singletics = true;
        _g->timingdemo = true;            // show stats after quit
        G_DeferedPlayDemo(myargv[p + 1]);
        _g->singledemo = true;            // quit after one demo
    }
    else
    {
        D_StartTitle();                 // start up intro loop
    }
}

//
// D_DoomMain
//

void D_DoomMain(void)
{
    D_DoomMainSetup(); // CPhipps - setup out of main execution stack

    D_DoomLoop ();  // never returns
}
