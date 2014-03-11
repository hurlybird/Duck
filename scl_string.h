//
//  scl_string.h
//  scl
//
//  Created by Derek Nylen on 2014-02-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _SCL_STRING_H_
#define _SCL_STRING_H_

#include "scl_value.h"


typedef scl_value scl_string;


// Wrap a static string in an scl_value
#define SCLSTR( s )     &(scl_string){ sizeof(s), SCL_VALUE_EXTERNAL | SCL_VALUE_STATIC, { s } }

// Wrap a string in an scl_value
#define SCLSTRING( s )  &(scl_string){ (uint32_t)(strlen(s)+1), SCL_VALUE_EXTERNAL, { s } }


#define scl_string_init( str )      scl_value_init( str )
#define scl_string_finalize( str )  scl_value_finalize( str )

void scl_strset( scl_string * str, const char * cstr );
const char * scl_strget( scl_string * str );

scl_string * scl_strcat( scl_string * str1, scl_string * str2 );
scl_string * scl_strcpy( scl_string * dst, scl_string * src );

// Case-sensitive comparison
int scl_strcmp( scl_string * str1, scl_string * str2 );
int scl_strncmp( scl_string * str1, scl_string * str2, size_t count );

// Case-insensitive comparison
int scl_stricmp( scl_string * str1, scl_string * str2 );
int scl_strnicmp( scl_string * str1, scl_string * str2, size_t count );

// Like the C function, returns the length of the string in bytes (not including the null terminator)
size_t scl_strlen( scl_string * str );

// Returns the length of the string in unicode glyphs
size_t scl_strlength( scl_string * str );

// Returns the nth unicode glyph in the string
int scl_strglyph( scl_string * str, size_t index );

// Searching for substrings
const char * scl_strstr( scl_string * str1, scl_string * str2 );
const char * scl_strrstr( scl_string * str1, scl_string * str2 );

// Extracting numbers
double scl_strtod( scl_string * str );
long scl_strtol( scl_string * str, int base );
unsigned long scl_strtoul( scl_string * str, int base );

// Splicing and dividing strings
void scl_string_insert( scl_string * str1, scl_string * str2, size_t index );
void scl_string_remove( scl_string * str1, size_t index, size_t count );
void scl_string_substring( scl_string * str1, scl_string * str2, size_t index, size_t count );

// Path manipulation
void scl_path_extension( scl_string * ext, scl_string * path );
void scl_path_append_extension( scl_string * path, scl_string * ext );
void scl_path_remove_extension( scl_string * path );

void scl_path_last_component( scl_string * path_component, scl_string * path );
void scl_path_append_component( scl_string * path, scl_string * path_component );
void scl_path_remove_last_component( scl_string * path );


#endif // _SCL_STRING_H_

