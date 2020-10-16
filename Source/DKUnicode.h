/*****************************************************************************************

  DKUnicode.h

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

#ifndef _DK_UNICODE_H_
#define _DK_UNICODE_H_

#include "DKPlatform.h"


// UTF8 aware versions of standard string functions

// strchr
DK_API const char * dk_ustrchr( const char * str, int ch );

// strrchr
DK_API const char * dk_ustrrchr( const char * str, int ch );

// strstr
DK_API const char * dk_ustrstr( const char * str1, const char * str2 );

// Works like strstr, but returns the range (in unicode characters) of str2 in str1
DK_API DKRange dk_ustrstr_range( const char * str1, const char * str2 );

// strcmp
DK_API int dk_ustrcmp( const char * str1, const char * str2 );

// strlen
DK_API size_t dk_ustrlen( const char * str );
DK_API size_t dk_ustrnlen( const char * str, size_t n );

// Returns a pointer to the start of the unicode character at index 'idx'
DK_API const char * dk_ustridx( const char * str, size_t idx );

// Returns a pointer to the start of the unicode character at reverse index 'idx'
DK_API const char * dk_ustrridx( const char * str, size_t idx );

// Scans one character from 'str' into 'ch' and returns the number of bytes read
DK_API size_t dk_ustrscan( const char * str, DKChar32 * ch );

// Writes the character 'ch' into 'str' and returns the number of bytes written
DK_API size_t dk_ustrwrite( DKChar32 ch, char * str, size_t str_size );

#endif // _DK_UNICODE_H_
