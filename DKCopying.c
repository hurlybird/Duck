//
//  DKCopying.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-13.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKCopying.h"


DKDefineInterface( DKCopying );


///
//  DKCopy()
//
DKTypeRef DKCopy( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKCopyingInterface * copyingInterface = DKGetInterface( obj, DKSelector( DKCopying ) );
        return copyingInterface->copy( obj );
    }

    return ref;
}


///
//  DKMutableCopy()
//
DKTypeRef DKMutableCopy( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKCopyingInterface * copyingInterface = DKGetInterface( obj, DKSelector( DKCopying ) );
        return copyingInterface->mutableCopy( obj );
    }

    return ref;
}








