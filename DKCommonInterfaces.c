//
//  DKCommonInterfaces.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-22.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKCommonInterfaces.h"


// LifeCycle =============================================================================
DKDefineInterface( LifeCycle );

///
//  DKDefaultAllocateImp()
//
DKTypeRef DKDefaultAllocateImp( void )
{
    assert( 0 );
    return NULL;
}


///
//  DKDefaultInitializeImp()
//
DKTypeRef DKDefaultInitializeImp( DKTypeRef ref )
{
    return ref;
}


///
//  DKDefaultFinalizeImp()
//
void DKDefaultFinalizeImp( DKTypeRef ref )
{
}


///
//  DKDefaultLifeCycle()
//
DKLifeCycle * DKDefaultLifeCycle( void )
{
    static struct DKLifeCycle * lifeCycle = NULL;
    
    if( !lifeCycle )
    {
        lifeCycle = (struct DKLifeCycle *)DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKDefaultAllocateImp;
        lifeCycle->initialize = DKDefaultInitializeImp;
        lifeCycle->finalize = DKDefaultFinalizeImp;
    }
    
    return lifeCycle;
}




// ReferenceCounting =====================================================================
DKDefineInterface( ReferenceCounting );

///
//  DKDefaultRetainImp()
//
DKTypeRef DKDefaultRetainImp( DKTypeRef ref )
{
    if( ref )
    {
        struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
        DKAtomicIncrement( &obj->refcount );
    }

    return ref;
}


///
//  DKDefaultReleaseImp()
//
void DKDefaultReleaseImp( DKTypeRef ref )
{
    if( ref )
    {
        struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
        DKAtomicInt n = DKAtomicDecrement( &obj->refcount );
        
        assert( n >= 0 );
        
        if( n == 0 )
        {
            DKFreeObject( ref );
        }
    }
}


///
//  DKDefaultReferenceCounting()
//
DKReferenceCounting * DKDefaultReferenceCounting( void )
{
    static struct DKReferenceCounting * referenceCounting = NULL;
    
    if( !referenceCounting )
    {
        referenceCounting = (struct DKReferenceCounting *)DKAllocInterface( DKSelector(ReferenceCounting), sizeof(DKReferenceCounting) );
        referenceCounting->retain = DKDefaultRetainImp;
        referenceCounting->release = DKDefaultReleaseImp;
    }
    
    return referenceCounting;
}


///
//  DKStaticObjectRetainImp()
//
DKTypeRef DKStaticObjectRetainImp( DKTypeRef ref )
{
    return ref;
}


///
//  DKStaticObjectReleaseImp()
//
void DKStaticObjectReleaseImp( DKTypeRef ref )
{
}


///
//  DKStaticObjectReferenceCounting()
//
DKReferenceCounting * DKStaticObjectReferenceCounting( void )
{
    static struct DKReferenceCounting * referenceCounting = NULL;
    
    if( !referenceCounting )
    {
        referenceCounting = (struct DKReferenceCounting *)DKAllocInterface( DKSelector(ReferenceCounting), sizeof(DKReferenceCounting) );
        referenceCounting->retain = DKStaticObjectRetainImp;
        referenceCounting->release = DKStaticObjectReleaseImp;
    }
    
    return referenceCounting;
}





// Introspection =========================================================================
DKDefineInterface( Introspection );

///
//  DKDefaultQueryInterfaceImp()
//
DKTypeRef DKDefaultQueryInterfaceImp( DKTypeRef ref, DKSEL sel )
{
    return DKLookupInterface( ref, sel );
}


///
//  DKDefaultQueryMethodImp()
//
DKTypeRef DKDefaultQueryMethodImp( DKTypeRef ref, DKSEL sel )
{
    return DKLookupMethod( ref, sel );
}


///
//  DKDefaultIntrospection()
//
DKIntrospection * DKDefaultIntrospection( void )
{
    static struct DKIntrospection * introspection = NULL;
    
    if( !introspection )
    {
        introspection = (struct DKIntrospection *)DKAllocInterface( DKSelector(Introspection), sizeof(DKIntrospection) );
        introspection->queryInterface = DKDefaultQueryInterfaceImp;
        introspection->queryMethod = DKDefaultQueryMethodImp;
    }
    
    return introspection;
}




// Comparison ============================================================================
DKDefineInterface( Comparison );

///
//  DKDefaultEqualImp()
//
int DKDefaultEqualImp( DKTypeRef ref, DKTypeRef other )
{
    return ref == other;
}


///
//  DKDefaultCompareImp()
//
int DKDefaultCompareImp( DKTypeRef ref, DKTypeRef other )
{
    if( ref < other )
        return 1;
    
    if( ref > other )
        return -1;
    
    return 0;
}


///
//  DKDefaultHashImp()
//
DKHashIndex DKDefaultHashImp( DKTypeRef ref )
{
    assert( sizeof(DKHashIndex) == sizeof(DKTypeRef) );
    return (DKHashIndex)ref;
}


///
//  DKDefaultComparison()
//
DKComparison * DKDefaultComparison( void )
{
    static struct DKComparison * comparison = NULL;
    
    if( !comparison )
    {
        comparison = (struct DKComparison *)DKAllocInterface( DKSelector(Comparison), sizeof(DKComparison) );
        comparison->equal = DKDefaultEqualImp;
        comparison->compare = DKDefaultCompareImp;
        comparison->hash = DKDefaultHashImp;
    }
    
    return comparison;
}


///
//  DKInterfaceEqualImp()
//
int DKInterfaceEqualImp( DKTypeRef ref, DKTypeRef other )
{
    const DKInterface * thisInterface = ref;
    const DKInterface * otherInterface = other;

    return DKEqual( thisInterface->sel, otherInterface->sel );
}


///
//  DKInterfaceCompareImp()
//
int DKInterfaceCompareImp( DKTypeRef ref, DKTypeRef other )
{
    const DKInterface * thisInterface = ref;
    const DKInterface * otherInterface = other;

    return DKCompare( thisInterface->sel, otherInterface->sel );
}


///
//  DKInterfaceHashImp()
//
DKHashIndex DKInterfaceHashImp( DKTypeRef ref )
{
    const DKInterface * interface = ref;
    
    return DKHash( interface->sel );
}


///
//  DKInterfaceComparison()
//
DKComparison * DKInterfaceComparison( void )
{
    static struct DKComparison * comparison = NULL;
    
    if( !comparison )
    {
        comparison = (struct DKComparison *)DKAllocInterface( DKSelector(Comparison), sizeof(DKComparison) );
        comparison->equal = DKInterfaceEqualImp;
        comparison->compare = DKInterfaceCompareImp;
        comparison->hash = DKInterfaceHashImp;
    }
    
    return comparison;
}









