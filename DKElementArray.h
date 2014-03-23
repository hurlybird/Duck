//
//  DKElementArray.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_ELEMENT_ARRAY_H_
#define _DK_ELEMENT_ARRAY_H_

#include "DKByteArray.h"


typedef struct
{
    DKByteArray byteArray;
    DKIndex elementSize;

} DKElementArray;

void DKElementArrayInit( DKElementArray * array, DKIndex elementSize );
void DKElementArrayReserve( DKElementArray * array, DKIndex length );
void DKElementArrayClear( DKElementArray * array );
void DKElementArrayReplaceElements( DKElementArray * array, DKRange range, const void * elements, DKIndex length );

DKIndex DKElementArrayGetCount( const DKElementArray * array );

void DKElementArrayAppendElement( DKElementArray * array, const void * element );
void DKElementArrayInsertElementAtIndex( DKElementArray * array, DKIndex index, const void * element );
void DKElementArraySetElementAtIndex( DKElementArray * array, DKIndex index, const void * element );
void DKElementArrayRemoveElementAtIndex( DKElementArray * array, DKIndex index );

#define DKElementArrayGetElementAtIndex( array, index, type ) \
    *((type *)&((array)->byteArray.data[(index) * (array)->elementSize]))




#endif // _DK_ELEMENT_ARRAY_H_
