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
    
    DKMutableDictionaryRef dict = (DKMutableDictionaryRef)DKCreate( dictionaryClass );
    
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

@end
