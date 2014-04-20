/*****************************************************************************************

  DKNumber.h

  Copyright (c) 2014 Derek W. Nylen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*****************************************************************************************/

#ifndef _DK_NUMBER_H_
#define _DK_NUMBER_H_

#include "DKRuntime.h"


typedef const struct DKNumber * DKNumberRef;

typedef enum
{
    DKNumberComponentInt8 =     1,
    DKNumberComponentInt16,
    DKNumberComponentInt32,
    DKNumberComponentInt64,
    
    DKNumberComponentUInt8,
    DKNumberComponentUInt16,
    DKNumberComponentUInt32,
    DKNumberComponentUInt64,
    
    DKNumberComponentFloat,
    DKNumberComponentDouble,
    
    DKNumberMaxComponentTypes,
    
} DKNumberComponentType;

typedef int32_t DKNumberType;

#define DKNumberMaxComponentCount               16

#define DKNumberMakeVectorType( type, count )   (((count) << 16) | (type & 0x0000FFFF))

#define DKNumberInt8    DKNumberMakeVectorType( DKNumberComponentInt8, 1 )
#define DKNumberInt16   DKNumberMakeVectorType( DKNumberComponentInt16, 1 )
#define DKNumberInt32   DKNumberMakeVectorType( DKNumberComponentInt32, 1 )
#define DKNumberInt64   DKNumberMakeVectorType( DKNumberComponentInt64, 1 )

#define DKNumberUInt8   DKNumberMakeVectorType( DKNumberComponentUInt8, 1 )
#define DKNumberUInt16  DKNumberMakeVectorType( DKNumberComponentUInt16, 1 )
#define DKNumberUInt32  DKNumberMakeVectorType( DKNumberComponentUInt32, 1 )
#define DKNumberUInt64  DKNumberMakeVectorType( DKNumberComponentUInt64, 1 )

#define DKNumberFloat   DKNumberMakeVectorType( DKNumberComponentFloat, 1 )
#define DKNumberDouble  DKNumberMakeVectorType( DKNumberComponentDouble, 1 )

#define DKNumberGetComponentType( type )    ((type) & 0x0000FFFF)
#define DKNumberGetComponentCount( type )   ((type) >> 16)

size_t  DKNumberGetComponentSize( DKNumberType type );
const char * DKNumberGetComponentName( DKNumberType type );
int     DKNumberTypeIsValid( int32_t type );




// DKNumber ==============================================================================

DKClassRef  DKNumberClass( void );

DKNumberRef DKNumberCreate( const void * value, DKNumberType type );

DKNumberRef DKNumberCreateInt32( int32_t x );
DKNumberRef DKNumberCreateInt64( int64_t x );
DKNumberRef DKNumberCreateUInt32( uint32_t x );
DKNumberRef DKNumberCreateUInt64( uint64_t x );
DKNumberRef DKNumberCreateFloat( float x );
DKNumberRef DKNumberCreateDouble( double x );

DKNumberType DKNumberGetType( DKNumberRef _self );

size_t      DKNumberGetValue( DKNumberRef _self, void * value );
size_t      DKNumberCastValue( DKNumberRef _self, void * value, DKNumberType type );
const void* DKNumberGetValuePtr( DKNumberRef _self );

#define     DKNumberGetValueAs( _self, type )     (*((type *)DKNumberGetValuePtr( _self )))

#define     DKNumberGetInt32( _self )     (*((int32_t *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetInt64( _self )     (*((int64_t *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetUInt32( _self )    (*((uint32_t *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetUInt64( _self )    (*((uint64_t *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetFloat( _self )     (*((float *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetDouble( _self )    (*((double *)DKNumberGetValuePtr( _self )))

int         DKNumberEqual( DKNumberRef a, DKNumberRef b );
int         DKNumberCompare( DKNumberRef a, DKNumberRef b );
DKHashCode  DKNumberHash( DKNumberRef _self );

DKStringRef DKNumberCopyDescription( DKNumberRef _self );

// Utility function for converting number types
size_t DKNumberConvert( const void * src, DKNumberType srcType, void * dst, DKNumberType dstType );


#endif // _DK_NUMBER_H_
