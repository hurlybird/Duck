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


DKClassRef  DKArrayClass( void );
DKClassRef  DKMutableArrayClass( void );

#define     DKArrayCreateEmpty()    DKCreate( DKArrayClass() )
#define     DKArrayCreateMutable()  DKCreate( DKMutableArrayClass() )

#define     DKArrayCreateWithCArray( cls, objects, count )  DKArrayInitWithObjects( DKAlloc( cls, 0 ), objects, count )
#define     DKArrayCreateWithCollection( cls, collection )  DKArrayInitWithCollection( DKAlloc( cls, 0 ), collection )
DKObjectRef DKArrayCreateWithCArrayNoCopy( /* DKClassRef _class, */ DKObjectRef objects[], DKIndex count );

DKObjectRef DKArrayInitWithVAObjects( DKArrayRef _self, va_list objects );
DKObjectRef DKArrayInitWithCArray( DKArrayRef _self, DKObjectRef objects[], DKIndex count );
DKObjectRef DKArrayInitWithCollection( DKArrayRef _self, DKObjectRef collection );

DKArrayRef  DKArrayCopy( DKArrayRef _self );
DKMutableArrayRef DKArrayMutableCopy( DKArrayRef _self );

DKIndex     DKArrayGetCount( DKArrayRef _self );
DKIndex     DKArrayGetObjects( DKArrayRef _self, DKObjectRef objects[] );

DKObjectRef DKArrayGetObjectAtIndex( DKArrayRef _self, DKIndex index );
DKIndex     DKArrayGetObjectsInRange( DKArrayRef _self, DKRange range, DKObjectRef objects[] );

void        DKArrayAppendCArray( DKMutableArrayRef _self, DKObjectRef objects[], DKIndex count );
void        DKArrayAppendCollection( DKMutableArrayRef _self, DKObjectRef srcCollection );

void        DKArrayReplaceRangeWithCArray( DKMutableArrayRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
void        DKArrayReplaceRangeWithCollection( DKMutableArrayRef _self, DKRange range, DKObjectRef collection );

void        DKArraySort( DKMutableArrayRef _self, DKCompareFunction cmp );
void        DKArrayShuffle( DKMutableArrayRef _self );

int         DKArrayApplyFunction( DKArrayRef _self, DKApplierFunction callback, void * context );




#endif // _DK_ARRAY_H_









