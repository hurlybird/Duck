//
//  DKByteArray.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKByteArray.h"


#define MIN_BYTE_ARRAY_SIZE 64


///
//  DKByteArrayInit()
//
void DKByteArrayInit( DKByteArray * array )
{
    array->data = NULL;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKByteArrayReserve()
//
void DKByteArrayReserve( DKByteArray * array, DKIndex length )
{
    if( array->maxLength < length )
    {
        if( length < MIN_BYTE_ARRAY_SIZE )
            length = MIN_BYTE_ARRAY_SIZE;
    
        uint8_t * data = DKAlloc( length );
        
        if( array->length > 0 )
        {
            memcpy( data, array->data, array->length );
            DKFree( array->data );
        }
        
        array->data = data;
        array->maxLength = length;
    }
}


///
//  DKByteArrayClear()
//
void DKByteArrayClear( DKByteArray * array )
{
    DKFree( array->data );
    array->data = NULL;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKByteArrayResize()
//
static void * DKByteArrayResize( void * ptr, DKIndex oldSize, DKIndex requestedSize, DKIndex * allocatedSize )
{
    if( requestedSize < oldSize )
        return ptr;
    
    DKIndex newSize = 2 * oldSize;
    
    if( newSize < requestedSize )
        newSize = requestedSize;
    
    if( newSize < MIN_BYTE_ARRAY_SIZE )
        newSize = MIN_BYTE_ARRAY_SIZE;
    
    *allocatedSize = newSize;
    
    return DKAlloc( newSize );
}


///
//  DKByteArrayReplaceBytes()
//
void DKByteArrayReplaceBytes( DKByteArray * array, DKRange range, const void * bytes, DKIndex length )
{
    DKIndex range_end = DKRangeEnd( range );

    assert( range.location >= 0 );
    assert( range.length >= 0 );
    assert( range_end <= array->length );
    assert( length >= 0 );

    // Do nothing
    if( (range.length == 0) && (length == 0) )
        return;

    DKRange prefixRange = DKRangeMake( 0, range.location );
    DKRange insertedRange = DKRangeMake( range.location, length );
    DKRange suffixRangeBeforeInsertion = DKRangeMake( range_end, array->length - range_end );
    DKRange suffixRangeAfterInsertion = DKRangeMake( range.location + length, array->length - range_end );

    // Resize
    DKIndex newLength = array->length + length - range.length;
    assert( newLength >= 0 );

    // Resize
    uint8_t * data = DKByteArrayResize( array->data, array->maxLength, newLength, &array->maxLength );
    
    if( array->data )
    {
        if( array->data != data )
        {
            // Copy prefix
            if( prefixRange.length > 0 )
            {
                memcpy( data, array->data, prefixRange.length );
            }
            
            // Copy suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uint8_t * dst = &data[suffixRangeAfterInsertion.location];
                uint8_t * src = &array->data[suffixRangeBeforeInsertion.location];
                memcpy( dst, src, suffixRangeBeforeInsertion.length );
            }
            
            DKFree( array->data );
        }
        
        else
        {
            // Shift suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uint8_t * dst = &data[suffixRangeAfterInsertion.location];
                uint8_t * src = &data[suffixRangeBeforeInsertion.location];
                memmove( dst, src, suffixRangeBeforeInsertion.length );
            }
        }
    }
    
    array->data = data;
    array->length = newLength;
    
    // Insert or Extend
    if( insertedRange.length > 0 )
    {
        uint8_t * dst = &array->data[insertedRange.location];
        
        if( bytes != NULL )
        {
            memcpy( dst, bytes, insertedRange.length );
        }
        
        else
        {
            memset( dst, 0, insertedRange.length );
        }
    }
}





