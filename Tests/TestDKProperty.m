//
//  TestDKProperty.m
//  Duck
//
//  Created by Derek Nylen on 2014-04-07.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}


typedef struct
{
    int a, b;
    
} Pair;

typedef enum
{
    One = 1,
    Two,
    Three

} Enum;

struct TestObject
{
    const DKObject _obj;
    
    DKObjectRef name;
    int32_t x;
    double y;
    Pair z;
    Enum e;
};

DKEnumRef EnumType( void );

DKDefineEnum( EnumType,
    "One", One,
    "Two", Two,
    "Three", Three )


@interface TestDKProperty : XCTestCase

@end

@implementation TestDKProperty

- (void) setUp
{
    [super setUp];

    DKRuntimeInit( 0 );
    DKSetErrorCallback( RaiseException );
    DKSetWarningCallback( RaiseException );
    DKPushAutoreleasePool();
}

- (void) tearDown
{
    DKPopAutoreleasePool();
    
    [super tearDown];
}

- (DKClassRef) createTestClass
{
    DKClassRef testClass = DKNewClass( DKSTR( "Test" ), DKObjectClass(), sizeof(struct TestObject), 0, NULL, NULL );

    DKPredicateRef predicate = DKPredicate( DKPredicateISA, NULL, DKStringClass() );

    DKInstallObjectProperty( testClass, DKSTR( "name" ), DKSemantic(DKString), 0, offsetof(struct TestObject, name), predicate, DK_NO_XETTERS, DK_NO_OBSERVERS );
    DKInstallNumberProperty( testClass, DKSTR( "x" ), DKSemantic(int32_t), 0, offsetof(struct TestObject, x), DKNumberInt32, DK_NO_XETTERS, DK_NO_OBSERVERS );
    DKInstallNumberProperty( testClass, DKSTR( "y" ), DKSemantic(double), 0, offsetof(struct TestObject, y), DKNumberDouble, DK_NO_XETTERS, DK_NO_OBSERVERS );
    DKInstallStructProperty( testClass, DKSTR( "z" ), DKSemantic(Pair), 0, offsetof(struct TestObject, z), sizeof(Pair), DK_NO_XETTERS, DK_NO_OBSERVERS );
    DKInstallEnumProperty( testClass, DKSTR( "e" ), DKSemantic(Enum), 0, offsetof(struct TestObject, e), DKEncodeIntegerType(Enum), EnumType(), DK_NO_XETTERS, DK_NO_OBSERVERS );

    return testClass;
}

- (void) testObjectProperty
{
    DKClassRef testClass = [self createTestClass];
    
    struct TestObject * testObject = DKNew( testClass );
    
    DKSetProperty( testObject, DKSTR( "name" ), DKSTR( "Jane" ) );
    XCTAssert( DKStringEqual( DKSTR( "Jane" ), testObject->name ) );

    DKStringRef name = DKGetProperty( testObject, DKSTR( "name" ) );
    XCTAssert( DKStringEqual( DKSTR( "Jane" ), name ) );
    
    DKRelease( testObject );
    DKRelease( testClass );
}

- (void) testNumberProperty
{
    DKClassRef testClass = [self createTestClass];

    struct TestObject * testObject = DKNew( testClass );
    
    // Implicit conversion from number object
    DKNumberRef intNumber = DKNumberWithInt32( 3 );
    DKSetProperty( testObject, DKSTR( "x" ), intNumber );
    
    XCTAssertThrows( DKSetProperty( testObject, DKSTR( "name" ), intNumber ) );
    XCTAssertThrows( DKSetProperty( testObject, DKSTR( "x" ), DKSTR( "Jane" ) ) );
    
    // Partial copies
    float v[3] = { 5, 0, 0 };
    DKSetNumberProperty( testObject, DKSTR( "x" ), v, DKEncode( DKEncodingTypeFloat, 3 ) );
    XCTAssert( testObject->x == 5 );

    float w[3] = { 0, 0, 0 };
    DKGetNumberProperty( testObject, DKSTR( "x" ), w, DKEncode( DKEncodingTypeFloat, 3 ) );
    XCTAssert( w[0] == 5 );

    // Implicit conversion from number object + cast
    DKNumberRef floatNumber = DKNumberWithFloat( 7 );
    DKSetProperty( testObject, DKSTR( "x" ), floatNumber );
    XCTAssert( testObject->x == 7 );
    
    
    DKRelease( testObject );
    DKRelease( testClass );
}

