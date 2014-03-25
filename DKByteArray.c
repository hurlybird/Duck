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
//  DKByteArrayInitWithExternalStorage()
//
void DKByteArrayInitWithExternalStorage( DKByteArray * array, const uint8_t bytes[], DKIndex length )
{
    array->data = (uint8_t *)bytes;
    array->length = length;
    array->maxLength = -1;
}


///
//  DKByteArrayFinalize()
//
void DKByteArrayFinalize( DKByteArray * array )
{
    if( array->data && (array->maxLength > 0) )
        dk_free( array->data );
    
    array->data = NULL;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKByteArrayReserve()
//
void DKByteArrayReserve( DKByteArray * array, DKIndex length )
{
    DKAssert( array->length >= 0 );

    if( array->maxLength < length )
    {
        if( length < MIN_BYTE_ARRAY_SIZE )
            length = MIN_BYTE_ARRAY_SIZE;
    
        uint8_t * data = dk_malloc( length );
        
        if( array->length > 0 )
        {
            memcpy( data, array->data, array->length );
            dk_free( array->data );
        }
        
        array->data = data;
        array->maxLength = length;
    }
}


///
//  DKByteArrayHasExternalStorage()
//
int DKByteArrayHasExternalStorage( DKByteArray * array )
{
    return array->maxLength < 0;
}


///
//  DKByteArrayResize()
//
static uint8_t * DKByteArrayResize( void * ptr, DKIndex oldSize, DKIndex requestedSize, DKIndex * allocatedSize )
{
    if( requestedSize < oldSize )
        return ptr;
    
    DKIndex newSize = 2 * oldSize;
    
    if( newSize < requestedSize )
        newSize = requestedSize;
    
    if( newSize < MIN_BYTE_ARRAY_SIZE )
        newSize = MIN_BYTE_ARRAY_SIZE;
    
    *allocatedSize = newSize;
    
    return dk_malloc( newSize );
}


///
//  DKByteArrayReplaceBytes()
//
void DKByteArrayReplaceBytes( DKByteArray * array, DKRange range, const uint8_t bytes[], DKIndex length )
{
    DKAssert( array->length >= 0 );

    DKIndex range_end = DKRangeEnd( range );

    DKAssert( range.location >= 0 );
    DKAssert( range.length >= 0 );
    DKAssert( range_end <= array->length );
    DKAssert( length >= 0 );

    // Do nothing
    if( (range.length == 0) && (length == 0) )
        return;

    DKRange prefixRange = DKRangeMake( 0, range.location );
    DKRange insertedRange = DKRangeMake( range.location, length );
    DKRange suffixRangeBeforeInsertion = DKRangeMake( range_end, array->length - range_end );
    DKRange suffixRangeAfterInsertion = DKRangeMake( range.location + length, array->length - range_end );

    // Resize
    DKIndex newLength = array->length + length - range.length;
    DKAssert( newLength >= 0 );

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
            
            dk_free( array->data );
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


///
//  DKByteArrayAppendBytes()
//
void DKByteArrayAppendBytes( DKByteArray * array, const uint8_t bytes[], DKIndex length )
{
    DKRange range = DKRangeMake( array->length, 0 );
    DKByteArrayReplaceBytes( array, range, bytes, length );
}





