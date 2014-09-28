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


typedef const struct DKData * DKDataRef;
typedef struct DKData * DKMutableDataRef;


DKClassRef  DKDataClass( void );
DKClassRef  DKMutableDataClass( void );

#define     DKDataCreateEmpty()    DKCreate( DKDataClass() )
DKMutableDataRef DKDataCreateMutable( DKIndex reserveLength );

DKDataRef   DKDataCreateWithBytes( DKClassRef _class, const void * bytes, DKIndex length );
DKDataRef   DKDataCreateWithBytesNoCopy( /* DKClassRef _class, */ const void * bytes, DKIndex length );

DKDataRef   DKDataCopy( DKDataRef _self );
DKMutableDataRef DKDataMutableCopy( DKDataRef _self );

bool        DKDataEqual( DKDataRef _self, DKObjectRef other );
int         DKDataCompare( DKDataRef _self, DKDataRef other );
DKHashCode  DKDataHash( DKDataRef _self );

DKIndex     DKDataGetLength( DKDataRef _self );
void        DKDataSetLength( DKMutableDataRef _self, DKIndex length );
void        DKDataIncreaseLength( DKMutableDataRef _self, DKIndex length );

const void * DKDataGetBytePtr( DKDataRef _self, DKIndex index );
const void * DKDataGetByteRange( DKDataRef _self, DKRange range );

void *      DKDataGetMutableBytePtr( DKMutableDataRef _self, DKIndex index );
void *      DKDataGetMutableByteRange( DKMutableDataRef _self, DKRange range );

DKIndex     DKDataGetBytes( DKDataRef _self, DKRange range, void * buffer );

void        DKDataReplaceBytes( DKMutableDataRef _self, DKRange range, const void * bytes, DKIndex length );
void        DKDataAppendBytes( DKMutableDataRef _self, const void * bytes, DKIndex length );
void        DKDataDeleteBytes( DKMutableDataRef _self, DKRange range );

int         DKDataSeek( DKDataRef _self, DKIndex offset, int origin );
DKIndex     DKDataTell( DKDataRef _self );

DKIndex     DKDataRead( DKDataRef _self, void * buffer, DKIndex size, DKIndex count );
DKIndex     DKDataWrite( DKMutableDataRef _self, const void * buffer, DKIndex size, DKIndex count );


#endif // _DK_DATA_H_


