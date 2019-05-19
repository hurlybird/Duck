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


// typedef struct DKNumber * DKNumberRef; -- Declared in DKPlatform.h

DK_API DKClassRef  DKNumberClass( void );
DK_API DKClassRef  DKVariableNumberClass( void );

#define            DKNumber( value, encoding )                     DKAutorelease( DKNumberInit( DKAlloc( DKNumberClass() ), value, encoding ) )
#define            DKNewNumber( value, encoding )                  DKNumberInit( DKAlloc( DKNumberClass() ), value, encoding )

#define            DKNumberWithNumber( number, encodingType )      DKAutorelease( DKNumberInitWithNumber( DKAlloc( DKNumberClass() ), number, encodingType ) )
#define            DKNewNumberWithNumber( number, encodingType )   DKNumberInitWithNumber( DKAlloc( DKNumberClass() ), number, encodingType )

#define            DKVariableNumber( value, encoding )             DKAutorelease( DKNumberInit( DKAlloc( DKVariableNumberClass() ), value, encoding ) )
#define            DKNewVariableNumber( value, encoding )          DKNumberInit( DKAlloc( DKVariableNumberClass() ), value, encoding )


DK_API DKNumberRef DKNewNumberWithInt32( int32_t x );
DK_API DKNumberRef DKNewNumberWithInt64( int64_t x );
DK_API DKNumberRef DKNewNumberWithUInt32( uint32_t x );
DK_API DKNumberRef DKNewNumberWithUInt64( uint64_t x );
DK_API DKNumberRef DKNewNumberWithFloat( float x );
DK_API DKNumberRef DKNewNumberWithDouble( double x );
DK_API DKNumberRef DKNewNumberWithUUID( const DKUUID * uuid );     // Passing NULL will generate a new UUID
DK_API DKNumberRef DKNewNumberWithDate( const DKDateTime * date ); // Passing NULL will retrieve the current date+time

#define            DKNumberWithInt32( x )      DKAutorelease( DKNewNumberWithInt32( x ) )
#define            DKNumberWithInt64( x )      DKAutorelease( DKNewNumberWithInt64( x ) )
#define            DKNumberWithUInt32( x )     DKAutorelease( DKNewNumberWithUInt32( x ) )
#define            DKNumberWithUInt64( x )     DKAutorelease( DKNewNumberWithUInt64( x ) )
#define            DKNumberWithFloat( x )      DKAutorelease( DKNewNumberWithFloat( x ) )
#define            DKNumberWithDouble( x )     DKAutorelease( DKNewNumberWithDouble( x ) )
#define            DKNumberWithUUID( x )       DKAutorelease( DKNewNumberWithUUID( x ) )
#define            DKNumberWithDate( x )       DKAutorelease( DKNewNumberWithDate( x ) )

DK_API DKNumberRef DKNumberInit( DKNumberRef _self, const void * value, DKEncoding encoding );
DK_API DKNumberRef DKNumberInitWithNumber( DKNumberRef _self, DKNumberRef number, DKEncodingType encodingType );

DK_API DKEncoding  DKNumberGetEncoding( DKNumberRef _self );

DK_API size_t      DKNumberGetValue( DKNumberRef _self, void * value );
DK_API size_t      DKNumberSetValue( DKNumberRef _self, const void * value, DKEncoding encoding ); // Variable numbers only

DK_API const void* DKNumberGetValuePtr( DKNumberRef _self );
DK_API void *      DKNumberGetVariableValuePtr( DKNumberRef _self ); // Variable numbers only

DK_API size_t      DKNumberCastValue( DKNumberRef _self, void * value, DKEncoding encoding );
DK_API const void* DKNumberGetBytePtr( DKNumberRef _self, DKEncoding * encoding );

#define            DKNumberGetValueAs( _self, type )     (*((type *)DKNumberGetValuePtr( _self )))

DK_API bool        DKNumberGetBool( DKNumberRef _self );
DK_API int32_t     DKNumberGetInt32( DKNumberRef _self );
DK_API int64_t     DKNumberGetInt64( DKNumberRef _self );
DK_API float       DKNumberGetFloat( DKNumberRef _self );
DK_API double      DKNumberGetDouble( DKNumberRef _self );

#define            DKNumberGetString( _self )    DKNumberFormatString( _self, " " )
#define            DKNumberGetInt8( _self )      ((_self) ? *((int8_t *)DKNumberGetValuePtr( _self )) : 0)
#define            DKNumberGetInt16( _self )     ((_self) ? *((int16_t *)DKNumberGetValuePtr( _self )) : 0)
//#define          DKNumberGetInt32( _self )     ((_self) ? *((int32_t *)DKNumberGetValuePtr( _self )) : 0)
//#define          DKNumberGetInt64( _self )     ((_self) ? *((int64_t *)DKNumberGetValuePtr( _self )) : 0)
#define            DKNumberGetUInt8( _self )     ((_self) ? *((uint8_t *)DKNumberGetValuePtr( _self )) : 0)
#define            DKNumberGetUInt16( _self )    ((_self) ? *((uint16_t *)DKNumberGetValuePtr( _self )) : 0)
#define            DKNumberGetUInt32( _self )    ((_self) ? *((uint32_t *)DKNumberGetValuePtr( _self )) : 0)
#define            DKNumberGetUInt64( _self )    ((_self) ? *((uint64_t *)DKNumberGetValuePtr( _self )) : 0)
//#define          DKNumberGetFloat( _self )     ((_self) ? *((float *)DKNumberGetValuePtr( _self )) : 0)
//#define          DKNumberGetDouble( _self )    ((_self) ? *((double *)DKNumberGetValuePtr( _self )) : 0)
#define            DKNumberGetUUID( _self )      ((_self) ? *((DKUUID *)DKNumberGetValuePtr( _self )) : DKUUIDZero)
#define            DKNumberGetDate( _self )      ((_self) ? *((DKDateTime *)DKNumberGetValuePtr( _self )) : 0)

DK_API bool        DKNumberEqual( DKNumberRef a, DKNumberRef b );
DK_API int         DKNumberCompare( DKNumberRef a, DKNumberRef b );
DK_API DKHashCode  DKNumberHash( DKNumberRef _self );

DK_API DKStringRef DKNumberFormatString( DKNumberRef _self, const char * seperator );
DK_API DKStringRef DKNumberGetDescription( DKNumberRef _self );

// Utility function for converting number types
DK_API size_t      DKNumberConvert( const void * src, DKEncoding srcType, void * dst, DKEncoding dstType );


#endif // _DK_NUMBER_H_
