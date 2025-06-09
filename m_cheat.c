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
static void cheat_rad(void);
static void cheat_map(void);
static void cheat_goggles(void);
static void cheat_exit(void);
static void cheat_end(void);
static void cheat_rockets(void);
static void cheat_fps(void);


typedef struct
{
	void (*cheat_function)(void);
	char *sequence;
	char *p;
} cheatseq_t;


static cheatseq_t cheat_def[] =
{
	{cheat_choppers,      "idchoppers", NULL},
	{cheat_god,           "iddqd",      NULL},
	{cheat_idkfa,         "idkfa",      NULL},
	{cheat_ammo,          "idfa",       NULL},
	{cheat_noclip,        "idspispopd", NULL},
	{cheat_invincibility, "idbeholdv",  NULL},
	{cheat_berserk,       "idbeholds",  NULL},
	{cheat_invisibility,  "idbeholdi",  NULL},
	{cheat_rad,           "idbeholdr",  NULL},
	{cheat_map,           "idbeholda",  NULL},
	{cheat_goggles,       "idbeholdl",  NULL},
	{cheat_exit,          "idclev",     NULL},
	{cheat_end,           "idend",      NULL},
	{cheat_rockets,       "idrocket",   NULL}, // Because Goldeneye!
	{cheat_fps,           "idrate",     NULL}
};


static const int16_t num_cheats = sizeof(cheat_def) / sizeof (cheatseq_t);


static boolean cht_CheckCheat(cheatseq_t *cht, char data1)
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
		for (int16_t i = 0; i < num_cheats; i++)
		{
			if (cht_CheckCheat(&cheat_def[i], ev->data1))
			{
				cheat_def[i].cheat_function();
				return true;
			}
		}
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

static void cheat_rad()
{
    P_GivePower(&_g_player, pw_ironfeet);
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

static void cheat_end()
{
	_g_gameaction = ga_victory;
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
