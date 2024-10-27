/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2024 Frenkel Smeijers
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
 *  Text mode version
 *
 *-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>

#include "compiler.h"
#include "doomdef.h"
#include "d_player.h"
#include "d_englsh.h"
#include "d_main.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_main.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "m_menu.h"
#include "doomtype.h"
#include "am_map.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "i_vtext.h"

#include "globdata.h"


#define DISABLE_SAVE_GAME
#define DISABLE_SOUND_OPTIONS


//
// MENU TYPEDEFS
//

typedef struct
{
  int16_t status; // 0 = no cursor here, 1 = ok, 2 = arrows ok
  char* name;

  // choice = menu item #.
  // if status = 2,
  //   choice=0:leftarrow,1:rightarrow
  void  (*routine)(int16_t choice);
} menuitem_t;

typedef struct menu_s
{
  int16_t           numitems;     // # of menu items
  const menuitem_t* menuitems;    // menu items
  void            (*routine)(); // draw routine
  int16_t           x;
  int16_t           y;            // x,y of menu
  const struct menu_s*	prevMenu;	// previous menu
  int16_t previtemOn;
} menu_t;


//
// defaulted values
//
int16_t _g_alwaysRun;

uint16_t _g_gamma;

static boolean messageToPrint;  // true = message to be printed

static const char* messageString; // ...and here is the message string!

static boolean messageLastMenuActive;


static const menu_t* currentMenu; // current menudef

static int16_t itemOn;           // menu item skull is on (for Big Font menus)
static int16_t skullAnimCounter; // skull animation counter
static int16_t whichSkull;       // which skull to draw (he blinks)

boolean _g_menuactive;    // The menus are up
static boolean messageNeedsInput; // timed message = no input from user

char _g_savegamestrings[8][8];


int16_t showMessages = 1;


static void (*messageRoutine)(boolean affirmative);

// we are going to be entering a savegame string

#define SKULLXOFF  -2
#define LINEHEIGHT  1

// graphic name of skulls

static const char skullName[2] = {'\x02', '\x01'};


// end of externs added for setup menus

//
// PROTOTYPES
//
static void M_NewGame(int16_t choice);
static void M_ChooseSkill(int16_t choice);
static void M_LoadGame(int16_t choice);
static void M_SaveGame(int16_t choice);
static void M_Options(int16_t choice);
static void M_EndGame(int16_t choice);
static void M_QuitDOOM(int16_t choice);


static void M_ChangeMessages(int16_t choice);
static void M_ChangeAlwaysRun(int16_t choice);
static void M_ChangeGamma(int16_t choice);
static void M_SfxVol(int16_t choice);
static void M_MusicVol(int16_t choice);
static void M_Sound(int16_t choice);

static void M_LoadSelect(int16_t choice);
static void M_SaveSelect(int16_t choice);
static void M_ReadSaveStrings(void);

static void M_DrawMainMenu(void);
static void M_DrawNewGame(void);
static void M_DrawOptions(void);
static void M_DrawSound(void);
static void M_DrawLoad(void);
static void M_DrawSave(void);

static void M_SetupNextMenu(const menu_t *menudef);
static void M_DrawThermo(int16_t x, int16_t y, int16_t thermWidth, int16_t thermDot);
static void M_WriteText(int16_t x, int16_t y, const char __far* string);
static int16_t M_StringHeight(const char *string);
static void M_StartMessage(const char *string,void (*routine)(boolean));
static void M_ClearMenus (void);


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
  options,
  loadgame,
#if !defined DISABLE_SAVE_GAME
  savegame,
#endif
  quitdoom,
  main_end
};

//
// MainMenu is the definition of what the main menu Screen should look
// like. Each entry shows that the cursor can land on each item (1), the
// built-in graphic lump (i.e. "M_NGAME") that should be displayed,
// and the program which takes over when an item is selected.
//

static const menuitem_t MainMenu[]=
{
  {1,"New Game",  M_NewGame},
  {1,"Options",   M_Options},
  {1,"Load game", M_LoadGame},
#if !defined DISABLE_SAVE_GAME
  {1,"Save game", M_SaveGame},
#endif
  {1,"Quit Game", M_QuitDOOM}
};

