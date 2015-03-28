//
//  main.c
//  TestBed
//
//  Created by Derek Nylen on 2015-03-25.
//  Copyright (c) 2015 Derek W. Nylen. All rights reserved.
//
#include <stdio.h>
#include <string.h>

#include "Duck.h"


int main( int argc, const char * argv[] )
{
    DKRuntimeInit();
    DKPushAutoreleasePool();

    DKStringRef desc = DKGetDescription( DKTrue() );
    DKPrintf( "%@\n\n", desc );

    DKMutableDictionaryRef document = DKDictionaryWithKeysAndObjects(
        DKSTR( "Dick" ), DKSTR( "boy" ),
        DKSTR( "Jane" ), DKSTR( "girl" ),
        DKSTR( "Spot" ), DKSTR( "dog" ),
        DKSTR( "List" ), DKListWithObjects(
            DKNumberWithInt32( 1 ),
            DKNumberWithInt32( 2 ),
            DKNumberWithDouble( 3.5 ),
            NULL ),
        DKSTR( "Yup" ), DKTrue(),
        DKSTR( "Nope" ), DKFalse(),
        DKSTR( "Null" ), NULL,
        NULL );

    
    DKMutableStringRef json = DKAutorelease( DKStringCreateMutable() );
    DKJSONWrite( json, document, DK_JSON_PRETTY );
    
    DKPrintf( "%@\n", json );
    

    DKPopAutoreleasePool();
    
    return 0;
}
