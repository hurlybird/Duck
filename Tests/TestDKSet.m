//
//  TestDKSet.m
//  Duck
//
//  Created by Derek Nylen on 2014-04-06.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKSet : XCTestCase

@end

@implementation TestDKSet

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

- (void) testDKHashTable
{
    [self testDKSetClass:DKMutableHashTableClass()];
}

- (void) testDKBinaryTree
{
    [self testDKSetClass:DKMutableBinaryTreeClass()];
}

- (void) testDKArray
{
    [self testDKSetClass:DKMutableArrayClass()];
}

- (void) testDKLinkedList
{
    [self testDKSetClass:DKMutableLinkedListClass()];
}

- (void) testDKSetClass:(DKClassRef)setClass
{
    DKMutableSetRef set = DKNew( setClass );
    
    DKStringRef a = DKStringWithCString( "A" );
    DKStringRef b = DKStringWithCString( "B" );
    DKStringRef c = DKStringWithCString( "A" );
    DKStringRef d = DKStringWithCString( "B" );
    
    DKSetAddObject( set, a );
    DKSetAddObject( set, b );
    DKSetAddObject( set, c );
    DKSetAddObject( set, d );
    
    XCTAssert( DKSetGetCount( set ) == 2 );
    
    XCTAssert( DKSetContainsObject( set, a ) );
    XCTAssert( DKSetContainsObject( set, b ) );
    XCTAssert( DKSetContainsObject( set, c ) );
    XCTAssert( DKSetContainsObject( set, d ) );

    XCTAssert( DKSetGetMember( set, a ) == a );
    XCTAssert( DKSetGetMember( set, b ) == b );
    XCTAssert( DKSetGetMember( set, c ) == a );
    XCTAssert( DKSetGetMember( set, d ) == b );
    
    XCTAssert( DKSetContainsObject( set, DKSTR( "A" ) ) );
    XCTAssert( DKSetContainsObject( set, DKSTR( "B" ) ) );
    XCTAssert( DKSetContainsObject( set, DKSTR( "C" ) ) == 0 );
    XCTAssert( DKSetContainsObject( set, DKSTR( "D" ) ) == 0 );
    
    DKRelease( set );
}


@end





