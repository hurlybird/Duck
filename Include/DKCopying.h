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

typedef DKObjectRef (*DKCopyMethod)( DKObjectRef );
typedef DKMutableObjectRef (*DKMutableCopyMethod)( DKObjectRef );

struct DKCopying
{
    DKInterface _interface;

    DKCopyMethod copy;
    DKMutableCopyMethod mutableCopy;
};

typedef const struct DKCopying DKCopying;


DKObjectRef DKCopy( DKObjectRef _self );
DKMutableObjectRef DKMutableCopy( DKObjectRef _self );




#endif // _DK_COPYING_H_
