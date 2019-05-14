/*****************************************************************************************

  DKThreadPool.c

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

#include "DKThreadPool.h"
#include "DKNodePool.h"
#include "DKString.h"
#include "DKMutex.h"
#include "DKCondition.h"
#include "DKSemaphore.h"
#include "DKCopying.h"


struct DKThreadPoolTask
{
    struct DKThreadPoolTask * next;

    DKThreadProc proc;
    void * context;
    
    DKObjectRef target;
    DKThreadMethod method;
    DKObjectRef param;
};


struct DKThreadPool
{
    DKObject _obj;
    
    DKNodePool nodePool;
    
    DKStringRef label;
    
    DKMutableListRef threads;
    DKMutexRef controlMutex;
    DKConditionRef controlCondition;

    struct DKThreadPoolTask * queue;
    DKMutexRef queueMutex;
    DKConditionRef queueCondition;
    
    DKSemaphoreRef stopCounter;
    
    DKThreadPoolCallback onThreadStart;
    DKThreadPoolCallback onThreadStop;
    void * onThreadStartStopContext;
    
    volatile int32_t pendingTasks;
    volatile int32_t idleThreads;
};


static DKObjectRef DKThreadPoolInit( DKObjectRef _untyped_self );
static void DKThreadPoolFinalize( DKObjectRef _untyped_self );


DKThreadSafeClassInit( DKThreadPoolClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKThreadPool" ), DKObjectClass(), sizeof(struct DKThreadPool), 0, DKThreadPoolInit, DKThreadPoolFinalize );
    
    return cls;
}


///
//  DKThreadPoolInit()
//
static DKObjectRef DKThreadPoolInit( DKObjectRef _untyped_self )
{
    DKThreadPoolRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        DKNodePoolInit( &_self->nodePool, sizeof(struct DKThreadPoolTask), 16 );
        
        _self->threads = DKNewMutableList();
        
        _self->controlMutex = DKNewMutex();
        _self->controlCondition = DKNewCondition();
        
        _self->queueMutex = DKNewMutex();
        _self->queueCondition = DKNewCondition();
        
        _self->stopCounter = DKNewSemaphore();
    }
    
    return _self;
}


///
//  DKThreadPoolFinalize()
//
static void DKThreadPoolFinalize( DKObjectRef _untyped_self )
{
    DKThreadPoolRef _self = _untyped_self;
    
    DKThreadPoolStop( _self );
    DKThreadPoolRemoveAllTasks( _self );
    
    DKRelease( _self->threads );
    DKRelease( _self->controlMutex );
    DKRelease( _self->controlCondition );
    DKRelease( _self->queueMutex );
    DKRelease( _self->queueCondition );
    DKRelease( _self->stopCounter );
    DKRelease( _self->label );

    DKNodePoolFinalize( &_self->nodePool );
}


///
//  DKThreadPoolSetCallbacks()
//
void DKThreadPoolSetCallbacks( DKThreadPoolRef _self,
    DKThreadPoolCallback onThreadStart,
    DKThreadPoolCallback onThreadStop,
    void * context )
{
    _self->onThreadStart = onThreadStart;
    _self->onThreadStop = onThreadStop;
    _self->onThreadStartStopContext = context;
}


///
//  DKThreadPoolSetLabel()
//
void DKThreadPoolSetLabel( DKThreadPoolRef _self, DKStringRef label )
{
    label = DKCopy( label );
    DKRelease( _self->label );
    _self->label = label;
}


///
//  DKThreadPoolAllocTask()
//
static struct DKThreadPoolTask * DKThreadPoolAllocTask( DKThreadPoolRef _self, DKThreadProc proc, void * context )
{
    struct DKThreadPoolTask * task = DKNodePoolAlloc( &_self->nodePool );

    memset( task, 0, sizeof(struct DKThreadPoolTask) );
    task->proc = proc;
    task->context = context;

    DKAtomicIncrement32( &_self->pendingTasks );
    
    return task;
}


///
//  DKThreadPoolAllocObjectTask()
//
static struct DKThreadPoolTask * DKThreadPoolAllocObjectTask( DKThreadPoolRef _self, DKObjectRef target, DKThreadMethod method, DKObjectRef param )
{
    struct DKThreadPoolTask * task = DKNodePoolAlloc( &_self->nodePool );

    memset( task, 0, sizeof(struct DKThreadPoolTask) );
    task->target = DKRetain( target );
    task->method = method;
    task->param = DKRetain( param );

    DKAtomicIncrement32( &_self->pendingTasks );

    return task;
}


///
//  DKThreadPoolFreeTasks()
//
static void DKThreadPoolFreeTasks( DKThreadPoolRef _self, struct DKThreadPoolTask * task )
{
    while( task )
    {
        struct DKThreadPoolTask * next = task->next;
        
        DKRelease( task->target );
        DKRelease( task->param );
        
        DKNodePoolFree( &_self->nodePool, task );
        
        task = next;

        DKAtomicDecrement32( &_self->pendingTasks );
        
        DKAssert( _self->pendingTasks >= 0 );
    }
}


///
//  DKThreadPoolAddTaskToQueue()
//
static void DKThreadPoolAddTaskToQueue( DKThreadPoolRef _self, struct DKThreadPoolTask * task )
{
    DKAssert( task->next == NULL );

    if( _self->queue == NULL )
    {
        _self->queue = task;
    }
    
    else
    {
        struct DKThreadPoolTask * end = _self->queue;
        
        while( end->next )
            end = end->next;
        
        end->next = task;
    }
}


///
//  DKThreadPoolExec()
//
static void DKThreadPoolExec( void * _untyped_self )
{
    DKThreadPoolRef _self = _untyped_self;
    DKThreadRef thread = DKThreadGetCurrentThread();

    if( _self->onThreadStart )
    {
        _self->onThreadStart( _self, _self->onThreadStartStopContext );
    }

    DKMutexLock( _self->queueMutex );
    
    while( DKThreadGetState( thread ) != DKThreadCancelled )
    {
        if( _self->queue == NULL )
        {
            DKAtomicIncrement32( &_self->idleThreads );
            DKConditionSignal( _self->controlCondition );

            DKConditionWait( _self->queueCondition, _self->queueMutex );

            DKAtomicDecrement32( &_self->idleThreads );
        }

        else
        {
            struct DKThreadPoolTask * task = _self->queue;
            _self->queue = task->next;
            task->next = NULL;
            
            DKMutexUnlock( _self->queueMutex );

            DKPushAutoreleasePool();
            
            if( task->proc )
                task->proc( task->context );
            
            else
                task->method( task->target, task->param );
            
            DKPopAutoreleasePool();

            DKMutexLock( _self->queueMutex );

            DKThreadPoolFreeTasks( _self, task );
        }
    }

    DKMutexUnlock( _self->queueMutex );

    if( _self->onThreadStop )
    {
        _self->onThreadStop( _self, _self->onThreadStartStopContext );
    }
    
    DKSemaphoreDecrement( _self->stopCounter, 1 );
}


///
//  DKThreadPoolStart()
//
int DKThreadPoolStart( DKThreadPoolRef _self, int numThreads )
{
    if( _self )
    {
        DKMutexLock( _self->controlMutex );
    
        int runningThreads = (int)DKListGetCount( _self->threads );
        int startedThreads = numThreads - runningThreads;
        
        if( startedThreads < 0 )
            startedThreads = 0;
        
        for( int i = 0; i < startedThreads; i++ )
        {
            DKThreadRef thread = DKThreadInit( DKAlloc( DKThreadClass() ), DKThreadPoolExec, _self );
            
            if( _self->label )
            {
                int x = (int)DKListGetCount( _self->threads );
                DKStringRef threadLabel = DKNewStringWithFormat( "%@[%d]", _self->label, x );
                DKThreadSetLabel( thread, threadLabel );
                DKRelease( threadLabel );
            }
            
            DKThreadStart( thread );
            
            DKListAppendObject( _self->threads, thread );
            DKRelease( thread );
        }
        
        DKMutexUnlock( _self->controlMutex );
        
        return startedThreads;
    }
    
    return 0;
}


///
//  DKThreadPoolStop()
//
void DKThreadPoolStop( DKThreadPoolRef _self )
{
    if( _self )
    {
        DKMutexLock( _self->controlMutex );

        // Note: pthread_join doesn't seem to be reliable enough to properly wait for
        // cancelled threads to finish, therefore this uses a semaphore to instead.
        
        // We DON'T want to use the control condition/mutex to watch the cancelled threads
        // because that would allow someone else to start new threads while we're trying
        // to stop the old ones.

        // Reset the stopped threads counter
        DKIndex threadCount = DKListGetCount( _self->threads );
        DKSemaphoreIncrement( _self->stopCounter, (uint32_t)threadCount );

        // Issue the cancel command
        for( DKIndex i = 0; i < threadCount; i++ )
        {
            DKThreadRef thread = DKListGetObjectAtIndex( _self->threads, i );
            DKThreadCancel( thread );
        }
        
        // Wake up any waiting threads
        DKConditionSignalAll( _self->queueCondition );

        // Wait for the threads to finish
        DKSemaphoreWait( _self->stopCounter, 0 );

        // Release the thread objects
        DKListRemoveAllObjects( _self->threads );

        DKMutexUnlock( _self->controlMutex );
    }
}


///
//  DKThreadPoolGetThreadCount()
//
int DKThreadPoolGetThreadCount( DKThreadPoolRef _self )
{
    if( _self )
        return (int)DKListGetCount( _self->threads );
    
    return 0;
}


///
//  DKThreadPoolIsBusy()
//
int DKThreadPoolIsBusy( DKThreadPoolRef _self )
{
    int busy = 0;

    if( _self )
    {
        DKMutexLock( _self->queueMutex );

        busy = _self->pendingTasks;

        DKMutexUnlock( _self->queueMutex );
    }
    
    return busy;
}


///
//  DKThreadPoolWaitUntilIdle()
//
void DKThreadPoolWaitUntilIdle( DKThreadPoolRef _self )
{
    if( _self )
    {
        DKMutexLock( _self->controlMutex );
        
        while( _self->idleThreads < DKListGetCount( _self->threads ) )
            DKConditionWait( _self->controlCondition, _self->controlMutex );
        
        DKMutexUnlock( _self->controlMutex );
    }
}


///
//  DKThreadPoolAddTask()
//
void DKThreadPoolAddTask( DKThreadPoolRef _self, DKThreadProc proc, void * context )
{
    if( _self )
    {
        DKMutexLock( _self->queueMutex );

        struct DKThreadPoolTask * task = DKThreadPoolAllocTask( _self, proc, context );
        DKThreadPoolAddTaskToQueue( _self, task );
        
        DKMutexUnlock( _self->queueMutex );
        DKConditionSignal( _self->queueCondition );
    }
}


///
//  DKThreadPoolAddObjectTask()
//
void DKThreadPoolAddObjectTask( DKThreadPoolRef _self, DKObjectRef target, DKThreadMethod method, DKObjectRef param )
{
    if( _self )
    {
        DKMutexLock( _self->queueMutex );

        struct DKThreadPoolTask * task = DKThreadPoolAllocObjectTask( _self, target, method, param );
        DKThreadPoolAddTaskToQueue( _self, task );
        
        DKMutexUnlock( _self->queueMutex );
        DKConditionSignal( _self->queueCondition );
    }
}


///
//  DKThreadPoolRemoveAllTasks()
//
void DKThreadPoolRemoveAllTasks( DKThreadPoolRef _self )
{
    if( _self )
    {
        DKMutexLock( _self->queueMutex );
        
        DKThreadPoolFreeTasks( _self, _self->queue );
        _self->queue = NULL;
        
        DKMutexUnlock( _self->queueMutex );
    }
}





