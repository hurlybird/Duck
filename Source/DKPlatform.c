//
//  cc_env.c
//  cc
//
//  Created by Derek Nylen on 11-12-07.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#include <assert.h>

#include "DKPlatform.h"
#include "DKString.h"
#include "DKStream.h"


// Error Reporting =======================================================================
static int (*DebugCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*WarningCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*ErrorCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*FatalErrorCallback)( const char * format, va_list arg_ptr ) = NULL;

void DKSetDebugCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
    DebugCallback = callback;
}

void DKSetWarningCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
    WarningCallback = callback;
}

void DKSetErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
    ErrorCallback = callback;
}

void DKSetFatalErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
    FatalErrorCallback = callback;
}


///
//  DKDebug()
//
static int _DKDebugInternal( const char * format, ... )
{
    int result = 0;

    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    if( DebugCallback )
        result = DebugCallback( format, arg_ptr );
    
    else
        result = vprintf( format, arg_ptr );

    va_end( arg_ptr );
    
    return result;
}

int _DKDebug( const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKMutableStringRef tmp = DKStringCreateMutable();
    DKVSPrintf( tmp, format, arg_ptr );

    va_end( arg_ptr );
    
    int result = _DKDebugInternal( "%s", DKStringGetCStringPtr( tmp ) );

    DKRelease( tmp );
    
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
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    if( ErrorCallback )
    {
        ErrorCallback( format, arg_ptr );
    }
    
    else
    {
        vfprintf( stderr, format, arg_ptr );
        assert( 0 );
    }

    va_end( arg_ptr );

    return 0;
}


///
//  DKFatalError()
//
int _DKFatalError( const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    if( FatalErrorCallback )
    {
        FatalErrorCallback( format, arg_ptr );
    }
    
    else
    {
        vfprintf( stderr, format, arg_ptr );
    }
    
    va_end( arg_ptr );

    assert( 0 );
    abort();
    
    return 0;
}




// Memory Allocation =====================================================================
static dk_malloc_callback malloc_callback = NULL;
static dk_free_callback free_callback = NULL;


///
//  DKSetExternalAllocator()
//
void DKSetExternalAllocator( dk_malloc_callback _malloc, dk_free_callback _free )
{
    malloc_callback = _malloc;
    free_callback = _free;
}


///
//  dk_malloc()
//
void * dk_malloc( size_t size )
{
    void * ptr;
    
    if( malloc_callback )
        ptr = malloc_callback( size );
    
    else
        ptr = malloc( size );
    
    return ptr;
}


///
//  dk_free()
//
void dk_free( void * ptr )
{
    if( ptr )
    {
        if( free_callback )
            free_callback( ptr );
        
        else
            free( ptr );
    }
}




// Other Utilities =======================================================================

///
//  dk_strhash()
//
DKHashCode dk_strhash( const char * str )
{
    DKHashCode hash = 0;
    
    const char * c = str;
    unsigned int i;
    
    for( i = 0; c[i] != '\0'; ++i )
        hash = 31 * hash + c[i];
        
    return hash;
}


///
//  dk_memhash()
//
DKHashCode dk_memhash( const void * buffer, size_t buffer_size )
{
    DKHashCode hash = 0;

    const char * c = (const char *)buffer;
    size_t i;
    
    for( i = 0; i < buffer_size; ++i )
        hash = 31 * hash + c[i];
        
    return hash;
}


///
//  dk_time()
//
#if DK_PLATFORM_POSIX
#include <sys/time.h>

double dk_time( void )
{
    struct timeval t;
    
    if( gettimeofday( &t, NULL ) )
        return 0.0;
    
    return (double)t.tv_sec + ((double)t.tv_usec / 1000000.0);
}
#endif






