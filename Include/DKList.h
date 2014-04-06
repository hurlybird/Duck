/*****************************************************************************************

  DKList.h

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

#ifndef _DK_LIST_H_
#define _DK_LIST_H_

#include "DKRuntime.h"


DKDeclareInterfaceSelector( List );


//typedef const void * DKListRef; -- Declared in DKPlatform.h
typedef void * DKMutableListRef;

typedef DKIndex (*DKListGetCountMethod)( DKListRef _self );
typedef DKIndex (*DKListGetObjectsMethod)( DKListRef _self, DKRange range, DKObjectRef objects[] );
typedef void    (*DKListReplaceObjectsMethod)( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
typedef void    (*DKListReplaceObjectsWithListMethod)( DKMutableListRef _self, DKRange range, DKListRef srcList );
typedef void    (*DKListSortMethod)( DKMutableListRef _self, DKCompareFunction cmp );
typedef void    (*DKListShuffleMethod)( DKMutableListRef _self );

typedef int (*DKListApplierFunction)( DKObjectRef object, void * context );

struct DKList
{
    DKInterface _interface;

    DKListGetCountMethod        getCount;
    DKListGetObjectsMethod      getObjects;
    DKListReplaceObjectsMethod  replaceObjects;
    DKListReplaceObjectsWithListMethod replaceObjectsWithList;
    DKListSortMethod            sort;
    DKListShuffleMethod         shuffle;
};

typedef const struct DKList DKList;


DKClassRef  DKListClass( void );
void        DKSetListClass( DKClassRef _self );

DKClassRef  DKMutableListClass( void );
void        DKSetMutableListClass( DKClassRef _self );

DKIndex     DKListGetCount( DKListRef _self );
DKIndex     DKListGetCountOfObject( DKListRef _self, DKObjectRef object );
DKIndex     DKListGetFirstIndexOfObject( DKListRef _self, DKObjectRef object );
DKIndex     DKListGetLastIndexOfObject( DKListRef _self, DKObjectRef object );

DKObjectRef DKListGetObjectAtIndex( DKListRef _self, DKIndex index );
DKIndex     DKListGetObjects( DKListRef _self, DKRange range, DKObjectRef objects[] );

int         DKListApplyFunction( DKListRef _self, DKListApplierFunction callback, void * context );

void        DKListAppendObject( DKMutableListRef _self, DKObjectRef object );
void        DKListAppendList( DKMutableListRef _self, DKListRef srcList );
void        DKListSetObjectAtIndex( DKMutableListRef _self, DKIndex index, DKObjectRef object );
void        DKListInsertObjectAtIndex( DKMutableListRef _self, DKIndex index, DKObjectRef object );
void        DKListReplaceObjects( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
void        DKListReplaceObjectsWithList( DKMutableListRef _self, DKRange range, DKListRef srcList );
void        DKListRemoveObjectAtIndex( DKMutableListRef _self, DKIndex index );
void        DKListRemoveAllObjects( DKMutableListRef _self );

void        DKListSort( DKMutableListRef _self, DKCompareFunction cmp );
void        DKListShuffle( DKMutableListRef _self );

DKStringRef DKListCopyDescription( DKListRef _self );


#endif // _DK_LIST_H_
