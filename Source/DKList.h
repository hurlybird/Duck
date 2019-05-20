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


DK_API DKDeclareInterfaceSelector( List );


//typedef struct DKList * DKListRef; -- Declared in DKPlatform.h
typedef struct DKList * DKMutableListRef;

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
typedef void        (*DKListReorderMethod)( DKMutableListRef _self );

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
    DKListReorderMethod                     reverse;
    DKListReorderMethod                     shuffle;
};

typedef const struct DKListInterface * DKListInterfaceRef;


DK_API DKClassRef  DKListClass( void );
DK_API void        DKSetDefaultListClass( DKClassRef _self );

DK_API DKClassRef  DKMutableListClass( void );
DK_API void        DKSetDefaultMutableListClass( DKClassRef _self );

#define            DKEmptyList()           DKAutorelease( DKNew( DKListClass() ) )
#define            DKMutableList()         DKAutorelease( DKNew( DKMutableListClass() ) )

#define            DKListWithObject( object )          DKAutorelease( DKListInitWithObject( DKAlloc( DKListClass() ), object ) )
#define            DKListWithObjects( ... )            DKAutorelease( DKListInitWithObjects( DKAlloc( DKListClass() ), __VA_ARGS__, NULL ) )
#define            DKListWithVAObjects( objects )      DKAutorelease( DKListInitWithVAObjects( DKAlloc( DKListClass() ), objects ) )
#define            DKListWithCArray( objects, count )  DKAutorelease( DKListInitWithCArray( DKAlloc( DKListClass() ), objects, count ) )
#define            DKListWithCollection( collection )  DKAutorelease( DKListInitWithCollection( DKAlloc( DKListClass() ), collection ) )

#define            DKNewMutableList()      DKNew( DKMutableListClass() )

DK_API DKObjectRef DKListInitWithObject( DKListRef _self, DKObjectRef object );
DK_API DKObjectRef DKListInitWithObjects( DKListRef _self, ... );
DK_API DKObjectRef DKListInitWithVAObjects( DKListRef _self, va_list objects );
DK_API DKObjectRef DKListInitWithCArray( DKListRef _self, DKObjectRef objects[], DKIndex count );
DK_API DKObjectRef DKListInitWithCollection( DKListRef _self, DKObjectRef srcCollection );

DK_API DKObjectRef DKListInitSetWithObjects( DKListRef _self, ... );
DK_API DKObjectRef DKListInitSetWithVAObjects( DKListRef _self, va_list objects );
DK_API DKObjectRef DKListInitSetWithCArray( DKListRef _self, DKObjectRef objects[], DKIndex count );
DK_API DKObjectRef DKListInitSetWithCollection( DKListRef _self, DKObjectRef srcCollection );

DK_API DKIndex     DKListGetCount( DKListRef _self );
DK_API DKIndex     DKListGetCountOfObject( DKListRef _self, DKObjectRef object );

#define            DKListGetIndexOfObject( list, obj )     DKListGetFirstIndexOfObject( list, obj )
DK_API DKIndex     DKListGetFirstIndexOfObject( DKListRef _self, DKObjectRef object );
DK_API DKIndex     DKListGetFirstIndexOfObjectInRange( DKListRef _self, DKObjectRef object, DKRange range );

DK_API DKIndex     DKListGetLastIndexOfObject( DKListRef _self, DKObjectRef object );
DK_API DKIndex     DKListGetLastIndexOfObjectInRange( DKListRef _self, DKObjectRef object, DKRange range );

DK_API DKObjectRef DKListGetObjectAtIndex( DKListRef _self, DKIndex index );
DK_API DKIndex     DKListGetObjectsInRange( DKListRef _self, DKRange range, DKObjectRef objects[] );

DK_API DKObjectRef DKListGetFirstObject( DKListRef _self );
DK_API DKObjectRef DKListGetLastObject( DKListRef _self );

DK_API void        DKListAppendObject( DKMutableListRef _self, DKObjectRef object );
DK_API void        DKListAppendCArray( DKMutableListRef _self, DKObjectRef objects[], DKIndex count );
DK_API void        DKListAppendCollection( DKMutableListRef _self, DKObjectRef srcCollection );

DK_API void        DKListSetObjectAtIndex( DKMutableListRef _self, DKObjectRef object, DKIndex index );
DK_API void        DKListInsertObjectAtIndex( DKMutableListRef _self, DKObjectRef object, DKIndex index );

DK_API void        DKListReplaceRangeWithCArray( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
DK_API void        DKListReplaceRangeWithCollection( DKMutableListRef _self, DKRange range, DKObjectRef srcCollection );

DK_API bool        DKListContainsObject( DKListRef _self, DKObjectRef object );
DK_API DKObjectRef DKListGetMemberOfSet( DKListRef _self, DKObjectRef object );
DK_API void        DKListAddObjectToSet( DKMutableListRef _self, DKObjectRef object );

DK_API void        DKListRemoveObject( DKMutableListRef _self, DKObjectRef object );
DK_API void        DKListRemoveObjectAtIndex( DKMutableListRef _self, DKIndex index );
DK_API void        DKListRemoveObjectsInRange( DKMutableListRef _self, DKRange range );
DK_API void        DKListRemoveAllObjects( DKMutableListRef _self );

DK_API void        DKListRemoveFirstObject( DKMutableListRef _self );
DK_API void        DKListRemoveLastObject( DKMutableListRef _self );

DK_API bool        DKListEqual( DKListRef _self, DKListRef other );
DK_API int         DKListCompare( DKListRef _self, DKListRef other );

DK_API void        DKListSort( DKMutableListRef _self, DKCompareFunction cmp );
DK_API void        DKListReverse( DKMutableListRef _self );
DK_API void        DKListShuffle( DKMutableListRef _self );



#endif // _DK_LIST_H_
