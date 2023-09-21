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
 *  DOOM selection menu, options etc. (aka Big Font menus)
 *  Sliders and icons. Kinda widget stuff.
 *  Setup Menus.
 *  Extended HELP screens.
 *  Dynamic HELP screen.
 *
 *-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <fcntl.h>

#include "doomdef.h"
#include "d_player.h"
#include "d_englsh.h"
#include "d_main.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_main.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "m_menu.h"
#include "doomtype.h"
#include "am_map.h"
#include "i_system.h"
#include "i_sound.h"

#include "globdata.h"


int32_t showMessages;


static void (*messageRoutine)(int32_t response);

// we are going to be entering a savegame string

#define SKULLXOFF  -32
#define LINEHEIGHT  16

// graphic name of skulls

static const char skullName[2][9] = {"M_SKULL1","M_SKULL2"};


// end of externs added for setup menus

//
// PROTOTYPES
//
static void M_NewGame(int32_t choice);
static void M_ChooseSkill(int32_t choice);
static void M_LoadGame(int32_t choice);
static void M_SaveGame(int32_t choice);
static void M_Options(int32_t choice);
static void M_EndGame(int32_t choice);


static void M_ChangeMessages(int32_t choice);
static void M_ChangeAlwaysRun(int32_t choice);
static void M_ChangeDetail(int32_t choice);
static void M_ChangeGamma(int32_t choice);
static void M_SfxVol(int32_t choice);
static void M_MusicVol(int32_t choice);
void M_StartGame(int32_t choice);
static void M_Sound(int32_t choice);

static void M_LoadSelect(int32_t choice);
static void M_SaveSelect(int32_t choice);
static void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

static void M_DrawMainMenu(void);
static void M_DrawNewGame(void);
static void M_DrawOptions(void);
static void M_DrawSound(void);
static void M_DrawLoad(void);
static void M_DrawSave(void);

static void M_SetupNextMenu(const menu_t *menudef);
static void M_DrawThermo(int16_t x, int16_t y, int16_t thermWidth, int16_t thermDot);
static void M_WriteText(int16_t x, int16_t y, const char *string);
static int16_t M_StringWidth(const char *string);
static int16_t M_StringHeight(const char *string);
static void M_StartMessage(const char *string,void (*routine)(int32_t),boolean input);
static void M_ClearMenus (void);

// phares 3/30/98
// prototypes added to support Setup Menus and Extended HELP screens

void M_Setup(int32_t choice);


// end of prototypes added to support Setup Menus and Extended HELP screens

/////////////////////////////////////////////////////////////////////////////
//
// DOOM MENUS
//

/////////////////////////////
//
// MAIN MENU
//

// main_e provides numerical values for which Big Font screen you're on

enum
{
  newgame = 0,
  loadgame,
  savegame,
  options,
  main_end
};

//
// MainMenu is the definition of what the main menu Screen should look
// like. Each entry shows that the cursor can land on each item (1), the
// built-in graphic lump (i.e. "M_NGAME") that should be displayed,
// the program which takes over when an item is selected, and the hotkey
// associated with the item.
//

static const menuitem_t MainMenu[]=
{
  {1,"M_NGAME", M_NewGame},
  {1,"M_OPTION",M_Options},
  {1,"M_LOADG", M_LoadGame},
  {1,"M_SAVEG", M_SaveGame},
};

static const menu_t MainDef =
{
  main_end,       // number of menu items
  MainMenu,       // table that defines menu items
  M_DrawMainMenu, // drawing routine
  97,64,          // initial cursor position
  NULL,0,
};

//
// M_DrawMainMenu
//

static void M_DrawMainMenu(void)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(94, 2, "M_DOOM");
}


// numerical values for the New Game menu items

enum
{
  killthings,
  toorough,
  hurtme,
  violence,
  nightmare,
  newg_end
};

// The definitions of the New Game menu

