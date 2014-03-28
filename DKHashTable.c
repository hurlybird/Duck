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
    DKHashCode  hash;
    DKTypeRef   key;
    DKTypeRef   object;
};

struct DKHashTable
{
    DKObjectHeader _obj;
    
    struct DKHashTableRow * rows;
    DKIndex count;
    DKIndex capacity;
    DKIndex rowCount;
};


#define MIN_HASHTABLE_SIZE  11
#define DELETED_KEY         ((void *)-1)


static DKTypeRef DKHashTableInitialize( DKTypeRef ref );
static void      DKHashTableFinalize( DKTypeRef ref );

static void Insert( struct DKHashTable * hashTable, DKHashCode hash, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy );
static void RemoveAll( struct DKHashTable * hashTable );

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

    hashTable->count = 0;
    hashTable->rowCount = MIN_HASHTABLE_SIZE;
    hashTable->capacity = hashTable->rowCount / 2;
    hashTable->rows = dk_malloc( sizeof(struct DKHashTableRow) * MIN_HASHTABLE_SIZE );

    memset( hashTable->rows, 0, sizeof(struct DKHashTableRow) * MIN_HASHTABLE_SIZE );
    
    return ref;
}


///
//  DKHashTableFinalize()
//
static void DKHashTableFinalize( DKTypeRef ref )
{
    struct DKHashTable * hashTable = (struct DKHashTable *)ref;
    RemoveAll( hashTable );
}




// Internals =============================================================================

struct HashTableSize
{
    DKIndex rowCount; // prime
    DKIndex capacity; // rowCount / 2
};

static const struct HashTableSize HashTableSizes[] =
{
    { 11, 5 },
    { 23, 11 },
    { 47, 23 },
    { 97, 48 },
    { 197, 98 },
    { 397, 198 },
    { 797, 398 },
    { 1597, 798 },
    { 3203, 1601 },
    { 6421, 3210 },
    { 12853, 6426 },
    { 25717, 12858 },
    { 51437, 25718 },
    { 102877, 51438 },
    { 205759, 102879 },
    { 411527, 205763 },
    { 823117, 411558 },
    { 1646237, 823118 },
    { 3292489, 1646244 },
    { 6584983, 3292491 },
    { 13169977, 6584988 },
    { 26339969, 13169984 },
    { 52679969, 26339984 },
    { 105359939, 52679969 },
    { 210719881, 105359940 },
    { 421439783, 210719891 },
    { 842879579, 421439789 },
    { 1685759167, 842879583 },
    
    // This could be larger on 64-bit architectures...
    
    { 0, 0 }
};

// Size table generation for load < 0.5
#if 0
static bool IsPrime( int64_t x )
{
    for( int64_t i = 3; (i * i) < x; i += 2 )
    {
        if( (x % i) == 0 )
            return false;
    }
    
    return true;
}

static int64_t NextPrime( int64_t x )
{
    if( (x & 1) == 0 )
        x++;
    
    for( ; !IsPrime( x ); x += 2 )
        ;
    
    return x;
}

static void GenerateHashTableSizes( void )
{
    int64_t max = 0x7fffffff;

    for( int64_t i = 11; i <= max; )
    {
        printf( "    { %lld, %lld },\n", i, i / 2 );
        i = NextPrime( i * 2 );
    }
}
#endif


///
//  NextHashTableSize()
//
static struct HashTableSize NextHashTableSize( DKIndex rowCount )
{
    for( int i = 0; HashTableSizes[i].rowCount != 0; i++ )
    {
        if( HashTableSizes[i].rowCount > rowCount )
            return HashTableSizes[i];
    }
    
    DKFatalError( "DKHashTable: Exceeded maximum table size (~843 million entries).\n" );

    struct HashTableSize zero = { 0, 0 };
    return zero;
}


///
//  RowIsActive()
//
static bool RowIsActive( struct DKHashTableRow * row )
{
    return (row->key != NULL) && (row->key != DELETED_KEY);
}


///
//  RowIsEmpty()
//
static bool RowIsEmpty( struct DKHashTableRow * row )
{
    return row->key == NULL;
}


