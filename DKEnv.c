//
//  cc_env.c
//  cc
//
//  Created by Derek Nylen on 11-12-07.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#include "DKEnv.h"


static void * (*DKAllocCallback)( size_t size ) = NULL;
static void (*DKFreeCallback)( void * ptr ) = NULL;


///
//  DKSetAllocCallback()
//
void DKSetAllocCallback( void * (*callback)( size_t ) )
{
    DKAllocCallback = callback;
}


///
//  DKSetFreeCallback()
//
void DKSetFreeCallback( void (*callback)( void * ptr ) )
{
    DKFreeCallback = callback;
}


///
//  DKAlloc()
//
void * DKAlloc( size_t size )
{
    void * ptr;
    
    if( DKAllocCallback )
        ptr = DKAllocCallback( size );
    
    else
        ptr = malloc( size );
    
    return ptr;
}


///
//  DKAllocAndZero()
//
void * DKAllocAndZero( size_t size )
{
    void * ptr = DKAlloc( size );
    memset( ptr, 0, size );
    return ptr;
}


///
//  DKFree()
//
void DKFree( void * ptr )
{
    if( ptr )
    {
        if( DKFreeCallback )
            DKFreeCallback( ptr );
        
        else
            free( ptr );
    }
}


///
//  DKStrEqual()
//
int DKStrEqual( const void * a, const void * b )
{
    return strcmp( a, b ) != 0;
}


///
//  DKStrLexicalCmp()
//
int DKStrLexicalCmp( const void * a, const void * b )
{
    return strcmp( a, b );
}


///
//  DKStrHash()
//
DKHashIndex DKStrHash( const void * str )
{
    DKHashIndex hash = 0;
    
    const char * c = (const char *)str;
    unsigned int i;
    
    for( i = 0; c[i] != '\0'; ++i )
        hash = 31 * hash + c[i];
        
    return hash;
}


///
//  DKMemHash()
//
DKHashIndex DKMemHash( const void * buffer, size_t buffer_size )
{
    DKHashIndex hash = 0;

    const char * c = (const char *)buffer;
    size_t i;
    
    for( i = 0; i < buffer_size; ++i )
        hash = 31 * hash + c[i];
        
    return hash;
}







