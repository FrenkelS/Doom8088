// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2023 by Frenkel Smeijers
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
// $Log:$
//
// DESCRIPTION:
//	Zone Memory Allocation. Neat.
//
//-----------------------------------------------------------------------------

#include <malloc.h>
#include <stdint.h>
#include "compiler.h"
#include "z_zone.h"
#include "doomdef.h"
#include "doomtype.h"
#include "i_system.h"


//
// ZONE MEMORY
// PU - purge tags.
// Tags < 100 are not overwritten until freed.
#define PU_STATIC		1	// static entire execution time
#define PU_LEVEL		2	// static until level exited
#define PU_LEVSPEC		3      // a special thinker in a level
#define PU_CACHE		4

#define PU_PURGELEVEL PU_CACHE


//
// ZONE MEMORY ALLOCATION
//
// There is never any space between memblocks,
//  and there will never be two contiguous free memblocks.
// The rover can be left pointing at a non-empty block.
//
// It is of no value to free a cachable block,
//  because it will get overwritten automatically if needed.
//

#if defined INSTRUMENTED
    static int32_t running_count = 0;
#endif


#define	ZONEID	0x1dea

typedef struct
{
    uint32_t  size:24;	// including the header and possibly tiny fragments
    uint32_t  tag:4;	// purgelevel
    void**    user;		// NULL if a free block
    segment_t next;
    segment_t prev;
#if defined _M_I86
    uint16_t id;		// should be ZONEID
#endif
} memblock_t;

#define UNOWNED MK_FP(0,2)

#define PARAGRAPH_SIZE 16

typedef char assertMemblockSize[sizeof(memblock_t) <= PARAGRAPH_SIZE ? 1 : -1];


static uint8_t    *mainzone;
static uint8_t     mainzone_blocklist_buffer[32];
static memblock_t *mainzone_blocklist;
static segment_t   mainzone_rover_segment;


static segment_t pointerToSegment(const memblock_t* ptr)
{
	if ((((uint32_t) ptr) & (PARAGRAPH_SIZE - 1)) != 0)
		I_Error("pointerToSegment: pointer is not aligned: 0x%lx", ptr);

	uint32_t seg = FP_SEG(ptr);
	uint16_t off = FP_OFF(ptr);
	uint32_t linear = seg * PARAGRAPH_SIZE + off;
	return linear / PARAGRAPH_SIZE;
}

static memblock_t* segmentToPointer(segment_t seg)
{
	return MK_FP(seg, 0);
}


//
// Z_Init
//
void Z_Init (void)
{
    memblock_t*	block;

    uint32_t heapSize;
    int32_t hallocNumb = 640 * 1024L / PARAGRAPH_SIZE;

    //Try to allocate memory.
    do
    {
        mainzone = halloc(hallocNumb, PARAGRAPH_SIZE);
        hallocNumb--;

    } while (mainzone == NULL);

    hallocNumb++;
    heapSize = hallocNumb * PARAGRAPH_SIZE;

    //align mainzone
    uint32_t m = (uint32_t) mainzone;
    if ((m & (PARAGRAPH_SIZE - 1)) != 0)
    {
        heapSize -= PARAGRAPH_SIZE;
        while ((m & (PARAGRAPH_SIZE - 1)) != 0)
            m = (uint32_t) ++mainzone;
    }

    printf("\t%ld bytes allocated for zone\n", heapSize);

    //align blocklist
    uint_fast8_t i = 0;
    uint32_t b = (uint32_t) &mainzone_blocklist_buffer[i++];
    while ((b & (PARAGRAPH_SIZE - 1)) != 0)
        b = (uint32_t) &mainzone_blocklist_buffer[i++];
    mainzone_blocklist = (memblock_t *)b;

    // set the entire zone to one free block
    block = (memblock_t *)mainzone;
    mainzone_rover_segment = pointerToSegment(block);

    mainzone_blocklist->next = mainzone_rover_segment;
    mainzone_blocklist->prev = mainzone_rover_segment;

    mainzone_blocklist->user = (void *)mainzone;
    mainzone_blocklist->tag  = PU_STATIC;

    block->prev = block->next = pointerToSegment(mainzone_blocklist);

    // NULL indicates a free block.
    block->user = NULL;

    block->size = heapSize;
}


