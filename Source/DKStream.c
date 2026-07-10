// =======================================================================================
//
// DKStream.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKStream.h"
#include "DKString.h"
#include "DKUnicode.h"
#include "DKDescription.h"


// The stream selector is initialized by DKRuntimeInit() so that constant strings can be
// used during initialization.
//DKThreadSafeFastSelectorInit( Stream );


// DKNullStream ==========================================================================
struct DKNullStream
{
    DKObject _obj;
};

static int DKNullStreamSeek( DKStreamRef _self, long offset, int origin )
{
    return 0;
}

static long DKNullStreamTell( DKStreamRef _self )
{
    return 0;
}

static int DKNullStreamGetStatus( DKStreamRef _self )
{
    return _self ? DKStreamOK : DKStreamEOF;
}

static DKIndex DKNullStreamGetLength( DKStreamRef _self )
{
    return 0;
}

static size_t DKNullStreamRead( DKStreamRef _self, void * buffer, size_t size, size_t count )
{
    return 0;
}

static size_t DKNullStreamWrite( DKStreamRef _self, const void * buffer, size_t size, size_t count )
{
    return size * count;
}

static int DKNullStreamFlush( DKStreamRef _self )
{
    return 0;
}

DKThreadSafeClassInit( DKNullStreamClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKNullStream" ), DKObjectClass(),
        sizeof(struct DKNullStream), 0, NULL, NULL );
    
    // Comparison
    struct DKStreamInterface * stream = DKNewInterface( DKSelector(Stream) );
    stream->seek = DKNullStreamSeek;
    stream->tell = DKNullStreamTell;
    stream->read = DKNullStreamRead;
    stream->write = DKNullStreamWrite;
    stream->flush = DKNullStreamFlush;
    stream->getStatus = DKNullStreamGetStatus;
    stream->getLength = DKNullStreamGetLength;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
}




// DKStream Functions ====================================================================

///
//  DKSeek()
//
int DKSeek( DKStreamRef _self, long offset, int origin )
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
long DKTell( DKStreamRef _self )
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
size_t DKRead( DKStreamRef _self, void * data, size_t size, size_t count )
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
size_t DKWrite( DKStreamRef _self, const void * data, size_t size, size_t count )
{
    if( _self )
    {
        DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
        return stream->write( _self, data, size, count );
    }
    
    return 0;
}


///
//  DKFlush()
//
int DKFlush( DKStreamRef _self )
{
    if( _self )
    {
        DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
        return stream->flush( _self );
    }
    
    return 0;
}


///
//  DKStreamGetStatus()
//
int DKStreamGetStatus( DKStreamRef _self )
{
    if( _self )
    {
        DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
        return stream->getStatus( _self );
    }
    
    return 0;
}


///
//  DKStreamGetLength()
//
DKIndex DKStreamGetLength( DKStreamRef _self )
{
    if( _self )
    {
        DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
        return stream->getLength( _self );
    }
    
    return DKNotFound;
}


///
//  DKSPrintf()
//
int DKSPrintf( DKStreamRef _self, const char * format, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, format );
    
    int result = DKVSPrintf( _self, format, arg_ptr );
    
    va_end( arg_ptr );
    
    return result;
}


///
//  DKVSPrintf()
//
enum
{
    LeftJustified =     (1 << 0),
    LeadingSign =       (1 << 1),
    LeadingSpace =      (1 << 2),
    AlternateForm =     (1 << 3),
    LeadingZeroes =     (1 << 4),
    WidthLiteral =      (1 << 5),
    WidthArgument =     (1 << 6),
    PrecisionLiteral =  (1 << 7),
    PrecisionArgument = (1 << 8)
};

typedef struct
{
    unsigned int flags;
    int width;
    int precision;

} FormatOptions;

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
    
    if( m1 == 'p' )
        return sizeof(intptr_t);
    
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

static unsigned int ReadFlag( char ch )
{
    switch( ch )
    {
    case '-': return LeftJustified;
    case '+': return LeadingSign;
    case ' ': return LeadingSpace;
    case '#': return AlternateForm;
    case '0': return LeadingZeroes;
    default: return 0;
    }
}

static void CopyFormat( char * dst, FormatOptions * options, const char * src, size_t len, size_t max_len )
{
    dst[0] = '\0';
    
    options->flags = 0;
    options->width = 0;
    options->precision = 0;

    DKRequire( len < max_len );

    char * dst_cursor = dst;
    const char * src_cursor = src;
    
    *dst_cursor++ = *src_cursor++;
    
    while( *src_cursor != '\0' )
    {
        unsigned int flag = ReadFlag( *src_cursor );

        if( flag )
        {
            options->flags |= flag;
            *dst_cursor++ = *src_cursor++;
            continue;
        }

        break;
    }
    
    if( *src_cursor == '*' )
    {
        options->flags |= WidthArgument;
        *dst_cursor++ = *src_cursor++;
    }
    
    else if( isdigit( *src_cursor ) )
    {
        options->flags |= WidthLiteral;

        char * end;
        options->width = (int)strtol( src_cursor, &end, 10 );
        
        while( isdigit( *src_cursor ) )
            *dst_cursor++ = *src_cursor++;
    }

    if( *src_cursor == '.' )
    {
        *dst_cursor++ = *src_cursor++;
        
        if( *src_cursor == '*' )
        {
            options->flags |= PrecisionArgument;
            *dst_cursor++ = *src_cursor++;
        }
        
        else if( isdigit( *src_cursor ) )
        {
            options->flags |= PrecisionLiteral;
            
            char * end;
            options->precision = (int)strtol( src_cursor, &end, 10 );
            
            while( isdigit( *src_cursor ) )
                *dst_cursor++ = *src_cursor++;
        }
    }
    
    while( (size_t)(dst_cursor - dst) < len )
        *dst_cursor++ = *src_cursor++;
    
    *dst_cursor = '\0';
}

