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


#ifndef DK_THREADPOOL_DIAGNOSTIC_OUTPUT
#define DK_THREADPOOL_DIAGNOSTIC_OUTPUT     0
#endif

#define DK_THREADPOOL_DEFAULT_YIELD_TIME    (100 * 1000)   // 100 us


struct DKThreadPoolTask
{
    struct DKThreadPoolTask * next;

    DKThreadProc proc;
    void * context;
    
    DKObjectRef target;
    DKThreadMethod method;
    DKObjectRef param;

    int64_t taskGroup;
};

struct DKThreadPoolQueue
{
    struct DKThreadPoolQueue * next;

    struct DKThreadPoolTask * tasks;
    struct DKThreadPoolTask * completions;

    int64_t taskGroup;
    int32_t pendingTasks;
    bool closed;
};

struct DKThreadPool
{
    DKObject _obj;
    
    DKNodePool nodePool;
    
    DKStringRef label;
    
    DKMutableListRef threads;
    DKMutexRef threadsMutex;
    
    struct DKThreadPoolQueue * queues;
    DKMutexRef queuesMutex;

    DKConditionRef workAvailableCondition;  // Signalled when work is added to the queue
    DKConditionRef stateChangedCondition;   // Signalled when a worker thread goes idle, a task group is completed, etc.
    
    DKSemaphoreRef stopCounter;
    
    DKThreadPoolCallback onThreadStart;
    DKThreadPoolCallback onThreadStop;
    void * onThreadStartStopContext;

    volatile int32_t idleThreads;
    volatile int32_t standbyThreads;

    DKThreadPoolScheduling scheduling;
    uint32_t yieldNSecs;

    int64_t nextTaskGroup;
};


static DKObjectRef DKThreadPoolInit( DKObjectRef _untyped_self );
static void DKThreadPoolFinalize( DKObjectRef _untyped_self );


DKThreadSafeClassInit( DKThreadPoolClass )
{
    _Static_assert( sizeof(struct DKThreadPoolQueue) <= sizeof(struct DKThreadPoolTask), "DKThreadPoolQueue struct size inconsistency." );

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
        
        _self->threadsMutex = DKNewMutex();
        _self->queuesMutex = DKNewMutex();

        _self->workAvailableCondition = DKNewCondition();
        _self->stateChangedCondition = DKNewCondition();
        
        _self->stopCounter = DKNewSemaphore();
        
        _self->scheduling = DKThreadPoolDefaultScheduling;
        _self->yieldNSecs = DK_THREADPOOL_DEFAULT_YIELD_TIME;
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
    DKRelease( _self->threadsMutex );
    DKRelease( _self->queuesMutex );
    DKRelease( _self->workAvailableCondition );
    DKRelease( _self->stateChangedCondition );
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
//  DKThreadPoolSetScheduling()
//
void DKThreadPoolSetScheduling( DKThreadPoolRef _self, DKThreadPoolScheduling scheduling, int yieldNSecs )
{
    _self->scheduling = scheduling;
    _self->yieldNSecs = (yieldNSecs >= 0) ? yieldNSecs : DK_THREADPOOL_DEFAULT_YIELD_TIME;
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

    #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
    fprintf( stderr, "DKThreadPool %s: allocating task %"PRIxPTR"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", (intptr_t)task );
    #endif

    memset( task, 0, sizeof(struct DKThreadPoolTask) );
    task->proc = proc;
    task->context = context;
    
    return task;
}


///
//  DKThreadPoolAllocObjectTask()
//
static struct DKThreadPoolTask * DKThreadPoolAllocObjectTask( DKThreadPoolRef _self, DKObjectRef target, DKThreadMethod method, DKObjectRef param )
{
    struct DKThreadPoolTask * task = DKNodePoolAlloc( &_self->nodePool );

    #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
    fprintf( stderr, "DKThreadPool %s: allocating task (method) %"PRIxPTR"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", (intptr_t)task );
    #endif

    memset( task, 0, sizeof(struct DKThreadPoolTask) );
    task->target = DKRetain( target );
    task->method = method;
    task->param = DKRetain( param );

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
        
        #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
        fprintf( stderr, "DKThreadPool %s: freeing task %"PRIxPTR"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", (intptr_t)task );
        #endif

        DKNodePoolFree( &_self->nodePool, task );
        
        task = next;
    }
}


