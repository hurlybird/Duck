//
//  DKEnv.h
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

#include "DKEnvApple.h"


// Basic Types ===========================================================================

// Objects
typedef const void * DKTypeRef;


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
#define DKRangeCheck( range, len )  (((range).location >= 0) && ((range).length >= 0) && (DKRangeEnd(range) <= len))


// Callback Types
typedef int  (*DKCompareFunction)( DKTypeRef a, DKTypeRef b );




// Error Reporting =======================================================================
void   DKSetDebugCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void   DKSetWarningCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void   DKSetErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void   DKSetFatalErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );

// Print a message. This is ignored in non-debug builds. Object descriptions can be
// printed using the Foundation/CoreFoundation idiom "%@".
int    _DKDebug( const char * format, ... );

// Print a warning. This is ignored in non-debug builds if DK_WARNINGS_AS_ERRORS is not
// defined.
int    _DKWarning( const char * format, ... );

// Print a error. In a debug build execution is halted with assert(0). In a non-debug
// build the program will continue running.
int    _DKError( const char * format, ... );

// Print a error. In a debug build execution is halted with assert(0). In a non-debug
// build the program is halted with abort().
int    _DKFatalError( const char * format, ... ) __attribute__((analyzer_noreturn));


#ifdef NDEBUG
    #ifndef DK_RUNTIME_WARNINGS
    #define DK_RUNTIME_WARNINGS 0
    #endif

    #ifndef DK_RUNTIME_ASSERTIONS
    #define DK_RUNTIME_ASSERTIONS 0
    #endif

    #ifndef DK_RUNTIME_TYPE_CHECKS
    #define DK_RUNTIME_TYPE_CHECKS 1
    #endif

    #ifndef DK_RUNTIME_RANGE_CHECKS
    #define DK_RUNTIME_RANGE_CHECKS 1
    #endif

    #ifndef DK_RUNTIME_INTEGRITY_CHECKS
    #define DK_RUNTIME_INTEGRITY_CHECKS 0
    #endif
#else
    #ifndef DK_RUNTIME_WARNINGS
    #define DK_RUNTIME_WARNINGS 1
    #endif

    #ifndef DK_RUNTIME_ASSERTIONS
    #define DK_RUNTIME_ASSERTIONS 1
    #endif

    #ifndef DK_RUNTIME_TYPE_CHECKS
    #define DK_RUNTIME_TYPE_CHECKS  1
    #endif

    #ifndef DK_RUNTIME_RANGE_CHECKS
    #define DK_RUNTIME_RANGE_CHECKS 1
    #endif

    #ifndef DK_RUNTIME_INTEGRITY_CHECKS
    #define DK_RUNTIME_INTEGRITY_CHECKS 0
    #endif
#endif


// Debug Messages
#ifdef NDEBUG
#define DKDebug( ... )
#else
#define DKDebug( ... )      _DKDebug( __VA_ARGS__ )
#endif


// Warnings
#if DK_RUNTIME_WARNINGS
#define DKWarning( ... )    _DKWarning( __VA_ARGS__ )
#else
#define DKWarning( ... )
#endif


// Errors
#define DKError( ... )      _DKError( __VA_ARGS__ )
#define DKFatalError( ... ) _DKFatalError( __VA_ARGS__ )


// Assertions
#if DK_RUNTIME_ASSERTIONS
#define DKAssert( x )               if( !(x) ) _DKFatalError( "%s %d: Failed Assert( %s )\n", __FILE__, __LINE__, #x )
#define DKAssertMsg( x, msg, ... )  if( !(x) ) _DKFatalError( msg, __VA_ARGS__ )
#else
#define DKAssert( x )
#define DKAssertMsg( x, ... )
#endif


// Type Checks
#if DK_RUNTIME_TYPE_CHECKS
#define DKVerifyKindOfClass( ref, cls, ... )                                            \
    if( !DKIsKindOfClass( ref, cls ) )                                                  \
    {                                                                                   \
        _DKError( "%s: Expected kind of class %s, received %s\n",                       \
            __func__, DKGetClassName( cls ), DKGetClassName( ref ) );                   \
        return __VA_ARGS__;                                                             \
    }

#define DKVerifyMemberOfClass( ref, cls, ... )                                          \
    if( !DKIsKindOfClass( ref, cls ) )                                                  \
    {                                                                                   \
        _DKError( "%s: Expected member of class %s, received %s\n",                     \
            __func__, DKGetClassName( cls ), DKGetClassName( ref ) );                   \
        return __VA_ARGS__;                                                             \
    }
#else
#define DKVerifyKindOfClass( ref, cls, ... )
#define DKVerifyMemberOfClass( ref, cls, ... )
#endif


// Range Checks
#if DK_RUNTIME_RANGE_CHECKS
#define DKVerifyIndex( range, len, ... )                                                \
    if( ((index) < 0) || ((index) >= len) )                                             \
    {                                                                                   \
        _DKError( "%s: Index %ld is outside 0,%ld\n", __func__, (index), len );         \
        return __VA_ARGS__;                                                             \
    }

#define DKVerifyRange( range, len, ... )                                                \
    if( !DKRangeCheck( range, len ) )                                                   \
    {                                                                                   \
        _DKError( "%s: Range %ld,%ld is outside 0,%ld\n",                               \
            __func__, (range).location, (range).length, len );                          \
        return __VA_ARGS__;                                                             \
    }
#else
#define DKVerifyIndex( index, len, ... )
#define DKVerifyRange( range, len, ... )
#endif




// Memory Allocation =====================================================================
typedef void * (*dk_malloc_callback)( size_t size );
typedef void (*dk_free_callback)( void * ptr );

void   DKSetExternalAllocator( dk_malloc_callback _malloc, dk_free_callback _free );

void * dk_malloc( size_t size );
void   dk_free( void * ptr );




// Other Utilities =======================================================================

// Basic hashing
DKHashCode dk_strhash( const char * str );
DKHashCode dk_memhash( const void * buffer, size_t buffer_size );

// Time in seconds since midnight January 1, 1970 (i.e. gettimeofday())
double dk_time( void );

// Shuffle an array of pointer-sized integers
void dk_shuffle( uintptr_t array[], DKIndex count );

// UTF8 aware versions of standard string functions
const char * dk_ustrchr( const char * str, int ch );
const char * dk_ustrrchr( const char * str, int ch );

int dk_ustrcmp( const char * str1, const char * str2, int options );

size_t dk_ustrlen( const char * str );

const char * dk_ustridx( const char * str, size_t idx );
size_t dk_ustrscan( const char * str, char32_t * ch );


#endif // _DK_ENV_H_







