/*****************************************************************************************

  DKHashTable.c

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

#include "DKHashTable.h"
#include "DKCopying.h"
#include "DKString.h"


struct DKHashTableRow
{
    DKHashCode  hash;
    DKObjectRef key;
    DKObjectRef object;
};

struct DKHashTable
{
    DKObject _obj;
    
    struct DKHashTableRow * rows;
    DKIndex count;
    DKIndex capacity;
    DKIndex rowCount;
};


#define MIN_HASHTABLE_SIZE  11
#define DELETED_KEY         ((void *)-1)


static DKObjectRef DKHashTableInitialize( DKObjectRef _self );
static void        DKHashTableFinalize( DKObjectRef _self );

static DKObjectRef DKHashTableCreateDictionaryWithVAKeysAndObjects( DKClassRef _class, va_list keysAndObjects );
static DKObjectRef DKHashTableCreateSetWithVAObjects( DKClassRef _class, va_list objects );

static void        DKHashTableAddObjectToSet( DKMutableHashTableRef _self, DKObjectRef object );

static void        Insert( struct DKHashTable * hashTable, DKHashCode hash, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );


///
//  DKHashTableClass()
//
DKThreadSafeClassInit( DKHashTableClass )
{
    // Since DKString, DKConstantString, DKHashTable and DKMutableHashTable are all
    // involved in creating constant strings, the names for these classes are
    // initialized in DKRuntimeInit().
    DKClassRef cls = DKAllocClass( NULL, DKObjectClass(), sizeof(struct DKHashTable), 0 );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->initialize = DKHashTableInitialize;
    allocation->finalize = DKHashTableFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (void *)DKHashTableMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Description
    struct DKDescriptionInterface * description = DKAllocInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->copyDescription = (DKCopyDescriptionMethod)DKCollectionCopyDescription;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Collection
    struct DKCollectionInterface * collection = DKAllocInterface( DKSelector(Collection), sizeof(struct DKCollectionInterface) );
    collection->getCount = (DKGetCountMethod)DKDictionaryGetCount;
    collection->foreachObject = (DKForeachObjectMethod)DKHashTableApplyFunctionToObjects;
    collection->foreachKey = (DKForeachObjectMethod)DKHashTableApplyFunctionToKeys;
    collection->foreachKeyAndObject = (DKForeachKeyAndObjectMethod)DKHashTableApplyFunction;
    
    DKInstallInterface( cls, collection );
    DKRelease( collection );

    // Dictionary
    struct DKDictionaryInterface * dictionary = DKAllocInterface( DKSelector(Dictionary), sizeof(struct DKDictionaryInterface) );
    dictionary->createWithVAKeysAndObjects = (DKDictionaryCreateWithVAKeysAndObjectsMethod)DKHashTableCreateDictionaryWithVAKeysAndObjects;
    dictionary->createWithDictionary = (DKDictionaryCreateWithDictionaryMethod)DKHashTableCreateDictionaryWithDictionary;
    
    dictionary->getCount = (DKGetCountMethod)DKHashTableGetCount;
    dictionary->getObject = (DKDictionaryGetObjectMethod)DKHashTableGetObject;
    
    dictionary->insertObject = (void *)DKImmutableObjectAccessError;
    dictionary->removeObject = (void *)DKImmutableObjectAccessError;
    dictionary->removeAllObjects = (void *)DKImmutableObjectAccessError;

    DKInstallInterface( cls, dictionary );
    DKRelease( dictionary );
    
    // Set
    struct DKSetInterface * set = DKAllocInterface( DKSelector(Set), sizeof(struct DKSetInterface) );
    set->createWithVAObjects = DKHashTableCreateSetWithVAObjects;
    set->createWithCArray = DKHashTableCreateSetWithCArray;
    set->createWithCollection = DKHashTableCreateSetWithCollection;
    
    set->getCount = (DKGetCountMethod)DKHashTableGetCount;
    set->getMember = (DKSetGetMemberMethod)DKHashTableGetObject;
    
    set->addObject = (void *)DKImmutableObjectAccessError;
    set->removeObject = (void *)DKImmutableObjectAccessError;
    set->removeAllObjects = (void *)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, set );
    DKRelease( set );
    
    return cls;
}


///
//  DKMutableHashTableClass()
//
DKThreadSafeClassInit(  DKMutableHashTableClass )
{
    // Since DKString, DKConstantString, DKHashTable and DKMutableHashTable are all
    // involved in creating constant strings, the names for these classes are
    // initialized in DKRuntimeInit().
    DKClassRef cls = DKAllocClass( NULL, DKHashTableClass(), sizeof(struct DKHashTable), 0 );
    
    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = (void *)DKHashTableMutableCopy;
    copying->mutableCopy = (void *)DKHashTableMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Dictionary
    struct DKDictionaryInterface * dictionary = DKAllocInterface( DKSelector(Dictionary), sizeof(struct DKDictionaryInterface) );
    dictionary->createWithVAKeysAndObjects = (DKDictionaryCreateWithVAKeysAndObjectsMethod)DKHashTableCreateDictionaryWithVAKeysAndObjects;
    dictionary->createWithDictionary = (DKDictionaryCreateWithDictionaryMethod)DKHashTableCreateDictionaryWithDictionary;

    dictionary->getCount = (DKGetCountMethod)DKHashTableGetCount;
    dictionary->getObject = (DKDictionaryGetObjectMethod)DKHashTableGetObject;

    dictionary->insertObject = (DKDictionaryInsertObjectMethod)DKHashTableInsertObject;
    dictionary->removeObject = (DKDictionaryRemoveObjectMethod)DKHashTableRemoveObject;
    dictionary->removeAllObjects = (DKDictionaryRemoveAllObjectsMethod)DKHashTableRemoveAllObjects;

    DKInstallInterface( cls, dictionary );
    DKRelease( dictionary );
    
    // Set
    struct DKSetInterface * set = DKAllocInterface( DKSelector(Set), sizeof(struct DKSetInterface) );
    set->createWithVAObjects = DKHashTableCreateSetWithVAObjects;
    set->createWithCArray = DKHashTableCreateSetWithCArray;
    set->createWithCollection = DKHashTableCreateSetWithCollection;

    set->getCount = (DKGetCountMethod)DKHashTableGetCount;
    set->getMember = (DKSetGetMemberMethod)DKHashTableGetObject;

    set->addObject = (DKSetAddObjectMethod)DKHashTableAddObjectToSet;
    set->removeObject = (DKSetRemoveObjectMethod)DKHashTableRemoveObject;
    set->removeAllObjects = (DKSetRemoveAllObjectsMethod)DKHashTableRemoveAllObjects;
    
    DKInstallInterface( cls, set );
    DKRelease( set );

    return cls;
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
            Insert( hashTable, row->hash, row->key, row->object, DKInsertAlways );
        
            DKRelease( row->key );
            DKRelease( row->object );
        }
    }
    
    dk_free( oldRows );
}


///
//  Find()
//
static struct DKHashTableRow * Find( struct DKHashTable * hashTable, DKHashCode hash, DKObjectRef key )
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
static void Insert( struct DKHashTable * hashTable, DKHashCode hash, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    if( key == NULL )
    {
        DKError( "DKHashTableInsert: Trying to insert a NULL key.\n" );
        return;
    }

    // It should be impossible for a key object to exist at the max addressable byte in
    // memory... but lets's not be surprised.
    DKAssert( key != DELETED_KEY );

    struct DKHashTableRow * row = Find( hashTable, hash, key );
    bool active = RowIsActive( row );

    if( !active && (policy == DKInsertIfFound) )
        return;
    
    if( active && (policy == DKInsertIfNotFound) )
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
static void Remove( struct DKHashTable * hashTable, DKHashCode hash, DKObjectRef key )
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


///
//  InsertKeyAndObject()
//
static int InsertKeyAndObject( DKObjectRef key, DKObjectRef object, void * context )
{
    struct DKHashTable * hashTable = context;
    
    DKHashCode hash = DKHash( key );
    Insert( hashTable, hash, key, object, DKInsertAlways );
    
    return 0;
}


///
//  InsertObject()
//
static int InsertObject( DKObjectRef object, void * context )
{
    struct DKHashTable * hashTable = context;
    
    DKHashCode hash = DKHash( object );
    Insert( hashTable, hash, object, object, DKInsertAlways );
    
    return 0;
}




// Interface =============================================================================

///
//  DKHashTableInitialize()
//
static DKObjectRef DKHashTableInitialize( DKObjectRef _self )
{
    if( _self )
    {
        struct DKHashTable * hashTable = (struct DKHashTable *)_self;

        hashTable->count = 0;
        hashTable->rowCount = MIN_HASHTABLE_SIZE;
        hashTable->capacity = hashTable->rowCount / 2;
        hashTable->rows = dk_malloc( sizeof(struct DKHashTableRow) * MIN_HASHTABLE_SIZE );

        memset( hashTable->rows, 0, sizeof(struct DKHashTableRow) * MIN_HASHTABLE_SIZE );
    }
    
    return _self;
}


///
//  DKHashTableFinalize()
//
static void DKHashTableFinalize( DKObjectRef _self )
{
    struct DKHashTable * hashTable = (struct DKHashTable *)_self;
    RemoveAll( hashTable );
}


///
//  DKHashTableCreateDictionaryWithVAKeysAndObjects()
//
static DKObjectRef DKHashTableCreateDictionaryWithVAKeysAndObjects( DKClassRef _class, va_list keysAndObjects )
{
    struct DKHashTable * hashTable = NULL;
    
    if( _class )
    {
        DKAssert( DKIsSubclass( _class, DKHashTableClass() ) );
        
        hashTable = DKCreate( _class );
        
        if( hashTable )
        {
            DKObjectRef key, object;
        
            while( (key = va_arg( keysAndObjects, DKObjectRef ) ) != NULL )
            {
                object = va_arg( keysAndObjects, DKObjectRef );
        
                DKHashCode hash = DKHash( key );
                Insert( hashTable, hash, key, object, DKInsertAlways );
            }
        }
    }
    
    return hashTable;
}


///
//  DKHashTableCreateDictionaryWithDictionary()
//
DKObjectRef DKHashTableCreateDictionaryWithDictionary( DKClassRef _class, DKDictionaryRef dictionary )
{
    struct DKHashTable * hashTable = NULL;
    
    if( _class )
    {
        DKAssert( DKIsSubclass( _class, DKHashTableClass() ) );
        
        hashTable = DKCreate( _class );
        
        if( hashTable )
        {
            DKForeachKeyAndObject( dictionary, InsertKeyAndObject, hashTable );
        }
    }
    
    return hashTable;
}


///
//  DKHashTableCreateSetWithVAObjects()
//
static DKObjectRef DKHashTableCreateSetWithVAObjects( DKClassRef _class, va_list objects )
{
    struct DKHashTable * hashTable = NULL;
    
    if( _class )
    {
        DKAssert( DKIsSubclass( _class, DKHashTableClass() ) );
        
        hashTable = DKCreate( _class );
        
        if( hashTable )
        {
            DKObjectRef object;
        
            while( (object = va_arg( objects, DKObjectRef ) ) != NULL )
            {
                DKHashCode hash = DKHash( object );
                Insert( hashTable, hash, object, object, DKInsertAlways );
            }
        }
    }
    
    return hashTable;
}


///
//  DKHashTableCreateSetWithCArray()
//
DKObjectRef DKHashTableCreateSetWithCArray( DKClassRef _class, DKObjectRef objects[], DKIndex count )
{
    struct DKHashTable * hashTable = NULL;
    
    if( _class )
    {
        DKAssert( DKIsSubclass( _class, DKHashTableClass() ) );
        
        hashTable = DKCreate( _class );
        
        if( hashTable )
        {
            for( DKIndex i = 0; i < count; ++i )
            {
                DKObjectRef object = objects[i];
                
                DKHashCode hash = DKHash( object );
                Insert( hashTable, hash, object, object, DKInsertAlways );
            }
        }
    }
    
    return hashTable;
}


///
//  DKHashTableCreateSetWithCollection()
//
DKObjectRef DKHashTableCreateSetWithCollection( DKClassRef _class, DKObjectRef collection )
{
    struct DKHashTable * hashTable = NULL;
    
    if( _class )
    {
        DKAssert( DKIsSubclass( _class, DKHashTableClass() ) );
        
        hashTable = DKCreate( _class );
        
        if( hashTable )
        {
            DKForeachObject( collection, InsertObject, hashTable );
        }
    }
    
    return hashTable;
}


///
//  DKHashTableCopy()
//
DKHashTableRef DKHashTableCopy( DKHashTableRef _self )
{
    return DKHashTableCreateDictionaryWithDictionary( DKGetClass( _self ), _self );
}


///
//  DKHashTableMutableCopy()
//
DKMutableHashTableRef DKHashTableMutableCopy( DKHashTableRef _self )
{
    return (DKMutableHashTableRef)DKHashTableCreateDictionaryWithDictionary( DKMutableHashTableClass(), _self );
}


///
//  DKHashTableGetCount()
//
DKIndex DKHashTableGetCount( DKHashTableRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );
        return _self->count;
    }
    
    return 0;
}


///
//  DKHashTableGetObject()
//
DKObjectRef DKHashTableGetObject( DKHashTableRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );

        struct DKHashTable * hashTable = (struct DKHashTable *)_self;
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
int DKHashTableApplyFunction( DKHashTableRef _self, DKKeyedApplierFunction callback, void * context )
{
    int result = 0;
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );

        for( DKIndex i = 0; i < _self->count; ++i )
        {
            struct DKHashTableRow * row = &_self->rows[i];
            
            if( RowIsActive( row ) )
            {
                if( (result = callback( row->key, row->object, context )) != 0 )
                    break;
            }
        }
    }
    
    return result;
}


///
//  DKHashTableApplyFunctionToKeys()
//
int DKHashTableApplyFunctionToKeys( DKHashTableRef _self, DKApplierFunction callback, void * context )
{
    int result = 0;
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );

        for( DKIndex i = 0; i < _self->count; ++i )
        {
            struct DKHashTableRow * row = &_self->rows[i];
            
            if( RowIsActive( row ) )
            {
                if( (result = callback( row->key, context )) != 0 )
                    break;
            }
        }
    }
    
    return result;
}


///
//  DKHashTableApplyFunctionToObjects()
//
int DKHashTableApplyFunctionToObjects( DKHashTableRef _self, DKApplierFunction callback, void * context )
{
    int result = 0;
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );

        for( DKIndex i = 0; i < _self->count; ++i )
        {
            struct DKHashTableRow * row = &_self->rows[i];
            
            if( RowIsActive( row ) )
            {
                if( (result = callback( row->object, context )) != 0 )
                    break;
            }
        }
    }
    
    return result;
}


///
//  DKHashTableInsertObject()
//
void DKHashTableInsertObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableHashTableClass() );

        DKHashCode hash = DKHash( key );

        Insert( _self, hash, key, object, policy );
    }
}


///
//  DKHashTableRemoveObject()
//
void DKHashTableRemoveObject( DKMutableHashTableRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableHashTableClass() );

        DKHashCode hash = DKHash( key );
        
        Remove( _self, hash, key );
    }
}


///
//  DKHashTableRemoveAllObjects()
//
void DKHashTableRemoveAllObjects( DKMutableHashTableRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableHashTableClass() );
        RemoveAll( _self );
    }
}


///
//  DKHashTableAddObjectToSet()
//
static void DKHashTableAddObjectToSet( DKMutableHashTableRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableHashTableClass() );

        DKHashCode hash = DKHash( object );

        Insert( _self, hash, object, object, DKInsertIfNotFound );
    }
}








