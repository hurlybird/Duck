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
#include "DKGenericHashTable.h"
#include "DKString.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKEgg.h"


struct DKHashTableRow
{
    DKObjectRef key;
    DKObjectRef object;
};

struct DKHashTable
{
    DKObject _obj;
    
    DKGenericHashTable table;
};


static DKObjectRef DKHashTableInitialize( DKObjectRef _self );
static void        DKHashTableFinalize( DKObjectRef _self );

static DKObjectRef DKHashTableInitWithEgg( DKHashTableRef _self, DKEggUnarchiverRef egg );
static void        DKHashTableAddToEgg( DKHashTableRef _self, DKEggArchiverRef egg );

static DKIndex     INTERNAL_DKHashTableGetCount( DKHashTableRef _self );
static DKObjectRef INTERNAL_DKHashTableGetObject( DKHashTableRef _self, DKObjectRef key );

static void        INTERNAL_DKHashTableSetObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object );
static void        INTERNAL_DKHashTableInsertObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
static void        INTERNAL_DKHashTableRemoveObject( DKMutableHashTableRef _self, DKObjectRef key );
static void        INTERNAL_DKHashTableRemoveAllObjects( DKMutableHashTableRef _self );

static void        INTERNAL_DKHashTableAddObjectToSet( DKMutableHashTableRef _self, DKObjectRef object );


///
//  DKHashTableClass()
//
DKThreadSafeClassInit( DKHashTableClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKHashTable" ), DKObjectClass(), sizeof(struct DKHashTable), DKImmutableInstances, DKHashTableInitialize, DKHashTableFinalize );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKNewInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKDictionaryEqual;
    comparison->compare = (DKCompareMethod)DKPointerCompare;
    comparison->hash = (DKHashMethod)DKPointerHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKHashTableMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Description
    struct DKDescriptionInterface * description = DKNewInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKKeyedCollectionGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Collection
    struct DKCollectionInterface * collection = DKNewInterface( DKSelector(Collection), sizeof(struct DKCollectionInterface) );
    collection->getCount = (DKGetCountMethod)INTERNAL_DKHashTableGetCount;
    collection->containsObject = (DKContainsMethod)DKDictionaryContainsObject;
    collection->foreachObject = (DKForeachObjectMethod)DKHashTableApplyFunctionToObjects;
    
    DKInstallInterface( cls, collection );
    DKRelease( collection );

    // KeyedCollection
    struct DKKeyedCollectionInterface * keyedCollection = DKNewInterface( DKSelector(KeyedCollection), sizeof(struct DKKeyedCollectionInterface) );
    keyedCollection->getCount = (DKGetCountMethod)INTERNAL_DKHashTableGetCount;
    keyedCollection->containsObject = (DKContainsMethod)DKDictionaryContainsObject;
    keyedCollection->foreachObject = (DKForeachObjectMethod)DKHashTableApplyFunctionToObjects;
    keyedCollection->containsKey = (DKContainsMethod)DKDictionaryContainsKey;
    keyedCollection->foreachKey = (DKForeachObjectMethod)DKHashTableApplyFunctionToKeys;
    keyedCollection->foreachKeyAndObject = (DKForeachKeyAndObjectMethod)DKHashTableApplyFunction;
    
    DKInstallInterface( cls, keyedCollection );
    DKRelease( keyedCollection );

    // Dictionary
    struct DKDictionaryInterface * dictionary = DKNewInterface( DKSelector(Dictionary), sizeof(struct DKDictionaryInterface) );
    dictionary->initWithVAKeysAndObjects = (DKDictionaryInitWithVAKeysAndObjectsMethod)DKHashTableInitDictionaryWithVAKeysAndObjects;
    dictionary->initWithDictionary = (DKDictionaryInitWithDictionaryMethod)DKHashTableInitDictionaryWithDictionary;
    
    dictionary->getCount = (DKGetCountMethod)INTERNAL_DKHashTableGetCount;
    dictionary->getObject = (DKDictionaryGetObjectMethod)INTERNAL_DKHashTableGetObject;
    
    dictionary->insertObject = (void *)DKImmutableObjectAccessError;
    dictionary->removeObject = (void *)DKImmutableObjectAccessError;
    dictionary->removeAllObjects = (void *)DKImmutableObjectAccessError;

    DKInstallInterface( cls, dictionary );
    DKRelease( dictionary );
    
    // Set
    struct DKSetInterface * set = DKNewInterface( DKSelector(Set), sizeof(struct DKSetInterface) );
    set->initWithVAObjects = (DKSetInitWithVAObjectsMethod)DKHashTableInitSetWithVAObjects;
    set->initWithCArray = (DKSetInitWithCArrayMethod)DKHashTableInitSetWithCArray;
    set->initWithCollection = (DKSetInitWithCollectionMethod)DKHashTableInitSetWithCollection;
    
    set->getCount = (DKGetCountMethod)INTERNAL_DKHashTableGetCount;
    set->getMember = (DKSetGetMemberMethod)INTERNAL_DKHashTableGetObject;
    
    set->addObject = (void *)DKImmutableObjectAccessError;
    set->removeObject = (void *)DKImmutableObjectAccessError;
    set->removeAllObjects = (void *)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, set );
    DKRelease( set );
    
    // Property
    struct DKPropertyInterface * property = DKNewInterface( DKSelector(Property), sizeof(struct DKPropertyInterface) );
    property->getProperty = (DKGetPropertyMethod)INTERNAL_DKHashTableGetObject;
    property->setProperty = (void *)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, property );
    DKRelease( property );
    
    // Egg
    struct DKEggInterface * egg = DKNewInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKHashTableInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKHashTableAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKMutableHashTableClass()
