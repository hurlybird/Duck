//
//  TestDKString.m
//  Duck
//
//  Created by Derek Nylen on 2014-03-29.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#import <XCTest/XCTest.h>
#import <Duck/Duck.h>

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

    DKRuntimeInit( 0 );
    DKSetErrorCallback( RaiseException );
    DKPushAutoreleasePool();
}

- (void) tearDown
{
    DKPopAutoreleasePool();
    
    [super tearDown];
}

- (void) testDKString
{
    DKRange range;

    DKStringRef quickFox = DKSTR( "The quick brown fox jumps over a lazy dog." );
    
    XCTAssert( strcmp( DKStringGetCStringPtr( DKSTR( "quick" ) ), "quick" ) == 0 );
    XCTAssert( DKStringGetCStringPtr( DKSTR( "quick" ) ) == "quick" );
    
    range = DKStringGetRangeOfString( quickFox, DKSTR( "The" ), 0 );
    XCTAssert( (range.location == 0) && (range.length == 3) );

    range = DKStringGetRangeOfString( quickFox, DKSTR( "dog." ), 0 );
    XCTAssert( (range.location == 38) && (range.length == 4) );

    range = DKStringGetRangeOfString( quickFox, DKSTR( "polka dots" ), 0 );
    XCTAssert( (range.location == DKNotFound) && (range.length == 0) );

    range = DKStringGetRangeOfString( quickFox, DKSTR( "quick" ), 0 );
    XCTAssert( (range.location == 4) && (range.length == 5) );

    DKStringRef substring = DKStringCopySubstring( quickFox, range );
    XCTAssert( DKStringEqual( DKSTR( "quick" ), substring ) );
    DKRelease( substring );

    DKMutableStringRef mutableString = DKStringMutableCopy( quickFox );
    XCTAssert( DKStringEqual( quickFox, mutableString ) );

    range = DKStringGetRangeOfString( mutableString, DKSTR( "quick " ), 0 );
    XCTAssert( (range.location == 4) && (range.length == 6) );
    
    DKStringReplaceSubstring( mutableString, range, DKSTR( "slow " ) );
    XCTAssert( DKStringEqual( mutableString, DKSTR( "The slow brown fox jumps over a lazy dog." ) ) );

    DKStringReplaceOccurrencesOfString( mutableString, DKSTR( "jumps " ), DKSTR( "leapt " ) );
    XCTAssert( DKStringEqual( mutableString, DKSTR( "The slow brown fox leapt over a lazy dog." ) ) );
    
    DKStringReplaceOccurrencesOfString( mutableString, DKSTR( "lazy dog." ), DKSTR( "cliff." ) );
    XCTAssert( DKStringEqual( mutableString, DKSTR( "The slow brown fox leapt over a cliff." ) ) );

    DKStringRef trimmedString = DKStringByTrimmingWhitespace( DKSTR( " \t\nhello \t\n" ) );
    XCTAssert( DKStringEqual( trimmedString, DKSTR( "hello" ) ) );
    
    DKRelease( mutableString );
}

- (void) testDKStringStream
{
    const char * a = "aaaaaaaaaa";
    const char * b = "bbbbbbbbbb";
    const char * c = "cccccccccc";

    DKMutableStringRef str = DKMutableString();
    
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
}

- (void) testDKStringConcatenation
{
    DKStringRef str = DKSTR( "She sells sea shells by the sea shore" );
    
    DKListRef list = DKStringSplit( str, DKSTR( " " ) );
    XCTAssert( DKListGetCount( list ) == 8 );
    
    DKStringRef cat = DKStringCombine( list, DKSTR( " " ) );
    XCTAssert( DKStringEqual( str, cat ) );
}

- (void) testDKStringPrefixAndSuffix
{
    DKStringRef str = DKSTR( "She sells sea shells by the sea shore" );

    XCTAssert( DKStringHasPrefix( str, DKSTR( "She" ) ) );
    XCTAssert( !DKStringHasPrefix( str, DKSTR( "sea" ) ) );

    XCTAssert( DKStringHasSuffix( str, DKSTR( "shore" ) ) );
    XCTAssert( !DKStringHasSuffix( str, DKSTR( "sea" ) ) );

    XCTAssert( DKStringHasSubstring( str, DKSTR( "shells" ) ) );
    XCTAssert( !DKStringHasSubstring( str, DKSTR( "blueberry" ) ) );
}

