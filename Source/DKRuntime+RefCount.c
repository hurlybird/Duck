// =======================================================================================
//
// DKRuntime+RefCount.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#define DK_RUNTIME_PRIVATE 1
#define DK_THREAD_PRIVATE  1

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"
#include "DKCollection.h"
#include "DKList.h"
#include "DKDictionary.h"
#include "DKString.h"
#include "DKGenericArray.h"
#include "DKThread.h"


// Object Life-cycle Watch ===============================================================
#if DK_DIAGNOSTIC_OBJECT_LIFECYCLE_WATCH

static DKObjectRef LifecycleWatchList[DK_OBJECT_LIFECYCLE_WATCH_LIST_SIZE] = { NULL, };
static DKSpinlock LifecycleWatchListLock = DKSpinlockInit;

static void DKLifecycleWatchListReport( DKObjectRef object, const char * label )
{
    DKSpinlockLock( &LifecycleWatchListLock );

    for( int i = 0; i < DK_OBJECT_LIFECYCLE_WATCH_LIST_SIZE; i++ )
    {
        if( LifecycleWatchList[i] == object )
        {
            DKObject * obj = object;
            int32_t rc = DKAtomicLoad32( &obj->refcount ) & DKRefCountMask;
            
            fprintf( stderr, "Object Life Cycle: 0x%p  rc=%d  %s\n", object, rc, label );
            
            if( rc == 0 )
                LifecycleWatchList[i] = NULL;
            break;
        }
    }

    DKSpinlockUnlock( &LifecycleWatchListLock );
}

void DKWatchObjectLifecycle( DKObjectRef object )
{
    DKSpinlockLock( &LifecycleWatchListLock );

    for( int i = 0; i < DK_OBJECT_LIFECYCLE_WATCH_LIST_SIZE; i++ )
    {
        if( LifecycleWatchList[i] == NULL )
        {
            LifecycleWatchList[i] = object;
            DKSpinlockUnlock( &LifecycleWatchListLock );
            
            DKLifecycleWatchListReport( object, "watching" );
            return;
        }
    }

    DKAssert( 0 );
    DKSpinlockUnlock( &LifecycleWatchListLock );
}

#else

#define DKLifecycleWatchListReport( object, label )

#endif




// Strong References =====================================================================

///
//  DKRetain()
//
DKObjectRef DKRetain( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;

        // The refcount flags do not change throughout an object's lifetime so it's
        // generally safe to check them nonatomically. (The metadata flag is a special
        // case--it's set once, atomically, and inside a spinlock.)
        int32_t rc = obj->refcount; // DKAtomicLoad32( &obj->refcount );

        if( (rc & DKRefCountDisabledBit) == 0 )
        {
            rc = DKAtomicIncrement32( &obj->refcount );
            
            DKLifecycleWatchListReport( obj, "retained" );
            DKAssert( (rc & DKRefCountOverflowBit) == 0 );
        }
    }

    return _self;
}


///
//  DKRelease()
//
DKObjectRef DKRelease( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;

        // The refcount flags do not change throughout an object's lifetime so it's
        // generally safe to check them nonatomically. (The metadata flag is a special
        // case--it's set once, atomically, and inside a spinlock.)
        int32_t rc = obj->refcount; // DKAtomicLoad32( &obj->refcount );

        if( (rc & DKRefCountDisabledBit) == 0 )
        {
            if( (rc & DKRefCountMetadataBit) == 0 )
            {
                rc = DKAtomicDecrement32( &obj->refcount );
                
                DKLifecycleWatchListReport( obj, "released" );
                DKAssert( (rc & DKRefCountOverflowBit) == 0 );

                if( (rc & DKRefCountMask) == 0 )
                {
                    DKFinalize( _self );
                    DKDealloc( _self );
                }
            }
            
            else
            {
                DKMetadataRef metadata = DKMetadataFindOrInsert( obj );
                
                DKSpinlockLock( &metadata->weakLock );
                
                rc = DKAtomicDecrement32( &obj->refcount );
                
                DKLifecycleWatchListReport( obj, "released" );
                DKAssert( (rc & DKRefCountOverflowBit) == 0 );

                if( (rc & DKRefCountMask) == 0 )
                    metadata->weakTarget = NULL;

                DKSpinlockUnlock( &metadata->weakLock );
                
                if( (rc & DKRefCountMask) == 0 )
                {
                    DKMetadataRemove( metadata );
                    DKFinalize( _self );
                    DKDealloc( _self );
                }
            }
        }
    }
    
    return NULL;
}


