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

- (void)setUp
{
    [super setUp];

    DKSetErrorCallback( RaiseException );
}

- (void)tearDown
{
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void) testEgg
{
}

@end
