// =======================================================================================
//
// DKMutex.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#define DK_THREAD_PRIVATE 1

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKMutex.h"
#include "DKString.h"
#include "DKLocking.h"



static void DKMutexFinalize( DKObjectRef _untyped_self );



DKThreadSafeClassInit( DKMutexClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKMutex" ), DKObjectClass(), sizeof(struct DKMutex), 0, DKMutexInit, DKMutexFinalize );
    
    struct DKLockingInterface * locking = DKNewInterface( DKSelector(Locking) );
    locking->lock = (DKLockMethod)DKMutexLock;
    locking->unlock = (DKUnlockMethod)DKMutexUnlock;
    
    DKInstallInterface( cls, locking );
    DKRelease( locking );
    
    return cls;
}



///
//  DKMutexInit()
//
DKObjectRef DKMutexInit( DKObjectRef _untyped_self )
{
    DKMutexRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
#if DK_PLATFORM_POSIX
        pthread_mutex_init( &_self->mutex, NULL );
#elif DK_PLATFORM_WINDOWS
        InitializeCriticalSection( &_self->criticalSection );
#endif
        
    }
    
    return _self;
}


///
//  DKRecursiveMutexInit()
//
DKObjectRef DKRecursiveMutexInit( DKObjectRef _untyped_self )
{
    DKMutexRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
#if DK_PLATFORM_POSIX
        pthread_mutexattr_t recursiveAttributes;
        pthread_mutexattr_init( &recursiveAttributes );
        pthread_mutexattr_settype( &recursiveAttributes, PTHREAD_MUTEX_RECURSIVE );
        pthread_mutex_init( &_self->mutex, &recursiveAttributes );
        pthread_mutexattr_destroy( &recursiveAttributes );
#elif DK_PLATFORM_WINDOWS
        InitializeCriticalSection( &_self->criticalSection );
#endif
    }
    
    return _self;
}


///
//  DKMutexFinalize()
//
static void DKMutexFinalize( DKObjectRef _untyped_self )
{
    DKMutexRef _self = _untyped_self;

#if DK_PLATFORM_POSIX
    pthread_mutex_destroy( &_self->mutex );
#elif DK_PLATFORM_WINDOWS
    DeleteCriticalSection( &_self->criticalSection );
#endif
}


///
//  DKMutexLock()
//
void DKMutexLock( DKMutexRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutexClass() );

#if DK_PLATFORM_POSIX
        pthread_mutex_lock( &_self->mutex );
#elif DK_PLATFORM_WINDOWS
        EnterCriticalSection( &_self->criticalSection );
#endif
    }
}


///
//  DKMutexTryLock()
//
bool DKMutexTryLock( DKMutexRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutexClass() );

#if DK_PLATFORM_POSIX
        return pthread_mutex_trylock( &_self->mutex ) == 0;
#elif DK_PLATFORM_WINDOWS
        return TryEnterCriticalSection( &_self->criticalSection );
#endif
    }
    
    return true;
}


///
//  DKMutexUnlock()
//
void DKMutexUnlock( DKMutexRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutexClass() );

#if DK_PLATFORM_POSIX
        pthread_mutex_unlock( &_self->mutex );
#elif DK_PLATFORM_WINDOWS
        LeaveCriticalSection( &_self->criticalSection );
#endif


    }
}



