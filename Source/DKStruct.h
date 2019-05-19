/*****************************************************************************************

  DKStruct.h

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

#ifndef _DK_STRUCT_H_
#define _DK_STRUCT_H_

#include "DKRuntime.h"


typedef struct DKStruct * DKStructRef;


DK_API DKClassRef  DKStructClass( void );

#define            DKSemantic( type )                      DKSTR( #type )

#define            DKStruct( semantic, bytes, size )       DKAutorelease( DKStructInit( DKAlloc( DKStructClass() ), semantic, bytes, size ) )
#define            DKStructWithType( ptr, type )           DKAutorelease( DKStructInit( DKAlloc( DKStructClass() ), DKSTR( #type ), ptr, sizeof(type) ) )

#define            DKNewStruct( semantic, bytes, size )    DKStructInit( DKAlloc( DKStructClass() ), semantic, bytes, size )
#define            DKNewStructWithType( ptr, type )        DKStructInit( DKAlloc( DKStructClass() ), DKSTR( #type ), ptr, sizeof(type) )

DK_API DKStructRef DKStructInit( DKStructRef _self, DKStringRef semantic, const void * bytes, size_t size );

DK_API bool        DKStructEqual( DKStructRef _self, DKStructRef other );
DK_API int         DKStructCompare( DKStructRef _self, DKStructRef other );
DK_API DKHashCode  DKStructHash( DKStructRef _self );

DK_API DKStringRef DKStructGetSemantic( DKStructRef _self );
DK_API size_t      DKStructGetSize( DKStructRef _self );
DK_API size_t      DKStructGetValue( DKStructRef _self, DKStringRef semantic, void * bytes, size_t size );

#define            DKStructGetValueAsType( st, dst, type ) DKStructGetValue( st, DKSTR( #type ), dst, sizeof(type) );


#endif
