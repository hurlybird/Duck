//
//  DKMemory.h
//  Duck
//
//  Created by Derek Nylen on 12-02-29.
//  Copyright (c) 2012 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_MEMORY_H_
#define _DK_MEMORY_H_

#include "DKEnv.h"


// DKMemorySegment =======================================================================
typedef struct
{
    uint8_t * data;
    DKIndex length;
    DKIndex maxLength;

} DKMemorySegment;


void DKMemorySegmentInit( DKMemorySegment * segment );
void DKMemorySegmentClear( DKMemorySegment * segment );
void DKMemorySegmentReplaceBytes( DKMemorySegment * segment, DKRange range, const void * bytes, DKIndex length );




// DKNodePool ============================================================================
typedef struct
{
    struct DKNodePoolFreeNode * freeList;
    struct DKNodePoolBlock * blockList;
    DKIndex nodeSize;
    
} DKNodePool;


void DKNodePoolInit( DKNodePool * pool, DKIndex nodeSize, DKIndex nodeCount );
void DKNodePoolClear( DKNodePool * pool );

void * DKNodePoolAlloc( DKNodePool * pool );
void DKNodePoolFree( DKNodePool * pool, void * node );



#endif // _DK_MEMORY_H_