//
DKThreadSafeClassInit(  DKMutableHashTableClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKMutablehashTable" ), DKHashTableClass(), sizeof(struct DKHashTable), 0, NULL, NULL );
    
    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = (DKCopyMethod)DKHashTableMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKHashTableMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Dictionary
    struct DKDictionaryInterface * dictionary = DKNewInterface( DKSelector(Dictionary), sizeof(struct DKDictionaryInterface) );
    dictionary->initWithVAKeysAndObjects = (DKDictionaryInitWithVAKeysAndObjectsMethod)DKHashTableInitDictionaryWithVAKeysAndObjects;
    dictionary->initWithDictionary = (DKDictionaryInitWithDictionaryMethod)DKHashTableInitDictionaryWithDictionary;

    dictionary->getCount = (DKGetCountMethod)INTERNAL_DKHashTableGetCount;
    dictionary->getObject = (DKDictionaryGetObjectMethod)INTERNAL_DKHashTableGetObject;

    dictionary->insertObject = (DKDictionaryInsertObjectMethod)INTERNAL_DKHashTableInsertObject;
    dictionary->removeObject = (DKDictionaryRemoveObjectMethod)INTERNAL_DKHashTableRemoveObject;
    dictionary->removeAllObjects = (DKDictionaryRemoveAllObjectsMethod)INTERNAL_DKHashTableRemoveAllObjects;

    DKInstallInterface( cls, dictionary );
    DKRelease( dictionary );
    
    // Set
    struct DKSetInterface * set = DKNewInterface( DKSelector(Set), sizeof(struct DKSetInterface) );
    set->initWithVAObjects = (DKSetInitWithVAObjectsMethod)DKHashTableInitSetWithVAObjects;
    set->initWithCArray = (DKSetInitWithCArrayMethod)DKHashTableInitSetWithCArray;
    set->initWithCollection = (DKSetInitWithCollectionMethod)DKHashTableInitSetWithCollection;

    set->getCount = (DKGetCountMethod)INTERNAL_DKHashTableGetCount;
    set->getMember = (DKSetGetMemberMethod)INTERNAL_DKHashTableGetObject;

    set->addObject = (DKSetAddObjectMethod)INTERNAL_DKHashTableAddObjectToSet;
    set->removeObject = (DKSetRemoveObjectMethod)INTERNAL_DKHashTableRemoveObject;
    set->removeAllObjects = (DKSetRemoveAllObjectsMethod)INTERNAL_DKHashTableRemoveAllObjects;
    
    DKInstallInterface( cls, set );
    DKRelease( set );

    // Property
    struct DKPropertyInterface * property = DKNewInterface( DKSelector(Property), sizeof(struct DKPropertyInterface) );
    property->getProperty = (DKGetPropertyMethod)INTERNAL_DKHashTableGetObject;
    property->setProperty = (DKSetPropertyMethod)INTERNAL_DKHashTableSetObject;
    
    DKInstallInterface( cls, property );
    DKRelease( property );
    
    return cls;
}




