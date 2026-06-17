// =======================================================================================
//
// DKUnicode.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_UNICODE_H_
#define _DK_UNICODE_H_

#ifdef __cplusplus
extern "C"
{
#endif

// Character conversions
DK_API DKChar8 DKChar8FromCString( const char * s );
DK_API DKChar8 DKChar8FromChar32( DKChar32 ch );

DK_API DKChar32 DKChar32FromCString( const char * s );
DK_API DKChar32 DKChar32FromChar8( DKChar8 ch );


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
DK_API int dk_ustrcasecmp( const char * str1, const char * str2 );

// strlen
DK_API size_t dk_ustrlen( const char * str );
DK_API size_t dk_ustrnlen( const char * str, size_t n );

// Returns a pointer to the start of the unicode character at index 'idx'
DK_API const char * dk_ustridx( const char * str, size_t idx );

// Returns a pointer to the start of the unicode character at reverse index 'idx'
DK_API const char * dk_ustrridx( const char * str, size_t idx );

// Scans one character from 'str' into 'ch' and returns the number of bytes read
DK_API size_t dk_ustrscan8( const char * str, DKChar8 * ch );
DK_API size_t dk_ustrscan( const char * str, DKChar32 * ch );

// Writes the character 'ch' into 'str' and returns the number of bytes written
DK_API size_t dk_ustrwrite( DKChar32 ch, char * str, size_t str_size );

// Versons of strlwr and strupr that leave multibyte code points unmodified
DK_API char * dk_ustrlwr( char * dst, size_t dst_size, const char * src );
DK_API char * dk_ustrupr( char * dst, size_t dst_size, const char * src );


#ifdef __cplusplus
}
#endif

#endif // _DK_UNICODE_H_
