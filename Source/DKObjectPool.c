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
static DKObjectPoolBlock * DKObjectPoolAllocBlock( DKObjectPool * pool, DKIndex count )
{
    if( count < MIN_RESERVE_NODE_COUNT )
        count = MIN_RESERVE_NODE_COUNT;

    DKIndex bytes = sizeof(DKObjectPoolBlock) + (pool->size * count);
    DKObjectPoolBlock * block = dk_malloc( bytes );
    
    block->next = NULL;
    block->count = count;
    
    uint8_t * firstObject = (uint8_t *)block + sizeof(DKObjectPoolBlock);
    
    for( DKIndex i = 0; i < count; ++i )
    {
        void * node = firstObject + (pool->size * i);
        DKObjectPoolFree( pool, node );
    }
    
    return block;
}


///
//  DKObjectPoolAddBlock()
//
static void DKObjectPoolAddBlock( DKObjectPool * pool, DKIndex count )
{
    if( pool->blockList )
    {
        DKObjectPoolBlock * newBlock = DKObjectPoolAllocBlock( pool, pool->count );
        
        newBlock->next = pool->blockList;
        pool->blockList = newBlock;
        pool->count += newBlock->count;
    }
    
    else
    {
        pool->blockList = DKObjectPoolAllocBlock( pool, count );
        pool->count = pool->blockList->count;
    }
}


///
//  DKObjectPoolInit()
//
void DKObjectPoolInit( DKObjectPool * pool, DKIndex size, DKIndex count )
{
    pool->freeList = NULL;
    pool->blockList = NULL;
    pool->size = size;
    pool->count = 0;
    pool->mutex = DKSpinLockInit;
    
    if( count > 0 )
        DKObjectPoolAddBlock( pool, count );
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
        DKObjectPoolAddBlock( pool, 0 );
        
    DKObjectPoolFreeNode * node = pool->freeList;
    pool->freeList = node->next;

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
            DKObjectPoolAddBlock( pool, 0 );
     
        DKSpinLockUnlock( &pool->mutex );
    }
        
    DKObjectPoolFreeNode * node = pool->freeList;
    DKObjectPoolFreeNode * next = node->next;
    
    while( !DKAtomicCmpAndSwapPtr( &pool->freeList, node, next ) )
    {
        node = pool->freeList;
        next = node->next;
    }
    
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
}
