//
//  DKCopying.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-13.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKCopying.h"


DKDefineInterface( Copying );


///
//  DKCopy()
//
DKTypeRef DKCopy( DKTypeRef ref )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        DKCopying * copying = DKQueryInterface( obj, DKSelector( Copying ) );
        
        return copying->copy( obj );
    }

    return ref;
}


///
//  DKMutableCopy()
//
DKTypeRef DKMutableCopy( DKTypeRef ref )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        DKCopying * copying = DKQueryInterface( obj, DKSelector( Copying ) );
        
        return copying->mutableCopy( obj );
    }

    return ref;
}








