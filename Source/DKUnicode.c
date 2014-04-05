//
//  DKUnicode.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-31.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKUnicode.h"
#include "icu/unicode/utf8.h"


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
//  dk_ustrstr()
//
const char * dk_ustrstr( const char * str1, const char * str2 )
{
    if( (*str1 == '\0') || (*str2 == '\0') )
        return NULL;

    for( int32_t next_i = 0; str1[next_i] != '\0'; )
    {
        int32_t curr_i = next_i;
        int32_t i = next_i;
        int32_t j = 0;
        char32_t ch1, ch2;
    
        U8_NEXT( str1, i, -1, ch1 );
        U8_NEXT( str2, j, -1, ch2 );

        next_i = i;

        while( ch1 == ch2 )
        {
            if( str2[j] == '\0' )
                return &str1[curr_i];
            
            U8_NEXT( str1, i, -1, ch1 );
            U8_NEXT( str2, j, -1, ch2 );
        }
    }
    
    return NULL;
}


///
//  dk_ustrstr_range()
//
DKRange dk_ustrstr_range( const char * str1, const char * str2 )
{
    if( (*str1 == '\0') || (*str2 == '\0') )
        return DKRangeMake( DKNotFound, 0 );

    DKRange range;
    range.location = 0;

    for( int32_t next_i = 0; str1[next_i] != '\0'; )
    {
        range.length = 1;
    
        int32_t i = next_i;
        int32_t j = 0;
        char32_t ch1, ch2;
    
        U8_NEXT( str1, i, -1, ch1 );
        U8_NEXT( str2, j, -1, ch2 );

        next_i = i;

        while( ch1 == ch2 )
        {
            if( str2[j] == '\0' )
                return range;
            
            U8_NEXT( str1, i, -1, ch1 );
            U8_NEXT( str2, j, -1, ch2 );

            range.length++;
        }
        
        range.location++;
    }
    
    return DKRangeMake( DKNotFound, 0 );
}


///
//  dk_ustrcmp()
//
int dk_ustrcmp( const char * str1, const char * str2 )
{
    int32_t i = 0;
    int32_t j = 0;
    char32_t ch1, ch2;
    
    do
    {
        U8_NEXT( str1, i, -1, ch1 );
        U8_NEXT( str2, j, -1, ch2 );
    }
    while( (ch1 == ch2) && (ch1 != '\0') );

    return ch2 - ch1;
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




