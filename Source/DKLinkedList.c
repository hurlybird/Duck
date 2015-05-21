/*****************************************************************************************

  DKLinkedList.c

  Copyright (c) 2014 Derek W. Nylen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*****************************************************************************************/

#include "DKLinkedList.h"
#include "DKNodePool.h"
#include "DKGenericArray.h"
#include "DKString.h"
#include "DKSet.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKEgg.h"


struct DKLinkedListNode
{
    struct DKLinkedListNode * prev;
    struct DKLinkedListNode * next;
    DKObjectRef object;
};

struct DKLinkedList
{
    DKObject _obj;

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
static void        DKLinkedListFinalize( DKObjectRef _self );

static DKObjectRef DKLinkedListInitWithEgg( DKLinkedListRef _self, DKEggUnarchiverRef egg );
static void        DKLinkedListAddToEgg( DKLinkedListRef _self, DKEggArchiverRef egg );

static DKIndex     INTERNAL_DKLinkedListGetCount( DKLinkedListRef _self );

static DKObjectRef INTERNAL_DKLinkedListGetObjectAtIndex( DKLinkedListRef _self, DKIndex index );
static DKIndex     INTERNAL_DKLinkedListGetObjectsInRange( DKLinkedListRef _self, DKRange range, DKObjectRef objects[] );

static void        INTERNAL_DKLinkedListAppendCArray( DKMutableLinkedListRef _self, DKObjectRef objects[], DKIndex count );
static void        INTERNAL_DKLinkedListAppendCollection( DKMutableLinkedListRef _self, DKObjectRef srcCollection );

static void        INTERNAL_DKLinkedListReplaceRangeWithCArray( DKMutableLinkedListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
static void        INTERNAL_DKLinkedListReplaceRangeWithCollection( DKMutableLinkedListRef _self, DKRange range, DKObjectRef srcCollection );


///
//  DKLinkedListClass()
//
DKThreadSafeClassInit( DKLinkedListClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKLinkedList" ), DKObjectClass(), sizeof(struct DKLinkedList), DKImmutableInstances, DKLinkedListInitialize, DKLinkedListFinalize );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKListEqual;
    comparison->compare = (DKCompareMethod)DKListCompare;
    comparison->hash = (DKHashMethod)DKPointerHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKLinkedListMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Description
    struct DKDescriptionInterface * description = DKAllocInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKCollectionGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Collection
    struct DKCollectionInterface * collection = DKAllocInterface( DKSelector(Collection), sizeof(struct DKCollectionInterface) );
    collection->getCount = (DKGetCountMethod)INTERNAL_DKLinkedListGetCount;
    collection->containsObject = (DKContainsMethod)DKListContainsObject;
    collection->foreachObject = (DKForeachObjectMethod)DKLinkedListApplyFunction;
    
    DKInstallInterface( cls, collection );
    DKRelease( collection );

    // List
    struct DKListInterface * list = DKAllocInterface( DKSelector(List), sizeof(struct DKListInterface) );
    list->initWithVAObjects = (DKListInitWithVAObjectsMethod)DKLinkedListInitWithVAObjects;
    list->initWithCArray = (DKListInitWithCArrayMethod)DKLinkedListInitWithCArray;
    list->initWithCollection = (DKListInitWithCollectionMethod)DKLinkedListInitWithCollection;
    
    list->getCount = (DKGetCountMethod)INTERNAL_DKLinkedListGetCount;
    list->getObjectAtIndex = (DKListGetObjectAtIndexMethod)INTERNAL_DKLinkedListGetObjectAtIndex;
    list->getObjectsInRange = (DKListGetObjectsInRangeMethod)INTERNAL_DKLinkedListGetObjectsInRange;
    
    list->appendCArray = (void *)DKImmutableObjectAccessError;
    list->appendCollection = (void *)DKImmutableObjectAccessError;
    list->replaceRangeWithCArray = (void *)DKImmutableObjectAccessError;
    list->replaceRangeWithCollection = (void *)DKImmutableObjectAccessError;
    list->sort = (void *)DKImmutableObjectAccessError;
    list->shuffle = (void *)DKImmutableObjectAccessError;

    DKInstallInterface( cls, list );
    DKRelease( list );
    
    // Set
    struct DKSetInterface * set = DKAllocInterface( DKSelector(Set), sizeof(struct DKSetInterface) );
    set->initWithVAObjects = (DKSetInitWithVAObjectsMethod)DKListInitSetWithVAObjects;
    set->initWithCArray = (DKSetInitWithCArrayMethod)DKListInitSetWithCArray;
    set->initWithCollection = (DKSetInitWithCollectionMethod)DKListInitSetWithCollection;

    set->getCount = (DKGetCountMethod)INTERNAL_DKLinkedListGetCount;
    set->getMember = (DKSetGetMemberMethod)DKListGetMemberOfSet;
    
    set->addObject = (void *)DKImmutableObjectAccessError;
    set->removeObject = (void *)DKImmutableObjectAccessError;
    set->removeAllObjects = (void *)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, set );
    DKRelease( set );

    // Egg
    struct DKEggInterface * egg = DKAllocInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKLinkedListInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKLinkedListAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKMutableLinkedListClass()
//
DKThreadSafeClassInit( DKMutableLinkedListClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKMutableLinkedList" ), DKLinkedListClass(), sizeof(struct DKLinkedList), 0, NULL, NULL );
    
    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = (DKCopyMethod)DKLinkedListMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKLinkedListMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // List
    struct DKListInterface * list = DKAllocInterface( DKSelector(List), sizeof(struct DKListInterface) );
    list->initWithVAObjects = (DKListInitWithVAObjectsMethod)DKLinkedListInitWithVAObjects;
    list->initWithCArray = (DKListInitWithCArrayMethod)DKLinkedListInitWithCArray;
    list->initWithCollection = (DKListInitWithCollectionMethod)DKLinkedListInitWithCollection;

    list->getCount = (DKGetCountMethod)INTERNAL_DKLinkedListGetCount;
    list->getObjectAtIndex = (DKListGetObjectAtIndexMethod)INTERNAL_DKLinkedListGetObjectAtIndex;
    list->getObjectsInRange = (DKListGetObjectsInRangeMethod)INTERNAL_DKLinkedListGetObjectsInRange;

    list->appendCArray = (DKListAppendCArrayMethod)INTERNAL_DKLinkedListAppendCArray;
    list->appendCollection = (DKListAppendCollectionMethod)INTERNAL_DKLinkedListAppendCollection;
    list->replaceRangeWithCArray = (DKListReplaceRangeWithCArrayMethod)INTERNAL_DKLinkedListReplaceRangeWithCArray;
    list->replaceRangeWithCollection = (DKListReplaceRangeWithCollectionMethod)INTERNAL_DKLinkedListReplaceRangeWithCollection;
    list->sort = (DKListSortMethod)DKLinkedListSort;
    list->shuffle = (DKListShuffleMethod)DKLinkedListShuffle;

    DKInstallInterface( cls, list );
    DKRelease( list );
    
    // Set
    struct DKSetInterface * set = DKAllocInterface( DKSelector(Set), sizeof(struct DKSetInterface) );
    set->initWithVAObjects = (DKSetInitWithVAObjectsMethod)DKListInitSetWithVAObjects;
    set->initWithCArray = (DKSetInitWithCArrayMethod)DKListInitSetWithCArray;
    set->initWithCollection = (DKSetInitWithCollectionMethod)DKListInitSetWithCollection;

    set->getCount = (DKGetCountMethod)INTERNAL_DKLinkedListGetCount;
    set->getMember = (DKSetGetMemberMethod)DKListGetMemberOfSet;
    
    set->addObject = (DKSetAddObjectMethod)DKListAddObjectToSet;
    set->removeObject = (DKSetRemoveObjectMethod)DKListRemoveObject;
    set->removeAllObjects = (DKSetRemoveAllObjectsMethod)DKListRemoveAllObjects;
    
    DKInstallInterface( cls, set );
    DKRelease( set );

    return cls;
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
//  RemoveRange()
//
static void RemoveRange( struct DKLinkedList * list, DKRange range )
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




// DKLinkedList Interface ================================================================

///
//  DKLinkedListInitialize()
//
static DKObjectRef DKLinkedListInitialize( DKObjectRef _self )
{
    _self = DKSuperInit( _self, DKObjectClass() );

    if( _self )
    {
        struct DKLinkedList * list = _self;
        
        DKNodePoolInit( &list->nodePool, sizeof(struct DKLinkedListNode), 0 );

        list->first = NULL;
        list->last = NULL;
        list->count = 0;
        
        list->cursor.node = NULL;
        list->cursor.index = 0;
    }
    
    return _self;
}


///
//  DKLinkedListFinalize()
//
static void DKLinkedListFinalize( DKObjectRef _self )
{
    struct DKLinkedList * list = _self;

    RemoveRange( list, DKRangeMake( 0, list->count ) );
    
    DKNodePoolFinalize( &list->nodePool );
}


///
//  DKLinkedListInitWithVAObjects()
//
DKObjectRef DKLinkedListInitWithVAObjects( DKLinkedListRef _self, va_list objects )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKLinkedListClass() );

        DKObjectRef object;
        
        while( (object = va_arg( objects, DKObjectRef )) != NULL )
        {
            InsertObject( _self, _self->count, object );
        }
    }

