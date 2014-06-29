//
//  TestDKProperty.m
//  Duck
//
//  Created by Derek Nylen on 2014-04-07.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}


typedef struct
{
    int a, b;
    
} Pair;

struct TestObject
{
    const DKObject _obj;
    
    DKObjectRef name;
    int32_t x;
    double y;
    Pair z;
};


@interface TestDKProperty : XCTestCase

@end

@implementation TestDKProperty

- (void)setUp
{
    [super setUp];

    DKSetErrorCallback( RaiseException );
    DKSetWarningCallback( RaiseException );
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (DKClassRef) createTestClass
{
    DKClassRef testClass = DKAllocClass( DKSTR( "Test" ), DKObjectClass(), sizeof(struct TestObject), 0 );

    DKInstallObjectProperty( testClass, DKSTR( "name" ), 0, offsetof(struct TestObject, name), DKStringClass(), NULL, NULL, NULL );
    DKInstallNumericalProperty( testClass, DKSTR( "x" ), NULL, 0, offsetof(struct TestObject, x), DKNumberInt32, NULL, NULL );
    DKInstallNumericalProperty( testClass, DKSTR( "y" ), NULL, 0, offsetof(struct TestObject, y), DKNumberDouble, NULL, NULL );
    DKInstallStructProperty( testClass, DKSTR( "z" ), DKSemantic(Pair), 0, offsetof(struct TestObject, z), sizeof(Pair), NULL, NULL );

    return testClass;
}

- (void) testObjectProperty
{
    DKClassRef testClass = [self createTestClass];
    
    struct TestObject * testObject = DKCreate( testClass );
    
    DKSetProperty( testObject, DKSTR( "name" ), DKSTR( "Jane" ) );
    XCTAssert( DKStringEqual( DKSTR( "Jane" ), testObject->name ) );

    DKStringRef name = DKGetProperty( testObject, DKSTR( "name" ) );
    XCTAssert( DKStringEqual( DKSTR( "Jane" ), name ) );
    DKRelease( name );
    
    DKRelease( testObject );
    DKRelease( testClass );
}

- (void) testNumberProperty
{
    DKClassRef testClass = [self createTestClass];

    struct TestObject * testObject = DKCreate( testClass );
    
    // Implicit conversion from number object
    DKNumberRef intNumber = DKNumberCreateInt32( 3 );
    DKSetProperty( testObject, DKSTR( "x" ), intNumber );
    
    XCTAssertThrows( DKSetProperty( testObject, DKSTR( "name" ), intNumber ) );
    XCTAssertThrows( DKSetProperty( testObject, DKSTR( "x" ), DKSTR( "Jane" ) ) );
    
    float v[3] = { 0, 0, 0 };
    XCTAssertThrows( DKSetNumericalProperty( testObject, DKSTR( "x" ), v, DKEncode( DKEncodingTypeFloat, 3 ) ) );
    
    XCTAssert( testObject->x == 3 );
    DKRelease( intNumber );

    // Implicit conversion from number object + cast
    DKNumberRef floatNumber = DKNumberCreateFloat( 7 );
    DKSetProperty( testObject, DKSTR( "x" ), floatNumber );
    XCTAssert( testObject->x == 7 );
    DKRelease( floatNumber );
    
    
    DKRelease( testObject );
    DKRelease( testClass );
}


- (void) testStructProperty
{
    DKClassRef testClass = [self createTestClass];
    struct TestObject * testObject = DKCreate( testClass );
    
    Pair p = { 101, 202 };
    
    DKSetStructProperty( testObject, DKSTR( "z" ), DKSemantic(Pair), &p, sizeof(Pair) );
    XCTAssert( testObject->z.a == 101 );
    XCTAssert( testObject->z.b == 202 );

    XCTAssertThrows( DKSetStructProperty( testObject, DKSTR( "z" ), DKSTR( "Not Pair" ), &p, sizeof(Pair) ) );
    XCTAssertThrows( DKSetStructProperty( testObject, DKSTR( "z" ), DKSemantic(Pair), &p, sizeof(int) ) );
    
    Pair q;
    DKGetStructProperty( testObject, DKSTR( "z" ), DKSemantic(Pair), &q, sizeof(Pair) );
    
    XCTAssert( q.a == 101 );
    XCTAssert( q.b == 202 );
    
    // Convert to object
    DKStructRef structure = DKGetProperty( testObject, DKSTR( "z" ) );

    Pair r;
    DKStructGetValueAs( structure, &r, Pair );

    XCTAssert( r.a == 101 );
    XCTAssert( r.b == 202 );
    
    DKRelease( structure );
    
    // Convert from object
    Pair r2 = { 303, 404 };
    DKStructRef structure2 = DKStructCreateAs( &r2, Pair );
    DKSetProperty( testObject, DKSTR( "z" ), structure2 );
    DKRelease( structure2 );
    
    XCTAssert( testObject->z.a == 303 );
    XCTAssert( testObject->z.b == 404 );
    
    DKRelease( testObject );
    DKRelease( testClass );
}


@end







