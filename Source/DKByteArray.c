/*****************************************************************************************

  DKByteArray.c

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

#include "DKByteArray.h"


#define MIN_BYTE_ARRAY_SIZE             64
#define HAS_ALLOCATED_STORAGE( array )  (((array)->bytes != NULL_BYTES) && ((array)->maxLength > 0))
#define HAS_EXTERNAL_STORAGE( array )   ((array)->maxLength < 0)


// This allows an unallocated byte array to point to an empty string instead of NULL
#define NULL_TERMINATOR_SIZE            4

static uint8_t NULL_BYTES[NULL_TERMINATOR_SIZE] = { '\0', '\0', '\0', '\0' };


///
//  SetNullTerminator()
//
static void SetNullTerminator( DKByteArray * array )
{
    array->bytes[array->length] = '\0';
    array->bytes[array->length+1] = '\0';
    array->bytes[array->length+2] = '\0';
    array->bytes[array->length+3] = '\0';
}


///
//  DKByteArrayInit()
//
void DKByteArrayInit( DKByteArray * array )
{
    array->bytes = NULL_BYTES;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKByteArrayInitWithExternalStorage()
//
void DKByteArrayInitWithExternalStorage( DKByteArray * array, const uint8_t bytes[], DKIndex length )
{
    array->bytes = (uint8_t *)bytes;
    array->length = length;
    array->maxLength = -1;
}


///
//  DKByteArrayFinalize()
//
void DKByteArrayFinalize( DKByteArray * array )
{
    if( HAS_ALLOCATED_STORAGE( array ) )
        dk_free( array->bytes );
    
    array->bytes = NULL_BYTES;
    array->length = 0;
    array->maxLength = 0;
}


///
//  DKByteArrayReserve()
//
void DKByteArrayReserve( DKByteArray * array, DKIndex length )
{
    DKAssert( array->length >= 0 );
    DKAssert( !HAS_EXTERNAL_STORAGE( array ) );

    if( array->maxLength < length )
    {
        if( length < MIN_BYTE_ARRAY_SIZE )
            length = MIN_BYTE_ARRAY_SIZE;
    
        uint8_t * bytes = dk_malloc( length + NULL_TERMINATOR_SIZE );
        
        if( HAS_ALLOCATED_STORAGE( array ) )
        {
            if( array->length > 0 )
                memcpy( bytes, array->bytes, array->length );
            
            dk_free( array->bytes );
        }
        
        array->bytes = bytes;
        array->maxLength = length;

        SetNullTerminator( array );
    }
}


///
//  DKByteArrayHasExternalStorage()
//
int DKByteArrayHasExternalStorage( DKByteArray * array )
{
    return HAS_EXTERNAL_STORAGE( array );
}


///
//  ResizeArray()
//
static void * ResizeArray( void * ptr, DKIndex oldLength, DKIndex requestedLength, DKIndex * allocatedLength )
{
    if( requestedLength < oldLength )
    {
        *allocatedLength = oldLength;
        return ptr;
    }
    
    DKIndex newLength = 2 * oldLength;
    
    if( newLength < requestedLength )
        newLength = requestedLength;
    
    if( newLength < MIN_BYTE_ARRAY_SIZE )
        newLength = MIN_BYTE_ARRAY_SIZE;
    
    *allocatedLength = newLength;
    
    return dk_malloc( newLength + NULL_TERMINATOR_SIZE );
}


///
//  DKByteArrayReplaceBytes()
//
void DKByteArrayReplaceBytes( DKByteArray * array, DKRange range, const uint8_t bytes[], DKIndex length )
{
    DKAssert( array->length >= 0 );
    DKAssert( !HAS_EXTERNAL_STORAGE( array ) );

    DKIndex range_end = DKRangeEnd( range );

    DKAssert( range.location >= 0 );
    DKAssert( range.length >= 0 );
    DKAssert( range_end <= array->length );
    //DKAssert( ((bytes != NULL) && (length >= 0)) || ((bytes == NULL) && (length == 0)) );
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
    uint8_t * newBytes = ResizeArray( array->bytes, array->maxLength, newLength, &array->maxLength );
    DKAssert( newBytes != NULL );
    
    if( array->bytes != NULL_BYTES )
    {
        if( array->bytes != newBytes )
        {
            // Copy prefix
            if( prefixRange.length > 0 )
            {
                memcpy( newBytes, array->bytes, prefixRange.length );
            }
            
            // Copy suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uint8_t * dst = &newBytes[suffixRangeAfterInsertion.location];
                uint8_t * src = &array->bytes[suffixRangeBeforeInsertion.location];
                memcpy( dst, src, suffixRangeBeforeInsertion.length );
            }
            
            dk_free( array->bytes );
        }
        
        else
        {
            // Shift suffix
            if( suffixRangeBeforeInsertion.length > 0 )
            {
                uint8_t * dst = &newBytes[suffixRangeAfterInsertion.location];
                uint8_t * src = &newBytes[suffixRangeBeforeInsertion.location];
                memmove( dst, src, suffixRangeBeforeInsertion.length );
            }
        }
    }
    
    array->bytes = newBytes;
    array->length = newLength;
    
    // Insert or Extend
    if( insertedRange.length > 0 )
    {
        uint8_t * dst = &array->bytes[insertedRange.location];
        
        if( bytes != NULL )
        {
            memcpy( dst, bytes, insertedRange.length );
        }
        
        else
        {
            memset( dst, 0, insertedRange.length );
        }
    }
    
    SetNullTerminator( array );
}


///
//  DKByteArrayAppendBytes()
//
void DKByteArrayAppendBytes( DKByteArray * array, const uint8_t bytes[], DKIndex length )
{
    DKRange range = DKRangeMake( array->length, 0 );
    DKByteArrayReplaceBytes( array, range, bytes, length );
}





