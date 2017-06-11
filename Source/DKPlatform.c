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
//  _DKPrintfInternal()
//
static int _DKPrintfInternal( int (*callback)( const char *, va_list ), FILE * file, const char * format, ... )
{
    int result = 0;

    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    if( callback )
        result = callback( format, arg_ptr );
    
    else
        result = vfprintf( file, format, arg_ptr );

    va_end( arg_ptr );
    
    return result;
}


///
//  DKPrintf()
//
int _DKPrintf( const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKMutableStringRef tmp = DKNewMutableString();
    DKVSPrintf( tmp, format, arg_ptr );

    va_end( arg_ptr );
    
    int result = _DKPrintfInternal( PrintfCallback, stdout, "%s", DKStringGetCStringPtr( tmp ) );

    DKRelease( tmp );
    
    return result;
}


///
//  DKWarning()
//
void _DKWarning( const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKMutableStringRef tmp = DKNewMutableString();
    DKVSPrintf( tmp, format, arg_ptr );

    va_end( arg_ptr );
    
    _DKPrintfInternal( WarningCallback, stdout, "%s", DKStringGetCStringPtr( tmp ) );

    DKRelease( tmp );
}


///
//  DKError()
//
void _DKError( const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKMutableStringRef tmp = DKNewMutableString();
    DKVSPrintf( tmp, format, arg_ptr );

    va_end( arg_ptr );
    
    _DKPrintfInternal( ErrorCallback, stderr, "%s", DKStringGetCStringPtr( tmp ) );

    DKRelease( tmp );

    assert( 0 );
}


///
//  DKFatalError()
//
void _DKFatalError( const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKMutableStringRef tmp = DKNewMutableString();
    DKVSPrintf( tmp, format, arg_ptr );

    va_end( arg_ptr );
    
    _DKPrintfInternal( FatalErrorCallback, stderr, "%s", DKStringGetCStringPtr( tmp ) );

    DKRelease( tmp );

    assert( 0 );
    abort();
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
    DKRequire( !DKRuntimeIsInitialized() );

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

#if DK_MALLOC_TRACE
    fprintf( stderr, "0x%lx = dk_malloc( %lu ):\n", (uintptr_t)ptr, size );

    void * callstack[128];
    int frameCount = backtrace( callstack, 128 );
    char ** frameStrings = backtrace_symbols( callstack, frameCount );

    if( frameStrings != NULL )
    {
        for( int i = 1; i < frameCount; i++ )
            fprintf( stderr, "    %s\n", frameStrings[i] );

        free( frameStrings );
    }
#endif

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
#if DK_PLATFORM_APPLE || DK_PLATFORM_LINUX
DKUUID dk_uuid_generate( void )
{
    DKAssert( sizeof(DKUUID) == sizeof(uuid_t) );
    
    DKUUID uuid;

    // Note: uuid_t is defined as an array of unsigned chars, thus the weird casting here
    uuid_generate( (void *)&uuid );
    
    return uuid;
}

#elif DK_PLATFORM_ANDROID_NDK
static const uint8_t ascii_to_hex[128] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //   0 - 15
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  16 - 31
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  32 - 47
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0, //  48 - 63

    0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  64 - 79
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  80 - 95
    0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  96 - 111
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 112 - 127
};

static uint8_t dk_uuid_readbyte( const char * ascii )
{
    uint8_t n0 = ascii_to_hex[ascii[0] & 0x7F];
    uint8_t n1 = ascii_to_hex[ascii[1] & 0x7F];
    return (n0 << 4) | n1;
}

DKUUID dk_uuid_generate( void )
{
    DKUUID uuid;
    char buffer[40];
    
    FILE * stream = fopen( "/proc/sys/kernel/random/uuid", "r" );

    int n = fread( buffer, 1, 36, stream );
    buffer[36] = '\0';

    fclose( stream );
    
    if( n == 36 )
    {
        buffer[36] = '\0';
        
        // 'buffer' should contain a UUID string: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
        
        uuid.bytes[0] = dk_uuid_readbyte( &buffer[0] );
        uuid.bytes[1] = dk_uuid_readbyte( &buffer[2] );
        uuid.bytes[2] = dk_uuid_readbyte( &buffer[4] );
        uuid.bytes[3] = dk_uuid_readbyte( &buffer[6] );

        uuid.bytes[4] = dk_uuid_readbyte( &buffer[9] );
        uuid.bytes[5] = dk_uuid_readbyte( &buffer[11] );
        
        uuid.bytes[6] = dk_uuid_readbyte( &buffer[14] );
        uuid.bytes[7] = dk_uuid_readbyte( &buffer[16] );

        uuid.bytes[8] = dk_uuid_readbyte( &buffer[19] );
        uuid.bytes[9] = dk_uuid_readbyte( &buffer[21] );

        uuid.bytes[10] = dk_uuid_readbyte( &buffer[24] );
        uuid.bytes[11] = dk_uuid_readbyte( &buffer[26] );
        uuid.bytes[12] = dk_uuid_readbyte( &buffer[28] );
        uuid.bytes[13] = dk_uuid_readbyte( &buffer[30] );
        uuid.bytes[14] = dk_uuid_readbyte( &buffer[32] );
        uuid.bytes[15] = dk_uuid_readbyte( &buffer[34] );
    }
    
    return uuid;
}

#else

#error dk_uuid_generate() is not defined for the current platform

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
//  dk_datetime()
//
#if DK_PLATFORM_POSIX
DKDateTime dk_datetime( void )
{
    struct timeval t;
    
    if( gettimeofday( &t, NULL ) )
        return 0.0;
    
    return ((DKDateTime)t.tv_sec) - DKAbsoluteTimeSince1970 + (((DKDateTime)t.tv_usec) * 1.0e-6);
}

#else

#error dk_datetime() is not defined for the current platform

#endif