    return _self;
}


///
//  DKLinkedListInitWithCArray()
//
DKObjectRef DKLinkedListInitWithCArray( DKLinkedListRef _self, DKObjectRef objects[], DKIndex count )
{
    _self = DKInit( _self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKLinkedListClass() );
        INTERNAL_DKLinkedListReplaceRangeWithCArray( _self, DKRangeMake( 0, 0 ), objects, count );
    }

    return _self;
}


///
//  DKLinkedListInitWithCollection()
//
DKObjectRef DKLinkedListInitWithCollection( DKLinkedListRef _self, DKObjectRef collection )
{
    _self = DKInit( _self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKLinkedListClass() );
        INTERNAL_DKLinkedListReplaceRangeWithCollection( _self, DKRangeMake( 0, 0 ), collection );
    }

    return _self;
}


///
//  DKLinkedListInitWithEgg()
//
static int DKLinkedListInitWithEggCallback( DKObjectRef object, void * context )
{
    struct DKLinkedList * list = context;

    InsertObject( list, list->count, object );
    
    return 0;
}

static DKObjectRef DKLinkedListInitWithEgg( DKLinkedListRef _self, DKEggUnarchiverRef egg )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKLinkedListClass() );
        DKEggGetCollection( egg, DKSTR( "objects" ), DKLinkedListInitWithEggCallback, _self );
    }
    
    return _self;
}


