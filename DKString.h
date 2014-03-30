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

const char* DKStringGetCStringPtr( DKStringRef ref );

DKStringRef DKStringCopySubstring( DKStringRef ref, DKRange range );
DKStringRef DKStringCopySubstringFromIndex( DKStringRef ref, DKIndex index );
DKStringRef DKStringCopySubstringToIndex( DKStringRef ref, DKIndex index );

DKRange     DKStringGetRangeOfString( DKStringRef ref, DKStringRef str, DKIndex startLoc );

void        DKStringSetString( DKMutableStringRef ref, DKStringRef str );

/*
void        DKStringAppend( DKMutableStringRef ref, const char * str );
void        DKStringAppendString( DKMutableStringRef ref, DKStringRef str );
void        DKStringAppendFormat( DKMutableStringRef ref, DKStringRef format, ... );

void        DKStringReplaceSubstring( DKMutableStringRef ref, DKRange range, DKStringRef str );
void        DKStringDeleteSubstring( DKMutableStringRef ref, DKRange range );

int         DKStringSeek( DKStringRef ref, DKIndex offset, int origin );
DKIndex     DKStringTell( DKStringRef ref );
DKIndex     DKStringRead( DKStringRef ref, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKStringWrite( DKStringRef ref, const void * buffer, DKIndex size, DKIndex count );
*/

#endif // _DK_DATA_H_


