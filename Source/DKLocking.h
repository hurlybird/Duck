/*****************************************************************************************

  DKLocking.h

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

#ifndef _DK_LOCKING_H_
#define _DK_LOCKING_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Locking );


typedef void (*DKLockMethod)( DKObjectRef _self );
typedef void (*DKUnlockMethod)( DKObjectRef _self );


struct DKLockingInterface
{
    const DKInterface _interface;

    DKLockMethod    lock;
    DKUnlockMethod  unlock;
};

typedef const struct DKLockingInterface * DKLockingInterfaceRef;


// Default locking interface that associates aretains and returns the object. This is used by the
// root classes so it's defined in DKRuntime.c.
DK_API DKInterfaceRef DKDefaultLocking( void );


DK_API void DKLock( DKObjectRef _self );
DK_API void DKUnlock( DKObjectRef _self );



#ifdef __cplusplus
}
#endif

#endif // _DK_LOCKING_H_
