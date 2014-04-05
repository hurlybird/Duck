//
//  DKNodePool.c
//  Duck
//
//  Created by Derek Nylen on 12-02-29.
//  Copyright (c) 2012 Derek W. Nylen. All rights reserved.
//

#include "DKNodePool.h"

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
    DKNodePoolBlock * block = dk_malloc( bytes );
    
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
//  DKNodePoolFinalize()
//
void DKNodePoolFinalize( DKNodePool * pool )
{
    DKNodePoolBlock * block = pool->blockList;
    
    while( block )
    {
        DKNodePoolBlock * tmp = block;
        block = block->next;
        dk_free( tmp );
    }
    
    pool->freeList = NULL;
    pool->blockList = NULL;
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
















