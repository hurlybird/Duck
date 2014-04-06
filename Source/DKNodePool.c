/*****************************************************************************************

  DKNodePool.c

  Copyright (c) 2014 Derek W. Nylen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*****************************************************************************************/

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
