///
//  DKThreadPoolAllocQueue()
//
static struct DKThreadPoolQueue * DKThreadPoolAllocQueue( DKThreadPoolRef _self )
{
    struct DKThreadPoolQueue * queue = DKNodePoolAlloc( &_self->nodePool );

    memset( queue, 0, sizeof(struct DKThreadPoolQueue) );

    queue->taskGroup = DKAtomicIncrement64( &_self->nextTaskGroup );
    
    #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
    fprintf( stderr, "DKThreadPool %s: allocating queue %"PRIxPTR" for group %"PRIi64"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", (intptr_t)queue, queue->taskGroup );
    #endif

    return queue;
}


///
//  DKThreadPoolFreeQueues()
//
static void DKThreadPoolFreeQueues( DKThreadPoolRef _self, struct DKThreadPoolQueue * queue )
{
    while( queue )
    {
        struct DKThreadPoolQueue * next = queue->next;
        
        DKThreadPoolFreeTasks( _self, queue->tasks );
        DKThreadPoolFreeTasks( _self, queue->completions );
        
        #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
        fprintf( stderr, "DKThreadPool %s: freeing queue %"PRIxPTR" for group %"PRIi64"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", (intptr_t)queue, queue->taskGroup );
        #endif

        DKNodePoolFree( &_self->nodePool, queue );
        
        queue = next;
    }
}


///
//  DKThreadPoolGetQueue()
//
static struct DKThreadPoolQueue * DKThreadPoolGetQueue( DKThreadPoolRef _self, int64_t taskGroup )
{
    struct DKThreadPoolQueue * queue = _self->queues;

    while( queue != NULL )
    {
        if( queue->taskGroup == taskGroup )
            return queue;
            
        queue = queue->next;
    }
        
    return NULL;
}


///
//  DKThreadPoolGetLastQueue()
//
static struct DKThreadPoolQueue * DKThreadPoolGetLastQueue( DKThreadPoolRef _self, bool createIfNotFound )
{
    if( _self->queues )
    {
        struct DKThreadPoolQueue * queue = _self->queues;

        while( queue->next != NULL )
            queue = queue->next;

        return queue;
    }
    
    else if( createIfNotFound )
    {
        _self->queues = DKThreadPoolAllocQueue( _self );
        return _self->queues;
    }

    return NULL;
}


///
//  DKThreadPoolGetOpenQueue()
//
static struct DKThreadPoolQueue * DKThreadPoolGetOpenQueue( DKThreadPoolRef _self )
{
    struct DKThreadPoolQueue * queue = DKThreadPoolGetLastQueue( _self, true );
    
    if( !queue->closed )
        return queue;
        
    queue->next = DKThreadPoolAllocQueue( _self );
    return queue->next;
}


///
//  DKThreadPoolAddTaskToList()
//
static void DKThreadPoolAddTaskToList( struct DKThreadPoolTask ** list, struct DKThreadPoolTask * task )
{
    DKAssert( task->next == NULL );

    if( *list == NULL )
    {
        *list = task;
    }

    else
    {
        struct DKThreadPoolTask * end = *list;
        
        while( end->next )
            end = end->next;
        
        end->next = task;
    }
}


