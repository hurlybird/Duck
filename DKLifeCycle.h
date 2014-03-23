//
//  DKCommonInterfaces.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-22.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_LIFE_CYCLE_H_
#define _DK_LIFE_CYCLE_H_

#include "DKRuntime.h"


DKDeclareInterface( LifeCycle );


struct DKLifeCycle
{
    DKInterface _interface;
    
    DKTypeRef   (*allocate)( void );
    DKTypeRef   (*initialize)( DKTypeRef ref );
    void        (*finalize)( DKTypeRef ref );
};

typedef const struct DKLifeCycle DKLifeCycle;



DKTypeRef   DKDefaultAllocateImp( void );
DKTypeRef   DKDefaultInitializeImp( DKTypeRef ref );
void        DKDefaultFinalizeImp( DKTypeRef ref );

DKLifeCycle * DKDefaultLifeCycle( void );




#endif // _DK_LIFE_CYCLE_H_






