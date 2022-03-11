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
static int (*LogCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*WarningCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*ErrorCallback)( const char * format, va_list arg_ptr ) = NULL;
static int (*FatalErrorCallback)( const char * format, va_list arg_ptr ) = NULL;

static const char * LogPrefix = "";
static const char * LogSuffix = "\n";
static const char * WarningPrefix = "WARNING: ";
static const char * WarningSuffix = "\n";
static const char * ErrorPrefix = "ERROR: ";
static const char * ErrorSuffix = "\n";
static const char * FatalErrorPrefix = "FATAL ERROR: ";
static const char * FatalErrorSuffix = "\n";

static bool AbortOnErrors = DK_DEFAULT_ABORT_ON_ERRORS;

void DKSetPrintfCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
    PrintfCallback = callback;
}

void DKSetLogCallback( int (*callback)( const char * format, va_list arg_ptr ) )
{
    LogCallback = callback;
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

void DKSetLogFormat( const char * prefix, const char * suffix )
{
    LogPrefix = prefix;
    LogSuffix = suffix;
}

void DKSetWarningFormat( const char * prefix, const char * suffix )
{
    WarningPrefix = prefix;
    WarningSuffix = suffix;
}

void DKSetErrorFormat( const char * prefix, const char * suffix )
{
    ErrorPrefix = prefix;
    ErrorSuffix = suffix;
}

void DKSetFatalErrorFormat( const char * prefix, const char * suffix )
{
    FatalErrorPrefix = prefix;
    FatalErrorSuffix = suffix;
}

void DKSetAbortOnErrors( bool flag )
{
    AbortOnErrors = flag;
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
    {
        result = callback( format, arg_ptr );
    }
    
    else
    {
#if DK_PLATFORM_WINDOWS || DK_PLATFORM_SCARLETT
        if( IsDebuggerPresent() )
        {
            char buffer[1024];
            vsnprintf( buffer, sizeof(buffer), format, arg_ptr );

            OutputDebugStringA( buffer );
        }
#endif

        result = vfprintf( file, format, arg_ptr );
    }

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
//  DKLog()
//
void _DKLog( const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKMutableStringRef tmp = DKNewMutableString();
    DKVSPrintf( tmp, format, arg_ptr );

    va_end( arg_ptr );
    
    _DKPrintfInternal( LogCallback, stdout, "%s%s%s", LogPrefix, DKStringGetCStringPtr( tmp ), LogSuffix );

    DKRelease( tmp );
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
    
    _DKPrintfInternal( WarningCallback, stdout, "%s%s%s", WarningPrefix, DKStringGetCStringPtr( tmp ), WarningSuffix );

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
    
    _DKPrintfInternal( ErrorCallback, stderr, "%s%s%s", ErrorPrefix, DKStringGetCStringPtr( tmp ), ErrorSuffix );

    DKRelease( tmp );

    if( AbortOnErrors )
    {
        #if DEBUG
        dk_breakhere();
        #endif

        abort();
    }
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
    
    _DKPrintfInternal( FatalErrorCallback, stderr, "%s%s%s", FatalErrorPrefix, DKStringGetCStringPtr( tmp ), FatalErrorSuffix );

    DKRelease( tmp );

    #if DEBUG
    dk_breakhere();
    #endif

    abort();
}


///
//  DKFailedAssert()
//
void _DKFailedAssert( const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKMutableStringRef tmp = DKNewMutableString();
    DKVSPrintf( tmp, format, arg_ptr );

    va_end( arg_ptr );
    
    _DKPrintfInternal( PrintfCallback, stdout, "%s", DKStringGetCStringPtr( tmp ) );

    DKRelease( tmp );
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
static dk_calloc_callback calloc_callback = NULL;
static dk_realloc_callback realloc_callback = NULL;
static dk_free_callback free_callback = NULL;


///
//  DKSetExternalAllocator()
//
DK_API void DKSetExternalAllocator( dk_malloc_callback _malloc,
    dk_realloc_callback _realloc,
    dk_calloc_callback _calloc,
    dk_free_callback _free )
{
    DKRequire( !DKRuntimeIsInitialized() );

    malloc_callback = _malloc;
    calloc_callback = _calloc;
    realloc_callback = _realloc;
    free_callback = _free;
}


///
//  dk_print_backtrace()
//
#if DK_MALLOC_TRACE
#include <execinfo.h>

static void dk_print_backtrace( FILE * stream )
{
    void * callstack[256];
    int frameCount = backtrace( callstack, 256 );
    char ** frameStrings = backtrace_symbols( callstack, frameCount );

    if( frameStrings != NULL )
    {
        for( int i = 1; i < frameCount; i++ )
            fprintf( stream, "    %s\n", frameStrings[i] );

        free( frameStrings );
    }
}
#endif


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
    dk_print_backtrace( stderr );
#endif

    return ptr;
}


///
//  dk_calloc()
//
void * dk_calloc( size_t num, size_t size )
{
    void * ptr;
    
    if( calloc_callback )
        ptr = calloc_callback( num, size );
    
    else
        ptr = calloc( num, size );

#if DK_MALLOC_TRACE
    fprintf( stderr, "0x%lx = dk_calloc( %lu, %lu ):\n", (uintptr_t)ptr, num, size );
    dk_print_backtrace( stderr );
#endif

    return ptr;
}


///
//  dk_realloc()
//
void * dk_realloc( void * ptr, size_t size )
{
#if DK_MALLOC_TRACE
    void * old_ptr = ptr;
#endif
    
    if( realloc_callback )
        ptr = realloc_callback( ptr, size );
    
    else
        ptr = realloc( ptr, size );

#if DK_MALLOC_TRACE
    fprintf( stderr, "0x%lx = dk_realloc( 0x%lx, %lu ):\n", (uintptr_t)ptr, (uintptr_t)old_ptr, size );
    dk_print_backtrace( stderr );
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
#if DK_PLATFORM_APPLE || DK_PLATFORM_LINUX || DK_PLATFORM_UNIX
DKUUID dk_uuid_generate( void )
{
    DKAssert( sizeof(DKUUID) == sizeof(uuid_t) );
    
    DKUUID uuid;

    // Note: uuid_t is defined as an array of unsigned chars, thus the weird casting here
    uuid_generate( (void *)&uuid );
    
    return uuid;
}

#elif DK_PLATFORM_ANDROID
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

#elif DK_PLATFORM_WINDOWS
#include <rpc.h>

DKUUID dk_uuid_generate( void )
{
    DKUUID uuid;
    UUID tmp;

    RPC_STATUS status = UuidCreate( &tmp );

    // UuidCreate may return RPC_S_OK, RPC_S_UUID_LOCAL_ONLY or RPC_S_UUID_NO_ADDRESS,
    // but these apparently aren't defined on all platforms.
    if( status != ERROR_SUCCESS /* RPC_S_OK */ )
    {
        DKWarning( "dk_uuid_generate: UuidCreate returned status 0x%X.", status );
    }

    *((uint32_t *)&uuid.bytes[0]) = DKSwapInt32HostToBig( tmp.Data1 );
    *((uint16_t *)&uuid.bytes[4]) = DKSwapInt16HostToBig( tmp.Data2 );
    *((uint16_t *)&uuid.bytes[6]) = DKSwapInt16HostToBig( tmp.Data3 );

    uuid.bytes[8] = tmp.Data4[0];
    uuid.bytes[9] = tmp.Data4[1];

    uuid.bytes[10] = tmp.Data4[2];
    uuid.bytes[11] = tmp.Data4[3];
    uuid.bytes[12] = tmp.Data4[4];
    uuid.bytes[13] = tmp.Data4[5];
    uuid.bytes[14] = tmp.Data4[6];
    uuid.bytes[15] = tmp.Data4[7];

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
//  dk_datetime()
//
#if DK_PLATFORM_APPLE || DK_PLATFORM_LINUX || DK_PLATFORM_UNIX
DKDateTime dk_datetime( void )
{
    struct timeval t;
    
    if( gettimeofday( &t, NULL ) )
        return 0.0;
    
    return ((DKDateTime)t.tv_sec) - DKAbsoluteTimeSince1970 + (((DKDateTime)t.tv_usec) * 1.0e-6);
}

#elif DK_PLATFORM_WINDOWS
DKDateTime dk_datetime( void )
{
    FILETIME fileTime;

    GetSystemTimePreciseAsFileTime( &fileTime );
    
    // These are actually 100 nanosecond intervals
    uint64_t nsecs_since_1601 = (((uint64_t)fileTime.dwHighDateTime) << 32) | ((uint64_t)fileTime.dwLowDateTime);
    uint64_t nsecs_since_1970 = nsecs_since_1601 - (11644473600000 * 10000);

    uint64_t secs = nsecs_since_1970 / 10000000;
    uint64_t nsecs = nsecs_since_1970 - secs;

    return (DKDateTime)secs - DKAbsoluteTimeSince1970 + ((DKDateTime)nsecs * 1.0e-7);
}

#endif


///
//  dk_localtime()
//
#if DK_PLATFORM_APPLE || DK_PLATFORM_LINUX || DK_PLATFORM_UNIX
DKDateTime dk_localtime( void )
{
    struct timeval t;
    struct timezone tz;
    
    if( gettimeofday( &t, &tz ) )
        return 0.0;
    
    return ((DKDateTime)t.tv_sec) - DKAbsoluteTimeSince1970 + (((DKDateTime)t.tv_usec) * 1.0e-6) + (tz.tz_minuteswest * 60.0);
}

#elif DK_PLATFORM_WINDOWS
DKDateTime dk_localtime( void )
{
    DYNAMIC_TIME_ZONE_INFORMATION timeZone;
    double timeZoneOffset = 0.0;

    DWORD result = GetDynamicTimeZoneInformation( &timeZone );

    if( result != TIME_ZONE_ID_INVALID )
        timeZoneOffset = timeZone.Bias * 60.0;

    return dk_datetime() + timeZoneOffset;
}

#endif


///
//  dk_systemtime()
//
#if DK_PLATFORM_APPLE || DK_PLATFORM_LINUX || DK_PLATFORM_UNIX
DKDateTime dk_systemtime( void )
{
    return dk_datetime();
}

#elif DK_PLATFORM_WINDOWS
DKDateTime dk_systemtime( void )
{
    static uint64_t Frequency = 0;

    if( Frequency == 0 )
    {
        LARGE_INTEGER tmp;
        QueryPerformanceFrequency( &tmp );

        Frequency = tmp.QuadPart;
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter( &now );

    return (double)now.QuadPart / (double)Frequency;
}

#endif






