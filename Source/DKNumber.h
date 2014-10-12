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
#include "DKEncoding.h"


#define DKNumberInt8    DKEncode( DKEncodingTypeInt8, 1 )
#define DKNumberInt16   DKEncode( DKEncodingTypeInt16, 1 )
#define DKNumberInt32   DKEncode( DKEncodingTypeInt32, 1 )
#define DKNumberInt64   DKEncode( DKEncodingTypeInt64, 1 )

#define DKNumberUInt8   DKEncode( DKEncodingTypeUInt8, 1 )
#define DKNumberUInt16  DKEncode( DKEncodingTypeUInt16, 1 )
#define DKNumberUInt32  DKEncode( DKEncodingTypeUInt32, 1 )
#define DKNumberUInt64  DKEncode( DKEncodingTypeUInt64, 1 )

#define DKNumberFloat   DKEncode( DKEncodingTypeFloat, 1 )
#define DKNumberDouble  DKEncode( DKEncodingTypeDouble, 1 )

#define DKNumberUUID    DKEncode( DKEncodingTypeUInt8, 16 )
#define DKNumberDate    DKEncode( DKEncodingTypeDouble, 1 )


typedef struct DKNumber * DKNumberRef;

DKClassRef  DKNumberClass( void );

#define     DKNumberCreate( value, encoding )   DKNumberInit( DKAlloc( DKNumberClass(), 0 ), value, encoding )

DKNumberRef DKNumberCreateInt32( int32_t x );
DKNumberRef DKNumberCreateInt64( int64_t x );
DKNumberRef DKNumberCreateUInt32( uint32_t x );
DKNumberRef DKNumberCreateUInt64( uint64_t x );
DKNumberRef DKNumberCreateFloat( float x );
DKNumberRef DKNumberCreateDouble( double x );
DKNumberRef DKNumberCreateUUID( const DKUUID * uuid );
DKNumberRef DKNumberCreateDate( const DKDateTime * date );

DKNumberRef DKNumberInit( DKNumberRef _self, const void * value, DKEncoding encoding );

DKEncoding  DKNumberGetEncoding( DKNumberRef _self );

size_t      DKNumberGetValue( DKNumberRef _self, void * value );
size_t      DKNumberCastValue( DKNumberRef _self, void * value, DKEncoding encoding );
const void* DKNumberGetValuePtr( DKNumberRef _self );

#define     DKNumberGetValueAs( _self, type )     (*((type *)DKNumberGetValuePtr( _self )))

#define     DKNumberGetInt32( _self )     (*((int32_t *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetInt64( _self )     (*((int64_t *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetUInt32( _self )    (*((uint32_t *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetUInt64( _self )    (*((uint64_t *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetFloat( _self )     (*((float *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetDouble( _self )    (*((double *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetUUID( _self )      (*((DKUUID *)DKNumberGetValuePtr( _self )))
#define     DKNumberGetDate( _self )      (*((DKDateTime *)DKNumberGetValuePtr( _self )))

bool        DKNumberEqual( DKNumberRef a, DKNumberRef b );
int         DKNumberCompare( DKNumberRef a, DKNumberRef b );
DKHashCode  DKNumberHash( DKNumberRef _self );

DKStringRef DKNumberGetDescription( DKNumberRef _self );

// Utility function for converting number types
size_t DKNumberConvert( const void * src, DKEncoding srcType, void * dst, DKEncoding dstType );


#endif // _DK_NUMBER_H_
