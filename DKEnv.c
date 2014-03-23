//
//  cc_env.c
//  cc
//
//  Created by Derek Nylen on 11-12-07.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#include <assert.h>

#include "DKEnv.h"


// Error Reporting =======================================================================
static int (*PrintCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*WarningCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*ErrorCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*FatalErrorCallback)( const char * format, va_list arg_ptr ) = NULL;

void DKSetPrintCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
}

void DKSetWarningCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
}

void DKSetErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
}

void DKSetFatalErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
}


///
//  DKPrintf()
//
int DKPrintf( const char * format, ... )
{
    int result = 0;

    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    if( PrintCallback )
        result = PrintCallback( format, arg_ptr );
    
    else
        result = vprintf( format, arg_ptr );
    
    va_end( arg_ptr );
    
    return result;
}


///
//  DKWarning()
//
int _DKWarning( const char * format, ... )
{
    int result = 0;

    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    if( WarningCallback )
        result = WarningCallback( format, arg_ptr );
    
    else
        result = vprintf( format, arg_ptr );
    
    va_end( arg_ptr );
    
    return result;
}


///
//  DKError()
//
int _DKError( const char * format, ... )
{
    int result = 0;

    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    if( ErrorCallback )
        result = ErrorCallback( format, arg_ptr );
    
    else
        result = vfprintf( stderr, format, arg_ptr );

    va_end( arg_ptr );

    assert( 0 );

    return result;
}


///
//  DKFatalError()
//
int _DKFatalError( const char * format, ... )
{
    int result = 0;

    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    if( FatalErrorCallback )
    {
        result = FatalErrorCallback( format, arg_ptr );
    }
    
    else
    {
        result = vfprintf( stderr, format, arg_ptr );
    }
    
    va_end( arg_ptr );

    assert( 0 );
    abort();
    
    return result;
}




// Memory Allocation =====================================================================
static void * (*DKAllocCallback)( size_t size ) = NULL;
static void (*DKFreeCallback)( void * ptr ) = NULL;

void DKSetAllocCallback( void * (*callback)( size_t ) )
{
    DKAllocCallback = callback;
}

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




// Other Utilities =======================================================================

///
//  DKStrHash()
//
DKHashIndex DKStrHash( const char * str )
{
    DKHashIndex hash = 0;
    
    const char * c = str;
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







