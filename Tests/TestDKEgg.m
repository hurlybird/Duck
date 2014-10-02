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
    DKStringRef string1 = DKStringCreateWithCString( DKStringClass(), "Hello World!" );

    DKEggArchiverRef archiver = DKCreate( DKEggArchiverClass() );
    DKEggAddObject( archiver, DKSTR( "string" ), string1 );
    
    DKDataRef archivedData = DKEggArchiverCreateData( archiver );
    
    DKEggUnarchiverRef unarchiver = DKEggCreateUnarchiverWithData( archivedData );
    DKStringRef string2 = DKRetain( DKEggGetObject( unarchiver, DKSTR( "string" ) ) );

    DKRelease( archiver );
    DKRelease( unarchiver );
    
    DKRelease( archivedData );
    DKRelease( string1 );
    DKRelease( string2 );
}

@end
