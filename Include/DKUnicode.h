//
//  DKUnicode.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-31.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_UNICODE_H_
#define _DK_UNICODE_H_

#include "DKPlatform.h"


// UTF8 aware versions of standard string functions

// strchr
const char * dk_ustrchr( const char * str, int ch );

// strrchr
const char * dk_ustrrchr( const char * str, int ch );

// strstr
const char * dk_ustrstr( const char * str1, const char * str2 );

// Works like strstr, but returns the range (in unicode characters) of str2 in str1
DKRange dk_ustrstr_range( const char * str1, const char * str2 );

// strcmp
int dk_ustrcmp( const char * str1, const char * str2 );

// strlen
size_t dk_ustrlen( const char * str );

// Returns a pointer to the start of the unicode character at index 'idx'
const char * dk_ustridx( const char * str, size_t idx );

// Scans one character from 'str' into 'ch' and returns the number of bytes read
size_t dk_ustrscan( const char * str, char32_t * ch );


#endif // _DK_UNICODE_H_
