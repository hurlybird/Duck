/*****************************************************************************************

  DKRuntime+RefCount.c

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
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"




// Weak Reference Table ==================================================================

static DKSpinLock WeakReferenceTableLock = DKSpinLockInit;
static DKGenericHashTable WeakReferenceTable;


// Hash Table Callbacks
static DKRowStatus WeakReferenceTableRowStatus( const void * _row )
{
    const DKWeakRef * row = _row;
    return (DKRowStatus)DK_HASHTABLE_ROW_STATUS( *row );
}

static DKHashCode WeakReferenceTableRowHash( const void * _row )
{
    const DKWeakRef * row = _row;
    return DKObjectUniqueHash( (*row)->target );
}

static bool WeakReferenceTableRowEqual( const void * _row1, const void * _row2 )
{
    const DKWeakRef * row1 = _row1;
    const DKWeakRef * row2 = _row2;
    return (*row1)->target == (*row2)->target;
}

static void WeakReferenceTableRowInit( void * _row )
{
    DKWeakRef * row = _row;
    (*row) = DK_HASHTABLE_EMPTY_KEY;
}

static void WeakReferenceTableRowUpdate( void * _row, const void * _src )
{
    DKWeakRef * row = _row;
    const DKWeakRef * src = _src;
    *row = *src;
}

static void WeakReferenceTableRowDelete( void * _row )
{
    DKWeakRef * row = _row;
    (*row) = DK_HASHTABLE_DELETED_KEY;
}


///
//  DKWeakReferenceTableInit()
//
void DKWeakReferenceTableInit( void )
{
    DKGenericHashTableCallbacks callbacks =
    {
        WeakReferenceTableRowStatus,
        WeakReferenceTableRowHash,
        WeakReferenceTableRowEqual,
        WeakReferenceTableRowInit,
        WeakReferenceTableRowUpdate,
        WeakReferenceTableRowDelete
    };
    
    DKGenericHashTableInit( &WeakReferenceTable, sizeof(DKWeakRef), &callbacks );
}


///
//  WeakReferenceTableFindOrInsert()
//
static DKWeakRef WeakReferenceTableFindOrInsert( DKObject * obj )
{
    DKWeakRef weakref = NULL;
    
    struct DKWeak _key;
    _key.target = obj;
    struct DKWeak * key = &_key;

    // Check the table for a weak reference
    DKSpinLockLock( &WeakReferenceTableLock );
    
    const DKWeakRef * entry = DKGenericHashTableFind( &WeakReferenceTable, &key );
    
    if( entry )
    {
        weakref = *entry;
        
        DKSpinLockUnlock( &WeakReferenceTableLock );
        
        return weakref;
    }
    
    DKSpinLockUnlock( &WeakReferenceTableLock );

    // Create a new weak reference
    DKWeakRef newWeakref = DKAlloc( DKWeakClass(), 0 );
    newWeakref->lock = DKSpinLockInit;
    newWeakref->target = obj;

    // Try to insert it into the table
    DKSpinLockLock( &WeakReferenceTableLock );

    if( DKGenericHashTableInsert( &WeakReferenceTable, &newWeakref, DKInsertIfNotFound ) )
    {
        DKAtomicAdd32( &obj->refcount, DKRefCountWeakBit );
        weakref = newWeakref;
    }
    
    else
    {
        const DKWeakRef * entry = DKGenericHashTableFind( &WeakReferenceTable, &key );
        weakref = *entry;
    }

    DKSpinLockUnlock( &WeakReferenceTableLock );

    // Discard the new weak reference if we're not using it
    if( weakref != newWeakref )
        DKRelease( newWeakref );
    
    return weakref;
}




// Strong References =====================================================================

///
//  DKRetain()
//
DKObjectRef DKRetain( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;

        int32_t rc = obj->refcount;

        if( (rc & DKRefCountDisabledBit) == 0 )
        {
            rc = DKAtomicIncrement32( &obj->refcount );
            DKAssert( (rc & DKRefCountErrorBit) == 0 );
        }
    }

    return _self;
}


///
//  DKRelease()
//
void DKRelease( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;

        int32_t rc = obj->refcount;

        if( (rc & DKRefCountDisabledBit) == 0 )
        {
            if( (rc & DKRefCountWeakBit) == 0 )
            {
                rc = DKAtomicDecrement32( &obj->refcount );
                DKAssert( (rc & DKRefCountErrorBit) == 0 );

                if( (rc & DKRefCountMask) == 0 )
                {
                    DKFinalize( _self );
                    DKDealloc( _self );
                }
            }
            
            else
            {
                DKWeakRef weakref = WeakReferenceTableFindOrInsert( obj );
                
                DKSpinLockLock( &weakref->lock );
                
                rc = DKAtomicDecrement32( &obj->refcount );
                DKAssert( (rc & DKRefCountErrorBit) == 0 );

                if( (rc & DKRefCountMask) == 0 )
                    weakref->target = NULL;

                DKSpinLockUnlock( &weakref->lock );
                
                if( (rc & DKRefCountMask) == 0 )
                {
                    struct DKWeak _key;
                    _key.target = obj;
                    struct DKWeak * key = &_key;

                    DKSpinLockLock( &WeakReferenceTableLock );
                    DKGenericHashTableRemove( &WeakReferenceTable, &key );
                    DKSpinLockUnlock( &WeakReferenceTableLock );
                
                    DKRelease( weakref );
                    DKFinalize( _self );
                    DKDealloc( _self );
                }
            }
        }
    }
}




// Weak References =======================================================================

///
//  DKRetainWeak()
//
DKWeakRef DKRetainWeak( DKObjectRef _self )
{
    if( _self )
    {
        DKWeakRef weakref = WeakReferenceTableFindOrInsert( _self );

        return DKRetain( weakref );
    }
    
    return NULL;
}


///
//  DKResolveWeak()
//
DKObjectRef DKResolveWeak( DKWeakRef weakref )
{
    if( weakref )
    {
        DKSpinLockLock( &weakref->lock );
        
        DKObjectRef target = DKRetain( weakref->target );
        
        DKSpinLockUnlock( &weakref->lock );
        
        return target;
    }
    
    return NULL;
}




// Autorelease Pools =====================================================================
#define DK_AUTORELEASE_POOL_RESERVE 128


///
//  DKPushAutoreleasePool()
//
void DKPushAutoreleasePool( void )
{
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    DKFatal( (threadContext->arpStack.top >= -1) && (threadContext->arpStack.top < (DK_AUTORELEASE_POOL_STACK_SIZE - 1)) );
    
    threadContext->arpStack.top++;
    
    DKGenericArray * arp = &threadContext->arpStack.arp[threadContext->arpStack.top];
    DKGenericArrayReserve( arp, DK_AUTORELEASE_POOL_RESERVE );
}


///
//  DKPopAutoreleasePool()
//
void DKPopAutoreleasePool( void )
{
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    DKFatal( (threadContext->arpStack.top >= 0) && (threadContext->arpStack.top < DK_AUTORELEASE_POOL_STACK_SIZE) );
    
    DKGenericArray * arp = &threadContext->arpStack.arp[threadContext->arpStack.top];
    DKIndex count = DKGenericArrayGetLength( arp );
    
    if( count > 0 )
    {
        DKObjectRef * objects = DKGenericArrayGetPointerToElementAtIndex( arp, 0 );
    
        for( DKIndex i = 0; i < count; ++i )
            DKRelease( objects[i] );

        DKGenericArrayReplaceElements( arp, DKRangeMake( 0, count ), NULL, 0 );
    }
    
    threadContext->arpStack.top--;
}


///
//  DKAutorelease()
//
DKObjectRef DKAutorelease( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;

        if( (obj->refcount & DKRefCountDisabledBit) == 0 )
        {
            struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
            DKFatal( (threadContext->arpStack.top >= 0) && (threadContext->arpStack.top < DK_AUTORELEASE_POOL_STACK_SIZE) );

            DKGenericArray * arp = &threadContext->arpStack.arp[threadContext->arpStack.top];
            DKGenericArrayAppendElements( arp, &_self, 1 );
        }
    }
    
    return _self;
}





