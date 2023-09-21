#include <stdint.h>
#include <stdlib.h>
#include "globdata.h"

#include "z_zone.h"

static globals_t globals;
globals_t* _g = &globals;

void InitGlobals()
{
    memset(_g, 0, sizeof(globals_t));


//******************************************************************************
//d_main.c
//******************************************************************************

    _g->wipegamestate = GS_DEMOSCREEN;
    _g->oldgamestate  = -1;


//******************************************************************************
//m_menu.c
//******************************************************************************

    _g->showMessages = 0;    // Show messages has default, 0 = off, 1 = on


//******************************************************************************
//r_main.c
//******************************************************************************

    _g->validcount = 1;         // increment every time a check is made


    _g->freehead = &_g->freetail;     // killough


//******************************************************************************
//s_sounds.c
//******************************************************************************

    _g->snd_SfxVolume = 15;

    // Maximum volume of music. Useless so far.
    _g->snd_MusicVolume = 15;
}
