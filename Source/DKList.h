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
#include "DKCollection.h"


DKDeclareInterfaceSelector( List );


//typedef const void * DKListRef; -- Declared in DKPlatform.h
typedef void * DKMutableListRef;

typedef DKObjectRef (*DKListInitWithVAObjectsMethod)( DKListRef _self, va_list objects );
typedef DKObjectRef (*DKListInitWithCArrayMethod)( DKListRef _self, DKObjectRef objects[], DKIndex count );
typedef DKObjectRef (*DKListInitWithCollectionMethod)( DKListRef _self, DKObjectRef srcCollection );

typedef DKObjectRef (*DKListGetObjectAtIndexMethod)( DKListRef _self, DKIndex index );
typedef DKIndex     (*DKListGetObjectsInRangeMethod)( DKListRef _self, DKRange range, DKObjectRef objects[] );

typedef void        (*DKListAppendCArrayMethod)( DKMutableListRef _self, DKObjectRef objects[], DKIndex count );
typedef void        (*DKListAppendCollectionMethod)( DKMutableListRef _self, DKListRef srcList );

typedef void        (*DKListReplaceRangeWithCArrayMethod)( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
typedef void        (*DKListReplaceRangeWithCollectionMethod)( DKMutableListRef _self, DKRange range, DKListRef srcList );

typedef void        (*DKListSortMethod)( DKMutableListRef _self, DKCompareFunction cmp );
typedef void        (*DKListShuffleMethod)( DKMutableListRef _self );

struct DKListInterface
{
    const DKInterface _interface;

    // List creation
    DKListInitWithVAObjectsMethod           initWithVAObjects;
    DKListInitWithCArrayMethod              initWithCArray;
    DKListInitWithCollectionMethod          initWithCollection;

    // Immutable lists
    DKGetCountMethod                        getCount;
    DKListGetObjectAtIndexMethod            getObjectAtIndex;
    DKListGetObjectsInRangeMethod           getObjectsInRange;
    
    // Mutable lists -- these raise errors when called on immutable lists
    DKListAppendCArrayMethod                appendCArray;
    DKListAppendCollectionMethod            appendCollection;
    
    DKListReplaceRangeWithCArrayMethod      replaceRangeWithCArray;
    DKListReplaceRangeWithCollectionMethod  replaceRangeWithCollection;
    
    DKListSortMethod                        sort;
    DKListShuffleMethod                     shuffle;
};

typedef const struct DKListInterface * DKListInterfaceRef;


DKClassRef  DKListClass( void );
void        DKSetDefaultListClass( DKClassRef _self );

DKClassRef  DKMutableListClass( void );
void        DKSetDefaultMutableListClass( DKClassRef _self );

#define     DKListCreateEmpty()    DKCreate( DKListClass() )
#define     DKListCreateMutable()  DKCreate( DKMutableListClass() )

#define     DKListCreateWithObject( cls, object )           DKListInitWithObject( DKAlloc( cls, 0 ), object )
#define     DKListCreateWithObjects( cls, ... )             DKListInitWithObjects( DKAlloc( cls, 0 ), __VA_ARGS__ )
#define     DKListCreateWithVAObjects( cls, objects )       DKListInitWithVAObjects( DKAlloc( cls, 0 ), objects )
#define     DKListCreateWithCArray( cls, objects, count )   DKListInitWithCArray( DKAlloc( cls, 0 ), objects, count )
#define     DKListCreateWithCollection( cls, collection )   DKListInitWithCollection( DKAlloc( cls, 0 ), collection )

#define     DKListCreateSetWithObjects( cls, ... )              DKListInitSetWithObjects( DKAlloc( cls, 0 ), __VA_ARGS__ )
#define     DKListCreateSetWithVAObjects( cls, objects )        DKListInitSetWithVAObjects( DKAlloc( cls, 0 ), objects )
#define     DKListCreateSetWithCArray( cls, objects, count )    DKListInitSetWithCArray( DKAlloc( cls, 0 ), objects, count )
#define     DKListCreateSetWithCollection( cls, collection )    DKListInitSetWithCollection( DKAlloc( cls, 0 ), collection )

DKObjectRef DKListInitWithObject( DKListRef _self, DKObjectRef object );
DKObjectRef DKListInitWithObjects( DKListRef _self, ... );
DKObjectRef DKListInitWithVAObjects( DKListRef _self, va_list objects );
DKObjectRef DKListInitWithCArray( DKListRef _self, DKObjectRef objects[], DKIndex count );
DKObjectRef DKListInitWithCollection( DKListRef _self, DKObjectRef srcCollection );

DKObjectRef DKListInitSetWithObjects( DKListRef _self, ... );
DKObjectRef DKListInitSetWithVAObjects( DKListRef _self, va_list objects );
DKObjectRef DKListInitSetWithCArray( DKListRef _self, DKObjectRef objects[], DKIndex count );
DKObjectRef DKListInitSetWithCollection( DKListRef _self, DKObjectRef srcCollection );

DKIndex     DKListGetCount( DKListRef _self );
DKIndex     DKListGetCountOfObject( DKListRef _self, DKObjectRef object );

DKIndex     DKListGetFirstIndexOfObject( DKListRef _self, DKObjectRef object );
DKIndex     DKListGetFirstIndexOfObjectInRange( DKListRef _self, DKObjectRef object, DKRange range );

DKIndex     DKListGetLastIndexOfObject( DKListRef _self, DKObjectRef object );
DKIndex     DKListGetLastIndexOfObjectInRange( DKListRef _self, DKObjectRef object, DKRange range );

DKObjectRef DKListGetObjectAtIndex( DKListRef _self, DKIndex index );
DKIndex     DKListGetObjectsInRange( DKListRef _self, DKRange range, DKObjectRef objects[] );

void        DKListAppendObject( DKMutableListRef _self, DKObjectRef object );
void        DKListAppendCArray( DKMutableListRef _self, DKObjectRef objects[], DKIndex count );
void        DKListAppendCollection( DKMutableListRef _self, DKListRef srcList );

void        DKListSetObjectAtIndex( DKMutableListRef _self, DKIndex index, DKObjectRef object );
void        DKListInsertObjectAtIndex( DKMutableListRef _self, DKIndex index, DKObjectRef object );

void        DKListReplaceRangeWithCArray( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
void        DKListReplaceRangeWithCollection( DKMutableListRef _self, DKRange range, DKObjectRef srcCollection );

bool        DKListContainsObject( DKListRef _self, DKObjectRef object );
DKObjectRef DKListGetMemberOfSet( DKListRef _self, DKObjectRef object );
void        DKListAddObjectToSet( DKMutableListRef _self, DKObjectRef object );

void        DKListRemoveObject( DKMutableListRef _self, DKObjectRef object );
void        DKListRemoveObjectAtIndex( DKMutableListRef _self, DKIndex index );
void        DKListRemoveObjectsInRange( DKMutableListRef _self, DKRange range );
void        DKListRemoveAllObjects( DKMutableListRef _self );

void        DKListSort( DKMutableListRef _self, DKCompareFunction cmp );
void        DKListShuffle( DKMutableListRef _self );



#endif // _DK_LIST_H_
