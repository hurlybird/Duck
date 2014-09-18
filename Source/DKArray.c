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


struct DKArray
{
    DKObject _obj;
    DKGenericArray ptrArray;
};


static DKObjectRef DKArrayInitialize( DKObjectRef _self );
static void        DKArrayFinalize( DKObjectRef _self );


///
//  DKArrayClass()
//
DKThreadSafeClassInit( DKArrayClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKArray" ), DKObjectClass(), sizeof(struct DKArray), 0, DKArrayInitialize, DKArrayFinalize );
    
    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKArrayMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Description
    struct DKDescriptionInterface * description = DKAllocInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->copyDescription = (DKCopyDescriptionMethod)DKCollectionCopyDescription;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Collection
    struct DKCollectionInterface * collection = DKAllocInterface( DKSelector(Collection), sizeof(struct DKCollectionInterface) );
    collection->getCount = (DKGetCountMethod)DKArrayGetCount;
    collection->containsObject = (DKContainsMethod)DKListContainsObject;
    collection->foreachObject = (DKForeachObjectMethod)DKArrayApplyFunction;
    
    DKInstallInterface( cls, collection );
    DKRelease( collection );

    // List
    struct DKListInterface * list = DKAllocInterface( DKSelector(List), sizeof(struct DKListInterface) );
    list->createWithVAObjects = (DKListCreateWithVAObjectsMethod)DKArrayCreateWithVAObjects;
    list->createWithCArray = (DKListCreateWithCArrayMethod)DKArrayCreateWithCArray;
    list->createWithCollection = (DKListCreateWithCollectionMethod)DKArrayCreateWithCollection;
    
    list->getCount = (DKGetCountMethod)DKArrayGetCount;
    list->getObjectAtIndex = (DKListGetObjectAtIndexMethod)DKArrayGetObjectAtIndex;
    list->getObjectsInRange = (DKListGetObjectsInRangeMethod)DKArrayGetObjectsInRange;
    
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
    set->createWithVAObjects = DKListCreateSetWithVAObjects;
    set->createWithCArray = DKListCreateSetWithCArray;
    set->createWithCollection = DKListCreateSetWithCollection;

    set->getCount = (DKGetCountMethod)DKArrayGetCount;
    set->getMember = DKListGetMemberOfSet;
    
    set->addObject = DKListAddObjectToSet;
    set->removeObject = DKListRemoveObject;
    set->removeAllObjects = DKListRemoveAllObjects;
    
    DKInstallInterface( cls, set );
    DKRelease( set );

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
    list->createWithVAObjects = (DKListCreateWithVAObjectsMethod)DKArrayCreateWithVAObjects;
    list->createWithCArray = (DKListCreateWithCArrayMethod)DKArrayCreateWithCArray;
    list->createWithCollection = (DKListCreateWithCollectionMethod)DKArrayCreateWithCollection;
    
    list->getCount = (DKGetCountMethod)DKArrayGetCount;
    list->getObjectAtIndex = (DKListGetObjectAtIndexMethod)DKArrayGetObjectAtIndex;
    list->getObjectsInRange = (DKListGetObjectsInRangeMethod)DKArrayGetObjectsInRange;

    list->appendCArray = (DKListAppendCArrayMethod)DKArrayAppendCArray;
    list->appendCollection = (DKListAppendCollectionMethod)DKArrayAppendCollection;
    list->replaceRangeWithCArray = (DKListReplaceRangeWithCArrayMethod)DKArrayReplaceRangeWithCArray;
    list->replaceRangeWithCollection = (DKListReplaceRangeWithCollectionMethod)DKArrayReplaceRangeWithCollection;
    list->sort = (DKListSortMethod)DKArraySort;
    list->shuffle = (DKListShuffleMethod)DKArrayShuffle;

    DKInstallInterface( cls, list );
    DKRelease( list );
    
    return cls;
}




// Internals =============================================================================

///
//  ReplaceRangeWithCArray()
//
static void ReplaceRangeWithCArray( struct DKArray * array, DKRange range, DKObjectRef objects[], DKIndex count )
{
    DKCheckRange( range, array->ptrArray.length );
    
    // Retain the incoming objects
    for( DKIndex i = 0; i < count; ++i )
    {
        DKRetain( objects[i] );
    }
    
    // Release the objects we're replacing
    for( DKIndex i = 0; i < range.length; ++i )
    {
        DKObjectRef obj = DKGenericArrayGetElementAtIndex( &array->ptrArray, range.location + i, DKObjectRef );
        DKRelease( obj );
    }
    
    // Copy the objects into the array
    DKGenericArrayReplaceElements( &array->ptrArray, range, objects, count );
}


///
//  ReplaceRangeWithCollection()
//
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

static void ReplaceRangeWithCollection( struct DKArray * array, DKRange range, DKObjectRef srcCollection )
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
        ReplaceRangeWithCArray( array, range, NULL, 0 );
    }
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
        struct DKArray * array = (struct DKArray *)_self;
        DKGenericArrayInit( &array->ptrArray, sizeof(DKObjectRef) );
    }
    
    return _self;
}