- (void) testDKStringPrintf
{
    DKMutableStringRef str = DKMutableString();
    
    DKSPrintf( str, "%d", 100 );
    XCTAssert( DKStringEqual( str, DKSTR( "100" ) ) );

    DKStringSetString( str, DKSTR( "" ) );
    DKSPrintf( str, "%%%s%%", "Hello" );
    XCTAssert( DKStringEqual( str, DKSTR( "%Hello%" ) ) );
    
    DKStringSetString( str, DKSTR( "" ) );
    DKSPrintf( str, "%s", "Hello" );
    XCTAssert( DKStringEqual( str, DKSTR( "Hello" ) ) );

    DKStringSetString( str, DKSTR( "" ) );
    DKSPrintf( str, "%@", DKSTR( "Hello" ) );
    XCTAssert( DKStringEqual( str, DKSTR( "Hello" ) ) );
}

- (void) testDKStringUTF8
{
    DKStringRef fruit = DKSTR( "🍒🍐🍋🍓" );
    XCTAssert( DKStringGetLength( fruit ) == 4 );

    DKStringRef lemon = DKStringCopySubstring( fruit, DKRangeMake( 2, 1 ) );
    XCTAssert( DKStringEqual( lemon, DKSTR( "🍋" ) ) );
    DKRelease( lemon );

}


struct PathTestCase
{
    DKStringRef path;
    DKStringRef result;
};

struct PathAppendTestCase
{
    DKStringRef path1;
    DKStringRef path2;
    DKStringRef result;
};

- (void) testDKStringGetLastPathComponent
{
    struct PathTestCase tests[] =
    {
        { DKSTR( "/path/file.ext" ),    DKSTR( "file.ext" ) },
        { DKSTR( "/path/file" ),        DKSTR( "file" ) },
        { DKSTR( "/path/" ),            DKSTR( "path" ) },
        { DKSTR( "/path" ),             DKSTR( "path" ) },
        { DKSTR( "/" ),                 DKSTR( "" ) },
        { DKSTR( "path" ),              DKSTR( "path" ) },
        { DKSTR( "" ),                  DKSTR( "" ) },
        { NULL, NULL }
    };

    for( int i = 0; tests[i].path != NULL; i++ )
    {
        struct PathTestCase * test = &tests[i];
        
        DKStringRef result = DKStringGetLastPathComponent( test->path );
        XCTAssert( DKStringEqual( result, test->result ) );
    }
}

