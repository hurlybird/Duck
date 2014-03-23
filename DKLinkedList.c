//
//  DKLinkedList.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "DKLinkedList.h"
#include "DKMemory.h"
#include "DKCommonInterfaces.h"
#include "DKCopying.h"


#define DK_LINKED_LIST_ERROR_CHECKS 1


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


static DKTypeRef    DKLinkedListAllocate( void );
static DKTypeRef    DKMutableLinkedListAllocate( void );
static DKTypeRef    DKLinkedListInitialize( DKTypeRef ref );
static void         DKLinkedListFinalize( DKTypeRef ref );
static DKTypeRef    DKLinkedListCopy( DKTypeRef ref );
static DKTypeRef    DKMutableLinkedListCopy( DKTypeRef ref );
static DKTypeRef    DKLinkedListMutableCopy( DKTypeRef ref );


//DKDefineMethod( DKIndex, Count );


// DKLinkedList Class ====================================================================

/*
static const DKListInterface __DKLinkedListListInterface__ =
{
    DKStaticInterfaceObject( DKList ),
    
    DKLinkedListGetCallbacks,
    DKLinkedListGetCount,
    DKLinkedListGetValues,
    DKLinkedListReplaceValues,
    DKLinkedListReplaceValuesWithList
};

static const DKCopyingInterface __DKLinkedListCopyingInterface__ =
{
    DKStaticInterfaceObject( DKCopying ),
    
    DKLinkedListCopy,
    DKLinkedListMutableCopy
};

static DKIndex DKLinkedListGetCountMethod( DKTypeRef ref, DKSEL sel )
{
    return DKLinkedListGetCount( ref );
}

static const DKMethod __DKLinkedListCountMethod__ =
{
    DKStaticMethodObject( Count ),
    DKLinkedListGetCountMethod
};

static DKTypeRef __DKLinkedListInterfaces__[] =
{
    &__DKLinkedListListInterface__,
    &__DKLinkedListCopyingInterface__
};

static DKTypeRef __DKLinkedListMethods__[] =
{
    &__DKLinkedListCountMethod__
};

static const DKClass __DKLinkedListClass__ =
{
    DK_STATIC_CLASS_OBJECT,

    DKInterfaceTable( __DKLinkedListInterfaces__ ),
    DKMethodTable( __DKLinkedListMethods__ ),
    DKEmptyPropertyTable(),
    
    DKObjectGetInterface,
    DKObjectGetMethod,
    
    DKObjectRetain,
    DKObjectRelease,
    
    DKLinkedListAllocate,
    DKLinkedListInitialize,
    DKLinkedListFinalize,
    
    DKPtrEqual,
    DKPtrCompare,
    DKPtrHash
};
*/


// DKMutableLinkedList Class =============================================================
/*
static const DKCopyingInterface __DKMutableLinkedListCopyingInterface__ =
{
    DKStaticInterfaceObject( DKCopying ),
    
    DKMutableLinkedListCopy,
    DKLinkedListMutableCopy
};

static DKTypeRef __DKMutableLinkedListInterfaces__[] =
{
    &__DKLinkedListListInterface__,
    &__DKMutableLinkedListCopyingInterface__
};

static const DKClass __DKMutableLinkedListClass__ =
{
    DK_STATIC_CLASS_OBJECT,

    DKInterfaceTable( __DKMutableLinkedListInterfaces__ ),
    DKMethodTable( __DKLinkedListMethods__ ),
    DKEmptyPropertyTable(),
    
    DKObjectGetInterface,
    DKObjectGetMethod,
    
    DKObjectRetain,
    DKObjectRelease,
    
    DKMutableLinkedListAllocate,
    DKLinkedListInitialize,
    DKLinkedListFinalize,
    
    DKPtrEqual,
    DKPtrCompare,
    DKPtrHash
};
*/



// DKObject Interface ====================================================================

static void DKLinkedListReplaceObjectsInternal( struct DKLinkedList * list, DKRange range, DKTypeRef objects[], DKIndex count );
static void DKLinkedListReplaceObjectsWithListInternal( struct DKLinkedList * list, DKRange range, DKListRef srcList );