void Z_ChangeTagToStatic(const void* ptr)
{
	memblock_t* block = segmentToPointer(pointerToSegment(ptr) - 1);
#if defined _M_I86
	if (block->id != ZONEID)
		I_Error("Z_ChangeTagToStatic: block has id %x instead of ZONEID", block->id);
#endif
	block->tag = PU_STATIC;
}


void Z_ChangeTagToCache(const void* ptr)
{
	memblock_t* block = segmentToPointer(pointerToSegment(ptr) - 1);
#if defined _M_I86
	if (block->id != ZONEID)
		I_Error("Z_ChangeTagToCache: block has id %x instead of ZONEID", block->id);
#endif
	block->tag = PU_CACHE;
}


static void Z_FreeBlock(memblock_t* block)
{
#if defined _M_I86
    if (block->id != ZONEID)
        I_Error("Z_FreeBlock: block has id %x instead of ZONEID", block->id);
#endif

    if (FP_SEG(block->user) != 0)
    {
        // far pointers with segment 0 are not user pointers
        // Note: OS-dependend

        // clear the user's mark
        *block->user = NULL;
    }

    // mark as free
    block->user = NULL;
    block->tag  = 0;


#if defined INSTRUMENTED
    running_count -= block->size;
    printf("Free: %ld\n", running_count);
#endif

    memblock_t* other = segmentToPointer(block->prev);

    if (!other->user)
    {
        // merge with previous free block
        other->size += block->size;
        other->next  = block->next;
        segmentToPointer(other->next)->prev = block->prev; // == pointerToSegment(other);

        if (pointerToSegment(block) == mainzone_rover_segment)
            mainzone_rover_segment = block->prev; // == pointerToSegment(other);

        block = other;
    }

    other = segmentToPointer(block->next);
    if (!other->user)
    {
        // merge the next free block onto the end
        block->size += other->size;
        block->next  = other->next;
        segmentToPointer(block->next)->prev = pointerToSegment(block);

        if (pointerToSegment(other) == mainzone_rover_segment)
            mainzone_rover_segment = pointerToSegment(block);
    }
}


//
// Z_Free
//
void Z_Free (const void* ptr)
{
	Z_FreeBlock(segmentToPointer(pointerToSegment(ptr) - 1));
}


static uint32_t Z_GetLargestFreeBlockSize(void)
{
	uint32_t largestFreeBlockSize = 0;

	segment_t mainzone_blocklist_segment = pointerToSegment(mainzone_blocklist);

	for (memblock_t* block = segmentToPointer(mainzone_blocklist->next); pointerToSegment(block) != mainzone_blocklist_segment; block = segmentToPointer(block->next))
		if (!block->user && block->size > largestFreeBlockSize)
			largestFreeBlockSize = block->size;

	return largestFreeBlockSize;
}

static uint32_t Z_GetTotalFreeMemory(void)
{
	uint32_t totalFreeMemory = 0;

	segment_t mainzone_blocklist_segment = pointerToSegment(mainzone_blocklist);

	for (memblock_t* block = segmentToPointer(mainzone_blocklist->next); pointerToSegment(block) != mainzone_blocklist_segment; block = segmentToPointer(block->next))
		if (!block->user)
			totalFreeMemory += block->size;

	return totalFreeMemory;
}


//
// Z_Malloc
// You can pass a NULL user if the tag is < PU_PURGELEVEL.
//
#define MINFRAGMENT		64


