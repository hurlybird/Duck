//
//  TestDKShell.m
//  Duck
//
//  Created by Derek Nylen on 2017-06-09.
//  Copyright © 2017 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

static int RaiseException( const char * format, va_list arg_ptr )
{
    @throw NSGenericException;
}

@interface TestDKShell : XCTestCase

@end

@implementation TestDKShell

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

- (void) testDKShell
{
    DKMutableDictionaryRef document = DKDictionaryWithKeysAndObjects(
        DKSTR( "Dick" ), DKSTR( "\"boy\"" ),
        DKSTR( "Jane" ), DKSTR( "girl" ),
        DKSTR( "Spot" ), DKSTR( "dog" ),
        DKSTR( "List" ), DKListWithObjects(
            DKNumberWithInt32( 1 ),
            DKNumberWithInt32( 2 ),
            DKNumberWithDouble( 3.5 ),
            NULL ),
        DKSTR( "Date" ), DKNumberWithDate( NULL ),
        DKSTR( "Yup" ), DKTrue(),
        DKSTR( "Nope" ), DKFalse(),
        DKSTR( "Null" ), NULL,
        NULL );
    
    DKStringRef annotation1 = DKSTR( "JSON File" );
    DKStringRef annotation2 = DKSTR( "Egg Archive" );

    DKMutableDataRef shell = DKMutableData();
    
    DKShellWrite( shell, document, DKShellContentTypeJSON, annotation1, 0 );
    XCTAssert( DKShellWrite( shell, document, DKShellContentTypeEgg, annotation2, 0 ) == 1);
    
    XCTAssertThrows( DKShellWrite( shell, document, DKShellContentTypeText, DKSTR( "Not text" ), 0 ) );
    XCTAssertThrows( DKShellWrite( shell, document, DKShellContentTypeBinary, DKSTR( "Not data" ), 0 ) );
    XCTAssertThrows( DKShellWrite( shell, document, DKShellContentTypeJSON, DKSTR( "Bad annotation\n" ), 0 ) );
    XCTAssertThrows( DKShellWrite( shell, document, DKShellContentTypeJSON, DKSTR( "Bad annotation\r" ), 0 ) );
    
    DKSeek( shell, 0, SEEK_SET );


    DKObjectRef readDocument1;
    DKStringRef readContentType1, readAnnotation1;
    XCTAssert( DKShellRead( shell, &readDocument1, &readContentType1, &readAnnotation1, 0 ) == 1 );

    XCTAssert( DKEqual( readDocument1, document ) );
    XCTAssert( DKStringEqualToString( readContentType1, DKShellContentTypeJSON ) );
    XCTAssert( DKStringEqualToString( readAnnotation1, annotation1 ) );
    

    DKObjectRef readDocument2;
    DKStringRef readContentType2, readAnnotation2;
    XCTAssert( DKShellRead( shell, &readDocument2, &readContentType2, &readAnnotation2, 0 ) == 1);

    XCTAssert( DKEqual( readDocument2, document ) );
    XCTAssert( DKStringEqualToString( readContentType2, DKShellContentTypeEgg ) );
    XCTAssert( DKStringEqualToString( readAnnotation2, annotation2 ) );\
}


@end


