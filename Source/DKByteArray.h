/*****************************************************************************************

  DKByteArray.h

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

#ifndef _DK_BYTE_ARRAY_H_
#define _DK_BYTE_ARRAY_H_

#include "DKPlatform.h"

// DKByteArray MUST guarantee that its storage is a contiguous C array of bytes (DKData
// and DKString rely on this behaviour).

// Note: DKByteArray internally stores four '\0' bytes (i.e. a UTF32 NULL) at data[length]
// to make storing strings safer. The NULLs aren't included in the length or maxLength of
// the array.

typedef struct
{
    uint8_t * bytes;
    DKIndex length;
    DKIndex maxLength;

} DKByteArray;


DK_API void DKByteArrayInit( DKByteArray * array );

DK_API void DKByteArrayInitWithExternalStorage( DKByteArray * array, const uint8_t bytes[], DKIndex length );
DK_API bool DKByteArrayHasExternalStorage( DKByteArray * array );

DK_API void DKByteArrayFinalize( DKByteArray * array );

DK_API void DKByteArrayReserve( DKByteArray * array, DKIndex length );

#define DKByteArrayGetLength( array )       ((array)->length)
#define DKByteArrayGetBytePtr( array, i )   ((void *)&((array)->bytes[i]))

DK_API void DKByteArraySetLength( DKByteArray * array, DKIndex length );

DK_API void DKByteArrayReplaceBytes( DKByteArray * array, DKRange range, const uint8_t bytes[], DKIndex length );
DK_API void DKByteArrayAppendBytes( DKByteArray * array, const uint8_t bytes[], DKIndex length );

DK_API DKIndex DKByteArrayAlignLength( DKByteArray * array, DKIndex byteAlignment );

#endif // _DK_BYTE_ARRAY_H_