///
//  DKArrayFinalize()
//
static void DKArrayFinalize( DKObjectRef _self )
{
    struct DKArray * array = (struct DKArray *)_self;

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
//  DKArrayCreateWithVAObjects()
//
DKObjectRef DKArrayCreateWithVAObjects( DKClassRef _class, va_list objects )
{
    DKAssert( (_class == NULL) || DKIsSubclass( _class, DKArrayClass() ) );

    if( _class == NULL )
        _class = DKArrayClass();

    struct DKArray * array = DKCreate( _class );

    if( array )
    {
        DKObjectRef object;
        
        while( (object = va_arg( objects, DKObjectRef )) != NULL )
        {
            ReplaceRangeWithCArray( array, DKRangeMake( array->ptrArray.length, 0 ), &object, 1 );
        }
    }

    return array;
}


///
//  DKArrayCreateWithCArray()
//
DKObjectRef DKArrayCreateWithCArray( DKClassRef _class, DKObjectRef objects[], DKIndex count )
{
    DKAssert( (_class == NULL) || DKIsSubclass( _class, DKArrayClass() ) );

    if( _class == NULL )
        _class = DKArrayClass();

    struct DKArray * array = DKCreate( _class );

    if( array )
    {
        ReplaceRangeWithCArray( array, DKRangeMake( 0, 0 ), objects, count );
    }

    return array;
}


///
//  DKArrayCreateWithCArrayNoCopy()
//
DKObjectRef DKArrayCreateWithCArrayNoCopy( /* DKClassRef _class, */ DKObjectRef objects[], DKIndex count )
{
    struct DKArray * array = DKCreate( DKArrayClass() );

    if( array )
    {
        DKGenericArrayInitWithExternalStorage( &array->ptrArray, objects, sizeof(DKObjectRef), count );
    }
    
    return array;
}


///
//  DKArrayCreateWithCollection()
//
DKObjectRef DKArrayCreateWithCollection( DKClassRef _class, DKObjectRef collection )
{
    DKAssert( (_class == NULL) || DKIsSubclass( _class, DKArrayClass() ) );

    if( _class == NULL )
        _class = DKArrayClass();

    struct DKArray * array = DKCreate( _class );

    if( array )
    {
        ReplaceRangeWithCollection( array, DKRangeMake( 0, 0 ), collection );
    }

    return array;
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
    return (DKMutableArrayRef)DKArrayCreateWithCollection( DKMutableArrayClass(), _self );
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


///
//  DKArrayGetObjects()
//
DKIndex DKArrayGetObjects( DKArrayRef _self, DKObjectRef objects[] )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );

        memcpy( objects, _self->ptrArray.elements, sizeof(DKObjectRef) * _self->ptrArray.length );
    }
    
    return 0;
}


///
//  DKArrayGetObjectAtIndex()
//
DKObjectRef DKArrayGetObjectAtIndex( DKArrayRef _self, DKIndex index )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        DKCheckIndex( index, _self->ptrArray.length, 0 );

        return DKGenericArrayGetElementAtIndex( (DKGenericArray *)&_self->ptrArray, index, DKObjectRef );
    }
    
    return 0;
}


///
//  DKArrayGetObjectsInRange()
//
DKIndex DKArrayGetObjectsInRange( DKArrayRef _self, DKRange range, DKObjectRef objects[] )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        DKCheckRange( range, _self->ptrArray.length, 0 );

        const DKObjectRef * src = DKGenericArrayGetPointerToElementAtIndex( (DKGenericArray *)&_self->ptrArray, range.location );
        memcpy( objects, src, sizeof(DKObjectRef) * range.length );
    }
    
    return 0;
}


///
//  DKArrayAppendCArray()
//
void DKArrayAppendCArray( DKMutableArrayRef _self, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
        ReplaceRangeWithCArray( _self, DKRangeMake( _self->ptrArray.length, 0 ), objects, count );
    }
}


///
//  DKArrayAppendCollection()
//
void DKArrayAppendCollection( DKMutableArrayRef _self, DKObjectRef srcCollection )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
        ReplaceRangeWithCollection( _self, DKRangeMake( _self->ptrArray.length, 0 ), srcCollection );
    }
}


///
//  DKArrayReplaceRangeWithCArray()
//
void DKArrayReplaceRangeWithCArray( DKMutableArrayRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
        ReplaceRangeWithCArray( _self, range, objects, count );
    }
}


///
//  DKArrayReplaceRangeWithCollection()
//
void DKArrayReplaceRangeWithCollection( DKMutableArrayRef _self, DKRange range, DKObjectRef srcCollection )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
        ReplaceRangeWithCollection( _self, range, srcCollection );
    }
}


///
//  DKArraySort()
//
void DKArraySort( DKMutableArrayRef _self, DKCompareFunction cmp )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
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
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
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






