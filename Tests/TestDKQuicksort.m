//
//  TestDKQuicksort.m
//  Duck-Tests
//
//  Created by Derek Nylen on 2025-05-10.
//  Copyright © 2025 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}


@interface TestDKQuicksort : XCTestCase

@end

@implementation TestDKQuicksort

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


static int CompareInts1( const void * _a, const void * _b )
{
    const int * a = _a;
    const int * b = _b;
    
    return *a - *b;
}

static int CompareInts2( const void * _a, const void * _b, void * context )
{
    const int * a = _a;
    const int * b = _b;
    
    return *a - *b;
}


- (void)testQuicksort
{
    const int N = 20;
    
    int * a = malloc( sizeof(int) * N );
    int * b = malloc( sizeof(int) * N );

    srand( 0 );

    for( int i = 0; i < N; i++ )
        a[i] = b[i] = rand();

    qsort( a, N, sizeof(int), CompareInts1 );
    DKQuicksort( b, N, sizeof(int), CompareInts2, NULL );
    
    XCTAssert( memcmp( a, b, sizeof(int) * N ) == 0 );
    
    free( a );
    free( b );
}


@end
