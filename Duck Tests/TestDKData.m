//
//  TestDKData.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>

@interface TestDKData : XCTestCase

@end

@implementation TestDKData

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

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

- (void) testDKData
{
    DKSetErrorCallback( RaiseException );

    DKMutableDataRef data = DKDataCreateMutable();
    
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

@end
