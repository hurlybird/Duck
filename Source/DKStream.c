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
static size_t IntegerSize( const char * format, size_t len )
{
    if( len <= 2 )
        return sizeof(int);

    int m1 = format[len - 2];
    int m2 = format[len - 3];

    // l
    if( m1 == 'l' )
    {
        if( m2 == 'l' )
            return sizeof(long long);
        
        return sizeof(long);
    }
    
    if( m1 == 'h' )
    {
        if( m2 == 'h' )
            return sizeof(char);
        
        return sizeof(short);
    }
    
    if( m1 == 'j' )
        return sizeof(intmax_t);
    
    if( m1 == 'z' )
        return sizeof(size_t);
    
    if( m1 == 't' )
        return sizeof(ptrdiff_t);
    
    return sizeof(int);
}

static size_t FloatSize( const char * format, size_t len )
{
    int modifier = format[len - 2];
    return (modifier == 'L') ? sizeof(long double) : sizeof(double);
}

static bool IsUnformattedFloat( const char * format, size_t len )
{
    int formatter = format[len - 1];
    int modifier = format[len - 2];

    return (formatter == 'f') && ((len == 2) || ((len == 3) && (modifier == 'l')));
}

static size_t TrimZeroes( char * num, size_t len )
{
    char * dp = strchr( num, '.' );
    
    if( dp )
    {
        size_t stop = (dp - num) + 1;
        
        for( size_t i = len - 1; (i > stop) && (num[i] == '0'); --i )
        {
            num[i] = '\0';
            len--;
        }
    }
    
    return len;
}

static void CopyFormat( char * dst, const char * src, size_t len, size_t max_len )
{
    DKAssert( len < max_len );
    strncpy( dst, src, len );
    dst[len] = '\0';
}

static void WriteCounter( const char * format, size_t len, size_t count, void * counter )
{
    int modifer = format[len - 2];

    if( len == 2 )
    {
        // %n
        *((int *)counter) = (int)count;
    }
    
    else if( len == 3 )
    {
        // %ln
        if( modifer == 'l' )
            *((long *)counter) = (long)count;
        
        // %hn
        else if( modifer == 'h' )
            *((short *)counter) = (short)count;
        
        // %jn
        else if( modifer == 'j' )
            *((intmax_t *)counter) = (intmax_t)count;
        
        // %zn
        else if( modifer == 'z' )
            *((size_t *)counter) = count;
        
        // %tn
        else if( modifer == 't' )
            *((ptrdiff_t *)counter) = (ptrdiff_t)count;
    }
    
    if( len == 4 )
    {
        // %lln
        if( modifer == 'l' )
            *((long long *)counter) = (long long)count;
        
        // %hhn
        else if( modifer == 'h' )
            *((char *)counter) = (char)count;
    }
}

DKIndex DKVSPrintf( DKStreamRef _self, const char * format, va_list arg_ptr )
{
    if( !_self )
        return 0;
    
    DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
    
    size_t write_count = 0;
    
    const char * seq_start = format;
    size_t seq_count = 0;

    const char * cursor = format;
    DKChar32 ch;
    size_t n;

    DKObjectRef object;
    DKStringRef desc;
    const char * cstr;
    size_t cstr_len;

    size_t num_size;

    char tmp_format[8];
    char tmp[120];
    size_t tmp_len;
    
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
        %a	scientific notation, hexadecimal exponent notation
        %A	scientific notation, hexadecimal exponent notation
        %f	floating point
        %F  floating point
        %g	use %e or %f, whichever is shorter
        %G	use %E or %f, whichever is shorter
        %o	octal
        %s	a string of characters
        %u	unsigned integer
        %x	unsigned hexadecimal, with lowercase letters
        %X	unsigned hexadecimal, with uppercase letters
        %p	a pointer
        %n	a pointer to a counter for the number of characters written so far
        %%	a '%' sign
        */
        
        // Find the format token
        size_t tok = strcspn( cursor + 1, "@%csdioxXufFeEaAgGnp" ) + 1;
        
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
            
        // %s - C string
        case 's':
            cstr = va_arg( arg_ptr, const char * );
            write_count += stream->write( _self, cstr, 1, strlen( cstr ) );
            break;
        
        // %@ - Object
        case '@':
            object = va_arg( arg_ptr, DKObjectRef );
            desc = DKGetDescription( object );
            cstr = DKStringGetCStringPtr( desc );
            cstr_len = DKStringGetByteLength( desc );
            write_count += stream->write( _self, cstr, 1, cstr_len );
            break;
            
        // Character
        case 'c':
            if( DKPutc( va_arg( arg_ptr, int ), _self ) == ch )
                write_count++;
            break;
        
        // Pointer
        case 'p':
            CopyFormat( tmp_format, cursor, tok + 1, sizeof(tmp_format) );
            write_count += sprintf( tmp, tmp_format, va_arg( arg_ptr, void * ) );
            break;
        
        // Integer
        case 'd':
        case 'i':
        case 'u':
        case 'o':
        case 'x':
        case 'X':
            CopyFormat( tmp_format, cursor, tok + 1, sizeof(tmp_format) );
            num_size = IntegerSize( cursor, tok + 1 );
            switch( num_size )
            {
            case sizeof(int8_t):
            case sizeof(int16_t):
                // char and short types are promoted to int
                tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, int ) );
                break;

            case sizeof(int32_t):
                tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, int32_t ) );
                break;
                
            case sizeof(int64_t):
                tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, int64_t ) );
                break;
                
            default:
                DKAssert( 0 );
                tmp_len = 0;
                break;
            };
            
            if( tmp_len > 0 )
                stream->write( _self, tmp, 1, tmp_len );

            write_count += tmp_len;
            break;
            
        // Float
        case 'f':
        case 'F':
        case 'g':
        case 'G':
        case 'e':
        case 'E':
            CopyFormat( tmp_format, cursor, tok + 1, sizeof(tmp_format) );
            num_size = FloatSize( cursor, tok + 1 );
            
            if( num_size == sizeof(double) )
            {
                tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, double ) );
            }

            else if( num_size == sizeof(long double) )
            {
                tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, long double ) );
            }
                
            else
            {
                DKAssert( 0 );
                tmp_len = 0;
            }
            
            if( tmp_len > 0 )
            {
                #if DK_PRETTY_PRINT_FLOATS
                if( IsUnformattedFloat( tmp_format, tok + 1 ) )
                    tmp_len = TrimZeroes( tmp, tmp_len );
                #endif
                
                stream->write( _self, tmp, 1, tmp_len );
            }

            write_count += tmp_len;
            break;
        
        // %n
        case 'n':
            CopyFormat( tmp_format, cursor, tok + 1, sizeof(tmp_format) );
            WriteCounter( tmp_format, tok + 1, write_count, va_arg( arg_ptr, void * ) );
            break;
        
        
        
        // All numeric types
        default:
            DKAssert( 0 );
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


///
//  DKGetc()
//
int DKGetc( DKStreamRef _self )
{
    char ch;
    
    if( DKRead( _self, &ch, 1, 1 ) == 1 )
        return ch;
    
    return EOF;
}


///
//  DKPutc()
//
int DKPutc( int ch, DKStreamRef _self )
{
    char _ch = (char)ch;

    if( DKWrite( _self, &_ch, 1, 1 ) == 1 )
        return ch;
    
    return EOF;
}








