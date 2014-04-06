/*******************************************************************************

  DKPointerArray.c

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

*******************************************************************************/

#include "DKPointerArray.h"


#define MIN_PTR_ARRAY_SIZE              32
#define HAS_ALLOCATED_STORAGE( array )  (((array)->data != NULL) && ((array)->maxLength > 0))
#define HAS_EXTERNAL_STORAGE( array )   ((array)->maxLength < 0)


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
    if( HAS_ALLOCATED_STORAGE( array ) )
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
    DKAssert( !HAS_EXTERNAL_STORAGE( array ) );
    
    if( array->maxLength < length )
    {
        if( length < MIN_PTR_ARRAY_SIZE )
            length = MIN_PTR_ARRAY_SIZE;
    
        uintptr_t * data = dk_malloc( length * sizeof(uintptr_t) );
        
        if( array->data != NULL )
        {
            if( array->length > 0 )
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
    return HAS_EXTERNAL_STORAGE( array );
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
    DKAssert( !HAS_EXTERNAL_STORAGE( array ) );
    
    DKIndex range_end = DKRangeEnd( range );

    DKAssert( range.location >= 0 );
    DKAssert( range.length >= 0 );
    DKAssert( range_end <= array->length );
    DKAssert( ((pointers != NULL) && (length >= 0)) || ((pointers == NULL) && (length == 0)) );

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
    DKAssert( data != NULL );
    
    if( array->data != NULL )
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


///
//  DKPointerArraySort()
//
void DKPointerArraySort( DKPointerArray * array, DKCompareFunction cmp )
{
    qsort( array->data, array->length, sizeof(uintptr_t), cmp );
}


///
//  DKPointerArrayShuffle()
//
void DKPointerArrayShuffle( DKPointerArray * array )
{
    DKIndex n = array->length - 1;

    if( n > 0 )
    {
        for( DKIndex i = 0; i < n; ++i )
        {
            DKIndex j = rand() % n;
            
            uintptr_t tmp = array->data[i];
            array->data[i] = array->data[j];
            array->data[j] = tmp;
        }
    }
}





