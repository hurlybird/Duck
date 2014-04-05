//
//  DKPointerArray.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_POINTER_ARRAY_H_
#define _DK_POINTER_ARRAY_H_

#include "DKPlatform.h"


typedef struct
{
    uintptr_t * data;
    DKIndex length;
    DKIndex maxLength;

} DKPointerArray;


void DKPointerArrayInit( DKPointerArray * array );

void DKPointerArrayInitWithExternalStorage( DKPointerArray * array, const uintptr_t pointers[], DKIndex length );
int  DKPointerArrayHasExternalStorage( DKPointerArray * array );

void DKPointerArrayFinalize( DKPointerArray * array );

void DKPointerArrayReserve( DKPointerArray * array, DKIndex length );

void DKPointerArrayReplacePointers( DKPointerArray * array, DKRange range, const uintptr_t pointers[], DKIndex length );
void DKPointerArrayAppendPointer( DKPointerArray * array, uintptr_t pointer );

void DKPointerArraySort( DKPointerArray * array, DKCompareFunction cmp );
void DKPointerArrayShuffle( DKPointerArray * array );



#endif // _DK_ELEMENT_ARRAY_H_