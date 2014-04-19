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

    DKSetErrorCallback( RaiseException );
}

- (void) tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
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
    
    DKElementArray keys;
    DKElementArrayInit( &keys, sizeof(DKStringRef) );
    DKElementArrayReserve( &keys, N );
    keys.length = N;

    DKElementArray values;
    DKElementArrayInit( &values, sizeof(DKStringRef) );
    DKElementArrayReserve( &values, N );
    values.length = N;
    
    DKMutableDictionaryRef dict = (DKMutableDictionaryRef)DKCreate( dictionaryClass );
    
    for( int i = 0; i < N; i++ )
    {
        char buffer[32];
        
        sprintf( buffer, "Key%d", i );
        DKStringRef key = DKStringCreateWithCString( DKStringClass(), buffer );
        
        sprintf( buffer, "Value%d", i );
        DKStringRef value = DKStringCreateWithCString( DKStringClass(), buffer );
        
        DKElementArrayGetElementAtIndex( &keys, i, DKStringRef ) = key;
        DKElementArrayGetElementAtIndex( &values, i, DKStringRef ) = value;

        DKDictionarySetObject( dict, key, value );

        XCTAssert( DKDictionaryGetCount( dict ) == (i + 1) );
        XCTAssert( DKDictionaryContainsKey( dict, key ) );
    }
    
    XCTAssert( DKDictionaryGetCount( dict ) == N );

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKElementArrayGetElementAtIndex( &keys, i, DKObjectRef );
        DKStringRef value = DKElementArrayGetElementAtIndex( &values, i, DKObjectRef );
        XCTAssert( DKDictionaryGetObject( dict, key ) == value );
    }
    
    DKElementArrayShuffle( &keys );
    
    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKElementArrayGetElementAtIndex( &keys, i, DKObjectRef );
        DKDictionaryRemoveObject( dict, key );
        XCTAssert( DKDictionaryGetCount( dict ) == N - (i + 1) );
    }

    for( int i = 0; i < N; i++ )
    {
        DKStringRef key = DKElementArrayGetElementAtIndex( &keys, i, DKStringRef );
        DKStringRef value = DKElementArrayGetElementAtIndex( &values, i, DKStringRef );

        DKRelease( key );
        DKRelease( value );
    }
    
    DKRelease( dict );
    
    DKElementArrayFinalize( &keys );
    DKElementArrayFinalize( &values );
}

@end
