//
//  DKLinkedList.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "DKLinkedList.h"
#include "DKCopying.h"


#define DK_LINKED_LIST_ERROR_CHECKS 1


struct DKLinkedListNode
{
    struct DKLinkedListNode * prev;
    struct DKLinkedListNode * next;
    DKTypeRef value;
};

struct DKLinkedList
{
    const DKObjectHeader _obj;

    DKNodePool nodePool;
    DKListCallbacks callbacks;

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


DKDefineMethod( DKIndex, Count );


// DKLinkedList Class ====================================================================

static const DKListInterface __DKLinkedListListInterface__ =
{
    DKStaticInterfaceObject( DKListInterfaceID ),
    
    DKLinkedListGetCallbacks,
    DKLinkedListGetCount,
    DKLinkedListGetValues,
    DKLinkedListReplaceValues,
    DKLinkedListReplaceValuesWithList
};

static const DKCopyingInterface __DKLinkedListCopyingInterface__ =
{
    DKStaticInterfaceObject( DKCopyingInterfaceID ),
    
    DKLinkedListCopy,
    DKLinkedListMutableCopy
};

static const DKMethod __DKLinkedListCountMethod__ =
{
    DKStaticMethodObject( DKSelector_Count ),
    
    DKLinkedListGetCount
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



// DKMutableLinkedList Class =============================================================

static const DKCopyingInterface __DKMutableLinkedListCopyingInterface__ =
{
    DKStaticInterfaceObject( DKCopyingInterfaceID ),
    
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




// DKObject Interface ====================================================================

static void DKLinkedListReplaceValuesInternal( struct DKLinkedList * list, DKRange range, const void ** values, DKIndex count );
static void DKLinkedListReplaceValuesWithListInternal( struct DKLinkedList * list, DKRange range, DKListRef srcList );


///
//  DKLinkedListClass()
//
DKTypeRef DKLinkedListClass( void )
{
    return &__DKLinkedListClass__;
}


///
//  DKMutableLinkedListClass()
//
DKTypeRef DKMutableLinkedListClass( void )
{
    return &__DKMutableLinkedListClass__;
}


///
//  DKLinkedListAllocate()
//
static DKTypeRef DKLinkedListAllocate( void )
{
    return DKNewObject( DKLinkedListClass(), sizeof(struct DKLinkedList), 0 );
}


///
//  DKMutableLinkedListAllocate()
//
static DKTypeRef DKMutableLinkedListAllocate( void )
{
    return DKNewObject( DKMutableLinkedListClass(), sizeof(struct DKLinkedList), DKObjectMutable );
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

        list->callbacks = *DKListObjectCallbacks();
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

        DKLinkedListReplaceValuesInternal( list, DKRangeMake( 0, list->count ), NULL, 0 );
        
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
static struct DKLinkedListNode * DKLinkedListAllocNode( struct DKLinkedList * list, DKTypeRef value )
{
    struct DKLinkedListNode * node = DKNodePoolAlloc( &list->nodePool );

    node->prev = NULL;
    node->next = NULL;
    
    node->value = list->callbacks.retain( value );
    
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

    list->callbacks.release( node->value );
    
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
//  DKLinkedListRemoveValues()
//
static void DKLinkedListRemoveValues( struct DKLinkedList * list, DKRange range )
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
//  DKLinkedListInsertValue()
//
static void DKLinkedListInsertValue( struct DKLinkedList * list, DKIndex index, const void * value )
{
    assert( index >= 0 );
    assert( index <= list->count );

    if( list->first == NULL )
    {
        assert( index == 0 );

        struct DKLinkedListNode * node = DKLinkedListAllocNode( list, value );

        list->first = node;
        list->last = node;
        
        list->cursor.node = node;
        list->cursor.index = 0;
    }
    
    else if( index == 0 )
    {
        struct DKLinkedListNode * node = DKLinkedListAllocNode( list, value );
        
        node->next = list->first;
        list->first->prev = node;
        list->first = node;
        
        list->cursor.node = node;
        list->cursor.index = 0;
    }
    
    else if( index == list->count )
    {
        struct DKLinkedListNode * node = DKLinkedListAllocNode( list, value );
        
        node->prev = list->last;
        list->last->next = node;
        list->last = node;
        
        list->cursor.node = node;
        list->cursor.index = list->count - 1;
    }
    
    else
    {
        struct DKLinkedListNode * next = DKLinkedListMoveCursor( list, index );
        struct DKLinkedListNode * node = DKLinkedListAllocNode( list, value );
        
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
//  DKLinkedListReplaceValuesInternal()
//
static void DKLinkedListReplaceValuesInternal( struct DKLinkedList * list, DKRange range, const void ** values, DKIndex count )
{
    DKLinkedListRemoveValues( list, range );
    
    for( DKIndex i = 0; i < count; i++ )
        DKLinkedListInsertValue( list, range.location + i, values[i] );
}


///
//  DKLinkedListReplaceValuesInRangeInternal()
//
static void DKLinkedListReplaceValuesWithListInternal( struct DKLinkedList * list, DKRange range, DKListRef srcList )
{
    DKLinkedListRemoveValues( list, range );

    DKListInterfaceRef srcListInterface = DKGetInterface( srcList, &DKListInterfaceID );
    assert( srcListInterface );
    
    DKIndex count = srcListInterface->getCount( srcList );
    
    for( DKIndex i = 0; i < count; ++i )
    {
        const void * value;
        
        srcListInterface->getValues( srcList, DKRangeMake( i, 1 ), &value );

        DKLinkedListInsertValue( list, range.location + i, value );
    }
}




// DKLinkedList Interface ================================================================

///
//  DKLinkedListCreate()
//
DKListRef DKLinkedListCreate( const void ** values, DKIndex count, const DKListCallbacks * callbacks )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKLinkedListClass() );
    
    if( callbacks )
        list->callbacks = *callbacks;

    DKLinkedListReplaceValuesInternal( list, DKRangeMake( 0, 0 ), values, count );
    
    return list;
}


///
//  DKLinkedListCreateCopy()
//
DKListRef DKLinkedListCreateCopy( DKListRef srcList )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKLinkedListClass() );
    
    DKListInterfaceRef srcListInterface = DKGetInterface( srcList, &DKListInterfaceID );
    assert( srcListInterface );

    list->callbacks = *srcListInterface->getCallbacks( srcList );

    DKLinkedListReplaceValuesWithListInternal( list, DKRangeMake( 0, 0 ), srcList );
    
    return list;
}


///
//  DKLinkedListCreateMutable()
//
DKMutableListRef DKLinkedListCreateMutable( const DKListCallbacks * callbacks )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKMutableLinkedListClass() );
    
    if( callbacks )
        list->callbacks = *callbacks;
    
    return list;
}


///
//  DKLinkedListCreateMutableCopy()
//
DKMutableListRef DKLinkedListCreateMutableCopy( DKListRef srcList )
{
    struct DKLinkedList * list = (struct DKLinkedList *)DKCreate( DKMutableLinkedListClass() );
    
    DKListInterfaceRef srcListInterface = DKGetInterface( srcList, &DKListInterfaceID );
    assert( srcListInterface );

    list->callbacks = *srcListInterface->getCallbacks( srcList );

    DKLinkedListReplaceValuesWithListInternal( list, DKRangeMake( 0, 0 ), srcList );
    
    return list;
}


///
//  DKLinkedListGetCallbacks()
//
const DKListCallbacks * DKLinkedListGetCallbacks( DKListRef ref )
{
    if( ref )
    {
        const struct DKLinkedList * list = ref;
        return &list->callbacks;
    }
    
    return NULL;
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
//  DKLinkedListGetValues()
//
DKIndex DKLinkedListGetValues( DKListRef ref, DKRange range, const void ** values )
{
    if( ref )
    {
        struct DKLinkedList * list = (struct DKLinkedList *)ref;
        
        for( DKIndex i = 0; i < range.length; ++i )
        {
            struct DKLinkedListNode * node = DKLinkedListMoveCursor( list, range.location + i );
            values[i] = node->value;
        }
    }
    
    return 0;
}


///
//  DKLinkedListReplaceValues()
//
void DKLinkedListReplaceValues( DKMutableListRef ref, DKRange range, const void ** values, DKIndex count )
{
    if( ref )
    {
        if( !DKTestAttribute( ref, DKObjectMutable ) )
        {
            assert( 0 );
            return;
        }

        struct DKLinkedList * list = (struct DKLinkedList *)ref;
        DKLinkedListReplaceValuesInternal( list, range, values, count );
    }
}


///
//  DKLinkedListReplaceValuesWithList()
//
void DKLinkedListReplaceValuesWithList( DKMutableListRef ref, DKRange range, DKListRef srcList )
{
    if( ref )
    {
        if( !DKTestAttribute( ref, DKObjectMutable ) )
        {
            assert( 0 );
            return;
        }

        struct DKLinkedList * list = (struct DKLinkedList *)ref;
        DKLinkedListReplaceValuesWithListInternal( list, range, srcList );
    }
}




