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

#include <libkern/OSAtomic.h>


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


// Flags
typedef uint32_t DKOptionFlags;


// Atomic Integers
//typedef int32_t DKAtomicInt;
//#define DKAtomicIncrement( ptr )    __sync_add_and_fetch( ptr, 1 )
//#define DKAtomicDecrement( ptr )    __sync_sub_and_fetch( ptr, 1 )

typedef int32_t DKAtomicInt;
#define DKAtomicIncrement( ptr )    OSAtomicIncrement32( ptr )
#define DKAtomicDecrement( ptr )    OSAtomicDecrement32( ptr )


// Memory Allocation
void   DKSetAllocCallback( void * (*callback)( size_t size ) );
void   DKSetFreeCallback( void (*callback)( void * ptr ) );

void * DKAlloc( size_t size );
void * DKAllocAndZero( size_t size );
void   DKFree( void * ptr );


// Hash functions
DKHashIndex DKPtrHash( const void * ptr );
DKHashIndex DKStrHash( const void * str );
DKHashIndex DKMemHash( const void * buffer, size_t buffer_size );



#endif // _DK_ENV_H_

