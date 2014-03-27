//
//  main.c
//  scl_test
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Duck.h"


#define VERIFY( x )     if( !(x) ) printf( "FAILED: %s\n", #x )


void TestDKObject( void );
void TestDKData( void );
void TestDKList( DKTypeRef listClass );
void TestDKDictionary( DKTypeRef dictionaryClass );
void TestDKListPerformance( DKTypeRef listClass );


int main( int argc, const char * argv[] )
{
    printf( "Testing the Duck Object Library (libDuck)\n" );

    TestDKObject();
    TestDKData();
    TestDKList( DKMutableLinkedListClass() );
    TestDKList( DKMutableArrayClass() );
    TestDKDictionary( DKMutableBinaryTreeClass() );
    
#if 0
    
    printf( "\nTesting DKLinkedList Performance vs. a C Array\n" );
    TestDKListPerformance( DKMutableLinkedListClass() );

    printf( "\nTesting DKArray Performance vs. a C Array\n" );
    TestDKListPerformance( DKMutableArrayClass() );
    
#endif

    return 0;
}



// TestDKObject ==========================================================================

DKDeclareMessage( Square, int, int * );
DKDefineMessage( Square, int, int * );

DKDeclareMessage( Cube, int, int * );
DKDefineMessage( Cube, int, int * );

static DKTypeRef TestClass = NULL;

static void TestSquare( DKTypeRef ref, DKSEL sel, int x, int * y )
{
    *y = x * x;
}

static void TestCube( DKTypeRef ref, DKSEL sel, int x, int * y )
{
    *y = x * x * x;
}

void TestDKObject( void )
{
    // Define a sample class
    TestClass = DKCreateClass( "Test", DKObjectClass(), sizeof(struct DKObjectHeader) );
    
    // Install some message handler
    DKInstallMsgHandler( TestClass, DKSelector(Square), TestSquare );
    DKInstallMsgHandler( TestClass, DKSelector(Cube), TestCube );
    
    // Create an instance of the object
    DKTypeRef object = DKCreate( TestClass );
    
    // Test class membership
    VERIFY( DKGetClass( TestClass ) == DKClassClass() );
    VERIFY( DKGetClass( object ) == TestClass );

    VERIFY( DKIsKindOfClass( object, TestClass ) );
    VERIFY( DKIsKindOfClass( object, DKObjectClass() ) );

    VERIFY( DKIsMemberOfClass( object, TestClass ) );
    VERIFY( !DKIsMemberOfClass( object, DKObjectClass() ) );
    
    // DKQueryInterface should return the same object when called on the class or an instance of the class
    VERIFY( DKGetInterface( TestClass, DKSelector(LifeCycle) ) == DKGetInterface( object, DKSelector(LifeCycle) ) );

    // Try calling our custom message handlers
    int y;
    
    DKSendMsg( object, Square, 2, &y );
    VERIFY( y == 4 );

    DKSendMsg( object, Cube, 2, &y );
    VERIFY( y == 8 );

    // Cleanup
    DKRelease( object );
    DKRelease( TestClass );
    TestClass = NULL;
}




