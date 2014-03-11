//
//  scl_buffer.h
//  scl
//
//  Created by Derek Nylen on 11-12-07.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#ifndef _SCL_BUFFER_H_
#define _SCL_BUFFER_H_

#include "scl_block.h"


typedef struct
{
    scl_block block;
    size_t cursor;

} scl_buffer;


void scl_buffer_init( scl_buffer * buffer, size_t reserve_size );
void scl_buffer_finalize( scl_buffer * buffer );

int scl_buffer_seek( scl_buffer * buffer, long offset, int origin );
long scl_buffer_tell( scl_buffer * buffer );

size_t scl_buffer_size( scl_buffer * buffer );
void scl_buffer_resize( scl_buffer * buffer, size_t size );

int scl_buffer_read( void * data, size_t size, size_t count, scl_buffer * buffer );
int scl_buffer_write( const void * data, size_t size, size_t count, scl_buffer * buffer );

int scl_buffer_align( scl_buffer * buffer, size_t alignment );

char * scl_buffer_gets( char * str, int num, scl_buffer * buffer );
int scl_buffer_puts( const char * str, scl_buffer * buffer );
int scl_buffer_printf( scl_buffer * buffer, const char * fmt, ... );


#endif
