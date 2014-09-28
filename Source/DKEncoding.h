/*****************************************************************************************

  DKEncoding.h

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

#ifndef _DK_ENCODING_H_
#define _DK_ENCODING_H_

#include "DKPlatform.h"


#define DKEncodingVersion           1

typedef enum
{
    DKEncodingNull = 0,

    // Special Types
    DKEncodingTypeClass,
    DKEncodingTypeSelector,

    // Object Types
    DKEncodingTypeObject,
    DKEncodingTypeKeyedObject,

    // Data Types
    DKEncodingTypeTextData,
    DKEncodingTypeBinaryData,

    // Number Types
    DKEncodingTypeInt8,
    DKEncodingTypeInt16,
    DKEncodingTypeInt32,
    DKEncodingTypeInt64,
    
    DKEncodingTypeUInt8,
    DKEncodingTypeUInt16,
    DKEncodingTypeUInt32,
    DKEncodingTypeUInt64,
    
    DKEncodingTypeFloat,
    DKEncodingTypeDouble,
    
    DKMaxEncodingTypes
    
} DKEncodingType;


typedef uint32_t DKEncoding;

#define DKMaxEncodingSize               (16 * 1024 * 1024) // 2^24

#define DKEncode( type, count )         (((type) << 24) | (count))
#define DKEncodingGetType( encoding )   ((encoding) >> 24)
#define DKEncodingGetCount( encoding )  ((encoding) & 0x00ffffff)


size_t DKEncodingGetSize( DKEncoding encoding );
size_t DKEncodingGetTypeSize( DKEncoding encoding );
const char * DKEncodingGetTypeName( DKEncoding encoding );
bool   DKEncodingIsNumber( DKEncoding encoding );
bool   DKEncodingIsInteger( DKEncoding encoding );
bool   DKEncodingIsReal( DKEncoding encoding );


#endif // _DK_ENCODING_H_


