/*****************************************************************************************

  DKGenericArray.h

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

#ifndef _DK_GENERIC_ARRAY_H_
#define _DK_GENERIC_ARRAY_H_

#include "DKPlatform.h"


typedef struct
{
    uint8_t * elements;
    DKIndex elementSize;
    DKIndex length;
    DKIndex maxLength;

} DKGenericArray;


void DKGenericArrayInit( DKGenericArray * array, DKIndex elementSize );

void DKGenericArrayInitWithExternalStorage( DKGenericArray * array, const void * elements, DKIndex elementSize, DKIndex length );
int  DKGenericArrayHasExternalStorage( DKGenericArray * array );

void DKGenericArrayFinalize( DKGenericArray * array );

void DKGenericArrayReserve( DKGenericArray * array, DKIndex length );

#define DKGenericArrayGetLength( array ) ((array)->length)

#define DKGenericArrayGetPointerToElementAtIndex( array, index )                        \
     ((void *)((array)->elements + ((index) * (array)->elementSize)))

#define DKGenericArrayGetElementAtIndex( array, index, type )                           \
    *((type *)((array)->elements + ((index) * (array)->elementSize)))

void DKGenericArrayReplaceElements( DKGenericArray * array, DKRange range, const void * elements, DKIndex length );
void DKGenericArrayAppendElements( DKGenericArray * array, const void * elements, DKIndex length );

void DKGenericArraySort( DKGenericArray * array, DKCompareFunction cmp );
void DKGenericArrayReverse( DKGenericArray * array );
void DKGenericArrayShuffle( DKGenericArray * array );



#endif // _DK_GENERIC_ARRAY_H_