///
//  DKLinkedListAddToEgg()
//
static void DKLinkedListAddToEgg( DKLinkedListRef _self, DKEggArchiverRef egg )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKLinkedListClass() );
        DKEggAddCollection( egg, DKSTR( "objects" ), _self );
    }
}


///
//  DKLinkedListCopy()
//
DKLinkedListRef DKLinkedListCopy( DKLinkedListRef _self )
{
    return DKLinkedListInitWithCollection( DKAlloc( DKGetClass( _self ) ), _self );
}


///
//  DKLinkedListMutableCopy()
//
DKMutableLinkedListRef DKLinkedListMutableCopy( DKLinkedListRef _self )
{
    return DKLinkedListInitWithCollection( DKAlloc( DKMutableLinkedListClass() ), _self );
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

static DKIndex INTERNAL_DKLinkedListGetCount( DKLinkedListRef _self )
{
    return _self->count;
}


///
//  DKLinkedListGetObjectAtIndex()
//
DKObjectRef DKLinkedListGetObjectAtIndex( DKLinkedListRef _self, DKIndex index )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKLinkedListClass() );
        return INTERNAL_DKLinkedListGetObjectAtIndex( _self, index );
    }
    
    return 0;
}

static DKObjectRef INTERNAL_DKLinkedListGetObjectAtIndex( DKLinkedListRef _self, DKIndex index )
{
    DKCheckIndex( index, _self->count, 0 );

    struct DKLinkedListNode * node = MoveCursor( _self, index );

    return node->object;
}


///
//  DKLinkedListGetObjectsInRange()
//
DKIndex DKLinkedListGetObjectsInRange( DKLinkedListRef _self, DKRange range, DKObjectRef objects[] )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKLinkedListClass() );
        return INTERNAL_DKLinkedListGetObjectsInRange( _self, range, objects );
    }
    
    return 0;
}

static DKIndex INTERNAL_DKLinkedListGetObjectsInRange( DKLinkedListRef _self, DKRange range, DKObjectRef objects[] )
{
    DKCheckRange( range, _self->count, 0 );

    for( DKIndex i = 0; i < range.length; ++i )
    {
        struct DKLinkedListNode * node = MoveCursor( _self, range.location + i );
        objects[i] = node->object;
    }
    
    return range.length;
}


///
//  DKLinkedListAppendCArray()
//
void DKLinkedListAppendCArray( DKMutableLinkedListRef _self, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableLinkedListClass() );
        INTERNAL_DKLinkedListReplaceRangeWithCArray( _self, DKRangeMake( _self->count, 0 ), objects, count );
    }
}

static void INTERNAL_DKLinkedListAppendCArray( DKMutableLinkedListRef _self, DKObjectRef objects[], DKIndex count )
{
    INTERNAL_DKLinkedListReplaceRangeWithCArray( _self, DKRangeMake( _self->count, 0 ), objects, count );
}


