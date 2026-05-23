// =======================================================================================
//
// DKReadWriteLock.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
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
    
    struct DKLockingInterface * locking = DKNewInterface( DKSelector(Locking) );
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
#if DK_PLATFORM_POSIX
    DKReadWriteLockRef _self = _untyped_self;
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



