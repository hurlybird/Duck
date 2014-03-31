//
//  testDKNumber.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-31.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface testDKNumber : XCTestCase

@end

@implementation testDKNumber

- (void)setUp
{
    [super setUp];

    DKSetErrorCallback( RaiseException );
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

typedef struct
{
    float x, y, z;

} Vector3;

- (void) testDKNumber
{
    DKNumberRef n = DKNumberCreateInt32( 100 );
    XCTAssert( DKNumberGetInt32( n ) == 100 );
    
    DKStringRef desc = DKNumberCopyDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100" ) ) );

    float f;
    DKNumberCastValue( n, &f, DKNumberFloat );
    XCTAssert( f == 100.0f );
    
    double d;
    DKNumberCastValue( n, &d, DKNumberDouble );
    XCTAssert( d == 100.0 );
    
    DKRelease( desc );
    DKRelease( n );
    

    n = DKNumberCreateFloat( 100 );
    XCTAssert( DKNumberGetFloat( n ) == 100.0f );
    
    desc = DKNumberCopyDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100.000000" ) ) );
    
    DKRelease( desc );
    DKRelease( n );


    n = DKNumberCreateDouble( 100 );
    XCTAssert( DKNumberGetDouble( n ) == 100.0 );
    
    desc = DKNumberCopyDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "100.000000" ) ) );
    
    DKRelease( desc );
    DKRelease( n );
    
    
    const Vector3 v = { 1, 2, 3 };
    n = DKNumberCreate( &v, DKNumberFloat, 3 );
    
    Vector3 w = DKNumberGetValueAs( n, Vector3 );
    XCTAssert( (v.x == w.x) && (v.y == w.y) && (v.z == w.z) );
    
    desc = DKNumberCopyDescription( n );
    XCTAssert( DKStringEqual( desc, DKSTR( "1.000000 2.000000 3.000000" ) ) );
    
    DKRelease( desc );
    DKRelease( n );
}

@end




