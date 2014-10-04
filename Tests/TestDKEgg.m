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

- (void) testEgg
{
    DKStringRef s1 = DKStringCreateWithCString( DKStringClass(), "Hello World!" );
    DKNumberRef n1 = DKNumberCreateInt32( 1 );

    DKEggArchiverRef archiver = DKCreate( DKEggArchiverClass() );
    DKEggAddObject( archiver, DKSTR( "string" ), s1 );
    DKEggAddObject( archiver, DKSTR( "number" ), n1 );
    
    DKDataRef archivedData = DKEggArchiverCreateData( archiver );
    
    DKEggUnarchiverRef unarchiver = DKEggCreateUnarchiverWithData( archivedData );
    DKStringRef s2 = DKEggGetObject( unarchiver, DKSTR( "string" ) );
    DKNumberRef n2 = DKEggGetObject( unarchiver, DKSTR( "number" ) );

    XCTAssert( DKEqual( s1, s2 ) );
    XCTAssert( DKEqual( n1, n2 ) );

    DKRelease( archiver );
    DKRelease( unarchiver );
    
    DKRelease( archivedData );
    DKRelease( s1 );
    DKRelease( n1 );
}

@end
