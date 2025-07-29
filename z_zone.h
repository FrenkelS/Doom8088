// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2023-2025 by Frenkel Smeijers
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//      Zone Memory Allocation, perhaps NeXT ObjectiveC inspired.
//	Remark: this was the only stuff that, according
//	 to John Carmack, might have been useful for
//	 Quake.
//
//---------------------------------------------------------------------

#ifndef __Z_ZONE__
#define __Z_ZONE__

#include <stddef.h>
#include "doomtype.h"

void Z_Init(void);
boolean Z_InitXms(uint32_t size);
void Z_MoveConventionalMemoryToExtendedMemory(uint32_t dest, const void __far* src, uint16_t length);
void Z_MoveExtendedMemoryToConventionalMemory(void __far* dest, uint32_t src, uint16_t length);
void Z_Shutdown(void);
boolean Z_IsEnoughFreeMemory(uint16_t size);
void __far* Z_TryMallocStatic(uint16_t size);
void __far* Z_MallocStatic(uint16_t size);
void __far* Z_MallocStaticWithUser(uint16_t size, void __far*__far* user); 
void __far* Z_MallocLevel(uint16_t size, void __far*__far* user);
void __far* Z_CallocLevel(uint16_t size);
void __far* Z_CallocLevSpec(uint16_t size);
void Z_ChangeTagToStatic(const void __far* ptr);
void Z_ChangeTagToCache(const void __far* ptr);
void Z_Free(const void __far* ptr);
void Z_FreeTags(void);
void Z_CheckHeap(void);

boolean Z_EqualNames(const char __far* farName, const char* nearName);

#endif
