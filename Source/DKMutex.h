// =======================================================================================
//
// DKMutex.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
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

