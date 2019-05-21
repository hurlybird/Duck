/*****************************************************************************************

  DKThread.c

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

#include "DKThread.h"
#include "DKString.h"
#include "DKCopying.h"




// DKThreadContext =======================================================================
enum
{
    DKThreadContextAllocated = (1 << 0)
};

static struct DKThreadContext DKMainThreadContext;

#if DK_PLATFORM_POSIX
static pthread_key_t DKThreadContextKey;
#elif DK_PLATFORM_WINDOWS
static DWORD DKThreadContextKey; 
#endif

static bool DKThreadContextKeyInitialized = false;




///
//  DKThreadContextInit()
//
void DKThreadContextInit( DKThreadContextRef threadContext, uint32_t options )
{
    threadContext->threadObject = NULL;
    threadContext->threadDictionary = DKNewMutableDictionary();
    
    threadContext->options = options;
    
    // Initialize the autorelease pool stack
    DKGenericArrayInit( &threadContext->arp.objects, sizeof(DKObjectRef) );
    
    threadContext->arp.top = -1;

    for( int i = 0; i < DK_AUTORELEASE_POOL_STACK_SIZE; i++ )
        threadContext->arp.count[i] = 0;
}


///
//  DKThreadContextFinalize()
//
void DKThreadContextFinalize( DKThreadContextRef threadContext )
{
    DKRelease( threadContext->threadObject );
    DKRelease( threadContext->threadDictionary );
    
    DKRequire( threadContext->arp.top == -1 );
    
    DKGenericArrayFinalize( &threadContext->arp.objects );
}


///
//  DKAllocThreadContext()
//
DKThreadContextRef DKAllocThreadContext( void )
{
    DKThreadContextRef threadContext = dk_malloc( sizeof(struct DKThreadContext) );
    
    DKThreadContextInit( threadContext, DKThreadContextAllocated );
    
    return threadContext;
}


///
//  DKFreeThreadContext()
//
void DKFreeThreadContext( DKThreadContextRef threadContext )
{
    if( threadContext )
    {
        DKThreadContextFinalize( threadContext );

        if( threadContext->options & DKThreadContextAllocated )
            dk_free( threadContext );
    }
}


///
//  DKThreadContextDestructor()
//
#if DK_PLATFORM_POSIX
static void DKThreadContextDestructor( void * context )
{
    DKFreeThreadContext( context );

    pthread_setspecific( DKThreadContextKey, NULL );
}
#endif


///
//  DKSetCurrentThreadContext()
//
void DKSetCurrentThreadContext( DKThreadContextRef threadContext )
{
    DKAssert( DKThreadContextKeyInitialized );
   
#if DK_PLATFORM_POSIX
    pthread_setspecific( DKThreadContextKey, threadContext );
#elif DK_PLATFORM_WINDOWS
    if( !TlsSetValue( DKThreadContextKey, threadContext ) )
        DKFatalError( "DKThread: Failed to save thread context to thread local storage.\n" );
#endif
}


///
//  DKCurrentThreadContextIsSet()
//
bool DKCurrentThreadContextIsSet( void )
{
    DKAssert( DKThreadContextKeyInitialized );

#if DK_PLATFORM_POSIX
    DKThreadContextRef threadContext = pthread_getspecific( DKThreadContextKey );
#elif DK_PLATFORM_WINDOWS
    DKThreadContextRef threadContext = TlsGetValue( DKThreadContextKey );
#endif

    return threadContext != NULL;
}


///
//  DKGetCurrentThreadContext()
//
DKThreadContextRef DKGetCurrentThreadContext( void )
{
    DKAssert( DKThreadContextKeyInitialized );

#if DK_PLATFORM_POSIX
    DKThreadContextRef threadContext = pthread_getspecific( DKThreadContextKey );
#elif DK_PLATFORM_WINDOWS
    DKThreadContextRef threadContext = TlsGetValue( DKThreadContextKey );
#endif

    DKAssert( threadContext != NULL );
    
    return threadContext;
}


///
//  DKMainThreadContextInit()
//
void DKMainThreadContextInit( void )
{
    DKRequire( !DKThreadContextKeyInitialized );
    
    DKThreadContextKeyInitialized = true;

#if DK_PLATFORM_POSIX
    pthread_key_create( &DKThreadContextKey, DKThreadContextDestructor );
#elif DK_PLATFORM_WINDOWS
    if( (DKThreadContextKey = TlsAlloc()) == TLS_OUT_OF_INDEXES ) 
        DKFatalError( "DKThread: Out of thread local storage indexes.\n" );
#endif

    DKThreadContextInit( &DKMainThreadContext, 0 );
    DKSetCurrentThreadContext( &DKMainThreadContext );
}


///
//  DKGetMainThreadContext()
//
DKThreadContextRef DKGetMainThreadContext( void )
{
    DKAssert( DKThreadContextKeyInitialized );

    return &DKMainThreadContext;
}


///
//  DKGetCurrentThreadDictionary()
//
DKMutableDictionaryRef DKGetCurrentThreadDictionary( void )
{
    DKThreadContextRef threadContext = DKGetCurrentThreadContext();
    return threadContext->threadDictionary;
}




// DKThread ==============================================================================
struct DKThread
{
    DKObject _obj;

#if DK_PLATFORM_POSIX
    pthread_t threadId;
#elif DK_PLATFORM_WINDOWS
    HANDLE threadHandle;
#endif

    DKThreadState state;
    DKSpinLock lock;

    DKStringRef label;

    DKThreadProc proc;
    void * context;
    
    DKObjectRef target;
    DKThreadMethod method;
    DKObjectRef param;
};


static DKObjectRef DKThreadInitialize( DKObjectRef _self );
static void DKThreadFinalize( DKObjectRef _self );


DKThreadSafeClassInit( DKThreadClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKThread" ), DKObjectClass(), sizeof(struct DKThread), 0, DKThreadInitialize, DKThreadFinalize );
    
    return cls;
}


///
//  DKThreadInitialize()
//
static DKObjectRef DKThreadInitialize( DKObjectRef _untyped_self )
{
    DKThreadRef _self = _untyped_self;
    
    if( _self )
    {
        _self->state = DKThreadCreated;
        _self->lock = DKSpinLockInit;
    }
    
    return _self;
}


///
//  DKThreadFinalize()
//
static void DKThreadFinalize( DKObjectRef _untyped_self )
{
    DKThreadRef _self = _untyped_self;
    
    DKAssert( (_self->state != DKThreadRunning) && (_self->state != DKThreadCancelled) );
    
    DKRelease( _self->target );
    DKRelease( _self->param );
    DKRelease( _self->label );
}


///
//  DKThreadSetLabel()
//
void DKThreadSetLabel( DKThreadRef _self, DKStringRef label )
{
    label = DKCopy( label );
    DKRelease( _self->label );
    _self->label = label;
}


///
//  DKThreadGetCurrentThread()
//
DKThreadRef DKThreadGetCurrentThread( void )
{
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    
    if( !threadContext->threadObject )
    {
        threadContext->threadObject = DKNew( DKThreadClass() );
        threadContext->threadObject->state = DKThreadStateUnknown;

#if DK_PLATFORM_POSIX
        threadContext->threadObject->threadId = pthread_self();
#elif DK_PLATFORM_WINDOWS
        threadContext->threadObject->threadHandle = GetCurrentThread();
#endif
    }
    
    return threadContext->threadObject;
}


///
//  DKThreadGetMainThread()
//
DKThreadRef DKThreadGetMainThread( void )
{
    struct DKThreadContext * threadContext = DKGetMainThreadContext();
    
    // The main thread object should have been created by DKRuntimeInit.
    DKRequire( threadContext != NULL );
    DKRequire( threadContext->threadObject != NULL );
    
    return threadContext->threadObject;
}


///
//  DKDetachNewThread()
//
void DKDetachNewThread( DKThreadProc func, void * context )
{
    DKThreadRef thread = DKThreadInit( DKAlloc( DKThreadClass() ), func, context );
    DKThreadStart( thread );
    DKRelease( thread );
}


///
//  DKDetachNewThreadToTarget()
//
void DKDetachNewThreadToTarget( DKObjectRef target, DKThreadMethod method, DKObjectRef param )
{
    DKThreadRef thread = DKThreadInitWithTarget( DKAlloc( DKThreadClass() ), target, method, param );
    DKThreadStart( thread );
    DKRelease( thread );
}


///
//  DKThreadInit()
//
DKObjectRef DKThreadInit( DKObjectRef _untyped_self, DKThreadProc proc, void * context )
{
    DKThreadRef _self = DKInit( _untyped_self );
    
    if( _self )
    {
        _self->proc = proc;
        _self->context = context;
    }
    
    return _self;
}


///
//  DKThreadInitWithTarget()
//
DKObjectRef DKThreadInitWithTarget( DKObjectRef _untyped_self, DKObjectRef target, DKThreadMethod method, DKObjectRef param )
{
    DKThreadRef _self = DKInit( _untyped_self );
    
    if( _self )
    {
        _self->target = DKRetain( target );
        _self->method = method;
        _self->param = DKRetain( param );
    }
    
    return _self;
}


///
//  DKThreadExec()
//
#if DK_PLATFORM_POSIX
static void * DKThreadExec( void * _thread )
#elif DK_PLATFORM_WINDOWS
static DWORD WINAPI DKThreadExec( LPVOID _thread )
#endif
{
    DKThreadRef thread = _thread;
    
    struct DKThreadContext threadContext;
    DKThreadContextInit( &threadContext, 0 );
    threadContext.threadObject = DKRetain( thread );
    
    DKSetCurrentThreadContext( &threadContext );
    
    DKSpinLockLock( &thread->lock );
    thread->state = DKThreadRunning;
    DKSpinLockUnlock( &thread->lock );
    
    DKPushAutoreleasePool();
    
    if( thread->proc )
        thread->proc( thread->context );
    
    else
        thread->method( thread->target, thread->param );
    
    DKPopAutoreleasePool();
    
    DKSpinLockLock( &thread->lock );
    thread->state = DKThreadFinished;
    DKSpinLockUnlock( &thread->lock );
    
    DKSetCurrentThreadContext( NULL );
    DKThreadContextFinalize( &threadContext );
    
    DKRelease( thread ); // Retained in DKThreadStart
   
#if DK_PLATFORM_POSIX
    return NULL;
#elif DK_PLATFORM_WINDOWS
    return 0;
#endif
}


///
//  DKThreadStart()
//
void DKThreadStart( DKThreadRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKThreadClass() );
        
        DKSpinLockLock( &_self->lock );

        if( _self->state == DKThreadCreated )
        {
            _self->state = DKThreadStarted;

            DKSpinLockUnlock( &_self->lock );
            
            DKRetain( _self ); // Released in DKThreadExec
            
#if DK_PLATFORM_POSIX
            pthread_create( &_self->threadId, NULL, DKThreadExec, _self );
            pthread_detach( _self->threadId );
#elif DK_PLATFORM_WINDOWS
            _self->threadHandle = CreateThread( NULL, 0, DKThreadExec, _self, 0, NULL );
#endif
        }
        
        else
        {
            DKSpinLockUnlock( &_self->lock );
        }
    }
}


///
//  DKThreadJoin()
//
void DKThreadJoin( DKThreadRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKThreadClass() );

        DKSpinLockLock( &_self->lock );
        
        if( _self->state < DKThreadStarted )
        {
            DKSpinLockUnlock( &_self->lock );
            DKError( "DKThreadJoin: Trying to join a thread that was never started." );
        }
        
        else
        {
            DKSpinLockUnlock( &_self->lock );
            
#if DK_PLATFORM_POSIX
            pthread_join( _self->threadId, NULL );
#elif DK_PLATFORM_WINDOWS
            WaitForSingleObject( _self->threadHandle, INFINITE );
#endif
        }
    }
}


///
//  DKThreadCancel()
//
void DKThreadCancel( DKThreadRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKThreadClass() );
        
        if( _self->state < DKThreadStarted )
        {
            DKError( "DKThreadCancel: Trying to cancel a thread that was never started." );
        }
        
        else
        {
            DKSpinLockLock( &_self->lock );
            
            if( (_self->state == DKThreadStarted) || (_self->state == DKThreadRunning) )
                _self->state = DKThreadCancelled;

            DKSpinLockUnlock( &_self->lock );
        }
    }
}


///
//  DKThreadExit()
//
void DKThreadExit( void )
{
    DKThreadRef thread = DKThreadGetCurrentThread();

    DKSpinLockLock( &thread->lock );
    thread->state = DKThreadFinished;
    DKSpinLockUnlock( &thread->lock );

#if DK_PLATFORM_POSIX
    pthread_exit( NULL );
#elif DK_PLATFORM_WINDOWS
    ExitThread( 0 );
#endif
}


///
//  DKThreadGetState()
//
DKThreadState DKThreadGetState( DKThreadRef _self )
{
    if( !_self )
        _self = DKThreadGetCurrentThread();

    DKAssertKindOfClass( _self, DKThreadClass() );
    
    DKSpinLockLock( &_self->lock );
    DKThreadState state = _self->state;
    DKSpinLockUnlock( &_self->lock );
    
    return state;
}


///
//  DKThreadIsMainThread()
//
bool DKThreadIsMainThread( DKThreadRef _self )
{
    if( !_self )
        _self = DKThreadGetCurrentThread();

    DKAssertKindOfClass( _self, DKThreadClass() );

    return _self == DKGetMainThreadContext()->threadObject;
}






