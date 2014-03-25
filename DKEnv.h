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
typedef uintptr_t DKHashIndex;


// Ranges
typedef struct
{
    DKIndex location;
    DKIndex length;

} DKRange;

#define DKRangeMake( loc, len )     (const DKRange){ loc, len }
#define DKRangeEnd( range )         (((range).location) + ((range).length))

enum
{
    DKNotFound = -1,
};




// Error Reporting =======================================================================
void   DKSetPrintCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void   DKSetWarningCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void   DKSetErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );
void   DKSetFatalErrorCallback( int (*callback)( const char * format, va_list arg_ptr ) );

// Print a message. This is ignored in non-debug builds. Objects can be printed with "%@".
int    _DKPrintf( const char * format, ... );

// Print a warning. This is ignored in non-debug builds if DK_WARNINGS_AS_ERRORS is not
// defined. Objects can be printed with "%@".
int    _DKWarning( const char * format, ... );

// Print a error. In a debug build execution is halted with assert(0). In a non-debug
// build the program will continue running. Objects can be printed with "%@".
int    _DKError( const char * format, ... );

// Print a error. In a debug build execution is halted with assert(0). In a non-debug
// build the program is halted with abort(). Objects cannot be printed.
int    _DKFatalError( const char * format, ... );

#if defined(NDEBUG)
#define DKPrintf( ... )
#else
#define DKPrintf( ... )     _DKPrintf( __VA_ARGS__ )
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
#define DKAssert( x )               if( !(x) ) _DKFatalError( "Assertion Failed: %s", #x )
#define DKAssertMsg( x, msg, ... )  if( !(x) ) _DKFatalError( msg, __VA_ARGS__ )
#endif



// Memory Allocation =====================================================================
typedef void * (*dk_malloc_callback)( size_t size );
typedef void (*dk_free_callback)( void * ptr );

void   DKSetExternalAllocator( dk_malloc_callback _malloc, dk_free_callback _free );

void * dk_malloc( size_t size );
void   dk_free( void * ptr );




// Other Utilities =======================================================================
DKHashIndex DKStrHash( const char * str );
DKHashIndex DKMemHash( const void * buffer, size_t buffer_size );

void DKShuffle( uintptr_t array[], DKIndex count );


#endif // _DK_ENV_H_







