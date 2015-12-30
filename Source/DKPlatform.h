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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "DKConfig.h"

#if DK_PLATFORM_APPLE
#include <libkern/OSAtomic.h>
#include <libkern/OSByteOrder.h>
#endif

#if DK_PLATFORM_ANDROID
// Do stuff here...
#endif

#if DK_PLATFORM_LINUX
// Do stuff here...
#endif

#if DK_PLATFORM_BSD
#include <sys/stat.h>
#endif

#if DK_PLATFORM_POSIX
#include <pthread.h>
#endif




// Basic Types & Constants ===============================================================

// Objects
typedef void * DKObjectRef;
typedef void * DKWeakRef;


// Forward declarations of common object types
typedef struct DKClass * DKClassRef;
typedef struct DKProperty * DKPropertyRef;
typedef struct DKString * DKStringRef;
typedef struct DKPredicate * DKPredicateRef;
typedef struct DKList * DKListRef;
typedef struct DKDictionary * DKDictionaryRef;


// Define a constant string with a compile-time constant C string.
#define DKSTR( s ) __DKStringGetConstantString( "" s "", true )

// Define a constant string. Constant strings require external storage so unless you
// know what you're doing, use the DKSTR macro instead of calling this directly
DKStringRef __DKStringGetConstantString( const char * str, bool insert );


// Indexes
typedef intptr_t  DKIndex;    // Indexes are basically a signed size_t
typedef uintptr_t DKHashCode; // Pointers can be used as hash codes

enum
{
    DKNotFound = -1,
};


// UTF32 character - char32_t support is flaky
typedef int32_t DKChar32;


// Date and time
typedef double DKDateTime;
typedef double DKTimeInterval;

#define DKAbsoluteTimeSince1970     978307200.0L    // POSIX reference time
#define DKAbsoluteTimeSince2001     0.0L            // CoreFoundation reference time


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
typedef bool (*DKEqualityFunction)( DKObjectRef a, DKObjectRef b );
typedef int  (*DKCompareFunction)( DKObjectRef a, DKObjectRef b );
typedef DKHashCode (*DKHashFunction)( DKObjectRef a );
typedef int  (*DKApplierFunction)( DKObjectRef object, void * context );
typedef int  (*DKKeyedApplierFunction)( DKObjectRef key, DKObjectRef object, void * context );


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




// Error Reporting =======================================================================

// Set external handlers for debug, warning and error messages
void DKSetPrintfCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void DKSetWarningCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void DKSetErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void DKSetFatalErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );


// print a message. Object descriptions can be printed using the
// Foundation/CoreFoundation idiom "%@".
int    _DKPrintf( const char * format, ... );

#define DKPrintf( ... )     _DKPrintf( __VA_ARGS__ )


// Print a debug message. This is ignored in non-debug builds. Object descriptions can be
// printed using the Foundation/CoreFoundation idiom "%@".
#ifdef DEBUG
#define DKDebug( ... )      _DKPrintf( __VA_ARGS__ )
#else
#define DKDebug( ... )
#endif


// Print a warning. This is ignored in non-debug builds unless DK_RUNTIME_WARNINGS is
// defined.
int    _DKWarning( const char * format, ... );

#if DK_RUNTIME_WARNINGS
#define DKWarning( ... )    _DKWarning( __VA_ARGS__ )
#else
#define DKWarning( ... )
#endif


// Print a error. In a debug build execution is halted with assert(0). In a non-debug
// build the program will continue running.
int    _DKError( const char * format, ... );

#define DKError( ... )      _DKError( __VA_ARGS__ )


// Print a error. In a debug build execution is halted with assert(0). In a non-debug
// build the program is halted with abort().
int    _DKFatalError( const char * format, ... ) __attribute__((analyzer_noreturn));

#define DKFatalError( ... ) _DKFatalError( __VA_ARGS__ )


