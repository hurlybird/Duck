//
//  DKList.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-09.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_LIST_H_
#define _DK_LIST_H_

#include "DKRuntime.h"


DKDeclareInterfaceSelector( List );


//typedef const void * DKListRef; -- Declared in DKPlatform.h
typedef void * DKMutableListRef;

typedef DKIndex (*DKListGetCountMethod)( DKListRef ref );
typedef DKIndex (*DKListGetObjectsMethod)( DKListRef ref, DKRange range, DKObjectRef objects[] );
typedef void    (*DKListReplaceObjectsMethod)( DKMutableListRef ref, DKRange range, DKObjectRef objects[], DKIndex count );
typedef void    (*DKListReplaceObjectsWithListMethod)( DKMutableListRef ref, DKRange range, DKListRef srcList );
typedef void    (*DKListSortMethod)( DKMutableListRef ref, DKCompareFunction cmp );
typedef void    (*DKListShuffleMethod)( DKMutableListRef ref );

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
void        DKSetListClass( DKClassRef ref );

DKIndex     DKListGetCount( DKListRef ref );
DKIndex     DKListGetCountOfObject( DKListRef ref, DKObjectRef object );
DKIndex     DKListGetFirstIndexOfObject( DKListRef ref, DKObjectRef object );
DKIndex     DKListGetLastIndexOfObject( DKListRef ref, DKObjectRef object );

const void * DKListGetObjectAtIndex( DKListRef ref, DKIndex index );
DKIndex     DKListGetObjects( DKListRef ref, DKRange range, DKObjectRef objects[] );

int         DKListApplyFunction( DKListRef ref, DKListApplierFunction callback, void * context );

void        DKListAppendObject( DKMutableListRef ref, DKObjectRef object );
void        DKListAppendList( DKMutableListRef ref, DKListRef srcList );
void        DKListSetObjectAtIndex( DKMutableListRef ref, DKIndex index, DKObjectRef object );
void        DKListInsertObjectAtIndex( DKMutableListRef ref, DKIndex index, DKObjectRef object );
void        DKListReplaceObjects( DKMutableListRef ref, DKRange range, DKObjectRef objects[], DKIndex count );
void        DKListReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList );
void        DKListRemoveObjectAtIndex( DKMutableListRef ref, DKIndex index );
void        DKListRemoveAllObjects( DKMutableListRef ref );

void        DKListSort( DKMutableListRef ref, DKCompareFunction cmp );
void        DKListShuffle( DKMutableListRef ref );



#endif // _DK_LIST_H_
