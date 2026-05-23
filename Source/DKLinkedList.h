// =======================================================================================
//
// DKLinkedList.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_LINKED_LIST_H_
#define _DK_LINKED_LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKLinkedList * DKLinkedListRef;
typedef struct DKLinkedList * DKMutableLinkedListRef;


DK_API DKClassRef  DKLinkedListClass( void );
DK_API DKClassRef  DKMutableLinkedListClass( void );

#define     DKEmptyLinkedList()         DKAutorelease( DKNew( DKLinkedListClass() ) )
#define     DKMutableLinkedList()       DKAutorelease( DKNew( DKMutableLinkedListClass() ) )

#define     DKLinkedListWithCArray( objects, count )  DKLinkedListInitWithObjects( DKAlloc( DKLinkedListClass() ), objects, count )
#define     DKLinkedListWithCollection( collection )  DKLinkedListInitWithCollection( DKAlloc( DKLinkedListClass() ), collection )

#define     DKNewMutableLinkedList()    DKNew( DKMutableLinkedListClass() )

DK_API DKObjectRef DKLinkedListInitWithVAObjects( DKLinkedListRef _self, va_list objects );
DK_API DKObjectRef DKLinkedListInitWithCArray( DKLinkedListRef _self, DKObjectRef objects[], DKIndex count );
DK_API DKObjectRef DKLinkedListInitWithCollection( DKLinkedListRef _self, DKObjectRef srcCollection );

DK_API DKLinkedListRef DKLinkedListCopy( DKLinkedListRef _self );
DK_API DKMutableLinkedListRef DKLinkedListMutableCopy( DKLinkedListRef _self );

DK_API DKIndex     DKLinkedListGetCount( DKLinkedListRef _self );

DK_API DKObjectRef DKLinkedListGetObjectAtIndex( DKLinkedListRef _self, DKIndex index );
DK_API DKIndex     DKLinkedListGetObjectsInRange( DKLinkedListRef _self, DKRange range, DKObjectRef objects[] );

DK_API void        DKLinkedListAppendCArray( DKMutableLinkedListRef _self, DKObjectRef objects[], DKIndex count );
DK_API void        DKLinkedListAppendCollection( DKMutableLinkedListRef _self, DKObjectRef srcCollection );

DK_API void        DKLinkedListReplaceRangeWithCArray( DKMutableLinkedListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
DK_API void        DKLinkedListReplaceRangeWithCollection( DKMutableLinkedListRef _self, DKRange range, DKObjectRef srcCollection );

DK_API void        DKLinkedListSort( DKMutableLinkedListRef _self, DKCompareFunction cmp );
DK_API void        DKLinkedListReverse( DKMutableLinkedListRef _self );
DK_API void        DKLinkedListShuffle( DKMutableLinkedListRef _self );

DK_API int         DKLinkedListApplyFunction( DKLinkedListRef _self, DKApplierFunction callback, void * context );

DK_API bool        DKLinkedListInsertObjectWithPriority( DKMutableLinkedListRef _self, DKObjectRef object, double priority, DKInsertPolicy policy );
DK_API double      DKLinkedListGetPriorityOfObjectAtIndex( DKLinkedListRef _self, DKIndex index );


#ifdef __cplusplus
}
#endif

#endif // _DK_LINKED_LIST_H_