static void* Z_Malloc(int32_t size, int32_t tag, void **user)
{
    size = (size + (PARAGRAPH_SIZE - 1)) & ~(PARAGRAPH_SIZE - 1);

    // scan through the block list,
    // looking for the first free block
    // of sufficient size,
    // throwing out any purgable blocks along the way.

    // account for size of block header
    size += PARAGRAPH_SIZE;

    // if there is a free block behind the rover,
    //  back up over them
    memblock_t* base = segmentToPointer(mainzone_rover_segment);

    memblock_t* previous_block = segmentToPointer(base->prev);
    if (!previous_block->user)
        base = previous_block;

    memblock_t* rover         = base;
    segment_t   start_segment = base->prev;

    do
    {
        if (pointerToSegment(rover) == start_segment)
        {
            // scanned all the way around the list
            I_Error ("Z_Malloc: failed to allocate %li B, max free block %li B, total free %li", size, Z_GetLargestFreeBlockSize(), Z_GetTotalFreeMemory());
        }

        if (rover->user)
        {
            if (rover->tag < PU_PURGELEVEL)
            {
                // hit a block that can't be purged,
                //  so move base past it
                base = rover = segmentToPointer(rover->next);
            }
            else
            {
                // free the rover block (adding the size to base)

                // the rover can be the base block
                base  = segmentToPointer(base->prev);
                Z_FreeBlock(rover);
                base  = segmentToPointer(base->next);
                rover = segmentToPointer(base->next);
            }
        }
        else
            rover = segmentToPointer(rover->next);

    } while (base->user || base->size < size);
    // found a block big enough

    int32_t newblock_size = base->size - size;
    if (newblock_size > MINFRAGMENT)
    {
        // there will be a free fragment after the allocated block
        segment_t base_segment     = pointerToSegment(base);
        segment_t newblock_segment = base_segment + size / PARAGRAPH_SIZE;

        memblock_t* newblock = segmentToPointer(newblock_segment);
        newblock->size = newblock_size;
        newblock->user = NULL; // NULL indicates free block.
        newblock->tag  = 0;
#if defined _M_I86
        newblock->id   = ZONEID;
#endif
        newblock->prev = base_segment;
        newblock->next = base->next;

        segmentToPointer(base->next)->prev = newblock_segment;
        base->next = newblock_segment;
        base->size = size;
    }

    if (user)
    {
        // mark as an in use block
        base->user = user;
    }
    else
    {
        if (tag >= PU_PURGELEVEL)
            I_Error ("Z_Malloc: an owner is required for purgable blocks");

        // mark as in use, but unowned
        base->user = UNOWNED;
    }

    base->tag = tag;
#if defined _M_I86
    base->id  = ZONEID;
#endif

    // next allocation will start looking here
    mainzone_rover_segment = base->next;

#if defined INSTRUMENTED
    running_count += base->size;
    printf("Alloc: %ld (%ld)\n", base->size, running_count);
#endif

    return segmentToPointer(pointerToSegment(base) + 1);
}


void* Z_MallocStatic(int32_t size)
{
	return Z_Malloc(size, PU_STATIC, NULL);
}


void* Z_MallocStaticWithUser(int32_t size, void **user)
{
	return Z_Malloc(size, PU_STATIC, user);
}


void* Z_MallocLevel(int32_t size, void **user)
{
	return Z_Malloc(size, PU_LEVEL, user);
}


void* Z_CallocLevel(int32_t size)
{
    void* ptr = Z_Malloc(size, PU_LEVEL, NULL);
    memset(ptr, 0, size);
    return ptr;
}


void* Z_CallocLevSpec(int32_t size)
{
	void *ptr = Z_Malloc(size, PU_LEVSPEC, NULL);
	memset(ptr, 0, size);
	return ptr;
}


//
// Z_FreeTags
//
void Z_FreeTags(void)
{
    memblock_t*	next;

    for (memblock_t* block = segmentToPointer(mainzone_blocklist->next); pointerToSegment(block) != pointerToSegment(mainzone_blocklist); block = next)
    {
        // get link before freeing
        next = segmentToPointer(block->next);

        // already a free block?
        if (!block->user)
            continue;

        if (PU_LEVEL <= block->tag && block->tag <= (PU_PURGELEVEL - 1))
            Z_FreeBlock(block);
    }
}

//
// Z_CheckHeap
//
void Z_CheckHeap (void)
{
    segment_t mainzone_blocklist_segment = pointerToSegment(mainzone_blocklist);

    for (memblock_t* block = segmentToPointer(mainzone_blocklist->next); ; block = segmentToPointer(block->next))
    {
        if (block->next == mainzone_blocklist_segment)
        {
            // all blocks have been hit
            break;
        }

#if defined _M_I86
        if (block->id != ZONEID)
            I_Error("Z_CheckHeap: block has id %x instead of ZONEID", block->id);
#endif

        if (pointerToSegment(block) + (block->size / PARAGRAPH_SIZE) != block->next)
            I_Error ("Z_CheckHeap: block size does not touch the next block\n");

        if (segmentToPointer(block->next)->prev != pointerToSegment(block))
            I_Error ("Z_CheckHeap: next block doesn't have proper back link\n");

        if (!block->user && !segmentToPointer(block->next)->user)
            I_Error ("Z_CheckHeap: two consecutive free blocks\n");
    }
}
