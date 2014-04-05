//
//  DKPlatform.h
//  Duck
//
//  Created by Derek Nylen on 11-12-07.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#ifndef _DK_ENV_H_
#define _DK_ENV_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "DKConfig.h"

#if DK_PLATFORM_APPLE
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
typedef const struct DKString * DKStringRef;
typedef const void * DKListRef;


// Indexes
typedef intptr_t  DKIndex;    // Indexes are basically a signed size_t
typedef uintptr_t DKHashCode; // Pointers can be used as hash codes

enum
{
    DKNotFound = -1,
};


// UTF32 character - C11 provides this type
#ifndef char32_t
typedef int32_t char32_t;
#endif


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
typedef int  (*DKCompareFunction)( DKObjectRef a, DKObjectRef b );


// Path Constants
#define DKPathComponentSeparator    '/'
#define DKPathExtensionSeparator    '.'




// Error Reporting =======================================================================

// Set external handlers for debug, warning and error messages
void   DKSetDebugCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void   DKSetWarningCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void   DKSetErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void   DKSetFatalErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );


// Print a debug message. This is ignored in non-debug builds. Object descriptions can be
// printed using the Foundation/CoreFoundation idiom "%@".
int    _DKDebug( const char * format, ... );

#ifdef NDEBUG
#define DKDebug( ... )
#else
#define DKDebug( ... )      _DKDebug( __VA_ARGS__ )
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
#define DKAssertKindOfClass( _self, cls, ... )
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
                __func__, (sel)->suid, DKGetClassName( _self ) );                       \
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
#define DKAtomicCmpAndSwapPtr( val, old, new )  OSAtomicCompareAndSwapPtrBarrier( old, new, val )

#elif DK_PLATFORM_ANDROID
#define DKAtomicIncrement32( ptr )              __sync_add_and_fetch( ptr, 1 )
#define DKAtomicDecrement32( ptr )              __sync_sub_and_fetch( ptr, 1 )
#define DKAtomicCmpAndSwapPtr( val, old, new )  __sync_bool_compare_and_swap( val, old, new )

#elif DK_PLATFORM_LINUX
#define DKAtomicIncrement32( ptr )              __sync_add_and_fetch( ptr, 1 )
#define DKAtomicDecrement32( ptr )              __sync_sub_and_fetch( ptr, 1 )
#define DKAtomicCmpAndSwapPtr( val, old, new )  __sync_bool_compare_and_swap( val, old, new )

#endif





// Other Utilities =======================================================================

// Basic hashing
DKHashCode dk_strhash( const char * str );
DKHashCode dk_memhash( const void * buffer, size_t buffer_size );

// Time in seconds since midnight January 1, 1970 (i.e. gettimeofday())
double dk_time( void );



#endif // _DK_ENV_H_







