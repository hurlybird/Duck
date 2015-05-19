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
    DKClassRef cls = DKAllocClass( DKSTR( "DKThread" ), DKObjectClass(), sizeof(struct DKThread), 0, DKThreadInitialize, DKThreadFinalize );
    
    return cls;
}


///
//  DKThreadInitialize()
//
static DKObjectRef DKThreadInitialize( DKObjectRef _self )
{
    if( _self )
    {
        struct DKThread * thread = _self;
        
        thread->state = DKThreadCreated;
        thread->lock = DKSpinLockInit;
    }
    
    return _self;
}


///
//  DKThreadFinalize()
//
static void DKThreadFinalize( DKObjectRef _self )
{
    struct DKThread * thread = _self;
    
    DKAssert( (thread->state != DKThreadRunning) && (thread->state != DKThreadCancelled) );
    
    DKRelease( thread->param );
    DKRelease( thread->dictionary );
}


///
//  DKThreadGetCurrentThread()
//
DKThreadRef DKThreadGetCurrentThread( void )
{
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    
    if( !threadContext->threadObject )
    {
        struct DKThread * thread = DKCreate( DKThreadClass() );
        
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
    DKAssert( threadContext->threadObject );
    
    return threadContext->threadObject;
}


///
//  DKDetachNewThread()
//
void DKDetachNewThread( DKThreadProc threadProc, DKObjectRef threadParam )
{
    DKThreadRef thread = DKThreadInit( DKAlloc( DKThreadClass(), 0 ), threadProc, threadParam );
    DKThreadStart( thread );
    DKRelease( thread );
}


///
//  DKThreadInit()
//
DKObjectRef DKThreadInit( DKObjectRef _self, DKThreadProc threadProc, DKObjectRef threadParam )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        struct DKThread * thread = _self;
        
        thread->proc = threadProc;
        thread->param = DKRetain( threadParam );
    }
    
    return _self;
}


///
//  DKThreadExec()
//
static void * DKThreadExec( void * _thread )
{
    struct DKThread * thread = _thread;
    
    struct DKThreadContext * threadContext = DKGetCurrentThreadContext();
    threadContext->threadObject = thread;
    
    DKSpinLockLock( &thread->lock );
    thread->state = DKThreadRunning;
    DKSpinLockUnlock( &thread->lock );
    
    thread->proc( thread->param );
    
    DKSpinLockLock( &thread->lock );
    thread->state = DKThreadFinished;
    DKSpinLockUnlock( &thread->lock );
    
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
        _self->dictionary = DKCreate( DKMutableDictionaryClass() );
    
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