///
//  DKThreadPoolScheduleTask()
//
static int64_t DKThreadPoolScheduleTask( DKThreadPoolRef _self, struct DKThreadPoolTask * task )
{
    struct DKThreadPoolQueue * queue = DKThreadPoolGetOpenQueue( _self );

    task->taskGroup = queue->taskGroup;

    #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
    fprintf( stderr, "DKThreadPool %s: scheduling task %"PRIxPTR" in group %"PRIi64"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", (intptr_t)task, queue->taskGroup );
    #endif

    DKThreadPoolAddTaskToList( &queue->tasks, task );
    
    DKAtomicIncrement32( &queue->pendingTasks );
    
    return queue->taskGroup;
}


///
//  DKThreadPoolScheduleCompletion()
//
static int64_t DKThreadPoolScheduleCompletion( DKThreadPoolRef _self, struct DKThreadPoolTask * completion )
{
    struct DKThreadPoolQueue * queue = DKThreadPoolGetLastQueue( _self, true );

    completion->taskGroup = 0;

    #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
    fprintf( stderr, "DKThreadPool %s: scheduling completion %"PRIxPTR" for group %"PRIi64"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", (intptr_t)completion, queue->taskGroup );
    #endif

    DKThreadPoolAddTaskToList( &queue->completions, completion );
    queue->closed = true;
    
    return queue->taskGroup;
}


///
//  DKThreadPoolGetNextTask()
//
static struct DKThreadPoolTask * DKThreadPoolGetNextTask( DKThreadPoolRef _self )
{
    struct DKThreadPoolQueue ** cursor = &_self->queues;
    
    while( *cursor )
    {
        struct DKThreadPoolQueue * queue = *cursor;
        
        // If there's a task available, run it
        if( queue->tasks )
        {
            struct DKThreadPoolTask * task = queue->tasks;
            queue->tasks = task->next;
            task->next = NULL;
            
            #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
            fprintf( stderr, "DKThreadPool %s: running task %"PRIxPTR" in group %"PRIi64"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", (intptr_t)task, queue->taskGroup );
            #endif
        
            return task;
        }
        
        // If all the tasks in the queue's task group are done...
        else if( queue->pendingTasks == 0 )
        {
            // If there's a completion available, run it
            if( queue->completions )
            {
                struct DKThreadPoolTask * completion = queue->completions;
                queue->completions = completion->next;
                completion->next = NULL;
                
                #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
                fprintf( stderr, "DKThreadPool %s: running completion %"PRIxPTR" in group %"PRIi64"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", (intptr_t)completion, queue->taskGroup );
                #endif

                return completion;
            }
            
            // ...otherwise free the queue and signal the task group completion
            else
            {
                #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
                fprintf( stderr, "DKThreadPool %s: completed group %"PRIi64"\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "", queue->taskGroup );
                #endif
                
                *cursor = queue->next;
                queue->next = NULL;
                
                DKThreadPoolFreeQueues( _self, queue );
                DKConditionSignalAll( _self->stateChangedCondition );
            }
        }
        
        else
        {
            cursor = &queue->next;
        }
    }
    
    #if DK_THREADPOOL_DIAGNOSTIC_OUTPUT
    fprintf( stderr, "DKThreadPool %s: queue is empty\n", _self->label ? DKStringGetCStringPtr( _self->label ) : "" );
    #endif

    return NULL;
}


///
//  DKThreadPoolExecuteTask()
//
static void DKThreadPoolExecuteTask( DKThreadPoolRef _self, struct DKThreadPoolTask * task )
{
    DKPushAutoreleasePool();
    
    if( task->proc )
        task->proc( task->context );
    
    else
        task->method( task->target, task->param );
    
    DKPopAutoreleasePool();
}


///
//  DKThreadPoolCompleteTask()
//
static void DKThreadPoolCompleteTask( DKThreadPoolRef _self, struct DKThreadPoolTask * task )
{
    if( task->taskGroup )
    {
        struct DKThreadPoolQueue * queue = DKThreadPoolGetQueue( _self, task->taskGroup );

        #if DIAGNOSTIC_OUTPUT
        fprintf( stderr, "DKThreadPool: completed task %"PRIXPTR" in group %"PRIi64"\n", (intptr_t)task, queue->taskGroup );
        #endif

        int32_t pending = DKAtomicDecrement32( &queue->pendingTasks );
        DKRequire( pending >= 0 );
    }

    DKThreadPoolFreeTasks( _self, task );
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

    DKMutexLock( _self->queuesMutex );
    
    while( DKThreadGetState( thread ) != DKThreadCancelled )
    {
        struct DKThreadPoolTask * task = DKThreadPoolGetNextTask( _self );

        if( task )
        {
            DKMutexUnlock( _self->queuesMutex );
            
            DKThreadPoolExecuteTask( _self, task );

            DKMutexLock( _self->queuesMutex );

            DKThreadPoolCompleteTask( _self, task );
        }

        else
        {
            DKAtomicIncrement32( &_self->idleThreads );
            DKConditionSignalAll( _self->stateChangedCondition );

            DKConditionWait( _self->workAvailableCondition, _self->queuesMutex );

            DKAtomicDecrement32( &_self->idleThreads );
        }
    }

    DKMutexUnlock( _self->queuesMutex );

    if( _self->onThreadStop )
    {
        _self->onThreadStop( _self, _self->onThreadStartStopContext );
    }
    
    DKSemaphoreDecrement( _self->stopCounter, 1 );
}


///
//  DKThreadPoolRealTimeExec()
//
static void DKThreadPoolRealTimeExec( void * _untyped_self )
{
    DKThreadPoolRef _self = _untyped_self;
    DKThreadRef thread = DKThreadGetCurrentThread();

    #if DK_PLATFORM_WINDOWS
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency );

