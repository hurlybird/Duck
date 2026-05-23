// =======================================================================================
//
// DKRuntime+Metadata.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#define DK_RUNTIME_PRIVATE 1

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"
#include "DKString.h"


static DKObjectRef DKMetadataInitWithOwner( DKObjectRef _self, DKObjectRef owner );


// Metadata Table ========================================================================

static DKSpinlock MetadataTableLock = DKSpinlockInit;
static DKGenericHashTable MetadataTable;


// Hash Table Callbacks
static DKRowStatus MetadataTableRowStatus( const void * _row, void * not_used )
{
    struct DKMetadata ** row = (void *)_row;
    return (DKRowStatus)(*row);
}

static DKHashCode MetadataTableRowHash( const void * _row, void * not_used )
{
    struct DKMetadata ** row =  (void *)_row;
    return DKObjectUniqueHash( (*row)->owner );
}

static bool MetadataTableRowEqual( const void * _row1, const void * _row2, void * not_used )
{
    struct DKMetadata ** row1 =  (void *)_row1;
    struct DKMetadata ** row2 =  (void *)_row2;
    return (*row1)->owner == (*row2)->owner;
}

static void MetadataTableRowInit( void * _row, void * not_used )
{
    DKMetadataRef * row = _row;
    (*row) = DKRowStatusEmpty;
}

static void MetadataTableRowUpdate( void * _row, const void * _src, void * not_used )
{
    DKMetadataRef * row = _row;
    DKMetadataRef * src = (void *)_src;
    *row = *src;
}

static void MetadataTableRowDelete( void * _row, void * not_used )
{
    DKMetadataRef * row = _row;
    (*row) = DKRowStatusDeleted;
}


///
//  DKWeakReferenceTableInit()
//
void DKMetadataTableInit( void )
{
    DKGenericHashTableCallbacks callbacks =
    {
        MetadataTableRowStatus,
        MetadataTableRowHash,
        MetadataTableRowEqual,
        MetadataTableRowInit,
        MetadataTableRowUpdate,
        MetadataTableRowDelete
    };
    
    DKGenericHashTableInit( &MetadataTable, sizeof(DKMetadataRef), &callbacks, NULL );
}


///
//  DKMetadataFindOrInsert()
//
DKMetadataRef DKMetadataFindOrInsert( DKObject * obj )
{
    DKMetadataRef metadata = NULL;
    
    struct DKMetadata _key;
    _key.owner = obj;
    struct DKMetadata * key = &_key;

    // Check the table for a weak reference
    DKSpinlockLock( &MetadataTableLock );
    
    DKMetadataRef * entry = (DKMetadataRef *)DKGenericHashTableFind( &MetadataTable, &key );
    
    if( entry )
    {
        metadata = *entry;
        
        DKSpinlockUnlock( &MetadataTableLock );
        
        return metadata;
    }
    
    DKSpinlockUnlock( &MetadataTableLock );

    // Create a new metadata object
    DKMetadataRef newMetadata = DKMetadataInitWithOwner( DKAlloc( DKMetadataClass() ), obj );

    // Try to insert it into the table
    DKSpinlockLock( &MetadataTableLock );

    if( DKGenericHashTableInsert( &MetadataTable, &newMetadata, DKInsertIfNotFound ) )
    {
        DKAtomicOr32( &obj->refcount, DKRefCountMetadataBit );
        metadata = newMetadata;
    }
    
    else
    {
        entry = (DKMetadataRef *)DKGenericHashTableFind( &MetadataTable, &key );
        metadata = *entry;
    }

    DKSpinlockUnlock( &MetadataTableLock );

    // Discard the new weak reference if we're not using it
    if( metadata != newMetadata )
        DKRelease( newMetadata );
    
    return metadata;
}


///
//  DKMetadataRemove()
//
void DKMetadataRemove( DKMetadataRef metadata )
{
    DKSpinlockLock( &MetadataTableLock );
    DKGenericHashTableRemove( &MetadataTable, &metadata );
    DKSpinlockUnlock( &MetadataTableLock );
    
    // The owner is about to be deallocated
    metadata->owner = NULL;
    
    DKRelease( metadata );
}


///
//  DKMetadataInit()
//
static DKObjectRef DKMetadataInitWithOwner( DKObjectRef _untyped_self, DKObjectRef owner )
{
    DKMetadataRef _self = _untyped_self;

    if( _self )
    {
        _self->owner = owner;
        
        _self->weakTarget = owner;
        _self->weakLock = DKSpinlockInit;
    }
    
    return _self;
}


///
//  DKMetadataFinalize()
//
void DKMetadataFinalize( DKObjectRef _untyped_self )
{
    DKMetadataRef _self = _untyped_self;

    DKRelease( _self->mutex );
}



