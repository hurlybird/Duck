//
//  DKCommonInterfaces.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-22.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_COMMON_INTERFACES_H_
#define _DK_COMMON_INTERFACES_H_

#include "DKRuntime.h"


// Common Interfaces Supported By All Objects ============================================

// Life-Cycle ============================================================================
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




// Reference Counting ====================================================================
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




// Introspection =========================================================================
DKDeclareInterface( Introspection );

struct DKIntrospection
{
    DKInterface _interface;
    
    DKTypeRef   (*queryInterface)( DKTypeRef ref, DKSEL sel );
    DKTypeRef   (*queryMethod)( DKTypeRef ref, DKSEL sel );
};

typedef const struct DKIntrospection DKIntrospection;

DKTypeRef   DKDefaultQueryInterfaceImp( DKTypeRef ref, DKSEL sel );
DKTypeRef   DKDefaultQueryMethodImp( DKTypeRef ref, DKSEL sel );

DKIntrospection * DKDefaultIntrospection( void );




// Comparison ============================================================================
DKDeclareInterface( Comparison );

struct DKComparison
{
    DKInterface _interface;
    
    int         (*equal)( DKTypeRef ref, DKTypeRef other );
    int         (*compare)( DKTypeRef ref, DKTypeRef other );
    DKHashIndex (*hash)( DKTypeRef ref );
};

typedef const struct DKComparison DKComparison;

int         DKDefaultEqualImp( DKTypeRef ref, DKTypeRef other );
int         DKDefaultCompareImp( DKTypeRef ref, DKTypeRef other );
DKHashIndex DKDefaultHashImp( DKTypeRef ptr );

int         DKInterfaceEqualImp( DKTypeRef ref, DKTypeRef other );
int         DKInterfaceCompareImp( DKTypeRef ref, DKTypeRef other );
DKHashIndex DKInterfaceHashImp( DKTypeRef ref );

DKComparison * DKDefaultComparison( void );
DKComparison * DKInterfaceComparison( void );



#endif // _DK_COMMON_INTERFACES_H_






