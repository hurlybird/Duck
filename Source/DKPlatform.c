/*****************************************************************************************

  DKPlatform.c

  Copyright (c) 2014 Derek W. Nylen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*****************************************************************************************/

#include <assert.h>

#include "DKPlatform.h"
#include "DKString.h"
#include "DKStream.h"


// Error Reporting =======================================================================
static int (*PrintfCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*WarningCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*ErrorCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*FatalErrorCallback)( const char * format, va_list arg_ptr ) = NULL;

void DKSetPrintfCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
    PrintfCallback = callback;
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
//  DKPrintf()
//
static int _DKPrintfInternal( const char * format, ... )
{
    int result = 0;

    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    if( PrintfCallback )
        result = PrintfCallback( format, arg_ptr );
    
    else
        result = vprintf( format, arg_ptr );

    va_end( arg_ptr );
    
    return result;
}

int _DKPrintf( const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKMutableStringRef tmp = DKStringCreateMutable();
    DKVSPrintf( tmp, format, arg_ptr );

    va_end( arg_ptr );
    
    int result = _DKPrintfInternal( "%s", DKStringGetCStringPtr( tmp ) );

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
        assert( 0 );
        abort();
    }
    
    va_end( arg_ptr );
    
    return 0;
}


///
//  DKImmutableObjectAccessError()
//
void DKImmutableObjectAccessError( DKObjectRef _self )
{
    DKCheckMutable( _self );
}




// Memory Allocation =====================================================================
static dk_malloc_callback malloc_callback = NULL;
static dk_free_callback free_callback = NULL;


///
//  DKSetExternalAllocator()
//
void DKSetExternalAllocator( dk_malloc_callback _malloc, dk_free_callback _free )
{
    DKFatal( !DKRuntimeIsInitialized() );

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
//  dk_uuid_generate()
//
#if DK_PLATFORM_POSIX
#include <uuid/uuid.h>

DKUUID dk_uuid_generate( void )
{
    DKAssert( sizeof(DKUUID) == sizeof(uuid_t) );
    
    DKUUID uuid;

    // Note: uuid_t is defined as an array of unsigned chars, thus the weird casting here
    uuid_generate( (void *)&uuid );
    
    return uuid;
}
#endif


///
//  dk_strhash()
//
uint32_t dk_strhash32( const char * str )
{
    uint32_t hash = 0;
    
    const char * c = str;
    unsigned int i;
    
    for( i = 0; c[i] != '\0'; ++i )
        hash = 31 * hash + c[i];
        
    return hash;
}

uint64_t dk_strhash64( const char * str )
{
    uint64_t hash = 0;
    
    const char * c = str;
    unsigned int i;
    
    for( i = 0; c[i] != '\0'; ++i )
        hash = 31 * hash + c[i];
        
    return hash;
}


///
//  dk_memhash()
//
uint32_t dk_memhash32( const void * buffer, size_t buffer_size )
{
    uint32_t hash = 0;

    const char * c = (const char *)buffer;
    size_t i;
    
    for( i = 0; i < buffer_size; ++i )
        hash = 31 * hash + c[i];
        
    return hash;
}

uint64_t dk_memhash64( const void * buffer, size_t buffer_size )
{
    uint64_t hash = 0;

    const char * c = (const char *)buffer;
    size_t i;
    
    for( i = 0; i < buffer_size; ++i )
        hash = 31 * hash + c[i];
        
    return hash;
}


///
//  dk_time()
//
const DKDateTime DKAbsoluteTimeSince1970 = 978307200.0L;    // POSIX reference time
const DKDateTime DKAbsoluteTimeSince2001 = 0.0L;            // CoreFoundation reference time

#if DK_PLATFORM_POSIX
#include <sys/time.h>

DKDateTime dk_datetime( void )
{
    struct timeval t;
    
    if( gettimeofday( &t, NULL ) )
        return 0.0;
    
    return ((DKDateTime)t.tv_sec) - DKAbsoluteTimeSince1970 + (((DKDateTime)t.tv_usec) * 1.0e-6);
}
#endif






