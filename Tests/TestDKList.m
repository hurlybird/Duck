//
//  Duck_Tests.m
//  Duck Tests
//
//  Created by Derek Nylen on 2014-03-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKList : XCTestCase

@end

@implementation TestDKList

- (void) setUp
{
    [super setUp];

    DKSetErrorCallback( RaiseException );
}

- (void) tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void) testDKArray
{
    [self testListClass:DKMutableArrayClass()];
}

- (void) testDKLinkedList
{
    [self testListClass:DKMutableLinkedListClass()];
}

- (void) testListClass:(DKClassRef)listClass
{
    DKStringRef a = DKSTR( "a" );
    DKStringRef b = DKSTR( "b" );
    DKStringRef c = DKSTR( "c" );
    DKStringRef d = DKSTR( "d" );
    
    DKMutableListRef list = (DKMutableListRef)DKCreate( listClass );
    
    // Append
    DKListAppendObject( list, a );
    DKListAppendObject( list, b );
    DKListAppendObject( list, c );
    DKListAppendObject( list, d );
    
    XCTAssert( DKListGetCount( list ) == 4 );
    
    XCTAssert( DKListGetFirstIndexOfObject( list, a ) == 0 );
    XCTAssert( DKListGetFirstIndexOfObject( list, b ) == 1 );
    XCTAssert( DKListGetFirstIndexOfObject( list, c ) == 2 );
    XCTAssert( DKListGetFirstIndexOfObject( list, d ) == 3 );

    XCTAssert( DKListGetLastIndexOfObject( list, a ) == 0 );
    XCTAssert( DKListGetLastIndexOfObject( list, b ) == 1 );
    XCTAssert( DKListGetLastIndexOfObject( list, c ) == 2 );
    XCTAssert( DKListGetLastIndexOfObject( list, d ) == 3 );
    
    XCTAssert( DKListGetObjectAtIndex( list, 0 ) == a );
    XCTAssert( DKListGetObjectAtIndex( list, 1 ) == b );
    XCTAssert( DKListGetObjectAtIndex( list, 2 ) == c );
    XCTAssert( DKListGetObjectAtIndex( list, 3 ) == d );
    
    DKListRemoveAllObjects( list );
    
    // Insert
    DKListInsertObjectAtIndex( list, 0, a );
    DKListInsertObjectAtIndex( list, 0, b );
    DKListInsertObjectAtIndex( list, 0, c );
    DKListInsertObjectAtIndex( list, 0, d );

    XCTAssert( DKListGetCount( list ) == 4 );

    XCTAssert( DKListGetObjectAtIndex( list, 0 ) == d );
    XCTAssert( DKListGetObjectAtIndex( list, 1 ) == c );
    XCTAssert( DKListGetObjectAtIndex( list, 2 ) == b );
    XCTAssert( DKListGetObjectAtIndex( list, 3 ) == a );

    DKListRemoveAllObjects( list );

    // Copy
    DKListAppendObject( list, a );
    DKListAppendObject( list, b );
    DKListAppendObject( list, c );
    DKListAppendObject( list, d );
    
    XCTAssert( DKListGetCount( list ) == 4 );
    
    DKListRef copy = DKCopy( list );
    
    XCTAssert( DKListGetObjectAtIndex( list, 0 ) == a );
    XCTAssert( DKListGetObjectAtIndex( list, 1 ) == b );
    XCTAssert( DKListGetObjectAtIndex( list, 2 ) == c );
    XCTAssert( DKListGetObjectAtIndex( list, 3 ) == d );
    
    DKListRemoveAllObjects( list );
    DKRelease( copy );
    
    DKRelease( list );
    
    DKRelease( a );
    DKRelease( b );
    DKRelease( c );
    DKRelease( d );
}


@end
