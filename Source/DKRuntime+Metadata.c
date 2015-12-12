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

#include "DKRuntime.h"
#include "DKString.h"
#include "DKGenericHashTable.h"


static DKObjectRef DKMetadataInitWithOwner( DKObjectRef _self, DKObjectRef owner );


// Metadata Table ========================================================================

static DKSpinLock MetadataTableLock = DKSpinLockInit;
static DKGenericHashTable MetadataTable;


// Hash Table Callbacks
static DKRowStatus MetadataTableRowStatus( const void * _row )
{
    const DKMetadataRef * row = _row;
    return (DKRowStatus)DK_HASHTABLE_ROW_STATUS( *row );
}

static DKHashCode MetadataTableRowHash( const void * _row )
{
    const DKMetadataRef * row = _row;
    return DKObjectUniqueHash( (*row)->owner );
}

static bool MetadataTableRowEqual( const void * _row1, const void * _row2 )
{
    const DKMetadataRef * row1 = _row1;
    const DKMetadataRef * row2 = _row2;
    return (*row1)->owner == (*row2)->owner;
}

static void MetadataTableRowInit( void * _row )
{
    DKMetadataRef * row = _row;
    (*row) = DK_HASHTABLE_EMPTY_KEY;
}

static void MetadataTableRowUpdate( void * _row, const void * _src )
{
    DKMetadataRef * row = _row;
    const DKMetadataRef * src = _src;
    *row = *src;
}

static void MetadataTableRowDelete( void * _row )
{
    DKMetadataRef * row = _row;
    (*row) = DK_HASHTABLE_DELETED_KEY;
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
    
    DKGenericHashTableInit( &MetadataTable, sizeof(DKWeakRef), &callbacks );
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
    
    const DKWeakRef * entry = DKGenericHashTableFind( &MetadataTable, &key );
    
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
        const DKMetadataRef * entry = DKGenericHashTableFind( &MetadataTable, &key );
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
static DKObjectRef DKMetadataInitWithOwner( DKObjectRef _self, DKObjectRef owner )
{
    if( _self )
    {
        struct DKMetadata * metadata = _self;

        metadata->owner = owner;
        
        metadata->weakTarget = owner;
        metadata->weakLock = DKSpinLockInit;
        
        metadata->spinLock = DKSpinLockInit;
    }
    
    return _self;
}




