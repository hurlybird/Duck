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
    DKEncodingNull =        0,

    // Special Types
    DKEncodingTypeClass,
    DKEncodingTypeSelector,

    // Object Types
    DKEncodingTypeObject,
    DKEncodingTypeKeyedObject,

    // Data Types
    DKEncodingTypeTextData,
    DKEncodingTypeBinaryData,

    // Reserved
    __DKEncodingTypeReserved,

    // Integers
    DKEncodingTypeInt8 =    8,
    DKEncodingTypeInt16,
    
    __DKEncodingTypeInvalid3ByteInt,
    
    DKEncodingTypeInt32,

    __DKEncodingTypeInvalid5ByteInt,
    __DKEncodingTypeInvalid6ByteInt,
    __DKEncodingTypeInvalid7ByteInt,

    DKEncodingTypeInt64,
    
    // Unsigned Integers
    DKEncodingTypeUInt8,
    DKEncodingTypeUInt16,

    __DKEncodingTypeInvalid3ByteUInt,

    DKEncodingTypeUInt32,

    __DKEncodingTypeInvalid5ByteUInt,
    __DKEncodingTypeInvalid6ByteUInt,
    __DKEncodingTypeInvalid7ByteUInt,

    DKEncodingTypeUInt64,
    
    // Floats
    DKEncodingTypeFloat,
    DKEncodingTypeDouble,
    
    DKMaxEncodingTypes
    
} DKEncodingType;


typedef uint32_t DKEncoding;

#define DKMaxEncodingCount                      (0x00ffffff) // 2^24

#define DKEncode( baseType, count )             (((baseType) << 24) | (count))
#define DKEncodingGetType( encoding )           ((encoding) >> 24)
#define DKEncodingGetCount( encoding )          ((encoding) & 0x00ffffff)

// Macro for building integer encodings from built-in C types (i.e. enums)
#define DKEncodingTypeInt( ctype )              DKEncode( (DKEncodingTypeInt8 + sizeof(ctype) - 1), 1 )

// Base type tests
#define DKEncodingTypeIsValid( baseType )       (((baseType) >= 0) && ((baseType) < DKMaxEncodingTypes))
#define DKEncodingTypeIsObject( baseType )      (((baseType) >= DKEncodingTypeClass) && ((baseType) <= DKEncodingTypeKeyedObject))
#define DKEncodingTypeIsInteger( baseType )     (((baseType) >= DKEncodingTypeInt8) && ((baseType) <= DKEncodingTypeUInt64))
#define DKEncodingTypeIsSigned( baseType )      (((baseType) >= DKEncodingTypeInt8) && ((baseType) <= DKEncodingTypeInt64))
#define DKEncodingTypeIsUnsigned( baseType )    (((baseType) >= DKEncodingTypeUInt8) && ((baseType) <= DKEncodingTypeUInt64))
#define DKEncodingTypeIsReal( baseType )        (((baseType) >= DKEncodingTypeFloat) && ((baseType) <= DKEncodingTypeDouble))
#define DKEncodingTypeIsNumber( baseType )      (((baseType) >= DKEncodingTypeInt8) && ((baseType) <= DKEncodingTypeDouble))

size_t DKEncodingGetSize( DKEncoding encoding );
size_t DKEncodingGetTypeSize( DKEncoding encoding );
const char * DKEncodingGetTypeName( DKEncoding encoding );
bool   DKEncodingIsNumber( DKEncoding encoding );
bool   DKEncodingIsInteger( DKEncoding encoding );
bool   DKEncodingIsReal( DKEncoding encoding );


#endif // _DK_ENCODING_H_


