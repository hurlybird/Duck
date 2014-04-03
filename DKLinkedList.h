//
//  DKLinkedList.h
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_LINKED_LIST_H_
#define _DK_LINKED_LIST_H_

#include "DKList.h"


typedef const struct DKLinkedList * DKLinkedListRef;
typedef struct DKLinkedList * DKMutableLinkedListRef;


DKClassRef DKLinkedListClass( void );
DKClassRef DKMutableLinkedListClass( void );

DKLinkedListRef DKLinkedListCreate( void );
DKLinkedListRef DKLinkedListCreateWithObjects( DKObjectRef firstObject, ... );
DKLinkedListRef DKLinkedListCreateWithCArray( DKObjectRef objects[], DKIndex count );
DKLinkedListRef DKLinkedListCreateCopy( DKListRef srcList );

DKMutableLinkedListRef DKLinkedListCreateMutable( void );
DKMutableLinkedListRef DKLinkedListCreateMutableCopy( DKListRef srcList );

DKIndex   DKLinkedListGetCount( DKLinkedListRef ref );
DKIndex   DKLinkedListGetObjects( DKLinkedListRef ref, DKRange range, DKObjectRef objects[] );

void      DKLinkedListReplaceObjects( DKMutableLinkedListRef ref, DKRange range, DKObjectRef objects[], DKIndex count );
void      DKLinkedListReplaceObjectsWithList( DKMutableLinkedListRef ref, DKRange range, DKListRef srcList );
void      DKLinkedListSort( DKMutableLinkedListRef ref, DKCompareFunction cmp );
void      DKLinkedListShuffle( DKMutableLinkedListRef ref );



#endif // _DK_LINKED_LIST_H_










