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

#define     DKStringCreateEmpty()    DKSTR( "" )
#define     DKStringCreateMutable()  DKInit( DKAlloc( DKMutableStringClass(), 0 ) )

#define     DKStringCreateWithString( _class, str )     DKStringInitWithString( DKAlloc( _class, 0 ), str )
#define     DKStringCreateWithCString( _class, cstr )   DKStringInitWithCString( DKAlloc( _class, 0 ), cstr )
#define     DKStringCreateWithCStringNoCopy( cstr )     DKStringInitWithCStringNoCopy( DKAlloc( DKStringClass(), 0 ), cstr )

void *      DKStringInitWithString( DKStringRef _self, DKStringRef other );
void *      DKStringInitWithCString( DKStringRef _self, const char * cstr );
void *      DKStringInitWithCStringNoCopy( DKStringRef _self, const char * cstr );

DKStringRef DKStringCopy( DKStringRef _self );
DKMutableStringRef DKStringMutableCopy( DKStringRef _self );

bool        DKStringEqual( DKStringRef _self, DKObjectRef other );
int         DKStringCompare( DKStringRef _self, DKStringRef other );
DKHashCode  DKStringHash( DKStringRef _self );

bool        DKStringEqualToString( DKStringRef _self, DKStringRef other );
int         DKStringCompareString( DKStringRef _self, DKStringRef other );

DKIndex     DKStringGetLength( DKStringRef _self );
DKIndex     DKStringGetByteLength( DKStringRef _self );

const char * DKStringGetCStringPtr( DKStringRef _self );

// Substrings
DKStringRef DKStringCopySubstring( DKStringRef _self, DKRange range );
DKStringRef DKStringCopySubstringFromIndex( DKStringRef _self, DKIndex index );
DKStringRef DKStringCopySubstringToIndex( DKStringRef _self, DKIndex index );

DKRange     DKStringGetRangeOfString( DKStringRef _self, DKStringRef str, DKIndex startLoc );

// Separating and concatenating strings
DKListRef   DKStringCreateListBySeparatingStrings( DKStringRef _self, DKStringRef separator );
DKStringRef DKStringCreateByCombiningStrings( DKListRef list, DKStringRef separator );

// Modifying mutable strings
void        DKStringSetString( DKMutableStringRef _self, DKStringRef str );

void        DKStringAppendString( DKMutableStringRef _self, DKStringRef str );
void        DKStringAppendFormat( DKMutableStringRef _self, const char * format, ... );

void        DKStringReplaceSubstring( DKMutableStringRef _self, DKRange range, DKStringRef str );
void        DKStringReplaceOccurrencesOfString( DKMutableStringRef _self, DKStringRef pattern, DKStringRef replacement );

void        DKStringDeleteSubstring( DKMutableStringRef _self, DKRange range );

// Stream interface
int         DKStringSeek( DKStringRef _self, DKIndex offset, int origin );
DKIndex     DKStringTell( DKStringRef _self );
DKIndex     DKStringRead( DKStringRef _self, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKStringWrite( DKMutableStringRef _self, const void * buffer, DKIndex size, DKIndex count );

// Paths
int         DKStringIsAbsolutePath( DKStringRef _self );
DKStringRef DKStringCopyLastPathComponent( DKStringRef _self );
DKStringRef DKStringCopyPathExtension( DKStringRef _self );
void        DKStringAppendPathComponent( DKMutableStringRef _self, DKStringRef pathComponent );
void        DKStringRemoveLastPathComponent( DKMutableStringRef _self );
void        DKStringAppendPathExtension( DKMutableStringRef _self, DKStringRef extension );
void        DKStringRemovePathExtension( DKMutableStringRef _self );
void        DKStringStandardizePath( DKMutableStringRef _self );


#endif // _DK_DATA_H_