// TestDKData ============================================================================
void TestDKData( void )
{
    DKMutableDataRef data = DKDataCreateMutable();
    
    const char * a = "aaaaaaaa";
    const char * b = "bbbbbbbb";
    const char * c = "cccccccc";
    const char * d = "dddddddd";
    const char * e = "eeeeeeee";
    
    DKDataAppendBytes( data, NULL, 0 );
    
    DKDataAppendBytes( data, a, 10 );
    DKDataAppendBytes( data, b, 10 );
    DKDataAppendBytes( data, c, 10 );
    DKDataAppendBytes( data, d, 10 );

    VERIFY( DKDataGetLength( data ) == 40 );
    
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 0, 10 ) ), a ) == 0 );
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 10, 10 ) ), b ) == 0 );
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 20, 10 ) ), c ) == 0 );
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 30, 10 ) ), d ) == 0 );

    DKDataRef copy = DKCopy( data );
    VERIFY( DKDataGetLength( copy ) == 40 );
    DKRelease( copy );

    DKDataReplaceBytes( data, DKRangeMake( 0, 0 ), e, 10 );
    VERIFY( DKDataGetLength( data ) == 50 );
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 0, 10 ) ), e ) == 0 );
    
    DKDataReplaceBytes( data, DKRangeMake( 0, 10 ), b, 10 );
    VERIFY( DKDataGetLength( data ) == 50 );
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 0, 10 ) ), b ) == 0 );
    
    DKDataDeleteBytes( data, DKRangeMake( 0, 10 ) );
    VERIFY( DKDataGetLength( data ) == 40 );
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 0, 10 ) ), a ) == 0 );

    DKDataReplaceBytes( data, DKRangeMake( 10, 0 ), e, 10 );
    VERIFY( DKDataGetLength( data ) == 50 );
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 10, 10 ) ), e ) == 0 );

    DKDataDeleteBytes( data, DKRangeMake( 10, 10 ) );
    VERIFY( DKDataGetLength( data ) == 40 );
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 10, 10 ) ), b ) == 0 );

    DKDataAppendBytes( data, e, 10 );
    VERIFY( DKDataGetLength( data ) == 50 );
    VERIFY( strcmp( (const char *)DKDataGetByteRange( data, DKRangeMake( 40, 10 ) ), e ) == 0 );

    DKDataDeleteBytes( data, DKRangeMake( 40, 10 ) );
    VERIFY( DKDataGetLength( data ) == 40 );
    VERIFY( DKDataGetByteRange( data, DKRangeMake( 40, 10 ) ) == NULL );
    
    DKDataDeleteBytes( data, DKRangeMake( 0, DKDataGetLength( data ) ) );
    
    DKRelease( data );
}




// TestDKList ============================================================================
void TestDKList( DKTypeRef listClass )
{
    DKDataRef a = DKDataCreate( "a", 2 );
    DKDataRef b = DKDataCreate( "b", 2 );
    DKDataRef c = DKDataCreate( "c", 2 );
    DKDataRef d = DKDataCreate( "d", 2 );
    
    DKMutableListRef list = (DKMutableListRef)DKCreate( listClass );
    
    // Append
    DKListAppendObject( list, a );
    DKListAppendObject( list, b );
    DKListAppendObject( list, c );
    DKListAppendObject( list, d );
    
    VERIFY( DKListGetCount( list ) == 4 );
    
    VERIFY( DKListGetFirstIndexOfObject( list, a ) == 0 );
    VERIFY( DKListGetFirstIndexOfObject( list, b ) == 1 );
    VERIFY( DKListGetFirstIndexOfObject( list, c ) == 2 );
    VERIFY( DKListGetFirstIndexOfObject( list, d ) == 3 );

    VERIFY( DKListGetLastIndexOfObject( list, a ) == 0 );
    VERIFY( DKListGetLastIndexOfObject( list, b ) == 1 );
    VERIFY( DKListGetLastIndexOfObject( list, c ) == 2 );
    VERIFY( DKListGetLastIndexOfObject( list, d ) == 3 );
    
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( list, 0 ) ), "a" ) == 0 );
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( list, 1 ) ), "b" ) == 0 );
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( list, 2 ) ), "c" ) == 0 );
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( list, 3 ) ), "d" ) == 0 );
    
    DKListRemoveAllObjects( list );
    
    // Insert
    DKListInsertObjectAtIndex( list, 0, a );
    DKListInsertObjectAtIndex( list, 0, b );
    DKListInsertObjectAtIndex( list, 0, c );
    DKListInsertObjectAtIndex( list, 0, d );

    VERIFY( DKListGetCount( list ) == 4 );

    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( list, 0 ) ), "d" ) == 0 );
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( list, 1 ) ), "c" ) == 0 );
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( list, 2 ) ), "b" ) == 0 );
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( list, 3 ) ), "a" ) == 0 );

    DKListRemoveAllObjects( list );

    // Copy
    DKListAppendObject( list, a );
    DKListAppendObject( list, b );
    DKListAppendObject( list, c );
    DKListAppendObject( list, d );
    
    VERIFY( DKListGetCount( list ) == 4 );
    
    DKTypeRef copy = DKCopy( list );
    
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( copy, 0 ) ), "a" ) == 0 );
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( copy, 1 ) ), "b" ) == 0 );
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( copy, 2 ) ), "c" ) == 0 );
    VERIFY( strcmp( DKDataGetBytePtr( DKListGetObjectAtIndex( copy, 3 ) ), "d" ) == 0 );
    
    DKListRemoveAllObjects( list );
    DKRelease( copy );
    
    DKRelease( list );
    
    DKRelease( a );
    DKRelease( b );
    DKRelease( c );
    DKRelease( d );
}




