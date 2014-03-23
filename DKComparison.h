//
//  DKComparison.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-22.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_COMPARISON_H_
#define _DK_COMPARISON_H_

#include "DKRuntime.h"


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



#endif // _DK_COMPARISON_H_
