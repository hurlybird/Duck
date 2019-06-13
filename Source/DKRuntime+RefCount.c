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
#include "DKString.h"
#include "DKGenericArray.h"
#include "DKThread.h"



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

        int32_t rc = obj->refcount;

        if( (rc & DKRefCountDisabledBit) == 0 )
        {
            if( (rc & DKRefCountMetadataBit) == 0 )
            {
                rc = DKAtomicDecrement32( &obj->refcount );
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
                
                DKSpinLockLock( &metadata->weakLock );
                
                rc = DKAtomicDecrement32( &obj->refcount );
                DKAssert( (rc & DKRefCountOverflowBit) == 0 );

                if( (rc & DKRefCountMask) == 0 )
                    metadata->weakTarget = NULL;

                DKSpinLockUnlock( &metadata->weakLock );
                
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

        int32_t rc = obj->refcount;

        if( (rc & DKRefCountDisabledBit) == 0 )
        {
            if( (rc & DKRefCountMetadataBit) == 0 )
            {
                if( (rc & DKRefCountMask) == 1 )
                {
                    int32_t rc_zero = rc & ~DKRefCountMask;
                    
                    if( DKAtomicCmpAndSwap32( &obj->refcount, rc, rc_zero ) )
                    {
                        DKFinalize( _self );
                        DKDealloc( _self );

                        result = NULL;
                    }
                }
            }
            
            else
            {
                DKMetadataRef metadata = DKMetadataFindOrInsert( obj );
                
                DKSpinLockLock( &metadata->weakLock );
                
                rc = obj->refcount; // Fetch again while locked
                
                if( (rc & DKRefCountMask) == 1 )
                {
                    int32_t rc_zero = rc & ~DKRefCountMask;
                    
                    if( DKAtomicCmpAndSwap32( &obj->refcount, rc, rc_zero ) )
                    {
                        metadata->weakTarget = NULL;
                        result = NULL;
                    }
                }

                DKSpinLockUnlock( &metadata->weakLock );
                
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
    
        DKSpinLockLock( &metadata->weakLock );
        
        DKObjectRef target = DKRetain( metadata->weakTarget );
        
        DKSpinLockUnlock( &metadata->weakLock );
        
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
        DKIndex count = DKGenericArrayGetLength( &threadContext->arp.objects );
        DKRange range;
        
        if( threadContext->arp.top == -1 )
        {
            range.location = 0;
            range.length = count;
        }
        
        else
        {
            range.location = threadContext->arp.count[threadContext->arp.top];
            range.length = count - range.location;
        }

        if( range.length > 0 )
        {
            #if !DKGenericArrayHasContiguousElements
            #error DKAutoreleasePool relies on contiguous elements in DKGenericArray
            #endif
            
            DKObjectRef * objects = DKGenericArrayGetPointerToElementAtIndex( &threadContext->arp.objects, range.location );

            for( DKIndex i = 0; i < range.length; ++i )
                DKRelease( objects[i] );
            
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
    
    if( threadContext->arp.top >= 0 )
    {
        // Save the number of objects currently in the pool
        DKIndex count = DKGenericArrayGetLength( &threadContext->arp.objects );
        threadContext->arp.count[threadContext->arp.top] = count;
    }

    threadContext->arp.top++;
}


///
//  DKPopAutoreleasePool()
//
void DKPopAutoreleasePool( void )
{
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    DKRequire( (threadContext->arp.top >= 0) && (threadContext->arp.top < DK_AUTORELEASE_POOL_STACK_SIZE) );

    threadContext->arp.top--;

    INTERNAL_DKDrainAutoreleasePool( threadContext );
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

            DKGenericArrayAppendElements( &threadContext->arp.objects, &_self, 1 );
        }
    }
    
    return _self;
}





