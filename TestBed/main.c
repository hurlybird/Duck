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


static void TestSerialization( void )
{
    // Create a document
    DKMutableDictionaryRef document = DKDictionaryWithKeysAndObjects(
        DKSTR( "Dick" ), DKSTR( "\"boy\"" ),
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
    
    
    // Convert it to an egg
    DKEggArchiverRef archiver = DKCreate( DKEggArchiverClass() );
    DKEggAddObject( archiver, DKSTR( "*" ), document );
    
    DKDataRef egg = DKAutorelease( DKEggArchiverCreateData( archiver ) );
    
    DKRelease( archiver );

    
    // Parse the JSON
    DKObjectRef parsedDocument = DKJSONParse( json, 0 );
    DKPrintf( "Parsed Document:\n%@\n\n", parsedDocument );


    // Deserialize the egg
    DKEggUnarchiverRef unarchiver = DKEggCreateUnarchiverWithData( egg );
    DKObjectRef unarchivedDocument = DKEggGetObject( unarchiver, DKSTR( "*" ) );
    DKPrintf( "Unarchived Document:\n%@\n\n", unarchivedDocument );

    DKRelease( unarchiver );


    // Print out some statistics
    DKPrintf( "JSON Size: %d\nEGG Size:  %d\n\n", DKStringGetByteLength( json ), DKDataGetLength( egg ) );
}


static void TestDictionaryPerformance()
{
    const int PERFORMANCE_N = 100;
    const int PERFORMANCE_I = 1000000;

    DKMutableDictionaryRef dict = DKCreate( DKMutableHashTableClass() );
    DKMutableListRef keys = DKCreate( DKMutableArrayClass() );

    for( int i = 0; i < PERFORMANCE_N; i++ )
    {
        DKStringRef s = DKStringCreateWithFormat( DKStringClass(), "%d", i );
        DKListAppendObject( keys, s );
        DKRelease( s );
    }

    srand( 0 );

    while( 1 )
    {
        for( int i = 0; i < PERFORMANCE_N; i++ )
        {
            DKStringRef key = DKListGetObjectAtIndex( keys, i );
            DKDictionarySetObject( dict, key, DKSTR( "Object" ) );
        }

        for( int i = 0; i < PERFORMANCE_I; i++ )
        {
            int index1 = rand() % PERFORMANCE_N;
            int index2 = rand() % PERFORMANCE_N;
        
            DKStringRef key1 = DKListGetObjectAtIndex( keys, index1 );
            DKStringRef key2 = DKListGetObjectAtIndex( keys, index2 );

            DKStringRef value1 = DKRetain( DKDictionaryGetObject( dict, key1 ) );
            DKStringRef value2 = DKRetain( DKDictionaryGetObject( dict, key2 ) );

            DKDictionarySetObject( dict, key2, value1 );
            DKDictionarySetObject( dict, key1, value2 );

            DKRelease( value1 );
            DKRelease( value2 );
        }
    }

    DKRelease( dict );
    DKRelease( keys );
}


int main( int argc, const char * argv[] )
{
    DKRuntimeInit();
    DKPushAutoreleasePool();


    //TestSerialization();
    TestDictionaryPerformance();
    

    DKPopAutoreleasePool();
    
    return 0;
}

