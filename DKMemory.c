//
//  DKMemory.c
//  Duck
//
//  Created by Derek Nylen on 12-02-29.
//  Copyright (c) 2012 Derek W. Nylen. All rights reserved.
//

#include "DKMemory.h"


// DKMemorySegment =======================================================================

#define MIN_DATA_SIZE 64

///
//  DKMemorySegmentInit()
//
void DKMemorySegmentInit( DKMemorySegment * segment )
{
    segment->data = NULL;
    segment->length = 0;
    segment->maxLength = 0;
}


///
//  DKMemorySegmentClear()
//
void DKMemorySegmentClear( DKMemorySegment * segment )
{
    DKFree( segment->data );
    segment->data = NULL;
    segment->length = 0;
    segment->maxLength = 0;
}


///
//  DKMemorySegmentResize()
//
static void * DKMemorySegmentResize( void * ptr, DKIndex oldSize, DKIndex requestedSize, DKIndex * allocatedSize )
{
    if( requestedSize < oldSize )
        return ptr;
    
    DKIndex newSize = 2 * oldSize;
    
    if( newSize < requestedSize )
        newSize = requestedSize;
    
    if( newSize < MIN_DATA_SIZE )
        newSize = MIN_DATA_SIZE;
    
    *allocatedSize = newSize;
    
    return DKAlloc( newSize );
}


///
//  DKMemorySegmentReplaceBytes()
//
void DKMemorySegmentReplaceBytes( DKMemorySegment * segment, DKRange range, const void * bytes, DKIndex length )
{
    DKIndex range_end = DKRangeEnd( range );

    assert( range.location >= 0 );
    assert( range.length >= 0 );
    assert( range_end <= segment->length );
    assert( length >= 0 );

    // Do nothing
    if( (range.length == 0) && (length == 0) )
        return;

    DKRange prefixRange = DKRangeMake( 0, range.location );
    DKRange insertedRange = DKRangeMake( range.location, length );
    DKRange suffixRangeBeforeInsertion = DKRangeMake( range_end, segment->length - range_end );
    DKRange suffixRangeAfterInsertion = DKRangeMake( range.location + length, segment->length - range_end );

    // Resize
    DKIndex newLength = segment->length + length - range.length;
    assert( newLength >= 0 );

    // Resize
    uint8_t * data = DKMemorySegmentResize( segment->data, segment->maxLength, newLength, &segment->maxLength );
    
    if( segment->data )
    {
        if( segment->data != data )
        {
            // Copy prefix
            if( prefixRange.length > 0 )
            {
                memcpy( data, segment->data, prefixRange.length );
            }
            
            // Copy suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uint8_t * dst = &data[suffixRangeAfterInsertion.location];
                uint8_t * src = &segment->data[suffixRangeBeforeInsertion.location];
                memcpy( dst, src, suffixRangeBeforeInsertion.length );
            }
            
            DKFree( segment->data );
        }
        
        else
        {
            // Shift suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uint8_t * dst = &data[suffixRangeAfterInsertion.location];
                uint8_t * src = &data[suffixRangeBeforeInsertion.location];
                memmove( dst, src, suffixRangeBeforeInsertion.length );
            }
        }
    }
    
    segment->data = data;
    segment->length = newLength;
    
    // Insert or Extend
    if( insertedRange.length > 0 )
    {
        uint8_t * dst = &segment->data[insertedRange.location];
        
        if( bytes != NULL )
        {
            memcpy( dst, bytes, insertedRange.length );
        }
        
        else
        {
            memset( dst, 0, insertedRange.length );
        }
    }
}




// DKNodePool ============================================================================

#define MIN_RESERVE_NODE_COUNT 32

typedef struct DKNodePoolFreeNode
{
    struct DKNodePoolFreeNode * next;
    
} DKNodePoolFreeNode;

typedef struct DKNodePoolBlock
{
    struct DKNodePoolBlock * next;
    DKIndex count;

} DKNodePoolBlock;


///
//  DKNodePoolAllocBlock()
//
static DKNodePoolBlock * DKNodePoolAllocBlock( DKNodePool * pool, DKIndex count )
{
    if( count < MIN_RESERVE_NODE_COUNT )
        count = MIN_RESERVE_NODE_COUNT;

    DKIndex bytes = sizeof(DKNodePoolBlock) + (pool->nodeSize * count);
    DKNodePoolBlock * block = DKAlloc( bytes );
    
    block->next = NULL;
    block->count = count;
    
    uint8_t * firstNode = (uint8_t *)block + sizeof(DKNodePoolBlock);
    
    for( DKIndex i = 0; i < count; ++i )
    {
        void * node = firstNode + (pool->nodeSize * i);
        DKNodePoolFree( pool, node );
    }
    
    return block;
}


///
//  DKNodePoolAddBlock()
//
static void DKNodePoolAddBlock( DKNodePool * pool, DKIndex count )
{
    if( pool->blockList )
    {
        count = 2 * pool->blockList->count;
        DKNodePoolBlock * newBlock = DKNodePoolAllocBlock( pool, count );
        
        newBlock->next = pool->blockList;
        pool->blockList = newBlock;
    }
    
    else
    {
        pool->blockList = DKNodePoolAllocBlock( pool, count );
    }
}


///
//  DKNodePoolInit()
//
void DKNodePoolInit( DKNodePool * pool, DKIndex nodeSize, DKIndex nodeCount )
{
    pool->freeList = NULL;
    pool->blockList = NULL;
    pool->nodeSize = nodeSize;
    
    if( nodeCount > 0 )
        DKNodePoolAddBlock( pool, nodeCount );
}


///
//  DKNodePoolClear()
//
void DKNodePoolClear( DKNodePool * pool )
{
    DKNodePoolBlock * block = pool->blockList;
    
    while( block )
    {
        DKNodePoolBlock * tmp = block;
        block = block->next;
        DKFree( tmp );
    }
}


///
//  DKNodePoolAlloc()
//
void * DKNodePoolAlloc( DKNodePool * pool )
{
    if( pool->freeList == NULL )
        DKNodePoolAddBlock( pool, 0 );
        
    DKNodePoolFreeNode * node = pool->freeList;
    pool->freeList = node->next;
    
    memset( node, 0, pool->nodeSize );
    
    return node;
}


///
//  DKNodePoolFree()
//
void DKNodePoolFree( DKNodePool * pool, void * node )
{
    DKNodePoolFreeNode * freeNode = (DKNodePoolFreeNode *)node;
    freeNode->next = pool->freeList;
    pool->freeList = freeNode;
}
















