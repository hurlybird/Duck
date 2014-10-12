//
//  TestDKDictionary.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

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

    DKRuntimeInit();
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
    
    DKMutableDictionaryRef dict = DKCreate( dictionaryClass );
    
    for( int i = 0; i < N; i++ )
    {
        char buffer[32];
        
        sprintf( buffer, "Key%d", i );
        DKStringRef key = DKStringCreateWithCString( DKStringClass(), buffer );
        
        sprintf( buffer, "Value%d", i );
        DKStringRef value = DKStringCreateWithCString( DKStringClass(), buffer );
        
        DKGenericArrayGetElementAtIndex( &keys, i, DKStringRef ) = key;
        DKGenericArrayGetElementAtIndex( &values, i, DKStringRef ) = value;

        DKDictionarySetObject( dict, key, value );

        XCTAssert( DKDictionaryGetCount( dict ) == (i + 1) );
        XCTAssert( DKDictionaryContainsKey( dict, key ) );
    }
    
    XCTAssert( DKDictionaryGetCount( dict ) == N );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKGenericArrayGetElementAtIndex( &keys, i, DKObjectRef );
        DKStringRef value = DKGenericArrayGetElementAtIndex( &values, i, DKObjectRef );
        XCTAssert( DKDictionaryGetObject( dict, key ) == value );
    }
    
    DKGenericArrayShuffle( &keys );
    
    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKGenericArrayGetElementAtIndex( &keys, i, DKObjectRef );
        DKDictionaryRemoveObject( dict, key );
        XCTAssert( DKDictionaryGetCount( dict ) == N - (i + 1) );
    }

    XCTAssert( DKDictionaryGetCount( dict ) == 0 );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKGenericArrayGetElementAtIndex( &keys, i, DKObjectRef );
        DKStringRef value = DKGenericArrayGetElementAtIndex( &values, i, DKObjectRef );
        DKDictionarySetObject( dict, key, value );
    }

    XCTAssert( DKDictionaryGetCount( dict ) == N );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKGenericArrayGetElementAtIndex( &keys, i, DKObjectRef );
        DKStringRef value = DKGenericArrayGetElementAtIndex( &values, i, DKObjectRef );
        XCTAssert( DKDictionaryGetObject( dict, key ) == value );
    }
        
    DKDictionaryRemoveAllObjects( dict );

    XCTAssert( DKDictionaryGetCount( dict ) == 0 );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKGenericArrayGetElementAtIndex( &keys, i, DKStringRef );
        DKStringRef value = DKGenericArrayGetElementAtIndex( &values, i, DKStringRef );

        DKRelease( key );
        DKRelease( value );
    }
    
    DKRelease( dict );
    
    DKGenericArrayFinalize( &keys );
    DKGenericArrayFinalize( &values );
}


#define PERFORMANCE_N   10000

- (void) testNSDictionaryPerformance
{
    NSMutableDictionary * dict = [NSMutableDictionary dictionary];
    NSMutableArray * keys = [NSMutableArray array];
    
    for( int i = 0; i < PERFORMANCE_N; i++ )
        [keys addObject:[NSString stringWithFormat:@"%d", i]];

    srand( 0 );

    [self measureBlock:^{
    
        for( int i = 0; i < PERFORMANCE_N; i++ )
            [dict setObject:@"Object" forKey:[keys objectAtIndex:i]];

        for( int i = 0; i < PERFORMANCE_N; i++ )
        {
            int index1 = rand() % PERFORMANCE_N;
            int index2 = rand() % PERFORMANCE_N;
        
            NSString * key1 = [keys objectAtIndex:index1];
            NSString * key2 = [keys objectAtIndex:index2];
            
            NSString * value1 = [dict objectForKey:key1];
            NSString * value2 = [dict objectForKey:key2];
            
            [dict setObject:value1 forKey:key2];
            [dict setObject:value2 forKey:key1];
        }
    }];
}


- (void) testDKHashTablePerformance
{
    [self testDictionaryClassPerformance:DKMutableHashTableClass()];
}


- (void) testDKBinaryTreePerformance
{
    [self testDictionaryClassPerformance:DKMutableBinaryTreeClass()];
}


- (void) testDictionaryClassPerformance:(DKClassRef)dictionaryClass
{
    DKMutableDictionaryRef dict = DKCreate( dictionaryClass );
    DKMutableListRef keys = DKCreate( DKMutableArrayClass() );

    for( int i = 0; i < PERFORMANCE_N; i++ )
    {
        DKStringRef s = DKStringCreateWithFormat( DKStringClass(), "%d", i );
        DKListAppendObject( keys, s );
        DKRelease( s );
    }

    srand( 0 );

    [self measureBlock:^{

        for( int i = 0; i < PERFORMANCE_N; i++ )
        {
            DKStringRef key = DKListGetObjectAtIndex( keys, i );
            DKDictionarySetObject( dict, key, DKSTR( "Object" ) );
        }

        for( int i = 0; i < PERFORMANCE_N; i++ )
        {
            int index1 = rand() % PERFORMANCE_N;
            int index2 = rand() % PERFORMANCE_N;
        
            DKStringRef key1 = DKListGetObjectAtIndex( keys, index1 );
            DKStringRef key2 = DKListGetObjectAtIndex( keys, index2 );

            DKStringRef value1 = DKRetain( DKDictionaryGetObject( dict, key1 ) );
            DKStringRef value2 = DKRetain( DKDictionaryGetObject( dict, key2 ) );

            DKDictionarySetObject( dict, key2, value1 );
            DKDictionarySetObject( dict, key1, value2 );

            DKRelease( value1 );
            DKRelease( value2 );
        }
    }];

    DKRelease( dict );
    DKRelease( keys );
}

@end



