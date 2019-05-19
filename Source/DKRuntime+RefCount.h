/*****************************************************************************************

  DKRuntime+RefCount.h

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

#ifndef _DK_RUNTIME_REFERENCE_COUNTING_H_
#define _DK_RUNTIME_REFERENCE_COUNTING_H_


DK_API DKObjectRef DKRetain( DKObjectRef _self );
DK_API DKObjectRef DKRelease( DKObjectRef _self ); // Always returns NULL

// If a call to DKRelease would free the object, do so and return NULL. Otherwise return
// the object unchanged. (This is mainly useful for implementing object caches.)
DK_API DKObjectRef DKTryRelease( DKObjectRef _self );

// Get a weak reference to an object. The weak reference itself must be released when the
// caller is finished with it.
DK_API DKWeakRef   DKRetainWeak( DKObjectRef _self );

// Resolve a weak reference into a strong reference. The returned object must be released
// when the caller is finished with it. This will return NULL if the object has been
// deallocated.
DK_API DKObjectRef DKResolveWeak( DKWeakRef weakref );

// Drain the current autorelease pool
DK_API void        DKDrainAutoreleasePool( void );

// Push/Pop the current autorelease pool
DK_API void        DKPushAutoreleasePool( void );
DK_API void        DKPopAutoreleasePool( void );

DK_API DKObjectRef DKAutorelease( DKObjectRef _self );



#endif // _DK_RUNTIME_REFERENCE_COUNTING_H_
