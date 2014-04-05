//
//  DKArray.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "DKArray.h"
#include "DKPointerArray.h"
#include "DKCopying.h"
#include "DKString.h"


struct DKArray
{
    DKObject _obj;
    DKPointerArray ptrArray;
};


static DKObjectRef DKArrayInitialize( DKObjectRef _self );
static void      DKArrayFinalize( DKObjectRef _self );

static void      DKImmutableArrayReplaceObjects( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
static void      DKImmutableArrayReplaceObjectsWithList( DKMutableListRef _self, DKRange range, DKListRef srcList );
static void      DKImmutableArraySort( DKMutableListRef _self, DKCompareFunction cmp );
static void      DKImmutableArrayShuffle( DKMutableListRef _self );


///
//  DKArrayClass()
//
DKThreadSafeClassInit( DKArrayClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKArray" ), DKObjectClass(), sizeof(struct DKArray), 0 );
    
    // Allocation
    struct DKAllocation * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(DKAllocation) );
    allocation->initialize = DKArrayInitialize;
    allocation->finalize = DKArrayFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKArrayCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // List
    struct DKList * list = DKAllocInterface( DKSelector(List), sizeof(DKList) );
    list->getCount = (DKListGetCountMethod)DKArrayGetCount;
    list->getObjects = (DKListGetObjectsMethod)DKArrayGetObjects;
    list->replaceObjects = DKImmutableArrayReplaceObjects;
    list->replaceObjectsWithList = DKImmutableArrayReplaceObjectsWithList;
    list->sort = DKImmutableArraySort;
    list->shuffle = DKImmutableArrayShuffle;

    DKInstallInterface( cls, list );
    DKRelease( list );
    
    return cls;
}


///
//  DKMutableArrayClass()
//
DKThreadSafeClassInit( DKMutableArrayClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKMutableArray" ), DKArrayClass(), sizeof(struct DKArray), 0 );
    
    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = (DKCopyMethod)DKArrayCreateMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKArrayCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // List
    struct DKList * list = DKAllocInterface( DKSelector(List), sizeof(DKList) );
    list->getCount = (DKListGetCountMethod)DKArrayGetCount;
    list->getObjects = (DKListGetObjectsMethod)DKArrayGetObjects;
    list->replaceObjects = (DKListReplaceObjectsMethod)DKArrayReplaceObjects;
    list->replaceObjectsWithList = (DKListReplaceObjectsWithListMethod)DKArrayReplaceObjectsWithList;
    list->sort = (DKListSortMethod)DKArraySort;
    list->shuffle = (DKListShuffleMethod)DKArrayShuffle;

    DKInstallInterface( cls, list );
    DKRelease( list );
    
    return cls;
}


///
//  DKArrayInitialize()
//
static DKObjectRef DKArrayInitialize( DKObjectRef _self )
{
    struct DKArray * array = (struct DKArray *)_self;
    DKPointerArrayInit( &array->ptrArray );
    
    return _self;
}


///
//  DKArrayFinalize()
//
static void DKArrayFinalize( DKObjectRef _self )
{
    struct DKArray * array = (struct DKArray *)_self;

    if( !DKPointerArrayHasExternalStorage( &array->ptrArray ) )
    {
        DKIndex count = array->ptrArray.length;

        for( DKIndex i = 0; i < count; ++i )
        {
            DKObjectRef elem = array->ptrArray.data[i];
            DKRelease( elem );
        }
    }
    
    DKPointerArrayFinalize( &array->ptrArray );
}




// Internals =============================================================================

///
//  ReplaceObjects()
//
static void ReplaceObjects( struct DKArray * array, DKRange range, DKObjectRef objects[], DKIndex count )
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
        DKObjectRef elem = array->ptrArray.data[range.location + i];
        DKRelease( elem );
    }
    
    // Copy the objects into the array
    DKPointerArrayReplacePointers( &array->ptrArray, range, (const uintptr_t *)objects, count );
}


///
//  ReplaceObjectsWithList()
//
static void ReplaceObjectsWithList( struct DKArray * array, DKRange range, DKListRef srcList )
{
    if( srcList )
    {
        DKCheckRange( range, array->ptrArray.length );

        DKList * srcListInterface = DKGetInterface( srcList, DKSelector(List) );
        
        DKIndex srcCount = srcListInterface->getCount( srcList );
        
        if( srcCount <= 128 )
        {
            DKObjectRef buffer[128];
            srcListInterface->getObjects( srcList, DKRangeMake( 0, srcCount ), buffer );
            ReplaceObjects( array, range, buffer, srcCount );
        }
        
        else
        {
            DKObjectRef * buffer = dk_malloc( sizeof(DKObjectRef) * srcCount );
            srcListInterface->getObjects( srcList, DKRangeMake( 0, srcCount ), buffer );
            ReplaceObjects( array, range, buffer, srcCount );
            dk_free( buffer );
        }
    }
    
    else
    {
        ReplaceObjects( array, range, NULL, 0 );
    }
}




