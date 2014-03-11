//
//  scl_buffer.c
//  scl
//
//  Created by Derek Nylen on 11-12-07.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "scl_buffer.h"


///
//  scl_buffer_init()
//
void scl_buffer_init( scl_buffer * buffer, size_t reserve_size )
{
    scl_block_init( &buffer->block, reserve_size );
    buffer->cursor = 0;
}


///
//  scl_buffer_finalize()
//
void scl_buffer_finalize( scl_buffer * buffer )
{
    scl_block_finalize( &buffer->block );
    buffer->cursor = 0;
}


///
//  scl_buffer_seek()
//
int scl_buffer_seek( scl_buffer * buffer, long offset, int origin )
{
    if( origin == SEEK_CUR )
    {
        offset = (long)(buffer->cursor + offset);
        origin = SEEK_SET;
    }

    else if( origin == SEEK_END )
    {
        offset = (long)scl_block_size( &buffer->block ) + offset;
        origin = SEEK_SET;
    }

    if( origin == SEEK_SET )
    {
        buffer->cursor = (offset > 0) ? offset : 0;
        return 0;
    }
    
    return -1;
}


///
//  scl_buffer_tell()
//
long scl_buffer_tell( scl_buffer * buffer )
{
    return (long)buffer->cursor;
}


///
//  scl_buffer_resize()
//
size_t scl_buffer_size( scl_buffer * buffer )
{
    return scl_block_size( &buffer->block );
}


///
//  scl_buffer_resize()
//
void scl_buffer_resize( scl_buffer * buffer, size_t size )
{
    scl_block_resize( &buffer->block, size );
}


///
//  scl_buffer_read()
//
int scl_buffer_read( void * data, size_t size, size_t count, scl_buffer * buffer )
{
    size_t location = buffer->cursor;
    size_t buffer_size = scl_block_size( &buffer->block );

    if( location < buffer_size )
    {
        size_t length = size * count;
        size_t bytes_left = buffer_size - location;
    
        if( length > bytes_left )
        {
            count = bytes_left / size;
            length = size * count;
        }

        if( length )
        {
            void * src = scl_block_range( &buffer->block, location, length, 0 );
            assert( src );
            
            memcpy( data, src, length );
            buffer->cursor += length;
        }
        
        return (int)count;
    }
    
    return 0;
}


///
//  scl_buffer_write()
//
int scl_buffer_write( const void * data, size_t size, size_t count, scl_buffer * buffer )
{
    size_t location = buffer->cursor;
    size_t length = size * count;
    
    void * dst = scl_block_range( &buffer->block, location, length, 1 );
    assert( dst );
    
    memcpy( dst, data, length );
    buffer->cursor += length;
    
    return (int)count;
}


///
//  scl_buffer_align()
//
int scl_buffer_align( scl_buffer * buffer, size_t alignment )
{
    size_t size = scl_block_size( &buffer->block );
    size_t pad = ((size + (alignment - 1)) & ~(alignment - 1)) - size;
    
    void * dst = scl_block_range( &buffer->block, buffer->cursor, pad, 1 );
    assert( dst );
    
    memset( dst, 0, pad );
    buffer->cursor += pad;
    
    return 1;
}


///
//  scl_buffer_gets()
//
char * scl_buffer_gets( char * str, int num, scl_buffer * buffer )
{
    size_t size = scl_block_size( &buffer->block );
    const char * data = scl_block_range( &buffer->block, 0, size, 0 );

    int i, c;
    
    for( i = 0; ; i++ )
    {
        if( buffer->cursor >= size )
            break;
            
        if( i == (num - 1) )
            break;

        c = *(data + buffer->cursor);
        buffer->cursor++;

        str[i] = c;

        if( c == '\n' )
        {
            i++;
            break;
        }
    }
    
    str[i] = '\0';
    
    return str;
}


///
//  scl_buffer_puts()
//
int scl_buffer_puts( const char * str, scl_buffer * buffer )
{
    return scl_buffer_write( str, 1, strlen( str ), buffer );
}


///
//  scl_buffer_printf()
//
int scl_buffer_printf( scl_buffer * buffer, const char * fmt, ... )
{
    va_list arg_ptr;
    int length;
    char scratch;
    
    va_start( arg_ptr, fmt );
    
    length = vsnprintf( &scratch, 1, fmt, arg_ptr );
    
    if( length > 0 )
    {
        void * dst = scl_block_range( &buffer->block, buffer->cursor, length + 1, 1 );
        assert( dst );
        
        vsprintf( (char *)dst, fmt, arg_ptr );
        scl_block_resize( &buffer->block, scl_block_size( &buffer->block ) - 1 );
        buffer->cursor += length;
    }
    
    va_end( arg_ptr );
    
    return length;
}








