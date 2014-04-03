//
//  DKUnicode.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-31.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKUnicode.h"
#include "Unicode/utf8.h"


///
//  dk_ustrchr()
//
const char * dk_ustrchr( const char * str, int ch )
{
    int32_t i = 0;
    char32_t c;
    
    while( str[i] != '\0' )
    {
        U8_NEXT( str, i, -1, c );
        
        if( c == ch )
            return &str[i];
    }
    
    return NULL;
}


///
//  dk_ustrrchr()
//
const char * dk_ustrrchr( const char * str, int ch )
{
    int32_t i = (int32_t)strlen( str );
    char32_t c;
    
    while( i > 0 )
    {
        U8_PREV( str, 0, i, c );
        
        if( c == ch )
            return &str[i];
    }
    
    return NULL;
}


///
//  dk_ustrcmp()
//
int dk_ustrcmp( const char * str1, const char * str2, int options )
{
    return strcmp( str1, str2 );
}


///
//  dk_ustrlen()
//
size_t dk_ustrlen( const char * str )
{
    size_t length = 0;
    const char * cur = str;

    while( *cur != '\0' )
    {
        char32_t ch;
        cur += dk_ustrscan( cur, &ch );

        length++;
    }

    return length;
}


///
//  dk_ustridx()
//
const char * dk_ustridx( const char * str, size_t idx )
{
    const char * cur = str;
    
    for( size_t i = 0; i < idx; ++i )
    {
        char32_t ch;
        cur += dk_ustrscan( cur, &ch );
        
        if( ch == '\0' )
            return NULL;
    }
    
    return cur;
}


///
//  dk_ustrscan()
//
size_t dk_ustrscan( const char * str, char32_t * ch )
{
    if( *str == '\0' )
    {
        *ch = '\0';
        return 0;
    }

    int32_t i = 0;
    char32_t c;

    U8_NEXT( str, i, -1, c )
    
    *ch = c;
    return i;
}




