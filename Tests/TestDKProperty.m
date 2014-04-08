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


struct TestObject
{
    const DKObject _obj;
    
    DKObjectRef name;
    int32_t x;
    float y;
};


@interface TestDKProperty : XCTestCase

@end

@implementation TestDKProperty

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

- (DKClassRef) createTestClass
{
    DKClassRef testClass = DKAllocClass( DKSTR( "Test" ), DKObjectClass(), sizeof(struct TestObject), 0 );

    DKInstallObjectProperty( testClass, DKSTR( "name" ), 0, offsetof(struct TestObject, name), DKStringClass(), NULL, NULL, NULL );
    DKInstallNumericalProperty( testClass, DKSTR( "x" ), 0, offsetof(struct TestObject, x), DKNumberInt32, NULL, NULL );
    DKInstallNumericalProperty( testClass, DKSTR( "y" ), 0, offsetof(struct TestObject, y), DKNumberFloat, NULL, NULL );

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
    
    DKSetProperty( testObject, DKSTR( "name" ), intNumber );
    DKSetProperty( testObject, DKSTR( "x" ), DKSTR( "Jane" ) );
    
    float v[3] = { 0, 0, 0 };
    DKSetNumericalProperty( testObject, DKSTR( "x" ), v, DKNumberMakeVectorType( DKNumberComponentFloat, 3 ) );
    
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




@end







