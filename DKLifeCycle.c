//
//  DKCommonInterfaces.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-22.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKLifeCycle.h"


DKDefineFastLookupInterface( LifeCycle );


///
//  DKDefaultAllocateImp()
//
DKTypeRef DKDefaultAllocateImp( void )
{
    DKError( "DKLifeCycle: The allocate interface is undefined." );
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







