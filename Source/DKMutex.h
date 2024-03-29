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

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKMutex * DKMutexRef;


DK_API DKClassRef DKMutexClass( void );

#define DKNewMutex()            DKMutexInit( DKAlloc( DKMutexClass() ) )
#define DKNewRecursiveMutex()   DKRecursiveMutexInit( DKAlloc( DKMutexClass() ) )


DK_API DKObjectRef DKMutexInit( DKObjectRef _self );
DK_API DKObjectRef DKRecursiveMutexInit( DKObjectRef _self );

DK_API void DKMutexLock( DKMutexRef _self );
DK_API bool DKMutexTryLock( DKMutexRef _self );
DK_API void DKMutexUnlock( DKMutexRef _self );




// Private ===============================================================================
#if DK_THREAD_PRIVATE

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


#ifdef __cplusplus
}
#endif

#endif // _DK_MUTEX_H_

