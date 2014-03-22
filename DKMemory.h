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


// DKByteArray ===========================================================================
typedef struct
{
    uint8_t * data;
    DKIndex length;
    DKIndex maxLength;

} DKByteArray;


void DKByteArrayInit( DKByteArray * array );
void DKByteArrayReserve( DKByteArray * array, DKIndex length );
void DKByteArrayClear( DKByteArray * array );
void DKByteArrayReplaceBytes( DKByteArray * array, DKRange range, const void * bytes, DKIndex length );



// DKElementArray ========================================================================
typedef struct
{
    DKByteArray byteArray;
    DKIndex elementSize;

} DKElementArray;

void DKElementArrayInit( DKElementArray * array, DKIndex elementSize );
void DKElementArrayReserve( DKElementArray * array, DKIndex length );
void DKElementArrayClear( DKElementArray * array );
void DKElementArrayReplaceElements( DKElementArray * array, DKRange range, const void * elements, DKIndex length );

DKIndex DKElementArrayGetCount( const DKElementArray * array );

void DKElementArrayAppendElement( DKElementArray * array, const void * element );
void DKElementArrayInsertElementAtIndex( DKElementArray * array, DKIndex index, const void * element );
void DKElementArraySetElementAtIndex( DKElementArray * array, DKIndex index, const void * element );
void DKElementArrayRemoveElementAtIndex( DKElementArray * array, DKIndex index );

#define DKElementArrayGetElementAtIndex( array, index, type ) \
    *((type *)&((array)->byteArray.data[(index) * (array)->elementSize]))




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
