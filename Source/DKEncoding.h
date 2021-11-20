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

#define DKMaxEncodingCount      0x00ffffff // 2^24
#define DKMaxEncodingRows       0x00000fff
#define DKMaxEncodingCols       0x00000fff

#define DKEncodingMatrixBit     0x80000000
#define DKEncodingTypeBits      0x7f000000
#define DKEncodingTypeShift     24
#define DKEncodingRowsShift     12


#define DKEncode( baseType, count ) \
    ((((baseType) << DKEncodingTypeShift) & DKEncodingTypeBits) | \
     ((count) & DKMaxEncodingCount))


#define DKEncodeMatrix( baseType, rows, cols ) \
    (DKEncodingMatrixBit | \
     (((baseType) << DKEncodingTypeShift) & DKEncodingTypeBits) | \
     (((rows) & DKMaxEncodingRows) << DKEncodingRowsShift) | \
     ((cols) & DKMaxEncodingCols))


// Macro for building integer encodings from built-in C types (i.e. enums)
#define DKEncodeIntegerType( ctype )            DKEncode( (DKEncodingTypeInt8 + sizeof(ctype) - 1), 1 )


// Base type tests
#define DKEncodingTypeIsValid( baseType )       (((baseType) >= 0) && ((baseType) < DKMaxEncodingTypes))
#define DKEncodingTypeIsObject( baseType )      (((baseType) >= DKEncodingTypeClass) && ((baseType) <= DKEncodingTypeKeyedObject))
#define DKEncodingTypeIsInteger( baseType )     (((baseType) >= DKEncodingTypeInt8) && ((baseType) <= DKEncodingTypeUInt64))
#define DKEncodingTypeIsSigned( baseType )      (((baseType) >= DKEncodingTypeInt8) && ((baseType) <= DKEncodingTypeInt64))
#define DKEncodingTypeIsUnsigned( baseType )    (((baseType) >= DKEncodingTypeUInt8) && ((baseType) <= DKEncodingTypeUInt64))
#define DKEncodingTypeIsReal( baseType )        (((baseType) >= DKEncodingTypeFloat) && ((baseType) <= DKEncodingTypeDouble))
#define DKEncodingTypeIsNumber( baseType )      (((baseType) >= DKEncodingTypeInt8) && ((baseType) <= DKEncodingTypeDouble))


#define DKEncodingIsMatrix( encoding )          (((encoding) & DKEncodingMatrixBit) != 0)


static inline DKEncodingType DKEncodingGetType( DKEncoding encoding )
{
    return (DKEncodingType)((encoding & DKEncodingTypeBits) >> DKEncodingTypeShift);
}

static inline unsigned int DKEncodingGetCount( DKEncoding encoding )
{
    if( encoding & DKEncodingMatrixBit )
    {
        unsigned int rows = (encoding >> DKEncodingRowsShift) & DKMaxEncodingRows;
        unsigned int cols = encoding & DKMaxEncodingCols;
        return rows * cols;
    }
    
    return (encoding & DKMaxEncodingCount);
}

static inline unsigned int DKEncodingGetRows( DKEncoding encoding )
{
    if( encoding & DKEncodingMatrixBit )
        return (encoding >> DKEncodingRowsShift) & DKMaxEncodingRows;
        
    return 1;
}

static inline unsigned int DKEncodingGetCols( DKEncoding encoding )
{
    if( encoding & DKEncodingMatrixBit )
        return encoding & DKMaxEncodingCols;
        
    return encoding & DKMaxEncodingCount;
}

static inline bool DKEncodingEqv( DKEncoding a, DKEncoding b )
{
    return (DKEncodingGetType( a ) == DKEncodingGetType( b )) &&
        (DKEncodingGetCount( a ) == DKEncodingGetCount( b ));
}


DK_API size_t DKEncodingTypeGetSize( DKEncodingType encodingType );
DK_API size_t DKEncodingGetSize( DKEncoding encoding );
DK_API size_t DKEncodingGetTypeSize( DKEncoding encoding );
DK_API const char * DKEncodingGetTypeName( DKEncoding encoding );
DK_API bool   DKEncodingIsNumber( DKEncoding encoding );
DK_API bool   DKEncodingIsInteger( DKEncoding encoding );
DK_API bool   DKEncodingIsReal( DKEncoding encoding );


#endif // _DK_ENCODING_H_


