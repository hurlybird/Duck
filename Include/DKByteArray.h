//
//  DKByteArray.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_BYTE_ARRAY_H_
#define _DK_BYTE_ARRAY_H_

#include "DKPlatform.h"


// Note: DKByteArray internally stores four '\0' bytes (i.e. a UTF32 NULL) at data[length]
// to make storing strings safer. The NULLs aren't included in the length or maxLength of
// the array.

typedef struct
{
    uint8_t * data;
    DKIndex length;
    DKIndex maxLength;

} DKByteArray;


void DKByteArrayInit( DKByteArray * array );

void DKByteArrayInitWithExternalStorage( DKByteArray * array, const uint8_t bytes[], DKIndex length );
int  DKByteArrayHasExternalStorage( DKByteArray * array );

void DKByteArrayFinalize( DKByteArray * array );

void DKByteArrayReserve( DKByteArray * array, DKIndex length );

void DKByteArrayReplaceBytes( DKByteArray * array, DKRange range, const uint8_t bytes[], DKIndex length );
void DKByteArrayAppendBytes( DKByteArray * array, const uint8_t bytes[], DKIndex length );


#endif // _DK_BYTE_ARRAY_H_
