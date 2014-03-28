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

DKStringRef   DKStringCreate( const char * str );
DKStringRef   DKStringCreateCopy( DKStringRef srcString );
DKStringRef   DKStringCreateNoCopy( const char * str );

DKMutableStringRef DKStringCreateMutable( void );
DKMutableStringRef DKStringCreateMutableCopy( DKStringRef srcString );

DKIndex     DKStringGetLength( DKStringRef ref );
DKIndex     DKStringGetByteLength( DKStringRef ref );

const char* DKStringGetCStringPtr( DKStringRef ref );
DKIndex     DKStringGetSubstring( DKStringRef ref, DKRange range, char * buffer );
DKStringRef DKStringCopySubstring( DKStringRef ref, DKRange range );

DKRange     DKStringGetRangeOfString( DKStringRef ref, DKStringRef str, DKIndex startLoc );

void        DKStringSetString( DKMutableStringRef ref, DKStringRef str );

void        DKStringAppend( DKMutableStringRef ref, const char * str );
void        DKStringAppendString( DKMutableStringRef ref, DKStringRef str );
void        DKStringAppendFormat( DKMutableStringRef ref, DKStringRef format, ... );

void        DKStringReplaceSubstring( DKMutableStringRef ref, DKRange range, DKStringRef str );
void        DKStringDeleteSubstring( DKMutableStringRef ref, DKRange range );

int         DKStringSeek( DKStringRef ref, DKIndex offset, int origin );
DKIndex     DKStringTell( DKStringRef ref );
DKIndex     DKStringRead( DKStringRef ref, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKStringWrite( DKStringRef ref, const void * buffer, DKIndex size, DKIndex count );


#endif // _DK_DATA_H_


