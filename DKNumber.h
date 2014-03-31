//
//  DKNumber.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-31.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_NUMBER_H_
#define _DK_NUMBER_H_

#include "DKEnv.h"


typedef DKTypeRef DKNumberRef;


typedef enum
{
    DKNumberTypeUnspecified = 0,
    
    DKNumberInt32,
    DKNumberInt64,
    DKNumberUInt32,
    DKNumberUInt64,
    DKNumberFloat,
    DKNumberDouble,
    
    DKNumberMaxTypes
    
} DKNumberType;


DKTypeRef   DKNumberClass( void );

DKNumberRef DKNumberCreate( DKNumberType type, size_t count, const void * value );

DKNumberRef DKNumberCreateInt32( int32_t x );
DKNumberRef DKNumberCreateInt64( int64_t x );
DKNumberRef DKNumberCreateUInt32( uint32_t x );
DKNumberRef DKNumberCreateUInt64( uint64_t x );
DKNumberRef DKNumberCreateFloat( float x );
DKNumberRef DKNumberCreateDouble( double x );

DKNumberType DKNumberGetType( DKNumberRef ref );
size_t      DKNumberGetCount( DKNumberRef ref );

size_t      DKNumberGetValue( DKNumberRef ref, void * value );
size_t      DKNumberCastValue( DKNumberRef ref, void * value, DKNumberType type );
const void* DKNumberGetValuePtr( DKNumberRef ref );

#define DKNumberGetValueAs( ref, type )     (*((type *)DKNumberGetValuePtr( ref ))

#define DKNumberGetInt32( ref )     (*((int32_t *)DKNumberGetValuePtr( ref ))
#define DKNumberGetInt64( ref )     (*((int64_t *)DKNumberGetValuePtr( ref ))
#define DKNumberGetUInt32( ref )    (*((uint32_t *)DKNumberGetValuePtr( ref ))
#define DKNumberGetUInt64( ref )    (*((uint64_t *)DKNumberGetValuePtr( ref ))
#define DKNumberGetFloat( ref )     (*((float *)DKNumberGetValuePtr( ref ))
#define DKNumberGetDouble( ref )    (*((double *)DKNumberGetValuePtr( ref ))





#endif // _DK_NUMBER_H_
