//
//  DKReferenceCounting.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-22.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKReferenceCounting.h"


DKDefineFastLookupInterface( ReferenceCounting );


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