// DKGenericHashTable Callbacks ==========================================================

static DKRowStatus RowStatus( const void * _row )
{
    const struct DKHashTableRow * row = _row;
    return (DKRowStatus)DK_HASHTABLE_ROW_STATUS( row->key );
}

static DKHashCode RowHash( const void * _row )
{
    const struct DKHashTableRow * row = _row;
    return DKHash( row->key );
}

static bool RowEqual( const void * _row1, const void * _row2 )
{
    const struct DKHashTableRow * row1 = _row1;
    const struct DKHashTableRow * row2 = _row2;

    return DKEqual( row1->key, row2->key );
}

static void RowInit( void * _row )
{
    struct DKHashTableRow * row = _row;
    
    row->key = DK_HASHTABLE_EMPTY_KEY;
    row->object = NULL;
}

static void RowUpdate( void * _row, const void * _src )
{
    struct DKHashTableRow * row = _row;
    const struct DKHashTableRow * src = _src;
    
    if( !DK_HASHTABLE_IS_POINTER( row->key ) )
    {
        row->key = DKCopy( src->key );
    }

    if( row->object != src->object )
    {
        DKRelease( row->object );
        row->object = DKRetain( src->object );
    }
}

static void RowDelete( void * _row )
{
    struct DKHashTableRow * row = _row;
    
    DKRelease( row->key );
    DKRelease( row->object );
    
    row->key = DK_HASHTABLE_DELETED_KEY;
    row->object = NULL;
}




// Insert Applier Functions ==============================================================

///
//  InsertKeyAndObject()
//
static int InsertKeyAndObject( DKObjectRef key, DKObjectRef object, void * context )
{
    struct DKHashTable * hashTable = context;
    
    struct DKHashTableRow row;
    row.key = DKCopy( key );
    row.object = object;
    
    DKGenericHashTableInsert( &hashTable->table, &row, DKInsertAlways );
    
    DKRelease( row.key );
    
    return 0;
}


///
//  InsertObject()
//
static int InsertObject( DKObjectRef object, void * context )
{
    struct DKHashTable * hashTable = context;
    
    struct DKHashTableRow row;
    row.key = object;
    row.object = object;
    
    DKGenericHashTableInsert( &hashTable->table, &row, DKInsertAlways );
    
    return 0;
}




// Interface =============================================================================

///
//  DKHashTableInitialize()
//
static DKObjectRef DKHashTableInitialize( DKObjectRef _self )
{
    _self = DKSuperInit( _self, DKObjectClass() );

    if( _self )
    {
        struct DKHashTable * hashTable = _self;

        DKGenericHashTableCallbacks callbacks =
        {
            RowStatus,
            RowHash,
            RowEqual,
            RowInit,
            RowUpdate,
            RowDelete
        };

        DKGenericHashTableInit( &hashTable->table, sizeof(struct DKHashTableRow), &callbacks );
    }
    
    return _self;
}


///
//  DKHashTableFinalize()
//
static void DKHashTableFinalize( DKObjectRef _self )
{
    struct DKHashTable * hashTable = _self;
    DKGenericHashTableFinalize( &hashTable->table );
}


