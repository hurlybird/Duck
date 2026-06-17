// =======================================================================================
//
// DKUnicode.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKUnicode.h"
#include "icu/unicode/utf8.h"


///
//  DKChar8FromCString()
//
inline DKChar8 DKChar8FromCString( const char * s )
{
    DKChar8 ch;
    *((uint64_t *)&ch) = 0;
    
    dk_ustrscan8( s, &ch );
    
    return ch;
}


///
//  DKChar8FromChar32()
//
DKChar8 DKChar8FromChar32( DKChar32 ch )
{
    DKChar8 ch8;
    *((uint64_t *)&ch8) = 0;
    
    dk_ustrwrite( ch, ch8.s, sizeof(DKChar8) );
    
    return ch8;
}


///
//  DKChar32FromCString()
//
DKChar32 DKChar32FromCString( const char * s )
{
    DKChar32 ch = 0;
    
    dk_ustrscan( s, &ch );
    
    return ch;
}


///
//  DKChar32FromChar8()
//
DKChar32 DKChar32FromChar8( DKChar8 ch )
{
    DKChar32 ch32 = 0;
    
    dk_ustrscan( ch.s, &ch32 );
    
    return ch32;
}



///
//  dk_ustrchr()
//
const char * dk_ustrchr( const char * str, int ch )
{
    int32_t i = 0;
    DKChar32 c;
    
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
    DKChar32 c;
    
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
        DKChar32 ch1, ch2;
    
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
        DKChar32 ch1, ch2;
    
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
    DKChar32 ch1, ch2;
    
    do
    {
        U8_NEXT( str1, i, -1, ch1 );
        U8_NEXT( str2, j, -1, ch2 );
    }
    while( (ch1 == ch2) && (ch1 != '\0') );

    return ch1 - ch2;
}


///
//  dk_ustrcasecmp()
//
int dk_ustrcasecmp( const char * str1, const char * str2 )
{
    int32_t i = 0;
    int32_t j = 0;
    DKChar32 ch1, ch2;
    
    do
    {
        if( U8_IS_SINGLE( str1[i] ) )
            ch1 = toupper( str1[i++] );
        
        else
            U8_NEXT( str1, i, -1, ch1 );
        
        if( U8_IS_SINGLE( str2[j] ) )
            ch2 = toupper( str2[j++] );
        
        else
            U8_NEXT( str2, j, -1, ch2 );
        
    }
    while( (ch1 == ch2) && (ch1 != '\0') );

    return ch1 - ch2;
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
        DKChar32 ch;
        cur += dk_ustrscan( cur, &ch );

        length++;
    }

    return length;
}


///
//  dk_ustrnlen()
//
size_t dk_ustrnlen( const char * str, size_t n )
{
    size_t length = 0;
    const char * cur = str;

    while( (*cur != '\0') && ((size_t)(cur - str) < n) )
    {
        DKChar32 ch;
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
        DKChar32 ch;
        cur += dk_ustrscan( cur, &ch );
        
        if( ch == '\0' )
            return NULL;
    }
    
    return cur;
}


///
//  dk_ustrridx()
//
const char * dk_ustrridx( const char * str, size_t idx )
{
    int32_t i = (int32_t)strlen( str );
    DKChar32 c;
    
    for( size_t j = 0; i > 0; j++ )
    {
        U8_PREV( str, 0, i, c );

        if( j >= idx )
            return &str[i];
    }
    
    return NULL;
}


///
//  dk_ustrscan8()
//
size_t dk_ustrscan8( const char * str, DKChar8 * ch )
{
    DKChar32 c;
    int32_t i = 0;

    U8_GET_OR_FFFD( (uint8_t *)str, 0, 0, -1, c );
    U8_APPEND_UNSAFE( (uint8_t *)ch->s, i, c );
        
    return i;
}


///
//  dk_ustrscan()
//
size_t dk_ustrscan( const char * str, DKChar32 * ch )
{
    if( *str == '\0' )
    {
        *ch = '\0';
        return 0;
    }

    int32_t i = 0;
    DKChar32 c;

    U8_NEXT( str, i, -1, c );
    
    *ch = c;
    return i;
}


///
//  dk_ustrwrite()
//
size_t dk_ustrwrite( DKChar32 ch, char * str, size_t str_size )
{
    uint32_t capacity = (uint32_t)str_size - 1;
    uint32_t i = 0;
    UBool error = 0;
    
    U8_APPEND( (uint8_t *)str, i, capacity, ch, error );

    if( !error )
    {
        if( i < str_size )
            str[i] = '\0';

        return i;
    }
     
    return 0;
}


///
//  dk_ustrlwr()
//
char * dk_ustrlwr( char * dst, size_t dst_size, const char * src )
{
    uint32_t capacity = (uint32_t)dst_size - 1;
    char ch;

    for( uint32_t i = 0; i < capacity; ++i )
    {
        if( (ch = *src++) == '\0' )
            break;
            
        if( U8_IS_SINGLE( ch ) )
            *dst++ = (char)tolower( ch );

        else
            *dst++ = ch;
    }

    *dst = '\0';
    return dst;
}


///
//  dk_ustrupr()
//
char * dk_ustrupr( char * dst, size_t dst_size, const char * src )
{
    uint32_t capacity = (uint32_t)dst_size - 1;
    char ch;

    for( uint32_t i = 0; i < capacity; ++i )
    {
        if( (ch = *src++) == '\0' )
            break;
            
        if( U8_IS_SINGLE( ch ) )
            *dst++ = (char)toupper( ch );

        else
            *dst++ = ch;
    }

    *dst = '\0';
    return dst;
}

