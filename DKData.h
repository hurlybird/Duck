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


DKClassRef  DKDataClass( void );
DKClassRef  DKMutableDataClass( void );

DKDataRef   DKDataCreate( void );
DKDataRef   DKDataCreateCopy( DKDataRef srcData );
DKDataRef   DKDataCreateWithBytes( const void * bytes, DKIndex length );
DKDataRef   DKDataCreateWithBytesNoCopy( const void * bytes, DKIndex length );

DKMutableDataRef DKDataCreateMutable( void );
DKMutableDataRef DKDataCreateMutableCopy( DKDataRef srcData );

int         DKDataEqual( DKDataRef _self, DKObjectRef other );
int         DKDataCompare( DKDataRef _self, DKDataRef other );
DKHashCode  DKDataHash( DKDataRef _self );

DKIndex     DKDataGetLength( DKDataRef _self );
void        DKDataSetLength( DKMutableDataRef _self, DKIndex length );
void        DKDataIncreaseLength( DKMutableDataRef _self, DKIndex length );

const void * DKDataGetBytePtr( DKDataRef _self );
const void * DKDataGetByteRange( DKDataRef _self, DKRange range );

void *      DKDataGetMutableBytePtr( DKMutableDataRef _self );
void *      DKDataGetMutableByteRange( DKMutableDataRef _self, DKRange range );

DKIndex     DKDataGetBytes( DKDataRef _self, DKRange range, void * buffer );

void        DKDataReplaceBytes( DKMutableDataRef _self, DKRange range, const void * bytes, DKIndex length );
void        DKDataAppendBytes( DKMutableDataRef _self, const void * bytes, DKIndex length );
void        DKDataDeleteBytes( DKMutableDataRef _self, DKRange range );

int         DKDataSeek( DKDataRef _self, DKIndex offset, int origin );
DKIndex     DKDataTell( DKDataRef _self );

DKIndex     DKDataRead( DKDataRef _self, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKDataWrite( DKMutableDataRef _self, const void * buffer, DKIndex size, DKIndex count );


#endif // _DK_DATA_H_