///
//  DKHashTableInitDictionaryWithVAKeysAndObjects()
//
DKObjectRef DKHashTableInitDictionaryWithVAKeysAndObjects( DKHashTableRef _self, va_list keysAndObjects )
{
    struct DKHashTable * hashTable = DKInit( _self );
        
    if( hashTable )
    {
        DKAssertKindOfClass( hashTable, DKHashTableClass() );

        DKObjectRef key;
    
        while( (key = va_arg( keysAndObjects, DKObjectRef ) ) != NULL )
        {
            struct DKHashTableRow row;
            row.key = DKCopy( key );
            row.object = va_arg( keysAndObjects, DKObjectRef );
    
            DKGenericHashTableInsert( &hashTable->table, &row, DKInsertAlways );
            
            DKRelease( row.key );
        }
    }
    
    return hashTable;
}


///
//  DKHashTableInitDictionaryWithDictionary()
//
DKObjectRef DKHashTableInitDictionaryWithDictionary( DKHashTableRef _self, DKDictionaryRef dictionary )
{
    struct DKHashTable * hashTable = DKInit( _self );
    
    if( hashTable )
    {
        DKAssertKindOfClass( hashTable, DKHashTableClass() );
        DKForeachKeyAndObject( dictionary, InsertKeyAndObject, hashTable );
    }
    
    return hashTable;
}


///
//  DKHashTableInitSetWithVAObjects()
//
DKObjectRef DKHashTableInitSetWithVAObjects( DKHashTableRef _self, va_list objects )
{
    struct DKHashTable * hashTable = DKInit( _self );
    
    if( hashTable )
    {
        DKAssertKindOfClass( hashTable, DKHashTableClass() );

        struct DKHashTableRow row;
    
        while( (row.key = va_arg( objects, DKObjectRef ) ) != NULL )
        {
            row.object = row.key;

            DKGenericHashTableInsert( &hashTable->table, &row, DKInsertAlways );
        }
    }
    
    return hashTable;
}


///
//  DKHashTableInitSetWithCArray()
//
DKObjectRef DKHashTableInitSetWithCArray( DKHashTableRef _self, DKObjectRef objects[], DKIndex count )
{
    struct DKHashTable * hashTable = DKInit( _self );
    
    if( hashTable )
    {
        DKAssertKindOfClass( hashTable, DKHashTableClass() );

        struct DKHashTableRow row;

        for( DKIndex i = 0; i < count; ++i )
        {
            row.key = objects[i];
            row.object = row.key;
            
            DKGenericHashTableInsert( &hashTable->table, &row, DKInsertAlways );
        }
    }
    
    return hashTable;
}


///
//  DKHashTableInitSetWithCollection()
//
DKObjectRef DKHashTableInitSetWithCollection( DKHashTableRef _self, DKObjectRef collection )
{
    struct DKHashTable * hashTable = DKInit( _self );
    
    if( hashTable )
    {
        DKAssertKindOfClass( hashTable, DKHashTableClass() );
        DKForeachObject( collection, InsertObject, hashTable );
    }
    
    return hashTable;
}


///
//  DKHashTableInitWithEgg()
//
static DKObjectRef DKHashTableInitWithEgg( DKHashTableRef _self, DKEggUnarchiverRef egg )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );
        DKEggGetKeyedCollection( egg, DKSTR( "pairs" ), InsertKeyAndObject, _self );
    }
    
    return _self;
}


///
//  DKHashTableAddToEgg()
//
static void DKHashTableAddToEgg( DKHashTableRef _self, DKEggArchiverRef egg )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );
        DKEggAddKeyedCollection( egg, DKSTR( "pairs" ), _self );
    }
}


///
//  DKHashTableCopy()
//
DKHashTableRef DKHashTableCopy( DKHashTableRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );

        struct DKHashTable * copy = DKNew( DKGetClass( _self ) );

        DKHashTableApplyFunction( _self, InsertKeyAndObject, copy );
        
        return copy;
    }
    
    return NULL;
}


///
//  DKHashTableMutableCopy()
//
DKMutableHashTableRef DKHashTableMutableCopy( DKHashTableRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );

        struct DKHashTable * copy = DKNew( DKMutableHashTableClass() );

        DKHashTableApplyFunction( _self, InsertKeyAndObject, copy );
        
        return copy;
    }
    
    return NULL;
}


