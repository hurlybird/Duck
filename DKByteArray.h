//
//  DKByteArray.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_BYTE_ARRAY_H_
#define _DK_BYTE_ARRAY_H_

#include "DKEnv.h"


typedef struct
{
    uint8_t * data;
    DKIndex length;
    DKIndex maxLength;

} DKByteArray;


void DKByteArrayInit( DKByteArray * array );
void DKByteArrayReserve( DKByteArray * array, DKIndex length );
void DKByteArrayClear( DKByteArray * array );
void DKByteArrayReplaceBytes( DKByteArray * array, DKRange range, const void * bytes, DKIndex length );



#endif // _DK_BYTE_ARRAY_H_
