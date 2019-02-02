/*****************************************************************************************

  DKEnum.h

  Copyright (c) 2017 Derek W. Nylen

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

#ifndef _DK_ENUM_H_
#define _DK_ENUM_H_

#include "DKRuntime.h"


// typedef struct DKEnum * DKEnumRef; -- Declared in DKPlatform.h

DKClassRef DKEnumClass( void );

DKObjectRef DKEnumInitWithCStringsAndValues( DKObjectRef _self, ... );

#define DKDefineEnum( accessor, ... )                                                   \
    DKThreadSafeSharedObjectInit( accessor, DKEnumRef )                                 \
    {                                                                                   \
        return DKEnumInitWithCStringsAndValues( DKAlloc( DKEnumClass() ),               \
            __VA_ARGS__,                                                                \
            NULL );                                                                     \
    }

#define DKEnumFromString( _self, str )  (int)DKEnumFromString64( (_self), (str) )
#define DKStringFromEnum( _self, val )  DKStringFromEnum64( (_self), (val) )

int64_t DKEnumFromString64( DKEnumRef _self, DKStringRef str );
DKStringRef DKStringFromEnum64( DKEnumRef _self, int64_t value );



#endif // _DK_ENUM_H_


