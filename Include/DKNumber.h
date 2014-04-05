//
//  DKNumber.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-31.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_NUMBER_H_
#define _DK_NUMBER_H_

#include "DKRuntime.h"


typedef const struct DKNumber * DKNumberRef;


typedef enum
{
    DKNumberVoid = 0,
    
    DKNumberInt32,
    DKNumberInt64,
    DKNumberUInt32,
    DKNumberUInt64,
    DKNumberFloat,
    DKNumberDouble,
    
    DKNumberMaxTypes
    
} DKNumberType;


DKClassRef  DKNumberClass( void );

DKNumberRef DKNumberCreate( const void * value, DKNumberType type, size_t count );

DKNumberRef DKNumberCreateInt32( int32_t x );
DKNumberRef DKNumberCreateInt64( int64_t x );
DKNumberRef DKNumberCreateUInt32( uint32_t x );
DKNumberRef DKNumberCreateUInt64( uint64_t x );
DKNumberRef DKNumberCreateFloat( float x );
DKNumberRef DKNumberCreateDouble( double x );

DKNumberType DKNumberGetType( DKNumberRef _self );
size_t      DKNumberGetCount( DKNumberRef _self );

size_t      DKNumberGetValue( DKNumberRef _self, void * value );
size_t      DKNumberCastValue( DKNumberRef _self, void * value, DKNumberType type );
const void* DKNumberGetValuePtr( DKNumberRef _self );

#define DKNumberGetValueAs( _self, type )     (*((type *)DKNumberGetValuePtr( _self )))

#define DKNumberGetInt32( _self )     (*((int32_t *)DKNumberGetValuePtr( _self )))
#define DKNumberGetInt64( _self )     (*((int64_t *)DKNumberGetValuePtr( _self )))
#define DKNumberGetUInt32( _self )    (*((uint32_t *)DKNumberGetValuePtr( _self )))
#define DKNumberGetUInt64( _self )    (*((uint64_t *)DKNumberGetValuePtr( _self )))
#define DKNumberGetFloat( _self )     (*((float *)DKNumberGetValuePtr( _self )))
#define DKNumberGetDouble( _self )    (*((double *)DKNumberGetValuePtr( _self )))

int         DKNumberEqual( DKNumberRef a, DKNumberRef b );
int         DKNumberCompare( DKNumberRef a, DKNumberRef b );
DKHashCode  DKNumberHash( DKNumberRef _self );

DKStringRef DKNumberCopyDescription( DKNumberRef _self );



#endif // _DK_NUMBER_H_
