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

#ifdef __cplusplus
extern "C"
{
#endif


//typedef struct DKData * DKDataRef; -- Declared in DKPlatform.h
typedef struct DKData * DKMutableDataRef;


DK_API DKClassRef  DKDataClass( void );
DK_API DKClassRef  DKMutableDataClass( void );

#define            DKEmptyData()                           DKAutorelease( DKNew( DKDataClass() ) )
#define            DKMutableData()                         DKAutorelease( DKNew( DKMutableDataClass() ) )

#define            DKDataWithBytes( bytes, length )        DKAutorelease( DKDataInitWithBytes( DKAlloc( DKDataClass() ), bytes, length ) )
#define            DKDataWithBytesNoCopy( bytes, length )  DKAutorelease( DKDataInitWithBytesNoCopy( DKAlloc( DKDataClass() ), bytes, length ) )
#define            DKDataWithContentsOfFile( filename )    DKAutorelease( DKDataInitWithContentsOfFile( DKAlloc( DKDataClass() ), filename ) )

#define            DKNewMutableData()                      DKNew( DKMutableDataClass() )
#define            DKNewMutableDataWithCapacity( length )  DKDataInitWithCapacity( DKAlloc( DKMutableDataClass() ), length )

#define            DKMutableDataWithBytes( bytes, length ) DKAutorelease( DKDataInitWithBytes( DKAlloc( DKMutableDataClass() ), bytes, length ) )
#define            DKMutableDataWithCapacity( length )     DKAutorelease( DKDataInitWithCapacity( DKAlloc( DKMutableDataClass() ), length ) )

DK_API DKDataRef   DKDataInitWithBytes( DKObjectRef _self, const void * bytes, DKIndex length );
DK_API DKDataRef   DKDataInitWithBytesNoCopy( DKObjectRef _self, const void * bytes, DKIndex length );
DK_API DKDataRef   DKDataInitWithLength( DKObjectRef _self, DKIndex length );
DK_API DKDataRef   DKDataInitWithContentsOfFile( DKObjectRef _self, DKObjectRef file );
DK_API DKMutableDataRef DKDataInitWithCapacity( DKObjectRef _self, DKIndex capacity );

DK_API DKDataRef   DKDataMakeImmutable( DKMutableDataRef _self );

DK_API DKDataRef   DKDataCopy( DKDataRef _self );
DK_API DKMutableDataRef DKDataMutableCopy( DKDataRef _self );

DK_API DKStringRef DKDataGetDescription( DKDataRef _self );

DK_API bool        DKDataEqual( DKDataRef _self, DKObjectRef other );
DK_API int         DKDataCompare( DKDataRef _self, DKObjectRef other );
DK_API DKHashCode  DKDataHash( DKDataRef _self );

DK_API DKIndex     DKDataGetLength( DKDataRef _self );
DK_API void        DKDataSetLength( DKMutableDataRef _self, DKIndex length );
DK_API void        DKDataIncreaseLength( DKMutableDataRef _self, DKIndex length );

DK_API DKEncodingType DKDataGetEncodingType( DKDataRef _self );
DK_API void DKDataSetEncodingType( DKDataRef _self, DKEncodingType type );

DK_API const void * DKDataGetBytePtr( DKDataRef _self, DKIndex index );
DK_API const void * DKDataGetByteRange( DKDataRef _self, DKRange range );
DK_API const void * DKDataGetByteEnd( DKDataRef _self );

DK_API void *      DKDataGetMutableBytePtr( DKMutableDataRef _self, DKIndex index );
DK_API void *      DKDataGetMutableByteRange( DKMutableDataRef _self, DKRange range );

#define            DKDataGetElementCount( data, type )             (DKDataGetLength( data ) / sizeof(type))
#define            DKDataGetElementPtr( data, type, index )        DKDataGetBytePtr( data, ((index) * sizeof(type)) )
#define            DKDataGetMutableElementPtr( data, type, index ) DKDataGetMutableBytePtr( data, ((index) * sizeof(type)) )

DK_API DKIndex     DKDataGetBytes( DKDataRef _self, DKRange range, void * buffer );

DK_API void        DKDataAppendData( DKMutableDataRef _self, DKDataRef data );

DK_API void        DKDataReplaceBytes( DKMutableDataRef _self, DKRange range, const void * bytes, DKIndex length );
DK_API void        DKDataAppendBytes( DKMutableDataRef _self, const void * bytes, DKIndex length );
DK_API void        DKDataDeleteBytes( DKMutableDataRef _self, DKRange range );

DK_API int         DKDataSeek( DKDataRef _self, long offset, int origin );
DK_API long        DKDataTell( DKDataRef _self );

DK_API size_t      DKDataRead( DKDataRef _self, void * buffer, size_t size, size_t count );
DK_API size_t      DKDataWrite( DKMutableDataRef _self, const void * buffer, size_t size, size_t count );


#ifdef __cplusplus
}
#endif

#endif // _DK_DATA_H_