static const menuitem_t NewGameMenu[]=
{
  {1,"M_JKILL", M_ChooseSkill},
  {1,"M_ROUGH", M_ChooseSkill},
  {1,"M_HURT",  M_ChooseSkill},
  {1,"M_ULTRA", M_ChooseSkill},
  {1,"M_NMARE", M_ChooseSkill}
};

static const menu_t NewDef =
{
  newg_end,       // # of menu items
  NewGameMenu,    // menuitem_t ->
  M_DrawNewGame,  // drawing routine ->
  48,63,          // x,y
  &MainDef,0,
};


//
// M_NewGame
//

static void M_DrawNewGame(void)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(96, 14, "M_NEWG");
  V_DrawNamePatch(54, 38, "M_SKILL");
}

static void M_NewGame(int32_t choice)
{
	UNUSED(choice);

	M_SetupNextMenu(&NewDef);
	_g->itemOn = 2; //Set hurt me plenty as default difficulty
}

// CPhipps - static
static void M_VerifyNightmare(int32_t ch)
{
    if (ch != key_enter)
        return;

    G_DeferedInitNew(nightmare);
}

static void M_ChooseSkill(int32_t choice)
{
    if (choice == nightmare)
    {   // Ty 03/27/98 - externalized
        M_StartMessage(NIGHTMARE,M_VerifyNightmare,true);
		_g->itemOn = 0;
    }
    else
    {
        G_DeferedInitNew(choice);
		M_ClearMenus ();
    }    
}

/////////////////////////////
//
// LOAD GAME MENU
//

// numerical values for the Load Game slots

enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7,
    load8,
    load_end
};

// The definitions of the Load Game screen

static const menuitem_t LoadMenue[]=
{
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
    {1,"", M_LoadSelect},
};

static const menu_t LoadDef =
{
  load_end,
  LoadMenue,
  M_DrawLoad,
  64,34, //jff 3/15/98 move menu up
  &MainDef,2,
};

#define LOADGRAPHIC_Y 8

//
// M_LoadGame & Cie.
//

static void M_DrawSaveLoad(const char* name)
{
	int8_t i, j;

	V_DrawNamePatch(72 ,LOADGRAPHIC_Y, name);

	const patch_t* lpatch = W_GetLumpByName("M_LSLEFT");
	const patch_t* mpatch = W_GetLumpByName("M_LSCNTR");
	const patch_t* rpatch = W_GetLumpByName("M_LSRGHT");

	for (i = 0; i < load_end; i++)
	{
		//
		// Draw border for the savegame description
		//
		int16_t x = LoadDef.x;
		const int16_t y = 27 + 13 * i + 7;
		V_DrawPatchNoScale(x - 8, y, lpatch);
		for (j = 0; j < 12; j++)
		{
			V_DrawPatchNoScale(x, y, mpatch);
			x += 8;
		}
		V_DrawPatchNoScale(x, y, rpatch);

		M_WriteText(LoadDef.x, y - 7, _g->savegamestrings[i]);
	}

	Z_ChangeTagToCache(lpatch);
	Z_ChangeTagToCache(mpatch);
	Z_ChangeTagToCache(rpatch);
}

static void M_DrawLoad(void)
{
	M_DrawSaveLoad("M_LOADG");
}


//
// User wants to load this game
//

static void M_LoadSelect(int32_t choice)
{
  // CPhipps - Modified so savegame filename is worked out only internal
  //  to g_game.c, this only passes the slot.

  G_LoadGame(choice); // killough 3/16/98: add slot

  M_ClearMenus ();
}

//
// Selected from DOOM menu
//

static void M_LoadGame (int32_t choice)
{
	UNUSED(choice);

	/* killough 5/26/98: exclude during demo recordings
	 * cph - unless a new demo */

	M_SetupNextMenu(&LoadDef);
	M_ReadSaveStrings();
}

/////////////////////////////
//
// SAVE GAME MENU
//

// The definitions of the Save Game screen

