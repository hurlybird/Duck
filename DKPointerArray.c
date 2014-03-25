//
//  DKPointerArray.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKPointerArray.h"


#define MIN_PTR_ARRAY_SIZE 32


///
//  DKPointerArrayInit()
//
void DKPointerArrayInit( DKPointerArray * array )
{
    array->data = NULL;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKPointerArrayInitWithExternalStorage()
//
void DKPointerArrayInitWithExternalStorage( DKPointerArray * array, const uintptr_t pointers[], DKIndex length )
{
    array->data = (uintptr_t *)pointers;
    array->length = length;
    array->maxLength = -1;
}


///
//  DKPointerArrayFinalize()
//
void DKPointerArrayFinalize( DKPointerArray * array )
{
    if( array->data && (array->maxLength > 0) )
        dk_free( array->data );
    
    array->data = NULL;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKPointerArrayReserve()
//
void DKPointerArrayReserve( DKPointerArray * array, DKIndex length )
{
    DKAssert( array->maxLength >= 0 );
    
    if( array->maxLength < length )
    {
        if( length < MIN_PTR_ARRAY_SIZE )
            length = MIN_PTR_ARRAY_SIZE;
    
        uintptr_t * data = dk_malloc( length * sizeof(uintptr_t) );
        
        if( array->length > 0 )
        {
            memcpy( data, array->data, array->length * sizeof(uintptr_t) );
            dk_free( array->data );
        }
        
        array->data = data;
        array->maxLength = length;
    }
}


///
//  DKPointerArrayHasExternalStorage()
//
int DKPointerArrayHasExternalStorage( DKPointerArray * array )
{
    return array->maxLength < 0;
}


///
//  DKPointerArrayResize()
//
static uintptr_t * DKPointerArrayResize( void * ptr, DKIndex oldSize, DKIndex requestedSize, DKIndex * allocatedSize )
{
    if( requestedSize < oldSize )
        return ptr;
    
    DKIndex newSize = 2 * oldSize;
    
    if( newSize < requestedSize )
        newSize = requestedSize;
    
    if( newSize < MIN_PTR_ARRAY_SIZE )
        newSize = MIN_PTR_ARRAY_SIZE;
    
    *allocatedSize = newSize;
    
    return dk_malloc( newSize * sizeof(uintptr_t) );
}


///
//  DKPointerArrayReplacePointers()
//
void DKPointerArrayReplacePointers( DKPointerArray * array, DKRange range, const uintptr_t pointers[], DKIndex length )
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
    uintptr_t * data = DKPointerArrayResize( array->data, array->maxLength, newLength, &array->maxLength );
    
    if( array->data )
    {
        if( array->data != data )
        {
            // Copy prefix
            if( prefixRange.length > 0 )
            {
                memcpy( data, array->data, prefixRange.length * sizeof(uintptr_t) );
            }
            
            // Copy suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uintptr_t * dst = &data[suffixRangeAfterInsertion.location];
                uintptr_t * src = &array->data[suffixRangeBeforeInsertion.location];
                memcpy( dst, src, suffixRangeBeforeInsertion.length * sizeof(uintptr_t) );
            }
            
            dk_free( array->data );
        }
        
        else
        {
            // Shift suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uintptr_t * dst = &data[suffixRangeAfterInsertion.location];
                uintptr_t * src = &data[suffixRangeBeforeInsertion.location];
                memmove( dst, src, suffixRangeBeforeInsertion.length * sizeof(uintptr_t) );
            }
        }
    }
    
    array->data = data;
    array->length = newLength;
    
    // Insert or Extend
    if( insertedRange.length > 0 )
    {
        uintptr_t * dst = &array->data[insertedRange.location];
        
        if( pointers != NULL )
        {
            memcpy( dst, pointers, insertedRange.length * sizeof(uintptr_t) );
        }
        
        else
        {
            memset( dst, 0, insertedRange.length * sizeof(uintptr_t) );
        }
    }
}


///
//  DKPointerArrayAppendPointer()
//
void DKPointerArrayAppendPointer( DKPointerArray * array, uintptr_t pointer )
{
    DKRange range = DKRangeMake( array->length, 0 );
    DKPointerArrayReplacePointers( array, range, &pointer, 1 );
}





