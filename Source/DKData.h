/*****************************************************************************************

  DKData.h

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

#ifndef _DK_DATA_H_
#define _DK_DATA_H_

#include "DKRuntime.h"


typedef struct DKData * DKDataRef;
typedef struct DKData * DKMutableDataRef;


DKClassRef  DKDataClass( void );
DKClassRef  DKMutableDataClass( void );

#define     DKEmptyData()                           DKAutorelease( DKNew( DKDataClass() ) )
#define     DKMutableData()                         DKAutorelease( DKNew( DKMutableDataClass() ) )

#define     DKDataWithBytes( bytes, length )        DKAutorelease( DKDataInitWithBytes( DKAlloc( DKDataClass() ), bytes, length ) )
#define     DKDataWithBytesNoCopy( bytes, length )  DKAutorelease( DKDataInitWithBytesNoCopy( DKAlloc( DKDataClass() ), bytes, length ) )
#define     DKDataWithContentsOfFile( filename )    DKAutorelease( DKDataInitWithContentsOfFile( DKAlloc( DKDataClass() ), filename ) )

#define     DKNewMutableData()                      DKNew( DKMutableDataClass() )

#define     DKMutableDataWithCapacity( length )     DKAutorelease( DKDataInitWithCapacity( DKAlloc( DKMutableDataClass() ), length ) )

DKDataRef   DKDataInitWithBytes( DKDataRef _self, const void * bytes, DKIndex length );
DKDataRef   DKDataInitWithBytesNoCopy( DKDataRef _self, const void * bytes, DKIndex length );
DKDataRef   DKDataInitWithLength( DKDataRef _self, DKIndex length );
DKDataRef   DKDataInitWithContentsOfFile( DKDataRef _self, DKStringRef filename );
DKMutableDataRef DKDataInitWithCapacity( DKMutableDataRef _self, DKIndex capacity );

DKDataRef DKDataMakeImmutable( DKMutableDataRef _self );

DKDataRef   DKDataCopy( DKDataRef _self );
DKMutableDataRef DKDataMutableCopy( DKDataRef _self );

DKStringRef DKDataGetDescription( DKDataRef _self );

bool        DKDataEqual( DKDataRef _self, DKObjectRef other );
int         DKDataCompare( DKDataRef _self, DKObjectRef other );
DKHashCode  DKDataHash( DKDataRef _self );

DKIndex     DKDataGetLength( DKDataRef _self );
void        DKDataSetLength( DKMutableDataRef _self, DKIndex length );
void        DKDataIncreaseLength( DKMutableDataRef _self, DKIndex length );

const void * DKDataGetBytePtr( DKDataRef _self, DKIndex index );
const void * DKDataGetByteRange( DKDataRef _self, DKRange range );
const void * DKDataGetByteEnd( DKDataRef _self );

void *      DKDataGetMutableBytePtr( DKMutableDataRef _self, DKIndex index );
void *      DKDataGetMutableByteRange( DKMutableDataRef _self, DKRange range );

#define     DKDataGetElementCount( data, type )             (DKDataGetLength( data ) / sizeof(type))
#define     DKDataGetElementPtr( data, type, index )        DKDataGetBytePtr( data, ((index) * sizeof(type)) )
#define     DKDataGetMutableElementPtr( data, type, index ) DKDataGetMutableBytePtr( data, ((index) * sizeof(type)) )

DKIndex     DKDataGetBytes( DKDataRef _self, DKRange range, void * buffer );

void        DKDataReplaceBytes( DKMutableDataRef _self, DKRange range, const void * bytes, DKIndex length );
void        DKDataAppendBytes( DKMutableDataRef _self, const void * bytes, DKIndex length );
void        DKDataDeleteBytes( DKMutableDataRef _self, DKRange range );

int         DKDataSeek( DKDataRef _self, DKIndex offset, int origin );
DKIndex     DKDataTell( DKDataRef _self );

DKIndex     DKDataRead( DKDataRef _self, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKDataWrite( DKMutableDataRef _self, const void * buffer, DKIndex size, DKIndex count );


#endif // _DK_DATA_H_


