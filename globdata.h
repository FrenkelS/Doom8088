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

extern uint32_t _g_gamma;

//fps counter stuff
extern uint16_t _g_fps_framerate;


//******************************************************************************
//g_game.c
//******************************************************************************

extern gameaction_t    _g_gameaction;
extern gamestate_t     _g_gamestate;
extern skill_t         _g_gameskill;

extern int32_t             _g_gamemap;

extern player_t        _g_player;

extern int32_t             _g_gametic;
extern int32_t             _g_basetic;       /* killough 9/29/98: for demo sync */
extern int32_t             _g_totalkills, _g_totallive, _g_totalitems, _g_totalsecret;    // for intermission
extern wbstartstruct_t _g_wminfo;               // parms for world map / intermission


extern boolean _g_gamekeydown[NUMKEYS];


extern boolean         _g_respawnmonsters;

extern boolean         _g_usergame;      // ok to save / end game
extern boolean         _g_timingdemo;    // if true, exit with report on completion
extern boolean         _g_playeringame;
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
extern int32_t _g_alwaysRun;

extern int32_t _g_highDetail;

extern boolean _g_menuactive;    // The menus are up

extern char _g_savegamestrings[8][8];


//******************************************************************************
//p_map.c
//******************************************************************************

extern mobj_t    *_g_tmthing;
extern fixed_t   _g_tmx;
extern fixed_t   _g_tmy;


// The tm* items are used to hold information globally, usually for
// line or object intersection checking

extern fixed_t   _g_tmbbox[4];  // bounding box for line intersection checks
extern fixed_t   _g_tmfloorz;   // floor you'd hit if free to fall
extern fixed_t   _g_tmceilingz; // ceiling of sector you're in
extern fixed_t   _g_tmdropoffz; // dropoff on other side of line you're crossing

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls

extern const line_t    *_g_ceilingline;
extern const line_t        *_g_blockline;    /* killough 8/11/98: blocking linedef */
extern const line_t        *_g_floorline;    /* killough 8/1/98: Highest touched floor */
extern int32_t         _g_tmunstuck;     /* killough 8/1/98: whether to allow unsticking */

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

// 1/11/98 killough: removed limit on special lines crossed
extern const line_t *_g_spechit[4];

extern int32_t _g_numspechit;

// Temporary holder for thing_sectorlist threads
extern msecnode_t* _g_sector_list;

extern fixed_t   _g_bestslidefrac;
extern const line_t*   _g_bestslideline;
extern mobj_t*   _g_slidemo;
extern fixed_t   _g_tmxmove;
extern fixed_t   _g_tmymove;

extern mobj_t*   _g_linetarget; // who got hit (or NULL)
extern mobj_t*   _g_shootthing;

// Height if not aiming up or down
extern fixed_t   _g_shootz;

extern int32_t       _g_la_damage;
extern fixed_t   _g_attackrange;

// slopes to top and bottom of target

extern fixed_t  _g_topslope;
extern fixed_t  _g_bottomslope;

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern boolean   _g_floatok;

/* killough 11/98: if "felldown" true, object was pushed down ledge */
extern boolean   _g_felldown;

extern boolean _g_crushchange, _g_nofit;

extern boolean _g_telefrag;   /* killough 8/9/98: whether to telefrag at exit */



//******************************************************************************
//p_maputl.c
//******************************************************************************

extern fixed_t _g_opentop;
extern fixed_t _g_openbottom;
extern fixed_t _g_openrange;
extern fixed_t _g_lowfloor;

// moved front and back outside P-LineOpening and changed    // phares 3/7/98
// them to these so we can pick up the new friction value
// in PIT_CheckLine()
extern sector_t *_g_openfrontsector;
extern sector_t *_g_openbacksector;

extern divline_t _g_trace;


// 1/11/98 killough: Intercept limit removed
extern intercept_t _g_intercepts[MAXINTERCEPTS];
extern intercept_t* _g_intercept_p;


//******************************************************************************
//p_setup.c
//******************************************************************************


//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//

extern int32_t      _g_numvertexes;
extern const vertex_t *_g_vertexes;

extern const seg_t    *_g_segs;

extern int32_t      _g_numsectors;
extern sector_t *_g_sectors;


extern int32_t      _g_numsubsectors;
extern subsector_t *_g_subsectors;



extern int32_t      _g_numlines;
extern const line_t   *_g_lines;
extern linedata_t* _g_linedata;


extern int32_t      _g_numsides;
extern side_t   *_g_sides;

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
extern const int16_t      *_g_blockmap;

// offsets in blockmap are from here
extern const int16_t      *_g_blockmaplump;

extern fixed_t   _g_bmaporgx, _g_bmaporgy;     // origin of block map