///
//  ResizeAndRehash()
//
static void ResizeAndRehash( struct DKHashTable * hashTable )
{
    if( hashTable->count < hashTable->capacity )
        return;
    
    struct DKHashTableRow * oldRows = hashTable->rows;
    DKIndex oldRowCount = hashTable->rowCount;
    
    struct HashTableSize newSize = NextHashTableSize( hashTable->rowCount );
    
    hashTable->rowCount = newSize.rowCount;
    hashTable->capacity = newSize.capacity;
    hashTable->count = 0;
    hashTable->rows = dk_malloc( sizeof(struct DKHashTableRow) * hashTable->rowCount );
    
    memset( hashTable->rows, 0, sizeof(struct DKHashTableRow) * hashTable->rowCount );
    
    for( DKIndex i = 0; i < oldRowCount; ++i )
    {
        struct DKHashTableRow * row = &oldRows[i];
        
        if( RowIsActive( row ) )
        {
            Insert( hashTable, row->hash, row->key, row->object, DKDictionaryInsertAlways );
        
            DKRelease( row->key );
            DKRelease( row->object );
        }
    }
    
    dk_free( oldRows );
}


///
//  Find()
//
static struct DKHashTableRow * Find( struct DKHashTable * hashTable, DKHashCode hash, DKTypeRef key )
{
    DKIndex i = 0;
    DKIndex x = hash % hashTable->rowCount;
    
    while( 1 )
    {
        struct DKHashTableRow * row = &hashTable->rows[x];

        if( RowIsEmpty( row ) )
            return row;
           
        if( row->hash == hash )
        {
            if( DKEqual( row->key, key ) )
                return row;
        }
        
        // Quadratic probing
        i++;
        x += (2 * i) - 1;
        
        if( x >= hashTable->rowCount )
            x -= hashTable->rowCount;
    }
        
    DKAssert( 0 );
    return NULL;
}


///
//  Insert()
//
static void Insert( struct DKHashTable * hashTable, DKHashCode hash, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
    // It should be impossible for a key object to exist at the max addressable byte in
    // memory... but lets's not be surprised.
    DKAssert( key != DELETED_KEY );

    struct DKHashTableRow * row = Find( hashTable, hash, key );
    bool active = RowIsActive( row );

    if( !active && (policy == DKDictionaryInsertIfFound) )
        return;
    
    if( active && (policy == DKDictionaryInsertIfNotFound) )
        return;

    row->hash = hash;

    if( active )
    {
        DKRetain( key );
        DKRelease( row->key );
        row->key = key;

        DKRetain( object );
        DKRelease( row->object );
        row->object = object;
    }
    
    else
    {
        row->key = DKRetain( key );
        row->object = DKRetain( object );

        hashTable->count++;
    }
    
    ResizeAndRehash( hashTable );
}


///
//  Remove()
//
static void Remove( struct DKHashTable * hashTable, DKHashCode hash, DKTypeRef key )
{
    struct DKHashTableRow * row = Find( hashTable, hash, key );
    
    if( RowIsActive( row ) )
    {
        DKAssert( row->hash == hash );
        DKAssert( DKEqual( row->key, key ) );
    
        DKRelease( row->key );
        row->key = DELETED_KEY;
        
        DKRelease( row->object );
        row->object = NULL;
        
        hashTable->count--;
    }
}


///
//  RemoveAll()
//
static void RemoveAll( struct DKHashTable * hashTable )
{
    for( DKIndex i = 0; i < hashTable->count; ++i )
    {
        struct DKHashTableRow * row = &hashTable->rows[i];
        
        if( RowIsActive( row ) )
        {
            DKRelease( row->key );
            DKRelease( row->object );
        }
        
        row->hash = 0;
        row->key = NULL;
        row->object = NULL;
    }
    
    hashTable->count = 0;
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
            DKHashCode hash = DKHash( keys[i] );
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

            DKHashCode hash = DKHash( key );
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
        DKVerifyKindOfClass( ref, DKHashTableClass(), 0 );
        
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
        DKVerifyKindOfClass( ref, DKHashTableClass(), NULL );

        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        DKHashCode hash = DKHash( key );
    
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
    int result = 0;
    
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKHashTableClass(), 0 );

        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        
        for( DKIndex i = 0; i < hashTable->count; ++i )
        {
            struct DKHashTableRow * row = &hashTable->rows[i];
            
            if( row->key )
            {
                if( (result = callback( context, row->key, row->object )) != 0 )
                    break;
            }
        }
    }
    
    return result;
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
        DKVerifyKindOfClass( ref, DKMutableHashTableClass() );

        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        DKHashCode hash = DKHash( key );

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
        DKVerifyKindOfClass( ref, DKMutableHashTableClass() );

        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        DKHashCode hash = DKHash( key );

        Remove( hashTable, hash, key );
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
        DKVerifyKindOfClass( ref, DKMutableHashTableClass() );

        struct DKHashTable * hashTable = (struct DKHashTable *)ref;
        RemoveAll( hashTable );
    }
}









