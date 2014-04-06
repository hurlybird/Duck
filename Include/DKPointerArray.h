/*****************************************************************************************

  DKPointerArray.h

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

#ifndef _DK_POINTER_ARRAY_H_
#define _DK_POINTER_ARRAY_H_

#include "DKPlatform.h"


typedef struct
{
    uintptr_t * data;
    DKIndex length;
    DKIndex maxLength;

} DKPointerArray;


void DKPointerArrayInit( DKPointerArray * array );

void DKPointerArrayInitWithExternalStorage( DKPointerArray * array, const uintptr_t pointers[], DKIndex length );
int  DKPointerArrayHasExternalStorage( DKPointerArray * array );

void DKPointerArrayFinalize( DKPointerArray * array );

void DKPointerArrayReserve( DKPointerArray * array, DKIndex length );

void DKPointerArrayReplacePointers( DKPointerArray * array, DKRange range, const uintptr_t pointers[], DKIndex length );
void DKPointerArrayAppendPointer( DKPointerArray * array, uintptr_t pointer );

void DKPointerArraySort( DKPointerArray * array, DKCompareFunction cmp );
void DKPointerArrayShuffle( DKPointerArray * array );



#endif // _DK_ELEMENT_ARRAY_H_
