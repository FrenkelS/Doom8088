#include "m_cheat.h"
#include "doomdef.h"
#include "d_englsh.h"
#include "p_inter.h"
#include "g_game.h"
#include "globdata.h"


static void cheat_god(void);
static void cheat_choppers(void);
static void cheat_idkfa(void);
static void cheat_ammo(void);
static void cheat_noclip(void);
static void cheat_invincibility(void);
static void cheat_berserk(void);
static void cheat_invisibility(void);
static void cheat_map(void);
static void cheat_goggles(void);
static void cheat_exit(void);
static void cheat_rockets(void);
static void cheat_fps(void);


typedef struct
{
	uint8_t *sequence;
	uint8_t *p;
} cheatseq_t;

static cheatseq_t cheatseq_choppers      = {"idchoppers", NULL};
static cheatseq_t cheatseq_god           = {"iddqd",      NULL};
static cheatseq_t cheatseq_kfa           = {"idkfa",      NULL};
static cheatseq_t cheatseq_ammo          = {"idfa",       NULL};
static cheatseq_t cheatseq_noclip        = {"idspispopd", NULL};
static cheatseq_t cheatseq_invincibility = {"idbeholdv",  NULL};
static cheatseq_t cheatseq_berserk       = {"idbeholds",  NULL};
static cheatseq_t cheatseq_invisibility  = {"idbeholdi",  NULL};
static cheatseq_t cheatseq_map           = {"idbeholda",  NULL};
static cheatseq_t cheatseq_goggles       = {"idbeholdl",  NULL};
static cheatseq_t cheatseq_exit          = {"idclev",     NULL};
static cheatseq_t cheatseq_rockets       = {"idrocket",   NULL}; //Because Goldeneye!
static cheatseq_t cheatseq_fps           = {"idrate",     NULL};


static boolean cht_CheckCheat(cheatseq_t *cht, int8_t data1)
{
	if (!cht->p)
		cht->p = cht->sequence; // initialize if first time

	if (*cht->p == data1)
		cht->p++;
	else
		cht->p = cht->sequence;

	if (*cht->p == '\0') // end of sequence character
	{
		cht->p = cht->sequence;
		return true;
	}

    return false;
}


boolean C_Responder (event_t *ev)
{
	if (ev->type == ev_keydown)
	{
		uint8_t data1 = ev->data1;
		if (cht_CheckCheat(&cheatseq_choppers,      data1)) { cheat_choppers();      return true; }
		if (cht_CheckCheat(&cheatseq_god,           data1)) { cheat_god();           return true; }
		if (cht_CheckCheat(&cheatseq_kfa,           data1)) { cheat_idkfa();         return true; }
		if (cht_CheckCheat(&cheatseq_ammo,          data1)) { cheat_ammo();          return true; }
		if (cht_CheckCheat(&cheatseq_noclip,        data1)) { cheat_noclip();        return true; }
		if (cht_CheckCheat(&cheatseq_invincibility, data1)) { cheat_invincibility(); return true; }
		if (cht_CheckCheat(&cheatseq_berserk,       data1)) { cheat_berserk();       return true; }
		if (cht_CheckCheat(&cheatseq_invisibility,  data1)) { cheat_invisibility();  return true; }
		if (cht_CheckCheat(&cheatseq_map,           data1)) { cheat_map();           return true; }
		if (cht_CheckCheat(&cheatseq_goggles,       data1)) { cheat_goggles();       return true; }
		if (cht_CheckCheat(&cheatseq_exit,          data1)) { cheat_exit();          return true; }
		if (cht_CheckCheat(&cheatseq_rockets,       data1)) { cheat_rockets();       return true; }
		if (cht_CheckCheat(&cheatseq_fps,           data1)) { cheat_fps();           return true; }
	}

	return false;
}


static void cheat_god()
{
    _g_player.cheats ^= CF_GODMODE;

    if(_g_player.cheats & CF_GODMODE)
    {
        _g_player.health = god_health;

        _g_player.message = STSTR_DQDON;
    }
    else
    {
        _g_player.message = STSTR_DQDOFF;

    }
}

