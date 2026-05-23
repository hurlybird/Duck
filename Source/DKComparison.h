// =======================================================================================
//
// DKComparison.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_COMPARISON_H_
#define _DK_COMPARISON_H_

#ifdef __cplusplus
extern "C"
{
#endif


DK_API DKDeclareInterfaceSelector( Comparison );


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



#ifdef __cplusplus
}
#endif

#endif // _DK_COMPARISON_H_



