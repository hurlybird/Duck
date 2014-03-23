//
//  DKReferenceCounting.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-22.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_REFERENCE_COUNTING_H_
#define _DK_REFERENCE_COUNTING_H_

#include "DKRuntime.h"


DKDeclareInterface( ReferenceCounting );


struct DKReferenceCounting
{
    DKInterface _interface;
    
    DKTypeRef   (*retain)( DKTypeRef ref );
    void        (*release)( DKTypeRef ref );
};

typedef const struct DKReferenceCounting DKReferenceCounting;


DKTypeRef   DKDefaultRetainImp( DKTypeRef ref );
void        DKDefaultReleaseImp( DKTypeRef ref );

DKTypeRef   DKStaticObjectRetainImp( DKTypeRef ref );
void        DKStaticObjectReleaseImp( DKTypeRef ref );

DKReferenceCounting * DKDefaultReferenceCounting( void );
DKReferenceCounting * DKStaticObjectReferenceCounting( void );


#endif // _DK_REFERENCE_COUNTING_H_
