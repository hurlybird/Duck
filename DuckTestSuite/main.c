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
    printf( "Testing Component/Container Library (libcc)\n" );

    TestDKObject();
    TestDKData();
    TestDKList( DKMutableLinkedListClass() );

    return 0;
}


void TestDKObject( void )
{
    DKTypeRef objectClass = DKObjectClass();
    DKTypeRef object = DKCreate( objectClass );
    
    VERIFY( DKGetClass( objectClass ) == &__DKClassClass__ );
    VERIFY( DKGetClass( object ) == objectClass );
    VERIFY( DKGetTypeID( objectClass ) == DKClassTypeID );
    VERIFY( DKGetTypeID( object ) == DKObjectTypeID );
    VERIFY( DKGetClass( object ) == DKGetInterface( object, DKObjectInterfaceID ) );
    VERIFY( DKGetInterface( objectClass, DKObjectInterfaceID ) == DKGetInterface( object, DKObjectInterfaceID ) );

    DKRelease( object );
}


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


void TestDKList( DKTypeRef listClass )
{
    DKDataRef a = DKDataCreate( "a", 2 );
    DKDataRef b = DKDataCreate( "b", 2 );
    DKDataRef c = DKDataCreate( "c", 2 );
    DKDataRef d = DKDataCreate( "d", 2 );
    
    DKMutableListRef list = (DKMutableListRef)DKCreate( listClass );
    
    DKListAppendValue( list, a );
    DKListAppendValue( list, b );
    DKListAppendValue( list, c );
    DKListAppendValue( list, d );
    
    DKRelease( list );
    
    DKRelease( a );
    DKRelease( b );
    DKRelease( c );
    DKRelease( d );
}









