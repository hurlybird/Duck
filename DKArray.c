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


struct DKArray
{
    DKObjectHeader _obj;
    DKPointerArray ptrArray;
};


static DKTypeRef DKArrayInitialize( DKTypeRef ref );
static void      DKArrayFinalize( DKTypeRef ref );

static void      DKImmutableArrayReplaceObjects( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count );
static void      DKImmutableArrayReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList );
static void      DKImmutableArraySort( DKMutableListRef ref, DKCompareFunction cmp );
static void      DKImmutableArrayShuffle( DKMutableListRef ref );


///
//  DKArrayClass()
//
DKTypeRef DKArrayClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKArray", DKObjectClass(), sizeof(struct DKArray) );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->initialize = DKArrayInitialize;
        lifeCycle->finalize = DKArrayFinalize;

        DKInstallInterface( SharedClassObject, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKRetain;
        copying->mutableCopy = DKArrayCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );

        // List
        struct DKList * list = (struct DKList *)DKCreateInterface( DKSelector(List), sizeof(DKList) );
        list->getCount = DKArrayGetCount;
        list->getObjects = DKArrayGetObjects;
        list->replaceObjects = DKImmutableArrayReplaceObjects;
        list->replaceObjectsWithList = DKImmutableArrayReplaceObjectsWithList;
        list->sort = DKImmutableArraySort;
        list->shuffle = DKImmutableArrayShuffle;

        DKInstallInterface( SharedClassObject, list );
        DKRelease( list );
    }
    
    return SharedClassObject;
}


///
//  DKMutableArrayClass()
//
DKTypeRef DKMutableArrayClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKMutableArray", DKArrayClass(), sizeof(struct DKArray) );
        
        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKArrayCreateMutableCopy;
        copying->mutableCopy = DKArrayCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );

        // List
        struct DKList * list = (struct DKList *)DKCreateInterface( DKSelector(List), sizeof(DKList) );
        list->getCount = DKArrayGetCount;
        list->getObjects = DKArrayGetObjects;
        list->replaceObjects = DKArrayReplaceObjects;
        list->replaceObjectsWithList = DKArrayReplaceObjectsWithList;
        list->sort = DKArraySort;
        list->shuffle = DKArrayShuffle;

        DKInstallInterface( SharedClassObject, list );
        DKRelease( list );
    }
    
    return SharedClassObject;
}


///
//  DKArrayInitialize()
//
static DKTypeRef DKArrayInitialize( DKTypeRef ref )
{
    struct DKArray * array = (struct DKArray *)ref;
    DKPointerArrayInit( &array->ptrArray );
    
    return ref;
}


///
//  DKArrayFinalize()
//
static void DKArrayFinalize( DKTypeRef ref )
{
    struct DKArray * array = (struct DKArray *)ref;

    if( !DKPointerArrayHasExternalStorage( &array->ptrArray ) )
    {
        DKIndex count = array->ptrArray.length;

        for( DKIndex i = 0; i < count; ++i )
        {
            DKTypeRef elem = array->ptrArray.data[i];
            DKRelease( elem );
        }
    }
    
    DKPointerArrayFinalize( &array->ptrArray );
}




// Internals =============================================================================

///
//  ReplaceObjects()
//
static void ReplaceObjects( struct DKArray * array, DKRange range, DKTypeRef objects[], DKIndex count )
{
    DKVerifyRange( range, array->ptrArray.length );
    
    // Retain the incoming objects
    for( DKIndex i = 0; i < count; ++i )
    {
        DKRetain( objects[i] );
    }
    
    // Release the objects we're replacing
    for( DKIndex i = 0; i < range.length; ++i )
    {
        DKTypeRef elem = array->ptrArray.data[range.location + i];
        DKRelease( elem );
    }
    
    // Copy the objects into the array
    DKPointerArrayReplacePointers( &array->ptrArray, range, (const uintptr_t *)objects, count );
}


