/*****************************************************************************************

  DKThreadPool.h

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

#ifndef _DK_THREAD_POOL_H_
#define _DK_THREAD_POOL_H_

#include "DKRuntime.h"
#include "DKThread.h"


typedef enum
{
    // Default scheduling uses conditions and signals to wake threads when tasks are added
    // to the work queue. This is appropriate for most use cases since synchronization
    // overhead is typically lower than the task overhead.
    DKThreadPoolDefaultScheduling = 0,
    
    // Real-Time scheduling uses a combination of yields and conditions to wake threads
    // while avoiding explict signalling when adding tasks. This is useful in sitations
    // involving a continuous series of small tasks, or when keeping the main thread
    // overhead as low as possible is important.
    DKThreadPoolRealTimeScheduling

} DKThreadPoolScheduling;

typedef struct DKThreadPool * DKThreadPoolRef;

typedef void (*DKThreadPoolCallback)( DKThreadPoolRef threadPool, void * context );


DK_API DKClassRef DKThreadPoolClass( void );

#define DKThreadPool()      DKAutorelease( DKNew( DKThreadPoolClass() ) )
#define DKNewThreadPool()   DKNew( DKThreadPoolClass() )

DK_API void DKThreadPoolSetCallbacks( DKThreadPoolRef _self,
    DKThreadPoolCallback onThreadStart,
    DKThreadPoolCallback onThreadStop,
    void * context );

DK_API void DKThreadPoolSetScheduling( DKThreadPoolRef _self, DKThreadPoolScheduling scheduling, int yieldNSecs );
DK_API void DKThreadPoolSetLabel( DKThreadPoolRef _self, DKStringRef label );

DK_API int DKThreadPoolStart( DKThreadPoolRef _self, int numThreads );
DK_API void DKThreadPoolStop( DKThreadPoolRef _self );

DK_API int DKThreadPoolGetThreadCount( DKThreadPoolRef _self );
DK_API int64_t DKThreadPoolGetCurrentTaskGroup( DKThreadPoolRef _self );

DK_API bool DKThreadPoolIsIdle( DKThreadPoolRef _self );
DK_API void DKThreadPoolWaitUntilIdle( DKThreadPoolRef _self );
DK_API void DKThreadPoolWaitForTasks( DKThreadPoolRef _self, int64_t taskGroup );
DK_API void DKThreadPoolWaitForCurrentTasks( DKThreadPoolRef _self );

DK_API int64_t DKThreadPoolAddTask( DKThreadPoolRef _self, DKThreadProc proc, void * context );
DK_API int64_t DKThreadPoolAddTaskMethod( DKThreadPoolRef _self, DKObjectRef target, DKThreadMethod method, DKObjectRef param );

DK_API int64_t DKThreadPoolAddCompletion( DKThreadPoolRef _self, DKThreadProc proc, void * context );
DK_API int64_t DKThreadPoolAddCompletionMethod( DKThreadPoolRef _self, DKObjectRef target, DKThreadMethod method, DKObjectRef param );

DK_API void DKThreadPoolRemoveAllTasks( DKThreadPoolRef _self );


#endif // _DK_THREAD_POOL_H_
