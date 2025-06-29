/*****************************************************************************************

  DKObjectPool.h

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
    DKObjectPoolFreeNode * volatile freeList;
    DKObjectPoolBlock * volatile blockList;
    size_t blockSize;
    size_t reserved;
    size_t volatile allocated;
    DKSpinLock mutex;
    
} DKObjectPool;


DK_API void DKObjectPoolInit( DKObjectPool * pool, size_t size, size_t reserve );
DK_API void DKObjectPoolFinalize( DKObjectPool * pool );

DK_API void * DKObjectPoolAlloc( DKObjectPool * pool );
DK_API void * DKObjectPoolThreadSafeAlloc( DKObjectPool * pool );

DK_API void DKObjectPoolFree( DKObjectPool * pool, void * node );
DK_API void DKObjectPoolThreadSafeFree( DKObjectPool * pool, void * node );

#define DKObjectPoolGetBlockSize( pool )        ((pool)->blockSize)
#define DKObjectPoolGetAllocatedCount( pool )   ((pool)->allocated)
#define DKObjectPoolGetReservedCount( pool )    ((pool)->reserved)


#ifdef __cplusplus
}
#endif

#endif // _DK_OBJECT_POOL_H_