- (void) testStructProperty
{
    DKClassRef testClass = [self createTestClass];
    struct TestObject * testObject = DKNew( testClass );
    
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
    DKStructGetValueAsType( structure, &r, Pair );

    XCTAssert( r.a == 101 );
    XCTAssert( r.b == 202 );
    
    // Convert from object
    Pair r2 = { 303, 404 };
    DKStructRef structure2 = DKStructWithType( &r2, Pair );
    DKSetProperty( testObject, DKSTR( "z" ), structure2 );
    
    XCTAssert( testObject->z.a == 303 );
    XCTAssert( testObject->z.b == 404 );
    
    DKRelease( testObject );
    DKRelease( testClass );
}

- (void) testEnumProperty
{
    DKClassRef testClass = [self createTestClass];

    struct TestObject * testObject = DKNew( testClass );
    
    // Get/SetEnum
    DKSetEnumProperty( testObject, DKSTR( "e" ), One );
    XCTAssert( testObject->e == One );
    XCTAssert( DKStringEqualToString( DKGetProperty( testObject, DKSTR( "e" ) ), DKSTR( "One" ) ) );
    
    // Get/Set enum from string
    DKSetProperty( testObject, DKSTR( "e" ), DKSTR( "Two" ) );
    XCTAssert( testObject->e == Two );
    XCTAssert( DKStringEqualToString( DKGetProperty( testObject, DKSTR( "e" ) ), DKSTR( "Two" ) ) );

    DKSetProperty( testObject, DKSTR( "e" ), DKSTR( "Three" ) );
    XCTAssert( testObject->e == Three );
    XCTAssert( DKStringEqualToString( DKGetProperty( testObject, DKSTR( "e" ) ), DKSTR( "Three" ) ) );

    DKSetProperty( testObject, DKSTR( "e" ), DKSTR( "Four" ) );
    XCTAssert( testObject->e == 0 );
    XCTAssert( DKGetProperty( testObject, DKSTR( "e" ) ) == NULL );
    
    DKRelease( testObject );
    DKRelease( testClass );
}

- (void) testPropertyKeyPaths
{
    DKMutableDictionaryRef dick = DKDictionaryWithKeysAndObjects(
        DKSTR( "name" ), DKSTR( "Dick" ),
        DKSTR( "phoneNumber" ), DKSTR( "555-1234" ),
        NULL );
    
    DKMutableDictionaryRef jane = DKDictionaryWithKeysAndObjects(
        DKSTR( "name" ), DKSTR( "Jane" ),
        DKSTR( "phoneNumber" ), DKSTR( "555-4321" ),
        NULL );
    
    DKMutableDictionaryRef db = DKDictionaryWithKeysAndObjects(
        DKSTR( "dick" ), dick,
        DKSTR( "jane" ), jane,
        NULL );
    
    DKObjectRef value = DKGetPropertyForKeyPath( db, DKSTR( "dick.name" ) );
    XCTAssert( DKEqual( value, DKSTR( "Dick" ) ) );

    value = DKGetPropertyForKeyPath( db, DKSTR( "jane.phoneNumber" ) );
    XCTAssert( DKEqual( value, DKSTR( "555-4321" ) ) );

    value = DKGetPropertyForKeyPath( db, DKSTR( "dick" ) );
    XCTAssert( value == dick );
}


@end







