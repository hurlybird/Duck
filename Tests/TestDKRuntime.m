//
//  TestDKObject.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#include <pthread.h>


DKDeclareMessageSelector( Square, int, int * );
DKThreadSafeSelectorInit( Square );

DKDeclareMessageSelector( Cube, int, int * );
DKThreadSafeSelectorInit( Cube );

static void TestOne( DKObjectRef _self, DKSEL sel, int x, int * y )
{
    *y = 1;
}

static void TestSquare( DKObjectRef _self, DKSEL sel, int x, int * y )
{
    *y = x * x;
}

static void TestCube( DKObjectRef _self, DKSEL sel, int x, int * y )
{
    *y = x * x * x;
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

    DKSetErrorCallback( RaiseException );
}

- (void) tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void) testRuntime
{
    // Define a sample class
    DKClassRef TestClassA = DKAllocClass( DKSTR( "A" ), DKObjectClass(), sizeof(DKObject), 0 );
    XCTAssert( TestClassA );
    
    DKClassRef TestClassB = DKAllocClass( DKSTR( "B" ), TestClassA, sizeof(DKObject), 0 );
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
    
    // DKQueryInterface should return the same object when called on the class or an instance of the class
    XCTAssert( DKGetInterface( TestClassB, DKSelector(Allocation) ) == DKGetInterface( b, DKSelector(Allocation) ) );

    // Try calling our custom message handlers
    int y;
    
    DKMsgSend( a, Square, 2, &y );
    XCTAssert( y == 1 );

    DKMsgSend( a, Cube, 2, &y );
    XCTAssert( y == 1 );

    DKMsgSend( b, Square, 2, &y );
    XCTAssert( y == 4 );

    DKMsgSend( b, Cube, 2, &y );
    XCTAssert( y == 8 );

    // Cleanup
    DKRelease( a );
    DKRelease( b );
    DKRelease( TestClassA );
    DKRelease( TestClassB );
}


static void * ReleaseThread( void * list )
{
    DKIndex count = DKListGetCount( list );
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKListRemoveObjectAtIndex( list, 0 );
    }

    return NULL;
}

static void * ResolveWeakThread( void * list )
{
    DKIndex count = DKListGetCount( list );
    
    intptr_t n = 0;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKWeakRef weakref = DKListGetObjectAtIndex( list, i );
        DKObjectRef strongref = DKResolveWeak( weakref );
        
        n += (strongref != NULL);
        
        DKRelease( strongref );
    }
    
    return (void *)n;
}

- (void) testDKReferenceCountingStressTest
{
    const int N = 10000;

    DKMutableListRef array1 = DKCreate( DKMutableArrayClass() );
    DKMutableListRef array2 = DKCreate( DKMutableArrayClass() );

    for( int i = 0; i < N; i++ )
    {
        char buffer[32];
        sprintf( buffer, "%d", i );
    
        DKStringRef s = DKStringCreateWithCString( DKStringClass(), buffer );
        
        DKListAppendObject( array1, s );
        DKListAppendObject( array2, s );
        DKRelease( s );
    }

    pthread_t thread1;
    pthread_create( &thread1, NULL, ReleaseThread, (void *)array1 );
    
    pthread_t thread2;
    pthread_create( &thread2, NULL, ReleaseThread, (void *)array2 );
    
    void * result1;
    pthread_join( thread1, &result1 );
    
    void * result2;
    pthread_join( thread2, &result2 );
    
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
        char buffer[32];
        sprintf( buffer, "%d", i );
    
        DKStringRef s = DKStringCreateWithCString( DKStringClass(), buffer );
        DKWeakRef w = DKRetainWeak( s );
        
        DKListAppendObject( array1, s );
        DKListAppendObject( array2, w );
        
        DKRelease( s );
        DKRelease( w );
    }

    pthread_t thread1;
    pthread_create( &thread1, NULL, ReleaseThread, (void *)array1 );
    
    pthread_t thread2;
    pthread_create( &thread2, NULL, ResolveWeakThread, (void *)array2 );
    
    intptr_t result1;
    pthread_join( thread1, (void **)&result1 );
    
    intptr_t result2;
    pthread_join( thread2, (void **)&result2 );
    
    DKRelease( array1 );
    DKRelease( array2 );
}


@end