static const menu_t MainDef =
{
  main_end,       // number of menu items
  MainMenu,       // table that defines menu items
  M_DrawMainMenu, // drawing routine
  (VIEWWINDOWWIDTH - 9) / 2,8,          // initial cursor position
  NULL,0,
};

//
// M_DrawMainMenu
//

static void M_DrawMainMenu(void)
{
	V_DrawString((VIEWWINDOWWIDTH - 4) / 2, 1, 14, "DOOM");
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
  {1,"I'm too young to die.", M_ChooseSkill},
  {1,"Hey, not too rough.",   M_ChooseSkill},
  {1,"Hurt me plenty.",       M_ChooseSkill},
  {1,"Ultra-Violence.",       M_ChooseSkill},
  {1,"NIGHTMARE!",            M_ChooseSkill}
};

static const menu_t NewDef =
{
  newg_end,       // # of menu items
  NewGameMenu,    // menuitem_t ->
  M_DrawNewGame,  // drawing routine ->
  (VIEWWINDOWWIDTH - 21) / 2,8,          // x,y
  &MainDef,0,
};


//
// M_NewGame
//

static void M_DrawNewGame(void)
{
	V_DrawString((VIEWWINDOWWIDTH -  8) / 2, 2, 12, "NEW GAME");
	V_DrawString((VIEWWINDOWWIDTH - 19) / 2, 4, 12, "Choose Skill Level:");
}

static void M_NewGame(int16_t choice)
{
	UNUSED(choice);

	M_SetupNextMenu(&NewDef);
	itemOn = 2; //Set hurt me plenty as default difficulty
}


static void M_VerifyNightmare(boolean affirmative)
{
    if (affirmative)
        G_DeferedInitNew(nightmare);
}

static void M_ChooseSkill(int16_t choice)
{
    if (choice == nightmare)
    {
        M_StartMessage(NIGHTMARE, M_VerifyNightmare);
		itemOn = 0;
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
  (VIEWWINDOWWIDTH - 5) / 2,4,
  &MainDef,2,
};

#define LOADGRAPHIC_Y 8

//
// M_LoadGame & Cie.
//

static void M_DrawSaveLoad(const char* name)
{
	V_DrawString((VIEWWINDOWWIDTH - strlen(name)) / 2, 2, 12, name);

	for (int16_t i = 0; i < load_end; i++)
		M_WriteText(LoadDef.x, i + 4, _g_savegamestrings[i]);
}

static void M_DrawLoad(void)
{
	M_DrawSaveLoad("Load game");
}


//
// User wants to load this game
//

static void M_LoadSelect(int16_t choice)
{
  // CPhipps - Modified so savegame filename is worked out only internal
  //  to g_game.c, this only passes the slot.

  G_LoadGame(choice); // killough 3/16/98: add slot

  M_ClearMenus ();
}

//
// Selected from DOOM menu
//

static void M_LoadGame (int16_t choice)
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

static const menuitem_t SaveMenu[]=
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

static const menu_t SaveDef =
{
  load_end, // same number of slots as the Load Game screen
  SaveMenu,
  M_DrawSave,
  (VIEWWINDOWWIDTH - 5) / 2,5,
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
	M_DrawSaveLoad("Save game");
}

//
// M_Responder calls this when user is finished
//
static void M_DoSave(int16_t slot)
{
  G_SaveGame (slot);
  M_ClearMenus ();
}

//
// User wants to save. Start string input for M_Responder
//
static void M_SaveSelect(int16_t choice)
{
    M_DoSave(choice);
}

//
// Selected from DOOM menu
//
static void M_SaveGame (int16_t choice)
{
	UNUSED(choice);

	// killough 10/6/98: allow savegames during single-player demo playback
	if (!_g_usergame && (!_g_demoplayback))
	{
		M_StartMessage(SAVEDEAD, NULL);
		return;
	}

	if (_g_gamestate != GS_LEVEL)
		return;

	M_SetupNextMenu(&SaveDef);
	M_ReadSaveStrings();
}


/////////////////////////////
//
// QUIT DOOM
//

static void M_QuitResponse(boolean affirmative)
{
	if (affirmative)
		I_Quit();
}


static void M_QuitDOOM(int16_t choice)
{
	UNUSED(choice);
	M_StartMessage(QUITMSG, M_QuitResponse);
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
  gamma,
#if !defined DISABLE_SOUND_OPTIONS
  soundvol,
#endif
  opt_end
};

// The definitions of the Options menu

static const menuitem_t OptionsMenu[]=
{
  // killough 4/6/98: move setup to be a sub-menu of OPTIONs
  {1,"End Game",     M_EndGame},
  {1,"Messages:",    M_ChangeMessages},
  {1,"Always Run:",  M_ChangeAlwaysRun},
  {2,"Gamma Boost:", M_ChangeGamma},
#if !defined DISABLE_SOUND_OPTIONS
  {1,"Sound Volume", M_Sound}
#endif
};

static const menu_t OptionsDef =
{
  opt_end,
  OptionsMenu,
  M_DrawOptions,
  (VIEWWINDOWWIDTH - 12) / 2,4,
  &MainDef,1,
};

//
// M_Options
//
static const char msgNames[2][4]  = {"Off","On"};


static void M_DrawOptions(void)
{
	V_DrawString((VIEWWINDOWWIDTH - 7) / 2, 2, 12, "OPTIONS");

	V_DrawString(OptionsDef.x + 13, OptionsDef.y + LINEHEIGHT * messages,  12, msgNames[showMessages]);

	V_DrawString(OptionsDef.x + 13, OptionsDef.y + LINEHEIGHT * alwaysrun, 12, msgNames[_g_alwaysRun]);

	M_DrawThermo(OptionsDef.x + 13, OptionsDef.y + LINEHEIGHT * gamma, 6, _g_gamma);
}

static void M_Options(int16_t choice)
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
  {2,"Sfx Volume",   M_SfxVol},
  {-1,"",0},
  {2,"Music Volume", M_MusicVol},
  {-1,"",0}
};

