/*****************************************************************************************

  DKGenericArray.c

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

#include "DKGenericArray.h"


#define MIN_ELEMENT_ARRAY_SIZE          32
#define HAS_ALLOCATED_STORAGE( array )  (((array)->elements != NULL) && ((array)->maxLength > 0))
#define HAS_EXTERNAL_STORAGE( array )   ((array)->maxLength < 0)


///
//  DKGenericArrayInit()
//
void DKGenericArrayInit( DKGenericArray * array, DKIndex elementSize )
{
    DKAssert( elementSize > 0 );

    array->elements = NULL;
    array->elementSize = elementSize;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKGenericArrayInitWithExternalStorage()
//
void DKGenericArrayInitWithExternalStorage( DKGenericArray * array, const void * elements, DKIndex elementSize, DKIndex length )
{
    array->elements = (uint8_t *)elements;
    array->elementSize = elementSize;
    array->length = length;
    array->maxLength = -1;
}


///
//  DKGenericArrayFinalize()
//
void DKGenericArrayFinalize( DKGenericArray * array )
{
    if( HAS_ALLOCATED_STORAGE( array ) )
        dk_free( array->elements );
    
    array->elements = NULL;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKGenericArrayReserve()
//
void DKGenericArrayReserve( DKGenericArray * array, DKIndex length )
{
    DKAssert( array->maxLength >= 0 );
    DKAssert( !HAS_EXTERNAL_STORAGE( array ) );
    
    if( array->maxLength < length )
    {
        if( length < MIN_ELEMENT_ARRAY_SIZE )
            length = MIN_ELEMENT_ARRAY_SIZE;
    
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
//  DKGenericArrayHasExternalStorage()
//
int DKGenericArrayHasExternalStorage( DKGenericArray * array )
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
    
    DKIndex newLength = oldLength + (oldLength >> 2); // 1.5x growth
    
    if( newLength < requestedLength )
        newLength = requestedLength;
    
    if( newLength < MIN_ELEMENT_ARRAY_SIZE )
        newLength = MIN_ELEMENT_ARRAY_SIZE;
    
    *allocatedLength = newLength;
    
    return dk_malloc( newLength * elementSize );
}


///
//  DKGenericArraySetLength()
//
void DKGenericArraySetLength( DKGenericArray * array, DKIndex length )
{
    DKGenericArrayReserve( array, length );
    array->length = length;
}


///
//  DKGenericArrayReplaceElements()
//
void DKGenericArrayReplaceElements( DKGenericArray * array, DKRange range, const void * elements, DKIndex length )
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
//  DKGenericArrayAppendElements()
//
void DKGenericArrayAppendElements( DKGenericArray * array, const void * elements, DKIndex length )
{
    DKRange range = DKRangeMake( array->length, 0 );
    DKGenericArrayReplaceElements( array, range, elements, length );
}


///
//  DKGenericArrayGetPointerToElementAtIndex()
//
void * DKGenericArrayGetPointerToElementAtIndex( DKGenericArray * array, DKIndex index )
{
    DKCheckIndex( index, array->length, NULL );
    return (void *)(array->elements + (index * array->elementSize));
}


///
//  DKGenericArraySort()
//
void DKGenericArraySort( DKGenericArray * array, int (*cmp)(const void *, const void *) )
{
    qsort( array->elements, array->length, array->elementSize, cmp );
}


///
//  DKGenericArraySortObjects()
//
static void BubbleSort( DKObjectRef * array, DKIndex lo, DKIndex hi, DKCompareFunction cmp )
{
    for( ; lo < hi; --hi )
    {
        for( DKIndex i = lo; i < hi; ++i )
        {
            if( cmp( array[i], array[i+1] ) < 0 )
            {
                DKObjectRef swap = array[i];
                array[i] = array[i+1];
                array[i+1] = swap;
            }
        }
    }
}

static DKObjectRef QuickSortPivot( DKObjectRef * array, DKIndex lo, DKIndex hi, DKCompareFunction cmp )
{
    DKIndex mid = lo + (hi - lo) / 2;
    DKObjectRef swap;

    if( cmp( array[mid], array[lo] ) < 0 )
    {
        swap = array[lo];
        array[lo] = array[mid];
        array[mid] = swap;
    }
    
    if( cmp( array[hi], array[lo] ) < 0 )
    {
        swap = array[lo];
        array[lo] = array[hi];
        array[hi] = swap;
    }

    if( cmp( array[mid], array[hi] ) < 0 )
    {
        swap = array[mid];
        array[mid] = array[hi];
        array[hi] = swap;
    }

    return array[hi];
}

static DKIndex QuickSortPartition( DKObjectRef * array, DKIndex lo, DKIndex hi, DKCompareFunction cmp )
{
    DKObjectRef pivot = QuickSortPivot( array, lo, hi, cmp );
    DKIndex i = lo - 1;
    DKIndex j = hi + 1;
    DKObjectRef swap;

    while( true )
    {
        while( cmp( array[++i], pivot ) > 0 )
            ;
        
        while( cmp( array[--j], pivot ) < 0 )
            ;
        
        if( i >= j )
            return j;
        
        swap = array[i];
        array[i] = array[j];
        array[j] = swap;
    }
}

static void QuickSort( DKObjectRef * array, DKIndex lo, DKIndex hi, DKCompareFunction cmp )
{
    if( (hi - lo) < 8 )
    {
        BubbleSort( array, lo, hi, cmp );
        return;
    }
 
    if( lo < hi )
    {
        DKIndex p = QuickSortPartition( array, lo, hi, cmp );
        
        QuickSort( array, lo, p, cmp );
        QuickSort( array, p + 1, hi, cmp );
    }
}



void DKGenericArraySortObjects( DKGenericArray * array, DKCompareFunction cmp )
{
    DKAssert( array->elementSize == sizeof(DKObjectRef) );
    QuickSort( (DKObjectRef *)array->elements, 0, array->length - 1, cmp );
}


///
//  DKGenericArrayReverse()
//
static void Reverse16( DKGenericArray * array )
{
    DKIndex n = array->length / 2;

    for( DKIndex i = 0; i < n; ++i )
    {
        int16_t * elem_i = DKGenericArrayGetPointerToElementAtIndex( array, i );
        int16_t * elem_j = DKGenericArrayGetPointerToElementAtIndex( array, array->length - 1 - i );
        
        int16_t tmp = *elem_i;
        *elem_i = *elem_j;
        *elem_j = tmp;
    }
}

static void Reverse32( DKGenericArray * array )
{
    DKIndex n = array->length / 2;

    for( DKIndex i = 0; i < n; ++i )
    {
        int32_t * elem_i = DKGenericArrayGetPointerToElementAtIndex( array, i );
        int32_t * elem_j = DKGenericArrayGetPointerToElementAtIndex( array, array->length - 1 - i );
        
        int32_t tmp = *elem_i;
        *elem_i = *elem_j;
        *elem_j = tmp;
    }
}

static void Reverse64( DKGenericArray * array )
{
    DKIndex n = array->length / 2;

    for( DKIndex i = 0; i < n; ++i )
    {
        int64_t * elem_i = DKGenericArrayGetPointerToElementAtIndex( array, i );
        int64_t * elem_j = DKGenericArrayGetPointerToElementAtIndex( array, array->length - 1 - i );
        
        int64_t tmp = *elem_i;
        *elem_i = *elem_j;
        *elem_j = tmp;
    }
}

static void Reverse( DKGenericArray * array )
{
    DKIndex n = array->length / 2;
    uint8_t tmp[array->elementSize];
    
    for( DKIndex i = 0; i < n; ++i )
    {
        uint8_t * elem_i = DKGenericArrayGetPointerToElementAtIndex( array, i );
        uint8_t * elem_j = DKGenericArrayGetPointerToElementAtIndex( array, array->length - 1 - i );
        
        memcpy( tmp, elem_i, array->elementSize );
        memcpy( elem_i, elem_j, array->elementSize );
        memcpy( elem_j, tmp, array->elementSize );
    }
}

void DKGenericArrayReverse( DKGenericArray * array )
{
    switch( array->elementSize )
    {
    case 2:
        Reverse16( array );
        break;
        
    case 4:
        Reverse32( array );
        break;
        
    case 8:
        Reverse64( array );
        break;
        
    default:
        Reverse( array );
        break;
    }
}


///
//  DKGenericArrayShuffle()
//
static void Shuffle16( DKGenericArray * array )
{
    DKIndex n = array->length - 1;

    for( DKIndex i = 0; i < n; ++i )
    {
        DKIndex j = rand() % n;
        
        int16_t * elem_i = DKGenericArrayGetPointerToElementAtIndex( array, i );
        int16_t * elem_j = DKGenericArrayGetPointerToElementAtIndex( array, j );
        
        int16_t tmp = *elem_i;
        *elem_i = *elem_j;
        *elem_j = tmp;
    }
}

static void Shuffle32( DKGenericArray * array )
{
    DKIndex n = array->length - 1;

    for( DKIndex i = 0; i < n; ++i )
    {
        DKIndex j = rand() % n;
        
        int32_t * elem_i = DKGenericArrayGetPointerToElementAtIndex( array, i );
        int32_t * elem_j = DKGenericArrayGetPointerToElementAtIndex( array, j );
        
        int32_t tmp = *elem_i;
        *elem_i = *elem_j;
        *elem_j = tmp;
    }
}

static void Shuffle64( DKGenericArray * array )
{
    DKIndex n = array->length - 1;

    for( DKIndex i = 0; i < n; ++i )
    {
        DKIndex j = rand() % n;
        
        int64_t * elem_i = DKGenericArrayGetPointerToElementAtIndex( array, i );
        int64_t * elem_j = DKGenericArrayGetPointerToElementAtIndex( array, j );
        
        int64_t tmp = *elem_i;
        *elem_i = *elem_j;
        *elem_j = tmp;
    }
}

static void Shuffle( DKGenericArray * array )
{
    DKIndex n = array->length - 1;
    uint8_t tmp[array->elementSize];

    for( DKIndex i = 0; i < n; ++i )
    {
        DKIndex j = rand() % n;
        
        uint8_t * elem_i = DKGenericArrayGetPointerToElementAtIndex( array, i );
        uint8_t * elem_j = DKGenericArrayGetPointerToElementAtIndex( array, j );
        
        memcpy( tmp, elem_i, array->elementSize );
        memcpy( elem_i, elem_j, array->elementSize );
        memcpy( elem_j, tmp, array->elementSize );
    }
}

void DKGenericArrayShuffle( DKGenericArray * array )
{
    switch( array->elementSize )
    {
    case 2:
        Shuffle16( array );
        break;
        
    case 4:
        Shuffle32( array );
        break;
        
    case 8:
        Shuffle64( array );
        
    default:
        Shuffle( array );
        break;
    }
}





