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

    DKRuntimeInit( 0 );
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
    DKDictionaryRef document = DKDictionaryWithKeysAndObjects(
        DKSTR( "Dick" ), DKSTR( "\"boy\"" ),
        DKSTR( "Jane" ), DKSTR( "girl" ),
        DKSTR( "Spot" ), DKSTR( "dog" ),
        DKSTR( "List" ), DKListWithObjects(
            DKNumberWithInt32( 1 ),
            DKNumberWithInt32( 2 ),
            DKNumberWithDouble( 3.5 ),
            NULL ),
        DKSTR( "Date" ), DKNumberWithDate( NULL ),
        DKSTR( "Yup" ), DKTrue(),
        DKSTR( "Nope" ), DKFalse(),
        DKSTR( "Null" ), NULL,
        NULL );

    // Convert it to JSON
    DKMutableStringRef json = DKMutableString();
    DKJSONWrite( json, document, 0 );

    // Parse the JSON
    DKObjectRef parsedDocument = DKJSONParse( json, 0 );

    XCTAssert( DKEqual( document, parsedDocument ) );

    //DKPrintf( "Original Document:\n%@\n\n", document );
    //DKPrintf( "JSON:\n%@\n\n", json );
    //DKPrintf( "Parsed Document:\n%@\n\n", parsedDocument );
}

- (void) testJSONExtendedSyntax
{
    // Create a document
    int64_t v[3] = { 1, 2, 3 };
    double w[3] = { 1.0, 2.0, 3.0 };
    
    DKDictionaryRef document = DKDictionaryWithKeysAndObjects(
        DKSTR( "Dick" ), DKSTR( "\"boy\"" ),
        DKSTR( "Jane" ), DKSTR( "girl" ),
        DKSTR( "Spot" ), DKSTR( "dog" ),
        DKSTR( "List" ), DKListWithObjects(
            DKNumberWithInt32( 1 ),
            DKNumberWithInt32( 2 ),
            DKNumberWithDouble( 3.5 ),
            DKNumber( v, DKEncode( DKEncodingTypeInt64, 3 ) ),
            DKNumber( w, DKEncode( DKEncodingTypeDouble, 3 ) ),
            NULL ),
        DKSTR( "Date" ), DKNumberWithDate( NULL ),
        DKSTR( "Yup" ), DKTrue(),
        DKSTR( "Nope" ), DKFalse(),
        DKSTR( "Null" ), NULL,
        NULL );

    // Convert it to JSON
    DKMutableStringRef json = DKMutableString();
    DKJSONWrite( json, document, DKJSONWritePretty | DKJSONVectorSyntaxExtension );
    
    // Parse the JSON
    DKObjectRef parsedDocument = DKJSONParse( json, DKJSONVectorSyntaxExtension );

    XCTAssert( DKEqual( document, parsedDocument ) );

    // Parse the JSON with 32-bit vector types
    DKObjectRef parsedDocument32 = DKJSONParse( json, DKJSONVectorSyntaxExtension | DKJSONVectorRead32BitTypes );

    XCTAssert( DKEqual( document, parsedDocument32 ) );

    //DKPrintf( "Original Document:\n%@\n\n", document );
    //DKPrintf( "JSON:\n%@\n\n", json );
    //DKPrintf( "Parsed Document:\n%@\n\n", parsedDocument );
}

- (void) testJSONUnicode
{
    // Create some JSON with utf code points
    DKStringRef json = DKStringWithCString(
        "{\n" \
        "    \"ASCII\" : \"\\u003F\",\n" \
        "    \"Latin-1\" : \"\\u00A3\",\n" \
        "    \"Latin Extended-A\" : \"\\u0152\",\n" \
        "    \"Greek and Coptic\" : \"\\u0398\",\n" \
        "    \"Unicode Symbols\" : \"\\u2014\",\n" \
        "    \"Mathematical Symbols\" : \"\\u2211\",\n" \
        "    \"Snowman\" : \"\\u26C4\"\n" \
        "}" );
        
    // Create the decoded verson of the json
    DKDictionaryRef decodedDocument =  DKDictionaryWithKeysAndObjects(
        DKSTR( "ASCII" ), DKSTR( "?" ),
        DKSTR( "Latin-1" ), DKSTR( "£" ),
        DKSTR( "Latin Extended-A" ), DKSTR( "Œ" ),
        DKSTR( "Greek and Coptic" ), DKSTR( "Θ" ),
        DKSTR( "Unicode Symbols" ), DKSTR( "—" ),
        DKSTR( "Mathematical Symbols" ), DKSTR( "∑" ),
        DKSTR( "Snowman" ), DKSTR( "⛄" ) );

    // Parse the JSON
    DKObjectRef parsedDocument = DKJSONParse( json, 0 );

    XCTAssert( DKEqual( decodedDocument, parsedDocument ) );

//    DKPrintf( "Decoded Document:\n%@\n\n", decodedDocument );
//    DKPrintf( "JSON:\n%@\n\n", json );
//    DKPrintf( "Parsed Document:\n%@\n\n", parsedDocument );
}


//- (void) testJSONReadPerformance
//{
//    NSString * _path = [[NSBundle bundleForClass:[self class]] pathForResource:@"largefile" ofType:@"json"];
//    DKStringRef path = DKStringWithCString( [_path UTF8String] );
//    DKStringRef json = DKStringWithContentsOfFile( path );
//
//    [self measureBlock:^{
//        DKPushAutoreleasePool();
//        DKJSONParse( json, 0 );
//        DKPopAutoreleasePool();
//    }];
//}


@end
