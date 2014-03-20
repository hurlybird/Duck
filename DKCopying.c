//
//  DKCopying.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-13.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKCopying.h"


DKDefineSUID( DKCopyingInterfaceID, "A29F6AD3-8BC5-40E1-A5AC-40B01B54256D" );


///
//  DKCopy()
//
DKTypeRef DKCopy( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKCopyingInterface * copyingInterface = DKGetInterface( obj, &DKCopyingInterfaceID );
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
        const DKCopyingInterface * copyingInterface = DKGetInterface( obj, &DKCopyingInterfaceID );
        return copyingInterface->mutableCopy( obj );
    }

    return ref;
}







