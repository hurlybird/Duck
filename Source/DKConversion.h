/*****************************************************************************************

  DKConversion.h

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

#ifndef _DK_CONVERSION_H_
#define _DK_CONVERSION_H_

#include "DKRuntime.h"


DK_API DKDeclareInterfaceSelector( Conversion );


typedef DKStringRef (*DKGetStringMethod)( DKObjectRef _self );
typedef bool        (*DKGetBoolMethod)( DKObjectRef _self );
typedef int32_t     (*DKGetInt32Method)( DKObjectRef _self );
typedef int64_t     (*DKGetInt64Method)( DKObjectRef _self );
typedef float       (*DKGetFloatMethod)( DKObjectRef _self );
typedef double      (*DKGetDoubleMethod)( DKObjectRef _self );


struct DKConversionInterface
{
    const DKInterface _interface;
    
    DKGetStringMethod   getString;
    DKGetBoolMethod     getBool;
    DKGetInt32Method    getInt32;
    DKGetInt64Method    getInt64;
    DKGetFloatMethod    getFloat;
    DKGetDoubleMethod   getDouble;
};

typedef const struct DKConversionInterface * DKConversionInterfaceRef;


DK_API DKStringRef DKGetString( DKObjectRef _self );
DK_API bool        DKGetBool( DKObjectRef _self );
DK_API int32_t     DKGetInt32( DKObjectRef _self );
DK_API int64_t     DKGetInt64( DKObjectRef _self );
DK_API float       DKGetFloat( DKObjectRef _self );
DK_API double      DKGetDouble( DKObjectRef _self );


#define DKGetInt( _self )       DKGetInt32( _self )
#define DKGetLongLong( _self )  DKGetInt64( _self )



#endif // _DK_CONVERSION_H_



