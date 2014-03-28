//
//  TestDKObject.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>



DKDeclareMessage( Square, int, int * );
DKDefineMessage( Square, int, int * );

DKDeclareMessage( Cube, int, int * );
DKDefineMessage( Cube, int, int * );

static void TestOne( DKTypeRef ref, DKSEL sel, int x, int * y )
{
    *y = 1;
}

static void TestSquare( DKTypeRef ref, DKSEL sel, int x, int * y )
{
    *y = x * x;
}

static void TestCube( DKTypeRef ref, DKSEL sel, int x, int * y )
{
    *y = x * x * x;
}



@interface TestDKRuntime : XCTestCase

@end

@implementation TestDKRuntime

- (void)setUp
{
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void) testRuntime
{
    // Define a sample class
    DKTypeRef TestClassA = DKCreateClass( "A", DKObjectClass(), sizeof(struct DKObjectHeader) );
    XCTAssert( TestClassA );
    
    DKTypeRef TestClassB = DKCreateClass( "B", TestClassA, sizeof(struct DKObjectHeader) );
    XCTAssert( TestClassB );
    
    // Install some message handlers
    DKInstallMsgHandler( TestClassA, DKSelector(Square), TestOne );
    XCTAssert( DKGetMsgHandler( TestClassA, DKSelector(Square) ) );
    
    DKInstallMsgHandler( TestClassA, DKSelector(Cube), TestOne );
    XCTAssert( DKGetMsgHandler( TestClassA, DKSelector(Cube) ) );

    DKInstallMsgHandler( TestClassB, DKSelector(Square), TestSquare );
    XCTAssert( DKGetMsgHandler( TestClassB, DKSelector(Square) ) );
    
    DKInstallMsgHandler( TestClassB, DKSelector(Cube), TestCube );
    XCTAssert( DKGetMsgHandler( TestClassB, DKSelector(Cube) ) );
    
    // Create an instance of the object
    DKTypeRef object = DKCreate( TestClassB );
    XCTAssert( object );
    
    // Test class membership
    XCTAssert( DKGetClass( TestClassB ) == DKClassClass() );
    XCTAssert( DKGetClass( object ) == TestClassB );

    XCTAssert( DKIsKindOfClass( object, TestClassB ) );
    XCTAssert( DKIsKindOfClass( object, DKObjectClass() ) );

    XCTAssert( DKIsMemberOfClass( object, TestClassB ) );
    XCTAssert( !DKIsMemberOfClass( object, DKObjectClass() ) );
    
    // DKQueryInterface should return the same object when called on the class or an instance of the class
    XCTAssert( DKGetInterface( TestClassB, DKSelector(LifeCycle) ) == DKGetInterface( object, DKSelector(LifeCycle) ) );

    // Try calling our custom message handlers
    int y;
    
    DKMsgSend( object, Square, 2, &y );
    XCTAssert( y == 4 );

    DKMsgSend( object, Cube, 2, &y );
    XCTAssert( y == 8 );

    DKMsgSendSuper( object, Square, 2, &y );
    XCTAssert( y == 1 );

    DKMsgSendSuper( object, Cube, 2, &y );
    XCTAssert( y == 1 );

    // Cleanup
    DKRelease( object );
    DKRelease( TestClassA );
    DKRelease( TestClassB );
}

@end
