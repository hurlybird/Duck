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

// Object Types
typedef const void * DKTypeRef;


// Callback Types
typedef int  (*DKCompareFunction)( DKTypeRef a, DKTypeRef b );


// Index Types
typedef intptr_t DKIndex;
typedef uintptr_t DKHashCode;


// Ranges
typedef struct
{
    DKIndex location;
    DKIndex length;

} DKRange;

#define DKRangeMake( loc, len )     (const DKRange){ loc, len }
#define DKRangeEnd( range )         (((range).location) + ((range).length))

// False if the range is outside 0..len OR is an empty sequence at len + 1
#define DKRangeCheck( range, len )  (((range).location >= 0) && ((range).length >= 0) && (DKRangeEnd(range) <= len))

enum
{
    DKNotFound = -1,
};




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

#if defined(NDEBUG)
#define DKDebug( ... )
#else
#define DKDebug( ... )      _DKDebug( __VA_ARGS__ )
#endif

#if defined(NDEBUG) && !defined(DK_WARNINGS_AS_ERRORS)
#define DKWarning( ... )
#else
#define DKWarning( ... )    _DKWarning( __VA_ARGS__ )
#endif

#define DKError( ... )      _DKError( __VA_ARGS__ )
#define DKFatalError( ... ) _DKFatalError( __VA_ARGS__ )


#if defined(NDEBUG) && !defined(DK_ASSERTIONS_AS_ERRORS)
#define DKAssert( x )
#define DKAssertMsg( x, ... )
#else
#define DKAssert( x )               if( !(x) ) _DKFatalError( "%s %d: Failed Assert( %s )\n", __FILE__, __LINE__, #x )
#define DKAssertMsg( x, msg, ... )  if( !(x) ) _DKFatalError( msg, __VA_ARGS__ )
#endif


#ifdef NDEBUG
    #ifndef DK_RUNTIME_TYPE_CHECKS
    #define DK_RUNTIME_TYPE_CHECKS  0
    #endif

    #ifndef DK_RUNTIME_RANGE_CHECKS
    #define DK_RUNTIME_RANGE_CHECKS 0
    #endif
#else
    #ifndef DK_RUNTIME_TYPE_CHECKS
    #define DK_RUNTIME_TYPE_CHECKS  1
    #endif

    #ifndef DK_RUNTIME_RANGE_CHECKS
    #define DK_RUNTIME_RANGE_CHECKS 1
    #endif
#endif



#if DK_RUNTIME_TYPE_CHECKS
#define DKVerifyKindOfClass( ref, cls, ... )                                            \
    if( !DKIsKindOfClass( ref, cls ) )                                                  \
    {                                                                                   \
        _DKFatalError( "%s: Expected kind of class %s, received %s\n",               \
            __func__, DKGetClassName( cls ), DKGetClassName( ref ) );         \
        return __VA_ARGS__;                                                             \
    }

#define DKVerifyMemberOfClass( ref, cls, ... )                                          \
    if( !DKIsKindOfClass( ref, cls ) )                                                  \
    {                                                                                   \
        _DKFatalError( "%s: Expected member of class %s, received %s\n",             \
            __func__, DKGetClassName( cls ), DKGetClassName( ref ) );         \
        return __VA_ARGS__;                                                             \
    }
#else
#define DKVerifyKindOfClass( ref, cls, ... )
#define DKVerifyMemberOfClass( ref, cls, ... )
#endif



#if DK_RUNTIME_RANGE_CHECKS
#define DKVerifyRange( range, length, ... )                                             \
    if( !DKRangeCheck( range, length ) )                                                \
    {                                                                                   \
        _DKFatalError( "%s: Range %d..%d is outside %d..%d\n",                       \
            __func__, range.location, DKRangeEnd( range ), 0, length );       \
        return __VA_ARGS__;                                                             \
    }
#else
#define DKVerifyRange( range, length, ... )
#endif




// Memory Allocation =====================================================================
typedef void * (*dk_malloc_callback)( size_t size );
typedef void (*dk_free_callback)( void * ptr );

void   DKSetExternalAllocator( dk_malloc_callback _malloc, dk_free_callback _free );

void * dk_malloc( size_t size );
void   dk_free( void * ptr );




// Other Utilities =======================================================================
DKHashCode DKStrHash( const char * str );
DKHashCode DKMemHash( const void * buffer, size_t buffer_size );

void DKShuffle( uintptr_t array[], DKIndex count );


#endif // _DK_ENV_H_







