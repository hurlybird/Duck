//
//  DKString.h
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_STRING_H_
#define _DK_STRING_H_

#include "DKRuntime.h"


// Define a constant string with a compile-time constant C string
#define DKSTR( s )      __DKStringDefineConstantString( "" s "" )


typedef DKTypeRef DKStringRef;
typedef DKTypeRef DKMutableStringRef;


DKTypeRef   DKStringClass( void );
DKTypeRef   DKMutableStringClass( void );

DKStringRef DKStringCreate( void );
DKStringRef DKStringCreateCopy( DKStringRef srcString );
DKStringRef DKStringCreateWithCString( const char * str );
DKStringRef DKStringCreateWithCStringNoCopy( const char * str );

DKMutableStringRef DKStringCreateMutable( void );
DKMutableStringRef DKStringCreateMutableCopy( DKStringRef srcString );

int         DKStringEqual( DKStringRef a, DKTypeRef b );
int         DKStringCompare( DKStringRef a, DKStringRef b );
DKHashCode  DKStringHash( DKStringRef ref );

DKIndex     DKStringGetLength( DKStringRef ref );
DKIndex     DKStringGetByteLength( DKStringRef ref );

const char * DKStringGetCStringPtr( DKStringRef ref );

// Substrings
DKStringRef DKStringCopySubstring( DKStringRef ref, DKRange range );
DKStringRef DKStringCopySubstringFromIndex( DKStringRef ref, DKIndex index );
DKStringRef DKStringCopySubstringToIndex( DKStringRef ref, DKIndex index );

DKRange     DKStringGetRangeOfString( DKStringRef ref, DKStringRef str, DKIndex startLoc );

// Modifying mutable strings
void        DKStringSetString( DKMutableStringRef ref, DKStringRef str );

void        DKStringAppendString( DKMutableStringRef ref, DKStringRef str );
void        DKStringAppendFormat( DKMutableStringRef ref, DKStringRef format, ... );

void        DKStringReplaceSubstring( DKMutableStringRef ref, DKRange range, DKStringRef str );
void        DKStringDeleteSubstring( DKMutableStringRef ref, DKRange range );

// Stream interface
int         DKStringSeek( DKStringRef ref, DKIndex offset, int origin );
DKIndex     DKStringTell( DKStringRef ref );
DKIndex     DKStringRead( DKStringRef ref, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKStringWrite( DKMutableStringRef ref, const void * buffer, DKIndex size, DKIndex count );

// Paths
// Do stuff here...

// Define a constant string. Constant strings require external storage so unless you
// know what you're doing, use the DKSTR macro instead of calling this directly
DKStringRef __DKStringDefineConstantString( const char * str );


#endif // _DK_DATA_H_


