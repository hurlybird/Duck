//
//  TestDKNumber.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-31.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKNumber : XCTestCase

@end

@implementation TestDKNumber

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

typedef struct
{
    float x, y, z;

} Vector3;

- (void) testDKNumber
{
    DKNumberRef n = DKNumberWithInt32( 100 );
    XCTAssert( DKNumberGetInt32( n ) == 100 );
    
    DKStringRef desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100" ) ) );

    int32_t i32;
    DKNumberCastValue( n, &i32, DKNumberInt32 );
    XCTAssert( i32 == 100 );

    int64_t i64;
    DKNumberCastValue( n, &i64, DKNumberInt64 );
    XCTAssert( i64 == 100 );

    float f;
    DKNumberCastValue( n, &f, DKNumberFloat );
    XCTAssert( f == 100.0f );
    
    double d;
    DKNumberCastValue( n, &d, DKNumberDouble );
    XCTAssert( d == 100.0 );
    

    n = DKNumberWithFloat( 100 );
    XCTAssert( DKNumberGetFloat( n ) == 100.0f );
    
    desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100.0" ) ) );


    n = DKNumberWithFloat( 100.5f );
    XCTAssert( DKNumberGetFloat( n ) == 100.5f );
    
    desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100.5" ) ) );


    n = DKNumberWithFloat( 100.0625f );
    XCTAssert( DKNumberGetFloat( n ) == 100.0625f );
    
    desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100.0625" ) ) );


    n = DKNumberWithDouble( 100 );
    XCTAssert( DKNumberGetDouble( n ) == 100.0 );
    
    desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100.0" ) ) );
    
    
    n = DKNumberWithDouble( 100.5 );
    XCTAssert( DKNumberGetDouble( n ) == 100.5 );
    
    desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100.5" ) ) );


    n = DKNumberWithDouble( 100.0625 );
    XCTAssert( DKNumberGetDouble( n ) == 100.0625 );
    
    desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100.0625" ) ) );


    const Vector3 v = { 1, 2, 3 };
    n = DKNumber( &v, DKEncode( DKEncodingTypeFloat, 3 ) );
    
    Vector3 w = DKNumberGetValueAs( n, Vector3 );
    XCTAssert( (v.x == w.x) && (v.y == w.y) && (v.z == w.z) );
    
    desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "1.0 2.0 3.0" ) ) );
}

@end




