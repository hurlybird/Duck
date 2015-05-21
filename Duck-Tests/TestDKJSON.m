//
//  TestDKJSON.m
//  Duck
//
//  Created by Derek Nylen on 2015-05-21.
//  Copyright (c) 2015 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKJSON : XCTestCase

@end

@implementation TestDKJSON

- (void) setUp
{
    [super setUp];

    DKRuntimeInit();
    DKSetErrorCallback( RaiseException );
    DKPushAutoreleasePool();
}

- (void) tearDown
{
    DKPopAutoreleasePool();
    
    [super tearDown];
}

- (void) testJSON
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

    // Convert it to JSON
    DKMutableStringRef json = DKAutorelease( DKStringCreateMutable() );
    DKJSONWrite( json, document, DK_JSON_PRETTY );
    
    // Parse the JSON
    DKObjectRef parsedDocument = DKJSONParse( json, 0 );

    XCTAssert( DKEqual( document, parsedDocument ) );

    //DKPrintf( "Orignal Document:\n%@\n\n", document );
    //DKPrintf( "JSON:\n%@\n\n", json );
    //DKPrintf( "Parsed Document:\n%@\n\n", parsedDocument );
}


@end
