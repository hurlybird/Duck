//
//  TestDKString.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-29.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKString : XCTestCase

@end

@implementation TestDKString

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

- (void) testDKString
{
    DKStringRef quickFox = DKSTR( "The quick brown fox jumps over a lazy dog." );
    DKStringRef quick = DKSTR( "quick" );
    
    XCTAssert( strcmp( DKStringGetCStringPtr( quick ), "quick" ) == 0 );
    XCTAssert( DKStringGetCStringPtr( quick ) == "quick" );
    
    DKRange range = DKStringGetRangeOfString( quickFox, quick, 0 );
    XCTAssert( (range.location == 4) && (range.length == 5) );

    DKStringRef substring = DKStringCopySubstring( quickFox, range );
    XCTAssert( DKStringEqual( quick, substring ) );
    DKRelease( substring );

    DKMutableStringRef mutableString = DKStringCreateMutableCopy( quickFox );
    XCTAssert( DKStringEqual( quickFox, mutableString ) );
    
    range = DKStringGetRangeOfString( mutableString, quick, 0 );
    XCTAssert( (range.location == 4) && (range.length == 5) );
    range.length += 1;
    
    DKStringReplaceSubstring( mutableString, range, DKSTR( "slow " ) );
    XCTAssert( DKStringEqual( mutableString, DKSTR( "The slow brown fox jumps over a lazy dog." ) ) );
    
    DKRelease( mutableString );
}

- (void) testDKStringStream
{

    const char * a = "aaaaaaaaaa";
    const char * b = "bbbbbbbbbb";
    const char * c = "cccccccccc";

    DKMutableStringRef str = DKStringCreateMutable();
    
    XCTAssert( DKWrite( str, a, 1, 10 ) == 10 );
    XCTAssert( DKTell( str ) == 10 );
    
    XCTAssert( DKWrite( str, b, 1, 10 ) == 10 );
    XCTAssert( DKTell( str ) == 20 );

    XCTAssert( DKWrite( str, c, 1, 10 ) == 10 );
    XCTAssert( DKTell( str ) == 30 );
    
    char buffer[11];
    buffer[10] = '\0';
    
    XCTAssert( DKSeek( str, 10, SEEK_SET ) == 0 );
    XCTAssert( DKRead( str, buffer, 1, 10 ) == 10 );
    XCTAssert( strcmp( buffer, b ) == 0 );
    
    DKRelease( str );
}

@end
