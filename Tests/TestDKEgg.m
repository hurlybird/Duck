//
//  TestDKEgg.m
//  Duck
//
//  Created by Derek Nylen on 2014-04-21.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKEgg : XCTestCase

@end

@implementation TestDKEgg

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

struct TestStruct
{
    int x;
    float y;
};

- (void) testEgg
{
    struct TestStruct testStruct = { 11, 17.0f };

    DKStringRef s1 = DKAutorelease( DKStringCreateWithCString( DKStringClass(), "Hello World!" ) );
    DKNumberRef n1 = DKAutorelease( DKNumberCreateInt32( 1 ) );
    DKStructRef x1 = DKAutorelease( DKStructCreate( DKSTR( "TestStruct" ), &testStruct, sizeof(struct TestStruct) ) );
    DKArrayRef a1 = DKAutorelease( DKListCreateWithObjects( DKArrayClass(), DKSTR( "Dick" ), DKSTR( "Jane" ), DKSTR( "Spot" ), NULL ) );
    DKLinkedListRef l1 = DKAutorelease( DKListCreateWithObjects( DKListClass(), DKSTR( "Dick" ), DKSTR( "Jane" ), DKSTR( "Spot" ), NULL ) );
    DKHashTableRef h1 = DKAutorelease( DKDictionaryCreateWithKeysAndObjects( DKHashTableClass(),
        DKSTR( "Dick" ), DKSTR( "Boy" ),
        DKSTR( "Jane" ), DKSTR( "Girl" ),
        DKSTR( "Spot" ), DKSTR( "Dog" ),
        NULL ) );
    DKBinaryTreeRef b1 = DKAutorelease( DKDictionaryCreateWithKeysAndObjects( DKBinaryTreeClass(),
        DKSTR( "Dick" ), DKSTR( "Boy" ),
        DKSTR( "Jane" ), DKSTR( "Girl" ),
        DKSTR( "Spot" ), DKSTR( "Dog" ),
        NULL ) );

    DKEggArchiverRef archiver = DKCreate( DKEggArchiverClass() );
    DKEggAddObject( archiver, DKSTR( "string" ), s1 );
    DKEggAddObject( archiver, DKSTR( "number" ), n1 );
    DKEggAddObject( archiver, DKSTR( "struct" ), x1 );
    DKEggAddObject( archiver, DKSTR( "array" ), a1 );
    DKEggAddObject( archiver, DKSTR( "linked-list" ), l1 );
    DKEggAddObject( archiver, DKSTR( "hash-table" ), h1 );
    DKEggAddObject( archiver, DKSTR( "binary-tree" ), b1 );
    
    DKDataRef archivedData = DKEggArchiverCreateData( archiver );
    
    DKEggUnarchiverRef unarchiver = DKEggCreateUnarchiverWithData( archivedData );

    DKStringRef s2 = DKEggGetObject( unarchiver, DKSTR( "string" ) );
    XCTAssert( DKEqual( s1, s2 ) );

    DKNumberRef n2 = DKEggGetObject( unarchiver, DKSTR( "number" ) );
    XCTAssert( DKEqual( n1, n2 ) );

    DKStructRef x2 = DKEggGetObject( unarchiver, DKSTR( "struct" ) );
    XCTAssert( DKEqual( x1, x2 ) );

    DKArrayRef a2 = DKEggGetObject( unarchiver, DKSTR( "array" ) );
    XCTAssert( DKEqual( a1, a2 ) );

    DKLinkedListRef l2 = DKEggGetObject( unarchiver, DKSTR( "linked-list" ) );
    XCTAssert( DKEqual( l1, l2 ) );

    DKHashTableRef h2 = DKEggGetObject( unarchiver, DKSTR( "hash-table" ) );
    XCTAssert( DKEqual( h1, h2 ) );

    DKBinaryTreeRef b2 = DKEggGetObject( unarchiver, DKSTR( "binary-tree" ) );
    XCTAssert( DKEqual( b1, b2 ) );

    DKRelease( archiver );
    DKRelease( unarchiver );
    DKRelease( archivedData );
}

@end
