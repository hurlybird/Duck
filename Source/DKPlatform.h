/*****************************************************************************************

  DKPlatform.h

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

#ifndef _DK_PLATFORM_H_
#define _DK_PLATFORM_H_

// Make sure DKConfig.h is included first
#ifndef _DK_CONFIG_H_
#error DKConfig.h has not been included
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <limits.h>

// Some system headers (i.e. inttypes.h) can cause errors when exposed by the framework.
#ifndef DK_EXCLUDE_NONMODULAR_HEADERS
#include <inttypes.h>
#endif

// C++ Compatibility
#ifdef __cplusplus

#ifndef restrict
#define restrict
#endif

#endif


// Clang/GCC/MSVC ------------------------------------------------------------------------
#if __clang__
#define dk_breakfatal() __builtin_trap()
#elif __GNUC__
#define dk_breakfatal() __builtin_trap()
#elif _MSC_VER
#define dk_breakfatal() __debugbreak(); abort()
#endif

#if DEBUG
#define dk_abortfatal() dk_breakfatal()
#else
#define dk_abortfatal() abort()
#endif


// Apple ---------------------------------------------------------------------------------
#if DK_PLATFORM_APPLE
#include <uuid/uuid.h>

#define DK_API
#define DK_ATTRIBUTE_ANALYZER_NO_RETURN     __attribute__((analyzer_noreturn))
#endif


// POSIX ---------------------------------------------------------------------------------
#if DK_PLATFORM_POSIX
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>

#define dk_spinlock_yield()     sched_yield()

#endif


// Linux/Unix ----------------------------------------------------------------------------
#if DK_PLATFORM_LINUX || DK_PLATFORM_UNIX
#include <endian.h>
#include <uuid/uuid.h>

#define DK_API
#define DK_ATTRIBUTE_ANALYZER_NO_RETURN

#if !defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)
    #if defined(BYTE_ORDER) && defined(BIG_ENDIAN)
        #if BYTE_ORDER == BIG_ENDIAN
            #define __BIG_ENDIAN__ 1
        #else
            #define __LITTLE_ENDIAN__ 1
        #endif
    #elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
        #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            #define __BIG_ENDIAN__ 1
        #else
            #define __LITTLE_ENDIAN__ 1
        #endif
    #else
        #error "Duck: Unknown or missing byte order definition"
    #endif
#endif

#endif


// Android -------------------------------------------------------------------------------
#if DK_PLATFORM_ANDROID
#define DK_API
#define DK_ATTRIBUTE_ANALYZER_NO_RETURN
#endif


// Windows -------------------------------------------------------------------------------
#if DK_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

//#define _CRT_NONSTDC_NO_DEPRECATE
//#define _CRT_SECURE_NO_WARNINGS
//#define _CRT_SECURE_NO_DEPRECATE  

#include <sys/stat.h>
#include <sys/types.h>

#if defined(DK_API_STATIC)
#define DK_API
#elif defined(DK_API_EXPORTS)
#define DK_API __declspec(dllexport)
#else
#define DK_API __declspec(dllimport)
#endif

#ifdef _WIN64
#ifndef __LP64__
#define __LP64__ 1
#endif
#endif

#ifndef __LITTLE_ENDIAN__
#define __LITTLE_ENDIAN__ 1
#endif

#define DK_ATTRIBUTE_ANALYZER_NO_RETURN

#ifndef restrict
#define restrict __restrict
#endif

#ifndef strcasecmp
#define strcasecmp( s1, s2 )        _stricmp( s1, s2 )
#define strncasecmp( s1, s2, n )    _strnicmp( s1, s2, n )
#endif

#define dk_spinlock_yield()         SwitchToThread()

#endif


// Nintendo Switch ----------------------------------------------------------------------
#if DK_PLATFORM_NX

#define DK_API
#define DK_ATTRIBUTE_ANALYZER_NO_RETURN

#ifndef restrict
#define restrict __restrict
#endif

#endif


// PlayStation 5 ------------------------------------------------------------------------
#if DK_PLATFORM_PS5

#define DK_API
#define DK_ATTRIBUTE_ANALYZER_NO_RETURN

#ifndef restrict
#define restrict __restrict
#endif

#endif




// Basic Types & Constants ===============================================================
#ifdef __cplusplus
extern "C"
{
#endif


// Objects
typedef void * DKObjectRef;
typedef void * DKWeakRef;


// Forward declarations of common object types
typedef struct DKClass *            DKClassRef;
typedef const struct DKProperty *   DKPropertyRef;
typedef struct DKString *           DKStringRef;
typedef struct DKData *             DKDataRef;
typedef struct DKNumber *           DKNumberRef;
typedef struct DKEnum *             DKEnumRef;
typedef struct DKPredicate *        DKPredicateRef;
typedef DKObjectRef                 DKListRef;
typedef DKObjectRef                 DKMutableListRef;
typedef DKObjectRef                 DKDictionaryRef;
typedef DKObjectRef                 DKMutableDictionaryRef;
typedef DKObjectRef                 DKSetRef;
typedef struct DKArray *            DKArrayRef;
typedef struct DKArray *            DKMutableArrayRef;
typedef struct DKHashTable *        DKHashTableRef;
typedef struct DKHashTable *        DKMutableHashTableRef;


// Define a constant string with a compile-time constant C string.
#define DKSTR( s ) __DKStringGetConstantString( "" s "", true )

// Define a constant string. Constant strings require external storage so unless you
// know what you're doing, use the DKSTR macro instead of calling this directly
DK_API DKStringRef __DKStringGetConstantString( const char * str, bool insert );


// DKIndex is a signed integer the same width as size_t, to make index arithmetic
// easier. A long works for clang and gcc (CFIndex is defined as a long), but in
// Microsoft compiler land under Win64 a long is 4-bytes and size_t is 8-bytes.
// Thus far intptr_t resolves to the desired size, but this may need to be updated
// for other architectures/compilers.
typedef intptr_t  DKIndex;

enum
{
    DKNotFound = -1,
};


// Hash codes must be the same size as pointers so pointers can be used as hashcodes
typedef uintptr_t DKHashCode;


// UTF-8 character - a single UTF-8 code point + '\0'
// Valid UTF-8 code points should be <= 4 bytes (and always <= 6 bytes)
typedef struct
{
    char s[8];
    
} DKChar8;

// UTF-32 character - char32_t support is flaky
typedef int32_t DKChar32;


// Date and time
typedef double DKDateTime;
typedef double DKTimeInterval;

#define DKAbsoluteTimeSince1970     978307200.0     // POSIX reference time
#define DKAbsoluteTimeSince2001     0.0             // CoreFoundation reference time


// Ranges
typedef struct
{
    DKIndex location;
    DKIndex length;

} DKRange;

#define DKRangeMake( loc, len )     (const DKRange){ loc, len }
#define DKRangeEnd( range )         (((range).location) + ((range).length))

// True if range is inside 0..len OR is the empty sequence at len+1
#define DKRangeInsideOrEnd( range, len )    (((range).location >= 0) && ((range).length >= 0) && (DKRangeEnd(range) <= len))


// Callback Types
typedef bool        (*DKEqualityFunction)( DKObjectRef a, DKObjectRef b );
typedef int         (*DKCompareFunction)( DKObjectRef a, DKObjectRef b );
typedef DKHashCode  (*DKHashFunction)( DKObjectRef a );
typedef int         (*DKApplierFunction)( DKObjectRef object, void * context );
typedef int         (*DKKeyedApplierFunction)( DKObjectRef key, DKObjectRef object, void * context );
typedef DKObjectRef (*DKModifierFunction)( DKObjectRef object, void * context );


// Path Constants
#define DKPathComponentSeparator    '/'
#define DKPathExtensionSeparator    '.'


// Insertion Policies
typedef enum
{
    DKInsertAlways,
    DKInsertIfFound,
    DKInsertIfNotFound
    
} DKInsertPolicy;


// Filter Actions
typedef enum
{
    DKFilterSkip =  0,
    DKFilterKeep =  0x1,
    DKFilterStop =  0x2,

    DKFilterKeepAndStop = (DKFilterKeep | DKFilterStop),
    DKFilterSkipAndStop = (DKFilterSkip | DKFilterStop)

} DKFilterAction;


// Byte Order
typedef enum
{
    DKByteOrderUnspecified = 0,
    
    DKByteOrderBigEndian,
    DKByteOrderLittleEndian,
    
    #ifdef __LITTLE_ENDIAN__
    DKByteOrderNative = DKByteOrderLittleEndian
    #endif
    
    #ifdef __BIG_ENDIAN__
    DKByteOrderNative = DKByteOrderBigEndian
    #endif

} DKByteOrder;


// UUID
typedef struct
{
    uint8_t bytes[16];

} DKUUID;

#define DKUUIDZero  (DKUUID){ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }



// Error Reporting =======================================================================

// Set external handlers for debug, warning and error messages
DK_API void DKSetPrintfCallback( int (*callback)( const char * format, va_list arg_ptr ) );
DK_API void DKSetLogCallback( int (*callback)( const char * format, va_list arg_ptr ) );
DK_API void DKSetWarningCallback( int (*callback)( const char * format, va_list arg_ptr ) );
DK_API void DKSetErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );
DK_API void DKSetFatalErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );

// Set a prefix for warnings, errors and fatal errors
DK_API void DKSetLogFormat( const char * prefix, const char * suffix );
DK_API void DKSetWarningFormat( const char * prefix, const char * suffix );
DK_API void DKSetErrorFormat( const char * prefix, const char * suffix );
DK_API void DKSetFatalErrorFormat( const char * prefix, const char * suffix );

// Set whether to abort on errors (default yes for debug builds, no for release builds)
DK_API void DKSetAbortOnErrors( bool flag );

#ifdef DEBUG
#define DK_DEFAULT_ABORT_ON_ERRORS  true
#else
#define DK_DEFAULT_ABORT_ON_ERRORS  false
#endif

// Print a message. Object descriptions can be printed using the
// Foundation/CoreFoundation idiom "%@".
DK_API int _DKPrintf( const char * format, ... );

#define DKPrintf( ... )     _DKPrintf( __VA_ARGS__ )


// Print a debug message. This is ignored in non-debug builds. Object descriptions can be
// printed using the Foundation/CoreFoundation idiom "%@".
#ifdef DEBUG
#define DKDebug( ... )      _DKPrintf( __VA_ARGS__ )
#else
#define DKDebug( ... )
#endif


// Print a logged message. Object descriptions can be printed using the
// Foundation/CoreFoundation idiom "%@".
DK_API void _DKLog( const char * format, ... );

#define DKLog( ... )        _DKLog( __VA_ARGS__ )


// Print a warning. This is ignored in non-debug builds unless DK_RUNTIME_WARNINGS is
// defined.
DK_API void _DKWarning( const char * format, ... );

#if DK_RUNTIME_WARNINGS
#define DKWarning( ... )    _DKWarning( __VA_ARGS__ )
#else
#define DKWarning( ... )
#endif


// Print a error. In a debug build execution is halted with assert(0). In a non-debug
// build the program will continue running.
DK_API void _DKError( const char * format, ... );

#define DKError( ... )      _DKError( __VA_ARGS__ )


// Print a error. In a debug build execution is halted with assert(0). In a non-debug
// build the program is halted with abort().
DK_API void _DKFatalError( const char * format, ... ) DK_ATTRIBUTE_ANALYZER_NO_RETURN;

#define DKFatalError( ... ) _DKFatalError( __VA_ARGS__ )


// Raise a fatal error if the given condition is not met. These checks are not omitted
// from release builds so use them sparingly.
#define DKRequire( x )                                                                  \
    do                                                                                  \
    {                                                                                   \
        if( !(x) )                                                                      \
        {                                                                               \
            _DKFatalError( "%s: Failed Runtime Requirement( %s )", __func__, #x );      \
        }                                                                               \
    } while( 0 )

// Raise a non-fatal error if the given condition is not met. These checks are not omitted
// from release builds so use them sparingly.
#if DK_RUNTIME_LOGIC_CHECKS
#define DKCheck( x, ... )                                                               \
    do                                                                                  \
    {                                                                                   \
        if( !(x) )                                                                      \
        {                                                                               \
            _DKError( "%s: Failed Runtime Check( %s )", __func__, #x );                 \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )
#else
#define DKCheck( x, ... )
#endif

// Assertions
DK_API void _DKFailedAssert( const char * format, ... );

#if DK_RUNTIME_ASSERTIONS
#define DKAssert( x )                                                                   \
    do                                                                                  \
    {                                                                                   \
        if( !(x) )                                                                      \
        {                                                                               \
            _DKFailedAssert( "%s: Failed Assert( %s )", __func__, #x );                 \
            dk_breakfatal();                                                            \
        }                                                                               \
    } while( 0 )

#define DKAssertKindOfClass( _self, cls )                                               \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsKindOfClass( _self, cls ) )                                            \
        {                                                                               \
            _DKFailedAssert( "%s: Required kind of class %s, received %s",              \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( cls ) ),                         \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
            dk_breakfatal();                                                            \
        }                                                                               \
    } while( 0 )

#define DKAssertMemberOfClass( _self, cls )                                             \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsMemberOfClass( _self, cls ) )                                          \
        {                                                                               \
            _DKFailedAssert( "%s: Required member of class %s, received %s",            \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( cls ) ),                         \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
            dk_breakfatal();                                                            \
        }                                                                               \
    } while( 0 )

#define DKAssertInterface( _self, sel )                                                 \
    do                                                                                  \
    {                                                                                   \
        if( !DKQueryInterface( _self, sel, NULL ) )                                     \
        {                                                                               \
            _DKFailedAssert( "%s: Required interface %s on class %s",                   \
                __func__,                                                               \
                DKStringGetCStringPtr( DKStringFromSelector( sel ) ),                   \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
            dk_breakfatal();                                                            \
        }                                                                               \
    } while( 0 )

#define DKAssertMutable( _self )                                                        \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsMutable( _self, cls ) )                                                \
        {                                                                               \
            _DKFailedAssert( "%s: Trying to modify an instance of immutable class %s",  \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
            dk_breakfatal();                                                            \
        }                                                                               \
    } while( 0 )

#else
#define DKAssert( x )
#define DKAssertMsg( x, ... )
#define DKAssertKindOfClass( _self, cls )
#define DKAssertMemberOfClass( _self, cls ) 
#define DKAssertInterface( _self, sel )
#endif


// Type Checks
#if DK_RUNTIME_TYPE_CHECKS
#define DKCheckKindOfClass( _self, cls, ... )                                           \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsKindOfClass( _self, cls ) )                                            \
        {                                                                               \
            _DKError( "%s: Expected kind of class %s, received %s",                     \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( cls ) ),                         \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#define DKCheckMemberOfClass( _self, cls, ... )                                         \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsKindOfClass( _self, cls ) )                                            \
        {                                                                               \
            _DKError( "%s: Expected member of class %s, received %s",                   \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( cls ) ),                         \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#define DKCheckInterface( _self, sel, ... )                                             \
    do                                                                                  \
    {                                                                                   \
        if( !DKQueryInterface( _self, sel, NULL ) )                                     \
        {                                                                               \
            _DKError( "%s: Expected interface %s on class %s",                          \
                __func__,                                                               \
                DKStringGetCStringPtr( DKStringFromSelector( sel ) ),                   \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#define DKCheckMutable( _self, ... )                                                    \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsMutable( _self ) )                                                     \
        {                                                                               \
            _DKError( "%s: Trying to modify an instance of immutable class %s",         \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#else
#define DKCheckKindOfClass( _self, cls, ... )
#define DKCheckMemberOfClass( _self, cls, ... )
#define DKCheckInterface( _self, sel, ... )
#define DKCheckMutable( _self, ... )
#endif


// Range Checks
#if DK_RUNTIME_RANGE_CHECKS
#define DKCheckIndex( index, len, ... )                                                 \
    do                                                                                  \
    {                                                                                   \
        if( ((index) < 0) || ((index) >= len) )                                         \
        {                                                                               \
            _DKError( "%s: Index %ld is outside 0,%ld", __func__, (index), len );       \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#define DKCheckRange( range, len, ... )                                                 \
    do                                                                                  \
    {                                                                                   \
        if( !DKRangeInsideOrEnd( range, len ) )                                         \
        {                                                                               \
            _DKError( "%s: Range %ld,%ld is outside 0,%ld",                             \
                __func__, (range).location, (range).length, len );                      \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#else
#define DKCheckIndex( index, len, ... )
#define DKCheckRange( range, len, ... )
#endif


// Specific Errors
DK_API void DKImmutableObjectAccessError( DKObjectRef _self );




// Memory Allocation =====================================================================
typedef void * (*dk_malloc_callback)( size_t size );
typedef void * (*dk_calloc_callback)( size_t num, size_t size );
typedef void * (*dk_realloc_callback)( void * ptr, size_t size );
typedef void (*dk_free_callback)( void * ptr );

DK_API void DKSetExternalAllocator( dk_malloc_callback _malloc,
    dk_realloc_callback _realloc,
    dk_calloc_callback _calloc,
    dk_free_callback _free );

DK_API void * dk_malloc( size_t size );
DK_API void * dk_calloc( size_t num, size_t size );
DK_API void * dk_realloc( void * ptr, size_t size );
DK_API void   dk_free( void * ptr );




// Atomic Operations =====================================================================
#if DK_PLATFORM_GCC_INTRINSICS

#define DKAtomicAdd32( ptr, x )                     __sync_add_and_fetch( ptr, x )
#define DKAtomicSub32( ptr, x )                     __sync_sub_and_fetch( ptr, x )
#define DKAtomicAnd32( ptr, x )                     __sync_and_and_fetch( ptr, x )
#define DKAtomicOr32( ptr, x )                      __sync_or_and_fetch( ptr, x )
#define DKAtomicIncrement32( ptr )                  __sync_add_and_fetch( ptr, 1 )
#define DKAtomicDecrement32( ptr )                  __sync_sub_and_fetch( ptr, 1 )
#define DKAtomicCmpAndSwap32( ptr, _old, _new )     __sync_bool_compare_and_swap( ptr, _old, _new )

#define DKAtomicAdd64( ptr, x )                     __sync_add_and_fetch( ptr, x )
#define DKAtomicSub64( ptr, x )                     __sync_sub_and_fetch( ptr, x )
#define DKAtomicAnd64( ptr, x )                     __sync_and_and_fetch( ptr, x )
#define DKAtomicOr64( ptr, x )                      __sync_or_and_fetch( ptr, x )
#define DKAtomicIncrement64( ptr )                  __sync_add_and_fetch( ptr, 1 )
#define DKAtomicDecrement64( ptr )                  __sync_sub_and_fetch( ptr, 1 )
#define DKAtomicCmpAndSwap64( ptr, _old, _new )     __sync_bool_compare_and_swap( ptr, _old, _new )

#define DKAtomicCmpAndSwapPtr( ptr, _old, _new )    __sync_bool_compare_and_swap( ptr, _old, _new )

#define DKMemoryBarrier()                           __sync_synchronize()

#endif

#if DK_PLATFORM_WINDOWS

static_assert( sizeof(LONG) == sizeof(int32_t), "DKAtomic: Windows LONG type is not 32-bits." );

#define DKAtomicAdd32( ptr, x )                     InterlockedAdd( (LONG volatile *)(ptr), (LONG)(x) )
#define DKAtomicSub32( ptr, x )                     InterlockedAdd( (LONG volatile *)(ptr), -(LONG)(x) )
#define DKAtomicAnd32( ptr, x )                     InterlockedAnd( (LONG volatile *)(ptr), (LONG)(x) )
#define DKAtomicOr32( ptr, x )                      InterlockedOr( (LONG volatile *)(ptr), (LONG)(x) )
#define DKAtomicIncrement32( ptr )                  InterlockedIncrement( (LONG volatile *)(ptr) )
#define DKAtomicDecrement32( ptr )                  InterlockedDecrement( (LONG volatile *)(ptr) )
#define DKAtomicCmpAndSwap32( ptr, _old, _new )     (InterlockedCompareExchange( (LONG volatile *)(ptr), (LONG)(_new), (LONG)(_old) ) == (LONG)(_old))

#define DKAtomicAdd64( ptr, x )                     InterlockedAdd64( (LONG64 volatile *)(ptr), (LONG64)(x) )
#define DKAtomicSub64( ptr, x )                     InterlockedAdd64( (LONG64 volatile *)(ptr), -(LONG64)(x) )
#define DKAtomicAnd64( ptr, x )                     InterlockedAnd64( (LONG64 volatile *)(ptr), (LONG64)(x) )
#define DKAtomicOr64( ptr, x )                      InterlockedOr64( (LONG64 volatile *)(ptr), -(LONG64)(x) )
#define DKAtomicIncrement64( ptr )                  InterlockedIncrement64( (LONG64 volatile *)(ptr) )
#define DKAtomicDecrement64( ptr )                  InterlockedDecrement64( (LONG64 volatile *)(ptr) )
#define DKAtomicCmpAndSwap64( ptr, _old, _new )     (InterlockedCompareExchange64( (LONG64 volatile *)(ptr), (LONG64)(_new), (LONG64)(_old) ) == (LONG64)(_old))

#define DKAtomicCmpAndSwapPtr( ptr, _old, _new )    (InterlockedCompareExchangePointer( (PVOID volatile *)ptr, _new, _old ) == (_old))

#define DKMemoryBarrier()                           MemoryBarrier()

#endif




// Spin Locks ============================================================================
typedef int32_t DKSpinLock;

#define DKSpinLockInit              0

inline static void DKSpinLockLock( DKSpinLock * spinlock )
{
    int32_t volatile * lock = spinlock;

    while( !DKAtomicCmpAndSwap32( lock, 0, 1 ) )
    {
        dk_spinlock_yield();
    }
}

inline static void DKSpinLockUnlock( DKSpinLock * spinlock )
{
    int32_t volatile * lock = spinlock;
    
    DKAtomicAnd32( lock, 0 );
}



// Byte Order Operations =================================================================
#if DK_PLATFORM_GCC_INTRINSICS

#define DKSwapInt16( x )                        __builtin_bswap16( x )
#define DKSwapInt32( x )                        __builtin_bswap32( x )
#define DKSwapInt64( x )                        __builtin_bswap64( x )

#else

static inline uint16_t DKSwapInt16( uint16_t x )
{
    uint16_t byte0 = x << 8;
    uint16_t byte1 = x >> 8;

    return byte0 | byte1;
}

static inline uint32_t DKSwapInt32( uint32_t x )
{
    uint32_t byte0 = x << 24;
    uint32_t byte1 = (x << 8) & 0x00ff0000;
    uint32_t byte2 = (x >> 8) & 0x0000ff00;
    uint32_t byte3 = x >> 24;
    
    return byte0 | byte1 | byte2 | byte3;
}

static inline int64_t DKSwapInt64( int64_t x )
{
    uint64_t word0 = DKSwapInt32( (uint32_t)x );
    uint64_t word1 = DKSwapInt32( (uint32_t)(x >> 32) );
    
    return word0 | word1;
}

#endif

static inline float DKSwapFloat( float x )
{
    union Swap { float fval; int32_t ival; } swap;
    swap.fval = x;
    swap.ival = DKSwapInt32( swap.ival );
    return swap.fval;
}

static inline double DKSwapDouble( double x )
{
    union Swap { double fval; int64_t ival; } swap;
    swap.fval = x;
    swap.ival = DKSwapInt64( swap.ival );
    return swap.fval;
}


#ifdef __LITTLE_ENDIAN__
#define DKSwapInt16BigToHost( x )               DKSwapInt16( x )
#define DKSwapInt16HostToBig( x )               DKSwapInt16( x )
#define DKSwapInt16LittleToHost( x )            ( x )
#define DKSwapInt16HostToLittle( x )            ( x )

#define DKSwapInt32BigToHost( x )               DKSwapInt32( x )
#define DKSwapInt32HostToBig( x )               DKSwapInt32( x )
#define DKSwapInt32LittleToHost( x )            ( x )
#define DKSwapInt32HostToLittle( x )            ( x )

#define DKSwapInt64BigToHost( x )               DKSwapInt64( x )
#define DKSwapInt64HostToBig( x )               DKSwapInt64( x )
#define DKSwapInt64LittleToHost( x )            ( x )
#define DKSwapInt64HostToLittle( x )            ( x )

#define DKSwapFloatBigToHost( x )               DKSwapFloat( x )
#define DKSwapFloatHostToBig( x )               DKSwapFloat( x )
#define DKSwapFloatLittleToHost( x )            ( x )
#define DKSwapFloatHostToLittle( x )            ( x )

#define DKSwapDoubleBigToHost( x )              DKSwapDouble( x )
#define DKSwapDoubleHostToBig( x )              DKSwapDouble( x )
#define DKSwapDoubleLittleToHost( x )           ( x )
#define DKSwapDoubleHostToLittle( x )           ( x )
#endif

#ifdef __BIG_ENDIAN__
#define DKSwapInt16BigToHost( x )               ( x )
#define DKSwapInt16HostToBig( x )               ( x )
#define DKSwapInt16LittleToHost( x )            DKSwapInt16( x )
#define DKSwapInt16HostToLittle( x )            DKSwapInt16( x )

#define DKSwapInt32BigToHost( x )               ( x )
#define DKSwapInt32HostToBig( x )               ( x )
#define DKSwapInt32LittleToHost( x )            DKSwapInt32( x )
#define DKSwapInt32HostToLittle( x )            DKSwapInt32( x )

#define DKSwapInt64BigToHost( x )               ( x )
#define DKSwapInt64HostToBig( x )               ( x )
#define DKSwapInt64LittleToHost( x )            DKSwapInt64( x )
#define DKSwapInt64HostToLittle( x )            DKSwapInt64( x )

#define DKSwapFloatBigToHost( x )               ( x )
#define DKSwapFloatHostToBig( x )               ( x )
#define DKSwapFloatLittleToHost( x )            DKSwapFloat( x )
#define DKSwapFloatHostToLittle( x )            DKSwapFloat( x )

#define DKSwapDoubleBigToHost( x )              ( x )
#define DKSwapDoubleHostToBig( x )              ( x )
#define DKSwapDoubleLittleToHost( x )           DKSwapDouble( x )
#define DKSwapDoubleHostToLittle( x )           DKSwapDouble( x )
#endif




// Other Utilities =======================================================================

// Generate UUIDs
DK_API DKUUID dk_uuid_generate( void );

// Basic hashing
DK_API uint32_t dk_strhash32( const char * str );
DK_API uint64_t dk_strhash64( const char * str );

DK_API uint32_t dk_memhash32( const void * buffer, size_t buffer_size );
DK_API uint64_t dk_memhash64( const void * buffer, size_t buffer_size );

#if __LP64__
#define dk_strhash( str )                   dk_strhash64( str )
#define dk_memhash( buffer, buffer_size )   dk_memhash64( buffer, buffer_size )
#else
#define dk_strhash( str )                   dk_strhash32( str )
#define dk_memhash( buffer, buffer_size )   dk_memhash32( buffer, buffer_size )
#endif


// Interpret a string as a base-10 integer (true) or floating-point (false) value
DK_API bool dk_strtonum( const char * str, int64_t * ival, double * fval, char const ** str_end );


// String-to-X wrappers (because MS likes to be different)
#define dk_strtoi32( str, end, base )       ((int32_t)strtol( (str), (end), (base) ));
#define dk_strtou32( str, end, base )       ((uint32_t)strtoul( (str), (end), (base) ));

#if LONG_MAX == 0x7FFFFFFFFFFFFFFF
#define dk_strtoi64( str, end, base )       strtol( (str), (end), (base) );
#define dk_strtou64( str, end, base )       strtoul( (str), (end), (base) );
#else
#define dk_strtoi64( str, end, base )       strtoll( (str), (end), (base) );
#define dk_strtou64( str, end, base )       strtoull( (str), (end), (base) );
#endif

#define dk_strtof32( str, end )             strtof( (str), (end) )
#define dk_strtof64( str, end )             strtod( (str), (end) )


// Time in seconds since Jan 1 2001 00:00:00 UTC (equivalent to Apple's CFDate)
DK_API DKDateTime dk_datetime( void );

// Local time in seconds since Jan 1 2001 00:00:00 UTC
DK_API DKDateTime dk_localtime( void );

// Time in seconds since a system-specific start time. The returned values are not
// compatible with, but may have greater precision than, those from dk_datetime().
DK_API DKDateTime dk_systemtime( void );


#ifdef __cplusplus
}
#endif

#endif // _DK_PLATFORM_H_







