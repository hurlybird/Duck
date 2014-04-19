/*****************************************************************************************

  DKElementArray.c

  Copyright (c) 2014 Derek W. Nylen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*****************************************************************************************/

#include "DKElementArray.h"


#define MIN_PTR_ARRAY_SIZE              32
#define HAS_ALLOCATED_STORAGE( array )  (((array)->elements != NULL) && ((array)->maxLength > 0))
#define HAS_EXTERNAL_STORAGE( array )   ((array)->maxLength < 0)


///
//  DKElementArrayInit()
//
void DKElementArrayInit( DKElementArray * array, DKIndex elementSize )
{
    DKAssert( elementSize > 0 );

    array->elements = NULL;
    array->elementSize = elementSize;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKElementArrayInitWithExternalStorage()
//
void DKElementArrayInitWithExternalStorage( DKElementArray * array, const void * elements, DKIndex elementSize, DKIndex length )
{
    array->elements = (uint8_t *)elements;
    array->elementSize = elementSize;
    array->length = length;
    array->maxLength = -1;
}


///
//  DKElementArrayFinalize()
//
void DKElementArrayFinalize( DKElementArray * array )
{
    if( HAS_ALLOCATED_STORAGE( array ) )
        dk_free( array->elements );
    
    array->elements = NULL;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKElementArrayReserve()
//
void DKElementArrayReserve( DKElementArray * array, DKIndex length )
{
    DKAssert( array->maxLength >= 0 );
    DKAssert( !HAS_EXTERNAL_STORAGE( array ) );
    
    if( array->maxLength < length )
    {
        if( length < MIN_PTR_ARRAY_SIZE )
            length = MIN_PTR_ARRAY_SIZE;
    
        uint8_t * elements = dk_malloc( length * array->elementSize );
        
        if( array->elements != NULL )
        {
            if( array->length > 0 )
                memcpy( elements, array->elements, array->length * array->elementSize );
            
            dk_free( array->elements );
        }
        
        array->elements = elements;
        array->maxLength = length;
    }
}


///
//  DKElementArrayHasExternalStorage()
//
int DKElementArrayHasExternalStorage( DKElementArray * array )
{
    return HAS_EXTERNAL_STORAGE( array );
}


///
//  ResizeArray()
//
static void * ResizeArray( void * ptr, DKIndex elementSize, DKIndex oldLength, DKIndex requestedLength, DKIndex * allocatedLength )
{
    if( requestedLength <= oldLength )
    {
        *allocatedLength = oldLength;
        return ptr;
    }
    
    DKIndex newLength = 2 * oldLength;
    
    if( newLength < requestedLength )
        newLength = requestedLength;
    
    if( newLength < MIN_PTR_ARRAY_SIZE )
        newLength = MIN_PTR_ARRAY_SIZE;
    
    *allocatedLength = newLength;
    
    return dk_malloc( newLength * elementSize );
}


///
//  DKElementArrayReplaceElements()
//
void DKElementArrayReplaceElements( DKElementArray * array, DKRange range, const void * elements, DKIndex length )
{
    DKAssert( array->length >= 0 );
    DKAssert( !HAS_EXTERNAL_STORAGE( array ) );
    
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
    uint8_t * newElements = ResizeArray( array->elements, array->elementSize, array->maxLength, newLength, &array->maxLength );
    DKAssert( newElements != NULL );
    
    if( array->elements != NULL )
    {
        if( array->elements != newElements )
        {
            // Copy prefix
            if( prefixRange.length > 0 )
            {
                memcpy( newElements, array->elements, prefixRange.length * array->elementSize );
            }
            
            // Copy suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uint8_t * dst = newElements + (suffixRangeAfterInsertion.location * array->elementSize);
                uint8_t * src = array->elements + (suffixRangeBeforeInsertion.location * array->elementSize);
                memcpy( dst, src, suffixRangeBeforeInsertion.length * array->elementSize );
            }
            
            dk_free( array->elements );
        }
        
        else
        {
            // Shift suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uint8_t * dst = newElements + (suffixRangeAfterInsertion.location * array->elementSize);
                uint8_t * src = newElements + (suffixRangeBeforeInsertion.location * array->elementSize);
                memmove( dst, src, suffixRangeBeforeInsertion.length * array->elementSize );
            }
        }
    }
    
    array->elements = newElements;
    array->length = newLength;
    
    // Insert or Extend
    if( insertedRange.length > 0 )
    {
        uint8_t * dst = array->elements + (insertedRange.location * array->elementSize);
        
        if( elements != NULL )
        {
            memcpy( dst, elements, insertedRange.length * array->elementSize );
        }
        
        else
        {
            memset( dst, 0, insertedRange.length * array->elementSize );
        }
    }
}


///
//  DKElementArrayAppendElements()
//
void DKElementArrayAppendElements( DKElementArray * array, const void * elements, DKIndex length )
{
    DKRange range = DKRangeMake( array->length, 0 );
    DKElementArrayReplaceElements( array, range, elements, length );
}


///
//  DKElementArraySort()
//
void DKElementArraySort( DKElementArray * array, DKCompareFunction cmp )
{
    qsort( array->elements, array->length, array->elementSize, cmp );
}


///
//  DKElementArrayReverse()
//
void DKElementArrayReverse( DKElementArray * array )
{
    DKIndex n = array->length / 2;

    if( array->elementSize == sizeof(intptr_t) )
    {
        for( DKIndex i = 0; i < n; ++i )
        {
            intptr_t * elem_i = DKElementArrayGetPointerToElementAtIndex( array, i );
            intptr_t * elem_j = DKElementArrayGetPointerToElementAtIndex( array, array->length - 1 - i );
            
            intptr_t tmp = *elem_i;
            *elem_i = *elem_j;
            *elem_j = tmp;
        }
    }
    
    else
    {
        uint8_t tmp[array->elementSize];
        
        for( DKIndex i = 0; i < n; ++i )
        {
            uint8_t * elem_i = DKElementArrayGetPointerToElementAtIndex( array, i );
            uint8_t * elem_j = DKElementArrayGetPointerToElementAtIndex( array, array->length - 1 - i );
            
            memcpy( tmp, elem_i, array->elementSize );
            memcpy( elem_i, elem_j, array->elementSize );
            memcpy( elem_j, tmp, array->elementSize );
        }
    }
}


///
//  DKElementArrayShuffle()
//
void DKElementArrayShuffle( DKElementArray * array )
{
    DKIndex n = array->length - 1;

    if( n > 0 )
    {
        if( array->elementSize == sizeof(intptr_t) )
        {
            for( DKIndex i = 0; i < n; ++i )
            {
                DKIndex j = rand() % n;
                
                intptr_t * elem_i = DKElementArrayGetPointerToElementAtIndex( array, i );
                intptr_t * elem_j = DKElementArrayGetPointerToElementAtIndex( array, j );
                
                intptr_t tmp = *elem_i;
                *elem_i = *elem_j;
                *elem_j = tmp;
            }
        }
        
        else
        {
            uint8_t tmp[array->elementSize];
        
            for( DKIndex i = 0; i < n; ++i )
            {
                DKIndex j = rand() % n;
                
                uint8_t * elem_i = DKElementArrayGetPointerToElementAtIndex( array, i );
                uint8_t * elem_j = DKElementArrayGetPointerToElementAtIndex( array, j );
                
                memcpy( tmp, elem_i, array->elementSize );
                memcpy( elem_i, elem_j, array->elementSize );
                memcpy( elem_j, tmp, array->elementSize );
            }
        }
    }
}





