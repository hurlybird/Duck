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

#include "DKConfig.h"

#if DK_PLATFORM_APPLE
#include <pthread.h>
#include <libkern/OSAtomic.h>

#elif DK_PLATFORM_ANDROID
// Do stuff here...

#elif DK_PLATFORM_LINUX
// Do stuff here...

#endif




// Basic Types & Constants ===============================================================

// Objects
typedef const void * DKObjectRef;
typedef void * DKMutableObjectRef;


// Forward declarations of common object types
typedef const struct DKClass * DKClassRef;
typedef const struct DKWeak * DKWeakRef;
typedef const struct DKProperty * DKPropertyRef;
typedef const struct DKString * DKStringRef;
typedef const struct DKPredicate * DKPredicateRef;
typedef const void * DKListRef;


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

extern const DKDateTime DKAbsoluteTimeSince1970; // POSIX reference time
extern const DKDateTime DKAbsoluteTimeSince2001; // CoreFoundation reference time


// Ranges
typedef struct
{
    DKIndex location;
    DKIndex length;

} DKRange;

#define DKRangeMake( loc, len )     (const DKRange){ loc, len }
#define DKRangeEnd( range )         (((range).location) + ((range).length))

// False if the range is outside 0..len OR is not the empty sequence at len + 1
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
    
    DKByteOrderNative = DKByteOrderLittleEndian

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
#define DKFatal( x )                                                                    \
    do                                                                                  \
    {                                                                                   \
        if( !(x) )                                                                      \
        {                                                                               \
            _DKFatalError( "%s: Fatal Error( %s )\n", __func__, #x );                   \
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
        if( !DKIsKindOfClass( _self, cls ) )                                            \
        {                                                                               \
            _DKFatalError( "%s: Required kind of class %s, received %s\n",              \
                __func__,                                                               \
                DKStringGetCStringPtr( DKGetClassName( cls ) ),                         \
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );                     \
        }                                                                               \
    } while( 0 )

#else
#define DKAssert( x )
#define DKAssertMsg( x, ... )
#define DKAssertKindOfClass( _self, cls )
#define DKAssertMemberOfClass( _self, cls ) 
#endif


// Type Checks
#if DK_RUNTIME_TYPE_CHECKS
#define DKCheckKindOfClass( _self, cls, ... )                                           \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsKindOfClass( _self, cls ) )                                            \
        {                                                                               \
            _DKError( "%s: Expected kind of class %s, received %s\n",                   \
                __func__, DKGetClassName( cls ), DKGetClassName( _self ) );             \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#define DKCheckMemberOfClass( _self, cls, ... )                                         \
    do                                                                                  \
    {                                                                                   \
        if( !DKIsKindOfClass( _self, cls ) )                                            \
        {                                                                               \
            _DKError( "%s: Expected member of class %s, received %s\n",                 \
                __func__, DKGetClassName( cls ), DKGetClassName( _self ) );             \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#define DKCheckInterface( _self, sel, ... )                                             \
    do                                                                                  \
    {                                                                                   \
        if( !DKQueryInterface( _self, sel, NULL ) )                                     \
        {                                                                               \
            _DKError( "%s: Expected interface %s on class %s\n",                        \
                __func__, (sel)->name, DKGetClassName( _self ) );                       \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )

#else
#define DKCheckKindOfClass( _self, cls, ... )
#define DKCheckMemberOfClass( _self, cls, ... )
#define DKCheckInterface( _self, sel, ... )
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

#elif DK_PLATFORM_ANDROID
// Do stuff here...

#elif DK_PLATFORM_LINUX
// Do stuff here...

#endif



// Atomic Operations =====================================================================
#if DK_PLATFORM_APPLE
#define DKAtomicIncrement32( ptr )              OSAtomicIncrement32Barrier( ptr )
#define DKAtomicDecrement32( ptr )              OSAtomicDecrement32Barrier( ptr )
#define DKAtomicCmpAndSwap32( val, old, new )   OSAtomicCompareAndSwap32Barrier( old, new, val )
#define DKAtomicCmpAndSwapPtr( val, old, new )  OSAtomicCompareAndSwapPtrBarrier( old, new, val )

#elif DK_PLATFORM_ANDROID
#define DKAtomicIncrement32( ptr )              __sync_add_and_fetch( ptr, 1 )
#define DKAtomicDecrement32( ptr )              __sync_sub_and_fetch( ptr, 1 )
#define DKAtomicCmpAndSwap32( val, old, new )   __sync_bool_compare_and_swap( val, old, new )
#define DKAtomicCmpAndSwapPtr( val, old, new )  __sync_bool_compare_and_swap( val, old, new )

#elif DK_PLATFORM_LINUX
#define DKAtomicIncrement32( ptr )              __sync_add_and_fetch( ptr, 1 )
#define DKAtomicDecrement32( ptr )              __sync_sub_and_fetch( ptr, 1 )
#define DKAtomicCmpAndSwap32( val, old, new )   __sync_bool_compare_and_swap( val, old, new )
#define DKAtomicCmpAndSwapPtr( val, old, new )  __sync_bool_compare_and_swap( val, old, new )

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