///
//  DKHashTableGetCount()
//
DKIndex DKHashTableGetCount( DKHashTableRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );
        return DKGenericHashTableGetCount( &_self->table );
    }
    
    return 0;
}

static DKIndex INTERNAL_DKHashTableGetCount( DKHashTableRef _self )
{
    return DKGenericHashTableGetCount( &_self->table );
}


///
//  DKHashTableGetObject()
//
DKObjectRef DKHashTableGetObject( DKHashTableRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKHashTableClass() );
        return INTERNAL_DKHashTableGetObject( _self, key );
    }

    return NULL;
}

static DKObjectRef INTERNAL_DKHashTableGetObject( DKHashTableRef _self, DKObjectRef key )
{
    struct DKHashTableRow findRow;
    findRow.key = key;
    findRow.object = NULL;
    
    const struct DKHashTableRow * row = DKGenericHashTableFind( &_self->table, &findRow );

    if( row )
        return row->object;

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

        for( DKIndex i = 0; i < DKGenericHashTableGetRowCount( &_self->table ); ++i )
        {
            const struct DKHashTableRow * row = DKGenericHashTableGetRow( &_self->table, i );
            
            if( RowStatus( row ) == DKRowStatusActive )
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

        for( DKIndex i = 0; i < DKGenericHashTableGetRowCount( &_self->table ); ++i )
        {
            const struct DKHashTableRow * row = DKGenericHashTableGetRow( &_self->table, i );
            
            if( RowStatus( row ) == DKRowStatusActive )
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

        for( DKIndex i = 0; i < DKGenericHashTableGetRowCount( &_self->table ); ++i )
        {
            const struct DKHashTableRow * row = DKGenericHashTableGetRow( &_self->table, i );
            
            if( RowStatus( row ) == DKRowStatusActive )
            {
                if( (result = callback( row->object, context )) != 0 )
                    break;
            }
        }
    }
    
    return result;
}


///
//  DKHashTableSetObject()
//
static void INTERNAL_DKHashTableSetObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object )
{
    return INTERNAL_DKHashTableInsertObject( _self, key, object, DKInsertAlways );
}


///
//  DKHashTableInsertObject()
//
void DKHashTableInsertObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableHashTableClass() );
        INTERNAL_DKHashTableInsertObject( _self, key, object, policy );
    }
}

static void INTERNAL_DKHashTableInsertObject( DKMutableHashTableRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    struct DKHashTableRow row;
    row.key = key;
    row.object = object;

    DKGenericHashTableInsert( &_self->table, &row, policy );
}


///
//  DKHashTableRemoveObject()
//
void DKHashTableRemoveObject( DKMutableHashTableRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableHashTableClass() );
        INTERNAL_DKHashTableRemoveObject( _self, key );
    }
}

static void INTERNAL_DKHashTableRemoveObject( DKMutableHashTableRef _self, DKObjectRef key )
{
    struct DKHashTableRow row;
    row.key = key;
    row.object = NULL;

    DKGenericHashTableRemove( &_self->table, &row );
}


///
//  DKHashTableRemoveAllObjects()
//
void DKHashTableRemoveAllObjects( DKMutableHashTableRef _self )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableHashTableClass() );
        DKGenericHashTableRemoveAll( &_self->table );
    }
}

static void INTERNAL_DKHashTableRemoveAllObjects( DKMutableHashTableRef _self )
{
    DKGenericHashTableRemoveAll( &_self->table );
}


///
//  DKHashTableAddObjectToSet()
//
void DKHashTableAddObjectToSet( DKMutableHashTableRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableHashTableClass() );
        INTERNAL_DKHashTableAddObjectToSet( _self, object );
    }
}

static void INTERNAL_DKHashTableAddObjectToSet( DKMutableHashTableRef _self, DKObjectRef object )
{
    struct DKHashTableRow row;
    row.key = object;
    row.object = object;

    DKGenericHashTableInsert( &_self->table, &row, DKInsertIfNotFound );
}








