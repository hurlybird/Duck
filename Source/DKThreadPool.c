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
#include "DKMutex.h"
#include "DKCondition.h"


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
    
    DKMutableListRef threads;
    DKMutexRef controlMutex;
    
    struct DKThreadPoolTask * queue;
    DKMutexRef queueMutex;
    DKConditionRef queueCondition;
    
    DKThreadPoolCallback onThreadStart;
    void * onThreadStartContext;
    
    DKThreadPoolCallback onThreadStop;
    void * onThreadStopContext;
    
    int pendingTasks;
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
        _self->queueMutex = DKNewMutex();
        _self->queueCondition = DKNewCondition();
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
    DKRelease( _self->queueMutex );
    DKRelease( _self->queueCondition );
    
    DKNodePoolFinalize( &_self->nodePool );
}


///
//  DKThreadPoolSetCallbacks()
//
void DKThreadPoolOnThreadStart( DKThreadPoolRef _self, DKThreadPoolCallback onThreadStart, void * context )
{
    _self->onThreadStart = onThreadStart;
    _self->onThreadStartContext = context;
}


///
//  DKThreadPoolOnThreadStop()
//
void DKThreadPoolOnThreadStop( DKThreadPoolRef _self, DKThreadPoolCallback onThreadStop, void * context )
{
    _self->onThreadStop = onThreadStop;
    _self->onThreadStopContext = context;
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

    _self->pendingTasks++;
    
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

    _self->pendingTasks++;
    
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
        
        _self->pendingTasks--;
        
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
        _self->onThreadStart( _self, _self->onThreadStartContext );
    }

    DKMutexLock( _self->queueMutex );
    
    while( DKThreadGetState( thread ) != DKThreadCancelled )
    {
        if( _self->queue == NULL )
        {
            DKConditionWait( _self->queueCondition, _self->queueMutex );
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
        _self->onThreadStop( _self, _self->onThreadStopContext );
    }
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

        DKIndex threadCount = DKListGetCount( _self->threads );
        
        // Issue the cancel command
        for( DKIndex i = 0; i < threadCount; i++ )
        {
            DKThreadRef thread = DKListGetObjectAtIndex( _self->threads, i );
            DKThreadCancel( thread );
        }
        
        // Wake up any waiting threads
        DKConditionSignalAll( _self->queueCondition );

        // Wait for the threads to finish
        for( DKIndex i = 0; i < threadCount; i++ )
        {
            DKThreadRef thread = DKListGetObjectAtIndex( _self->threads, i );
            DKThreadJoin( thread );
        }
        
        // Release the thread objects
        DKListRemoveAllObjects( _self->threads );

        DKMutexUnlock( _self->controlMutex );
    }
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





