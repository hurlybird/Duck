//
//  DKLinkedList.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "DKLinkedList.h"
#include "DKNodePool.h"
#include "DKCopying.h"
#include "DKPointerArray.h"
#include "DKString.h"


struct DKLinkedListNode
{
    struct DKLinkedListNode * prev;
    struct DKLinkedListNode * next;
    DKObjectRef object;
};

struct DKLinkedList
{
    DKObjectHeader _obj;

    DKNodePool nodePool;

    struct DKLinkedListNode * first;
    struct DKLinkedListNode * last;
    DKIndex count;

    struct
    {
        struct DKLinkedListNode * node;
        DKIndex index;
        
    } cursor;
};


static DKObjectRef DKLinkedListInitialize( DKObjectRef _self );
static void      DKLinkedListFinalize( DKObjectRef _self );

static void      DKImmutableLinkedListReplaceObjects( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
static void      DKImmutableLinkedListReplaceObjectsWithList( DKMutableListRef _self, DKRange range, DKListRef srcList );
static void      DKImmutableLinkedListSort( DKMutableListRef _self, DKCompareFunction cmp );
static void      DKImmutableLinkedListShuffle( DKMutableListRef _self );

static void ReplaceObjects( struct DKLinkedList * list, DKRange range, DKObjectRef objects[], DKIndex count );



///
//  DKLinkedListClass()
//
DKThreadSafeClassInit( DKLinkedListClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKLinkedList" ), DKObjectClass(), sizeof(struct DKLinkedList), 0 );
    
    // Allocation
    struct DKAllocation * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(DKAllocation) );
    allocation->initialize = DKLinkedListInitialize;
    allocation->finalize = DKLinkedListFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKLinkedListCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // List
    struct DKList * list = DKAllocInterface( DKSelector(List), sizeof(DKList) );
    list->getCount = (DKListGetCountMethod)DKLinkedListGetCount;
    list->getObjects = (DKListGetObjectsMethod)DKLinkedListGetObjects;
    list->replaceObjects = DKImmutableLinkedListReplaceObjects;
    list->replaceObjectsWithList = DKImmutableLinkedListReplaceObjectsWithList;
    list->sort = DKImmutableLinkedListSort;
    list->shuffle = DKImmutableLinkedListShuffle;

    DKInstallInterface( cls, list );
    DKRelease( list );
    
    return cls;
}


///
//  DKMutableLinkedListClass()
//
DKThreadSafeClassInit( DKMutableLinkedListClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKMutableLinkedList" ), DKLinkedListClass(), sizeof(struct DKLinkedList), 0 );
    
    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = (DKCopyMethod)DKLinkedListCreateCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKLinkedListCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // List
    struct DKList * list = DKAllocInterface( DKSelector(List), sizeof(DKList) );
    list->getCount = (DKListGetCountMethod)DKLinkedListGetCount;
    list->getObjects = (DKListGetObjectsMethod)DKLinkedListGetObjects;
    list->replaceObjects = (DKListReplaceObjectsMethod)DKLinkedListReplaceObjects;
    list->replaceObjectsWithList = (DKListReplaceObjectsWithListMethod)DKLinkedListReplaceObjectsWithList;
    list->sort = (DKListSortMethod)DKLinkedListSort;
    list->shuffle = (DKListShuffleMethod)DKLinkedListShuffle;

    DKInstallInterface( cls, list );
    DKRelease( list );
    
    return cls;
}


///
//  DKLinkedListInitialize()
//
static DKObjectRef DKLinkedListInitialize( DKObjectRef _self )
{
    struct DKLinkedList * list = (struct DKLinkedList *)_self;
    
    DKNodePoolInit( &list->nodePool, sizeof(struct DKLinkedListNode), 0 );

    list->first = NULL;
    list->last = NULL;
    list->count = 0;
    
    list->cursor.node = NULL;
    list->cursor.index = 0;
    
    return _self;
}


///
//  DKLinkedListFinalize()
//
static void DKLinkedListFinalize( DKObjectRef _self )
{
    struct DKLinkedList * list = (struct DKLinkedList *)_self;

    ReplaceObjects( list, DKRangeMake( 0, list->count ), NULL, 0 );
    DKNodePoolFinalize( &list->nodePool );
}