    LONGLONG yieldInterval = (LONGLONG)floor( ((double)frequency.QuadPart / 1000000000.0) * (double)_self->yieldNSecs );
    #endif

    bool idle = false;
    bool standby = false;
    bool cooldown = false;

    if( _self->onThreadStart )
    {
        _self->onThreadStart( _self, _self->onThreadStartStopContext );
    }

    DKMutexLock( _self->queuesMutex );
    
    while( DKThreadGetState( thread ) != DKThreadCancelled )
    {
        struct DKThreadPoolTask * task = DKThreadPoolGetNextTask( _self );

        if( task )
        {
            if( idle )
            {
                idle = false;
                DKAtomicDecrement32( &_self->idleThreads );
            }
            
            if( standby )
            {
                standby = false;
                DKAtomicAnd32( &_self->standbyThreads, 0 );
                DKConditionSignal( _self->workAvailableCondition );
            }
        
            DKMutexUnlock( _self->queuesMutex );
            
            DKThreadPoolExecuteTask( _self, task );

            DKMutexLock( _self->queuesMutex );

            DKThreadPoolCompleteTask( _self, task );
            
            cooldown = true;
        }

        else
        {
            if( !idle )
            {
                idle = true;
                DKAtomicIncrement32( &_self->idleThreads );
                DKConditionSignalAll( _self->stateChangedCondition );
                
                if( DKAtomicCmpAndSwap32( &_self->standbyThreads, 0, 1 ) )
                    standby = true;
            }
            
            if( standby || cooldown )
            {
                DKMutexUnlock( _self->queuesMutex );
                
                #if DK_PLATFORM_POSIX
                if( _self->yieldNSecs > 0 )
                {
                    struct timespec sleeptime, remainder;
                    sleeptime.tv_sec = 0;
                    sleeptime.tv_nsec = _self->yieldNSecs;

                    nanosleep( &sleeptime, &remainder );
                }
                
                else
                {
                    sched_yield();
                }
                
                #elif DK_PLATFORM_WINDOWS
                LARGE_INTEGER t0, t1;
                
                QueryPerformanceCounter( &t0 )
                t0.QuadPart += yieldInterval;
                
                do
                {
                    SwitchToThread();
                    QueryPerformanceCounter( &t1 );
                
                } while( t0.QuadPart > t1.QuadPart );
                #endif

                DKMutexLock( _self->queuesMutex );
                
                cooldown = false;
            }
            
            else
            {
                DKConditionWait( _self->workAvailableCondition, _self->queuesMutex );
            }
        }
    }

