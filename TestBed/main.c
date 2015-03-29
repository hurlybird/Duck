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


    // Create a document
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

    DKPrintf( "Orignal Document:\n%@\n\n", document );


    // Convert it to JSON
    DKMutableStringRef json = DKAutorelease( DKStringCreateMutable() );
    DKJSONWrite( json, document, DK_JSON_PRETTY );
    
    DKPrintf( "JSON:\n%@\n\n", json );

    
    // Parse the JSON
    DKObjectRef parsedDocument = DKJSONParse( json, 0 );
    
    DKPrintf( "Parsed Document:\n%@\n\n", parsedDocument );
    

    DKPopAutoreleasePool();
    
    return 0;
}