// DKArray Interface =====================================================================

///
//  DKArrayCreate()
//
DKArrayRef DKArrayCreate( void )
{
    return DKAllocObject( DKArrayClass(), 0 );
}


///
//  DKArrayCreateWithObjects()
//
DKArrayRef DKArrayCreateWithObjects( DKObjectRef firstObject, ... )
{
    struct DKArray * array = DKAllocObject( DKArrayClass(), 0 );

    va_list arg_ptr;
    va_start( arg_ptr, firstObject );

    for( DKObjectRef object = firstObject; object != NULL; )
    {
        ReplaceObjects( array, DKRangeMake( array->ptrArray.length, 0 ), &object, 1 );
        
        object = va_arg( arg_ptr, DKObjectRef );
    }

    va_end( arg_ptr );

    return array;
}


///
//  DKArrayCreateWithCArray()
//
DKArrayRef DKArrayCreateWithCArray( DKObjectRef objects[], DKIndex count )
{
    struct DKArray * array = DKAllocObject( DKArrayClass(), 0 );

    ReplaceObjects( array, DKRangeMake( 0, 0 ), objects, count );

    return array;
}


///
//  DKArrayCreateWithCArrayNoCopy()
//
DKArrayRef DKArrayCreateWithCArrayNoCopy( DKObjectRef objects[], DKIndex count )
{
    struct DKArray * array = DKAllocObject( DKArrayClass(), 0 );

    if( array )
    {
        DKPointerArrayInitWithExternalStorage( &array->ptrArray, (void *)objects, count );
    }
    
    return array;
}


///
//  DKArrayCreateCopy()
//
DKArrayRef DKArrayCreateCopy( DKListRef srcList )
{
    struct DKArray * array = DKAllocObject( DKArrayClass(), 0 );
    
    if( array )
    {
        ReplaceObjectsWithList( array, DKRangeMake( 0, 0 ), srcList );
    }
    
    return array;
}


///
//  DKArrayCreateMutable()
//
DKMutableArrayRef DKArrayCreateMutable( void )
{
    return DKAllocObject( DKMutableArrayClass(), 0 );
}


///
//  DKArrayCreateMutableCopy()
//
DKMutableArrayRef DKArrayCreateMutableCopy( DKListRef srcList )
{
    struct DKArray * array = DKAllocObject( DKMutableArrayClass(), 0 );
    
    if( array )
    {
        ReplaceObjectsWithList( array, DKRangeMake( 0, 0 ), srcList );
    }
    
    return array;
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
DKIndex DKArrayGetObjects( DKArrayRef _self, DKRange range, DKObjectRef objects[] )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKArrayClass() );
        DKCheckRange( range, _self->ptrArray.length, 0 );
        
        for( DKIndex i = 0; i < range.length; ++i )
            objects[i] = _self->ptrArray.data[range.location + i];
    }
    
    return 0;
}


///
//  DKArrayReplaceObjects()
//
static void DKImmutableArrayReplaceObjects( DKMutableListRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    DKError( "DKArrayReplaceObjects: Trying to modify an immutable object." );
}

void DKArrayReplaceObjects( DKMutableArrayRef _self, DKRange range, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
        ReplaceObjects( _self, range, objects, count );
    }
}


///
//  DKArrayReplaceObjectsWithList()
//
static void DKImmutableArrayReplaceObjectsWithList( DKMutableListRef _self, DKRange range, DKListRef srcList )
{
    DKError( "DKLinkedListReplaceObjectsWithList: Trying to modify an immutable object." );
}

void DKArrayReplaceObjectsWithList( DKMutableArrayRef _self, DKRange range, DKListRef srcList )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
        ReplaceObjectsWithList( _self, range, srcList );
    }
}


///
//  DKArraySort()
//
static void DKImmutableArraySort( DKMutableListRef _self, DKCompareFunction cmp )
{
    DKError( "DKArraySort: Trying to modify an immutable object." );
}

void DKArraySort( DKMutableArrayRef _self, DKCompareFunction cmp )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
        DKPointerArraySort( &_self->ptrArray, cmp );
    }
}


///
//  DKArrayShuffle()
//
static void DKImmutableArrayShuffle( DKMutableListRef _self )
{
    DKError( "DKArrayShuffle: Trying to modify an immutable object." );
}

void DKArrayShuffle( DKMutableArrayRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableArrayClass() );
        DKPointerArrayShuffle( &_self->ptrArray );
    }
}






