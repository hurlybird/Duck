//
//  TestDKString.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-29.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

@interface TestDKString : XCTestCase

@end

@implementation TestDKString

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

- (void) testDKString
{
    const char * _quickFox = "The quick brown fox jumps over a lazy dog.";
    const char * _quick = "quick";

    DKStringRef quickFox = DKStringCreateWithCString( _quickFox );
    DKStringRef quick = DKStringCreateWithCString( _quick );
    
    XCTAssert( strcmp( DKStringGetCStringPtr( quickFox ), _quickFox ) == 0 );
    
    DKRange range = DKStringGetRangeOfString( quickFox, quick, 0 );
    XCTAssert( (range.location == 4) && (range.length == 5) );

    DKStringRef substring = DKStringCopySubstring( quickFox, range );
    XCTAssert( DKEqual( quick, substring ) );
    DKRelease( substring );
    
    DKRelease( quickFox );
    DKRelease( quick );
}

@end
