/*****************************************************************************************

  DKRuntime+Metadata.c

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

static DKSpinLock MetadataTableLock = DKSpinLockInit;
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
    DKSpinLockLock( &MetadataTableLock );
    
    DKMetadataRef * entry = (DKMetadataRef *)DKGenericHashTableFind( &MetadataTable, &key );
    
    if( entry )
    {
        metadata = *entry;
        
        DKSpinLockUnlock( &MetadataTableLock );
        
        return metadata;
    }
    
    DKSpinLockUnlock( &MetadataTableLock );

    // Create a new metadata object
    DKMetadataRef newMetadata = DKMetadataInitWithOwner( DKAlloc( DKMetadataClass() ), obj );

    // Try to insert it into the table
    DKSpinLockLock( &MetadataTableLock );

    if( DKGenericHashTableInsert( &MetadataTable, &newMetadata, DKInsertIfNotFound ) )
    {
        DKAtomicOr32( (uint32_t *)&obj->refcount, DKRefCountMetadataBit );
        metadata = newMetadata;
    }
    
    else
    {
        entry = (DKMetadataRef *)DKGenericHashTableFind( &MetadataTable, &key );
        metadata = *entry;
    }

    DKSpinLockUnlock( &MetadataTableLock );

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
    DKSpinLockLock( &MetadataTableLock );
    DKGenericHashTableRemove( &MetadataTable, &metadata );
    DKSpinLockUnlock( &MetadataTableLock );
    
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
        _self->weakLock = DKSpinLockInit;
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