static size_t WriteBinary( DKStreamRef _self, DKStreamInterfaceRef stream, const char * format, const FormatOptions * options, int width, uint64_t number )
{
    size_t write_count = 0;
    
    char buffer[64]; // Not null terminated
    int length = 0;

    uint64_t bit = (uint64_t)1 << 63;

    for( ; bit != 0; bit = (bit >> 1) )
    {
        if( number & bit )
            break;
    }

    for( ; bit != 0; bit = (bit >> 1) )
    {
        buffer[length++] = (number & bit) ? '1' : '0';
    }
    
    if( options->flags & AlternateForm )
    {
        write_count += stream->write( _self, "0b", 1, 2 );
    }

    if( options->flags & LeftJustified )
    {
        write_count += stream->write( _self, buffer, 1, length );
        
        if( options->flags & WidthLiteral )
        {
            for( int i = length; i < options->width; i++ )
                write_count += stream->write( _self, " ", 1, 1 );
        }
        
        else if( (options->flags & WidthArgument) && (width > 0) )
        {
            for( int i = length; i < width; i++ )
                write_count += stream->write( _self, " ", 1, 1 );
        }
    }
    
    else
    {
        const char * fill = (options->flags & LeadingZeroes) ? "0" : " ";
        
        if( options->flags & WidthLiteral )
        {
            for( int i = length; i < options->width; i++ )
                write_count += stream->write( _self, fill, 1, 1 );
        }
        
        else if( (options->flags & WidthArgument) && (width > 0) )
        {
            for( int i = length; i < width; i++ )
                write_count += stream->write( _self, fill, 1, 1 );
        }

        write_count += stream->write( _self, buffer, 1, length );
    }
    
    return write_count;
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

int DKVSPrintf( DKStreamRef _self, const char * format, va_list arg_ptr )
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
    const wchar_t * wstr;
    size_t cstr_len;

    size_t num_size;

    char tmp_format[16];
    char tmp[128];
    FormatOptions tmp_options;
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
        size_t tok = strcspn( cursor + 1, "@%csdiboxXufFeEaAgGnp" ) + 1;
        
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
            if( cursor[tok - 1] == 'l' )
            {
                wstr = va_arg( arg_ptr, const wchar_t * );

                if( !wstr )
                    wstr = L"(null)";

                CopyFormat( tmp_format, &tmp_options, cursor, tok + 1, sizeof(tmp_format) );

                int wlen = snprintf( NULL, 0, tmp_format, wstr );

                if( wlen > 0 )
                {
                    char * buf = dk_malloc( wlen + 1 );
                    snprintf( buf, wlen + 1, tmp_format, wstr );
                    write_count += stream->write( _self, buf, 1, strlen( buf ) );
                    dk_free( buf );
                }
            }
            
            else
            {
                cstr = va_arg( arg_ptr, const char * );

                if( !cstr )
                    cstr = "(null)";

                if( tok > 1 )
                {
                    CopyFormat( tmp_format, &tmp_options, cursor, tok + 1, sizeof(tmp_format) );

                    int clen = snprintf( NULL, 0, tmp_format, cstr );

                    if( clen > 0 )
                    {
                        char * buf = dk_malloc( clen + 1 );
                        snprintf( buf, clen + 1, tmp_format, cstr );
                        write_count += stream->write( _self, buf, 1, strlen( buf ) );
                        dk_free( buf );
                    }
                }
                
                else
                {
                    write_count += stream->write( _self, cstr, 1, strlen( cstr ) );
                }
            }
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
            if( DKPutc( _self, va_arg( arg_ptr, int ) ) == ch )
                write_count++;
            break;
        
        // Binary (this can move to using sprintf in C23)
        case 'b':
            CopyFormat( tmp_format, &tmp_options, cursor, tok + 1, sizeof(tmp_format) );
            num_size = IntegerSize( cursor, tok + 1 );
            switch( num_size )
            {
            case sizeof(int8_t):
            case sizeof(int16_t):
            case sizeof(int32_t):
                if( tmp_options.flags & WidthArgument )
                    tmp_len = WriteBinary( _self, stream, tmp_format, &tmp_options, va_arg( arg_ptr, int ), va_arg( arg_ptr, uint32_t ) );
                
                else
                    tmp_len = WriteBinary( _self, stream, tmp_format, &tmp_options, 1, va_arg( arg_ptr, uint32_t ) );
                break;
                
            case sizeof(int64_t):
                if( tmp_options.flags & WidthArgument )
                    tmp_len = WriteBinary( _self, stream, tmp_format, &tmp_options, va_arg( arg_ptr, int ), va_arg( arg_ptr, uint64_t ) );
                
                else
                    tmp_len = WriteBinary( _self, stream, tmp_format, &tmp_options, 1, va_arg( arg_ptr, uint64_t ) );
                break;
                
            default:
                DKAssert( 0 );
                tmp_len = 0;
                break;
            };

            write_count += tmp_len;
            break;
        
        // Integer / Pointer
        case 'd':
        case 'i':
        case 'u':
        case 'o':
        case 'x':
        case 'X':
        case 'p':
            CopyFormat( tmp_format, &tmp_options, cursor, tok + 1, sizeof(tmp_format) );
            num_size = IntegerSize( cursor, tok + 1 );
            switch( num_size )
            {
            case sizeof(int8_t):
            case sizeof(int16_t):
            case sizeof(int32_t):
                if( tmp_options.flags & WidthArgument )
                    tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, int ), va_arg( arg_ptr, int32_t ) );
                
                else
                    tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, int32_t ) );
                break;
                
            case sizeof(int64_t):
                if( tmp_options.flags & WidthArgument )
                    tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, int ), va_arg( arg_ptr, int64_t ) );
                
                else
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
        case 'a':
        case 'A':
            CopyFormat( tmp_format, &tmp_options, cursor, tok + 1, sizeof(tmp_format) );
            num_size = FloatSize( cursor, tok + 1 );
            
            if( num_size == sizeof(double) )
            {
                if( tmp_options.flags & WidthArgument )
                {
                    if( tmp_options.flags & PrecisionArgument )
                        tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, int ), va_arg( arg_ptr, int ), va_arg( arg_ptr, double ) );
                    
                    else
                        tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, int ), va_arg( arg_ptr, double ) );
                }
                
                else
                {
                    if( tmp_options.flags & PrecisionArgument )
                        tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, int ), va_arg( arg_ptr, double ) );
                    
                    else
                        tmp_len = sprintf( tmp, tmp_format, va_arg( arg_ptr, double ) );
                }
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
            CopyFormat( tmp_format, &tmp_options, cursor, tok + 1, sizeof(tmp_format) );
            WriteCounter( tmp_format, tok + 1, write_count, va_arg( arg_ptr, void * ) );
            break;
        
        // Unrecognized
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
    
    return (int)write_count;
}


