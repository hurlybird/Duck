/*****************************************************************************************

  DKEgg.h

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

#ifndef _DK_EGG_H_
#define _DK_EGG_H_

#include "DKRuntime.h"
#include "DKData.h"
#include "DKCollection.h"
#include "DKNumber.h"


typedef struct DKEggReader * DKEggReaderRef;
typedef struct DKEggWriter * DKEggWriterRef;


// Egg Interface - Adopted by any object that supports egg storage =======================
DKDeclareInterfaceSelector( Egg );

typedef DKObjectRef (*DKCreateFromEggMethod)( DKClassRef _class, DKEggReaderRef egg );
typedef void        (*DKWriteToEggMethod)( DKObjectRef _self, DKEggWriterRef egg );

struct DKEggInterface
{
    const DKInterface _interface;

    DKCreateFromEggMethod   createFromEgg;
    DKWriteToEggMethod      writeToEgg;
};

typedef const struct DKEggInterface * DKEggInterfaceRef;




// Egg Storage Types =====================================================================

enum
{
    DKEggObject = 1,
    DKEggCollection,
    DKEggKeyedCollection,
    
    // Numerical Types are defined in DKNumber.h
};




// DKEggReader ===========================================================================
DKClassRef DKEggReaderClass( void );

DKEggReaderRef DKEggCreateReader( DKDataRef data );

int32_t DKEggGetTypeOfKey( DKEggReaderRef _self, DKStringRef key );
size_t  DKEggGetLengthOfKey( DKEggReaderRef _self, DKStringRef key );

DKObjectRef DKEggReadObject( DKEggReaderRef _self, DKStringRef key );

void    DKEggReadCollection( DKEggReaderRef _self, DKStringRef key, DKApplierFunction callback, void * context );
void    DKEggReadKeyedCollection( DKEggReaderRef _self, DKStringRef key, DKKeyedApplierFunction callback, void * context );

size_t  DKEggReadNumber( DKEggReaderRef _self, DKStringRef key, void * number, DKNumberType numberType, size_t count );
size_t  DKEggReadBytes( DKEggWriterRef _self, DKStringRef key, void * bytes, uint32_t length );



// DKEggWriter ===========================================================================
DKClassRef DKEggWriterClass( void );

DKEggWriterRef DKEggCreateWriter( DKByteOrder byteOrder );

DKDataRef DKEggCompile( DKEggWriterRef _self );

void DKEggWriteObject( DKEggWriterRef _self, DKStringRef key, DKObjectRef object );

void DKEggWriteCollection( DKEggWriterRef _self, DKStringRef key, DKObjectRef collection );
void DKEggWriteKeyedCollection( DKEggWriterRef _self, DKStringRef key, DKObjectRef collection );

void DKEggWriteNumber( DKEggWriterRef _self, DKStringRef key, const void * src, DKNumberType srcType, size_t count );
void DKEggWriteBytes( DKEggWriterRef _self, DKStringRef key, const void * bytes, size_t length );


#endif // _DK_EGG_H_





