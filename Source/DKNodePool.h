/*****************************************************************************************

  DKNodePool.h

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

#ifndef _DK_NODE_POOL_H_
#define _DK_NODE_POOL_H_

#include "DKPlatform.h"


typedef struct _DKNodePoolFreeNode
{
    struct _DKNodePoolFreeNode * next;
    
} DKNodePoolFreeNode;

typedef struct _DKNodePoolBlock
{
    struct _DKNodePoolBlock * next;
    DKIndex nodeCount;

} DKNodePoolBlock;

typedef struct
{
    DKNodePoolFreeNode * freeList;
    DKNodePoolBlock * blockList;
    DKIndex nodeSize;
    DKIndex nodeCount;
    
} DKNodePool;


DK_API void DKNodePoolInit( DKNodePool * pool, DKIndex nodeSize, DKIndex nodeCount );
DK_API void DKNodePoolFinalize( DKNodePool * pool );

DK_API void * DKNodePoolAlloc( DKNodePool * pool );
DK_API void DKNodePoolFree( DKNodePool * pool, void * node );

DK_API void * DKNodePoolGetBlockSegment( const DKNodePoolBlock * block );


#endif // _DK_NODE_POOL_H_
