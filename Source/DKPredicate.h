/*****************************************************************************************

  DKPredicate.h

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

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum
{
    DKPredicateUnspecified = 0,

    // Logic
    DKPredicateNOT,     // !A
    DKPredicateAND,     // A && B
    DKPredicateOR,      // A || B
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

    DKPredicateALL,     // OR( a, b, ... )
    DKPredicateANY,     // AND( a, b, ... )
    DKPredicateNONE,    // NOT( OR( a, b, ... ) )

    // Is a member of collection, or substring of string
    DKPredicateIN,
    DKPredicateCONTAINS,
    
    // Is a key in a keyed collection
    DKPredicateKEYIN,
    DKPredicateCONTAINSKEY,
    
    // Is kind of class, or implements an interface
    DKPredicateISA,
    
    // String comparison
    DKPredicateLIKE,
    DKPredicateBEGINSWITH,
    DKPredicateENDSWITH,
    
    // Set comprison
    //DKPredicateISSUBSET,
    //DKPredicateINTERSECTS,
    
    DKMaxPredicateOp

} DKPredicateOp;


// typedef const struct DKPredicate * DKPredicateRef; -- Declared in DKPlatform.h


DK_API DKClassRef DKPredicateClass( void );

#define DKPredicate( op, a, b )                 DKAutorelease( DKPredicateInit( DKAlloc( DKPredicateClass() ), op, a, NULL, b, NULL ) )
#define DKNewPredicate( op, a, b )              DKPredicateInit( DKAlloc( DKPredicateClass() ), op, a, NULL, b, NULL )

#define DKPredicateWithFormat( fmt, ... )       DKAutorelease( DKPredicateInitWithFormat( DKAlloc( DKPredicateClass() ), fmt, __VA_ARGS__ ) )
#define DKNewPredicateWithFormat( fmt, ... )    DKPredicateInit( DKAlloc( DKPredicateClass() ), fmt, __VA_ARGS__ )

DK_API DKObjectRef DKPredicateInit( DKObjectRef _self, DKPredicateOp op, DKObjectRef obj_a, DKStringRef key_a, DKObjectRef b, DKStringRef key_b );
DK_API DKObjectRef DKPredicateInitWithFormat( DKObjectRef _self, DKStringRef fmt, ... );

DK_API bool DKPredicateEvaluate( DKPredicateRef _self );
DK_API bool DKPredicateEvaluateWithObject( DKPredicateRef _self, DKObjectRef subst );

DK_API DKStringRef DKStringFromPredicateOp( DKPredicateOp op );
DK_API DKPredicateOp DKPredicateOpFromString( DKStringRef str );

DK_API bool DKEvaluate( DKObjectRef obj );


#ifdef __cplusplus
}
#endif

#endif // _DK_PREDICATE_H_





