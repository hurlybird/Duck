//
//  DKArray.h
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_ARRAY_H_
#define _DK_ARRAY_H_

#include "DKList.h"


DKTypeRef DKArrayClass( void );
DKTypeRef DKMutableArrayClass( void );

DKListRef DKArrayCreate( DKTypeRef objects[], DKIndex count );
DKListRef DKArrayCreateNoCopy( DKTypeRef objects[], DKIndex count );
DKListRef DKArrayCreateCopy( DKListRef srcList );

DKMutableListRef DKArrayCreateMutable( void );
DKMutableListRef DKArrayCreateMutableCopy( DKListRef srcList );

DKIndex DKArrayGetCount( DKListRef ref );
DKIndex DKArrayGetObjects( DKListRef ref, DKRange range, DKTypeRef objects[] );
void    DKArrayReplaceObjects( DKMutableListRef ref, DKRange range, DKTypeRef objects[], DKIndex count );
void    DKArrayReplaceObjectsWithList( DKMutableListRef ref, DKRange range, DKListRef srcList );



#endif // _DK_ARRAY_H_