const static menuitem_t SaveMenu[]=
{
  {1,"", M_SaveSelect},
  {1,"", M_SaveSelect},
  {1,"", M_SaveSelect},
  {1,"", M_SaveSelect},
  {1,"", M_SaveSelect},
  {1,"", M_SaveSelect},
  {1,"", M_SaveSelect}, //jff 3/15/98 extend number of slots
  {1,"", M_SaveSelect},
};

const static menu_t SaveDef =
{
  load_end, // same number of slots as the Load Game screen
  SaveMenu,
  M_DrawSave,
  80,34, //jff 3/15/98 move menu up
  &MainDef,3,
};

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
static void M_ReadSaveStrings(void)
{

}

//
//  M_SaveGame & Cie.
//
static void M_DrawSave(void)
{
	M_DrawSaveLoad("M_SAVEG");
}

//
// M_Responder calls this when user is finished
//
static void M_DoSave(int32_t slot)
{
  G_SaveGame (slot);
  M_ClearMenus ();
}

//
// User wants to save. Start string input for M_Responder
//
static void M_SaveSelect(int32_t choice)
{
    _g->saveSlot = choice;

    M_DoSave(_g->saveSlot);
}

//
// Selected from DOOM menu
//
static void M_SaveGame (int32_t choice)
{
	UNUSED(choice);

	// killough 10/6/98: allow savegames during single-player demo playback
	if (!_g->usergame && (!_g->demoplayback))
	{
		M_StartMessage(SAVEDEAD,NULL,false); // Ty 03/27/98 - externalized
		return;
	}

	if (_g->gamestate != GS_LEVEL)
		return;

	M_SetupNextMenu(&SaveDef);
	M_ReadSaveStrings();
}

/////////////////////////////
//
// OPTIONS MENU
//

// numerical values for the Options menu items

enum
{                                                   // phares 3/21/98
  endgame,
  messages,
  alwaysrun,
  detail,
  gamma,
  soundvol,
  opt_end
};

// The definitions of the Options menu

static const menuitem_t OptionsMenu[]=
{
  // killough 4/6/98: move setup to be a sub-menu of OPTIONs
  {1,"M_ENDGAM", M_EndGame},
  {1,"M_MESSG",  M_ChangeMessages},
  {1,"M_ARUN",   M_ChangeAlwaysRun},
  {1,"M_DETAIL", M_ChangeDetail},
  {2,"M_GAMMA",   M_ChangeGamma},
  {1,"M_SVOL",   M_Sound}
};

const static menu_t OptionsDef =
{
  opt_end,
  OptionsMenu,
  M_DrawOptions,
  60,37,
  &MainDef,1,
};

//
// M_Options
//
static const char msgNames[2][9]  = {"M_MSGOFF","M_MSGON"};
static const char detailNames[2][9]  = {"M_GDLOW","M_GDHIGH"};


static void M_DrawOptions(void)
{
  // CPhipps - patch drawing updated
  // proff/nicolas 09/20/98 -- changed for hi-res
  V_DrawNamePatch(108, 15, "M_OPTTTL");

  V_DrawNamePatch(OptionsDef.x + 120, OptionsDef.y+LINEHEIGHT*messages, msgNames[showMessages]);

  V_DrawNamePatch(OptionsDef.x + 146, OptionsDef.y+LINEHEIGHT*alwaysrun, msgNames[_g->alwaysRun]);

  V_DrawNamePatch(OptionsDef.x + 176, OptionsDef.y+LINEHEIGHT*detail, detailNames[_g->highDetail]);

  M_DrawThermo(OptionsDef.x + 158, OptionsDef.y+LINEHEIGHT*gamma+2,6,_g->gamma);
}

static void M_Options(int32_t choice)
{
	UNUSED(choice);

	M_SetupNextMenu(&OptionsDef);
}

/////////////////////////////
//
// SOUND VOLUME MENU
//

// numerical values for the Sound Volume menu items
// The 'empty' slots are where the sliding scales appear.

enum
{
  sfx_vol,
  sfx_empty1,
  music_vol,
  sfx_empty2,
  sound_end
};

