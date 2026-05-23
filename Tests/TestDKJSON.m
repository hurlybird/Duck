// =======================================================================================
//
// TestDKJSON.m
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#import <XCTest/XCTest.h>
#import <Duck/Duck.h>


// JSON Serialization Object
struct MyObject
{
    DKObject _obj;

    DKStringRef string;
    int integer;
};

typedef struct MyObject * MyObjectRef;

static DKObjectRef MyObjectInit( DKObjectRef _untyped_self, DKStringRef string, int integer )
{
    MyObjectRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        _self->string = DKCopy( string );
        _self->integer = integer;
    }
    
    return _self;
}

static DKObjectRef MyObjectInitWithJSONObject( DKObjectRef _untyped_self, DKDictionaryRef jsonObject )
{
    MyObjectRef _self = DKSuperInit( _untyped_self, DKObjectClass() );

    if( _self )
    {
        _self->string = DKCopy( DKDictionaryGetObject( jsonObject, DKSTR( "string" ) ) );
        _self->integer = DKGetInt32( DKDictionaryGetObject( jsonObject, DKSTR( "integer" ) ) );
    }
    
    return _self;
}

static void MyObjectWriteJSONObject( DKObjectRef _untyped_self, DKMutableDictionaryRef jsonObject )
{
    MyObjectRef _self = _untyped_self;
    
    DKDictionarySetObject( jsonObject, DKSTR( "string" ), _self->string );
    DKDictionarySetObject( jsonObject, DKSTR( "integer" ), DKNumberWithInt32( _self->integer ) );
}

static bool MyObjectEqual( DKObjectRef _untyped_self, DKObjectRef _untyped_other )
{
    MyObjectRef _self = _untyped_self;
    MyObjectRef other = _untyped_other;
    
    return (_self->integer == other->integer) && DKStringEqualToString( _self->string, other->string );
}

static void MyObjectFinalize( DKObjectRef _untyped_self )
{
    MyObjectRef _self = _untyped_self;
    
    DKRelease( _self->string );
}

DKThreadSafeStaticClassInit( MyObjectClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "MyObject" ), DKObjectClass(), sizeof(struct MyObject), 0, NULL, MyObjectFinalize );

    // Comparison
    struct DKComparisonInterface * comparison = DKNewInterface( DKSelector(Comparison) );
    comparison->equal = MyObjectEqual;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // JSON Serialization
    struct DKJSONSerializationInterface * jsonSerialization = DKNewInterface( DKSelector(JSONSerialization) );
    jsonSerialization->initWithJSONObject = MyObjectInitWithJSONObject;
    jsonSerialization->writeJSONObject = MyObjectWriteJSONObject;
    
    DKInstallInterface( cls, jsonSerialization );
    DKRelease( jsonSerialization );
    
    return cls;
}


// Exception Wiring
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


- (void) testJSONTrailingComma
{
    // Create some JSON with trailing commas
    DKStringRef json = DKStringWithCString(
        "{\n" \
        "    \"a\" : [ 1, 2, 3 ],\n" \
        "    \"b\" : [ 1, 2, 3, ],\n" \
        "    \"c\" : 3,\n" \
        "}" );
        
    // Create the decoded verson of the json
    DKDictionaryRef decodedDocument =  DKDictionaryWithKeysAndObjects(
        DKSTR( "a" ), DKListWithObjects( DKNumberWithInt64( 1 ), DKNumberWithInt64( 2 ), DKNumberWithInt64( 3 ) ),
        DKSTR( "b" ), DKListWithObjects( DKNumberWithInt64( 1 ), DKNumberWithInt64( 2 ), DKNumberWithInt64( 3 ) ),
        DKSTR( "c" ), DKNumberWithInt64( 3 ) );

    // Parse the JSON
    DKObjectRef parsedDocument = DKJSONParse( json, 0 );

    XCTAssert( DKEqual( decodedDocument, parsedDocument ) );

//    DKPrintf( "Decoded Document:\n%@\n\n", decodedDocument );
//    DKPrintf( "JSON:\n%@\n\n", json );
//    DKPrintf( "Parsed Document:\n%@\n\n", parsedDocument );
}


- (void) testJSONSerialization
{
    MyObjectRef myObject = MyObjectInit( DKAlloc( MyObjectClass() ), DKSTR( "Don't Panic" ), 42 );
    
    // Convert it to JSON
    DKMutableStringRef json = DKMutableString();
    DKJSONWrite( json, myObject, DKJSONObjectSerialization );

    // Parse the JSON
    DKObjectRef parsedObject = DKJSONParse( json, DKJSONObjectSerialization );

    XCTAssert( DKEqual( myObject, parsedObject ) );
}



#if 0
- (void) testNSJSONReadPerformance
{
    NSString * path = [[NSBundle bundleForClass:[self class]] pathForResource:@"largefile" ofType:@"json"];
    NSData * jsonData = [NSData dataWithContentsOfFile:path];

    [self measureBlock:^{
        @autoreleasepool
        {
            [NSJSONSerialization JSONObjectWithData:jsonData options:0 error:nil];
        }
    }];
}

- (void) testJSONReadPerformance
{
    NSString * _path = [[NSBundle bundleForClass:[self class]] pathForResource:@"largefile" ofType:@"json"];
    DKStringRef path = DKStringWithCString( [_path UTF8String] );
    DKStringRef json = DKStringWithContentsOfFile( path );

    [self measureBlock:^{
        DKPushAutoreleasePool();
        DKJSONParse( json, 0 );
        DKPopAutoreleasePool();
    }];
}

- (void) testJSONWritePerformance
{
    NSString * _path = [[NSBundle bundleForClass:[self class]] pathForResource:@"largefile" ofType:@"json"];
    DKStringRef path = DKStringWithCString( [_path UTF8String] );
    DKStringRef json = DKStringWithContentsOfFile( path );

    DKObjectRef object = DKJSONParse( json, 0 );

    [self measureBlock:^{
        DKPushAutoreleasePool();
        DKMutableStringRef buffer = DKMutableString();
        DKJSONWrite( buffer, object, 0 );
        DKPopAutoreleasePool();
    }];
}
#endif

@end
