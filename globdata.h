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


typedef struct globals_t
{
#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H

//******************************************************************************
//am_map.c
//******************************************************************************

enum automapmode_e automapmode; // Mode that the automap is in


//******************************************************************************
//d_client.c
//******************************************************************************

int32_t maketic;


//******************************************************************************
//d_main.c
//******************************************************************************

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t    wipegamestate;
gamestate_t oldgamestate;

int32_t  demosequence;         // killough 5/2/98: made static
int32_t  pagetic;

boolean singletics; // debug flag to cancel adaptiveness
boolean advancedemo;
boolean fps_show;

uint32_t gamma;

//fps counter stuff

uint32_t fps_timebefore;
uint32_t fps_frames;
uint16_t fps_framerate;


//******************************************************************************
//f_finale.c
//******************************************************************************


// Stage of animation:
//  false = text, true = art screen
boolean finalestage; // cph -
int32_t finalecount; // made static

boolean midstage;                 // whether we're in "mid-stage"


//******************************************************************************
//g_game.c
//******************************************************************************

const byte *demobuffer;   /* cph - only used for playback */
int32_t demolength; // check for overrun (missing DEMOMARKER)

const byte *demo_p;

gameaction_t    gameaction;
gamestate_t     gamestate;
skill_t         gameskill;

int32_t             gamemap;

player_t        player;

int32_t             starttime;     // for comparative timing purposes

int32_t             gametic;
int32_t             basetic;       /* killough 9/29/98: for demo sync */
int32_t             totalkills, totallive, totalitems, totalsecret;    // for intermission
wbstartstruct_t wminfo;               // parms for world map / intermission
int32_t             totalleveltimes;      // CPhipps - total time for all completed levels


// CPhipps - made lots of key/button state vars static
boolean gamekeydown[NUMKEYS];
int32_t     turnheld;       // for accelerative turning

gamestate_t prevgamestate;

skill_t d_skill;

byte  savegameslot;         // Slot to load if gameaction == ga_loadgame


boolean secretexit;

boolean         respawnmonsters;

boolean         usergame;      // ok to save / end game
boolean         timingdemo;    // if true, exit with report on completion
boolean         playeringame;
boolean         demoplayback;
boolean         singledemo;           // quit after playing a demo from cmdline


//******************************************************************************
//hu_stuff.c
//******************************************************************************

// widgets
hu_textline_t  w_title;
hu_stext_t     w_message;
int16_t        message_counter;

boolean    message_on;
boolean    message_dontfuckwithme;
boolean    headsupactive;

//******************************************************************************
//i_audio.c
//******************************************************************************

int32_t lasttimereply;
int32_t basetime;


//******************************************************************************
//m_cheat.c
//******************************************************************************

uint32_t cheat_buffer;

//******************************************************************************
//m_menu.c
//******************************************************************************

//
// defaulted values
//
int32_t alwaysRun;

int32_t highDetail;

boolean messageToPrint;  // true = message to be printed

// CPhipps - static const
const char* messageString; // ...and here is the message string!

boolean messageLastMenuActive;

int32_t saveSlot;        // which slot to save in
// old save description before edit

const menu_t* currentMenu; // current menudef

int16_t itemOn;           // menu item skull is on (for Big Font menus)
int16_t skullAnimCounter; // skull animation counter
int16_t whichSkull;       // which skull to draw (he blinks)

boolean menuactive;    // The menus are up
boolean messageNeedsInput; // timed message = no input from user

char savegamestrings[8][8];


//******************************************************************************
//m_random.c
//******************************************************************************

int32_t	rndindex;
int32_t	prndindex;


//******************************************************************************
//p_ceiling.c
//******************************************************************************

// the list of ceilings moving currently, including crushers
ceilinglist_t *activeceilings;

//******************************************************************************
//p_enemy.c
//******************************************************************************

fixed_t dropoff_deltax, dropoff_deltay, floorz;


//******************************************************************************
//p_map.c
//******************************************************************************

mobj_t    *tmthing;
fixed_t   tmx;
fixed_t   tmy;


// The tm* items are used to hold information globally, usually for
// line or object intersection checking

fixed_t   tmbbox[4];  // bounding box for line intersection checks
fixed_t   tmfloorz;   // floor you'd hit if free to fall
fixed_t   tmceilingz; // ceiling of sector you're in
fixed_t   tmdropoffz; // dropoff on other side of line you're crossing

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls

const line_t    *ceilingline;
const line_t        *blockline;    /* killough 8/11/98: blocking linedef */
const line_t        *floorline;    /* killough 8/1/98: Highest touched floor */
int32_t         tmunstuck;     /* killough 8/1/98: whether to allow unsticking */

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

// 1/11/98 killough: removed limit on special lines crossed
const line_t *spechit[4];                // new code -- killough

int32_t numspechit;

// Temporary holder for thing_sectorlist threads
msecnode_t* sector_list;                             // phares 3/16/98

/* killough 8/2/98: make variables static */
fixed_t   bestslidefrac;
const line_t*   bestslideline;
mobj_t*   slidemo;
fixed_t   tmxmove;
fixed_t   tmymove;

mobj_t*   linetarget; // who got hit (or NULL)
mobj_t*   shootthing;

// Height if not aiming up or down
fixed_t   shootz;

int32_t       la_damage;
fixed_t   attackrange;

// slopes to top and bottom of target
// killough 4/20/98: make static instead of using ones in p_sight.c

fixed_t  topslope;
fixed_t  bottomslope;

mobj_t *bombsource, *bombspot;
int32_t bombdamage;

mobj_t*   usething;

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean   floatok;

/* killough 11/98: if "felldown" true, object was pushed down ledge */
boolean   felldown;

boolean crushchange, nofit;

boolean telefrag;   /* killough 8/9/98: whether to telefrag at exit */



//******************************************************************************
//p_maputl.c
//******************************************************************************

fixed_t opentop;
fixed_t openbottom;
fixed_t openrange;
fixed_t lowfloor;

// moved front and back outside P-LineOpening and changed    // phares 3/7/98
// them to these so we can pick up the new friction value
// in PIT_CheckLine()
sector_t *openfrontsector; // made global                    // phares
sector_t *openbacksector;  // made global

divline_t trace;


// 1/11/98 killough: Intercept limit removed
intercept_t intercepts[MAXINTERCEPTS];
intercept_t* intercept_p;


//******************************************************************************
//p_plats.c
//******************************************************************************

platlist_t *activeplats;       // killough 2/14/98: made global again


//******************************************************************************
//p_pspr.c
//******************************************************************************

fixed_t bulletslope;

//******************************************************************************
//p_setup.c
//******************************************************************************


//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//

int32_t      numvertexes;
const vertex_t *vertexes;

const seg_t    *segs;

int32_t      numsectors;
sector_t *sectors;


int32_t      numsubsectors;
subsector_t *subsectors;



int32_t      numlines;
const line_t   *lines;
linedata_t* linedata;


int32_t      numsides;
side_t   *sides;

// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.

int16_t       bmapwidth, bmapheight;  // size in mapblocks

// killough 3/1/98: remove blockmap limit internally:
const int16_t      *blockmap;              // was short -- killough

// offsets in blockmap are from here
const int16_t      *blockmaplump;          // was short -- killough

fixed_t   bmaporgx, bmaporgy;     // origin of block map

mobj_t    **blocklinks;           // for thing chains

//
// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without the special effect, this could
// be used as a PVS lookup as well.
//

const byte *rejectmatrix; // cph - const*

// Maintain single and multi player starting spots.
mapthing_t playerstarts[MAXPLAYERS];

mobj_t*      thingPool;
uint32_t thingPoolSize;


//******************************************************************************
//p_sight.c
//******************************************************************************

los_t los; // cph - made static


//******************************************************************************
//p_switch.c
//******************************************************************************

int16_t switchlist[MAXSWITCHES * 2];
int32_t   numswitches;                           // killough

button_t  buttonlist[MAXBUTTONS];


//******************************************************************************
//p_tick.c
//******************************************************************************

int32_t leveltime; // tics in game play for par

// killough 8/29/98: we maintain several separate threads, each containing
// a special class of thinkers, to allow more efficient searches.
thinker_t thinkerclasscap;

//******************************************************************************
//p_user.c
//******************************************************************************

boolean onground; // whether player is on ground or in air


//******************************************************************************
//r_hotpatch_iwram.c
//******************************************************************************

visplane_t *visplanes[MAXVISPLANES];   // killough
visplane_t *freetail;                  // killough


//******************************************************************************
//r_segs.c
//******************************************************************************

//
// regular wall
//
drawseg_t drawsegs[MAXDRAWSEGS];

int16_t openings[MAXOPENINGS];
int16_t* lastopening;



//******************************************************************************
//r_sky.c
//******************************************************************************

//
// sky mapping
//
int16_t skyflatnum;


//******************************************************************************
//r_things.c
//******************************************************************************

// variables used to look up and range check thing_t sprites patches

spritedef_t *sprites;


//******************************************************************************
//s_sound.c
//******************************************************************************

// the set of channels available
channel_t *channels;

// music currently being played
int32_t mus_playing;

// whether songs are mus_paused
boolean mus_paused;

//******************************************************************************
//st_stuff.c
//******************************************************************************

uint32_t st_needrefresh;

// 0-9, tall numbers
int16_t tallnum[10];

// 0-9, short, yellow (,different!) numbers
int16_t shortnum[10];

int16_t keys[NUMCARDS];

// face status patches
int16_t faces[ST_NUMFACES];


// weapon ownership patches
int16_t arms[6][2];

// ready-weapon widget
st_number_t w_ready;

// health widget
st_number_t st_health;

// weapon ownership widgets
st_multicon_t w_arms[6];

// face status widget
st_multicon_t w_faces;

// keycard widgets
st_multicon_t w_keyboxes[3];

// ammo widgets
st_number_t w_ammo[4];

// max ammo widgets
st_number_t w_maxammo[4];

// armor widget
st_number_t  st_armor;

// used for evil grin
boolean  oldweaponsowned[NUMWEAPONS];

 // count until face changes
int32_t      st_facecount;

// current face index, used by w_faces
int32_t      st_faceindex;

// holds key-type for each key box on bar
int32_t      keyboxes[3];

// a random number per tick
int32_t      st_randomnumber;

int8_t st_palette;


//******************************************************************************
//v_video.c
//******************************************************************************

// The screen is [SCREENWIDTH*SCREENHEIGHT];
uint16_t *screen;

//******************************************************************************
//wi_stuff.c
//******************************************************************************

// used to accelerate or skip a stage
boolean   acceleratestage;           // killough 3/28/98: made global

 // specifies current state
stateenum_t  state;

// contains information passed into intermission
wbstartstruct_t* wbs;

wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
int32_t    cnt;

// used for timing of background animation
int32_t    bcnt;

int32_t    cnt_time;
int32_t    cnt_total_time;
int32_t    cnt_par;
int32_t    cnt_pause;


int32_t  sp_state;

int32_t cnt_kills;
int32_t cnt_items;
int32_t cnt_secret;

boolean snl_pointeron;


//******************************************************************************
#endif // GLOBAL_DEFS_H

} globals_t;

void InitGlobals();

extern globals_t* _g;


extern int32_t showMessages;    // Show messages has default, 0 = off, 1 = on


extern uint16_t validcount;         // increment every time a check is made
extern visplane_t **freehead;


// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
extern int32_t snd_SfxVolume;

// Maximum volume of music. Useless so far.
extern int32_t snd_MusicVolume;

#endif // GLOBAL_DATA_H
