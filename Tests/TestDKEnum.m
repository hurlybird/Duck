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
DKEnumRef TestEnum64( void );

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

DKDefineEnum64( TestEnum64,
    "Zero", (int64_t)Zero,
    "One", (int64_t)One,
    "Two", (int64_t)Two,
    "Three", (int64_t)Three,
    "Four", (int64_t)Four,
    "Five", (int64_t)Five,
    "Size", (int64_t)Six,
    "Seven", (int64_t)Seven,
    "Eight", (int64_t)Eight,
    "Nine", (int64_t)Nine )


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

- (void) testDKStringFromEnum64
{
    XCTAssert( DKEqual( DKStringFromEnum64( TestEnum64(), Four ), DKSTR( "Four" ) ) );
    XCTAssert( !DKEqual( DKStringFromEnum64( TestEnum64(), Seven ), DKSTR( "Five" ) ) );
    XCTAssert( DKStringFromEnum64( TestEnum64(), 17 ) == NULL );
}

- (void) testDKEnumFromString64
{
    XCTAssert( DKEnumFromString64( TestEnum64(), DKSTR( "Three" ) ) == Three );
    XCTAssert( DKEnumFromString64( TestEnum64(), DKSTR( "Eight" ) ) != Two );
    XCTAssert( DKEnumFromString64( TestEnum64(), DKSTR( "Eleven" ) ) == 0 );
}


@end


