// =======================================================================================
//
// DKComparison.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKComparison.h"


// The comparison selector is initialized by DKRuntimeInit() so that constant strings can
// be used during initialization.
//DKThreadSafeFastSelectorInit( Comparison );


///
//  DKPointerEqual()
//
bool DKPointerEqual( DKObjectRef _self, DKObjectRef other )
{
    return _self == other;
}


///
//  DKPointerCompare()
//
int DKPointerCompare( DKObjectRef _self, DKObjectRef other )
{
    if( _self < other )
        return -1;
    
    if( _self > other )
        return 1;
    
    return 0;
}


///
//  DKPointerHash()
//
DKHashCode DKPointerHash( DKObjectRef _self )
{
    return DKObjectUniqueHash( _self );
}


///
//  DKEqual()
//
bool DKEqual( DKObjectRef a, DKObjectRef b )
{
    if( a == b )
    {
        return true;
    }

    if( a && b )
    {
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->equal( a, b );
    }
    
    return false;
}


///
//  DKCompare()
//
int DKCompare( DKObjectRef a, DKObjectRef b )
{
    if( a == b )
    {
        return 0;
    }

    if( a && b )
    {
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->compare( a, b );
    }
    
    return a < b ? -1 : 1;
}


///
//  DKReverseCompare()
//
int DKReverseCompare( DKObjectRef a, DKObjectRef b )
{
    return -DKCompare( a, b );
}


///
//  DKHash()
//
DKHashCode DKHash( DKObjectRef _self )
{
    if( _self )
    {
        DKComparisonInterfaceRef comparison = DKGetInterface( _self, DKSelector(Comparison) );
        return comparison->hash( _self );
    }
    
    return 0;
}