- (void) testDKStringAppendPathComponent
{
    struct PathAppendTestCase tests[] =
    {
        { DKSTR( "/a" ),    DKSTR( "b" ),       DKSTR( "/a/b" )  },
        { DKSTR( "/a" ),    DKSTR( "/b" ),      DKSTR( "/a/b" )  },
        { DKSTR( "/a" ),    DKSTR( "b/" ),      DKSTR( "/a/b" )  },
        { DKSTR( "/a" ),    DKSTR( "/b/" ),     DKSTR( "/a/b" )  },
        { DKSTR( "/a/" ),   DKSTR( "b" ),       DKSTR( "/a/b" )  },
        { DKSTR( "/a/" ),   DKSTR( "/b" ),      DKSTR( "/a/b" )  },
        { DKSTR( "/a/" ),   DKSTR( "b/" ),      DKSTR( "/a/b" )  },
        { DKSTR( "/a/" ),   DKSTR( "/b/" ),     DKSTR( "/a/b" )  },

        { DKSTR( "a" ),     DKSTR( "b" ),       DKSTR( "a/b" )   },
        { DKSTR( "a" ),     DKSTR( "/b" ),      DKSTR( "a/b" )   },
        { DKSTR( "a" ),     DKSTR( "b/" ),      DKSTR( "a/b" )   },
        { DKSTR( "a" ),     DKSTR( "/b/" ),     DKSTR( "a/b" )   },
        { DKSTR( "a/" ),    DKSTR( "b" ),       DKSTR( "a/b" )   },
        { DKSTR( "a/" ),    DKSTR( "/b" ),      DKSTR( "a/b" )   },
        { DKSTR( "a/" ),    DKSTR( "b/" ),      DKSTR( "a/b" )   },
        { DKSTR( "a/" ),    DKSTR( "/b/" ),     DKSTR( "a/b" )   },

        { DKSTR( "" ),      DKSTR( "b" ),       DKSTR( "b" )     },
        { DKSTR( "" ),      DKSTR( "/b" ),      DKSTR( "/b" )    },
        { DKSTR( "" ),      DKSTR( "b/" ),      DKSTR( "b" )     },
        { DKSTR( "" ),      DKSTR( "/b/" ),     DKSTR( "/b" )    },
        { DKSTR( "/" ),     DKSTR( "b" ),       DKSTR( "/b" )    },
        { DKSTR( "/" ),     DKSTR( "/b" ),      DKSTR( "/b" )    },
        { DKSTR( "/" ),     DKSTR( "b/" ),      DKSTR( "/b" )    },
        { DKSTR( "/" ),     DKSTR( "/b/" ),     DKSTR( "/b" )    },

        { DKSTR( "a" ),     DKSTR( "" ),        DKSTR( "a" )     },
        { DKSTR( "a/" ),    DKSTR( "" ),        DKSTR( "a" )     },
        { DKSTR( "/a" ),    DKSTR( "" ),        DKSTR( "/a" )    },
        { DKSTR( "/a/" ),   DKSTR( "" ),        DKSTR( "/a" )    },
        { DKSTR( "a" ),     DKSTR( "/" ),       DKSTR( "a" )     },
        { DKSTR( "a/" ),    DKSTR( "/" ),       DKSTR( "a" )     },
        { DKSTR( "/a" ),    DKSTR( "/" ),       DKSTR( "/a" )    },
        { DKSTR( "/a/" ),   DKSTR( "/" ),       DKSTR( "/a" )    },
        
        { DKSTR( "" ),      DKSTR( "" ),        DKSTR( "" )      },
        { DKSTR( "/" ),     DKSTR( "" ),        DKSTR( "/" )     },
        { DKSTR( "" ),      DKSTR( "/" ),       DKSTR( "/" )     },
        { DKSTR( "/" ),     DKSTR( "/" ),       DKSTR( "/" )     },
        
        { DKSTR( "//a" ),   DKSTR( "" ),        DKSTR( "/a" )    },
        { DKSTR( "a//" ),   DKSTR( "" ),        DKSTR( "a" )     },
        { DKSTR( "" ),      DKSTR( "//b" ),     DKSTR( "/b" )    },
        { DKSTR( "" ),      DKSTR( "b//" ),     DKSTR( "b" )     },
        { DKSTR( "//a//" ), DKSTR( "//b//" ),   DKSTR( "/a/b" )  },
        { NULL, NULL, NULL }
    };

    for( int i = 0; tests[i].path1 != NULL; i++ )
    {
        struct PathAppendTestCase * test = &tests[i];
        
        DKMutableStringRef result = DKStringMutableCopy( test->path1 );
        DKStringAppendPathComponent( result, test->path2 );
        XCTAssert( DKStringEqual( result, test->result ) );
        DKRelease( result );
    }
}

- (void) testDKStringRemoveLastPathComponent
{
    struct PathTestCase tests[] =
    {
        { DKSTR( "/path/file.ext" ),    DKSTR( "/path" ) },
        { DKSTR( "/path/file" ),        DKSTR( "/path" ) },
        { DKSTR( "/path/" ),            DKSTR( "/" ) },
        { DKSTR( "/path" ),             DKSTR( "/" ) },
        { DKSTR( "/" ),                 DKSTR( "/" ) },
        { DKSTR( "path" ),              DKSTR( "" ) },
        { DKSTR( "" ),                  DKSTR( "" ) },
        { NULL, NULL }
    };

    for( int i = 0; tests[i].path != NULL; i++ )
    {
        struct PathTestCase * test = &tests[i];
        
        DKMutableStringRef result = DKStringMutableCopy( test->path );
        DKStringRemoveLastPathComponent( result );
        XCTAssert( DKStringEqual( result, test->result ) );
        DKRelease( result );
    }
}

