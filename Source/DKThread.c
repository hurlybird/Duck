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




// DKThreadContext =======================================================================
enum
{
    DKThreadContextAllocated = (1 << 0)
};

static struct DKThreadContext DKMainThreadContext;
static pthread_key_t DKThreadContextKey;
static bool DKThreadContextInitialized = false;


///
//  DKThreadContextInit()
//
void DKThreadContextInit( DKThreadContextRef threadContext, uint32_t options )
{
    threadContext->threadObject = NULL;
    
    threadContext->options = options;
    
    // Initialize the autorelease pool stack
    DKGenericArrayInit( &threadContext->arp.objects, sizeof(DKObjectRef) );
    
    threadContext->arp.top = -1;

    for( int i = 0; i < DK_AUTORELEASE_POOL_STACK_SIZE; i++ )
        threadContext->arp.count[i] = 0;
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
        DKRelease( threadContext->threadObject );
        
        DKRequire( threadContext->arp.top == -1 );
        
        DKGenericArrayFinalize( &threadContext->arp.objects );

        if( threadContext->options & DKThreadContextAllocated )
            dk_free( threadContext );
    }
}


///
//  DKThreadContextDestructor()
//
static void DKThreadContextDestructor( void * context )
{
    DKFreeThreadContext( context );
    pthread_setspecific( DKThreadContextKey, NULL );
}


///
//  DKSetCurrentThreadContext()
//
void DKSetCurrentThreadContext( DKThreadContextRef threadContext )
{
    DKAssert( DKThreadContextInitialized );
    
    pthread_setspecific( DKThreadContextKey, threadContext );
}


///
//  DKGetCurrentThreadContext()
//
DKThreadContextRef DKGetCurrentThreadContext( void )
{
    DKAssert( DKThreadContextInitialized );

    DKThreadContextRef threadContext = pthread_getspecific( DKThreadContextKey );
    DKAssert( threadContext != NULL );
    
    return threadContext;
}


///
//  DKMainThreadContextInit()
//
void DKMainThreadContextInit( void )
{
    DKRequire( !DKThreadContextInitialized );
    
    DKThreadContextInitialized = true;

    pthread_key_create( &DKThreadContextKey, DKThreadContextDestructor );

    DKThreadContextInit( &DKMainThreadContext, 0 );
    DKSetCurrentThreadContext( &DKMainThreadContext );
}


///
//  DKGetMainThreadContext()
//
DKThreadContextRef DKGetMainThreadContext( void )
{
    DKAssert( DKThreadContextInitialized );

    return &DKMainThreadContext;
}




// DKThread ==============================================================================
struct DKThread
{
    DKObject _obj;

    pthread_t threadId;
    DKThreadState state;
    DKSpinLock lock;
    
    DKThreadProc proc;
    DKObjectRef param;
    DKMutableDictionaryRef dictionary;
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
    
    DKRelease( _self->param );
    DKRelease( _self->dictionary );
}


///
//  DKThreadGetCurrentThread()
//
DKThreadRef DKThreadGetCurrentThread( void )
{
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    
    if( !threadContext->threadObject )
    {
        struct DKThread * thread = DKNew( DKThreadClass() );
        
        thread->state = DKThreadUnknown;
        thread->threadId = pthread_self();
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
void DKDetachNewThread( DKThreadProc threadProc, DKObjectRef threadParam )
{
    DKThreadRef thread = DKThreadInit( DKAlloc( DKThreadClass() ), threadProc, threadParam );
    DKThreadStart( thread );
    DKRelease( thread );
}


///
//  DKThreadInit()
//
DKObjectRef DKThreadInit( DKObjectRef _untyped_self, DKThreadProc threadProc, DKObjectRef threadParam )
{
    DKThreadRef _self = DKInit( _untyped_self );
    
    if( _self )
    {
        _self->proc = threadProc;
        _self->param = DKRetain( threadParam );
    }
    
    return _self;
}


///
//  DKThreadExec()
//
static void * DKThreadExec( void * _thread )
{
    DKThreadRef thread = _thread;
    
    struct DKThreadContext threadContext;
    
    DKThreadContextInit( &threadContext, 0 );
    threadContext.threadObject = thread; // Retained in DKThreadStart
    
    DKSetCurrentThreadContext( &threadContext );
    
    DKSpinLockLock( &thread->lock );
    thread->state = DKThreadRunning;
    DKSpinLockUnlock( &thread->lock );
    
    thread->proc( thread->param );
    
    DKSpinLockLock( &thread->lock );
    thread->state = DKThreadFinished;
    DKSpinLockUnlock( &thread->lock );
    
    DKSetCurrentThreadContext( NULL );
    
    return NULL;
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
            
            DKRetain( _self ); // Released by the associated DKThreadContext
            
            pthread_create( &_self->threadId, NULL, DKThreadExec, _self );
            pthread_detach( _self->threadId );
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
            pthread_join( _self->threadId, NULL );
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
            DKError( "DKThreadJoin: Trying to join a thread that was never started." );
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

    pthread_exit( NULL );
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
//  DKThreadGetDictionary()
//
DKMutableDictionaryRef DKThreadGetDictionary( DKThreadRef _self )
{
    if( !_self )
        _self = DKThreadGetCurrentThread();

    DKAssertKindOfClass( _self, DKThreadClass() );
    
    if( !_self->dictionary )
        _self->dictionary = DKNew( DKMutableDictionaryClass() );
    
    return _self->dictionary;
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






