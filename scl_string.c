//
//  scl_string.c
//  scl
//
//  Created by Derek Nylen on 2014-02-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "scl_string.h"


///
//  scl_strset()
//
void scl_strset( scl_string * str, const char * cstr )
{
    size_t size = strlen( cstr ) + 1;
    scl_value_set( str, cstr, size );
}


///
//  scl_strget()
//
const char * scl_strget( scl_string * str )
{
    if( scl_value_size( str ) > 0 )
        return scl_value_data( str );
    
    return "";
}


///
//  scl_strcat()
//
scl_string * scl_strcat( scl_string * str1, scl_string * str2 )
{
    return str1;
}


///
//  scl_strcpy()
//
scl_string * scl_strcpy( scl_string * dst, scl_string * src )
{
    scl_value_copy( dst, src );
    return dst;
}


///
//  scl_strcmp()
//
int scl_strcmp( scl_string * str1, scl_string * str2 )
{
    const char * s1 = scl_strget( str1 );
    const char * s2 = scl_strget( str2 );

    return strcmp( s1, s2 );
}


///
//  scl_strncmp()
//
int scl_strncmp( scl_string * str1, scl_string * str2, size_t count )
{
    const char * s1 = scl_strget( str1 );
    const char * s2 = scl_strget( str2 );

    return strncmp( s1, s2, count );
}


///
//  scl_strcmp()
//
int scl_stricmp( scl_string * str1, scl_string * str2 )
{
    const char * s1 = scl_strget( str1 );
    const char * s2 = scl_strget( str2 );

    return strcasecmp( s1, s2 );
}


///
//  scl_strnicmp()
//
int scl_strnicmp( scl_string * str1, scl_string * str2, size_t count )
{
    const char * s1 = scl_strget( str1 );
    const char * s2 = scl_strget( str2 );

    return strncasecmp( s1, s2, count );
}


///
//  scl_strlen()
//
size_t scl_strlen( scl_string * str )
{
    const char * s = scl_strget( str );
    return strlen( s );
}


///
//  scl_strlength()
//
size_t scl_strlength( scl_string * str )
{
    const char * s = scl_strget( str );
    return strlen( s );
}


///
//  scl_strglyph()
//
int scl_strglyph( scl_string * str, size_t index )
{
    const char * s = scl_strget( str );
    
    if( index < strlen( s ) )
        return s[index];
    
    return 0;
}


///
//  scl_strstr()
//
const char * scl_strstr( scl_string * str1, scl_string * str2 )
{
    const char * s1 = scl_strget( str1 );
    const char * s2 = scl_strget( str2 );

    return strstr( s1, s2 );
}

const char * scl_strrstr( scl_string * str1, scl_string * str2 )
{
    return NULL;
}


///
//  scl_strtod()
//
double scl_strtod( scl_string * str )
{
    const char * s = scl_strget( str );
    char * scratch;
    return strtod( s, &scratch );
}


///
//  scl_strtol()
//
long scl_strtol( scl_string * str, int base )
{
    const char * s = scl_strget( str );
    char * scratch;
    return strtol( s, &scratch, base );
}


///
//  scl_strtoul()
//
unsigned long scl_strtoul( scl_string * str, int base )
{
    const char * s = scl_strget( str );
    char * scratch;
    return strtoul( s, &scratch, base );
}


/*
void scl_string_insert( scl_string * str1, scl_string * str2, size_t index );
void scl_string_remove( scl_string * str1, size_t index, size_t count );
void scl_string_substring( scl_string * str1, scl_string * str2, size_t index, size_t count );

void scl_path_extension( scl_string * ext, scl_string * path );
void scl_path_append_extension( scl_string * path, scl_string * ext );
void scl_path_remove_extension( scl_string * path );

void scl_path_last_component( scl_string * path_component, scl_string * path );
void scl_path_append_component( scl_string * path, scl_string * path_component );
void scl_path_remove_last_component( scl_string * path );
*/


