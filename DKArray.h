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


typedef const struct DKArray * DKArrayRef;
typedef struct DKArray * DKMutableArrayRef;


DKClassRef DKArrayClass( void );
DKClassRef DKMutableArrayClass( void );

DKArrayRef DKArrayCreate( void );
DKArrayRef DKArrayCreateWithObjects( DKObjectRef firstObject, ... );
DKArrayRef DKArrayCreateWithCArray( DKObjectRef objects[], DKIndex count );
DKArrayRef DKArrayCreateWithCArrayNoCopy( DKObjectRef objects[], DKIndex count );
DKArrayRef DKArrayCreateCopy( DKListRef srcList );

DKMutableArrayRef DKArrayCreateMutable( void );
DKMutableArrayRef DKArrayCreateMutableCopy( DKListRef srcList );

DKIndex DKArrayGetCount( DKArrayRef _self );
DKIndex DKArrayGetObjects( DKArrayRef _self, DKRange range, DKObjectRef objects[] );

void    DKArrayReplaceObjects( DKMutableArrayRef _self, DKRange range, DKObjectRef objects[], DKIndex count );
void    DKArrayReplaceObjectsWithList( DKMutableArrayRef _self, DKRange range, DKListRef srcList );

void    DKArraySort( DKMutableArrayRef _self, DKCompareFunction cmp );
void    DKArrayShuffle( DKMutableArrayRef _self );


#endif // _DK_ARRAY_H_










