//
//  TestDKDictionary.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

@interface TestDKDictionary : XCTestCase

@end

@implementation TestDKDictionary

- (void)setUp
{
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown
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
    
    DKTypeRef * keys = dk_malloc( sizeof(DKTypeRef) * N );
    DKTypeRef * values = dk_malloc( sizeof(DKTypeRef) * N );

    DKMutableDictionaryRef dict = (DKMutableDictionaryRef)DKCreate( dictionaryClass );
    
    for( int i = 0; i < N; i++ )
    {
        char buffer[32];
        
        sprintf( buffer, "Key%d", i );
        keys[i] = DKDataCreateWithBytes( buffer, strlen( buffer) + 1 );

        sprintf( buffer, "Value%d", i );
        values[i] = DKDataCreateWithBytes( buffer, strlen( buffer) + 1 );

        DKDictionarySetObject( dict, keys[i], values[i] );

        XCTAssert( DKDictionaryGetCount( dict ) == (i + 1) );
        XCTAssert( DKDictionaryContainsKey( dict, keys[i] ) );
    }
    
    XCTAssert( DKDictionaryGetCount( dict ) == N );

    for( int i = 0; i < N; i++ )
    {
        DKTypeRef value = DKDictionaryGetObject( dict, keys[i] );
        XCTAssert( value == values[i] );
    }
    
    dk_shuffle( (uintptr_t *)keys, N );
    
    for( int i = 0; i < N; i++ )
    {
        DKDictionaryRemoveObject( dict, keys[i] );
        XCTAssert( DKDictionaryGetCount( dict ) == N - (i + 1) );
    }

    for( int i = 0; i < N; i++ )
    {
        DKRelease( keys[i] );
        DKRelease( values[i] );
    }
    
    DKRelease( dict );
    dk_free( keys );
    dk_free( values );
}

@end
