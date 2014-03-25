//
//  DKHashTable.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-24.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKHashTable.h"
#include "DKCopying.h"


struct DKHashTableRow
{
    DKHashIndex hash;
    DKTypeRef   key;
    DKTypeRef   object;
};

struct DKHashTable
{
    DKObjectHeader _obj;
    
    struct DKHashTableRow * rows;
    DKIndex count;
    DKIndex maxCount;
};



static DKTypeRef    DKHashTableAllocate( void );
static DKTypeRef    DKMutableHashTableAllocate( void );
static DKTypeRef    DKHashTableInitialize( DKTypeRef ref );
static void         DKHashTableFinalize( DKTypeRef ref );


///
//  DKHashTableClass()
//
DKTypeRef DKHashTableClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKHashTableAllocate;
        lifeCycle->initialize = DKHashTableInitialize;
        lifeCycle->finalize = DKHashTableFinalize;

        DKInstallInterface( SharedClassObject, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKRetain;
        copying->mutableCopy = DKHashTableCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );
        
        // Dictionary
        struct DKDictionary * dictionary = (struct DKDictionary *)DKCreateInterface( DKSelector(Dictionary), sizeof(DKDictionary) );
        dictionary->getCount = DKHashTableGetCount;
        dictionary->getObject = DKHashTableGetObject;
        dictionary->insertObject = DKHashTableInsertObject;
        dictionary->removeObject = DKHashTableRemoveObject;
        dictionary->removeAllObjects = DKHashTableRemoveAllObjects;
        dictionary->applyFunction = DKHashTableApplyFunction;

        DKInstallInterface( SharedClassObject, dictionary );
        DKRelease( dictionary );
    }
    
    return SharedClassObject;
}


///
//  DKMutableHashTableClass()
//
DKTypeRef DKMutableHashTableClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKMutableHashTableAllocate;
        lifeCycle->initialize = DKHashTableInitialize;
        lifeCycle->finalize = DKHashTableFinalize;

        DKInstallInterface( SharedClassObject, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKHashTableCreateMutableCopy;
        copying->mutableCopy = DKHashTableCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );

        // Dictionary
        struct DKDictionary * dictionary = (struct DKDictionary *)DKCreateInterface( DKSelector(Dictionary), sizeof(DKDictionary) );
        dictionary->getCount = DKHashTableGetCount;
        dictionary->getObject = DKHashTableGetObject;
        dictionary->insertObject = DKHashTableInsertObject;
        dictionary->removeObject = DKHashTableRemoveObject;
        dictionary->removeAllObjects = DKHashTableRemoveAllObjects;
        dictionary->applyFunction = DKHashTableApplyFunction;

        DKInstallInterface( SharedClassObject, dictionary );
        DKRelease( dictionary );
    }
    
    return SharedClassObject;
}


///
//  DKHashTableAllocate()
//
static DKTypeRef DKHashTableAllocate( void )
{
    return DKAllocObject( DKHashTableClass(), sizeof(struct DKHashTable), 0 );
}


///
//  DKMutableHashTableAllocate()
//
static DKTypeRef DKMutableHashTableAllocate( void )
{
    return DKAllocObject( DKMutableHashTableClass(), sizeof(struct DKHashTable), DKObjectIsMutable );
}


///
//  DKHashTableInitialize()
//
static DKTypeRef DKHashTableInitialize( DKTypeRef ref )
{
    ref = DKObjectInitialize( ref );
    
    if( ref )
    {
        struct DKHashTable * hashTable = (struct DKHashTable *)ref;

        hashTable->rows = NULL;
        hashTable->count = 0;
        hashTable->maxCount = 0;
    }
    
    return ref;
}


///
//  DKHashTableFinalize()
//
static void DKHashTableFinalize( DKTypeRef ref )
{
    if( ref )
    {
        struct DKHashTable * hashTable = (struct DKHashTable *)ref;

        DKHashTableRemoveAllObjects( hashTable );
    }
}




// Internals =============================================================================

///
//  Find()
//
static struct DKHashTableRow * Find( struct DKHashTable * hashTable, DKHashIndex hash, DKTypeRef key )
{
    return NULL;
}


