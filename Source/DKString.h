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


DK_API DKClassRef  DKStringClass( void );
DK_API DKClassRef  DKConstantStringClass( void );
DK_API DKClassRef  DKMutableStringClass( void );

#define            DKEmptyString()         DKSTR( "" )
#define            DKMutableString()       DKAutorelease( DKNew( DKMutableStringClass() ) )

#define            DKStringWithString( str )               DKAutorelease( DKStringInitWithString( DKAlloc( DKStringClass() ), str ) )
#define            DKStringWithCString( cstr )             DKAutorelease( DKStringInitWithCString( DKAlloc( DKStringClass() ), cstr ) )
#define            DKStringWithCStringNoCopy( cstr )       DKAutorelease( DKStringInitWithCStringNoCopy( DKAlloc( DKStringClass() ), cstr ) )
#define            DKStringWithBytes( bytes, length )      DKAutorelease( DKStringInitWithBytes( DKAlloc( DKStringClass() ), bytes, length ) )
#define            DKStringWithFormat( fmt, ... )          DKAutorelease( DKStringInitWithFormat( DKAlloc( DKStringClass() ), fmt, __VA_ARGS__ ) )
#define            DKStringWithContentsOfFile( filename )  DKAutorelease( DKStringInitWithContentsOfFile( DKAlloc( DKStringClass() ), filename ) )

#define            DKNewMutableString()    DKNew( DKMutableStringClass() )

#define            DKNewStringWithString( str )                DKStringInitWithString( DKAlloc( DKStringClass() ), str )
#define            DKNewStringWithCString( cstr )              DKStringInitWithCString( DKAlloc( DKStringClass() ), cstr )
#define            DKNewStringWithCStringNoCopy( cstr )        DKStringInitWithCStringNoCopy( DKAlloc( DKStringClass() ), cstr )
#define            DKNewStringWithBytes( bytes, length )       DKStringInitWithBytes( DKAlloc( DKStringClass() ), bytes, length )
#define            DKNewStringWithFormat( fmt, ... )           DKStringInitWithFormat( DKAlloc( DKStringClass() ), fmt, __VA_ARGS__ )
#define            DKNewStringWithContentsOfFile( filename )   DKStringInitWithContentsOfFile( DKAlloc( DKStringClass() ), filename )

DK_API DKObjectRef DKStringInitWithString( DKObjectRef _self, DKStringRef other );
DK_API DKObjectRef DKStringInitWithCString( DKObjectRef _self, const char * cstr );
DK_API DKObjectRef DKStringInitWithCStringNoCopy( DKObjectRef _self, const char * cstr );
DK_API DKObjectRef DKStringInitWithBytes( DKObjectRef _self, const void * bytes, DKIndex length );
DK_API DKObjectRef DKStringInitWithFormat( DKObjectRef _self, const char * format, ... );
DK_API DKObjectRef DKStringInitWithContentsOfFile( DKObjectRef _self, DKStringRef filename );

DK_API DKStringRef DKStringMakeImmutable( DKMutableStringRef _self );

// Unlike DKCopy, which may retain and return an immutable object as an optimization,
// DKStringCopy and DKStringMutableCopy always return a new copy of a string's contents.
DK_API DKStringRef DKStringCopy( DKStringRef _self );
DK_API DKMutableStringRef DKStringMutableCopy( DKStringRef _self );

DK_API bool        DKStringEqual( DKStringRef _self, DKObjectRef other );
DK_API int         DKStringCompare( DKStringRef _self, DKObjectRef other );
DK_API DKHashCode  DKStringHash( DKStringRef _self );

DK_API bool        DKStringEqualToString( DKStringRef _self, DKStringRef other );
DK_API int         DKStringCompareString( DKStringRef _self, DKStringRef other );
DK_API int         DKStringCompareCString( DKStringRef _self, const char * cstr );

DK_API bool        DKStringIsEmptyString( DKStringRef _self );
DK_API bool        DKStringHasPrefix( DKStringRef _self, DKStringRef other );
DK_API bool        DKStringHasSuffix( DKStringRef _self, DKStringRef other );
DK_API bool        DKStringHasSubstring( DKStringRef _self, DKStringRef other );

DK_API DKIndex     DKStringGetLength( DKStringRef _self );
DK_API DKIndex     DKStringGetByteLength( DKStringRef _self );
DK_API void        DKStringSetByteLength( DKMutableStringRef _self, DKIndex length );

DK_API const char * DKStringGetCStringPtr( DKStringRef _self );
DK_API DKChar32    DKStringGetCharacterAtIndex( DKStringRef _self, DKIndex index, DKChar8 * utf8 );

DK_API const void * DKStringGetBytePtr( DKStringRef _self, DKIndex index );
DK_API void *      DKStringGetMutableBytePtr( DKStringRef _self, DKIndex index );

// Substrings
DK_API DKStringRef DKStringCopySubstring( DKStringRef _self, DKRange range );
DK_API DKStringRef DKStringCopySubstringFromIndex( DKStringRef _self, DKIndex index );
DK_API DKStringRef DKStringCopySubstringToIndex( DKStringRef _self, DKIndex index );

