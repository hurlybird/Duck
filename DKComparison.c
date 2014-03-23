//
//  DKComparison.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-22.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKComparison.h"


DKDefineFastLookupInterface( Comparison );


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
    DKAssert( sizeof(DKHashIndex) == sizeof(DKTypeRef) );
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




