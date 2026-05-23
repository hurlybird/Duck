// =======================================================================================
//
// DKObjectPool.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKObjectPool.h"

#define MIN_RESERVE_NODE_COUNT 32


///
//  DKObjectPoolAllocBlock()
//
static DKObjectPoolBlock * DKObjectPoolAllocBlock( DKObjectPool * pool, size_t count, DKObjectPoolFreeNode ** head, DKObjectPoolFreeNode ** tail )
{
    if( count < MIN_RESERVE_NODE_COUNT )
        count = MIN_RESERVE_NODE_COUNT;

    DKIndex bytes = sizeof(DKObjectPoolBlock) + (pool->objectSize * count);
    DKObjectPoolBlock * block = dk_malloc( bytes );
    
    block->next = NULL;
    block->count = count;
    
    uint8_t * nodes = (uint8_t *)block + sizeof(DKObjectPoolBlock);
    
    DKObjectPoolFreeNode * prev = (DKObjectPoolFreeNode *)nodes;
    
    for( size_t i = 1; i < count; ++i )
    {
        DKObjectPoolFreeNode * curr = (DKObjectPoolFreeNode *)(nodes + (pool->objectSize * i));

        prev->next = curr;
        prev = curr;
    }
    
    prev->next = NULL;

    *head = (DKObjectPoolFreeNode *)nodes;
    *tail = (DKObjectPoolFreeNode *)(nodes + (pool->objectSize * (count - 1)));

    return block;
}


///
//  DKObjectPoolAddBlock()
//
static void DKObjectPoolAddBlock( DKObjectPool * pool )
{
    DKSpinlockLock( &pool->mutex );
            
    if( DKAtomicLoadPtr( &pool->freeList ) == NULL )
    {
        DKObjectPoolFreeNode * head;
        DKObjectPoolFreeNode * tail;
        DKObjectPoolBlock * newBlock = DKObjectPoolAllocBlock( pool, pool->reserved, &head, &tail );
        
        newBlock->next = pool->blockList;
        pool->blockList = newBlock;
        pool->reserved += newBlock->count;

        tail->next = DKAtomicLoadPtr( &pool->freeList );
        
        while( !DKAtomicCompareAndSwapPtr( &pool->freeList, &tail->next, head ) )
            ;
    }
    
    DKSpinlockUnlock( &pool->mutex );
}


///
//  DKObjectPoolInit()
//
void DKObjectPoolInit( DKObjectPool * pool, size_t size, size_t reserve )
{
    pool->freeList = NULL;
    pool->blockList = NULL;
    pool->objectSize = (uint32_t)size;
    pool->reserved = reserve;
    pool->allocated = 0;
    pool->mutex = DKSpinlockInit;
    
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
//  DKObjectPoolGetReservedCount()
//
size_t DKObjectPoolGetReservedCount( DKObjectPool * pool )
{
    size_t count;
    
    DKSpinlockLock( &pool->mutex );
    count = pool->reserved;
    DKSpinlockUnlock( &pool->mutex );
    
    return count;
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
    DKObjectPoolFreeNode * node = DKAtomicLoadPtr( &pool->freeList );
    
    while( 1 )
    {
        if( node )
        {
            DKObjectPoolFreeNode * next = node->next;

            if( DKAtomicCompareAndSwapPtr( &pool->freeList, &node, next ) )
            {
#if DK_RUNTIME_STATS
                DKAtomicIncrement64( &pool->allocated );
#endif
                return node;
            }
        }
        
        else
        {
            DKObjectPoolAddBlock( pool );
            node = DKAtomicLoadPtr( &pool->freeList );
        }
    }
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
    node->next = DKAtomicLoadPtr( &pool->freeList );
    
    while( !DKAtomicCompareAndSwapPtr( &pool->freeList, &node->next, node ) )
        ;

#if DK_RUNTIME_STATS
    DKAtomicDecrement64( &pool->allocated );
#endif
}
