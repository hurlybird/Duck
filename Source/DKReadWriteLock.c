/*****************************************************************************************

  DKReadWriteLock.c

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

#include "DKReadWriteLock.h"
#include "DKString.h"
#include "DKLocking.h"



struct DKReadWriteLock
{
    DKObject _obj;

#if DK_PLATFORM_POSIX
    pthread_rwlock_t rwlock;
#elif DK_PLATFORM_WINDOWS
    SRWLOCK rwlock;
    bool exclusive;
#endif
};


static DKObjectRef DKReadWriteLockInit( DKObjectRef _untyped_self );
static void DKReadWriteLockFinalize( DKObjectRef _untyped_self );

static void DKReadWriteLockMutexLock( DKReadWriteLockRef _self );


DKThreadSafeClassInit( DKReadWriteLockClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKReadWriteLock" ), DKObjectClass(), sizeof(struct DKReadWriteLock), 0, DKReadWriteLockInit, DKReadWriteLockFinalize );
    
    struct DKLockingInterface * locking = DKNewInterface( DKSelector(Locking), sizeof(struct DKLockingInterface) );
    locking->lock = (DKLockMethod)DKReadWriteLockMutexLock;
    locking->unlock = (DKUnlockMethod)DKReadWriteLockUnlock;
    
    DKInstallInterface( cls, locking );
    DKRelease( locking );
    
    return cls;
}



///
//  DKReadWriteLockInit()
//
static DKObjectRef DKReadWriteLockInit( DKObjectRef _untyped_self )
{
    DKReadWriteLockRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
#if DK_PLATFORM_POSIX
        pthread_rwlock_init( &_self->rwlock, NULL );
#elif DK_PLATFORM_WINDOWS
        InitializeSRWLock( &_self->rwlock );
#endif
    }
    
    return _self;
}


///
//  DKReadWriteLockFinalize()
//
static void DKReadWriteLockFinalize( DKObjectRef _untyped_self )
{
    DKReadWriteLockRef _self = _untyped_self;
    
#if DK_PLATFORM_POSIX
    pthread_rwlock_destroy( &_self->rwlock );
#elif DK_PLATFORM_WINDOWS
    // Nothing to do here
#endif
}


///
//  DKReadWriteLockMutexLock()
//
static void DKReadWriteLockMutexLock( DKReadWriteLockRef _self )
{
    DKReadWriteLockLock( _self, true );
}



///
//  DKReadWriteLockLock()
//
void DKReadWriteLockLock( DKReadWriteLockRef _self, bool readwrite )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKReadWriteLockClass() );
        
 #if DK_PLATFORM_POSIX
        if( readwrite )
            pthread_rwlock_wrlock( &_self->rwlock );
        
        else
            pthread_rwlock_rdlock( &_self->rwlock );
#elif DK_PLATFORM_WINDOWS
        if( readwrite )
        {
            AcquireSRWLockExclusive( &_self->rwlock );
            _self->exclusive = true;
        }

        else
        {
            AcquireSRWLockShared( &_self->rwlock );
        }
#endif
    }
}


///
//  DKReadWriteLockTryLock()
//
bool DKReadWriteLockTryLock( DKReadWriteLockRef _self, bool readwrite )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKReadWriteLockClass() );

 #if DK_PLATFORM_POSIX
        if( readwrite )
            return pthread_rwlock_trywrlock( &_self->rwlock ) == 0;
        
        else
            return pthread_rwlock_tryrdlock( &_self->rwlock ) == 0;
#elif DK_PLATFORM_WINDOWS
        if( readwrite )
        {
            if( TryAcquireSRWLockExclusive( &_self->rwlock ) )
            {
                _self->exclusive = true;
                return true;
            }
        }

        else
        {
            return TryAcquireSRWLockShared( &_self->rwlock ) != 0;
        }
#endif
    }
    
    return true;
}


///
//  DKReadWriteLockUnlock()
//
void DKReadWriteLockUnlock( DKReadWriteLockRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKReadWriteLockClass() );
 
#if DK_PLATFORM_POSIX
        pthread_rwlock_unlock( &_self->rwlock );
#elif DK_PLATFORM_WINDOWS
        if( _self->exclusive )
        {
            _self->exclusive = false;
            ReleaseSRWLockExclusive( &_self->rwlock );
        }

        else
        {
            ReleaseSRWLockShared( &_self->rwlock );
        }
#endif
    }
}



