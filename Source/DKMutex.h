/*****************************************************************************************

  DKMutex.h

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

#ifndef _DK_MUTEX_H_
#define _DK_MUTEX_H_

#include "DKRuntime.h"


// DKMutex ===============================================================================
typedef struct DKMutex * DKMutexRef;


DKClassRef DKMutexClass( void );

#define DKNewMutex()            DKMutexInit( DKAlloc( DKMutexClass() ) )
#define DKNewRecursiveMutex()   DKRecursiveMutexInit( DKAlloc( DKMutexClass() ) )


DKObjectRef DKMutexInit( DKObjectRef _self );
DKObjectRef DKRecursiveMutexInit( DKObjectRef _self );

void DKMutexLock( DKMutexRef _self );
bool DKMutexTryLock( DKMutexRef _self );
void DKMutexUnlock( DKMutexRef _self );




// Private ===============================================================================
#if DK_RUNTIME_PRIVATE

struct DKMutex
{
    DKObject _obj;
    
#if DK_PLATFORM_POSIX
    pthread_mutex_t mutex;
#elif DK_PLATFORM_WINDOWS
    CRITICAL_SECTION criticalSection;
#endif
};


#endif // DK_RUNTIME_PRIVATE



#endif // _DK_MUTEX_H_

