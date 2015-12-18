/*****************************************************************************************

  DKString.h

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

#ifndef _DK_STRING_H_
#define _DK_STRING_H_

#include "DKRuntime.h"


// typedef const struct DKString * DKStringRef; -- Declared in DKPlatform.h
typedef struct DKString * DKMutableStringRef;


DKClassRef  DKStringClass( void );
DKClassRef  DKConstantStringClass( void );
DKClassRef  DKMutableStringClass( void );

#define     DKEmptyString()         DKSTR( "" )
#define     DKMutableString()       DKAutorelease( DKNew( DKMutableStringClass() ) )

#define     DKStringWithString( str )               DKAutorelease( DKStringInitWithString( DKAlloc( DKStringClass() ), str ) )
#define     DKStringWithCString( cstr )             DKAutorelease( DKStringInitWithCString( DKAlloc( DKStringClass() ), cstr ) )
#define     DKStringWithCStringNoCopy( cstr )       DKAutorelease( DKStringInitWithCStringNoCopy( DKAlloc( DKStringClass() ), cstr ) )
#define     DKStringWithFormat( fmt, ... )          DKAutorelease( DKStringInitWithFormat( DKAlloc( DKStringClass() ), fmt, __VA_ARGS__ ) )
#define     DKStringWithContentsOfFile( filename )  DKAutorelease( DKStringInitWithContentsOfFile( DKAlloc( DKStringClass() ), filename ) )

#define     DKNewMutableString()    DKNew( DKMutableStringClass() )

DKObjectRef DKStringInitWithString( DKStringRef _self, DKStringRef other );
DKObjectRef DKStringInitWithCString( DKStringRef _self, const char * cstr );
DKObjectRef DKStringInitWithCStringNoCopy( DKStringRef _self, const char * cstr );
DKObjectRef DKStringInitWithFormat( DKStringRef _self, const char * format, ... );
DKObjectRef DKStringInitWithContentsOfFile( DKStringRef _self, DKStringRef filename );

DKStringRef DKStringMakeImmutable( DKMutableStringRef _self );

DKStringRef DKStringCopy( DKStringRef _self );
DKMutableStringRef DKStringMutableCopy( DKStringRef _self );

bool        DKStringEqual( DKStringRef _self, DKObjectRef other );
int         DKStringCompare( DKStringRef _self, DKObjectRef other );
DKHashCode  DKStringHash( DKStringRef _self );

bool        DKStringEqualToString( DKStringRef _self, DKStringRef other );
int         DKStringCompareString( DKStringRef _self, DKStringRef other );

bool        DKStringHasPrefix( DKStringRef _self, DKStringRef other );
bool        DKStringHasSuffix( DKStringRef _self, DKStringRef other );
bool        DKStringHasSubstring( DKStringRef _self, DKStringRef other );

DKIndex     DKStringGetLength( DKStringRef _self );
DKIndex     DKStringGetByteLength( DKStringRef _self );

const char * DKStringGetCStringPtr( DKStringRef _self );

// Substrings
DKStringRef DKStringCopySubstring( DKStringRef _self, DKRange range );
DKStringRef DKStringCopySubstringFromIndex( DKStringRef _self, DKIndex index );
DKStringRef DKStringCopySubstringToIndex( DKStringRef _self, DKIndex index );

DKRange     DKStringGetRangeOfString( DKStringRef _self, DKStringRef str, DKIndex startLoc );

// Separating and concatenating strings
DKListRef   DKStringSplit( DKStringRef _self, DKStringRef separator );
DKStringRef DKStringCombine( DKListRef list, DKStringRef separator );

// Modifying mutable strings
void        DKStringSetString( DKMutableStringRef _self, DKStringRef str );
void        DKStringSetCString( DKMutableStringRef _self, const char * cstr );

void        DKStringAppendString( DKMutableStringRef _self, DKStringRef str );
void        DKStringAppendFormat( DKMutableStringRef _self, const char * format, ... );
void        DKStringAppendCString( DKMutableStringRef _self, const char * cstr );

void        DKStringReplaceSubstring( DKMutableStringRef _self, DKRange range, DKStringRef str );
void        DKStringReplaceOccurrencesOfString( DKMutableStringRef _self, DKStringRef pattern, DKStringRef replacement );

void        DKStringEscape( DKMutableStringRef _self, DKDictionaryRef patterns );
void        DKStringUnescape( DKMutableStringRef _self, DKDictionaryRef patterns );

void        DKStringDeleteSubstring( DKMutableStringRef _self, DKRange range );

// Stream interface
int         DKStringSeek( DKStringRef _self, DKIndex offset, int origin );
DKIndex     DKStringTell( DKStringRef _self );
DKIndex     DKStringRead( DKStringRef _self, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKStringWrite( DKMutableStringRef _self, const void * buffer, DKIndex size, DKIndex count );

// Paths
bool        DKStringIsAbsolutePath( DKStringRef _self );
DKStringRef DKStringCopyLastPathComponent( DKStringRef _self );
DKStringRef DKStringCopyPathExtension( DKStringRef _self );
void        DKStringAppendPathComponent( DKMutableStringRef _self, DKStringRef pathComponent );
void        DKStringRemoveLastPathComponent( DKMutableStringRef _self );
void        DKStringAppendPathExtension( DKMutableStringRef _self, DKStringRef extension );
void        DKStringRemovePathExtension( DKMutableStringRef _self );
void        DKStringStandardizePath( DKMutableStringRef _self );

// URLs
DKDictionaryRef DKStringURLPercentEscapePatterns( void );

DKStringRef DKStringByAddingURLPercentEscapes( DKStringRef _self );
DKStringRef DKStringByRemovingURLPercentEscapes( DKStringRef _self );

DKDictionaryRef DKStringSplitQueryParameters( DKStringRef _self );
DKStringRef DKStringCombineQueryParameters( DKDictionaryRef queryParameters );

// Conversions
bool        DKStringGetBool( DKStringRef _self );
int32_t     DKStringGetInt32( DKStringRef _self );
int64_t     DKStringGetInt64( DKStringRef _self );
float       DKStringGetFloat( DKStringRef _self );
double      DKStringGetDouble( DKStringRef _self );



#endif // _DK_DATA_H_


