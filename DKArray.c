//
//  DKArray.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#include "DKArray.h"
#include "DKElementArray.h"
#include "DKLifeCycle.h"
#include "DKCopying.h"


struct DKArray
{
    DKObjectHeader _obj;
    DKElementArray elemArray;
};


static DKTypeRef    DKArrayAllocate( void );
static DKTypeRef    DKMutableArrayAllocate( void );
static DKTypeRef    DKArrayInitialize( DKTypeRef ref );
static void         DKArrayFinalize( DKTypeRef ref );
static DKTypeRef    DKArrayCopy( DKTypeRef ref );
static DKTypeRef    DKMutableArrayCopy( DKTypeRef ref );
static DKTypeRef    DKArrayMutableCopy( DKTypeRef ref );

///
//  DKArrayClass()
//
DKTypeRef DKArrayClass( void )
{
    static DKTypeRef arrayClass = NULL;

    if( !arrayClass )
    {
        arrayClass = DKAllocClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKArrayAllocate;
        lifeCycle->initialize = DKArrayInitialize;
        lifeCycle->finalize = DKArrayFinalize;

        DKInstallInterface( arrayClass, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKArrayCopy;
        copying->mutableCopy = DKArrayMutableCopy;
        
        DKInstallInterface( arrayClass, copying );
        DKRelease( copying );

        // List
        struct DKList * list = (struct DKList *)DKAllocInterface( DKSelector(List), sizeof(DKList) );
        list->getCount = DKArrayGetCount;
        list->getObjects = DKArrayGetObjects;
        list->replaceObjects = DKArrayReplaceObjects;
        list->replaceObjectsWithList = DKArrayReplaceObjectsWithList;

        DKInstallInterface( arrayClass, list );
        DKRelease( list );
    }
    
    return arrayClass;
}


///
//  DKMutableArrayClass()
//
DKTypeRef DKMutableArrayClass( void )
{
    static DKTypeRef mutableArrayClass = NULL;

    if( !mutableArrayClass )
    {
        mutableArrayClass = DKAllocClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKMutableArrayAllocate;
        lifeCycle->initialize = DKArrayInitialize;
        lifeCycle->finalize = DKArrayFinalize;

        DKInstallInterface( mutableArrayClass, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKMutableArrayCopy;
        copying->mutableCopy = DKArrayMutableCopy;
        
        DKInstallInterface( mutableArrayClass, copying );
        DKRelease( copying );

        // List
        struct DKList * list = (struct DKList *)DKAllocInterface( DKSelector(List), sizeof(DKList) );
        list->getCount = DKArrayGetCount;
        list->getObjects = DKArrayGetObjects;
        list->replaceObjects = DKArrayReplaceObjects;
        list->replaceObjectsWithList = DKArrayReplaceObjectsWithList;

        DKInstallInterface( mutableArrayClass, list );
        DKRelease( list );
    }
    
    return mutableArrayClass;
}


///
//  DKArrayAllocate()
//
static DKTypeRef DKArrayAllocate( void )
{
    return DKAllocObject( DKArrayClass(), sizeof(struct DKArray), 0 );
}


///
//  DKMutableArrayAllocate()
//
static DKTypeRef DKMutableArrayAllocate( void )
{
    return DKAllocObject( DKMutableArrayClass(), sizeof(struct DKArray), DKObjectIsMutable );
}


///
//  DKArrayInitialize()
//
static DKTypeRef DKArrayInitialize( DKTypeRef ref )
{
    ref = DKObjectInitialize( ref );
    
    if( ref )
    {
        struct DKArray * array = (struct DKArray *)ref;
        DKElementArrayInit( &array->elemArray, sizeof(DKTypeRef) );
    }
    
    return ref;
}


///
//  DKArrayFinalize()
//
static void DKArrayFinalize( DKTypeRef ref )
{
    if( ref )
    {
        struct DKArray * array = (struct DKArray *)ref;

        DKIndex count = DKElementArrayGetCount( &array->elemArray );

        if( !DKTestObjectAttribute( ref, DKObjectContentIsExternal ) )
        {
            for( DKIndex i = 0; i < count; ++i )
            {
                DKTypeRef elem = DKElementArrayGetElementAtIndex( &array->elemArray, i, DKTypeRef );
                DKRelease( elem );
            }
            
            DKElementArrayClear( &array->elemArray );
        }
    }
}


///
//  DKArrayCopy()
//
static DKTypeRef DKArrayCopy( DKTypeRef ref )
{
    return DKRetain( ref );
}


///
//  DKMutableArrayCopy()
//
static DKTypeRef DKMutableArrayCopy( DKTypeRef ref )
{
    return DKArrayCreateCopy( ref );
}


///
//  DKArrayMutableCopy()
//
static DKTypeRef DKArrayMutableCopy( DKTypeRef ref )
{
    return DKArrayCreateMutableCopy( ref );
}




// Internals =============================================================================

///
//  DKArrayReplaceObjectsInternal()
//
static void DKArrayReplaceObjectsInternal( struct DKArray * array, DKRange range, DKTypeRef objects[], DKIndex count )
{
    DKAssert( range.location >= 0 );
    DKAssert( DKRangeEnd( range ) <= DKElementArrayGetCount( &array->elemArray ) );
    
    // Retain the incoming objects
    for( DKIndex i = 0; i < count; ++i )
    {
        DKRetain( objects[i] );
    }
    
    // Release the objects we're replacing
    for( DKIndex i = 0; i < range.length; ++i )
    {
        DKTypeRef elem = DKElementArrayGetElementAtIndex( &array->elemArray, range.location + i, DKTypeRef );
        DKRelease( elem );
    }
    
    // Copy the objects into the array
    DKElementArrayReplaceElements( &array->elemArray, range, objects, count );
}


///
//  DKArrayReplaceObjectsWithListInternal()
//
static void DKArrayReplaceObjectsWithListInternal( struct DKArray * array, DKRange range, DKTypeRef srcList )
{
    DKList * srcListInterface = DKLookupInterface( srcList, DKSelector(List) );
    
    DKIndex srcCount = srcListInterface->getCount( srcList );
    
    if( srcCount <= 128 )
    {
        DKTypeRef buffer[128];
        srcListInterface->getObjects( srcList, DKRangeMake( 0, srcCount ), buffer );
        DKArrayReplaceObjectsInternal( array, range, buffer, srcCount );
    }
    
    else
    {
        DKTypeRef * buffer = DKAlloc( sizeof(DKTypeRef) * srcCount );
        srcListInterface->getObjects( srcList, DKRangeMake( 0, srcCount ), buffer );
        DKArrayReplaceObjectsInternal( array, range, buffer, srcCount );
        DKFree( buffer );
    }
}




// DKArray Interface =====================================================================

///
//  DKArrayCreate()
//
DKListRef DKArrayCreate( DKTypeRef objects[], DKIndex count )
{
    struct DKArray * array = (struct DKArray *)DKCreate( DKArrayClass() );
    
    DKArrayReplaceObjectsInternal( array, DKRangeMake( 0, 0 ), objects, count );
    
    return array;
}


///
//  DKArrayCreateNoCopy()
//
DKListRef DKArrayCreateNoCopy( DKTypeRef objects[], DKIndex count )
{
    struct DKArray * array = (struct DKArray *)DKAllocObject( DKArrayClass(), sizeof(struct DKArray), DKObjectContentIsExternal );

    DKElementArrayInit( &array->elemArray, sizeof(DKTypeRef) );
    array->elemArray.byteArray.data = (uint8_t *)objects;
    array->elemArray.byteArray.length = sizeof(DKTypeRef) * count;
    array->elemArray.byteArray.maxLength = sizeof(DKTypeRef) * count;
    
    return array;
}


///
//  DKArrayCreateCopy()
//
DKListRef DKArrayCreateCopy( DKListRef srcList )
{
    struct DKArray * array = (struct DKArray *)DKCreate( DKArrayClass() );
    
    DKArrayReplaceObjectsWithListInternal( array, DKRangeMake( 0, 0 ), srcList );
    
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
    
    DKArrayReplaceObjectsWithListInternal( array, DKRangeMake( 0, 0 ), srcList );
    
    return array;
}


///
//  DKArrayGetCount()
//
DKIndex DKArrayGetCount( DKListRef ref )
{
    if( ref )
    {
        const struct DKArray * array = ref;
        return DKElementArrayGetCount( &array->elemArray );
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
        
        for( DKIndex i = 0; i < range.length; ++i )
        {
            objects[i] = DKElementArrayGetElementAtIndex( &array->elemArray, range.location + i, DKTypeRef );
        }
    }
    
    return 0;
}


///
//  DKArrayReplaceObjects()
//
void DKArrayReplaceObjects( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKArrayReplaceObjects: Trying to modify an immutable object." );
            return;
        }

        struct DKArray * array = (struct DKArray *)ref;
        DKArrayReplaceObjectsInternal( array, range, objects, count );
    }
}


///
//  DKArrayReplaceObjectsWithList()
//
void DKArrayReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKLinkedListReplaceObjectsWithList: Trying to modify an immutable object." );
            return;
        }

        struct DKArray * array = (struct DKArray *)ref;
        DKArrayReplaceObjectsWithListInternal( array, range, srcList );
    }
}




