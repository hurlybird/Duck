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

typedef struct
{
    float m[3][3];
    
} Matrix3x3;

typedef struct
{
    float m[3][4];
    
} Matrix3x4;


- (void) testDKNumber
{
    DKNumberRef n = DKNumberWithInt32( 100 );
    XCTAssert( DKNumberGetInt32( n ) == 100 );
    
    DKStringRef desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100" ) ) );

    // Casting
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
    

    // Floats
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


    // Doubles
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


    // Vector types
    const Vector3 v = { 1, 2, 3 };
    n = DKNumber( &v, DKEncode( DKEncodingTypeFloat, 3 ) );
    
    Vector3 w = DKNumberGetValueAs( n, Vector3 );
    XCTAssert( (v.x == w.x) && (v.y == w.y) && (v.z == w.z) );
    
    desc = DKNumberGetDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "1.0 2.0 3.0" ) ) );
    
    
    // Row-by-row casting for matrix encodings
    const Matrix3x3 m1 = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    n = DKNumber( &m1, DKEncodeMatrix( DKEncodingTypeFloat, 3, 3 ) );
    
    Matrix3x4 m2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    DKNumberCastValue( n, &m2, DKEncodeMatrix( DKEncodingTypeFloat, 3, 4 ) );

    XCTAssert( (m2.m[0][0] == 1) && (m2.m[0][1] == 2) && (m2.m[0][2] == 3) && (m2.m[0][3] == 0) );
    XCTAssert( (m2.m[1][0] == 4) && (m2.m[1][1] == 5) && (m2.m[1][2] == 6) && (m2.m[1][3] == 0) );
    XCTAssert( (m2.m[2][0] == 7) && (m2.m[2][1] == 8) && (m2.m[2][2] == 9) && (m2.m[2][3] == 0) );


    const Matrix3x4 m3 = { 1, 2, 3, 0, 4, 5, 6, 0, 7, 8, 9, 0 };
    n = DKNumber( &m3, DKEncodeMatrix( DKEncodingTypeFloat, 3, 4 ) );
    
    Matrix3x3 m4 = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    DKNumberCastValue( n, &m4, DKEncodeMatrix( DKEncodingTypeFloat, 3, 3 ) );

    XCTAssert( (m4.m[0][0] == 1) && (m4.m[0][1] == 2) && (m4.m[0][2] == 3) );
    XCTAssert( (m4.m[1][0] == 4) && (m4.m[1][1] == 5) && (m4.m[1][2] == 6) );
    XCTAssert( (m4.m[2][0] == 7) && (m4.m[2][1] == 8) && (m4.m[2][2] == 9) );
}

@end