// Internals =============================================================================

///
//  CheckListIntegrity()
//
#if DK_RUNTIME_INTEGRITY_CHECKS
static void CheckListIntegrity( struct DKLinkedList * list )
{
    DKIndex count = 0;
    
    struct DKLinkedListNode * node = list->first;
    
    if( node == NULL )
    {
        DKAssert( list->last == NULL );
        DKAssert( list->cursor.node == NULL );
        DKAssert( list->cursor.index == 0 );
    }
    
    while( node )
    {
        DKAssert( count < list->count );

        if( node->prev )
        {
            DKAssert( node->prev->next == node );
        }
        
        if( node->next )
        {
            DKAssert( node->next->prev == node );
        }
        
        if( list->cursor.node == node )
        {
            DKAssert( list->cursor.index == count );
        }
        
        node = node->next;
    
        count++;
    }
    
    DKAssert( count == list->count );
}
#else
#define CheckListIntegrity( list )
#endif


///
//  AllocNode()
//
static struct DKLinkedListNode * AllocNode( struct DKLinkedList * list, DKObjectRef object )
{
    struct DKLinkedListNode * node = DKNodePoolAlloc( &list->nodePool );

    node->prev = NULL;
    node->next = NULL;
    
    node->object = DKRetain( object );
    
    list->count++;
    
    return node;
}


///
//  FreeNode()
//
static void FreeNode( struct DKLinkedList * list, struct DKLinkedListNode * node )
{
    DKAssert( list->count > 0 );
    list->count--;

    DKRelease( node->object );
    
    DKNodePoolFree( &list->nodePool, node );
}


///
//  MoveCursor()
//
static struct DKLinkedListNode * MoveCursor( struct DKLinkedList * list, DKIndex index )
{
    if( list->first == NULL )
    {
        CheckListIntegrity( list );
        return NULL;
    }

    if( index > list->cursor.index )
    {
        DKIndex distFromCursor = index - list->cursor.index;
        DKIndex distFromBack = list->count - index - 1;
        
        if( distFromCursor <= distFromBack )
        {
            for( ; distFromCursor > 0; --distFromCursor )
                list->cursor.node = list->cursor.node->next;
        }
        
        else
        {
            list->cursor.node = list->last;
            
            for( ; distFromBack > 0; --distFromBack )
                list->cursor.node = list->cursor.node->prev;
        }
    }

    if( index < list->cursor.index )
    {
        DKIndex distFromCursor = list->cursor.index - index;
        DKIndex distFromFront = index;
    
        if( distFromCursor <= distFromFront )
        {
            for( ; distFromCursor > 0; --distFromCursor )
                list->cursor.node = list->cursor.node->prev;
        }
        
        else
        {
            list->cursor.node = list->first;
            
            for( ; distFromFront > 0; --distFromFront )
                list->cursor.node = list->cursor.node->next;
        }
    }
    
    list->cursor.index = index;

    CheckListIntegrity( list );
    
    return list->cursor.node;
}


///
//  RemoveObjects()
//
static void RemoveObjects( struct DKLinkedList * list, DKRange range )
{
    DKAssert( range.location >= 0 );
    DKAssert( range.length >= 0 );
    DKAssert( DKRangeEnd( range ) <= list->count );

    if( range.length == 0 )
        return;
    
    MoveCursor( list, range.location );

    for( DKIndex i = 0; i < range.length; ++i )
    {
        struct DKLinkedListNode * node = list->cursor.node;

        DKAssert( node != NULL );

        struct DKLinkedListNode * next = node->next;

        if( node->prev )
            node->prev->next = node->next;
        
        if( node->next )
            node->next->prev = node->prev;

        if( list->first == node )
            list->first = node->next;
        
        if( list->last == node )
            list->last = node->prev;

        FreeNode( list, node );
        
        list->cursor.node = next;
    }
    
    if( list->cursor.node == NULL )
    {
        list->cursor.node = list->last;
        list->cursor.index = list->count;
    }

    CheckListIntegrity( list );
}