// Raise a fatal error if the given condition is not met. These checks are not omitted
// from release builds so use them sparingly.
#define DKRequire( x )                                                                  \
    do                                                                                  \
    {                                                                                   \
        if( !(x) )                                                                      \
        {                                                                               \
            _DKFatalError( "%s: Failed Runtime Requirement( %s )\n", __func__, #x );    \
        }                                                                               \
    } while( 0 )

// Raise a non-fatal error if the given condition is not met. These checks are not omitted
// from release builds so use them sparingly.
#define DKCheck( x, ... )                                                               \
    do                                                                                  \
    {                                                                                   \
        if( !(x) )                                                                      \
        {                                                                               \
            _DKError( "%s: Failed Runtime Check( %s )\n", __func__, #x );               \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )


// Assertions
#if DK_RUNTIME_ASSERTIONS
#define DKAssert( x )                                                                   \
    do                                                                                  \
    {                                                                                   \
        if( !(x) )                                                                      \
        {                                                                               \
            _DKFatalError( "%s: Failed Assert( %s )\n", __func__, #x );                 \
        }                                                                               \
    } while( 0 )

#define DKAssertKindOfClass( _self, cls )                                               \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsKindOfClass( _self, cls ) )                                            \
        {                                                                               \
            _DKFatalError( "%s: Required kind of class %s, received %s\n",              \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( cls ) ),                         \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
        }                                                                               \
    } while( 0 )

#define DKAssertMemberOfClass( _self, cls )                                             \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsMemberOfClass( _self, cls ) )                                          \
        {                                                                               \
            _DKFatalError( "%s: Required member of class %s, received %s\n",            \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( cls ) ),                         \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
        }                                                                               \
    } while( 0 )

#define DKAssertInterface( _self, sel )                                                 \
    do                                                                                  \
    {                                                                                   \
        if( !DKQueryInterface( _self, sel, NULL ) )                                     \
        {                                                                               \
            _DKFatalError( "%s: Required interface %s on class %s\n",                   \
                __func__,                                                               \
                DKStringGetCStringPtr( DKStringFromSelector( sel ) ),                   \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
        }                                                                               \
    } while( 0 )

#define DKAssertMutable( _self )                                                        \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsMutable( _self, cls ) )                                                \
        {                                                                               \
            _DKFatalError( "%s: Trying to modify an instance of immutable class %s\n",  \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
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
            _DKError( "%s: Expected kind of class %s, received %s\n",                   \
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
            _DKError( "%s: Expected member of class %s, received %s\n",                 \
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
            _DKError( "%s: Expected interface %s on class %s\n",                        \
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
            _DKError( "%s: Trying to modify an instance of immutable class %s\n",       \
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
#define DKCheckIndex( range, len, ... )                                                 \
    do                                                                                  \
    {                                                                                   \
        if( ((index) < 0) || ((index) >= len) )                                         \
        {                                                                               \
            _DKError( "%s: Index %ld is outside 0,%ld\n", __func__, (index), len );     \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#define DKCheckRange( range, len, ... )                                                 \
    do                                                                                  \
    {                                                                                   \
        if( !DKRangeInsideOrEnd( range, len ) )                                         \
        {                                                                               \
            _DKError( "%s: Range %ld,%ld is outside 0,%ld\n",                           \
                __func__, (range).location, (range).length, len );                      \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#else
#define DKCheckIndex( index, len, ... )
#define DKCheckRange( range, len, ... )
#endif


// Specific Errors
void DKImmutableObjectAccessError( DKObjectRef _self );




// Memory Allocation =====================================================================
typedef void * (*dk_malloc_callback)( size_t size );
typedef void (*dk_free_callback)( void * ptr );

void   DKSetExternalAllocator( dk_malloc_callback _malloc, dk_free_callback _free );

void * dk_malloc( size_t size );
void   dk_free( void * ptr );




// Spin Locks ============================================================================
#if DK_PLATFORM_APPLE
typedef OSSpinLock DKSpinLock;

#define DKSpinLockInit              OS_SPINLOCK_INIT
#define DKSpinLockLock( s )         OSSpinLockLock( s )
#define DKSpinLockUnlock( s )       OSSpinLockUnlock( s )

#else
typedef spinlock_t DKSpinLock;

#define DKSpinLockInit              SPIN_LOCK_UNLOCKED
#define DKSpinLockLock( s )         spin_lock( s )
#define DKSpinLockUnlock( s )       spin_unlock( s )

#endif



// Atomic Operations =====================================================================
#if DK_PLATFORM_APPLE
#define DKAtomicAdd32( ptr, x )                 OSAtomicAdd32Barrier( x, ptr )
#define DKAtomicSub32( ptr, x )                 OSAtomicSub32Barrier( x, ptr )
#define DKAtomicAnd32( ptr, x )                 OSAtomicAnd32Barrier( x, ptr )
#define DKAtomicOr32( ptr, x )                  OSAtomicOr32Barrier( x, ptr )
#define DKAtomicIncrement32( ptr )              OSAtomicIncrement32Barrier( ptr )
#define DKAtomicDecrement32( ptr )              OSAtomicDecrement32Barrier( ptr )
#define DKAtomicCmpAndSwap32( val, old, new )   OSAtomicCompareAndSwap32Barrier( old, new, val )
#define DKAtomicCmpAndSwapPtr( val, old, new )  OSAtomicCompareAndSwapPtrBarrier( old, new, val )

#else
#define DKAtomicAdd32( ptr, x )                 __sync_add_and_fetch( ptr, x )
#define DKAtomicSub32( ptr, x )                 __sync_sub_and_fetch( ptr, x )
#define DKAtomicAnd32( ptr, x )                 __sync_and_and_fetch( ptr, x )
#define DKAtomicOr32( ptr, x )                  __sync_or_and_fetch( ptr, x )
#define DKAtomicIncrement32( ptr )              __sync_add_and_fetch( ptr, 1 )
#define DKAtomicDecrement32( ptr )              __sync_sub_and_fetch( ptr, 1 )
#define DKAtomicCmpAndSwap32( val, old, new )   __sync_bool_compare_and_swap( val, old, new )
#define DKAtomicCmpAndSwapPtr( val, old, new )  __sync_bool_compare_and_swap( val, old, new )

#endif




// Byte Order Operations =================================================================
#if DK_PLATFORM_APPLE
#define DKSwapInt16( x )                        OSSwapInt16( x )
#define DKSwapInt32( x )                        OSSwapInt32( x )
#define DKSwapInt64( x )                        OSSwapInt64( x )

#else
// Do stuff here

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
DKUUID dk_uuid_generate( void );

// Basic hashing
uint32_t dk_strhash32( const char * str );
uint64_t dk_strhash64( const char * str );

uint32_t dk_memhash32( const void * buffer, size_t buffer_size );
uint64_t dk_memhash64( const void * buffer, size_t buffer_size );

#if __LP64__
#define dk_strhash( str )                   dk_strhash64( str )
#define dk_memhash( buffer, buffer_size )   dk_memhash64( buffer, buffer_size )
#else
#define dk_strhash( str )                   dk_strhash32( str )
#define dk_memhash( buffer, buffer_size )   dk_memhash32( buffer, buffer_size )
#endif


// Time in seconds since midnight January 1, 1970 (i.e. gettimeofday())
DKDateTime dk_datetime( void );



#endif // _DK_PLATFORM_H_







