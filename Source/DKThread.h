/*****************************************************************************************

  DKThread.h

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

#ifndef _DK_THREAD_H_
#define _DK_THREAD_H_

#include "DKRuntime.h"
#include "DKDictionary.h"


// DKThread ==============================================================================
typedef struct DKThread * DKThreadRef;

typedef void (*DKThreadProc)( DKObjectRef object );

typedef enum
{
    DKThreadCreated,
    DKThreadStarted,
    DKThreadRunning,
    DKThreadCancelled,
    DKThreadFinished,

    // The thread was not created by DKThread
    DKThreadUnknown

} DKThreadState;


DKClassRef DKThreadClass( void );

DKThreadRef DKThreadGetCurrentThread( void );
DKThreadRef DKThreadGetMainThread( void );


void DKDetachNewThread( DKThreadProc threadProc, DKObjectRef threadParam );

DKObjectRef DKThreadInit( DKObjectRef _self, DKThreadProc threadProc, DKObjectRef threadParam );

void DKThreadStart( DKThreadRef _self );
void DKThreadJoin( DKThreadRef _self );
void DKThreadCancel( DKThreadRef _self );
void DKThreadExit( void );

// Get thread information. Unlike most interfaces, if '_self' is NULL, the values
// associated with the current thread are returned.
DKThreadState DKThreadGetState( DKThreadRef _self );
DKMutableDictionaryRef DKThreadGetDictionary( DKThreadRef _self );
bool DKThreadIsMainThread( DKThreadRef _self );




// DKThreadContext =======================================================================
typedef struct DKThreadContext * DKThreadContextRef;

DKThreadContextRef DKAllocThreadContext( void );
void DKFreeThreadContext( DKThreadContextRef threadContext );

void DKSetCurrentThreadContext( DKThreadContextRef threadContext );
DKThreadContextRef DKGetCurrentThreadContext( void );

DKThreadContextRef DKGetMainThreadContext( void );




// Private ===============================================================================
#if DK_RUNTIME_PRIVATE

struct DKThreadContext
{
    DKObjectRef threadObject;

    uint32_t options;

    struct
    {
        DKGenericArray objects;
        DKIndex top;
        DKIndex count[DK_AUTORELEASE_POOL_STACK_SIZE];
        
    } arp;
};

void DKThreadContextInit( struct DKThreadContext * threadContext, uint32_t options );
void DKMainThreadContextInit( void );


#endif // DK_RUNTIME_PRIVATE



#endif // _DK_THREAD_H_


