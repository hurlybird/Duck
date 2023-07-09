/*****************************************************************************************

  DKAllocation.h

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

#ifndef _DK_ALLOCATION_H_
#define _DK_ALLOCATION_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Allocation );


typedef DKObjectRef (*DKAllocMethod)( DKClassRef _class, size_t extraBytes );
typedef void (*DKDeallocMethod)( DKObjectRef _self );

struct DKAllocationInterface
{
    const DKInterface _interface;
 
    DKAllocMethod       alloc;
    DKDeallocMethod     dealloc;
};


typedef const struct DKAllocationInterface * DKAllocationInterfaceRef;


// Default allocation interface that maps to DKAllocObject and DKDeallocObject. This
// is used by the root classes so it's defined in DKRuntime.c.
DK_API DKInterfaceRef DKDefaultAllocation( void );



#ifdef __cplusplus
}
#endif

#endif // _DK_ALLOCATION_H_