static const menu_t SoundDef =
{
  sound_end,
  SoundMenu,
  M_DrawSound,
  (VIEWWINDOWWIDTH - 12) / 2,8,
  &OptionsDef,4,
};

//
// Change Sfx & Music volumes
//

static void M_DrawSound(void)
{
	V_DrawNamePatchScaled(60, 38, "M_SVOL");

	M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (sfx_vol   + 1), 16, snd_SfxVolume);

	M_DrawThermo(SoundDef.x, SoundDef.y + LINEHEIGHT * (music_vol + 1), 16, snd_MusicVolume);
}

static void M_Sound(int16_t choice)
{
	UNUSED(choice);

	M_SetupNextMenu(&SoundDef);
}

static void M_SfxVol(int16_t choice)
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

static void M_MusicVol(int16_t choice)
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

static void M_EndGameResponse(boolean affirmative)
{
  if (!affirmative)
    return;

  // killough 5/26/98: make endgame quit if recording or playing back demo
  if (_g_singledemo)
    G_CheckDemoStatus();

  M_ClearMenus ();
  D_StartTitle ();
}

static void M_EndGame(int16_t choice)
{
	UNUSED(choice);

	M_StartMessage(ENDGAME, M_EndGameResponse);
}

/////////////////////////////
//
//    Toggle messages on/off
//

static void M_ChangeMessages(int16_t choice)
{
  UNUSED(choice);

  showMessages = 1 - showMessages;

  _g_player.message = showMessages ? MSGON : MSGOFF;

  _g_message_dontfuckwithme = true;

  G_SaveSettings();
}


static void M_ChangeAlwaysRun(int16_t choice)
{
    UNUSED(choice);

    _g_alwaysRun = 1 - _g_alwaysRun;

    if (!_g_alwaysRun)
      _g_player.message = RUNOFF; // Ty 03/27/98 - externalized
    else
      _g_player.message = RUNON ; // Ty 03/27/98 - externalized

    G_SaveSettings();
}

