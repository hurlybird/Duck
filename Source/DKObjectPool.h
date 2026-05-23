// =======================================================================================
//
// DKObjectPool.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_OBJECT_POOL_H_
#define _DK_OBJECT_POOL_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct _DKObjectPoolFreeNode
{
    struct _DKObjectPoolFreeNode * next;
    
} DKObjectPoolFreeNode;

typedef struct _DKObjectPoolBlock
{
    struct _DKObjectPoolBlock * next;
    size_t count;

} DKObjectPoolBlock;

typedef struct
{
    DKAtomic(DKObjectPoolFreeNode *) freeList;
    DKObjectPoolBlock * blockList;
    DKSpinlock mutex;
    uint32_t objectSize;
    size_t reserved;
    DKAtomicInt64 allocated;
    
} DKObjectPool;


DK_API void DKObjectPoolInit( DKObjectPool * pool, size_t size, size_t reserve );
DK_API void DKObjectPoolFinalize( DKObjectPool * pool );

DK_API void * DKObjectPoolAlloc( DKObjectPool * pool );
DK_API void * DKObjectPoolThreadSafeAlloc( DKObjectPool * pool );

DK_API void DKObjectPoolFree( DKObjectPool * pool, void * node );
DK_API void DKObjectPoolThreadSafeFree( DKObjectPool * pool, void * node );

#define DKObjectPoolGetObjectSize( pool )       ((size_t)((pool)->objectSize))
#define DKObjectPoolGetAllocatedCount( pool )   ((size_t)DKAtomicLoad64( &((pool)->allocated) ))
DK_API size_t DKObjectPoolGetReservedCount( DKObjectPool * pool );

#ifdef __cplusplus
}
#endif

#endif // _DK_OBJECT_POOL_H_
