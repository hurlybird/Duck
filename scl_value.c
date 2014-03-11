//
//  scl_value.c
//  scl
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "scl_env.h"
#include "scl_value.h"


///
//  scl_value_init()
//
void scl_value_init( scl_value * value )
{
    value->size = 0;
    value->flags = 0;
}


///
//  scl_value_finalize()
//
void scl_value_finalize( scl_value * value )
{
    scl_value_clear( value );
}


///
//  scl_value_set()
//
void scl_value_set( scl_value * value, const void * data, size_t size )
{
    scl_value_clear( value );
    
    if( size == 0 )
    {
        size = strlen( data ) + 1;
    }

    if( size > sizeof(value->data) )
    {
        value->flags |= SCL_VALUE_ALLOCATED;
        value->data[0] = scl_alloc( size );
        memcpy( value->data[0], data, size );
    }

    else
    {
        memcpy( value->data, data, size );
    }

    value->size = (uint32_t)size;
}


///
//  scl_value_get()
//
void * scl_value_get( scl_value * value, size_t * size )
{
    if( size )
    {
        *size = value->size;
    }

    if( value->size )
    {
        if( value->flags & (SCL_VALUE_ALLOCATED | SCL_VALUE_EXTERNAL) )
            return value->data[0];
        
        return value->data;
    }
    
    return NULL;
}


///
//  scl_value_clear()
//
void scl_value_clear( scl_value * value )
{
    if( value->flags & SCL_VALUE_ALLOCATED )
        scl_free( value->data[0] );
    
    value->size = 0;
    value->flags = 0;
}


///
//  scl_value_data()
//
void * scl_value_data( scl_value * value )
{
    if( value->flags & (SCL_VALUE_ALLOCATED | SCL_VALUE_EXTERNAL | SCL_VALUE_STATIC) )
        return value->data[0];
    
    else
        return value->data;
}


///
//  scl_value_size()
//
size_t scl_value_size( scl_value * value )
{
    return value->size;
}


///
//  scl_value_copy()
//
void scl_value_copy( scl_value * dst, scl_value * src )
{
    scl_value_clear( dst );
    
    if( src->flags & SCL_VALUE_STATIC )
    {
        dst->size = src->size;
        dst->flags = src->flags;
        dst->data[0] = src->data[0];
    }
    
    else
    {
        scl_value_set( dst, scl_value_data( src ), scl_value_size( src ) );
    }
}


///
//  scl_value_swap()
//
void scl_value_swap( scl_value * a, scl_value * b )
{
    scl_value tmp = *a;
    *a = *b;
    *b = tmp;
}


///
//  scl_memcmp()
//
int scl_memcmp( scl_value * a, scl_value * b )
{
    assert( a->size == b->size );
    return memcmp( scl_value_data( a ), scl_value_data( b ), a->size );
}


///
//  scl_ptrcmp()
//
int scl_ptrcmp( scl_value * a, scl_value * b )
{
    void * _a = scl_value_as( a, void * );
    void * _b = scl_value_as( b, void * );
    
    if( _a < _b )
        return 1;
    
    if( _a > _b )
        return -1;
    
    return 0;
}


///
//  scl_intcmp()
//
int scl_intcmp( scl_value * a, scl_value * b )
{
    int _a = scl_value_as( a, int );
    int _b = scl_value_as( b, int );
    return _b - _a;
}


///
//  scl_indexcmp()
//
int scl_indexcmp( scl_value * a, scl_value * b )
{
    size_t _a = scl_value_as( a, size_t );
    size_t _b = scl_value_as( b, size_t );

    if( _a < _b )
        return 1;
    
    if( _a > _b )
        return -1;
    
    return 0;
}






