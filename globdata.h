#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include "d_player.h"
#include "doomdef.h"
#include "m_fixed.h"
#include "am_map.h"
#include "g_game.h"
#include "r_defs.h"
#include "hu_stuff.h"
#include "i_sound.h"
#include "m_menu.h"
#include "p_spec.h"
#include "p_enemy.h"
#include "p_map.h"
#include "p_maputl.h"

#include "p_mobj.h"
#include "p_tick.h"

#include "r_main.h"
#include "r_things.h"

#include "s_sound.h"

#include "st_stuff.h"

#include "v_video.h"

#include "w_wad.h"

#include "wi_stuff.h"


extern boolean _g_fps_show;

extern uint16_t _g_gamma;

//fps counter stuff
extern int16_t _g_fps_framerate;


//******************************************************************************
//g_game.c
//******************************************************************************

extern gameaction_t    _g_gameaction;
extern gamestate_t     _g_gamestate;
extern skill_t         _g_gameskill;

extern int16_t             _g_gamemap;

extern player_t        _g_player;

extern int32_t             _g_gametic;
extern int32_t             _g_basetic;       /* killough 9/29/98: for demo sync */
extern int32_t             _g_totalkills, _g_totallive, _g_totalitems, _g_totalsecret;    // for intermission
extern wbstartstruct_t _g_wminfo;               // parms for world map / intermission


extern boolean         _g_respawnmonsters;

extern boolean         _g_usergame;      // ok to save / end game
extern boolean         _g_timingdemo;    // if true, exit with report on completion
extern boolean         _g_demoplayback;
extern boolean         _g_singledemo;           // quit after playing a demo from cmdline


//******************************************************************************
//hu_stuff.c
//******************************************************************************

extern boolean    _g_message_dontfuckwithme;


//******************************************************************************
//m_menu.c
//******************************************************************************

//
// defaulted values
//
extern int16_t _g_alwaysRun;

extern boolean _g_menuactive;    // The menus are up

extern char _g_savegamestrings[8][8];


//******************************************************************************
//p_map.c
//******************************************************************************

// The tm* items are used to hold information globally, usually for
// line or object intersection checking

extern fixed_t   _g_tmbbox[4];  // bounding box for line intersection checks
extern fixed_t   _g_tmfloorz;   // floor you'd hit if free to fall
extern fixed_t   _g_tmceilingz; // ceiling of sector you're in
extern fixed_t   _g_tmdropoffz; // dropoff on other side of line you're crossing

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls

extern const line_t    __far* _g_ceilingline;

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

// 1/11/98 killough: removed limit on special lines crossed
extern line_t __far* _g_spechit[4];

extern int16_t _g_numspechit;


extern mobj_t __far*   _g_linetarget; // who got hit (or NULL)


//******************************************************************************
//p_maputl.c
//******************************************************************************

extern fixed_t _g_opentop;
extern fixed_t _g_openbottom;
extern fixed_t _g_openrange;
extern fixed_t _g_lowfloor;

extern divline_t _g_trace;


//******************************************************************************
//p_setup.c
//******************************************************************************


//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//

extern const seg_t    __far* _g_segs;

extern int16_t      _g_numsectors;
extern sector_t __far* _g_sectors;


extern subsector_t __far* _g_subsectors;



extern int16_t      _g_numlines;
extern line_t   __far* _g_lines;


extern side_t   __far* _g_sides;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.

extern int16_t       _g_bmapwidth, _g_bmapheight;  // size in mapblocks

// killough 3/1/98: remove blockmap limit internally:
extern const int16_t      __far* _g_blockmap;

// offsets in blockmap are from here
extern const int16_t      __far* _g_blockmaplump;

extern fixed_t   _g_bmaporgx, _g_bmaporgy;     // origin of block map

extern mobj_t    __far*__far* _g_blocklinks;           // for thing chains

//
// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without the special effect, this could
// be used as a PVS lookup as well.
//

extern const byte __far* _g_rejectmatrix;


extern mobj_t __far*      _g_thingPool;
extern int16_t _g_thingPoolSize;


//******************************************************************************
//p_switch.c
//******************************************************************************

extern button_t  _g_buttonlist[MAXBUTTONS];


//******************************************************************************
//p_tick.c
//******************************************************************************

extern int32_t _g_leveltime; // tics in game play for par

// killough 8/29/98: we maintain several separate threads, each containing
// a special class of thinkers, to allow more efficient searches.
extern thinker_t _g_thinkerclasscap;


//******************************************************************************
//wi_stuff.c
//******************************************************************************

// used to accelerate or skip a stage
extern boolean   _g_acceleratestage;


extern enum automapmode_e automapmode;


extern gamestate_t wipegamestate;


extern int16_t showMessages;    // Show messages has default, 0 = off, 1 = on


extern uint16_t validcount;         // increment every time a check is made


//
// sky mapping
//
extern int16_t skyflatnum;


// variables used to look up and range check thing_t sprites patches
extern spritedef_t __far* sprites;


// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
extern int16_t snd_SfxVolume;

// Maximum volume of music. Useless so far.
extern int16_t snd_MusicVolume;

#endif // GLOBAL_DATA_H