///
//  DKLinkedListClass()
//
DKTypeRef DKLinkedListClass( void )
{
    static DKTypeRef linkedListClass = NULL;

    if( !linkedListClass )
    {
        linkedListClass = DKAllocClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKLinkedListAllocate;
        lifeCycle->initialize = DKLinkedListInitialize;
        lifeCycle->finalize = DKLinkedListFinalize;

        DKInstallInterface( linkedListClass, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKLinkedListCopy;
        copying->mutableCopy = DKLinkedListMutableCopy;
        
        DKInstallInterface( linkedListClass, copying );
        DKRelease( copying );

        // List
        struct DKList * list = (struct DKList *)DKAllocInterface( DKSelector(List), sizeof(DKList) );
        list->getCount = DKLinkedListGetCount;
        list->getObjects = DKLinkedListGetObjects;
        list->replaceObjects = DKLinkedListReplaceObjects;
        list->replaceObjectsWithList = DKLinkedListReplaceObjectsWithList;

        DKInstallInterface( linkedListClass, list );
        DKRelease( list );
    }
    
    return linkedListClass;
}


///
//  DKMutableLinkedListClass()
//
DKTypeRef DKMutableLinkedListClass( void )
{
    static DKTypeRef mutableLinkedListClass = NULL;

    if( !mutableLinkedListClass )
    {
        mutableLinkedListClass = DKAllocClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKMutableLinkedListAllocate;
        lifeCycle->initialize = DKLinkedListInitialize;
        lifeCycle->finalize = DKLinkedListFinalize;

        DKInstallInterface( mutableLinkedListClass, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKMutableLinkedListCopy;
        copying->mutableCopy = DKLinkedListMutableCopy;
        
        DKInstallInterface( mutableLinkedListClass, copying );
        DKRelease( copying );

        // List
        struct DKList * list = (struct DKList *)DKAllocInterface( DKSelector(List), sizeof(DKList) );
        list->getCount = DKLinkedListGetCount;
        list->getObjects = DKLinkedListGetObjects;
        list->replaceObjects = DKLinkedListReplaceObjects;
        list->replaceObjectsWithList = DKLinkedListReplaceObjectsWithList;

        DKInstallInterface( mutableLinkedListClass, list );
        DKRelease( list );
    }
    
    return mutableLinkedListClass;
}


///
//  DKLinkedListAllocate()
//
static DKTypeRef DKLinkedListAllocate( void )
{
    return DKAllocObject( DKLinkedListClass(), sizeof(struct DKLinkedList), 0 );
}


///
//  DKMutableLinkedListAllocate()
//
static DKTypeRef DKMutableLinkedListAllocate( void )
{
    return DKAllocObject( DKMutableLinkedListClass(), sizeof(struct DKLinkedList), DKObjectIsMutable );
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

        DKLinkedListReplaceObjectsInternal( list, DKRangeMake( 0, list->count ), NULL, 0 );
        
        DKNodePoolClear( &list->nodePool );
    }
}


///
//  DKLinkedListCopy()
//
static DKTypeRef DKLinkedListCopy( DKTypeRef ref )
{
    return DKRetain( ref );
}


///
//  DKMutableLinkedListCopy()
//
static DKTypeRef DKMutableLinkedListCopy( DKTypeRef ref )
{
    return DKLinkedListCreateCopy( ref );
}


///
//  DKLinkedListMutableCopy()
//
static DKTypeRef DKLinkedListMutableCopy( DKTypeRef ref )
{
    return DKLinkedListCreateMutableCopy( ref );
}




// Internals =============================================================================

///
//  DKLinkedListCheckForErrors()
//
#if DK_LINKED_LIST_ERROR_CHECKS
static void DKLinkedListCheckForErrors( struct DKLinkedList * list )
{
    DKIndex count = 0;
    
    struct DKLinkedListNode * node = list->first;
    
    if( node == NULL )
    {
        assert( list->last == NULL );
        assert( list->cursor.node == NULL );
        assert( list->cursor.index == 0 );
    }
    
    while( node )
    {
        assert( count < list->count );

        if( node->prev )
        {
            assert( node->prev->next == node );
        }
        
        if( node->next )
        {
            assert( node->next->prev == node );
        }
        
        if( list->cursor.node == node )
        {
            assert( list->cursor.index == count );
        }
        
        node = node->next;
    
        count++;
    }
    
    assert( count == list->count );
}
#else
#define DKLinkedListCheckForErrors( list )
#endif


///
//  DKLinkedListAllocNode()
//
static struct DKLinkedListNode * DKLinkedListAllocNode( struct DKLinkedList * list, DKTypeRef object )
{
    struct DKLinkedListNode * node = DKNodePoolAlloc( &list->nodePool );

    node->prev = NULL;
    node->next = NULL;
    
    node->object = DKRetain( object );
    
    list->count++;
    
    return node;
}


///
//  DKLinkedListFreeNode()
//
static void DKLinkedListFreeNode( struct DKLinkedList * list, struct DKLinkedListNode * node )
{
    assert( list->count > 0 );
    list->count--;

    DKRelease( node->object );
    
    DKNodePoolFree( &list->nodePool, node );
}


///
//  DKLinkedListMoveCursor()
//
static struct DKLinkedListNode * DKLinkedListMoveCursor( struct DKLinkedList * list, DKIndex index )
{
    if( list->first == NULL )
    {
        DKLinkedListCheckForErrors( list );
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

    DKLinkedListCheckForErrors( list );
    
    return list->cursor.node;
}


///
//  DKLinkedListRemoveObjects()
//
static void DKLinkedListRemoveObjects( struct DKLinkedList * list, DKRange range )
{
    assert( range.location >= 0 );
    assert( range.length >= 0 );
    assert( DKRangeEnd( range ) <= list->count );

    if( range.length == 0 )
        return;
    
    DKLinkedListMoveCursor( list, range.location );

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

        DKLinkedListFreeNode( list, node );
        
        list->cursor.node = next;
    }
    
    if( list->cursor.node == NULL )
    {
        list->cursor.node = list->last;
        list->cursor.index = list->count;
    }

    DKLinkedListCheckForErrors( list );
}


///
//  DKLinkedListInsertObject()
//
static void DKLinkedListInsertObject( struct DKLinkedList * list, DKIndex index, DKTypeRef object )
{
    assert( index >= 0 );
    assert( index <= list->count );

    if( list->first == NULL )
    {
        assert( index == 0 );

        struct DKLinkedListNode * node = DKLinkedListAllocNode( list, object );

        list->first = node;
        list->last = node;
        
        list->cursor.node = node;
        list->cursor.index = 0;
    }
    
    else if( index == 0 )
    {
        struct DKLinkedListNode * node = DKLinkedListAllocNode( list, object );
        
        node->next = list->first;
        list->first->prev = node;
        list->first = node;
        
        list->cursor.node = node;
        list->cursor.index = 0;
    }
    
    else if( index == list->count )
    {
        struct DKLinkedListNode * node = DKLinkedListAllocNode( list, object );
        
        node->prev = list->last;
        list->last->next = node;
        list->last = node;
        
        list->cursor.node = node;
        list->cursor.index = list->count - 1;
    }
    
    else
    {
        struct DKLinkedListNode * next = DKLinkedListMoveCursor( list, index );
        struct DKLinkedListNode * node = DKLinkedListAllocNode( list, object );
        
        assert( next );
        
        node->next = next;
        node->prev = next->prev;
        node->prev->next = node;
        node->next->prev = node;
        
        list->cursor.node = node;
    }

    DKLinkedListCheckForErrors( list );
}


///
//  DKLinkedListReplaceObjectsInternal()
//
static void DKLinkedListReplaceObjectsInternal( struct DKLinkedList * list, DKRange range, DKTypeRef objects[], DKIndex count )
{
    DKLinkedListRemoveObjects( list, range );
    
    for( DKIndex i = 0; i < count; i++ )
        DKLinkedListInsertObject( list, range.location + i, objects[i] );
}


///
//  DKLinkedListReplaceObjectsWithListInternal()
//
static void DKLinkedListReplaceObjectsWithListInternal( struct DKLinkedList * list, DKRange range, DKListRef srcList )
{
    DKLinkedListRemoveObjects( list, range );

    DKList * srcListInterface = DKLookupInterface( srcList, DKSelector(List) );
    assert( srcListInterface );
    
    DKIndex count = srcListInterface->getCount( srcList );
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKTypeRef object;
        
        srcListInterface->getObjects( srcList, DKRangeMake( i, 1 ), &object );

        DKLinkedListInsertObject( list, range.location + i, object );
    }
}




// DKLinkedList Interface ================================================================

///
//  DKLinkedListCreate()
//
DKListRef DKLinkedListCreate( DKTypeRef objects[], DKIndex count )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKLinkedListClass() );
    
    DKLinkedListReplaceObjectsInternal( list, DKRangeMake( 0, 0 ), objects, count );
    
    return list;
}


///
//  DKLinkedListCreateCopy()
//
DKListRef DKLinkedListCreateCopy( DKListRef srcList )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKLinkedListClass() );
    
    DKLinkedListReplaceObjectsWithListInternal( list, DKRangeMake( 0, 0 ), srcList );
    
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
    
    DKLinkedListReplaceObjectsWithListInternal( list, DKRangeMake( 0, 0 ), srcList );
    
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
            struct DKLinkedListNode * node = DKLinkedListMoveCursor( list, range.location + i );
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
            assert( 0 );
            return;
        }

        struct DKLinkedList * list = (struct DKLinkedList *)ref;
        DKLinkedListReplaceObjectsInternal( list, range, objects, count );
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
            assert( 0 );
            return;
        }

        struct DKLinkedList * list = (struct DKLinkedList *)ref;
        DKLinkedListReplaceObjectsWithListInternal( list, range, srcList );
    }
}




