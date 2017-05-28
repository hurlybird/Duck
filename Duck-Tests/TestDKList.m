//
//  Duck_Tests.m
//  Duck Tests
//
//  Created by Derek Nylen on 2014-03-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

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

    DKRuntimeInit( 0 );
    DKSetErrorCallback( RaiseException );
    DKPushAutoreleasePool();
}

- (void) tearDown
{
    DKPopAutoreleasePool();
    
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

- (void) testDKPriorityQueue
{
    DKStringRef a = DKSTR( "a" );
    DKStringRef b = DKSTR( "b" );
    DKStringRef c = DKSTR( "c" );
    DKStringRef d = DKSTR( "d" );

    DKObjectRef pq = DKMutableLinkedList();
    
    DKLinkedListInsertObjectWithPriority( pq, a, 4, DKInsertAlways );
    DKLinkedListInsertObjectWithPriority( pq, d, 1, DKInsertAlways );
    DKLinkedListInsertObjectWithPriority( pq, b, 3, DKInsertAlways );
    DKLinkedListInsertObjectWithPriority( pq, c, 2, DKInsertAlways );
    
    XCTAssert( DKListGetFirstIndexOfObject( pq, a ) == 0 );
    XCTAssert( DKListGetFirstIndexOfObject( pq, b ) == 1 );
    XCTAssert( DKListGetFirstIndexOfObject( pq, c ) == 2 );
    XCTAssert( DKListGetFirstIndexOfObject( pq, d ) == 3 );

    DKLinkedListInsertObjectWithPriority( pq, a, 1, DKInsertAlways );
    DKLinkedListInsertObjectWithPriority( pq, d, 4, DKInsertAlways );
    DKLinkedListInsertObjectWithPriority( pq, b, 2, DKInsertAlways );
    DKLinkedListInsertObjectWithPriority( pq, c, 3, DKInsertAlways );
    
    XCTAssert( DKListGetCount( pq ) == 4 );
    XCTAssert( DKListGetFirstIndexOfObject( pq, a ) == 3 );
    XCTAssert( DKListGetFirstIndexOfObject( pq, b ) == 2 );
    XCTAssert( DKListGetFirstIndexOfObject( pq, c ) == 1 );
    XCTAssert( DKListGetFirstIndexOfObject( pq, d ) == 0 );
}

- (void) testListClass:(DKClassRef)listClass
{
    DKStringRef a = DKSTR( "a" );
    DKStringRef b = DKSTR( "b" );
    DKStringRef c = DKSTR( "c" );
    DKStringRef d = DKSTR( "d" );
    
    DKMutableListRef list = DKNew( listClass );
    
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
    DKListInsertObjectAtIndex( list, a, 0 );
    DKListInsertObjectAtIndex( list, b, 0 );
    DKListInsertObjectAtIndex( list, c, 0 );
    DKListInsertObjectAtIndex( list, d, 0 );

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

    XCTAssert( DKEqual( list, copy ) );
    XCTAssert( DKListGetObjectAtIndex( list, 0 ) == a );
    XCTAssert( DKListGetObjectAtIndex( list, 1 ) == b );
    XCTAssert( DKListGetObjectAtIndex( list, 2 ) == c );
    XCTAssert( DKListGetObjectAtIndex( list, 3 ) == d );
    
    DKListRemoveAllObjects( list );
    DKRelease( copy );
    
    // Reverse
    DKListAppendObject( list, a );
    DKListAppendObject( list, b );
    DKListAppendObject( list, c );
    DKListAppendObject( list, d );
    
    DKListReverse( list );
    
    XCTAssert( DKListGetObjectAtIndex( list, 0 ) == d );
    XCTAssert( DKListGetObjectAtIndex( list, 1 ) == c );
    XCTAssert( DKListGetObjectAtIndex( list, 2 ) == b );
    XCTAssert( DKListGetObjectAtIndex( list, 3 ) == a );

    DKListRemoveAllObjects( list );

    // Cleanup
    DKRelease( list );
}


#define PERFORMANCE_N   100000

- (void) testNSArrayPerformanceRandomAccess
{
#if !DEBUG
    NSMutableArray * array = [NSMutableArray array];
    srand( 0 );

    [self measureBlock:^{
    
        for( int i = 0; i < PERFORMANCE_N; i++ )
            [array addObject:[NSString stringWithFormat:@"%d", i]];
        
        for( int i = 0; i < PERFORMANCE_N; i++ )
        {
            int index1 = rand() % PERFORMANCE_N;
            int index2 = rand() % PERFORMANCE_N;
        
            NSString * value1 = [array objectAtIndex:index1];
            NSString * value2 = [array objectAtIndex:index2];
            
            [array replaceObjectAtIndex:index2 withObject:value1];
            [array replaceObjectAtIndex:index1 withObject:value2];
        }
    }];
#endif
}

- (void) testNSArrayPerformanceQueueAccess
{
#if !DEBUG
    NSMutableArray * array = [NSMutableArray array];

    [self measureBlock:^()
    {
        for( int i = 0; i < PERFORMANCE_N; i++ )
            [array addObject:[NSString stringWithFormat:@"%d", i]];
        
        while( array.count > 0 )
            [array removeObjectAtIndex:0];
    }];
#endif
}

- (void) testDKArrayPerformanceRandomAccess
{
#if !DEBUG
    DKObjectRef list = DKNewMutableArray();
    srand( 0 );
    
    [self measureBlock:^()
    {
        [self testListClassRandomAccess:list];
    }];

    DKRelease( list );
#endif
}


- (void) testDKArrayPerformanceQueueAccess
{
#if !DEBUG
    DKObjectRef list = DKNewMutableArray();

    [self measureBlock:^()
    {
        [self testListClassQueueAccess:list];
    }];

    DKRelease( list );
#endif
}


- (void) testDKLinkedListPerformanceRandomAccess
{
#if !DEBUG
    //DKObjectRef list = DKNewMutableLinkedList();
    //srand( 0 );

    //[self measureBlock:^()
    //{
    //    [self testListClassRandomAccess:list];
    //}];

    //DKRelease( list );
#endif
}

- (void) testDKLinkedListPerformanceQueueAccess
{
#if !DEBUG
    DKObjectRef list = DKNewMutableLinkedList();
    
    [self measureBlock:^()
    {
        [self testListClassQueueAccess:list];
    }];

    DKRelease( list );
#endif
}


- (void) testListClassRandomAccess:(DKMutableListRef)list
{
    for( int i = 0; i < PERFORMANCE_N; i++ )
    {
        DKStringRef s = DKStringInitWithFormat( DKAlloc( DKStringClass() ), "%d", i );
        DKListAppendObject( list, s );
        DKRelease( s );
    }
    
    for( int i = 0; i < PERFORMANCE_N; i++ )
    {
        int index1 = rand() % PERFORMANCE_N;
        int index2 = rand() % PERFORMANCE_N;
    
        DKStringRef value1 = DKRetain( DKListGetObjectAtIndex( list, index1 ) );
        DKStringRef value2 = DKRetain( DKListGetObjectAtIndex( list, index2 ) );

        DKListSetObjectAtIndex( list, value1, index2 );
        DKListSetObjectAtIndex( list, value2, index1 );
        
        DKRelease( value1 );
        DKRelease( value2 );
    }
}


- (void) testListClassQueueAccess:(DKMutableListRef)list
{
    for( int i = 0; i < PERFORMANCE_N; i++ )
    {
        DKStringRef s = DKStringInitWithFormat( DKAlloc( DKStringClass() ), "%d", i );
        DKListAppendObject( list, s );
        DKRelease( s );
    }
    
    while( DKListGetCount( list ) )
    {
        DKListRemoveFirstObject( list );
    }
}



@end
