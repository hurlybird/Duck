/*****************************************************************************************

  DKMutex.c

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
    
    struct DKLockingInterface * locking = DKNewInterface( DKSelector(Locking), sizeof(struct DKLockingInterface) );
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



