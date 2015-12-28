//
//  TestDKEgg.m
//  Duck
//
//  Created by Derek Nylen on 2014-04-21.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

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

    DKRuntimeInit( 0 );
    DKSetErrorCallback( RaiseException );
    DKPushAutoreleasePool();
}

- (void) tearDown
{
    DKPopAutoreleasePool();
    
    [super tearDown];
}

typedef struct
{
    int x;
    float y;
    
} TestStruct;


- (void) testEgg
{
    TestStruct testStruct = { 11, 17.0f };

    DKStringRef s1 = DKStringWithCString( "Hello World!" );
    DKNumberRef n1 = DKNumberWithInt32( 1 );
    DKNumberRef d1 = DKNumberWithDate( NULL );
    DKStructRef x1 = DKStructWithType( &testStruct, TestStruct );
    DKArrayRef a1 = DKAutorelease( DKListInitWithObjects( DKAlloc( DKArrayClass() ),
        DKSTR( "Dick" ),
        DKSTR( "Jane" ),
        DKSTR( "Spot" ),
        NULL ) );
    DKLinkedListRef l1 = DKAutorelease( DKListInitWithObjects( DKAlloc( DKListClass() ),
        DKSTR( "Dick" ),
        DKSTR( "Jane" ),
        DKSTR( "Spot" ),
        NULL ) );
    DKHashTableRef h1 = DKAutorelease( DKDictionaryInitWithKeysAndObjects( DKAlloc( DKHashTableClass() ),
        DKSTR( "Dick" ), DKSTR( "Boy" ),
        DKSTR( "Jane" ), DKSTR( "Girl" ),
        DKSTR( "Spot" ), DKSTR( "Dog" ),
        NULL ) );
    DKBinaryTreeRef b1 = DKAutorelease( DKDictionaryInitWithKeysAndObjects( DKAlloc( DKBinaryTreeClass() ),
        DKSTR( "Dick" ), DKSTR( "Boy" ),
        DKSTR( "Jane" ), DKSTR( "Girl" ),
        DKSTR( "Spot" ), DKSTR( "Dog" ),
        NULL ) );
    DKMutableDictionaryRef doc1 = DKDictionaryWithKeysAndObjects(
        DKSTR( "Dick" ), DKSTR( "\"boy\"" ),
        DKSTR( "Jane" ), DKSTR( "girl" ),
        DKSTR( "Spot" ), DKSTR( "dog" ),
        DKSTR( "List" ), DKListWithObjects(
            DKNumberWithInt32( 1 ),
            DKNumberWithInt32( 2 ),
            DKNumberWithDouble( 3.5 ),
            NULL ),
        DKSTR( "Yup" ), DKTrue(),
        DKSTR( "Nope" ), DKFalse(),
        DKSTR( "Null" ), NULL,
        NULL );

    DKEggArchiverRef archiver = DKNew( DKEggArchiverClass() );
    DKEggAddObject( archiver, DKSTR( "string" ), s1 );
    DKEggAddObject( archiver, DKSTR( "number" ), n1 );
    DKEggAddObject( archiver, DKSTR( "date" ), d1 );
    DKEggAddObject( archiver, DKSTR( "struct" ), x1 );
    DKEggAddObject( archiver, DKSTR( "array" ), a1 );
    DKEggAddObject( archiver, DKSTR( "linked-list" ), l1 );
    DKEggAddObject( archiver, DKSTR( "hash-table" ), h1 );
    DKEggAddObject( archiver, DKSTR( "binary-tree" ), b1 );
    DKEggAddObject( archiver, DKSTR( "document" ), doc1 );
    
    DKDataRef archivedData = DKEggArchiverCopyData( archiver );
    
    DKEggUnarchiverRef unarchiver = DKNewEggUnarchiverWithData( archivedData );

    DKStringRef s2 = DKEggGetObject( unarchiver, DKSTR( "string" ) );
    XCTAssert( DKEqual( s1, s2 ) );

    DKNumberRef n2 = DKEggGetObject( unarchiver, DKSTR( "number" ) );
    XCTAssert( DKEqual( n1, n2 ) );

    DKNumberRef d2 = DKEggGetObject( unarchiver, DKSTR( "date" ) );
    XCTAssert( DKEqual( d1, d2 ) );

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
    
    DKDictionaryRef doc2 = DKEggGetObject( unarchiver, DKSTR( "document" ) );
    XCTAssert( DKEqual( doc1, doc2 ) );

    DKRelease( archiver );
    DKRelease( unarchiver );
    DKRelease( archivedData );
}

@end