// TestDKDictionary ======================================================================
void TestDKDictionary( DKTypeRef dictionaryClass )
{
    const int N = 100;
    
    DKTypeRef keys[N];
    DKTypeRef values[N];

    DKMutableDictionaryRef dict = (DKMutableDictionaryRef)DKCreate( dictionaryClass );
    
    for( int i = 0; i < N; i++ )
    {
        char buffer[32];
        
        sprintf( buffer, "Key%d", i );
        keys[i] = DKDataCreate( buffer, strlen( buffer) + 1 );

        sprintf( buffer, "Value%d", i );
        values[i] = DKDataCreate( buffer, strlen( buffer) + 1 );

        DKDictionarySetObject( dict, keys[i], values[i] );
    }
    
    VERIFY( DKDictionaryGetCount( dict ) == N );

    for( int i = 0; i < N; i++ )
    {
        DKTypeRef value = DKDictionaryGetObject( dict, keys[i] );
        VERIFY( value == values[i] );
    }
    
    DKDictionaryRemoveAllObjects( dict );
    VERIFY( DKDictionaryGetCount( dict ) == 0 );

    for( int i = 0; i < N; i++ )
    {
        DKRelease( keys[i] );
        DKRelease( values[i] );
    }
    
    DKRelease( dict );
}




// TestDKListPerformance =================================================================
void TestDKListPerformance( DKTypeRef listClass )
{
    const int N = 10000000;

    DKDataRef foo = DKDataCreate( "foo", 4 );
    DKDataRef bar = NULL;
    
    // Array Setup
    DKPointerArray array;
    DKPointerArrayInit( &array );
    
    clock_t arraySetupStart = clock();

    for( int i = 0; i < N; ++i )
    {
        DKRetain( foo );
        DKPointerArrayAppendPointer( &array, foo );
    }
    
    clock_t arraySetupEnd = clock();

    double arraySetupTime = (double)(arraySetupEnd - arraySetupStart) / (double)CLOCKS_PER_SEC;
    printf( "C Array Write: %lf\n", arraySetupTime );

    // Array Access
    clock_t arrayAccessStart = clock();
    
    for( int i = 0; i < N; ++i )
        bar = array.data[i];
    
    clock_t arrayAccessEnd = clock();

    double arrayAccessTime = (double)(arrayAccessEnd - arrayAccessStart) / (double)CLOCKS_PER_SEC;
    printf( "C Array Read:  %lf\n", arrayAccessTime );


    // List Setup
    DKMutableListRef list = (DKMutableListRef)DKCreate( listClass );

    clock_t listSetupStart = clock();
    
    for( int i = 0; i < N; ++i )
        DKListAppendObject( list, foo );
    
    clock_t listSetupEnd = clock();

    double listSetupTime = (double)(listSetupEnd - listSetupStart) / (double)CLOCKS_PER_SEC;
    printf( "DKList Write:  %lf\n", listSetupTime );
    
    
    // List Access
    clock_t listAccessStart = clock();
    
    for( int i = 0; i < N; ++i )
        bar = DKListGetObjectAtIndex( list, i );
    
    clock_t listAccessEnd = clock();
    
    double listAccessTime = (double)(listAccessEnd - listAccessStart) / (double)CLOCKS_PER_SEC;
    printf( "DKList Read:   %lf\n", listAccessTime );

    
    // Cleanup
    DKPointerArrayFinalize( &array );
    DKRelease( list );

    for( int i = 0; i < N; ++i )
        DKRelease( foo );
    
    DKRelease( foo );
}







