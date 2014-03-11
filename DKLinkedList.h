//
//  DKLinkedList.h
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_LINKED_LIST_H_
#define _DK_LINKED_LIST_H_

#include "DKObject.h"
#include "DKList.h"
#include "DKMemory.h"


DKDeclareSUID( DKLinkedListTypeID );
DKDeclareSUID( DKMutableLinkedListTypeID );


DKTypeRef DKLinkedListClass( void );
DKTypeRef DKMutableLinkedListClass( void );

DKListRef DKLinkedListCreate( const void ** values, DKIndex count, const DKListCallbacks * callbacks );
DKListRef DKLinkedListCreateCopy( DKListRef srcList );

DKMutableListRef DKLinkedListCreateMutable( const DKListCallbacks * callbacks );
DKMutableListRef DKLinkedListCreateMutableCopy( DKListRef srcList );

const DKListCallbacks * DKLinkedListGetCallbacks( DKListRef ref );

DKIndex DKLinkedListGetCount( DKListRef ref );
DKIndex DKLinkedListGetValues( DKListRef ref, DKRange range, const void ** values );
void    DKLinkedListReplaceValues( DKMutableListRef ref, DKRange range, const void ** values, DKIndex count );
void    DKLinkedListReplaceValuesWithList( DKMutableListRef ref, DKRange range, DKListRef srcList );


#endif // _DK_LINKED_LIST_H_










