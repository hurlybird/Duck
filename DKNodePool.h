//
//  DKNodePool.h
//  Duck
//
//  Created by Derek Nylen on 12-02-29.
//  Copyright (c) 2012 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_NODE_POOL_H_
#define _DK_NODE_POOL_H_

#include "DKEnv.h"


typedef struct
{
    struct DKNodePoolFreeNode * freeList;
    struct DKNodePoolBlock * blockList;
    DKIndex nodeSize;
    
} DKNodePool;


void DKNodePoolInit( DKNodePool * pool, DKIndex nodeSize, DKIndex nodeCount );
void DKNodePoolClear( DKNodePool * pool );

void * DKNodePoolAlloc( DKNodePool * pool );
void DKNodePoolFree( DKNodePool * pool, void * node );



#endif // _DK_NODE_POOL_H_
