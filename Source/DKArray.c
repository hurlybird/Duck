/*****************************************************************************************

  DKArray.c

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

#include "DKArray.h"
#include "DKGenericArray.h"
#include "DKString.h"
#include "DKSet.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKEgg.h"


struct DKArray
{
    DKObject _obj;
    DKGenericArray ptrArray;
};


static DKObjectRef DKArrayInitialize( DKObjectRef _self );
static void        DKArrayFinalize( DKObjectRef _self );

static DKObjectRef DKArrayInitWithEgg( DKArrayRef _self, DKEggUnarchiverRef egg );
static void        DKArrayAddToEgg( DKArrayRef _self, DKEggArchiverRef egg );

static DKIndex     INTERNAL_DKArrayGetCount( DKArrayRef _self );

static DKObjectRef INTERNAL_DKArrayGetObjectAtIndex( DKArrayRef _self, DKIndex index );
static DKIndex     INTERNAL_DKArrayGetObjectsInRange( DKArrayRef _self, DKRange range, DKObjectRef objects[] );

static void        INTERNAL_DKArrayAppendCArray( DKMutableArrayRef _self, DKObjectRef objects[], DKIndex count );
static void        INTERNAL_DKArrayAppendCollection( DKMutableArrayRef _self, DKObjectRef srcCollection );

static void        INTERNAL_DKArrayReplaceRangeWithCArray( DKMutableArrayRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
static void        INTERNAL_DKArrayReplaceRangeWithCollection( DKMutableArrayRef _self, DKRange range, DKObjectRef collection );


///
//  DKArrayClass()
//
DKThreadSafeClassInit( DKArrayClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKArray" ), DKObjectClass(), sizeof(struct DKArray), DKImmutableInstances, DKArrayInitialize, DKArrayFinalize );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKListEqual;
    comparison->like = (DKEqualityMethod)DKListEqual;
    comparison->compare = (DKCompareMethod)DKListCompare;
    comparison->hash = (DKHashMethod)DKPointerHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKArrayMutableCopy;
    
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
    collection->getCount = (DKGetCountMethod)INTERNAL_DKArrayGetCount;
    collection->containsObject = (DKContainsMethod)DKListContainsObject;
    collection->foreachObject = (DKForeachObjectMethod)DKArrayApplyFunction;
    
    DKInstallInterface( cls, collection );
    DKRelease( collection );

    // List
    struct DKListInterface * list = DKAllocInterface( DKSelector(List), sizeof(struct DKListInterface) );
    list->initWithVAObjects = (DKListInitWithVAObjectsMethod)DKArrayInitWithVAObjects;
    list->initWithCArray = (DKListInitWithCArrayMethod)DKArrayInitWithCArray;
    list->initWithCollection = (DKListInitWithCollectionMethod)DKArrayInitWithCollection;
    
    list->getCount = (DKGetCountMethod)INTERNAL_DKArrayGetCount;
    list->getObjectAtIndex = (DKListGetObjectAtIndexMethod)INTERNAL_DKArrayGetObjectAtIndex;
    list->getObjectsInRange = (DKListGetObjectsInRangeMethod)INTERNAL_DKArrayGetObjectsInRange;
    
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

    set->getCount = (DKGetCountMethod)INTERNAL_DKArrayGetCount;
    set->getMember = (DKSetGetMemberMethod)DKListGetMemberOfSet;
    
    set->addObject = (void *)DKImmutableObjectAccessError;
    set->removeObject = (void *)DKImmutableObjectAccessError;
    set->removeAllObjects = (void *)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, set );
    DKRelease( set );

    // Egg
    struct DKEggInterface * egg = DKAllocInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKArrayInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKArrayAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKMutableArrayClass()
//
DKThreadSafeClassInit( DKMutableArrayClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKMutableArray" ), DKArrayClass(), sizeof(struct DKArray), 0, NULL, NULL );
    
    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = (DKCopyMethod)DKArrayMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKArrayMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // List
    struct DKListInterface * list = DKAllocInterface( DKSelector(List), sizeof(struct DKListInterface) );
    list->initWithVAObjects = (DKListInitWithVAObjectsMethod)DKArrayInitWithVAObjects;
    list->initWithCArray = (DKListInitWithCArrayMethod)DKArrayInitWithCArray;
    list->initWithCollection = (DKListInitWithCollectionMethod)DKArrayInitWithCollection;
    
    list->getCount = (DKGetCountMethod)INTERNAL_DKArrayGetCount;
    list->getObjectAtIndex = (DKListGetObjectAtIndexMethod)INTERNAL_DKArrayGetObjectAtIndex;
    list->getObjectsInRange = (DKListGetObjectsInRangeMethod)INTERNAL_DKArrayGetObjectsInRange;

    list->appendCArray = (DKListAppendCArrayMethod)INTERNAL_DKArrayAppendCArray;
    list->appendCollection = (DKListAppendCollectionMethod)INTERNAL_DKArrayAppendCollection;
    list->replaceRangeWithCArray = (DKListReplaceRangeWithCArrayMethod)INTERNAL_DKArrayReplaceRangeWithCArray;
    list->replaceRangeWithCollection = (DKListReplaceRangeWithCollectionMethod)INTERNAL_DKArrayReplaceRangeWithCollection;
    list->sort = (DKListSortMethod)DKArraySort;
    list->shuffle = (DKListShuffleMethod)DKArrayShuffle;

    DKInstallInterface( cls, list );
    DKRelease( list );

    // Set
    struct DKSetInterface * set = DKAllocInterface( DKSelector(Set), sizeof(struct DKSetInterface) );
    set->initWithVAObjects = (DKSetInitWithVAObjectsMethod)DKListInitSetWithVAObjects;
    set->initWithCArray = (DKSetInitWithCArrayMethod)DKListInitSetWithCArray;
    set->initWithCollection = (DKSetInitWithCollectionMethod)DKListInitSetWithCollection;

    set->getCount = (DKGetCountMethod)INTERNAL_DKArrayGetCount;
    set->getMember = (DKSetGetMemberMethod)DKListGetMemberOfSet;
    
    set->addObject = (DKSetAddObjectMethod)DKListAddObjectToSet;
    set->removeObject = (DKSetRemoveObjectMethod)DKListRemoveObject;
    set->removeAllObjects = (DKSetRemoveAllObjectsMethod)DKListRemoveAllObjects;
    
    DKInstallInterface( cls, set );
    DKRelease( set );
    
    return cls;
}




// DKArray Interface =====================================================================

///
//  DKArrayInitialize()
//
static DKObjectRef DKArrayInitialize( DKObjectRef _self )
{
    _self = DKSuperInit( _self, DKObjectClass() );

    if( _self )
    {
        struct DKArray * array = _self;
        DKGenericArrayInit( &array->ptrArray, sizeof(DKObjectRef) );
    }
    
    return _self;
}


///
//  DKArrayFinalize()
//
static void DKArrayFinalize( DKObjectRef _self )
{
    struct DKArray * array = _self;

    if( !DKGenericArrayHasExternalStorage( &array->ptrArray ) )
    {
        DKIndex count = DKGenericArrayGetLength( &array->ptrArray );

        for( DKIndex i = 0; i < count; ++i )
        {
            DKObjectRef elem = DKGenericArrayGetElementAtIndex( &array->ptrArray, i, DKObjectRef );
            DKRelease( elem );
        }
    }
    
    DKGenericArrayFinalize( &array->ptrArray );
}


///
//  DKArrayInitWithVAObjects()
//
DKObjectRef DKArrayInitWithVAObjects( DKArrayRef _self, va_list objects )
{
    _self = DKInit( _self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );

        DKObjectRef object;
        
        while( (object = va_arg( objects, DKObjectRef )) != NULL )
        {
            INTERNAL_DKArrayReplaceRangeWithCArray( _self, DKRangeMake( _self->ptrArray.length, 0 ), &object, 1 );
        }
    }

    return _self;
}


///
//  DKArrayInitWithCArray()
//
DKObjectRef DKArrayInitWithCArray( DKArrayRef _self, DKObjectRef objects[], DKIndex count )
{
    _self = DKInit( _self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        INTERNAL_DKArrayReplaceRangeWithCArray( _self, DKRangeMake( 0, 0 ), objects, count );
    }

    return _self;
}


///
//  DKArrayInitWithCollection()
//
DKObjectRef DKArrayInitWithCollection( DKArrayRef _self, DKObjectRef collection )
{
    _self = DKInit( _self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        INTERNAL_DKArrayReplaceRangeWithCollection( _self, DKRangeMake( 0, 0 ), collection );
    }

    return _self;
}


///
//  DKArrayInitWithCArrayNoCopy()
//
DKObjectRef DKArrayInitWithCArrayNoCopy( DKArrayRef _self, DKObjectRef objects[], DKIndex count )
{
    _self = DKInit( _self );

    if( _self )
    {
        DKAssertMemberOfClass( _self, DKArrayClass() );
        DKGenericArrayInitWithExternalStorage( (DKGenericArray *)&_self->ptrArray, objects, sizeof(DKObjectRef), count );
    }
    
    return _self;
}


///
//  DKArrayInitWithEgg()
//
static int DKArrayInitWithEggCallback( DKObjectRef object, void * context )
{
    struct DKArray * array = context;

    DKRetain( object );
    DKGenericArrayAppendElements( &array->ptrArray, &object, 1 );
    
    return 0;
}

static DKObjectRef DKArrayInitWithEgg( DKArrayRef _self, DKEggUnarchiverRef egg )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        DKEggGetCollection( egg, DKSTR( "objects" ), DKArrayInitWithEggCallback, _self );
    }
    
    return _self;
}


///
//  DKArrayAddToEgg()
//
static void DKArrayAddToEgg( DKArrayRef _self, DKEggArchiverRef egg )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        DKEggAddCollection( egg, DKSTR( "objects" ), _self );
    }
}


///
//  DKArrayCopy()
//
DKArrayRef DKArrayCopy( DKArrayRef _self )
{
    return DKArrayCreateWithCollection( DKGetClass( _self ), _self );
}


///
//  DKArrayMutableCopy()
//
DKMutableArrayRef DKArrayMutableCopy( DKArrayRef _self )
{
    return DKArrayCreateWithCollection( DKMutableArrayClass(), _self );
}


///
//  DKArrayGetCount()
//
DKIndex DKArrayGetCount( DKArrayRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        return _self->ptrArray.length;
    }
    
    return 0;
}

static DKIndex INTERNAL_DKArrayGetCount( DKArrayRef _self )
{
    return _self->ptrArray.length;
}


///
//  DKArrayGetObjectAtIndex()
//
DKObjectRef DKArrayGetObjectAtIndex( DKArrayRef _self, DKIndex index )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        return INTERNAL_DKArrayGetObjectAtIndex( _self, index );
    }
    
    return NULL;
}

static DKObjectRef INTERNAL_DKArrayGetObjectAtIndex( DKArrayRef _self, DKIndex index )
{
    DKCheckIndex( index, _self->ptrArray.length, 0 );
    
    return DKGenericArrayGetElementAtIndex( (DKGenericArray *)&_self->ptrArray, index, DKObjectRef );
}


///
//  DKArrayGetObjectsInRange()
//
DKIndex DKArrayGetObjectsInRange( DKArrayRef _self, DKRange range, DKObjectRef objects[] )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        return INTERNAL_DKArrayGetObjectsInRange( _self, range, objects );
    }
    
    return 0;
}

static DKIndex INTERNAL_DKArrayGetObjectsInRange( DKArrayRef _self, DKRange range, DKObjectRef objects[] )
{
    DKCheckRange( range, _self->ptrArray.length, 0 );

    const DKObjectRef * src = DKGenericArrayGetPointerToElementAtIndex( (DKGenericArray *)&_self->ptrArray, range.location );
    memcpy( objects, src, sizeof(DKObjectRef) * range.length );
    
    return range.length;
}


///
//  DKArrayAppendCArray()
//
void DKArrayAppendCArray( DKMutableArrayRef _self, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableArrayClass() );
        INTERNAL_DKArrayReplaceRangeWithCArray( _self, DKRangeMake( _self->ptrArray.length, 0 ), objects, count );
    }
}

static void INTERNAL_DKArrayAppendCArray( DKMutableArrayRef _self, DKObjectRef objects[], DKIndex count )
{
    INTERNAL_DKArrayReplaceRangeWithCArray( _self, DKRangeMake( _self->ptrArray.length, 0 ), objects, count );
}


///
//  DKArrayAppendCollection()
//
void DKArrayAppendCollection( DKMutableArrayRef _self, DKObjectRef srcCollection )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableArrayClass() );
        INTERNAL_DKArrayReplaceRangeWithCollection( _self, DKRangeMake( _self->ptrArray.length, 0 ), srcCollection );
    }
}

static void INTERNAL_DKArrayAppendCollection( DKMutableArrayRef _self, DKObjectRef srcCollection )
{
    INTERNAL_DKArrayReplaceRangeWithCollection( _self, DKRangeMake( _self->ptrArray.length, 0 ), srcCollection );
}


///
//  DKArrayReplaceRangeWithCArray()
//
void DKArrayReplaceRangeWithCArray( DKMutableArrayRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableArrayClass() );
        INTERNAL_DKArrayReplaceRangeWithCArray( _self, range, objects, count );
    }
}

static void INTERNAL_DKArrayReplaceRangeWithCArray( DKMutableArrayRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    DKCheckRange( range, _self->ptrArray.length );
    
    // Retain the incoming objects
    for( DKIndex i = 0; i < count; ++i )
    {
        DKRetain( objects[i] );
    }
    
    // Release the objects we're replacing
    for( DKIndex i = 0; i < range.length; ++i )
    {
        DKObjectRef obj = DKGenericArrayGetElementAtIndex( &_self->ptrArray, range.location + i, DKObjectRef );
        DKRelease( obj );
    }
    
    // Copy the objects into the array
    DKGenericArrayReplaceElements( &_self->ptrArray, range, objects, count );
}


///
//  DKArrayReplaceRangeWithCollection()
//
void DKArrayReplaceRangeWithCollection( DKMutableArrayRef _self, DKRange range, DKObjectRef srcCollection )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableArrayClass() );
        INTERNAL_DKArrayReplaceRangeWithCollection( _self, range, srcCollection );
    }
}

struct ReplaceRangeWithCollectionContext
{
    struct DKArray * array;
    DKIndex index;
};

static int ReplaceRangeWithCollectionCallback( DKObjectRef object, void * context )
{
    struct ReplaceRangeWithCollectionContext * ctx = context;
    
    DKGenericArrayGetElementAtIndex( &ctx->array->ptrArray, ctx->index, DKObjectRef ) = DKRetain( object );
    ctx->index++;
    
    return 0;
}

static void INTERNAL_DKArrayReplaceRangeWithCollection( struct DKArray * array, DKRange range, DKObjectRef srcCollection )
{
    if( srcCollection )
    {
        DKCheckRange( range, array->ptrArray.length );

        DKCollectionInterfaceRef collection = DKGetInterface( srcCollection, DKSelector(Collection) );
        
        // Release the objects we're replacing
        for( DKIndex i = 0; i < range.length; ++i )
        {
            DKObjectRef obj = DKGenericArrayGetElementAtIndex( &array->ptrArray, range.location + i, DKObjectRef );
            DKRelease( obj );
        }

        // Resize our array
        DKIndex srcCount = collection->getCount( srcCollection );

        DKGenericArrayReplaceElements( &array->ptrArray, range, NULL, srcCount );
        
        // Copy the collection into our array
        struct ReplaceRangeWithCollectionContext ctx = { array, range.location };
        collection->foreachObject( srcCollection, ReplaceRangeWithCollectionCallback, &ctx );
    }
    
    else
    {
        INTERNAL_DKArrayReplaceRangeWithCArray( array, range, NULL, 0 );
    }
}


///
//  DKArraySort()
//
void DKArraySort( DKMutableArrayRef _self, DKCompareFunction cmp )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableArrayClass() );
        DKGenericArraySort( &_self->ptrArray, cmp );
    }
}


///
//  DKArrayShuffle()
//
void DKArrayShuffle( DKMutableArrayRef _self )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableArrayClass() );
        DKGenericArrayShuffle( &_self->ptrArray );
    }
}


///
//  DKArrayApplyFunction()
//
int DKArrayApplyFunction( DKArrayRef _self, DKApplierFunction callback, void * context )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );

        for( DKIndex i = 0; i < _self->ptrArray.length; ++i )
        {
            DKObjectRef obj = DKGenericArrayGetElementAtIndex( (DKGenericArray *)&_self->ptrArray, i, DKObjectRef );
        
            int result = callback( obj, context );
            
            if( result )
                return result;
        }
    }
    
    return 0;
}