- (void) testDKStringHasPathExtension
{
    struct PathTestCase trueTests[] =
    {
        { DKSTR( "file.ext" ),          DKSTR( "ext" ) },
        { DKSTR( "file.foo.ext" ),      DKSTR( "ext" ) },
        { DKSTR( "path.foo/file.ext" ), DKSTR( "ext" ) },
        { DKSTR( "path.foo/file" ),     DKSTR( "" ) },
        { DKSTR( "path.foo/" ),         DKSTR( "" ) },
        { DKSTR( "/" ),                 DKSTR( "" ) },
        { DKSTR( "" ),                  DKSTR( "" ) },
        { NULL, NULL }
    };

    struct PathTestCase falseTests[] =
    {
        { DKSTR( "file.ext" ),          DKSTR( "ext2" ) },
        { DKSTR( "file.foo.ext" ),      DKSTR( "ext2" ) },
        { DKSTR( "path.foo/file.ext" ), DKSTR( "ext2" ) },
        { DKSTR( "path.foo/file" ),     DKSTR( "ext" ) },
        { DKSTR( "path.foo/" ),         DKSTR( "ext" ) },
        { DKSTR( "/" ),                 DKSTR( "ext" ) },
        { DKSTR( "" ),                  DKSTR( "ext" ) },
        { NULL, NULL }
    };
    
    for( int i = 0; trueTests[i].path != NULL; i++ )
    {
        struct PathTestCase * test = &trueTests[i];
        
        bool result = DKStringHasPathExtension( test->path, test->result );
        XCTAssert( result == true );
    }

    for( int i = 0; falseTests[i].path != NULL; i++ )
    {
        struct PathTestCase * test = &falseTests[i];
        
        bool result = DKStringHasPathExtension( test->path, test->result );
        XCTAssert( result == false );
    }
}

- (void) testDKStringGetPathExtension
{
    struct PathTestCase tests[] =
    {
        { DKSTR( "file.ext" ),          DKSTR( "ext" ) },
        { DKSTR( "file.foo.ext" ),      DKSTR( "ext" ) },
        { DKSTR( "path.foo/file.ext" ), DKSTR( "ext" ) },
        { DKSTR( "path.foo/file" ),     DKSTR( "" ) },
        { DKSTR( "path.foo/" ),         DKSTR( "" ) },
        { DKSTR( "/" ),                 DKSTR( "" ) },
        { DKSTR( "" ),                  DKSTR( "" ) },
        { NULL, NULL }
    };
    
    for( int i = 0; tests[i].path != NULL; i++ )
    {
        struct PathTestCase * test = &tests[i];
        
        DKStringRef result = DKStringGetPathExtension( test->path );
        XCTAssert( DKStringEqual( result, test->result ) );
    }
}

- (void) testDKStringAppendPathExtension
{
    struct PathTestCase tests[] =
    {
        { DKSTR( "file" ),          DKSTR( "file.ext" ) },
        { DKSTR( "/dir/file" ),     DKSTR( "/dir/file.ext" ) },
        { DKSTR( "/dir/file.ext" ), DKSTR( "/dir/file.ext.ext" ) },
        { NULL, NULL }
    };

    for( int i = 0; tests[i].path != NULL; i++ )
    {
        struct PathTestCase * test = &tests[i];
        
        DKMutableStringRef result = DKStringMutableCopy( test->path );
        DKStringAppendPathExtension( result, DKSTR( "ext" ) );
        XCTAssert( DKStringEqual( result, test->result ) );
        DKRelease( result );
    }
}

- (void) testDKStringRemovePathExtension
{
    struct PathTestCase tests[] =
    {
        { DKSTR( "/path/file.ext" ),        DKSTR( "/path/file" ) },
        { DKSTR( "/path/file" ),            DKSTR( "/path/file" ) },
        { DKSTR( "/path.ext/file.ext" ),    DKSTR( "/path.ext/file" ) },
        { DKSTR( "/path.ext/file" ),        DKSTR( "/path.ext/file" ) },
        { DKSTR( "/path.ext/" ),            DKSTR( "/path" ) },
        { DKSTR( "/" ),                     DKSTR( "/" ) },
        { DKSTR( "" ),                      DKSTR( "" ) },
        { NULL, NULL }
    };
    
    for( int i = 0; tests[i].path != NULL; i++ )
    {
        struct PathTestCase * test = &tests[i];
        
        DKMutableStringRef result = DKStringMutableCopy( test->path );
        DKStringRemovePathExtension( result );
        XCTAssert( DKStringEqual( result, test->result ) );
        DKRelease( result );
    }
}


@end




