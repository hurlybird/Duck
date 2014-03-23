//
//  DKData.h
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_DATA_H_
#define _DK_DATA_H_

#include "DKRuntime.h"


typedef const struct DKData * DKDataRef;
typedef struct DKData * DKMutableDataRef;


DKTypeRef   DKDataClass( void );
DKTypeRef   DKMutableDataClass( void );

DKDataRef   DKDataCreate( const void * bytes, DKIndex length );
DKDataRef   DKDataCreateCopy( DKDataRef srcData );
DKDataRef   DKDataCreateWithBytesNoCopy( const void * bytes, DKIndex length );

DKMutableDataRef DKDataCreateMutable( void );
DKMutableDataRef DKDataCreateMutableCopy( DKDataRef srcData );

DKIndex     DKDataGetLength( DKDataRef ref );
void        DKDataSetLength( DKMutableDataRef ref, DKIndex length );
void        DKDataIncreaseLength( DKMutableDataRef ref, DKIndex length );

const void * DKDataGetBytePtr( DKDataRef ref );
const void * DKDataGetByteRange( DKDataRef ref, DKRange range );

void *      DKDataGetMutableBytePtr( DKMutableDataRef ref );
void *      DKDataGetMutableByteRange( DKMutableDataRef ref, DKRange range );

DKIndex     DKDataGetBytes( DKDataRef ref, DKRange range, void * buffer );

void        DKDataReplaceBytes( DKMutableDataRef ref, DKRange range, const void * bytes, DKIndex length );
void        DKDataAppendBytes( DKMutableDataRef ref, const void * bytes, DKIndex length );
void        DKDataDeleteBytes( DKMutableDataRef ref, DKRange range );



#endif // _DK_DATA_H_


