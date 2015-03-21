/*****************************************************************************************

  DKPredicate

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

#ifndef _DK_PREDICATE_H_
#define _DK_PREDICATE_H_

#include "DKRuntime.h"


typedef enum
{
    // Constants
    DKPredicateFALSE,
    DKPredicateTRUE,

    // Logic
    DKPredicateNOT,     // !A
    DKPredicateAND,     // A && B, AND( a, b, ... )
    DKPredicateOR,      // A || B, OR( a, b, ... )
    DKPredicateIS,      // A == B
    DKPredicateISNT,    // A != B
    
    // DKEqual( A, B )
    DKPredicateEQ,
    DKPredicateNEQ,

    // DKCompare( A, B )
    DKPredicateLT,
    DKPredicateLTE,
    DKPredicateGT,
    DKPredicateGTE,

    // Is a member of collection, or substring of string
    DKPredicateIN,
    
    // Is a key in a keyed collection
    DKPredicateKEYIN,
    
    // Is kind of class, or implements an interface
    DKPredicateISA,
    
    // String comparison
    DKPredicateHASPREFIX,
    DKPredicateHASSUFFIX,
    
    // Set comprison
    //DKPredicateISSUBSET,
    //DKPredicateINTERSECTS,
    
    DKMaxPredicateOp

} DKPredicateOp;


// typedef const struct DKPredicate * DKPredicateRef; -- Declared in DKPlatform.h


DKClassRef DKPredicateClass( void );

DKPredicateRef DKPredicateCreate( DKPredicateOp op, DKObjectRef a, DKObjectRef b );

bool DKPredicateEvaluate( DKPredicateRef _self );
bool DKPredicateEvaluateWithObject( DKPredicateRef _self, DKObjectRef subst );

DKStringRef DKStringFromPredicateOp( DKPredicateOp op );
DKPredicateOp DKPredicateOpFromString( DKStringRef str );

bool DKEvaluate( DKObjectRef obj );




#endif // _DK_PREDICATE_H_