static void M_ChangeGamma(int16_t choice)
{
	switch(choice)
    {
		case 0:
		  if (_g_gamma)
			_g_gamma--;
		  break;
		case 1:
		  if (_g_gamma < 5)
			_g_gamma++;
		  break;
    }
	I_ReloadPalette();
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
    int16_t    ch;

    // Mouse input processing removed

    // Process keyboard input

    if (ev->type == ev_keydown)
        ch = ev->data1;
    else
        return false; // we can't use the event here

    // Take care of any messages that need input

    if (messageToPrint)
    {
        if (messageNeedsInput == true &&
                !(ch == 'n' || ch == 'y' || ch == key_escape))
            return false;

        _g_menuactive  = messageLastMenuActive;
        messageToPrint = false;
        if (messageRoutine)
            messageRoutine(ch == 'y');

        I_InitScreenPages();
        _g_menuactive = false;
        S_StartSound(NULL,sfx_swtchx);
        return true;
    }

    // Pop-up Main menu?

    if (!_g_menuactive)
    {
        if (ch == key_escape)
        {
            M_StartControlPanel ();
            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        return false;
    }

    // From here on, these navigation keys are used on the BIG FONT menus
    // like the Main Menu.

    if (ch == key_menu_down)
    {
        do
        {
            if (itemOn+1 > currentMenu->numitems-1)
                itemOn = 0;
            else
                itemOn++;
            S_StartSound(NULL,sfx_pstop);
        }
        while(currentMenu->menuitems[itemOn].status==-1);
        return true;
    }

    if (ch == key_menu_up)
    {
        do
        {
            if (!itemOn)
                itemOn = currentMenu->numitems-1;
            else
                itemOn--;
            S_StartSound(NULL,sfx_pstop);
        }
        while(currentMenu->menuitems[itemOn].status==-1);
        return true;
    }

    if (ch == key_menu_left)
    {
        if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL,sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(0);
        }
        return true;
    }

    if (ch == key_menu_right)
    {
        if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status == 2)
        {
            S_StartSound(NULL,sfx_stnmov);
            currentMenu->menuitems[itemOn].routine(1);
        }
        return true;
    }

    if (ch == key_menu_enter)
    {
        if (currentMenu->menuitems[itemOn].routine &&
                currentMenu->menuitems[itemOn].status)
        {
            if (currentMenu->menuitems[itemOn].status == 2)
            {
                currentMenu->menuitems[itemOn].routine(1);   // right arrow
                S_StartSound(NULL,sfx_stnmov);
            }
            else
            {
                currentMenu->menuitems[itemOn].routine(itemOn);
                S_StartSound(NULL,sfx_pistol);
            }
        }

        return true;
    }

    if (ch == key_menu_escape)
    {
        M_ClearMenus ();
        S_StartSound(NULL,sfx_swtchx);
        return true;
    }

	//Allow being able to go back in menus
	if (ch == key_fire)
    {
		//If the prevMenu == NULL (Such as main menu screen), then just get out of the menu altogether
		if(currentMenu->prevMenu == NULL)
		{
			M_ClearMenus();
		}else //Otherwise, change to the parent menu and match the row used to get there.
		{
			int16_t previtemOn = currentMenu->previtemOn; //Temporarily store this so after menu change, we store the last row it was on.
			M_SetupNextMenu(currentMenu->prevMenu);					
			itemOn = previtemOn;
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

  if (_g_menuactive)
    return;

  //jff 3/24/98 make default skill menu choice follow -skill or defaultskill
  //from command line or config file
  //
  // killough 10/98:
  // Fix to make "always floating" with menu selections, and to always follow
  // defaultskill, instead of -skill.

  _g_menuactive = true;
  currentMenu = &MainDef;         // JDC
}


static char __far* Z_Strdup(const char* s)
{
    const size_t len = strlen(s);

    if(!len)
        return NULL;

    char __far* ptr = Z_MallocStatic(len+1);

    if(ptr)
        _fstrcpy(ptr, s);

    return ptr;
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//

void M_Drawer (void)
{
	if (messageToPrint)
	{
		// Horiz. & Vertically center string and print it.

		I_InitScreenPage();

		/* cph - strdup string to writable memory */
		char __far* ms = Z_Strdup(messageString);
		char __far* p = ms;

		int16_t y = (VIEWWINDOWHEIGHT - M_StringHeight(messageString)) / 2;
		while (*p)
		{
			char __far* string = p;
			char c;
			while ((c = *p) && *p != '\n')
				p++;
			*p = 0;
			M_WriteText((VIEWWINDOWWIDTH - _fstrlen(string)) / 2, y, string);
			y += 1;
			if ((*p = c))
				p++;
		}
		Z_Free(ms);
	}
	else if (_g_menuactive)
	{
		int16_t x,y,max,i;

		I_InitScreenPage();

		if (currentMenu->routine)
			currentMenu->routine();     // call Draw routine

		// DRAW MENU

		x = currentMenu->x;
		y = currentMenu->y;
		max = currentMenu->numitems;

		for (i=0;i<max;i++)
		{
			if (currentMenu->menuitems[i].name[0])
				V_DrawString(x, y, 12, currentMenu->menuitems[i].name);
			y += LINEHEIGHT;
		}

		// DRAW SKULL
		V_DrawCharacter(x + SKULLXOFF, currentMenu->y + itemOn*LINEHEIGHT, 12, skullName[whichSkull]);
	}
}

//
// M_ClearMenus
//
// Called when leaving the menu screens for the real world

static void M_ClearMenus (void)
{
  _g_menuactive = false;
  itemOn = 0;
  I_InitScreenPages();
}

//
// M_SetupNextMenu
//
static void M_SetupNextMenu(const menu_t *menudef)
{
  currentMenu = menudef;
  itemOn = 0;
}

/////////////////////////////
//
// M_Ticker
//
void M_Ticker (void)
{
  if (--skullAnimCounter <= 0)
    {
      whichSkull ^= 1;
      skullAnimCounter = 8;
    }
}

/////////////////////////////
//
// Message Routines
//

static void M_StartMessage (const char* string, void (*routine)(boolean))
{
	messageLastMenuActive = _g_menuactive;
	messageToPrint        = true;
	messageString         = string;
	messageRoutine        = routine;
	messageNeedsInput     = routine != NULL;
	_g_menuactive         = true;
}


/////////////////////////////
//
// Thermometer Routines
//

//
// M_DrawThermo draws the thermometer graphic for Mouse Sensitivity,
// Sound Volume, etc.
//
//
static void M_DrawThermo(int16_t x, int16_t y, int16_t thermWidth, int16_t thermDot )
{
	for (int16_t w = 0; w < thermWidth; w++)
		V_DrawCharacter(x + w, y, 8, '-');

	V_DrawCharacter(x + thermDot, y, 9, '|');
}

/////////////////////////////
//
// String-drawing Routines
//

//
//    Find string height from hu_font chars
//

static int16_t M_StringHeight(const char* string)
{
	int16_t i, h = 1;
	for (i = 0; string[i]; i++)
		if (string[i] == '\n')
			h += 1;
	return h;
}

//
//    Write a string using the hu_font
//
static void M_WriteText (int16_t x, int16_t y, const char __far* string)
{
	const char __far* ch = string;
	int16_t cx = x;
	int16_t cy = y;

	while (true) {
		char c = *ch++;
		if (!c)
			break;

		if (c == '\n') {
			cx = x;
			cy += 1;
			continue;
		}

		V_DrawCharacter(cx, cy, 12, c);
		cx++;
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
	M_InitDefaults();
	currentMenu           = &MainDef;
	_g_menuactive         = false;
	whichSkull            = 0;
	skullAnimCounter      = 10;
	messageToPrint        = false;
	messageString         = NULL;
	messageLastMenuActive = _g_menuactive;

	G_UpdateSaveGameStrings();
}

//
// End of General Routines
//
/////////////////////////////////////////////////////////////////////////////
