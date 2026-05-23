// =======================================================================================
//
// DKThread.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_THREAD_H_
#define _DK_THREAD_H_

#ifdef __cplusplus
extern "C"
{
#endif


// DKThread ==============================================================================
typedef struct DKThread * DKThreadRef;

typedef void (*DKThreadProc)( void * context );
typedef void (*DKThreadMethod)( DKObjectRef _self, DKObjectRef param );

typedef enum
{
    DKThreadCreated = 0,
    DKThreadStarted,
    DKThreadRunning,
    DKThreadCancelled,
    DKThreadFinished,

    // The thread was not created by DKThread
    DKThreadStateUnknown

} DKThreadState;


DK_API DKClassRef DKThreadClass( void );

DK_API DKThreadRef DKThreadGetCurrentThread( void );
DK_API DKThreadRef DKThreadGetMainThread( void );


DK_API void DKDetachNewThread( DKThreadProc proc, void * context );
DK_API void DKDetachNewThreadToTarget( DKObjectRef target, DKThreadMethod method, DKObjectRef param );

DK_API DKObjectRef DKThreadInit( DKObjectRef _self, DKThreadProc proc, void * context );
DK_API DKObjectRef DKThreadInitWithTarget( DKObjectRef _self, DKObjectRef target, DKThreadMethod method, DKObjectRef param );

DK_API void DKThreadSetLabel( DKThreadRef _self, DKStringRef label );

DK_API void DKThreadStart( DKThreadRef _self );
DK_API void DKThreadJoin( DKThreadRef _self );
DK_API void DKThreadCancel( DKThreadRef _self );
DK_API void DKThreadExit( void );

// Get thread information. Unlike most interfaces, if '_self' is NULL, the values
// associated with the current thread are returned.
DK_API DKThreadState DKThreadGetState( DKThreadRef _self );
DK_API bool DKThreadIsMainThread( DKThreadRef _self );




// DKThreadContext =======================================================================
typedef struct DKThreadContext * DKThreadContextRef;

// The main thread and DKThreads manage their own thread contexts. Any threads created
// manually must allocate a DKThreadContext structure and set it before calling any Duck
// functions, and free the DKThreadContext once it is no longer in use.
DK_API DKThreadContextRef DKAllocThreadContext( void );
DK_API void DKFreeThreadContext( DKThreadContextRef threadContext );

// Get/Set the thread context for the current thread
DK_API void DKSetCurrentThreadContext( DKThreadContextRef threadContext );
DK_API bool DKCurrentThreadContextIsSet( void );
DK_API DKThreadContextRef DKGetCurrentThreadContext( void );

// Get the thread context of the main thread
DK_API DKThreadContextRef DKGetMainThreadContext( void );

// Get rhe thread-specific dictionary for the current thread
DK_API DKMutableDictionaryRef DKGetCurrentThreadDictionary( void );




// Private ===============================================================================
#if DK_THREAD_PRIVATE

struct DKThreadContext
{
    DKThreadRef threadObject;
    DKMutableDictionaryRef threadDictionary;

    uint32_t options;

    struct
    {
        DKGenericArray objects;
        DKIndex top;
        DKIndex lowWater[DK_AUTORELEASE_POOL_STACK_SIZE];
        
    } arp;
};

void DKThreadContextInit( DKThreadContextRef threadContext, uint32_t options );
void DKThreadContextFinalize( DKThreadContextRef threadContext );
void DKMainThreadContextInit( void );


#endif // DK_RUNTIME_PRIVATE


#ifdef __cplusplus
}
#endif

#endif // _DK_THREAD_H_


