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
DKObjectRef DKCopy( DKObjectRef ref )
{
    if( ref )
    {
        DKCopying * copying = DKGetInterface( ref, DKSelector( Copying ) );
        return copying->copy( ref );
    }

    return ref;
}


///
//  DKMutableCopy()
//
DKMutableObjectRef DKMutableCopy( DKObjectRef ref )
{
    if( ref )
    {
        DKCopying * copying = DKGetInterface( ref, DKSelector( Copying ) );
        return copying->mutableCopy( ref );
    }

    return NULL;
}








