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
int _DKDebug( const char * format, ... )
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
//  dk_shuffle()
//
void dk_shuffle( uintptr_t array[], DKIndex count )
{
    if( count > 1 )
    {
        for( DKIndex i = 0; i < count - 1; ++i )
        {
            DKIndex j = rand() % (count - i);
            
            uintptr_t tmp = array[i];
            array[i] = array[j];
            array[j] = tmp;
        }
    }
}


///
//  dk_ustrchr()
//
const char * dk_ustrchr( const char * str, int ch )
{
    const char * cur = str;

    while( *cur != '\0' )
    {
        char32_t _ch;
        size_t n = dk_ustrscan( cur, &_ch );

        if( _ch == ch )
            return cur;

        cur += n;
    }

    return NULL;
}


///
//  dk_ustrrchr()
//
const char * dk_ustrrchr( const char * str, int ch )
{
    return strrchr( str, ch );
}


///
//  dk_ustrcmp()
//
int dk_ustrcmp( const char * str1, const char * str2, int options )
{
    return strcmp( str1, str2 );
}


///
//  dk_ustrlen()
//
size_t dk_ustrlen( const char * str )
{
    size_t length = 0;
    const char * cur = str;

    while( *cur != '\0' )
    {
        char32_t ch;
        cur += dk_ustrscan( cur, &ch );

        length++;
    }

    return length;
}


///
//  dk_ustridx()
//
const char * dk_ustridx( const char * str, size_t idx )
{
    const char * cur = str;
    
    for( size_t i = 0; i < idx; ++i )
    {
        char32_t ch;
        cur += dk_ustrscan( cur, &ch );
        
        if( ch == '\0' )
            return NULL;
    }
    
    return cur;
}


///
//  dk_ustrscan()
//
size_t dk_ustrscan( const char * str, char32_t * ch )
{
    if( *str == '\0' )
        return 0;
    
    *ch = str[0];
    return 1;
}





