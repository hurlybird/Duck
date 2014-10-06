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




// Strong References =====================================================================

///
//  DKRetain()
//
DKObjectRef DKRetain( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = (DKObject *)_self;

        if( (obj->isa->options & DKDisableReferenceCounting) == 0 )
        {
            DKAtomicIncrement32( &obj->refcount );
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
        DKObject * obj = (DKObject *)_self;

        if( (obj->isa->options & DKDisableReferenceCounting) == 0 )
        {
            struct DKWeak * weakref = (struct DKWeak *)obj->weakref;
            
            int32_t n;
            
            if( weakref == NULL )
            {
                n = DKAtomicDecrement32( &obj->refcount );
                DKAssert( n >= 0 );
            }
            
            else
            {
                DKSpinLockLock( &weakref->lock );
                
                n = DKAtomicDecrement32( &obj->refcount );
                DKAssert( n >= 0 );

                if( n == 0 )
                {
                    obj->weakref = NULL;
                    weakref->target = NULL;
                }
                
                DKSpinLockUnlock( &weakref->lock );
            }
            
            if( n == 0 )
            {
                DKRelease( weakref );
                DKFinalize( _self );
                DKDealloc( _self );
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
        const DKObject * obj = _self;
        
        // It doesn't make sense to get a weak reference to a weak reference.
        if( obj->isa == DKWeakClass() )
        {
            return DKRetain( obj );
        }
        
        if( !obj->weakref )
        {
            struct DKWeak * weakref = DKCreate( DKWeakClass() );
            
            weakref->lock = DKSpinLockInit;
            weakref->target = obj;
            
            if( !DKAtomicCmpAndSwapPtr( (void * volatile *)&obj->weakref, NULL, weakref ) )
                DKRelease( weakref );
        }
        
        return DKRetain( obj->weakref );
    }
    
    return NULL;
}


///
//  DKResolveWeak()
//
DKObjectRef DKResolveWeak( DKWeakRef weak_ref )
{
    if( weak_ref )
    {
        struct DKWeak * weakref = (struct DKWeak *)weak_ref;
        
        if( weakref->target )
        {
            DKSpinLockLock( &weakref->lock );
            
            DKObjectRef target = DKRetain( weakref->target );
            
            DKSpinLockUnlock( &weakref->lock );
            
            return target;
        }
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
        struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
        DKFatal( (threadContext->arpStack.top >= 0) && (threadContext->arpStack.top < DK_AUTORELEASE_POOL_STACK_SIZE) );

        DKGenericArray * arp = &threadContext->arpStack.arp[threadContext->arpStack.top];
        DKGenericArrayAppendElements( arp, &_self, 1 );
    }
    
    return _self;
}





