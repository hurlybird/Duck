/*****************************************************************************************

  DKLocking.c

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

#include "DKLocking.h"


// The copying selector is initialized by DKRuntimeInit() so that constant strings can
// be used during initialization.
//DKThreadSafeFastSelectorInit( Locking );


///
//  DKLock()
//
void DKLock( DKObjectRef _self )
{
    if( _self )
    {
        DKLockingInterfaceRef lockingInterface = DKGetInterface( _self, DKSelector(Locking) );
        lockingInterface->lock( _self );
    }
}


///
//  DKTryLock()
//
bool DKTryLock( DKObjectRef _self )
{
    if( _self )
    {
        DKLockingInterfaceRef lockingInterface = DKGetInterface( _self, DKSelector(Locking) );
        return lockingInterface->tryLock( _self );
    }
    
    // Allow the lock when the locking object is NULL?
    return true;
}


///
//  DKReadLock()
//
void DKReadLock( DKObjectRef _self )
{
    if( _self )
    {
        DKLockingInterfaceRef lockingInterface = DKGetInterface( _self, DKSelector(Locking) );
        lockingInterface->readLock( _self );
    }
}


///
//  DKTryReadLock()
//
bool DKTryReadLock( DKObjectRef _self )
{
    if( _self )
    {
        DKLockingInterfaceRef lockingInterface = DKGetInterface( _self, DKSelector(Locking) );
        return lockingInterface->tryReadLock( _self );
    }
    
    // Allow the lock when the locking object is NULL?
    return true;
}


///
//  DKWriteLock()
//
void DKWriteLock( DKObjectRef _self )
{
    if( _self )
    {
        DKLockingInterfaceRef lockingInterface = DKGetInterface( _self, DKSelector(Locking) );
        lockingInterface->writeLock( _self );
    }
}


///
//  DKTryWriteLock()
//
bool DKTryWriteLock( DKObjectRef _self )
{
    if( _self )
    {
        DKLockingInterfaceRef lockingInterface = DKGetInterface( _self, DKSelector(Locking) );
        return lockingInterface->tryWriteLock( _self );
    }
    
    // Allow the lock when the locking object is NULL?
    return true;
}


///
//  DKUnlock()
//
void DKUnlock( DKObjectRef _self )
{
    if( _self )
    {
        DKLockingInterfaceRef lockingInterface = DKGetInterface( _self, DKSelector(Locking) );
        lockingInterface->unlock( _self );
    }
}




