// =======================================================================================
//
// DKSemaphore.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKSemaphore.h"
#include "DKString.h"
#include "DKLocking.h"



struct DKSemaphore
{
    DKObject _obj;
   
#if DK_PLATFORM_POSIX
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int32_t counter;
#elif DK_PLATFORM_WINDOWS
    CRITICAL_SECTION criticalSection;
    CONDITION_VARIABLE conditionVariable;
    int32_t counter;
#endif
};


static DKObjectRef DKSemaphoreInit( DKObjectRef _untyped_self );
static void DKSemaphoreFinalize( DKObjectRef _untyped_self );


DKThreadSafeClassInit( DKSemaphoreClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKSemaphore" ), DKObjectClass(), sizeof(struct DKSemaphore), 0, DKSemaphoreInit, DKSemaphoreFinalize );
       
    return cls;
}



///
//  DKSemaphoreInit()
//
static DKObjectRef DKSemaphoreInit( DKObjectRef _untyped_self )
{
    DKSemaphoreRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
#if DK_PLATFORM_POSIX
        pthread_mutex_init( &_self->mutex, NULL );
        pthread_cond_init( &_self->condition, NULL );
#elif DK_PLATFORM_WINDOWS
        InitializeCriticalSection( &_self->criticalSection );
        InitializeConditionVariable( &_self->conditionVariable );
#endif
    }
    
    return _self;
}


///
//  DKSemaphoreFinalize()
//
static void DKSemaphoreFinalize( DKObjectRef _untyped_self )
{
    DKSemaphoreRef _self = _untyped_self;
    
#if DK_PLATFORM_POSIX
    pthread_mutex_destroy( &_self->mutex );
    pthread_cond_destroy( &_self->condition );
#elif DK_PLATFORM_WINDOWS
    DeleteCriticalSection( &_self->criticalSection );
#endif
}


///
//  DKSemaphoreIncrement()
//
void DKSemaphoreIncrement( DKSemaphoreRef _self, uint32_t value )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKSemaphoreClass() );

#if DK_PLATFORM_POSIX
        pthread_mutex_lock( &_self->mutex );
        
        _self->counter += value;
        
        pthread_mutex_unlock( &_self->mutex );
        
        pthread_cond_broadcast( &_self->condition );
#elif DK_PLATFORM_WINDOWS
        EnterCriticalSection( &_self->criticalSection );

        _self->counter += value;

        LeaveCriticalSection( &_self->criticalSection );

        WakeAllConditionVariable( &_self->conditionVariable );
#endif
    }
}


///
//  DKSemaphoreDecrement()
//
void DKSemaphoreDecrement( DKSemaphoreRef _self, uint32_t value )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKSemaphoreClass() );

#if DK_PLATFORM_POSIX
        pthread_mutex_lock( &_self->mutex );
        
        DKRequire( _self->counter >= (int32_t)value );
        _self->counter -= value;
        
        pthread_mutex_unlock( &_self->mutex );
        
        pthread_cond_broadcast( &_self->condition );
#elif DK_PLATFORM_WINDOWS
        EnterCriticalSection( &_self->criticalSection );

        DKRequire( _self->counter >= (int32_t)value );
        _self->counter -= value;

        LeaveCriticalSection( &_self->criticalSection );

        WakeAllConditionVariable( &_self->conditionVariable );
#endif
    }
}


///
//  DKSemaphoreWait()
//
void DKSemaphoreWait( DKSemaphoreRef _self, uint32_t value )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKSemaphoreClass() );

#if DK_PLATFORM_POSIX
        pthread_mutex_lock( &_self->mutex );
        
        while( _self->counter != (int32_t)value )
            pthread_cond_wait( &_self->condition, &_self->mutex );
        
        pthread_mutex_unlock( &_self->mutex );
#elif DK_PLATFORM_WINDOWS
        EnterCriticalSection( &_self->criticalSection );

        while( _self->counter != (int32_t)value )
            SleepConditionVariableCS( &_self->conditionVariable, &_self->criticalSection, INFINITE );

        LeaveCriticalSection( &_self->criticalSection );
#endif
    }
}

