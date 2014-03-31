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

- (void) testDKDictionaryClass:(DKTypeRef)dictionaryClass
{
    const int N = 10000;
    
    DKPointerArray keys;
    DKPointerArrayInit( &keys );
    DKPointerArrayReserve( &keys, N );
    keys.length = N;

    DKPointerArray values;
    DKPointerArrayInit( &values );
    DKPointerArrayReserve( &values, N );
    values.length = N;
    
    DKMutableDictionaryRef dict = (DKMutableDictionaryRef)DKCreate( dictionaryClass );
    
    for( int i = 0; i < N; i++ )
    {
        char buffer[32];
        
        sprintf( buffer, "Key%d", i );
        keys.data[i] = (uintptr_t)DKStringCreateWithCString( buffer );

        sprintf( buffer, "Value%d", i );
        values.data[i] = (uintptr_t)DKStringCreateWithCString( buffer );

        DKDictionarySetObject( dict, (DKTypeRef)keys.data[i], (DKTypeRef)values.data[i] );

        XCTAssert( DKDictionaryGetCount( dict ) == (i + 1) );
        XCTAssert( DKDictionaryContainsKey( dict, (DKTypeRef)keys.data[i] ) );
    }
    
    XCTAssert( DKDictionaryGetCount( dict ) == N );

    for( int i = 0; i < N; i++ )
    {
        DKTypeRef value = DKDictionaryGetObject( dict, (DKTypeRef)keys.data[i] );
        XCTAssert( value == (DKTypeRef)values.data[i] );
    }
    
    DKPointerArrayShuffle( &keys );
    
    for( int i = 0; i < N; i++ )
    {
        DKDictionaryRemoveObject( dict, (DKTypeRef)keys.data[i] );
        XCTAssert( DKDictionaryGetCount( dict ) == N - (i + 1) );
    }

    for( int i = 0; i < N; i++ )
    {
        DKRelease( (DKTypeRef)keys.data[i] );
        DKRelease( (DKTypeRef)values.data[i] );
    }
    
    DKRelease( dict );
    
    DKPointerArrayFinalize( &keys );
    DKPointerArrayFinalize( &values );
}

@end