///
//  DKTryRelease()
//
DKObjectRef DKTryRelease( DKObjectRef _self )
{
    DKObjectRef result = _self;

    if( _self )
    {
        DKObject * obj = _self;

        int32_t rc = DKAtomicLoad32( &obj->refcount );

        if( (rc & DKRefCountDisabledBit) == 0 )
        {
            if( (rc & DKRefCountMetadataBit) == 0 )
            {
                if( (rc & DKRefCountMask) == 1 )
                {
                    int32_t rc_zero = rc & ~DKRefCountMask;
                    
                    if( DKAtomicCompareAndSwap32( &obj->refcount, &rc, rc_zero ) )
                    {
                        DKLifecycleWatchListReport( obj, "released" );
                        
                        DKFinalize( _self );
                        DKDealloc( _self );

                        result = NULL;
                    }
                }
            }
            
            else
            {
                DKMetadataRef metadata = DKMetadataFindOrInsert( obj );
                
                DKSpinlockLock( &metadata->weakLock );
                
                rc = DKAtomicLoad32( &obj->refcount ); // Fetch again while locked
                
                if( (rc & DKRefCountMask) == 1 )
                {
                    int32_t rc_zero = rc & ~DKRefCountMask;
                    
                    if( DKAtomicCompareAndSwap32( &obj->refcount, &rc, rc_zero ) )
                    {
                        DKLifecycleWatchListReport( obj, "released" );
                        
                        metadata->weakTarget = NULL;
                        result = NULL;
                    }
                }

                DKSpinlockUnlock( &metadata->weakLock );
                
                if( result == NULL )
                {
                    DKMetadataRemove( metadata );
                    DKFinalize( _self );
                    DKDealloc( _self );
                }
            }
        }
    }

    return result;
}




// Weak References =======================================================================

///
//  DKRetainWeak()
//
DKWeakRef DKRetainWeak( DKObjectRef _self )
{
    if( _self )
    {
        DKWeakRef weakref = DKMetadataFindOrInsert( _self );

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
        DKAssertMemberOfClass( weakref, DKMetadataClass() );
    
        DKMetadataRef metadata = weakref;
    
        DKSpinlockLock( &metadata->weakLock );
        
        DKObjectRef target = DKRetain( metadata->weakTarget );
        
        DKSpinlockUnlock( &metadata->weakLock );
        
        return target;
    }
    
    return NULL;
}




// Autorelease Pools =====================================================================

///
//  DKDrainAutoreleasePool()
//
static void INTERNAL_DKDrainAutoreleasePool( struct DKThreadContext * threadContext )
{
    // Loop over this to handle the (unusual) case where an object is autoreleased within
    // the release call to another object.
    while( 1 )
    {
        DKIndex arrayLength = DKGenericArrayGetLength( &threadContext->arp.objects );
        DKRange range;
        
        if( threadContext->arp.top == -1 )
        {
            range.location = 0;
            range.length = arrayLength;
        }
        
        else
        {
            range.location = threadContext->arp.lowWater[threadContext->arp.top];
            range.length = arrayLength - range.location;
        }

        if( range.length > 0 )
        {
            #if !DKGenericArrayHasContiguousElements
            #error DKAutoreleasePool relies on contiguous elements in DKGenericArray
            #endif
            
            DKObjectRef * objects = DKGenericArrayGetPointerToElementAtIndex( &threadContext->arp.objects, range.location );

            for( DKIndex i = 0; i < range.length; ++i )
            {
                DKRelease( objects[i] );
                
                // Reset the pointer if the array contents have changed
                if( arrayLength != DKGenericArrayGetLength( &threadContext->arp.objects ) )
                {
                    arrayLength = DKGenericArrayGetLength( &threadContext->arp.objects );
                    objects = DKGenericArrayGetPointerToElementAtIndex( &threadContext->arp.objects, range.location );
                }
            }
            
            DKGenericArrayReplaceElements( &threadContext->arp.objects, range, NULL, 0 );
        }
        
        else
        {
            break;
        }
    }
}

void DKDrainAutoreleasePool( void )
{
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    DKRequire( (threadContext->arp.top >= -1) && (threadContext->arp.top < (DK_AUTORELEASE_POOL_STACK_SIZE - 1)) );

    INTERNAL_DKDrainAutoreleasePool( threadContext );
}


///
//  DKPushAutoreleasePool()
//
void DKPushAutoreleasePool( void )
{
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    DKRequire( (threadContext->arp.top >= -1) && (threadContext->arp.top < (DK_AUTORELEASE_POOL_STACK_SIZE - 1)) );

    threadContext->arp.top++;

    // Save the number of objects currently in the pool
    DKIndex count = DKGenericArrayGetLength( &threadContext->arp.objects );
    threadContext->arp.lowWater[threadContext->arp.top] = count;
}


///
//  DKPopAutoreleasePool()
//
void DKPopAutoreleasePool( void )
{
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    DKRequire( (threadContext->arp.top >= 0) && (threadContext->arp.top < DK_AUTORELEASE_POOL_STACK_SIZE) );

    INTERNAL_DKDrainAutoreleasePool( threadContext );

    threadContext->arp.top--;
}


///
//  DKAutorelease()
//
DKObjectRef DKAutorelease( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;

        // The refcount flags do not change throughout an object's lifetime so it's
        // generally safe to check them nonatomically. (The metadata flag is a special
        // case--it's set once, atomically, and inside a spinlock.)
        int32_t rc = obj->refcount; // DKAtomicLoad32( &obj->refcount )

        if( (rc & DKRefCountDisabledBit) == 0 )
        {
            struct DKThreadContext * threadContext = DKGetCurrentThreadContext();

            DKGenericArrayAppendElements( &threadContext->arp.objects, &_self, 1 );

            DKLifecycleWatchListReport( obj, "autoreleased" );
        }
    }
    
    return _self;
}





