//
//  DKCopying.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-13.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKCopying.h"


DKThreadSafeSelectorInit( Copying );


///
//  DKCopy()
//
DKObjectRef DKCopy( DKObjectRef _self )
{
    if( _self )
    {
        DKCopying * copying = DKGetInterface( _self, DKSelector( Copying ) );
        return copying->copy( _self );
    }

    return _self;
}


///
//  DKMutableCopy()
//
DKMutableObjectRef DKMutableCopy( DKObjectRef _self )
{
    if( _self )
    {
        DKCopying * copying = DKGetInterface( _self, DKSelector( Copying ) );
        return copying->mutableCopy( _self );
    }

    return NULL;
}