// The definitions of the Sound Volume menu

static const menuitem_t SoundMenu[]=
{
  {2,"M_SFXVOL",M_SfxVol},
  {-1,"",0},
  {2,"M_MUSVOL",M_MusicVol},
  {-1,"",0}
};

static const menu_t SoundDef =
{
  sound_end,
  SoundMenu,
  M_DrawSound,
  80,64,
  &OptionsDef,4,
};

//
// Change Sfx & Music volumes
//

static void M_DrawSound(void)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(60, 38, "M_SVOL");

  M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (sfx_vol   + 1), 16, snd_SfxVolume);

  M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (music_vol + 1), 16, snd_MusicVolume);
}

static void M_Sound(int32_t choice)
{
	UNUSED(choice);

	M_SetupNextMenu(&SoundDef);
}

static void M_SfxVol(int32_t choice)
{
  switch(choice)
    {
    case 0:
      if (snd_SfxVolume)
        snd_SfxVolume--;
      break;
    case 1:
      if (snd_SfxVolume < 15)
        snd_SfxVolume++;
      break;
    }

  G_SaveSettings();

  S_SetSfxVolume(snd_SfxVolume /* *8 */);
}

static void M_MusicVol(int32_t choice)
{
  switch(choice)
    {
    case 0:
      if (snd_MusicVolume)
        snd_MusicVolume--;
      break;
    case 1:
      if (snd_MusicVolume < 15)
        snd_MusicVolume++;
      break;
    }

  G_SaveSettings();

  S_SetMusicVolume(snd_MusicVolume /* *8 */);
}

/////////////////////////////
//
// M_EndGame
//

static void M_EndGameResponse(int32_t ch)
{
  if (ch != key_enter)
    return;

  // killough 5/26/98: make endgame quit if recording or playing back demo
  if (_g->singledemo)
    G_CheckDemoStatus();

  M_ClearMenus ();
  D_StartTitle ();
}

static void M_EndGame(int32_t choice)
{
	UNUSED(choice);

	M_StartMessage(ENDGAME,M_EndGameResponse,true); // Ty 03/27/98 - externalized
}

/////////////////////////////
//
//    Toggle messages on/off
//

static void M_ChangeMessages(int32_t choice)
{
  UNUSED(choice);

  showMessages = 1 - showMessages;

  _g->player.message = showMessages ? MSGON : MSGOFF;

  _g->message_dontfuckwithme = true;

  G_SaveSettings();
}


static void M_ChangeAlwaysRun(int32_t choice)
{
    UNUSED(choice);

    _g->alwaysRun = 1 - _g->alwaysRun;

    if (!_g->alwaysRun)
      _g->player.message = RUNOFF; // Ty 03/27/98 - externalized
    else
      _g->player.message = RUNON ; // Ty 03/27/98 - externalized

    G_SaveSettings();
}

static void M_ChangeDetail(int32_t choice)
{
    UNUSED(choice);

    _g->highDetail = 1 - _g->highDetail;

    if (!_g->highDetail)
      _g->player.message = LOWDETAIL; // Ty 03/27/98 - externalized
    else
      _g->player.message = HIGHDETAIL ; // Ty 03/27/98 - externalized

    G_SaveSettings();
}

static void M_ChangeGamma(int32_t choice)
{
	switch(choice)
    {
		case 0:
		  if (_g->gamma)
			_g->gamma--;
		  break;
		case 1:
		  if (_g->gamma < 5)
			_g->gamma++;
		  break;
    }
	I_SetPalette(0);

    G_SaveSettings();
}

//
// End of Original Menus
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////
//
// General routines used by the Setup screens.
//

//
// M_InitDefaults()
//
// killough 11/98:
//
// This function converts all setup menu entries consisting of cfg
// variable names, into pointers to the corresponding default[]
// array entry. var.name becomes converted to var.def.
//

static void M_InitDefaults(void)
{

}

//
// End of Setup Screens.
//
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//
// M_Responder
//
// Examines incoming keystrokes and button pushes and determines some
// action based on the state of the system.
//

