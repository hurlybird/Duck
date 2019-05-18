/*****************************************************************************************

  DKCondition.c

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

#include "DKCondition.h"
#include "DKString.h"



struct DKCondition
{
    DKObject _obj;

#if DK_PLATFORM_POSIX
    pthread_cond_t condition;
#elif DK_PLATFORM_WINDOWS
    CONDITION_VARIABLE conditionVariable;
#endif
};


static DKObjectRef DKConditionInit( DKObjectRef _untyped_self );
static void DKConditionFinalize( DKObjectRef _untyped_self );


DKThreadSafeClassInit( DKConditionClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKCondition" ), DKObjectClass(), sizeof(struct DKCondition), 0, DKConditionInit, DKConditionFinalize );
    
    return cls;
}



///
//  DKMutexInit()
//
DKObjectRef DKConditionInit( DKObjectRef _untyped_self )
{
    DKConditionRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
#if DK_PLATFORM_POSIX
        pthread_cond_init( &_self->condition, NULL );
#elif DK_PLATFORM_WINDOWS
        InitializeConditionVariable( &_self->conditionVariable );
#endif
    }
    
    return _self;
}


///
//  DKConditionFinalize()
//
static void DKConditionFinalize( DKObjectRef _untyped_self )
{
    DKConditionRef _self = _untyped_self;
    
#if DK_PLATFORM_POSIX
    pthread_cond_destroy( &_self->condition );
#elif DK_PLATFORM_WINDOWS
    // Nothing to do here
#endif
}


///
//  DKConditionWait()
//
void DKConditionWait( DKConditionRef _self, DKMutexRef mutex )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKConditionClass() );
        DKAssertKindOfClass( mutex, DKMutexClass() );
    
#if DK_PLATFORM_POSIX
        pthread_cond_wait( &_self->condition, &mutex->mutex );
#elif DK_PLATFORM_WINDOWS
        SleepConditionVariableCS( &_self->conditionVariable, &mutex->criticalSection, INFINITE );
#endif
    }
}


///
//  DKConditionSignal()
//
void DKConditionSignal( DKConditionRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKConditionClass() );
        
#if DK_PLATFORM_POSIX
        pthread_cond_signal( &_self->condition );
#elif DK_PLATFORM_WINDOWS
        WakeConditionVariable( &_self->conditionVariable );
#endif
    }
}


///
//  DKConditionSignalAll()
//
void DKConditionSignalAll( DKConditionRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKConditionClass() );
        
#if DK_PLATFORM_POSIX
        pthread_cond_broadcast( &_self->condition );
#elif DK_PLATFORM_WINDOWS
        WakeAllConditionVariable( &_self->conditionVariable );
#endif
    }
}