///
//  DKGets()
//
DKStringRef DKGets( DKStreamRef _self )
{
    DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );
    
    DKMutableStringRef s = DKMutableString();
    
    char ch[2] = { '\0', '\0' };
    
    while( stream->read( _self, ch, 1, 1 ) == 1 )
    {
        if( ch[0] == '\n' )
            break;
        
        DKStringAppendCString( s, ch );
    }
    
    return s;
}


///
//  DKPuts()
//
int DKPuts( DKStreamRef _self, DKStringRef s )
{
    DKStreamInterfaceRef stream = DKGetInterface( _self, DKSelector(Stream) );

    const char * cstr = DKStringGetCStringPtr( s );
    size_t bytes = DKStringGetByteLength( s );
    
    if( stream->write( _self, cstr, 1, bytes ) == bytes )
        return 0;
    
    return EOF;
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
int DKPutc( DKStreamRef _self, int ch )
{
    char _ch = (char)ch;

    if( DKWrite( _self, &_ch, 1, 1 ) == 1 )
        return ch;
    
    return EOF;
}


///
//  DKGetChar8()
//
DKChar32 DKGetChar8( DKStreamRef _self, DKChar8 * ch )
{
    int len = 0;
    
    for( int i = 0; i < 6; i++ )
    {
        if( DKRead( _self, &ch->s[len], 1, 1 ) != 1 )
            break;

        len++;
        ch->s[len] = '\0';
        
        DKChar32 ch32;
        dk_ustrscan( ch->s, &ch32 );
        
        if( ch32 >= 0 )
            return ch32;
    }
    
    return EOF;
}


///
//  DKGetChar32()
//
DKChar32 DKGetChar32( DKStreamRef _self )
{
    DKChar8 ch8;
    return DKGetChar8( _self, &ch8 );
}


///
//  DKPutChar8()
//
DKChar32 DKPutChar8( DKStreamRef _self, DKChar8 ch )
{
    DKChar32 ch32;
    size_t len = dk_ustrscan( ch.s, &ch32 );
    
    if( len > 0 )
    {
        if( DKWrite( _self, ch.s, 1, len ) == len )
            return ch32;
    }
        
    return EOF;
}


///
//  DKPutChar32()
//
DKChar32 DKPutChar32( DKStreamRef _self, DKChar32 ch )
{
    DKChar8 ch8 = DKChar8FromChar32( ch );
    return DKPutChar8( _self, ch8 );
}






