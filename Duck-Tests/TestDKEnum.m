//
//  TestDKEnum.m
//  Duck
//
//  Created by Derek Nylen on 2017-04-21.
//  Copyright © 2017 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}


enum
{
    Zero,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine

} Test;

DKEnumRef TestEnum( void );

DKDefineEnum( TestEnum,
    "Zero", Zero,
    "One", One,
    "Two", Two,
    "Three", Three,
    "Four", Four,
    "Five", Five,
    "Size", Six,
    "Seven", Seven,
    "Eight", Eight,
    "Nine", Nine )


@interface TestDKEnum : XCTestCase

@end

@implementation TestDKEnum

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

- (void) testDKStringFromEnum
{
    XCTAssert( DKEqual( DKStringFromEnum( TestEnum(), Four ), DKSTR( "Four" ) ) );
    XCTAssert( !DKEqual( DKStringFromEnum( TestEnum(), Seven ), DKSTR( "Five" ) ) );
    XCTAssert( DKStringFromEnum( TestEnum(), 17 ) == NULL );
}

- (void) testDKEnumFromString
{
    XCTAssert( DKEnumFromString( TestEnum(), DKSTR( "Three" ) ) == Three );
    XCTAssert( DKEnumFromString( TestEnum(), DKSTR( "Eight" ) ) != Two );
    XCTAssert( DKEnumFromString( TestEnum(), DKSTR( "Eleven" ) ) == 0 );

}


@end


