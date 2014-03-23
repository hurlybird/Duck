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
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "DKEnvApple.h"



typedef const void * DKTypeRef;


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


int DKStrEqual( const void * a, const void * b );
int DKStrLexicalCmp( const void * a, const void * b );

DKHashIndex DKStrHash( const void * str );
DKHashIndex DKMemHash( const void * buffer, size_t buffer_size );



#endif // _DK_ENV_H_







