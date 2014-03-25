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


#define DK_LINKED_LIST_ERROR_CHECKS 0


struct DKLinkedListNode
{
    struct DKLinkedListNode * prev;
    struct DKLinkedListNode * next;
    DKTypeRef object;
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


static DKTypeRef    DKLinkedListInitialize( DKTypeRef ref );
static DKTypeRef    DKMutableLinkedListInitialize( DKTypeRef ref );
static void         DKLinkedListFinalize( DKTypeRef ref );

static void ReplaceObjects( struct DKLinkedList * list, DKRange range, DKTypeRef objects[], DKIndex count );
static void ReplaceObjectsWithList( struct DKLinkedList * list, DKRange range, DKListRef srcList );


///
//  DKLinkedListClass()
//
DKTypeRef DKLinkedListClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( DKObjectClass(), sizeof(struct DKLinkedList) );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->initialize = DKLinkedListInitialize;
        lifeCycle->finalize = DKLinkedListFinalize;

        DKInstallInterface( SharedClassObject, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKRetain;
        copying->mutableCopy = DKLinkedListCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );

        // List
        struct DKList * list = (struct DKList *)DKCreateInterface( DKSelector(List), sizeof(DKList) );
        list->getCount = DKLinkedListGetCount;
        list->getObjects = DKLinkedListGetObjects;
        list->replaceObjects = DKLinkedListReplaceObjects;
        list->replaceObjectsWithList = DKLinkedListReplaceObjectsWithList;
        list->sort = DKLinkedListSort;
        list->shuffle = DKLinkedListShuffle;

        DKInstallInterface( SharedClassObject, list );
        DKRelease( list );
    }
    
    return SharedClassObject;
}


///
//  DKMutableLinkedListClass()
//
DKTypeRef DKMutableLinkedListClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( DKLinkedListClass(), sizeof(struct DKLinkedList) );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->initialize = DKMutableLinkedListInitialize;
        lifeCycle->finalize = DKLinkedListFinalize;

        DKInstallInterface( SharedClassObject, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKLinkedListCreateMutableCopy;
        copying->mutableCopy = DKLinkedListCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );

        // List
        struct DKList * list = (struct DKList *)DKCreateInterface( DKSelector(List), sizeof(DKList) );
        list->getCount = DKLinkedListGetCount;
        list->getObjects = DKLinkedListGetObjects;
        list->replaceObjects = DKLinkedListReplaceObjects;
        list->replaceObjectsWithList = DKLinkedListReplaceObjectsWithList;
        list->sort = DKLinkedListSort;
        list->shuffle = DKLinkedListShuffle;

        DKInstallInterface( SharedClassObject, list );
        DKRelease( list );
    }
    
    return SharedClassObject;
}


///
//  DKLinkedListInitialize()
//
static DKTypeRef DKLinkedListInitialize( DKTypeRef ref )
{
    ref = DKObjectInitialize( ref );
    
    if( ref )
    {
        struct DKLinkedList * list = (struct DKLinkedList *)ref;
        
        DKNodePoolInit( &list->nodePool, sizeof(struct DKLinkedListNode), 0 );

        list->first = NULL;
        list->last = NULL;
        list->count = 0;
        
        list->cursor.node = NULL;
        list->cursor.index = 0;
    }
    
    return ref;
}


///
//  DKMutableLinkedListInitialize()
//
static DKTypeRef DKMutableLinkedListInitialize( DKTypeRef ref )
{
    ref = DKLinkedListInitialize( ref );
    
    if( ref )
    {
        DKSetObjectAttribute( ref, DKObjectIsMutable, 1 );
    }
    
    return ref;
}


///
//  DKLinkedListFinalize()
//
static void DKLinkedListFinalize( DKTypeRef ref )
{
    if( ref )
    {
        struct DKLinkedList * list = (struct DKLinkedList *)ref;

        ReplaceObjects( list, DKRangeMake( 0, list->count ), NULL, 0 );
        
        DKNodePoolClear( &list->nodePool );
    }
}




// Internals =============================================================================

///
//  CheckForErrors()
//
#if DK_LINKED_LIST_ERROR_CHECKS
static void CheckForErrors( struct DKLinkedList * list )
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
#define CheckForErrors( list )
#endif


///
//  AllocNode()
//
static struct DKLinkedListNode * AllocNode( struct DKLinkedList * list, DKTypeRef object )
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
        CheckForErrors( list );
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

    CheckForErrors( list );
    
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

    CheckForErrors( list );
}


///
//  InsertObject()
//
static void InsertObject( struct DKLinkedList * list, DKIndex index, DKTypeRef object )
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
        
        DKAssert( next );
        
        node->next = next;
        node->prev = next->prev;
        node->prev->next = node;
        node->next->prev = node;
        
        list->cursor.node = node;
    }

    CheckForErrors( list );
}


