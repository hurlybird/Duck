/*****************************************************************************************

  DKStream.c

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

#include "DKStream.h"
#include "DKString.h"
#include "DKUnicode.h"
#include "DKDescription.h"


// The stream selector is initialized by DKRuntimeInit() so that constant strings can be
// used during initialization.
//DKThreadSafeFastSelectorInit( Stream );


///
//  DKSeek()
//
int DKSeek( DKStreamRef _self, DKIndex offset, int origin )
{
    if( _self )
    {
        DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
        return stream->seek( _self, offset, origin );
    }
    
    return -1;
}


///
//  DKTell()
//
DKIndex DKTell( DKStreamRef _self )
{
    if( _self )
    {
        DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
        return stream->tell( _self );
    }
    
    return -1;
}


///
//  DKRead()
//
DKIndex DKRead( DKStreamRef _self, void * data, DKIndex size, DKIndex count )
{
    if( _self )
    {
        DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
        return stream->read( _self, data, size, count );
    }
    
    return 0;
}


///
//  DKWrite()
//
DKIndex DKWrite( DKStreamRef _self, const void * data, DKIndex size, DKIndex count )
{
    if( _self )
    {
        DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
        return stream->write( _self, data, size, count );
    }
    
    return 0;
}


///
//  DKSPrintf()
//
DKIndex DKSPrintf( DKStreamRef _self, const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    DKIndex result = DKVSPrintf( _self, format, arg_ptr );
    
    va_end( arg_ptr );
    
    return result;
}


///
//  DKVSPrintf()
//
static size_t WriteNumber( DKStreamRef _self, DKStreamInterfaceRef stream, const char * format, size_t formatLength, va_list arg_ptr )
{
    char fmt[32];
    char num[32];

    DKAssert( formatLength < (sizeof(fmt) -1) );
    strncpy( fmt, format, formatLength );
    fmt[formatLength] = '\0';
    
    size_t n = vsprintf( num, fmt, arg_ptr );
    
    if( n > 0 )
        stream->write( _self, num, 1, n );
    
    return n;
}

DKIndex DKVSPrintf( DKStreamRef _self, const char * format, va_list arg_ptr )
{
    if( !_self )
        return 0;
    
    DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
    
    size_t write_count = 0;
    
    const char * seq_start = format;
    size_t seq_count = 0;

    DKObjectRef object;
    DKStringRef desc;
    const char * cstr;
    int * counter;

    const char * cursor = format;
    DKChar32 ch;
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
            write_count += stream->write( _self, seq_start, 1, seq_count );
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
            cstr = va_arg( arg_ptr, const char * );
            write_count += stream->write( _self, cstr, 1, strlen( cstr ) );
            break;
        
        // %@
        case '@':
            object = va_arg( arg_ptr, DKObjectRef );
            desc = DKGetDescription( object );
            cstr = DKStringGetCStringPtr( desc );
            write_count += stream->write( _self, cstr, 1, strlen( cstr ) );
            break;
        
        // %n
        case 'n':
            counter = va_arg( arg_ptr, int * );
            *counter = (int)write_count;
            break;
        
        // All numeric types
        default:
            write_count += WriteNumber( _self, stream, cursor, tok + 1, arg_ptr );
            break;
        }
        
        // Update the cursor
        cursor = cursor + tok + 1;
    }

    // Write anything left in the current sequence
    if( seq_count > 0 )
    {
        write_count += stream->write( _self, seq_start, 1, seq_count );
    }
    
    return write_count;
}










