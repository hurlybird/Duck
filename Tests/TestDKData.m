//
//  TestDKData.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKData : XCTestCase

@end

@implementation TestDKData

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

- (void) testDKData
{
    DKMutableDataRef data = DKNewMutableData();
    
    const char * a = "aaaaaaaa";
    const char * b = "bbbbbbbb";
    const char * c = "cccccccc";
    const char * d = "dddddddd";
    const char * e = "eeeeeeee";
    
    DKDataAppendBytes( data, a, 10 );
    DKDataAppendBytes( data, b, 10 );
    DKDataAppendBytes( data, c, 10 );
    DKDataAppendBytes( data, d, 10 );

    XCTAssert( DKDataGetLength( data ) == 40 );
    
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 0, 10 ) ), a ) == 0 );
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 10, 10 ) ), b ) == 0 );
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 20, 10 ) ), c ) == 0 );
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 30, 10 ) ), d ) == 0 );

    DKDataRef copy = DKCopy( data );
    XCTAssert( DKDataGetLength( copy ) == 40 );
    DKRelease( copy );

    DKDataReplaceBytes( data, DKRangeMake( 0, 0 ), e, 10 );
    XCTAssert( DKDataGetLength( data ) == 50 );
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 0, 10 ) ), e ) == 0 );
    
    DKDataReplaceBytes( data, DKRangeMake( 0, 10 ), b, 10 );
    XCTAssert( DKDataGetLength( data ) == 50 );
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 0, 10 ) ), b ) == 0 );
    
    DKDataDeleteBytes( data, DKRangeMake( 0, 10 ) );
    XCTAssert( DKDataGetLength( data ) == 40 );
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 0, 10 ) ), a ) == 0 );

    DKDataReplaceBytes( data, DKRangeMake( 10, 0 ), e, 10 );
    XCTAssert( DKDataGetLength( data ) == 50 );
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 10, 10 ) ), e ) == 0 );

    DKDataDeleteBytes( data, DKRangeMake( 10, 10 ) );
    XCTAssert( DKDataGetLength( data ) == 40 );
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 10, 10 ) ), b ) == 0 );

    DKDataAppendBytes( data, e, 10 );
    XCTAssert( DKDataGetLength( data ) == 50 );
    XCTAssert( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 40, 10 ) ), e ) == 0 );

    DKDataDeleteBytes( data, DKRangeMake( 40, 10 ) );
    XCTAssert( DKDataGetLength( data ) == 40 );
    XCTAssertThrows( DKDataGetByteRange( data, DKRangeMake( 40, 10 ) ) );
    
    DKDataDeleteBytes( data, DKRangeMake( 0, DKDataGetLength( data ) ) );
    
    DKRelease( data );
}

- (void) testDKDataStream
{
    DKSetErrorCallback( RaiseException );

    const char * a = "aaaaaaaa";
    const char * b = "bbbbbbbb";
    const char * c = "cccccccc";

    DKMutableDataRef data = DKNewMutableData();
    
    XCTAssert( DKWrite( data, a, 1, 10 ) == 10 );
    XCTAssert( DKTell( data ) == 10 );
    
    XCTAssert( DKWrite( data, b, 1, 10 ) == 10 );
    XCTAssert( DKTell( data ) == 20 );

    XCTAssert( DKWrite( data, c, 1, 10 ) == 10 );
    XCTAssert( DKTell( data ) == 30 );
    
    char buffer[10];
    
    XCTAssert( DKSeek( data, 10, SEEK_SET ) == 0 );
    XCTAssert( DKRead( data, buffer, 1, 10 ) == 10 );
    XCTAssert( strcmp( buffer, b ) == 0 );
    
    DKRelease( data );
}

@end