///
//  ReplaceObjects()
//
static void ReplaceObjects( struct DKLinkedList * list, DKRange range, DKTypeRef objects[], DKIndex count )
{
    RemoveObjects( list, range );
    
    for( DKIndex i = 0; i < count; i++ )
        InsertObject( list, range.location + i, objects[i] );
}


///
//  ReplaceObjectsWithList()
//
static void ReplaceObjectsWithList( struct DKLinkedList * list, DKRange range, DKListRef srcList )
{
    DKList * srcListInterface = DKLookupInterface( srcList, DKSelector(List) );
    
    if( !srcListInterface )
    {
        DKError( "DKLinkedListReplaceObjectsWithListInternal: Source object is not a list." );
        return;
    }

    RemoveObjects( list, range );
    
    DKIndex count = srcListInterface->getCount( srcList );
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKTypeRef object;
        
        srcListInterface->getObjects( srcList, DKRangeMake( i, 1 ), &object );

        InsertObject( list, range.location + i, object );
    }
}




// DKLinkedList Interface ================================================================

///
//  DKLinkedListCreate()
//
DKListRef DKLinkedListCreate( DKTypeRef objects[], DKIndex count )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKLinkedListClass() );
    
    ReplaceObjects( list, DKRangeMake( 0, 0 ), objects, count );
    
    return list;
}


///
//  DKLinkedListCreateCopy()
//
DKListRef DKLinkedListCreateCopy( DKListRef srcList )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKLinkedListClass() );
    
    ReplaceObjectsWithList( list, DKRangeMake( 0, 0 ), srcList );
    
    return list;
}


///
//  DKLinkedListCreateMutable()
//
DKMutableListRef DKLinkedListCreateMutable( void )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKMutableLinkedListClass() );
    
    return list;
}


///
//  DKLinkedListCreateMutableCopy()
//
DKMutableListRef DKLinkedListCreateMutableCopy( DKListRef srcList )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKMutableLinkedListClass() );
    
    ReplaceObjectsWithList( list, DKRangeMake( 0, 0 ), srcList );
    
    return list;
}


///
//  DKLinkedListGetCount()
//
DKIndex DKLinkedListGetCount( DKListRef ref )
{
    if( ref )
    {
        const struct DKLinkedList * list = ref;
        return list->count;
    }
    
    return 0;
}


///
//  DKLinkedListGetObjects()
//
DKIndex DKLinkedListGetObjects( DKListRef ref, DKRange range, DKTypeRef objects[] )
{
    if( ref )
    {
        struct DKLinkedList * list = (struct DKLinkedList *)ref;
        
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
void DKLinkedListReplaceObjects( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKLinkedListReplaceObjects: Trying to modify an immutable object." );
            return;
        }

        struct DKLinkedList * list = (struct DKLinkedList *)ref;
        ReplaceObjects( list, range, objects, count );
    }
}


///
//  DKLinkedListReplaceObjectsWithList()
//
void DKLinkedListReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKLinkedListReplaceObjectsWithList: Trying to modify an immutable object." );
            return;
        }

        struct DKLinkedList * list = (struct DKLinkedList *)ref;
        ReplaceObjectsWithList( list, range, srcList );
    }
}


///
//  DKLinkedListSort()
//
static void ListToBuffer( DKTypeRef buffer[], struct DKLinkedList * list )
{
    struct DKLinkedListNode * node = list->first;
    
    for( DKIndex i = 0; i < list->count; ++i )
    {
        buffer[i] = node->object;
        node = node->next;
    }
}

static void BufferToList( struct DKLinkedList * list, DKTypeRef buffer[] )
{
    struct DKLinkedListNode * node = list->first;
    
    for( DKIndex i = 0; i < list->count; ++i )
    {
        node->object = buffer[i];
        node = node->next;
    }
}

void DKLinkedListSort( DKMutableListRef ref, DKCompareFunction cmp )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKLinkedListSort: Trying to modify an immutable object." );
            return;
        }

        // This is absurd, yet probably not much slower than doing all the pointer
        // gymnastics needed for sorting the list nodes.
        struct DKLinkedList * list = (struct DKLinkedList *)ref;

        DKTypeRef * buffer = dk_malloc( sizeof(DKTypeRef) * list->count );
        ListToBuffer( buffer, list );

        qsort( buffer, list->count, sizeof(DKTypeRef), cmp );

        BufferToList( list, buffer );
        dk_free( buffer );
    }
}


///
//  DKLinkedListShuffle()
//
void DKLinkedListShuffle( DKMutableListRef ref )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKLinkedListShuffle: Trying to modify an immutable object." );
            return;
        }

        // This is absurd, yet probably not much slower than doing all the pointer
        // gymnastics needed for shuffling the list nodes.
        struct DKLinkedList * list = (struct DKLinkedList *)ref;

        DKTypeRef * buffer = dk_malloc( sizeof(DKTypeRef) * list->count );
        ListToBuffer( buffer, list );

        DKShuffle( (uintptr_t *)buffer, list->count );

        BufferToList( list, buffer );
        dk_free( buffer );
    }
}





