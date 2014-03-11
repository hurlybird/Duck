//
//  scl_array.c
//  scl
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "scl_array.h"

// Interface =============================================================================

///
//  scl_array_init
//
void scl_array_init( scl_array * array, size_t element_size, size_t reserve_count )
{
    scl_block_init( &array->block, element_size * reserve_count );
    array->element_size = element_size;
}


///
//  scl_array_finalize()
//
void scl_array_finalize( scl_array * array )
{
    scl_block_finalize( &array->block );
    array->element_size = 0;
}


///
//  scl_array_size()
//
size_t scl_array_size( scl_array * array )
{
    return scl_block_size( &array->block ) / array->element_size;
}


///
//  scl_array_resize()
//
void scl_array_resize( scl_array * array, size_t count )
{
    scl_block_resize( &array->block, array->element_size * count );
}


///
//  scl_array_first()
//
void * scl_array_first( scl_array * array )
{
    return scl_block_range( &array->block, 0, array->element_size, 0 );
}


///
//  scl_array_last()
//
void * scl_array_last( scl_array * array )
{
    size_t size = scl_block_size( &array->block );
    
    if( size >= array->element_size )
    {
        size_t location = size - array->element_size;
        return scl_block_range( &array->block, location, array->element_size, 0 );
    }
        
    return NULL;
}


///
//  scl_array_index()
//
void * scl_array_index( scl_array * array, size_t index )
{
    size_t location = array->element_size * index;
    size_t length = array->element_size;

    return scl_block_range( &array->block, location, length, 0 );
}


///
//  scl_array_range()
//
void * scl_array_range( scl_array * array, size_t index, size_t count )
{
    size_t location = array->element_size * index;
    size_t length = array->element_size * count;

    return scl_block_range( &array->block, location, length, 0 );
}


///
//  scl_array_add()
//
void scl_array_add( scl_array * array, const void * buf, size_t count )
{
    size_t location = scl_block_size( &array->block );
    size_t length = array->element_size * count;

    void * dst = scl_block_insert( &array->block, location, length );
    memcpy( dst, buf, length );
}


///
//  scl_array_insert()
//
void scl_array_insert( scl_array * array, const void * buf, size_t index, size_t count )
{
    size_t location = array->element_size * index;
    size_t length = array->element_size * count;

    void * dst = scl_block_insert( &array->block, location, length );
    memcpy( dst, buf, length );
}


///
//  scl_array_remove_first()
//
void scl_array_remove_first( scl_array * array )
{
    scl_block_remove( &array->block, 0, array->element_size );
}


///
//  scl_array_remove_last()
//
void scl_array_remove_last( scl_array * array )
{
    size_t size = scl_block_size( &array->block );
    
    if( size >= array->element_size )
    {
        size_t location = size - array->element_size;
        scl_block_remove( &array->block, location, array->element_size );
    }
}


///
//  scl_array_remove_index()
//
void scl_array_remove_index( scl_array * array, size_t index )
{
    size_t location = array->element_size * index;
    scl_block_remove( &array->block, location, array->element_size );
}


///
//  scl_array_remove_range()
//
void scl_array_remove_range( scl_array * array, size_t index, size_t count )
{
    size_t location = array->element_size * index;
    size_t length = array->element_size * count;
    
    scl_block_remove( &array->block, location, length );
}


///
//  scl_array_remove_all()
//
void scl_array_remove_all( scl_array * array )
{
    scl_block_resize( &array->block, 0 );
}


///
//  scl_array_foreach()
//
int scl_array_foreach( scl_array * array, scl_array_traversal callback, void * context )
{
    int result = 0;
    
    size_t n = scl_array_size( array );
    
    for( size_t i = 0; i < n; ++i )
    {
        void * element = scl_array_index( array, i );
        
        if( (result = callback( context, element, array->element_size )) != 0 )
            break;
    }
    
    return result;
}