static void cheat_choppers()
{
    _g_player.weaponowned[wp_chainsaw] = true;
    _g_player.pendingweapon = wp_chainsaw;

    P_GivePower(&_g_player, pw_invulnerability);

    _g_player.message = STSTR_CHOPPERS;
}

static void cheat_idkfa()
{
    int16_t i;

    player_t* plyr = &_g_player;

    if (!plyr->backpack)
    {
        for (i=0 ; i<NUMAMMO ; i++)
            plyr->maxammo[i] *= 2;

        plyr->backpack = true;
    }

    plyr->armorpoints = idfa_armor;      // Ty 03/09/98 - deh
    plyr->armortype = idfa_armor_class;  // Ty 03/09/98 - deh

    // You can't own weapons that aren't in the game // phares 02/27/98
    for (i=0;i<NUMWEAPONS;i++)
        if (!(i == wp_plasma || i == wp_bfg || i == wp_supershotgun))
            plyr->weaponowned[i] = true;

    for (i=0;i<NUMAMMO;i++)
        if (i!=am_cell)
            plyr->ammo[i] = plyr->maxammo[i];

    for (i=0;i<NUMCARDS;i++)
            plyr->cards[i] = true;

    plyr->message = STSTR_KFAADDED;
}

static void cheat_ammo()
{
    int16_t i;
    player_t* plyr = &_g_player;

    if (!plyr->backpack)
    {
        for (i=0 ; i<NUMAMMO ; i++)
            plyr->maxammo[i] *= 2;

        plyr->backpack = true;
    }

    plyr->armorpoints = idfa_armor;      // Ty 03/09/98 - deh
    plyr->armortype = idfa_armor_class;  // Ty 03/09/98 - deh

    // You can't own weapons that aren't in the game // phares 02/27/98
    for (i=0;i<NUMWEAPONS;i++)
        if (!(i == wp_plasma || i == wp_bfg || i == wp_supershotgun))
            plyr->weaponowned[i] = true;

    for (i=0;i<NUMAMMO;i++)
        if (i!=am_cell)
            plyr->ammo[i] = plyr->maxammo[i];

    plyr->message = STSTR_FAADDED;
}

static void cheat_noclip()
{
    _g_player.cheats ^= CF_NOCLIP;

    if(_g_player.cheats & CF_NOCLIP)
    {
        _g_player.message = STSTR_NCON;
    }
    else
    {
        _g_player.message = STSTR_NCOFF;

    }
}

static void cheat_invincibility()
{
    P_GivePower(&_g_player, pw_invulnerability);
}

static void cheat_berserk()
{
    P_GivePower(&_g_player, pw_strength);
}

static void cheat_invisibility()
{
    P_GivePower(&_g_player, pw_invisibility);
}

static void cheat_map()
{
    P_GivePower(&_g_player, pw_allmap);
}

static void cheat_goggles()
{
    P_GivePower(&_g_player, pw_infrared);
}

static void cheat_exit()
{
    G_ExitLevel();
}

static void cheat_rockets()
{
    _g_player.cheats ^= CF_ENEMY_ROCKETS;

    if(_g_player.cheats & CF_ENEMY_ROCKETS)
    {
        _g_player.health = god_health;

        _g_player.weaponowned[wp_missile] = true;
        _g_player.ammo[am_misl] = _g_player.maxammo[am_misl];

        _g_player.pendingweapon = wp_missile;

        _g_player.message = STSTR_ROCKETON;
    }
    else
    {
        _g_player.message = STSTR_ROCKETOFF;
    }
}

static void cheat_fps()
{
    _g_fps_show = !_g_fps_show;
	if(_g_fps_show)
	{
		_g_player.message = STSTR_FPSON;
	}else
	{
		_g_player.message = STSTR_FPSOFF;
	}
}
