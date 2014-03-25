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


static DKTypeRef DKHashTableInitialize( DKTypeRef ref );
static void      DKHashTableFinalize( DKTypeRef ref );

static void DKImmutableHashTableInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy );
static void DKImmutableHashTableRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key );
static void DKImmutableHashTableRemoveAllObjects( DKMutableDictionaryRef ref );

///
//  DKHashTableClass()
//
DKTypeRef DKHashTableClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKHashTable", DKObjectClass(), sizeof(struct DKHashTable) );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
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
        dictionary->applyFunction = DKHashTableApplyFunction;
        dictionary->insertObject = DKImmutableHashTableInsertObject;
        dictionary->removeObject = DKImmutableHashTableRemoveObject;
        dictionary->removeAllObjects = DKImmutableHashTableRemoveAllObjects;

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
        SharedClassObject = DKCreateClass( "DKMutableHashTable", DKHashTableClass(), sizeof(struct DKHashTable) );
        
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
        dictionary->applyFunction = DKHashTableApplyFunction;
        dictionary->insertObject = DKHashTableInsertObject;
        dictionary->removeObject = DKHashTableRemoveObject;
        dictionary->removeAllObjects = DKHashTableRemoveAllObjects;

        DKInstallInterface( SharedClassObject, dictionary );
        DKRelease( dictionary );
    }
    
    return SharedClassObject;
}


///
//  DKHashTableInitialize()
//
static DKTypeRef DKHashTableInitialize( DKTypeRef ref )
{
    struct DKHashTable * hashTable = (struct DKHashTable *)ref;

    hashTable->rows = NULL;
    hashTable->count = 0;
    hashTable->maxCount = 0;
    
    return ref;
}


///
//  DKHashTableFinalize()
//
static void DKHashTableFinalize( DKTypeRef ref )
{
    struct DKHashTable * hashTable = (struct DKHashTable *)ref;
    DKHashTableRemoveAllObjects( hashTable );
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
    struct DKHashTable * hashTable = (struct DKHashTable *)DKCreate( DKHashTableClass() );
    
    if( hashTable )
    {
        for( DKIndex i = 0; i < count; ++i )
        {
            DKHashIndex hash = DKHash( keys[i] );
            Insert( hashTable, hash, keys[i], objects[i], DKDictionaryInsertAlways );
        }
    }
    
    return hashTable;
}


///
//  DKHashTableCreateWithKeysAndObjects()
//
DKDictionaryRef DKHashTableCreateWithKeysAndObjects( DKTypeRef firstKey, ... )
{
    struct DKHashTable * hashTable = (struct DKHashTable *)DKCreate( DKHashTableClass() );
    
    if( hashTable )
    {
        va_list arg_ptr;
        va_start( arg_ptr, firstKey );

        for( DKTypeRef key = firstKey; key != NULL; )
        {
            DKTypeRef object = va_arg( arg_ptr, DKTypeRef );

            DKHashIndex hash = DKHash( key );
            Insert( hashTable, hash, key, object, DKDictionaryInsertAlways );
            
            key = va_arg( arg_ptr, DKTypeRef );
        }

        va_end( arg_ptr );
    }
    
    return hashTable;
}


///
//  DKHashTableCreateCopy()
//
DKDictionaryRef DKHashTableCreateCopy( DKDictionaryRef srcDictionary )
{
    DKMutableDictionaryRef ref = DKHashTableCreateMutableCopy( srcDictionary );

    // Turn the object into a regular hash table
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    DKRelease( obj->isa );
    obj->isa = DKRetain( DKHashTableClass() );
    
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
//  DKHashTableApplyFunction()
//
int DKHashTableApplyFunction( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context )
{
    return 0;
}


///
//  DKHashTableInsertObject()
//
static void DKImmutableHashTableInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
    DKError( "DKHashTableInsertObject: Trying to modify an immutable object." );
}

void DKHashTableInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
    if( ref )
    {
        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        DKHashIndex hash = DKHash( key );

        Insert( hashTable, hash, key, object, policy );
    }
}


///
//  DKHashTableRemoveObject()
//
static void DKImmutableHashTableRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key )
{
    DKError( "DKHashTableRemoveObject: Trying to modify an immutable object." );
}

void DKHashTableRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key )
{
    if( ref )
    {
        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        DKHashIndex hash = DKHash( key );

        Erase( hashTable, hash, key );
    }
}


///
//  DKHashTableRemoveAllObjects()
//
static void DKImmutableHashTableRemoveAllObjects( DKMutableDictionaryRef ref )
{
    DKError( "DKHashTableRemoveAllObjects: Trying to modify an immutable object." );
}

void DKHashTableRemoveAllObjects( DKMutableDictionaryRef ref )
{
    if( ref )
    {
        //struct DKHashTable * hashTable = (struct DKHashTable *)ref;
    }
}









