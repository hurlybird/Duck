//
//  DKStruct.h
//  Duck
//
//  Created by Derek Nylen on 2014-04-08.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_STRUCT_H_
#define _DK_STRUCT_H_

#include "DKRuntime.h"


typedef const struct DKStruct * DKStructRef;


DKClassRef  DKStructClass( void );

DKStructRef DKStructCreate( DKStringRef semantic, const void * bytes, size_t size );

DKStringRef DKStructGetSemantic( DKStructRef _self );
size_t      DKStructGetValue( DKStructRef _self, DKStringRef semantic, void * bytes, size_t size );

// Macros for creating/retrieving C structs
#define DKSemantic( type )                  DKSTR( #type )

#define DKStructCreateAs( ptr, type )       DKStructCreate( DKSTR( #type ), ptr, sizeof(type) );
#define DKStructGetValueAs( st, dst, type ) DKStructGetValue( st, DKSTR( #type ), dst, sizeof(type) );


#endif
