//
//  TestDKDictionary.m
//  Duck
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

@interface TestDKDictionary : XCTestCase

@end

@implementation TestDKDictionary

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
    [self testDKDictionaryClass:DKMutableHashTableClass()];
}

- (void) testDKBinaryTree
{
    [self testDKDictionaryClass:DKMutableBinaryTreeClass()];
}

- (void) testDKDictionaryClass:(DKClassRef)dictionaryClass
{
    const int N = 10000;
    
    DKGenericArray keys;
    DKGenericArrayInit( &keys, sizeof(DKStringRef) );
    DKGenericArrayReserve( &keys, N );
    keys.length = N;

    DKGenericArray values;
    DKGenericArrayInit( &values, sizeof(DKStringRef) );
    DKGenericArrayReserve( &values, N );
    values.length = N;
    
    DKMutableDictionaryRef dict = DKNew( dictionaryClass );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKStringWithFormat( "Key%d", i );
        DKStringRef value = DKStringWithFormat( "Value%d", i );
        
        DKGenericArrayElementAtIndex( &keys, i, DKStringRef ) = key;
        DKGenericArrayElementAtIndex( &values, i, DKStringRef ) = value;

        DKDictionarySetObject( dict, key, value );
        DKDictionaryRemoveObject( dict, key );

        XCTAssert( DKDictionaryGetCount( dict ) == 0 );
    }
    
    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKStringWithFormat( "Key%d", i );
        DKStringRef value = DKStringWithFormat( "Value%d", i );
        
        DKGenericArrayElementAtIndex( &keys, i, DKStringRef ) = key;
        DKGenericArrayElementAtIndex( &values, i, DKStringRef ) = value;

        DKDictionarySetObject( dict, key, value );

        XCTAssert( DKDictionaryGetCount( dict ) == (i + 1) );
        XCTAssert( DKDictionaryContainsKey( dict, key ) );
    }
    
    XCTAssert( DKDictionaryGetCount( dict ) == N );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKGenericArrayElementAtIndex( &keys, i, DKObjectRef );
        DKStringRef value = DKGenericArrayElementAtIndex( &values, i, DKObjectRef );
        XCTAssert( DKDictionaryGetObject( dict, key ) == value );
    }
    
    DKGenericArrayShuffle( &keys );
    
    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKGenericArrayElementAtIndex( &keys, i, DKObjectRef );
        DKDictionaryRemoveObject( dict, key );
        XCTAssert( DKDictionaryGetCount( dict ) == N - (i + 1) );
    }

    XCTAssert( DKDictionaryGetCount( dict ) == 0 );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKGenericArrayElementAtIndex( &keys, i, DKObjectRef );
        DKStringRef value = DKGenericArrayElementAtIndex( &values, i, DKObjectRef );
        DKDictionarySetObject( dict, key, value );
    }

    XCTAssert( DKDictionaryGetCount( dict ) == N );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKGenericArrayElementAtIndex( &keys, i, DKObjectRef );
        DKStringRef value = DKGenericArrayElementAtIndex( &values, i, DKObjectRef );
        XCTAssert( DKDictionaryGetObject( dict, key ) == value );
    }
        
    DKDictionaryRemoveAllObjects( dict );

    XCTAssert( DKDictionaryGetCount( dict ) == 0 );

    DKRelease( dict );
    DKGenericArrayFinalize( &keys );
    DKGenericArrayFinalize( &values );
}




// Performance Tests =====================================================================
#define PERFORMANCE_TESTS 1

const int PERFORMANCE_N = 1000000;


- (void) testNSDictionaryReadPerformance
{
#if PERFORMANCE_TESTS
    NSString * path = [[NSBundle bundleForClass:[self class]] pathForResource:@"dictionary" ofType:@"txt"];
    NSString * file = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
    NSArray * words = [file componentsSeparatedByString:@"\n"];
    int count = (int)[words count];

    NSMutableDictionary * dict = [NSMutableDictionary dictionary];
    
    for( NSString * word in words )
        [dict setObject:word forKey:word];
    
    srand( 0 );
    
    [self measureBlock:^{

        for( int i = 0; i < PERFORMANCE_N; i++ )
        {
            int x = rand() % count;
            NSString * word = [words objectAtIndex:x];
            [dict objectForKey:word];
        }
    }];
#endif
}


