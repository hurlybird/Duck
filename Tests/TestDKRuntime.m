//
//  TestDKObject.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#include <pthread.h>


DKDeclareMessageSelector( Square, int );
DKThreadSafeSelectorInit( Square );

DKDeclareMessageSelector( Cube, int );
DKThreadSafeSelectorInit( Cube );

static intptr_t TestOne( DKObjectRef _self, DKSEL sel, int x )
{
    return 1;
}

static intptr_t TestSquare( DKObjectRef _self, DKSEL sel, int x )
{
    return x * x;
}

static intptr_t TestCube( DKObjectRef _self, DKSEL sel, int x )
{
    return x * x * x;
}



static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKRuntime : XCTestCase

@end

@implementation TestDKRuntime

- (void) setUp
{
    [super setUp];

    DKRuntimeInit();
    DKSetErrorCallback( RaiseException );
    DKPushAutoreleasePool();
}

- (void) tearDown
{
    DKPopAutoreleasePool();
    
    [super tearDown];
}

- (void) testRuntime
{
    // Define a sample class
    DKClassRef TestClassA = DKAllocClass( DKSTR( "A" ), DKObjectClass(), sizeof(DKObject), 0, NULL, NULL );
    XCTAssert( TestClassA );
    
    DKClassRef TestClassB = DKAllocClass( DKSTR( "B" ), TestClassA, sizeof(DKObject), 0, NULL, NULL );
    XCTAssert( TestClassB );
    
    // Install some message handlers
    DKInstallMsgHandler( TestClassA, DKSelector(Square), (DKMsgFunction)TestOne );
    DKInstallMsgHandler( TestClassA, DKSelector(Cube), (DKMsgFunction)TestOne );
    DKInstallMsgHandler( TestClassB, DKSelector(Square), (DKMsgFunction)TestSquare );
    DKInstallMsgHandler( TestClassB, DKSelector(Cube), (DKMsgFunction)TestCube );
    
    // Create some instances
    DKObjectRef a = DKCreate( TestClassA );
    XCTAssert( a );

    DKObjectRef b = DKCreate( TestClassB );
    XCTAssert( b );
    
    // Test class membership
    XCTAssert( DKGetClass( TestClassB ) == DKClassClass() );
    XCTAssert( DKGetClass( b ) == TestClassB );

    XCTAssert( DKIsKindOfClass( b, DKObjectClass() ) );
    XCTAssert( DKIsKindOfClass( b, TestClassA ) );
    XCTAssert( DKIsKindOfClass( b, TestClassB ) );

    XCTAssert( !DKIsMemberOfClass( b, DKObjectClass() ) );
    XCTAssert( !DKIsMemberOfClass( b, TestClassA ) );
    XCTAssert( DKIsMemberOfClass( b, TestClassB ) );
    
    // Try calling our custom message handlers
    intptr_t y = DKMsgSend( a, Square, 2 );
    XCTAssert( y == 1 );

    y = DKMsgSend( a, Cube, 2 );
    XCTAssert( y == 1 );

    y = DKMsgSend( b, Square, 2 );
    XCTAssert( y == 4 );

    y = DKMsgSend( b, Cube, 2 );
    XCTAssert( y == 8 );

    // Cleanup
    DKRelease( a );
    DKRelease( b );
    DKRelease( TestClassA );
    DKRelease( TestClassB );
}


static void LinearReleaseThread( DKObjectRef param )
{
    DKMutableListRef list = (DKMutableListRef)param;

    DKIndex count = DKListGetCount( list );
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKListRemoveObjectAtIndex( list, 0 );
    }
}

static void RandomReleaseThread( DKObjectRef param )
{
    DKMutableListRef list = (DKMutableListRef)param;

    DKIndex count = DKListGetCount( list );
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKIndex x = (DKIndex)rand() % DKListGetCount( list );
        DKListRemoveObjectAtIndex( list, x );
    }
}

static void ResolveWeakThread( DKObjectRef param )
{
    DKMutableListRef list = (DKMutableListRef)param;

    DKIndex count;

    while( (count = DKListGetCount( list )) != 0 )
    {
        for( DKIndex i = count - 1; i >= 0; --i )
        {
            DKWeakRef weakref = DKListGetObjectAtIndex( list, i );
            DKObjectRef strongref = DKResolveWeak( weakref );

            if( strongref == NULL )
                DKListRemoveObjectAtIndex( list, i );

            else
                DKRelease( strongref );
        }
    }
}

- (void) testDKReferenceCountingStressTest
{
    const int N = 10000;

    DKMutableListRef array1 = DKCreate( DKMutableArrayClass() );
    DKMutableListRef array2 = DKCreate( DKMutableArrayClass() );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef s = DKStringCreateWithFormat( DKStringClass(), "%d", i );
        
        DKListAppendObject( array1, s );
        DKListAppendObject( array2, s );
        DKRelease( s );
    }

    DKThreadRef thread1 = DKThreadInit( DKAlloc( DKThreadClass(), 0 ), LinearReleaseThread, array1 );
    DKThreadStart( thread1 );

    DKThreadRef thread2 = DKThreadInit( DKAlloc( DKThreadClass(), 0 ), RandomReleaseThread, array2 );
    DKThreadStart( thread2 );
    
    DKThreadJoin( thread1 );
    DKThreadJoin( thread2 );
    
    DKRelease( thread1 );
    DKRelease( thread2 );

    DKRelease( array1 );
    DKRelease( array2 );
}


- (void) testDKWeakReferenceStressTest
{
    const int N = 10000;

    DKMutableListRef array1 = DKCreate( DKMutableArrayClass() );
    DKMutableListRef array2 = DKCreate( DKMutableArrayClass() );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef s = DKStringCreateWithFormat( DKStringClass(), "%d", i );
        DKWeakRef w = DKRetainWeak( s );
        
        DKListAppendObject( array1, s );
        DKListAppendObject( array2, w );
        
        DKRelease( s );
        DKRelease( w );
    }

    DKThreadRef thread1 = DKThreadInit( DKAlloc( DKThreadClass(), 0 ), RandomReleaseThread, array1 );
    DKThreadStart( thread1 );

    DKThreadRef thread2 = DKThreadInit( DKAlloc( DKThreadClass(), 0 ), ResolveWeakThread, array2 );
    DKThreadStart( thread2 );
    
    DKThreadJoin( thread1 );
    DKThreadJoin( thread2 );

    DKRelease( thread1 );
    DKRelease( thread2 );
    
    DKRelease( array1 );
    DKRelease( array2 );
}


@end






