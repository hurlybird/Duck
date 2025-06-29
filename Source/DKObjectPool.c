/*****************************************************************************************

  DKObjectPool.c

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

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKObjectPool.h"

#define MIN_RESERVE_NODE_COUNT 32


///
//  DKObjectPoolAllocBlock()
//
static DKObjectPoolBlock * DKObjectPoolAllocBlock( DKObjectPool * pool, size_t count )
{
    if( count < MIN_RESERVE_NODE_COUNT )
        count = MIN_RESERVE_NODE_COUNT;

    DKIndex bytes = sizeof(DKObjectPoolBlock) + (pool->blockSize * count);
    DKObjectPoolBlock * block = dk_malloc( bytes );
    
    block->next = NULL;
    block->count = count;
    
    uint8_t * nodes = (uint8_t *)block + sizeof(DKObjectPoolBlock);
    
    DKObjectPoolFreeNode * cursor = (DKObjectPoolFreeNode *)nodes;
    
    for( size_t i = 1; i < count; ++i )
    {
        cursor->next = (DKObjectPoolFreeNode *)(nodes + (pool->blockSize * i));
        cursor = cursor->next;
    }
    
    DKObjectPoolFreeNode * last = (DKObjectPoolFreeNode *)(nodes + (pool->blockSize * (count - 1)));
    last->next = pool->freeList;
    pool->freeList = (DKObjectPoolFreeNode *)nodes;
    
    return block;
}


///
//  DKObjectPoolAddBlock()
//
static void DKObjectPoolAddBlock( DKObjectPool * pool )
{
    DKObjectPoolBlock * newBlock = DKObjectPoolAllocBlock( pool, pool->reserved );
    
    if( pool->blockList )
    {
        newBlock->next = pool->blockList;
        pool->blockList = newBlock;
        pool->reserved += newBlock->count;
    }
    
    else
    {
        pool->blockList = newBlock;
        pool->reserved = pool->blockList->count;
    }
}


///
//  DKObjectPoolInit()
//
void DKObjectPoolInit( DKObjectPool * pool, size_t size, size_t reserve )
{
    pool->freeList = NULL;
    pool->blockList = NULL;
    pool->blockSize = size;
    pool->reserved = reserve;
    pool->allocated = 0;
    pool->mutex = DKSpinLockInit;
    
    if( reserve > 0 )
        DKObjectPoolAddBlock( pool );
}


///
//  DKObjectPoolFinalize()
//
void DKObjectPoolFinalize( DKObjectPool * pool )
{
    DKObjectPoolBlock * block = pool->blockList;
    
    while( block )
    {
        DKObjectPoolBlock * tmp = block;
        block = block->next;
        dk_free( tmp );
    }
    
    pool->freeList = NULL;
    pool->blockList = NULL;
}


///
//  DKObjectPoolAlloc()
//
void * DKObjectPoolAlloc( DKObjectPool * pool )
{
    if( pool->freeList == NULL )
        DKObjectPoolAddBlock( pool );
        
    DKObjectPoolFreeNode * node = pool->freeList;
    pool->freeList = node->next;
    
#if DK_RUNTIME_STATS
    pool->allocated++;
#endif

    return node;
}


///
//  DKObjectPoolThreadSafeAlloc()
//
void * DKObjectPoolThreadSafeAlloc( DKObjectPool * pool )
{
    if( pool->freeList == NULL )
    {
        DKSpinLockUnlock( &pool->mutex );
        
        if( pool->freeList == NULL )
            DKObjectPoolAddBlock( pool );
     
        DKSpinLockUnlock( &pool->mutex );
    }
        
    DKObjectPoolFreeNode * node = pool->freeList;
    DKObjectPoolFreeNode * next = node->next;
    
    while( !DKAtomicCmpAndSwapPtr( &pool->freeList, node, next ) )
    {
        node = pool->freeList;
        next = node->next;
    }

#if DK_RUNTIME_STATS
    DKAtomicIncrement64( &pool->allocated );
#endif
    
    return node;
}


///
//  DKObjectPoolFree()
//
void DKObjectPoolFree( DKObjectPool * pool, void * _node )
{
    DKObjectPoolFreeNode * node = _node;
    
    node->next = pool->freeList;
    pool->freeList = node;

#if DK_RUNTIME_STATS
    pool->allocated--;
#endif
}

///
//  DKObjectPoolThreadSafeFree()
//
void DKObjectPoolThreadSafeFree( DKObjectPool * pool, void * _node )
{
    DKObjectPoolFreeNode * node = _node;
    
    DKObjectPoolFreeNode * next = pool->freeList;
    node->next = next;
    
    while( !DKAtomicCmpAndSwapPtr( &pool->freeList, next, node ) )
    {
        next = pool->freeList;
        node->next = next;
    }

#if DK_RUNTIME_STATS
    DKAtomicDecrement64( &pool->allocated );
#endif
}