///
//  InsertObject()
//
static void InsertObject( struct DKLinkedList * list, DKIndex index, DKObjectRef object )
{
    DKAssert( index >= 0 );
    DKAssert( index <= list->count );

    if( list->first == NULL )
    {
        DKAssert( index == 0 );

        struct DKLinkedListNode * node = AllocNode( list, object );

        list->first = node;
        list->last = node;
        
        list->cursor.node = node;
        list->cursor.index = 0;
    }
    
    else if( index == 0 )
    {
        struct DKLinkedListNode * node = AllocNode( list, object );
        
        node->next = list->first;
        list->first->prev = node;
        list->first = node;
        
        list->cursor.node = node;
        list->cursor.index = 0;
    }
    
    else if( index == list->count )
    {
        struct DKLinkedListNode * node = AllocNode( list, object );
        
        node->prev = list->last;
        list->last->next = node;
        list->last = node;
        
        list->cursor.node = node;
        list->cursor.index = list->count - 1;
    }
    
    else
    {
        struct DKLinkedListNode * next = MoveCursor( list, index );
        struct DKLinkedListNode * node = AllocNode( list, object );
        
        DKAssert( next != NULL );
        
        node->next = next;
        node->prev = next->prev;
        node->prev->next = node;
        node->next->prev = node;
        
        list->cursor.node = node;
    }

    CheckListIntegrity( list );
}


///
//  ReplaceObjects()
//
static void ReplaceObjects( struct DKLinkedList * list, DKRange range, DKObjectRef objects[], DKIndex count )
{
    DKCheckRange( range, list->count );

    RemoveObjects( list, range );
    
    for( DKIndex i = 0; i < count; i++ )
        InsertObject( list, range.location + i, objects[i] );
}


///
//  ReplaceObjectsWithList()
//
static void ReplaceObjectsWithList( struct DKLinkedList * list, DKRange range, DKListRef srcList )
{
    if( srcList )
    {
        DKCheckRange( range, list->count );

        DKList * srcListInterface = DKGetInterface( srcList, DKSelector(List) );
        
        RemoveObjects( list, range );
        
        DKIndex count = srcListInterface->getCount( srcList );
        
        for( DKIndex i = 0; i < count; ++i )
        {
            DKObjectRef object;
            
            srcListInterface->getObjects( srcList, DKRangeMake( i, 1 ), &object );

            InsertObject( list, range.location + i, object );
        }
    }
    
    else
    {
        ReplaceObjects( list, range, NULL, 0 );
    }
}



// DKLinkedList Interface ================================================================

DKLinkedListRef DKLinkedListCreate( void )
{
    return DKAllocObject( DKLinkedListClass(), 0 );
}


///
//  DKLinkedListCreateWithObjects()
//
DKLinkedListRef DKLinkedListCreateWithObjects( DKObjectRef firstObject, ... )
{
    struct DKLinkedList * list = DKAllocObject( DKLinkedListClass(), 0 );

    va_list arg_ptr;
    va_start( arg_ptr, firstObject );

    for( DKObjectRef object = firstObject; object != NULL; )
    {
        ReplaceObjects( list, DKRangeMake( list->count, 0 ), &object, 1 );
        
        object = va_arg( arg_ptr, DKObjectRef );
    }

    va_end( arg_ptr );

    CheckListIntegrity( list );
    
    return list;
}


///
//  DKLinkedListCreateWithCArray()
//
DKLinkedListRef DKLinkedListCreateWithCArray( DKObjectRef objects[], DKIndex count )
{
    struct DKLinkedList * list = DKAllocObject( DKLinkedListClass(), 0 );

    ReplaceObjects( list, DKRangeMake( 0, 0 ), objects, count );

    return list;
}


///
//  DKLinkedListCreateCopy()
//
DKLinkedListRef DKLinkedListCreateCopy( DKListRef srcList )
{
    struct DKLinkedList * list = DKAllocObject( DKLinkedListClass(), 0 );
    
    ReplaceObjectsWithList( list, DKRangeMake( 0, 0 ), srcList );
    
    return list;
}


///
//  DKLinkedListCreateMutable()
//
DKMutableLinkedListRef DKLinkedListCreateMutable( void )
{
    return DKAllocObject( DKMutableLinkedListClass(), 0 );
}


