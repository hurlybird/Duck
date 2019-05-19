/*****************************************************************************************

  DKComparison.h

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

#ifndef _DK_COMPARISON_H_
#define _DK_COMPARISON_H_

#include "DKRuntime.h"


DKDeclareInterfaceSelector( Comparison );


typedef DKEqualityFunction DKEqualityMethod;
typedef DKCompareFunction DKCompareMethod;
typedef DKHashFunction DKHashMethod;

struct DKComparisonInterface
{
    const DKInterface _interface;
    
    DKEqualityMethod    equal;
    DKCompareMethod     compare;
    DKHashMethod        hash;
};

typedef const struct DKComparisonInterface * DKComparisonInterfaceRef;


// Default comparison interface that implements pointer comparison/equality. This
// is used by the root classes so it's defined in DKRuntime.c.
DK_API DKInterfaceRef DKDefaultComparison( void );


// Pointer equality, comparison and hashing
DK_API bool        DKPointerEqual( DKObjectRef _self, DKObjectRef other );
DK_API int         DKPointerCompare( DKObjectRef _self, DKObjectRef other );
DK_API DKHashCode  DKPointerHash( DKObjectRef _self );


// Wrappers for the comparison interface
DK_API bool        DKEqual( DKObjectRef a, DKObjectRef b );
DK_API int         DKCompare( DKObjectRef a, DKObjectRef b );
DK_API int         DKReverseCompare( DKObjectRef a, DKObjectRef b );
DK_API DKHashCode  DKHash( DKObjectRef _self );





#endif // _DK_COMPARISON_H_