boolean M_Responder (event_t* ev)
{
    int32_t    ch;

    ch = -1; // will be changed to a legit char if we're going to use it here


    // Mouse input processing removed

    // Process keyboard input

    if (ev->type == ev_keydown)
    {
        ch = ev->data1;               // phares 4/11/98:
    }                             // down so you can get at the !,#,


    if (ch == -1)
        return false; // we can't use the event here

    // Take care of any messages that need input

    if (_g->messageToPrint)
    {
        if (_g->messageNeedsInput == true &&
                !(ch == ' ' || ch == 'n' || ch == 'y' || ch == key_escape || ch == key_fire || ch == key_enter)) // phares
            return false;

        _g->menuactive     = _g->messageLastMenuActive;
        _g->messageToPrint = false;
        if (messageRoutine)
            messageRoutine(ch);

        _g->menuactive = false;
        S_StartSound(NULL,sfx_swtchx);
        return true;
    }

    // Pop-up Main menu?

    if (!_g->menuactive)
    {
        if (ch == key_escape)                                     // phares
        {
            M_StartControlPanel ();
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        return false;
    }

    // From here on, these navigation keys are used on the BIG FONT menus
    // like the Main Menu.

    if (ch == key_menu_down)                             // phares 3/7/98
    {
        do
        {
            if (_g->itemOn+1 > _g->currentMenu->numitems-1)
                _g->itemOn = 0;
            else
                _g->itemOn++;
            S_StartSound(NULL,sfx_pstop);
        }
        while(_g->currentMenu->menuitems[_g->itemOn].status==-1);
        return true;
    }

    if (ch == key_menu_up)                               // phares 3/7/98
    {
        do
        {
            if (!_g->itemOn)
                _g->itemOn = _g->currentMenu->numitems-1;
            else
                _g->itemOn--;
            S_StartSound(NULL,sfx_pstop);
        }
        while(_g->currentMenu->menuitems[_g->itemOn].status==-1);
        return true;
    }

    if (ch == key_menu_left)                             // phares 3/7/98
    {
        if (_g->currentMenu->menuitems[_g->itemOn].routine &&
                _g->currentMenu->menuitems[_g->itemOn].status == 2)
        {
            S_StartSound(NULL,sfx_stnmov);
            _g->currentMenu->menuitems[_g->itemOn].routine(0);
        }
        return true;
    }

    if (ch == key_menu_right)                            // phares 3/7/98
    {
        if (_g->currentMenu->menuitems[_g->itemOn].routine &&
                _g->currentMenu->menuitems[_g->itemOn].status == 2)
        {
            S_StartSound(NULL,sfx_stnmov);
            _g->currentMenu->menuitems[_g->itemOn].routine(1);
        }
        return true;
    }

    if (ch == key_menu_enter)                            // phares 3/7/98
    {
        if (_g->currentMenu->menuitems[_g->itemOn].routine &&
                _g->currentMenu->menuitems[_g->itemOn].status)
        {
            if (_g->currentMenu->menuitems[_g->itemOn].status == 2)
            {
                _g->currentMenu->menuitems[_g->itemOn].routine(1);   // right arrow
                S_StartSound(NULL,sfx_stnmov);
            }
            else
            {
                _g->currentMenu->menuitems[_g->itemOn].routine(_g->itemOn);
                S_StartSound(NULL,sfx_pistol);
            }
        }
        //jff 3/24/98 remember last skill selected
        // killough 10/98 moved to skill-specific functions
        return true;
    }

    if (ch == key_menu_escape)                           // phares 3/7/98
    {
        M_ClearMenus ();
        S_StartSound(NULL,sfx_swtchx);
        return true;
    }

	//Allow being able to go back in menus ~Kippykip
	if (ch == key_fire)                           // phares 3/7/98
    {
		//If the prevMenu == NULL (Such as main menu screen), then just get out of the menu altogether
		if(_g->currentMenu->prevMenu == NULL)
		{
			M_ClearMenus();
		}else //Otherwise, change to the parent menu and match the row used to get there.
		{
			int16_t previtemOn = _g->currentMenu->previtemOn; //Temporarily store this so after menu change, we store the last row it was on.
			M_SetupNextMenu(_g->currentMenu->prevMenu);					
			_g->itemOn = previtemOn;
		}
        S_StartSound(NULL,sfx_swtchx);
        return true;		
    }
	
    return false;
}

//
// End of M_Responder
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// General Routines
//
// This displays the Main menu and gets the menu screens rolling.
// Plus a variety of routines that control the Big Font menu display.
// Plus some initialization for game-dependant situations.

void M_StartControlPanel (void)
{
  // intro might call this repeatedly

  if (_g->menuactive)
    return;

  //jff 3/24/98 make default skill menu choice follow -skill or defaultskill
  //from command line or config file
  //
  // killough 10/98:
  // Fix to make "always floating" with menu selections, and to always follow
  // defaultskill, instead of -skill.

  _g->menuactive = true;
  _g->currentMenu = &MainDef;         // JDC
}


static char* Z_Strdup(const char* s)
{
    const uint32_t len = strlen(s);

    if(!len)
        return NULL;

    char* ptr = Z_MallocStatic(len+1);

    if(ptr)
        strcpy(ptr, s);

    return ptr;
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
// killough 9/29/98: Significantly reformatted source
//

void M_Drawer (void)
{
    // Horiz. & Vertically center string and print it.
    // killough 9/29/98: simplified code, removed 40-character width limit
    if (_g->messageToPrint)
    {
        /* cph - strdup string to writable memory */
        char *ms = Z_Strdup(_g->messageString);
        char *p = ms;

        int16_t y = 80 - M_StringHeight(_g->messageString)/2;
        while (*p)
        {
            char *string = p, c;
            while ((c = *p) && *p != '\n')
                p++;
            *p = 0;
            M_WriteText(120 - M_StringWidth(string)/2, y, string);
            y += HU_FONT_HEIGHT;
            if ((*p = c))
                p++;
        }
        Z_Free(ms);
    }
    else
        if (_g->menuactive)
        {
            int32_t x,y,max,i;

            if (_g->currentMenu->routine)
                _g->currentMenu->routine();     // call Draw routine

            // DRAW MENU

            x = _g->currentMenu->x;
            y = _g->currentMenu->y;
            max = _g->currentMenu->numitems;

            for (i=0;i<max;i++)
            {
                if (_g->currentMenu->menuitems[i].name[0])
                    V_DrawNamePatch(x,y,_g->currentMenu->menuitems[i].name);
                y += LINEHEIGHT;
            }

            // DRAW SKULL

            // CPhipps - patch drawing updated
            V_DrawNamePatch(x + SKULLXOFF, _g->currentMenu->y - 5 + _g->itemOn*LINEHEIGHT, skullName[_g->whichSkull]);
        }
}

//
// M_ClearMenus
//
// Called when leaving the menu screens for the real world

static void M_ClearMenus (void)
{
  _g->menuactive = false;
  _g->itemOn = 0;
}

//
// M_SetupNextMenu
//
static void M_SetupNextMenu(const menu_t *menudef)
{
  _g->currentMenu = menudef;
  _g->itemOn = 0;
}

/////////////////////////////
//
// M_Ticker
//
void M_Ticker (void)
{
  if (--_g->skullAnimCounter <= 0)
    {
      _g->whichSkull ^= 1;
      _g->skullAnimCounter = 8;
    }
}

/////////////////////////////
//
// Message Routines
//

static void M_StartMessage (const char* string, void (*routine)(int32_t), boolean input)
{
	_g->messageLastMenuActive = _g->menuactive;
	_g->messageToPrint        = true;
	_g->messageString         = string;
	messageRoutine            = routine;
	_g->messageNeedsInput     = input;
	_g->menuactive            = true;
}


/////////////////////////////
//
// Thermometer Routines
//

//
// M_DrawThermo draws the thermometer graphic for Mouse Sensitivity,
// Sound Volume, etc.
//
// proff/nicolas 09/20/98 -- changed for hi-res
// CPhipps - patch drawing updated
//
static void M_DrawThermo(int16_t x, int16_t y, int16_t thermWidth, int16_t thermDot )
{
    int16_t          xx;
    int16_t           i;
    /*
   * Modification By Barry Mead to allow the Thermometer to have vastly
   * larger ranges. (the thermWidth parameter can now have a value as
   * large as 200.      Modified 1-9-2000  Originally I used it to make
   * the sensitivity range for the mouse better. It could however also
   * be used to improve the dynamic range of music and sound affect
   * volume controls for example.
   */
    int16_t horizScaler; //Used to allow more thermo range for mouse sensitivity.
    thermWidth = (thermWidth > 200) ? 200 : thermWidth; //Clamp to 200 max
    horizScaler = (thermWidth > 23) ? (200 / thermWidth) : 8; //Dynamic range
    xx = x;

    int16_t thermm_lump = W_GetNumForName("M_THERMM");

    V_DrawNamePatch(xx, y, "M_THERML");

    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
        V_DrawNumPatch(xx, y, thermm_lump);
        xx += horizScaler;
    }

    xx += (8 - horizScaler);  /* make the right end look even */

    V_DrawNamePatch(xx, y, "M_THERMR");
    V_DrawNamePatch((x+8)+thermDot*horizScaler,y, "M_THERMO");
}

/////////////////////////////
//
// String-drawing Routines
//

static int16_t font_lump_offset;

//
// Find string width from hu_font chars
//

static int16_t M_StringWidth(const char* string)
{
	int16_t	w = 0;

	for (int16_t i = 0; i < strlen(string); i++)
	{
		char c = toupper(string[i]);
		if (HU_FONTSTART <= c && c <= HU_FONTEND)
		{
			const patch_t* patch = W_GetLumpByNum(c + font_lump_offset);
			w += patch->width;
			Z_ChangeTagToCache(patch);
		} else
			w += HU_FONT_SPACE_WIDTH;
	}

	return w;
}

//
//    Find string height from hu_font chars
//

static int16_t M_StringHeight(const char* string)
{
	int16_t i, h = HU_FONT_HEIGHT;
	for (i = 0; string[i]; i++)            // killough 1/31/98
		if (string[i] == '\n')
			h += HU_FONT_HEIGHT;
	return h;
}

//
//    Write a string using the hu_font
//
static void M_WriteText (int16_t x, int16_t y, const char* string)
{
	const char* ch = string;
	int16_t cx = x;
	int16_t cy = y;

	while (true) {
		char c = *ch++;
		if (!c)
			break;

		if (c == '\n') {
			cx = x;
			cy += 12;
			continue;
		}

		c = toupper(c);
		if (HU_FONTSTART <= c && c <= HU_FONTEND)
		{
			const patch_t* patch = W_GetLumpByNum(c + font_lump_offset);
			V_DrawPatchNoScale(cx, cy, patch);
			cx += patch->width;
			Z_ChangeTagToCache(patch);
		} else {
			cx += HU_FONT_SPACE_WIDTH;
		}
	}
}

/////////////////////////////
//
// Initialization Routines to take care of one-time setup
//

//
// M_Init
//
void M_Init(void)
{
	M_InitDefaults();                // killough 11/98
	_g->currentMenu           = &MainDef;
	_g->menuactive            = false;
	_g->whichSkull            = 0;
	_g->skullAnimCounter      = 10;
	_g->messageToPrint        = false;
	_g->messageString         = NULL;
	_g->messageLastMenuActive = _g->menuactive;

	font_lump_offset = W_GetNumForName(HU_FONTSTART_LUMP) - HU_FONTSTART;

	G_UpdateSaveGameStrings();
}

//
// End of General Routines
//
/////////////////////////////////////////////////////////////////////////////
