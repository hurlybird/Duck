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


typedef const struct DKStruct * DKStructRef;


DKClassRef  DKStructClass( void );

DKStructRef DKStructCreate( DKStringRef semantic, const void * bytes, size_t size );

DKStringRef DKStructGetSemantic( DKStructRef _self );
size_t      DKStructGetValue( DKStructRef _self, DKStringRef semantic, void * bytes, size_t size );

// Macros for creating/retrieving C structs
#define DKSemantic( type )                  DKSTR( #type )

#define DKStructCreateAs( ptr, type )       DKStructCreate( DKSTR( #type ), ptr, sizeof(type) );
#define DKStructGetValueAs( st, dst, type ) DKStructGetValue( st, DKSTR( #type ), dst, sizeof(type) );


#endif