#define            DKStringGetSubstring( str, range )      DKAutorelease( DKStringCopySubstring( (str), (range) ) )
#define            DKStringGetSubstringFromIndex( str, i ) DKAutorelease( DKStringCopySubstringFromIndex( (str), (i) ) )
#define            DKStringGetSubstringToIndex( str, i )   DKAutorelease( DKStringCopySubstringToIndex( (str), (i) ) )

DK_API DKRange     DKStringGetRangeOfString( DKStringRef _self, DKStringRef str, DKIndex startLoc );

// Separating and concatenating strings
DK_API DKListRef   DKStringSplit( DKStringRef _self, DKStringRef separator );
DK_API DKListRef   DKStringWrap( DKStringRef _self, size_t glyphsPerLine );
DK_API DKStringRef DKStringCombine( DKListRef list, DKStringRef separator );

// Trimming
DK_API DKStringRef DKStringByTrimmingWhitespace( DKStringRef _self );

// Filtering
DK_API typedef DKFilterAction (*ZLStringFilterFunction)( DKIndex sourceIndex, DKIndex destinationIndex, DKChar8 utf8, DKChar32 utf32, void * context );
DK_API DKStringRef DKStringByFilteringString( DKStringRef _self, ZLStringFilterFunction filter, void * context );

// Modifying mutable strings
DK_API void        DKStringSetString( DKMutableStringRef _self, DKStringRef str );
DK_API void        DKStringSetCString( DKMutableStringRef _self, const char * cstr );
DK_API void        DKStringSetBytes( DKMutableStringRef _self, const void * bytes, DKIndex length );

DK_API void        DKStringAppendString( DKMutableStringRef _self, DKStringRef str );
DK_API void        DKStringAppendFormat( DKMutableStringRef _self, const char * format, ... );
DK_API void        DKStringAppendCString( DKMutableStringRef _self, const char * cstr );
DK_API void        DKStringAppendBytes( DKMutableStringRef _self, const void * bytes, DKIndex length );

DK_API void        DKStringReplaceSubstring( DKMutableStringRef _self, DKRange range, DKStringRef str );
DK_API void        DKStringReplaceOccurrencesOfString( DKMutableStringRef _self, DKStringRef pattern, DKStringRef replacement );

DK_API void        DKStringEscape( DKMutableStringRef _self, DKDictionaryRef patterns );
DK_API void        DKStringUnescape( DKMutableStringRef _self, DKDictionaryRef patterns );

DK_API void        DKStringDeleteSubstring( DKMutableStringRef _self, DKRange range );

// Stream interface
DK_API int         DKStringSeek( DKStringRef _self, long offset, int origin );
DK_API long        DKStringTell( DKStringRef _self );
DK_API size_t      DKStringRead( DKStringRef _self, void * buffer, size_t size, size_t count );
DK_API size_t      DKStringWrite( DKMutableStringRef _self, const void * buffer, size_t size, size_t count );

// Paths
DK_API bool        DKStringIsAbsolutePath( DKStringRef _self );
DK_API bool        DKStringHasPathExtension( DKStringRef _self, DKStringRef extension );

DK_API DKStringRef DKStringGetLastPathComponent( DKStringRef _self );
DK_API DKStringRef DKStringGetPathExtension( DKStringRef _self );

DK_API DKStringRef DKStringByAppendingPathComponent( DKStringRef _self, DKStringRef pathComponent );
DK_API DKStringRef DKStringByAppendingPathExtension( DKStringRef _self, DKStringRef pathExtension );
DK_API DKStringRef DKStringByDeletingLastPathComponent( DKStringRef _self );
DK_API DKStringRef DKStringByDeletingPathExtension( DKStringRef _self );

DK_API void        DKStringAppendPathComponent( DKMutableStringRef _self, DKStringRef pathComponent );
DK_API void        DKStringDeleteLastPathComponent( DKMutableStringRef _self );
DK_API void        DKStringAppendPathExtension( DKMutableStringRef _self, DKStringRef extension );
DK_API void        DKStringDeletePathExtension( DKMutableStringRef _self );

DK_API void        DKStringStandardizePath( DKMutableStringRef _self );

// URLs
DK_API DKDictionaryRef DKStringURLPercentEscapePatterns( void );

DK_API DKStringRef DKStringByAddingURLPercentEscapes( DKStringRef _self );
DK_API DKStringRef DKStringByRemovingURLPercentEscapes( DKStringRef _self );

DK_API DKDictionaryRef DKStringSplitQueryParameters( DKStringRef _self );
DKStringRef DKStringCombineQueryParameters( DKDictionaryRef queryParameters );

DK_API DKDictionaryRef DKStringSplitCSSStyles( DKStringRef _self );

// Conversions
DK_API bool        DKStringGetBool( DKStringRef _self );
DK_API int32_t     DKStringGetInt32( DKStringRef _self );
DK_API int64_t     DKStringGetInt64( DKStringRef _self );
DK_API float       DKStringGetFloat( DKStringRef _self );
DK_API double      DKStringGetDouble( DKStringRef _self );



#endif // _DK_DATA_H_


