/*****************************************************************************************

  DKCopying.h

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

#ifndef _DK_COPYING_H_
#define __Duck__DKCopying__

#include "DKRuntime.h"


DKDeclareInterfaceSelector( Copying );


typedef DKObjectRef (*DKCopyMethod)( DKObjectRef );
typedef DKObjectRef (*DKMutableCopyMethod)( DKObjectRef );

struct DKCopyingInterface
{
    const DKInterface _interface;

    DKCopyMethod        copy;
    DKMutableCopyMethod mutableCopy;
};

typedef const struct DKCopyingInterface * DKCopyingInterfaceRef;


// Default copying interface that retains and returns the object. This is used by the
// root classes so it's defined in DKRuntime.c.
DKInterfaceRef DKDefaultCopying( void );


DKObjectRef DKCopy( DKObjectRef _self );
DKObjectRef DKMutableCopy( DKObjectRef _self );



#endif // _DK_COPYING_H_
