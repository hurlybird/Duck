//
//  main.c
//  scl_test
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include "Duck.h"


#define VERIFY( x )     if( !(x) ) printf( "FAILED: %s\n", #x )


void TestDKObject( void );
void TestDKData( void );
void TestDKList( DKTypeRef listClass );


int main( int argc, const char * argv[] )
{
    printf( "Testing the Duck Object Library (libDuck)\n" );

    TestDKObject();
    TestDKData();
    TestDKList( DKMutableLinkedListClass() );

    return 0;
}



// TestDKObject ==========================================================================

DKDeclareMethod( int, Square, int );
DKDefineMethod( int, Square, int );

DKDeclareMethod( int, Cube, int );
DKDefineMethod( int, Cube, int );

static DKTypeRef TestClass = NULL;

static DKTypeRef TestObjectAllocate( void )
{
    return DKAllocObject( TestClass, sizeof(DKObjectHeader), 0 );
}

static int TestSquare( DKTypeRef ref, DKSEL sel, int x )
{
    return x * x;
}

static int TestCube( DKTypeRef ref, DKSEL sel, int x )
{
    return x * x * x;
}

void TestDKObject( void )
{
    // Define a sample class
    TestClass = DKAllocClass( DKObjectClass() );
    
    // Add a custom life-cycle interface
    struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
    lifeCycle->allocate = TestObjectAllocate;
    lifeCycle->initialize = DKDefaultInitializeImp;
    lifeCycle->finalize = DKDefaultFinalizeImp;
    
    DKInstallInterface( TestClass, lifeCycle );
    DKRelease( lifeCycle );
    
    // Install some custom methods
    DKInstallMethod( TestClass, DKSelector(Square), TestSquare );
    DKInstallMethod( TestClass, DKSelector(Cube), TestCube );
    
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
    VERIFY( DKLookupInterface( TestClass, DKSelector(LifeCycle) ) == DKLookupInterface( object, DKSelector(LifeCycle) ) );

    // Try calling our custom methods
    VERIFY( DKCallMethod( object, Square, 2 ) == 4 );
    VERIFY( DKCallMethod( object, Cube, 2 ) == 8 );

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
    
    //DKIndex n = DKCallMethod( list, Count );
    
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









