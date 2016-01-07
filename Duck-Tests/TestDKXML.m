//
//  TestDKXML.m
//  Duck
//
//  Created by Derek Nylen on 2016-01-07.
//  Copyright © 2016 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKXML : XCTestCase

@end

@implementation TestDKXML

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

- (void) testXML
{
    NSString * _path = [[NSBundle bundleForClass:[self class]] pathForResource:@"test" ofType:@"xml"];
    DKStringRef path = DKStringWithCString( [_path UTF8String] );
    DKStringRef file = DKStringWithContentsOfFile( path );
    
    DKXMLDocumentRef xml = DKXMLParse( file, 0 );
    
    DKPrintf( "%@\n", xml );
}


@end


