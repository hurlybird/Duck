//
//  DKCopying.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-13.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_COPYING_H_
#define _DK_COPYING_H_

#include "DKObject.h"

DKDeclareSUID( DKCopyingInterfaceID );

typedef struct
{
    const DKInterface _interface;

    DKTypeRef   (* const copy)( DKTypeRef ref );
    DKTypeRef   (* const mutableCopy)( DKTypeRef ref );

} DKCopyingInterface;


DKTypeRef DKCopy( DKTypeRef ref );
DKTypeRef DKMutableCopy( DKTypeRef ref );




#endif // _DK_COPYING_H_
