/*****************************************************************************************

  DKElementArray.h

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

#ifndef _DK_ELEMENT_ARRAY_H_
#define _DK_ELEMENT_ARRAY_H_

#include "DKPlatform.h"


typedef struct
{
    uint8_t * elements;
    DKIndex elementSize;
    DKIndex length;
    DKIndex maxLength;

} DKElementArray;


void DKElementArrayInit( DKElementArray * array, DKIndex elementSize );

void DKElementArrayInitWithExternalStorage( DKElementArray * array, const void * elements, DKIndex elementSize, DKIndex length );
int  DKElementArrayHasExternalStorage( DKElementArray * array );

void DKElementArrayFinalize( DKElementArray * array );

void DKElementArrayReserve( DKElementArray * array, DKIndex length );

#define DKElementArrayGetLength( array ) ((array)->length)

#define DKElementArrayGetPointerToElementAtIndex( array, index )                        \
     ((void *)((array)->elements + ((index) * (array)->elementSize)))

#define DKElementArrayGetElementAtIndex( array, index, type )                           \
    *((type *)((array)->elements + ((index) * (array)->elementSize)))

void DKElementArrayReplaceElements( DKElementArray * array, DKRange range, const void * elements, DKIndex length );
void DKElementArrayAppendElements( DKElementArray * array, const void * elements, DKIndex length );

void DKElementArraySort( DKElementArray * array, DKCompareFunction cmp );
void DKElementArrayReverse( DKElementArray * array );
void DKElementArrayShuffle( DKElementArray * array );



#endif // _DK_ELEMENT_ARRAY_H_