///
//  ReplaceObjectsWithList()
//
static void ReplaceObjectsWithList( struct DKArray * array, DKRange range, DKTypeRef srcList )
{
    if( srcList )
    {
        DKVerifyRange( range, array->ptrArray.length );

        DKList * srcListInterface = DKGetInterface( srcList, DKSelector(List) );
        
        DKIndex srcCount = srcListInterface->getCount( srcList );
        
        if( srcCount <= 128 )
        {
            DKTypeRef buffer[128];
            srcListInterface->getObjects( srcList, DKRangeMake( 0, srcCount ), buffer );
            ReplaceObjects( array, range, buffer, srcCount );
        }
        
        else
        {
            DKTypeRef * buffer = dk_malloc( sizeof(DKTypeRef) * srcCount );
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
DKListRef DKArrayCreate( DKTypeRef objects[], DKIndex count )
{
    struct DKArray * array = (struct DKArray *)DKCreate( DKArrayClass() );
    
    if( array )
    {
        ReplaceObjects( array, DKRangeMake( 0, 0 ), objects, count );
    }
    
    return array;
}


///
//  DKArrayCreateNoCopy()
//
DKListRef DKArrayCreateNoCopy( DKTypeRef objects[], DKIndex count )
{
    struct DKArray * array = (struct DKArray *)DKCreate( DKArrayClass() );

    if( array )
    {
        DKPointerArrayInitWithExternalStorage( &array->ptrArray, (void *)objects, count );
    }
    
    return array;
}


///
//  DKArrayCreateCopy()
//
DKListRef DKArrayCreateCopy( DKListRef srcList )
{
    struct DKArray * array = (struct DKArray *)DKCreate( DKArrayClass() );
    
    if( array )
    {
        ReplaceObjectsWithList( array, DKRangeMake( 0, 0 ), srcList );
    }
    
    return array;
}


///
//  DKArrayCreateMutable()
//
DKMutableListRef DKArrayCreateMutable( void )
{
    return (DKMutableListRef)DKCreate( DKMutableArrayClass() );
}


///
//  DKArrayCreateMutableCopy()
//
DKMutableListRef DKArrayCreateMutableCopy( DKListRef srcList )
{
    struct DKArray * array = (struct DKArray *)DKCreate( DKMutableArrayClass() );
    
    if( array )
    {
        ReplaceObjectsWithList( array, DKRangeMake( 0, 0 ), srcList );
    }
    
    return array;
}


///
//  DKArrayGetCount()
//
DKIndex DKArrayGetCount( DKListRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKArrayClass(), 0 );

        const struct DKArray * array = ref;
        return array->ptrArray.length;
    }
    
    return 0;
}


///
//  DKArrayGetObjects()
//
DKIndex DKArrayGetObjects( DKListRef ref, DKRange range, DKTypeRef objects[] )
{
    if( ref )
    {
        struct DKArray * array = (struct DKArray *)ref;

        DKVerifyKindOfClass( ref, DKArrayClass(), 0 );
        DKVerifyRange( range, array->ptrArray.length, 0 );
        
        for( DKIndex i = 0; i < range.length; ++i )
            objects[i] = array->ptrArray.data[range.location + i];
    }
    
    return 0;
}


///
//  DKArrayReplaceObjects()
//
static void DKImmutableArrayReplaceObjects( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count )
{
    DKError( "DKArrayReplaceObjects: Trying to modify an immutable object." );
}

void DKArrayReplaceObjects( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableArrayClass() );

        struct DKArray * array = (struct DKArray *)ref;
        ReplaceObjects( array, range, objects, count );
    }
}


///
//  DKArrayReplaceObjectsWithList()
//
static void DKImmutableArrayReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList )
{
    DKError( "DKLinkedListReplaceObjectsWithList: Trying to modify an immutable object." );
}

void DKArrayReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableArrayClass() );

        struct DKArray * array = (struct DKArray *)ref;
        ReplaceObjectsWithList( array, range, srcList );
    }
}


///
//  DKArraySort()
//
static void DKImmutableArraySort( DKMutableListRef ref, DKCompareFunction cmp )
{
    DKError( "DKArraySort: Trying to modify an immutable object." );
}

void DKArraySort( DKMutableListRef ref, DKCompareFunction cmp )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableArrayClass() );

        struct DKArray * array = (struct DKArray *)ref;
        qsort( array->ptrArray.data, array->ptrArray.length, sizeof(DKTypeRef), cmp );
    }
}


///
//  DKArrayShuffle()
//
static void DKImmutableArrayShuffle( DKMutableListRef ref )
{
    DKError( "DKArrayShuffle: Trying to modify an immutable object." );
}

void DKArrayShuffle( DKMutableListRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableArrayClass() );
    
        struct DKArray * array = (struct DKArray *)ref;
        dk_shuffle( array->ptrArray.data, array->ptrArray.length );
    }
}






