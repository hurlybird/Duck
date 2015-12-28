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


#define PERFORMANCE_N   100000

- (void) testNSArrayPerformance
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


- (void) testDKArrayPerformance
{
#if !DEBUG
    [self testListClassPerformance:DKMutableArrayClass()];
#endif
}


- (void) testDKLinkedListPerformance
{
#if !DEBUG
    //[self testListClassPerformance:DKMutableLinkedListClass()];
#endif
}


- (void) testListClassPerformance:(DKClassRef)listClass
{
    DKMutableListRef list = DKNew( listClass );

    srand( 0 );

    [self measureBlock:^{
    
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
    }];

    DKRelease( list );
}



@end