- (void) testNSDictionaryWritePerformance
{
#if PERFORMANCE_TESTS
    NSString * path = [[NSBundle bundleForClass:[self class]] pathForResource:@"dictionary" ofType:@"txt"];
    NSString * file = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
    NSArray * words = [file componentsSeparatedByString:@"\n"];
    int count = (int)[words count];
    
    srand( 0 );
    
    [self measureBlock:^{

        NSMutableDictionary * dict = [NSMutableDictionary dictionary];

        for( int i = 0; i < PERFORMANCE_N; i++ )
        {
            int x = rand() % count;
            NSString * word = [words objectAtIndex:x];
            [dict setObject:word forKey:word];
        }
    }];
#endif
}



- (void) testDKHashTableReadPerformance
{
#if PERFORMANCE_TESTS
    [self testDictionaryClassReadPerformance:DKMutableHashTableClass()];
#endif
}


- (void) testDKBinaryTreeReadPerformance
{
#if PERFORMANCE_TESTS
    //[self testDictionaryClassReadPerformance:DKMutableBinaryTreeClass()];
#endif
}

- (void) testDKHashTableWritePerformance
{
#if PERFORMANCE_TESTS
    [self testDictionaryClassWritePerformance:DKMutableHashTableClass()];
#endif
}


- (void) testDKBinaryTreeWritePerformance
{
#if PERFORMANCE_TESTS
    //[self testDictionaryClassWritePerformance:DKMutableBinaryTreeClass()];
#endif
}


- (void) testDictionaryClassReadPerformance:(DKClassRef)dictionaryClass
{
    NSString * _path = [[NSBundle bundleForClass:[self class]] pathForResource:@"dictionary" ofType:@"txt"];
    DKStringRef path = DKStringWithCString( [_path UTF8String] );
    DKStringRef file = DKStringWithContentsOfFile( path );
    DKArrayRef words = (DKArrayRef)DKStringSplit( file, DKSTR( "\n" ) );
    int count = (int)DKArrayGetCount( words );
    
    DKMutableDictionaryRef dict = DKAutorelease( DKNew( dictionaryClass ) );
    
    for( int i = 0; i < count; i++ )
    {
        DKStringRef word = DKArrayGetObjectAtIndex( words, i );
        DKDictionarySetObject( dict, word, word );
    }
    
    srand( 0 );
    
    [self measureBlock:^{

        for( int i = 0; i < PERFORMANCE_N; i++ )
        {
            int x = rand() % count;
            DKStringRef word = DKArrayGetObjectAtIndex( words, x );
            DKDictionaryGetObject( dict, word );
        }
    }];
}

- (void) testDictionaryClassWritePerformance:(DKClassRef)dictionaryClass
{
    NSString * _path = [[NSBundle bundleForClass:[self class]] pathForResource:@"dictionary" ofType:@"txt"];
    DKStringRef path = DKStringWithCString( [_path UTF8String] );
    DKStringRef file = DKStringWithContentsOfFile( path );
    DKArrayRef words = (DKArrayRef)DKStringSplit( file, DKSTR( "\n" ) );
    int count = (int)DKArrayGetCount( words );
    
    srand( 0 );
    
    [self measureBlock:^{

        DKMutableDictionaryRef dict = DKNew( dictionaryClass );

        for( int i = 0; i < PERFORMANCE_N; i++ )
        {
            int x = rand() % count;
            DKStringRef word = DKArrayGetObjectAtIndex( words, x );
            DKDictionarySetObject( dict, word, word );
        }

        DKRelease( dict );
    }];
}

@end



