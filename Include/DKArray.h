/*****************************************************************************************

  DKArray.h

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

#ifndef _DK_ARRAY_H_
#define _DK_ARRAY_H_

#include "DKList.h"


typedef const struct DKArray * DKArrayRef;
typedef struct DKArray * DKMutableArrayRef;


DKClassRef DKArrayClass( void );
DKClassRef DKMutableArrayClass( void );

DKArrayRef DKArrayCreate( void );
DKArrayRef DKArrayCreateWithObjects( DKObjectRef firstObject, ... );
DKArrayRef DKArrayCreateWithCArray( DKObjectRef objects[], DKIndex count );
DKArrayRef DKArrayCreateWithCArrayNoCopy( DKObjectRef objects[], DKIndex count );
DKArrayRef DKArrayCreateCopy( DKListRef srcList );

DKMutableArrayRef DKArrayCreateMutable( void );
DKMutableArrayRef DKArrayCreateMutableCopy( DKListRef srcList );

DKIndex DKArrayGetCount( DKArrayRef _self );
DKIndex DKArrayGetObjects( DKArrayRef _self, DKRange range, DKObjectRef objects[] );

void    DKArrayReplaceObjects( DKMutableArrayRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
void    DKArrayReplaceObjectsWithList( DKMutableArrayRef _self, DKRange range, DKListRef srcList );

void    DKArraySort( DKMutableArrayRef _self, DKCompareFunction cmp );
void    DKArrayShuffle( DKMutableArrayRef _self );


#endif // _DK_ARRAY_H_