///
//  DKLinkedListCreateMutableCopy()
//
DKMutableLinkedListRef DKLinkedListCreateMutableCopy( DKListRef srcList )
{
    struct DKLinkedList * list = DKAllocObject( DKMutableLinkedListClass(), 0 );
    
    ReplaceObjectsWithList( list, DKRangeMake( 0, 0 ), srcList );
    
    return list;
}


///
//  DKLinkedListGetCount()
//
DKIndex DKLinkedListGetCount( DKLinkedListRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKLinkedListClass() );
        return _self->count;
    }
    
    return 0;
}


///
//  DKLinkedListGetObjects()
//
DKIndex DKLinkedListGetObjects( DKLinkedListRef _self, DKRange range, DKObjectRef objects[] )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKLinkedListClass() );
        DKCheckRange( range, _self->count, 0 );

        struct DKLinkedList * list = (struct DKLinkedList *)_self;
        
        for( DKIndex i = 0; i < range.length; ++i )
        {
            struct DKLinkedListNode * node = MoveCursor( list, range.location + i );
            objects[i] = node->object;
        }
    }
    
    return 0;
}


///
//  DKLinkedListReplaceObjects()
//
static void DKImmutableLinkedListReplaceObjects( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    DKError( "DKLinkedListReplaceObjects: Trying to modify an immutable object." );
}

void DKLinkedListReplaceObjects( DKMutableLinkedListRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableLinkedListClass() );
        ReplaceObjects( _self, range, objects, count );
    }
}


///
//  DKLinkedListReplaceObjectsWithList()
//
static void DKImmutableLinkedListReplaceObjectsWithList( DKMutableListRef _self, DKRange range, DKListRef srcList )
{
    DKError( "DKLinkedListReplaceObjectsWithList: Trying to modify an immutable object." );
}

void DKLinkedListReplaceObjectsWithList( DKMutableLinkedListRef _self, DKRange range, DKListRef srcList )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableLinkedListClass() );
        ReplaceObjectsWithList( _self, range, srcList );
    }
}


///
//  DKLinkedListSort()
//
static void ListToArray( DKPointerArray * array, struct DKLinkedList * list )
{
    struct DKLinkedListNode * node = list->first;
    
    for( DKIndex i = 0; i < list->count; ++i )
    {
        array->data[i] = node->object;
        node = node->next;
    }
    
    array->length = list->count;
}

static void ArrayToList( struct DKLinkedList * list, DKPointerArray * array )
{
    struct DKLinkedListNode * node = list->first;
    
    for( DKIndex i = 0; i < list->count; ++i )
    {
        node->object = array->data[i];
        node = node->next;
    }
}

static void DKImmutableLinkedListSort( DKMutableListRef _self, DKCompareFunction cmp )
{
    DKError( "DKLinkedListSort: Trying to modify an immutable object." );
}

void DKLinkedListSort( DKMutableLinkedListRef _self, DKCompareFunction cmp )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableLinkedListClass() );

        // This is absurd, yet probably not much slower than doing all the pointer
        // gymnastics needed for sorting the list nodes.
        DKPointerArray array;
        DKPointerArrayInit( &array );
        DKPointerArrayReserve( &array, _self->count );
        ListToArray( &array, _self );

        DKPointerArraySort( &array, cmp );

        ArrayToList( _self, &array );
        DKPointerArrayFinalize( &array );
    }
}


///
//  DKLinkedListShuffle()
//
static void DKImmutableLinkedListShuffle( DKMutableListRef _self )
{
    DKError( "DKLinkedListShuffle: Trying to modify an immutable object." );
}

void DKLinkedListShuffle( DKMutableLinkedListRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableLinkedListClass() );
        
        // This is absurd, yet probably not much slower than doing all the pointer
        // gymnastics needed for shuffling the list nodes.
        DKPointerArray array;
        DKPointerArrayInit( &array );
        DKPointerArrayReserve( &array, _self->count );
        ListToArray( &array, _self );

        DKPointerArrayShuffle( &array );

        ArrayToList( _self, &array );
        DKPointerArrayFinalize( &array );
    }
}