///
//  DKLinkedListAppendCollection()
//
void DKLinkedListAppendCollection( DKMutableLinkedListRef _self, DKObjectRef collection )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableLinkedListClass() );
        INTERNAL_DKLinkedListReplaceRangeWithCollection( _self, DKRangeMake( _self->count, 0 ), collection );
    }
}

static void INTERNAL_DKLinkedListAppendCollection( DKMutableLinkedListRef _self, DKObjectRef collection )
{
    INTERNAL_DKLinkedListReplaceRangeWithCollection( _self, DKRangeMake( _self->count, 0 ), collection );
}


///
//  DKLinkedListReplaceRangeWithCArray()
//
void DKLinkedListReplaceRangeWithCArray( DKMutableLinkedListRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableLinkedListClass() );
        INTERNAL_DKLinkedListReplaceRangeWithCArray( _self, range, objects, count );
    }
}

static void INTERNAL_DKLinkedListReplaceRangeWithCArray( struct DKLinkedList * list, DKRange range, DKObjectRef objects[], DKIndex count )
{
    DKCheckRange( range, list->count );

    RemoveRange( list, range );
    
    for( DKIndex i = 0; i < count; i++ )
        InsertObject( list, range.location + i, objects[i] );
}


///
//  DKLinkedListReplaceRangeWithCollection()
//
void DKLinkedListReplaceRangeWithCollection( DKMutableLinkedListRef _self, DKRange range, DKObjectRef collection )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableLinkedListClass() );
        INTERNAL_DKLinkedListReplaceRangeWithCollection( _self, range, collection );
    }
}

struct ReplaceRangeWithCollectionContext
{
    struct DKLinkedList * list;
    DKIndex index;
};

static int ReplaceRangeWithCollectionCallback( DKObjectRef object, void * context )
{
    struct ReplaceRangeWithCollectionContext * ctx = context;
    
    InsertObject( ctx->list, ctx->index, object );
    
    ctx->index++;
    
    return 0;
}

static void INTERNAL_DKLinkedListReplaceRangeWithCollection( struct DKLinkedList * list, DKRange range, DKObjectRef srcCollection )
{
    DKCheckRange( range, list->count );

    RemoveRange( list, range );

    if( srcCollection )
    {
        struct ReplaceRangeWithCollectionContext ctx = { list, range.location };
        DKForeachObject( srcCollection, ReplaceRangeWithCollectionCallback, &ctx );
    }
}


///
//  DKLinkedListSort()
//
static void ListToArray( DKGenericArray * array, struct DKLinkedList * list )
{
    struct DKLinkedListNode * node = list->first;
    
    for( DKIndex i = 0; i < list->count; ++i )
    {
        DKGenericArrayGetElementAtIndex( array, i, DKObjectRef ) = node->object;
        node = node->next;
    }
    
    array->length = list->count;
}

static void ArrayToList( struct DKLinkedList * list, DKGenericArray * array )
{
    struct DKLinkedListNode * node = list->first;
    
    for( DKIndex i = 0; i < list->count; ++i )
    {
        node->object = DKGenericArrayGetElementAtIndex( array, i, DKObjectRef );
        node = node->next;
    }
}

void DKLinkedListSort( DKMutableLinkedListRef _self, DKCompareFunction cmp )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableLinkedListClass() );

        // This is absurd, yet probably not much slower than doing all the pointer
        // gymnastics needed for sorting the list nodes.
        DKGenericArray array;
        DKGenericArrayInit( &array, sizeof(DKObjectRef) );
        DKGenericArrayReserve( &array, _self->count );
        ListToArray( &array, _self );

        DKGenericArraySort( &array, cmp );

        ArrayToList( _self, &array );
        DKGenericArrayFinalize( &array );
    }
}


///
//  DKLinkedListShuffle()
//
void DKLinkedListShuffle( DKMutableLinkedListRef _self )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableLinkedListClass() );
        
        // This is absurd, yet probably not much slower than doing all the pointer
        // gymnastics needed for shuffling the list nodes.
        DKGenericArray array;
        DKGenericArrayInit( &array, sizeof(DKObjectRef) );
        DKGenericArrayReserve( &array, _self->count );
        ListToArray( &array, _self );

        DKGenericArrayShuffle( &array );

        ArrayToList( _self, &array );
        DKGenericArrayFinalize( &array );
    }
}


///
//  DKLinkedListApplyFunction()
//
int DKLinkedListApplyFunction( DKLinkedListRef _self, DKApplierFunction callback, void * context )
{
    if( _self )
    {
        struct DKLinkedListNode * node = _self->first;
        
        while( node )
        {
            int result = callback( node->object, context );
            
            if( result )
                return result;
            
            node = node->next;
        }
    }
    
    return 0;
}




