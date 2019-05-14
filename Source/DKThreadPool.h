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


typedef struct DKThreadPool * DKThreadPoolRef;

typedef void (*DKThreadPoolCallback)( DKThreadPoolRef threadPool, void * context );


DKClassRef DKThreadPoolClass( void );

#define DKNewThreadPool()   DKNew( DKThreadPoolClass() )

void DKThreadPoolSetCallbacks( DKThreadPoolRef _self,
    DKThreadPoolCallback onThreadStart,
    DKThreadPoolCallback onThreadStop,
    void * context );

void DKThreadPoolSetLabel( DKThreadPoolRef _self, DKStringRef label );

int DKThreadPoolStart( DKThreadPoolRef _self, int numThreads );
void DKThreadPoolStop( DKThreadPoolRef _self );

int DKThreadPoolGetThreadCount( DKThreadPoolRef _self );

int DKThreadPoolIsBusy( DKThreadPoolRef _self );
void DKThreadPoolWaitUntilIdle( DKThreadPoolRef _self );

void DKThreadPoolAddTask( DKThreadPoolRef _self, DKThreadProc proc, void * context );
void DKThreadPoolAddObjectTask( DKThreadPoolRef _self, DKObjectRef target, DKThreadMethod method, DKObjectRef param );

void DKThreadPoolRemoveAllTasks( DKThreadPoolRef _self );


#endif // _DK_THREAD_POOL_H_
