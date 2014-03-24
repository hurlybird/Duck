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


DKDeclareInterface( List );


typedef const void * DKListRef;
typedef void * DKMutableListRef;

typedef void (*DKListApplierFunction)( DKTypeRef object, void * context );

struct DKList
{
    DKInterface _interface;

    DKIndex     (*getCount)( DKListRef ref );
    DKIndex     (*getObjects)( DKListRef ref, DKRange range, DKTypeRef objects[] );
    void        (*replaceObjects)( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count );
    void        (*replaceObjectsWithList)( DKMutableListRef ref, DKRange range, DKListRef srcList );
};

typedef const struct DKList DKList;


DKTypeRef   DKListClass( void );
void        DKSetListClass( DKTypeRef ref );

DKIndex     DKListGetCount( DKListRef ref );
DKIndex     DKListGetCountOfObject( DKListRef ref, DKTypeRef object );
DKIndex     DKListGetFirstIndexOfObject( DKListRef ref, DKTypeRef object );
DKIndex     DKListGetLastIndexOfObject( DKListRef ref, DKTypeRef object );

const void * DKListGetObjectAtIndex( DKListRef ref, DKIndex index );
DKIndex     DKListGetObjects( DKListRef ref, DKRange range, DKTypeRef objects[] );

void        DKListApplyFunction( DKListRef ref, DKListApplierFunction callback, void * context );

void        DKListAppendObject( DKMutableListRef ref, DKTypeRef object );
void        DKListAppendList( DKMutableListRef ref, DKTypeRef srcList );
void        DKListSetObjectAtIndex( DKMutableListRef ref, DKIndex index, DKTypeRef object );
void        DKListInsertObjectAtIndex( DKMutableListRef ref, DKIndex index, DKTypeRef object );
void        DKListReplaceObjects( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count );
void        DKListReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList );
void        DKListRemoveObjectAtIndex( DKMutableListRef ref, DKIndex index );
void        DKListRemoveAllObjects( DKMutableListRef ref );



#endif // _DK_LIST_H_