///
//  Insert()
//
static void Insert( struct DKHashTable * hashTable, DKHashIndex hash, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
}


///
//  Erase()
//
static void Erase( struct DKHashTable * hashTable, DKHashIndex hash, DKTypeRef key )
{
}




// Interface =============================================================================

///
//  DKHashTableCreate()
//
DKDictionaryRef DKHashTableCreate( DKTypeRef keys[], DKTypeRef objects[], DKIndex count )
{
    DKMutableDictionaryRef ref = DKHashTableCreateMutable();
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKHashTableInsertObject( ref, keys[i], objects[i], DKDictionaryInsertAlways );
    }
    
    // Turn the mutable tree into an immutable tree
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    obj->isa = DKHashTableClass();
    obj->attributes &= ~DKObjectIsMutable;
    
    return ref;
}


///
//  DKHashTableCreateWithKeysAndObjects()
//
DKDictionaryRef DKHashTableCreateWithKeysAndObjects( DKTypeRef firstKey, ... )
{
    DKMutableDictionaryRef ref = DKHashTableCreateMutable();

    va_list arg_ptr;
    va_start( arg_ptr, firstKey );

    for( DKTypeRef key = firstKey; key != NULL; )
    {
        DKTypeRef object = va_arg( arg_ptr, DKTypeRef );

        DKHashTableInsertObject( ref, key, object, DKDictionaryInsertAlways );
        
        key = va_arg( arg_ptr, DKTypeRef );
    }

    va_end( arg_ptr );
    
    // Turn the mutable tree into an immutable tree
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    obj->isa = DKHashTableClass();
    obj->attributes &= ~DKObjectIsMutable;
    
    return ref;
}


///
//  DKHashTableCreateCopy()
//
DKDictionaryRef DKHashTableCreateCopy( DKDictionaryRef srcDictionary )
{
    DKMutableDictionaryRef ref = DKHashTableCreateMutableCopy( srcDictionary );

    // Turn the mutable tree into an immutable tree
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    obj->isa = DKHashTableClass();
    obj->attributes &= ~DKObjectIsMutable;
    
    return ref;
}


///
//  DKHashTableCreateMutable()
//
DKMutableDictionaryRef DKHashTableCreateMutable( void )
{
    return (DKMutableDictionaryRef)DKCreate( DKMutableHashTableClass() );
}


///
//  DKHashTableCreateMutableCopy()
//
DKMutableDictionaryRef DKHashTableCreateMutableCopy( DKDictionaryRef srcDictionary )
{
    DKMutableDictionaryRef ref = DKHashTableCreateMutable();
    DKDictionaryAddEntriesFromDictionary( ref, srcDictionary );
    
    return ref;
}


///
//  DKHashTableGetCount()
//
DKIndex DKHashTableGetCount( DKDictionaryRef ref )
{
    if( ref )
    {
        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        return hashTable->count;
    }
    
    return 0;
}

///
//  DKHashTableInsertObject()
//
void DKHashTableInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKHashTableInsertObject: Trying to modify an immutable object." );
            return;
        }
        
        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        DKHashIndex hash = DKHash( key );

        Insert( hashTable, hash, key, object, policy );
    }
}


///
//  DKHashTableGetObject()
//
DKTypeRef DKHashTableGetObject( DKDictionaryRef ref, DKTypeRef key )
{
    if( ref )
    {
        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        DKHashIndex hash = DKHash( key );
    
        struct DKHashTableRow * row = Find( hashTable, hash, key );
    
        if( row )
            return row->object;
    }

    return NULL;
}


///
//  DKHashTableRemoveObject()
//
void DKHashTableRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKHashTableRemoveObject: Trying to modify an immutable object." );
            return;
        }
        
        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        DKHashIndex hash = DKHash( key );

        Erase( hashTable, hash, key );
    }
}


///
//  DKHashTableRemoveAllObjects()
//
void DKHashTableRemoveAllObjects( DKMutableDictionaryRef ref )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKHashTableRemoveAllObjects: Trying to modify an immutable object." );
            return;
        }
        
        //struct DKHashTable * hashTable = (struct DKHashTable *)ref;
    }
}


///
//  DKHashTableApplyFunction()
//
int DKHashTableApplyFunction( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context )
{
    return 0;
}