    if( idle )
    {
        DKAtomicDecrement32( &_self->idleThreads );
    }

    DKMutexUnlock( _self->queuesMutex );

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
        DKMutexLock( _self->threadsMutex );
    
        int runningThreads = (int)DKListGetCount( _self->threads );
        int startedThreads = numThreads - runningThreads;
        
        if( startedThreads < 0 )
            startedThreads = 0;
        
        for( int i = 0; i < startedThreads; i++ )
        {
            DKThreadRef thread;

            if( _self->scheduling == DKThreadPoolRealTimeScheduling )
                thread = DKThreadInit( DKAlloc( DKThreadClass() ), DKThreadPoolRealTimeExec, _self );
            
            else // if( _self->scheduling == DKThreadPoolDefaultScheduling )
                thread = DKThreadInit( DKAlloc( DKThreadClass() ), DKThreadPoolExec, _self );
            
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
        
        DKMutexUnlock( _self->threadsMutex );
        
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
        DKMutexLock( _self->threadsMutex );

        // Note: pthread_join doesn't seem to be reliable enough to properly wait for
        // cancelled threads to finish, therefore this uses a semaphore instead.
        
        // We DON'T want to let go of the mutex while waiting for the cancelled threads
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
        
        // Wake up any threads waiting for work
        DKConditionSignalAll( _self->workAvailableCondition );

        // Wait for the threads to finish
        DKSemaphoreWait( _self->stopCounter, 0 );

        // Release the thread objects
        DKListRemoveAllObjects( _self->threads );

        DKMutexUnlock( _self->threadsMutex );
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
//  DKThreadPoolGetCurrentTaskGroup()
//
int64_t DKThreadPoolGetCurrentTaskGroup( DKThreadPoolRef _self )
{
    int64_t taskGroup = 0;
    
    if( _self )
    {
        DKMutexLock( _self->queuesMutex );
        
        struct DKThreadPoolQueue * queue = DKThreadPoolGetLastQueue( _self, false );
        
        if( queue )
            taskGroup = queue->taskGroup;
        
        DKMutexUnlock( _self->queuesMutex );
    }
    
    return taskGroup;
}


///
//  DKThreadPoolIsIdle()
//
bool DKThreadPoolIsIdle( DKThreadPoolRef _self )
{
    bool busy = false;

    if( _self )
    {
        DKMutexLock( _self->threadsMutex );
        
        busy = _self->idleThreads == (int32_t)DKListGetCount( _self->threads );
        
        DKMutexUnlock( _self->threadsMutex );
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
        DKMutexLock( _self->threadsMutex );
        
        while( _self->idleThreads < (int32_t)DKListGetCount( _self->threads ) )
            DKConditionWait( _self->stateChangedCondition, _self->threadsMutex );
        
        DKMutexUnlock( _self->threadsMutex );
    }
}


///
//  DKThreadPoolWaitForTasks()
//
void DKThreadPoolWaitForTasks( DKThreadPoolRef _self, int64_t taskGroup )
{
    if( _self && (taskGroup != 0) )
    {
        DKMutexLock( _self->queuesMutex );
        
        while( true )
        {
            struct DKThreadPoolQueue * queue = DKThreadPoolGetQueue( _self, taskGroup );
            DKAssert( (queue == NULL) || (queue->pendingTasks > 0) );
            
            if( queue == NULL )
                break;
            
            // Close the queue to make sure nothing new gets added to it
            queue->closed = true;
            
            DKConditionWait( _self->stateChangedCondition, _self->queuesMutex );
        }
        
        DKMutexUnlock( _self->queuesMutex );
    }
}


///
//  DKThreadPoolWaitForCurrentTasks()
//
void DKThreadPoolWaitForCurrentTasks( DKThreadPoolRef _self )
{
    DKThreadPoolWaitForTasks( _self, DKThreadPoolGetCurrentTaskGroup( _self ) );
}


///
//  DKThreadPoolAddTask()
//
int64_t DKThreadPoolAddTask( DKThreadPoolRef _self, DKThreadProc proc, void * context )
{
    int64_t taskGroup = 0;
    
    if( _self )
    {
        DKMutexLock( _self->queuesMutex );

        struct DKThreadPoolTask * task = DKThreadPoolAllocTask( _self, proc, context );
        taskGroup = DKThreadPoolScheduleTask( _self, task );
        
        DKMutexUnlock( _self->queuesMutex );
        
        if( _self->scheduling == DKThreadPoolDefaultScheduling )
            DKConditionSignal( _self->workAvailableCondition );
    }
    
    return taskGroup;
}


///
//  DKThreadPoolAddTaskMethod()
//
int64_t DKThreadPoolAddTaskMethod( DKThreadPoolRef _self, DKObjectRef target, DKThreadMethod method, DKObjectRef param )
{
    int64_t taskGroup = 0;
    
    if( _self )
    {
        DKMutexLock( _self->queuesMutex );

        struct DKThreadPoolTask * task = DKThreadPoolAllocObjectTask( _self, target, method, param );
        taskGroup = DKThreadPoolScheduleTask( _self, task );
        
        DKMutexUnlock( _self->queuesMutex );

        if( _self->scheduling == DKThreadPoolDefaultScheduling )
            DKConditionSignal( _self->workAvailableCondition );
    }
    
    return taskGroup;
}


///
//  DKThreadPoolAddCompletion()
//
int64_t DKThreadPoolAddCompletion( DKThreadPoolRef _self, DKThreadProc proc, void * context )
{
    int64_t taskGroup = 0;
    
    if( _self )
    {
        DKMutexLock( _self->queuesMutex );

        if( _self->queues )
        {
            struct DKThreadPoolTask * completion = DKThreadPoolAllocTask( _self, proc, context );
            taskGroup = DKThreadPoolScheduleCompletion( _self, completion );
        
            DKMutexUnlock( _self->queuesMutex );

            if( _self->scheduling == DKThreadPoolDefaultScheduling )
                DKConditionSignal( _self->workAvailableCondition );
        }
        
        else
        {
            DKMutexUnlock( _self->queuesMutex );

            proc( context );
        }
    }
    
    return taskGroup;
}


///
//  DKThreadPoolAddCompletionMethod()
//
int64_t DKThreadPoolAddCompletionMethod( DKThreadPoolRef _self, DKObjectRef target, DKThreadMethod method, DKObjectRef param )
{
    int64_t taskGroup = 0;
    
    if( _self )
    {
        DKMutexLock( _self->queuesMutex );

        if( _self->queues )
        {
            struct DKThreadPoolTask * completion = DKThreadPoolAllocObjectTask( _self, target, method, param );
            taskGroup = DKThreadPoolScheduleCompletion( _self, completion );
            
            DKMutexUnlock( _self->queuesMutex );

            if( _self->scheduling == DKThreadPoolDefaultScheduling )
                DKConditionSignal( _self->workAvailableCondition );
        }
        
        else
        {
            DKMutexUnlock( _self->queuesMutex );

            method( target, param );
        }
    }
    
    return taskGroup;
}


///
//  DKThreadPoolRemoveAllTasks()
//
void DKThreadPoolRemoveAllTasks( DKThreadPoolRef _self )
{
    if( _self )
    {
        DKMutexLock( _self->queuesMutex );
        
        DKThreadPoolFreeQueues( _self, _self->queues );
        _self->queues = NULL;
        
        DKMutexUnlock( _self->queuesMutex );
    }
}





