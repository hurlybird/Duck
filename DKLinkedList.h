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


DKTypeRef DKLinkedListClass( void );
DKTypeRef DKMutableLinkedListClass( void );

DKListRef DKLinkedListCreate( DKTypeRef objects[], DKIndex count );
DKListRef DKLinkedListCreateCopy( DKListRef srcList );

DKMutableListRef DKLinkedListCreateMutable( void );
DKMutableListRef DKLinkedListCreateMutableCopy( DKListRef srcList );

DKIndex DKLinkedListGetCount( DKListRef ref );
DKIndex DKLinkedListGetObjects( DKListRef ref, DKRange range, DKTypeRef objects[] );
void    DKLinkedListReplaceObjects( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count );
void    DKLinkedListReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList );



#endif // _DK_LINKED_LIST_H_










