/*****************************************************************************************

  DKBuffer.h

  Copyright (c) 2017 Derek W. Nylen

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

#ifndef _DK_BUFFER_H_
#define _DK_BUFFER_H_

#include "DKRuntime.h"


DKDeclareInterfaceSelector( Buffer );


typedef DKIndex      (*DKBufferGetLengthMethod)( DKObjectRef _self );
typedef const void * (*DKBufferGetBytePtrMethod)( DKObjectRef _self, DKIndex index );

typedef void         (*DKBufferSetLengthMethod)( DKObjectRef _self, DKIndex length );
typedef void *       (*DKBufferGetMutableBytePtrMethod)( DKObjectRef _self, DKIndex index );


struct DKBufferInterface
{
    const DKInterface _interface;
    
    DKBufferGetLengthMethod         getLength;
    DKBufferGetBytePtrMethod        getBytePtr;

    DKBufferSetLengthMethod         setLength;
    DKBufferGetMutableBytePtrMethod getMutableBytePtr;
};

typedef const struct DKBufferInterface * DKBufferInterfaceRef;


// Wrappers
DK_API DKIndex      DKBufferGetLength( DKObjectRef _self );
DK_API const void * DKBufferGetBytePtr( DKObjectRef _self, DKIndex index );

DK_API void         DKBufferSetLength( DKObjectRef _self, DKIndex length );
DK_API void *       DKBufferGetMutableBytePtr( DKObjectRef _self, DKIndex index );



#endif // _DK_BUFFER_H_

