//
//  DKStream.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKStream.h"
#include "DKString.h"
#include "DKUnicode.h"


DKThreadSafeSelectorInit( Stream );


///
//  DKSeek()
//
int DKSeek( DKTypeRef ref, DKIndex offset, int origin )
{
    if( ref )
    {
        DKStream * stream = DKGetInterface( ref, DKSelector(Stream) );
        return stream->seek( ref, offset, origin );
    }
    
    return -1;
}


///
//  DKTell()
//
DKIndex DKTell( DKTypeRef ref )
{
    if( ref )
    {
        DKStream * stream = DKGetInterface( ref, DKSelector(Stream) );
        return stream->tell( ref );
    }
    
    return -1;
}


///
//  DKRead()
//
DKIndex DKRead( DKTypeRef ref, void * data, DKIndex size, DKIndex count )
{
    if( ref )
    {
        DKStream * stream = DKGetInterface( ref, DKSelector(Stream) );
        return stream->read( ref, data, size, count );
    }
    
    return 0;
}


///
//  DKWrite()
//
DKIndex DKWrite( DKTypeRef ref, const void * data, DKIndex size, DKIndex count )
{
    if( ref )
    {
        DKStream * stream = DKGetInterface( ref, DKSelector(Stream) );
        return stream->write( ref, data, size, count );
    }
    
    return 0;
}


///
//  DKSPrintf()
//
DKIndex DKSPrintf( DKTypeRef ref, const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKIndex result = DKVSPrintf( ref, format, arg_ptr );
    
    va_end( arg_ptr );
    
    return result;
}


///
//  DKVSPrintf()
//
static size_t WriteNumber( DKTypeRef ref, DKStream * stream, const char * format, size_t formatLength, va_list arg_ptr )
{
    char fmt[32];
    char num[32];

    DKAssert( formatLength < (sizeof(fmt) -1) );
    strncpy( fmt, format, formatLength );
    fmt[formatLength] = '\0';
    
    size_t n = vsprintf( num, fmt, arg_ptr );
    
    if( n > 0 )
        stream->write( ref, num, 1, n );
    
    return n;
}

DKIndex DKVSPrintf( DKTypeRef ref, const char * format, va_list arg_ptr )
{
    if( !ref )
        return 0;
    
    DKStream * stream = DKGetInterface( ref, DKSelector(Stream) );
    
    size_t write_count = 0;
    
    const char * seq_start = format;
    size_t seq_count = 0;

    DKTypeRef object;
    DKTypeRef desc;
    const char * string;
    int * counter;

    const char * cursor = format;
    char32_t ch;
    size_t n;
    
    while( (n = dk_ustrscan( cursor, &ch )) != 0 )
    {
        // Non-format character, add it to the current sequence
        if( (n > 1) || (ch != '%') )
        {
            seq_count += n;
            cursor += n;
            
            continue;
        }
    
        // Flush the current sequence
        if( seq_count > 0 )
        {
            write_count += stream->write( ref, seq_start, 1, seq_count );
        }

        /*
        %c	character
        %d	signed integers
        %i	signed integers
        %e	scientific notation, with a lowercase "e"
        %E	scientific notation, with a uppercase "E"
        %f	floating point
        %g	use %e or %f, whichever is shorter
        %G	use %E or %f, whichever is shorter
        %o	octal
        %s	a string of characters
        %u	unsigned integer
        %x	unsigned hexadecimal, with lowercase letters
        %X	unsigned hexadecimal, with uppercase letters
        %p	a pointer
        %n	the argument shall be a pointer to an integer
            into which is placed the number of characters
            written so far
        %%	a '%' sign
        */
        
        // Find the format token
        size_t tok = strcspn( cursor + 1, "cdieEfgGosuxXpn@%" ) + 1;
        
        seq_start = cursor + tok + 1;
        seq_count = 0;
        
        ch = *(cursor + tok);
        
        switch( ch )
        {
        // %% - Skip over the first % and include the second in the next sequence
        case '%':
            seq_start = cursor + tok;
            seq_count = 1;
            break;
            
        // %s
        case 's':
            string = va_arg( arg_ptr, const char * );
            write_count += stream->write( ref, string, 1, strlen( string ) );
            break;
        
        // %@
        case '@':
            object = va_arg( arg_ptr, DKTypeRef );
            desc = DKCopyDescription( object );
            string = DKStringGetCStringPtr( desc );
            write_count += stream->write( ref, string, 1, strlen( string ) );
            DKRelease( desc );
            break;
        
        // %n
        case 'n':
            counter = va_arg( arg_ptr, int * );
            *counter = (int)write_count;
            break;
        
        // All numeric types
        default:
            write_count += WriteNumber( ref, stream, cursor, tok + 1, arg_ptr );
            break;
        }
        
        // Update the cursor
        cursor = cursor + tok + 1;
    }

    // Write anything left in the current sequence
    if( seq_count > 0 )
    {
        write_count += stream->write( ref, seq_start, 1, seq_count );
    }
    
    return write_count;
}










