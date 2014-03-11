//
//  scl_value.h
//  scl
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#ifndef _SCL_VALUE_H_
#define _SCL_VALUE_H_

#include "scl_env.h"


#define SCL_VALUE_ALLOCATED     (1 << 0) // Storage is managed by scl_value
#define SCL_VALUE_EXTERNAL      (1 << 1) // Storage is managed outside scl_value
#define SCL_VALUE_STATIC        (1 << 2) // Storage is in static memory and never modified

typedef struct scl_value
{
    uint32_t size;
    uint32_t flags;
    void * data[24 / sizeof(void *)];

} scl_value;

#define SCLINDEX( i )           &(scl_value){ sizeof(size_t), 0, { (void *)i } }
#define SCLDATA( ptr, size )    &(scl_value){ (uint32_t)size, SCL_VALUE_EXTERNAL, { ptr } }


typedef int (*scl_value_cmp)( scl_value * a, scl_value * b );
typedef scl_hash (*scl_hash_func)( const void * buffer, size_t buffer_size );


void scl_value_init( scl_value * value );
void scl_value_finalize( scl_value * value );

void scl_value_set( scl_value * value, const void * data, size_t size );
void * scl_value_get( scl_value * value, size_t * size );
void scl_value_clear( scl_value * value );

void * scl_value_data( scl_value * value );
size_t scl_value_size( scl_value * value );

void scl_value_copy( scl_value * dst, scl_value * src );
void scl_value_swap( scl_value * a, scl_value * b );

// A macro to extract the value data as the given type
#define scl_value_as( value, type )     *((type *)scl_value_data( value ))

// Compare functions
int scl_memcmp( scl_value * a, scl_value * b );
int scl_ptrcmp( scl_value * a, scl_value * b );
int scl_intcmp( scl_value * a, scl_value * b );
int scl_indexcmp( scl_value * a, scl_value * b );


#endif // _SCL_VALUE_H_