extern mobj_t    **_g_blocklinks;           // for thing chains

//
// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without the special effect, this could
// be used as a PVS lookup as well.
//

extern const byte *_g_rejectmatrix;

// Maintain single and multi player starting spots.
extern mapthing_t _g_playerstarts[MAXPLAYERS];

extern mobj_t*      _g_thingPool;
extern uint32_t _g_thingPoolSize;


//******************************************************************************
//p_switch.c
//******************************************************************************

extern int16_t _g_switchlist[MAXSWITCHES * 2];
extern int32_t   _g_numswitches;

extern button_t  _g_buttonlist[MAXBUTTONS];


//******************************************************************************
//p_tick.c
//******************************************************************************

extern int32_t _g_leveltime; // tics in game play for par

// killough 8/29/98: we maintain several separate threads, each containing
// a special class of thinkers, to allow more efficient searches.
extern thinker_t _g_thinkerclasscap;


//******************************************************************************
//r_hotpatch_iwram.c
//******************************************************************************

extern visplane_t *_g_visplanes[MAXVISPLANES];
extern visplane_t *_g_freetail;


//******************************************************************************
//r_segs.c
//******************************************************************************

//
// regular wall
//
extern drawseg_t _g_drawsegs[MAXDRAWSEGS];

extern int16_t _g_openings[MAXOPENINGS];
extern int16_t* _g_lastopening;


//******************************************************************************
//s_sound.c
//******************************************************************************

// the set of channels available
extern channel_t *_g_channels;

// music currently being played
extern int32_t _g_mus_playing;

// whether songs are mus_paused
extern boolean _g_mus_paused;

//******************************************************************************
//st_stuff.c
//******************************************************************************

extern uint32_t _g_st_needrefresh;

// 0-9, tall numbers
extern int16_t _g_tallnum[10];

// 0-9, short, yellow (,different!) numbers
extern int16_t _g_shortnum[10];

extern int16_t _g_keys[NUMCARDS];

// face status patches
extern int16_t _g_faces[ST_NUMFACES];


// weapon ownership patches
extern int16_t _g_arms[6][2];

// ready-weapon widget
extern st_number_t _g_w_ready;

// health widget
extern st_number_t _g_st_health;

// weapon ownership widgets
extern st_multicon_t _g_w_arms[6];

// face status widget
extern st_multicon_t _g_w_faces;

// keycard widgets
extern st_multicon_t _g_w_keyboxes[3];

// ammo widgets
extern st_number_t _g_w_ammo[4];

// max ammo widgets
extern st_number_t _g_w_maxammo[4];

// armor widget
extern st_number_t  _g_st_armor;

// used for evil grin
extern boolean  _g_oldweaponsowned[NUMWEAPONS];

 // count until face changes
extern int32_t      _g_st_facecount;

// current face index, used by w_faces
extern int32_t      _g_st_faceindex;

// holds key-type for each key box on bar
extern int32_t      _g_keyboxes[3];

// a random number per tick
extern int32_t      _g_st_randomnumber;

extern int8_t _g_st_palette;


//******************************************************************************
//v_video.c
//******************************************************************************

// The screen is [SCREENWIDTH*SCREENHEIGHT];
extern uint16_t *_g_screen;

//******************************************************************************
//wi_stuff.c
//******************************************************************************

// used to accelerate or skip a stage
extern boolean   _g_acceleratestage;

 // specifies current state
extern stateenum_t  _g_state;

// contains information passed into intermission
extern wbstartstruct_t* _g_wbs;

extern wbplayerstruct_t* _g_plrs;  // wbs->plyr[]

// used for general timing
extern int32_t    _g_cnt;

// used for timing of background animation
extern int32_t    _g_bcnt;

extern int32_t    _g_cnt_time;
extern int32_t    _g_cnt_total_time;
extern int32_t    _g_cnt_par;
extern int32_t    _g_cnt_pause;


extern int32_t  _g_sp_state;

extern int32_t _g_cnt_kills;
extern int32_t _g_cnt_items;
extern int32_t _g_cnt_secret;

extern boolean _g_snl_pointeron;


extern enum automapmode_e automapmode;


extern gamestate_t wipegamestate;


extern int32_t showMessages;    // Show messages has default, 0 = off, 1 = on


extern uint16_t validcount;         // increment every time a check is made
extern visplane_t **freehead;


//
// sky mapping
//
extern int16_t skyflatnum;


// variables used to look up and range check thing_t sprites patches
extern spritedef_t *sprites;


// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
extern int32_t snd_SfxVolume;

// Maximum volume of music. Useless so far.
extern int32_t snd_MusicVolume;

#endif // GLOBAL_DATA_H
