/*****************************************************************************************

  DKLinkedList.h

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

#ifndef _DK_LINKED_LIST_H_
#define _DK_LINKED_LIST_H_

#include "DKList.h"


typedef struct DKLinkedList * DKLinkedListRef;
typedef struct DKLinkedList * DKMutableLinkedListRef;


DKClassRef  DKLinkedListClass( void );
DKClassRef  DKMutableLinkedListClass( void );

#define     DKLinkedListCreateEmpty()    DKCreate( DKLinkedListClass() )
#define     DKLinkedListCreateMutable()  DKCreate( DKMutableLinkedListClass() )

#define     DKLinkedListCreateWithCArray( cls, objects, count )  DKLinkedListInitWithObjects( DKAlloc( cls, 0 ), objects, count )
#define     DKLinkedListCreateWithCollection( cls, collection )  DKLinkedListInitWithCollection( DKAlloc( cls, 0 ), collection )

DKObjectRef DKLinkedListInitWithVAObjects( DKLinkedListRef _self, va_list objects );
DKObjectRef DKLinkedListInitWithCArray( DKLinkedListRef _self, DKObjectRef objects[], DKIndex count );
DKObjectRef DKLinkedListInitWithCollection( DKLinkedListRef _self, DKObjectRef srcCollection );

DKLinkedListRef DKLinkedListCopy( DKLinkedListRef _self );
DKMutableLinkedListRef DKLinkedListMutableCopy( DKLinkedListRef _self );

DKIndex     DKLinkedListGetCount( DKLinkedListRef _self );

DKObjectRef DKLinkedListGetObjectAtIndex( DKLinkedListRef _self, DKIndex index );
DKIndex     DKLinkedListGetObjectsInRange( DKLinkedListRef _self, DKRange range, DKObjectRef objects[] );

void        DKLinkedListAppendCArray( DKMutableLinkedListRef _self, DKObjectRef objects[], DKIndex count );
void        DKLinkedListAppendCollection( DKMutableLinkedListRef _self, DKObjectRef srcCollection );

void        DKLinkedListReplaceRangeWithCArray( DKMutableLinkedListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
void        DKLinkedListReplaceRangeWithCollection( DKMutableLinkedListRef _self, DKRange range, DKObjectRef srcCollection );

void        DKLinkedListSort( DKMutableLinkedListRef _self, DKCompareFunction cmp );
void        DKLinkedListShuffle( DKMutableLinkedListRef _self );

int         DKLinkedListApplyFunction( DKLinkedListRef _self, DKApplierFunction callback, void * context );



#endif // _DK_LINKED_LIST_H_





