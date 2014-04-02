//
//  DKCopying.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-13.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_COPYING_H_
#define _DK_COPYING_H_

#include "DKRuntime.h"

DKDeclareInterfaceSelector( Copying );

struct DKCopying
{
    DKInterface _interface;

    DKTypeRef   (*copy)( DKTypeRef ref );
    DKTypeRef   (*mutableCopy)( DKTypeRef ref );
};

typedef const struct DKCopying DKCopying;


DKTypeRef DKCopy( DKTypeRef ref );
DKTypeRef DKMutableCopy( DKTypeRef ref );




#endif // _DK_COPYING_H_
