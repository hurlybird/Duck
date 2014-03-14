//
//  DKEnv.h
//  Duck
//
//  Created by Derek Nylen on 11-12-07.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#ifndef _DK_ENV_H_
#define _DK_ENV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "DKEnvApple.h"



typedef const void * DKTypeRef;


// SUID
typedef const char * DKSUID;

#define DKDeclareSUID( x )  extern DKSUID x
#define DKDefineSUID( x )   DKSUID x = #x


// Index Types
typedef intptr_t DKIndex;
typedef uintptr_t DKHashIndex;

typedef struct
{
    DKIndex location;
    DKIndex length;

} DKRange;

#define DKRangeMake( loc, len )     (const DKRange){ loc, len }
#define DKRangeEnd( range )         (((range).location) + ((range).length))

enum
{
    DKNotFound = -1,
};


// Memory Allocation
void   DKSetAllocCallback( void * (*callback)( size_t size ) );
void   DKSetFreeCallback( void (*callback)( void * ptr ) );

void * DKAlloc( size_t size );
void * DKAllocAndZero( size_t size );
void   DKFree( void * ptr );


// Equal
int DKPtrEqual( const void * a, const void * b );
int DKStrEqual( const void * a, const void * b );

// Compare
int DKPtrCompare( const void * a, const void * b );
int DKStrCompare( const void * a, const void * b );

// Hash
DKHashIndex DKPtrHash( const void * ptr );
DKHashIndex DKStrHash( const void * str );
DKHashIndex DKMemHash( const void * buffer, size_t buffer_size );



#endif // _DK_ENV_H_







