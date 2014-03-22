//
//  DKMemory.c
//  Duck
//
//  Created by Derek Nylen on 12-02-29.
//  Copyright (c) 2012 Derek W. Nylen. All rights reserved.
//

#include "DKMemory.h"


// DKByteArray ===========================================================================

#define MIN_BYTE_ARRAY_SIZE 64

///
//  DKByteArrayInit()
//
void DKByteArrayInit( DKByteArray * array )
{
    array->data = NULL;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKByteArrayReserve()
//
void DKByteArrayReserve( DKByteArray * array, DKIndex length )
{
    if( array->maxLength < length )
    {
        if( length < MIN_BYTE_ARRAY_SIZE )
            length = MIN_BYTE_ARRAY_SIZE;
    
        uint8_t * data = DKAlloc( length );
        
        if( array->length > 0 )
        {
            memcpy( data, array->data, array->length );
            DKFree( array->data );
        }
        
        array->data = data;
        array->maxLength = length;
    }
}


///
//  DKByteArrayClear()
//
void DKByteArrayClear( DKByteArray * array )
{
    DKFree( array->data );
    array->data = NULL;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKByteArrayResize()
//
static void * DKByteArrayResize( void * ptr, DKIndex oldSize, DKIndex requestedSize, DKIndex * allocatedSize )
{
    if( requestedSize < oldSize )
        return ptr;
    
    DKIndex newSize = 2 * oldSize;
    
    if( newSize < requestedSize )
        newSize = requestedSize;
    
    if( newSize < MIN_BYTE_ARRAY_SIZE )
        newSize = MIN_BYTE_ARRAY_SIZE;
    
    *allocatedSize = newSize;
    
    return DKAlloc( newSize );
}


///
//  DKByteArrayReplaceBytes()
//
void DKByteArrayReplaceBytes( DKByteArray * array, DKRange range, const void * bytes, DKIndex length )
{
    DKIndex range_end = DKRangeEnd( range );

    assert( range.location >= 0 );
    assert( range.length >= 0 );
    assert( range_end <= array->length );
    assert( length >= 0 );

    // Do nothing
    if( (range.length == 0) && (length == 0) )
        return;

    DKRange prefixRange = DKRangeMake( 0, range.location );
    DKRange insertedRange = DKRangeMake( range.location, length );
    DKRange suffixRangeBeforeInsertion = DKRangeMake( range_end, array->length - range_end );
    DKRange suffixRangeAfterInsertion = DKRangeMake( range.location + length, array->length - range_end );

    // Resize
    DKIndex newLength = array->length + length - range.length;
    assert( newLength >= 0 );

    // Resize
    uint8_t * data = DKByteArrayResize( array->data, array->maxLength, newLength, &array->maxLength );
    
    if( array->data )
    {
        if( array->data != data )
        {
            // Copy prefix
            if( prefixRange.length > 0 )
            {
                memcpy( data, array->data, prefixRange.length );
            }
            
            // Copy suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uint8_t * dst = &data[suffixRangeAfterInsertion.location];
                uint8_t * src = &array->data[suffixRangeBeforeInsertion.location];
                memcpy( dst, src, suffixRangeBeforeInsertion.length );
            }
            
            DKFree( array->data );
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
    
    array->data = data;
    array->length = newLength;
    
    // Insert or Extend
    if( insertedRange.length > 0 )
    {
        uint8_t * dst = &array->data[insertedRange.location];
        
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




// DKPointerArray ========================================================================

///
//  DKElementArrayInit()
//
void DKElementArrayInit( DKElementArray * array, DKIndex elementSize )
{
    DKByteArrayInit( &array->byteArray );
    array->elementSize = elementSize;
}


///
//  DKElementArrayReserve()
//
void DKElementArrayReserve( DKElementArray * array, DKIndex length )
{
    DKByteArrayReserve( &array->byteArray, length * array->elementSize );
}


///
//  DKElementArrayClear()
void DKElementArrayClear( DKElementArray * array )
{
    DKByteArrayClear( &array->byteArray );
}


///
//  DKElementArrayReplaceElements()
//
void DKElementArrayReplaceElements( DKElementArray * array, DKRange range, const void * elements, DKIndex length )
{
    DKRange byteRange = DKRangeMake( range.location * array->elementSize, range.length * array->elementSize );
    DKIndex byteLength = length * array->elementSize;
    
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, elements, byteLength );
}


///
//  DKElementArrayGetCount()
//
DKIndex DKElementArrayGetCount( const DKElementArray * array )
{
    return array->byteArray.length / array->elementSize;
}


///
//  DKElementArrayAppendElement()
//
void DKElementArrayAppendElement( DKElementArray * array, const void * element )
{
    DKRange byteRange = DKRangeMake( array->byteArray.length, 0 );
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, element, array->elementSize );
}


///
//  DKElementArrayInsertElementAtIndex()
//
void DKElementArrayInsertElementAtIndex( DKElementArray * array, DKIndex index, const void * element )
{
    DKRange byteRange = DKRangeMake( index * array->elementSize, 0 );
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, element, array->elementSize );
}


///
//  DKElementArraySetElementAtIndex()
//
void DKElementArraySetElementAtIndex( DKElementArray * array, DKIndex index, const void * element )
{
    DKRange byteRange = DKRangeMake( index * array->elementSize, array->elementSize );
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, element, array->elementSize );
}


///
//  DKElementArrayRemoveElementAtIndex()
//
void DKElementArrayRemoveElementAtIndex( DKElementArray * array, DKIndex index )
{
    DKRange byteRange = DKRangeMake( index * array->elementSize, array->elementSize );
    DKByteArrayReplaceBytes( &array->byteArray, byteRange, NULL, 0 );
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
















