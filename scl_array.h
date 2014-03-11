//
//  scl_array.h
//  scl
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#ifndef _SCL_ARRAY_H_
#define _SCL_ARRAY_H_

#include "scl_block.h"


typedef struct
{
    scl_block block;
    size_t element_size;

} scl_array;

typedef int (*scl_array_traversal)( void * context, void * value, size_t value_size );


void scl_array_init( scl_array * array, size_t element_size, size_t reserve_count );
void scl_array_finalize( scl_array * array );

size_t scl_array_size( scl_array * array );
void scl_array_resize( scl_array * array, size_t count );

void scl_array_add( scl_array * array, const void * buf, size_t count );
void scl_array_insert( scl_array * array, const void * buf, size_t index, size_t count );

void * scl_array_first( scl_array * array );
void * scl_array_last( scl_array * array );
void * scl_array_index( scl_array * array, size_t index );
void * scl_array_range( scl_array * array, size_t index, size_t count );

void scl_array_remove_first( scl_array * array );
void scl_array_remove_last( scl_array * array );
void scl_array_remove_index( scl_array * array, size_t index );
void scl_array_remove_range( scl_array * array, size_t index, size_t count );
void scl_array_remove_all( scl_array * array );

int scl_array_foreach( scl_array * array, scl_array_traversal callback, void * context );


#endif
