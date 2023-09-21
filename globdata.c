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
}
