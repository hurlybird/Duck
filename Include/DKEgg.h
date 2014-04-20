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

typedef enum
{
    DKEggObject = 1,
    
    DKEggCollection,
    DKEggKeyedCollection,
    
    DKEggInt8,
    DKEggInt16,
    DKEggInt32,
    DKEggInt64,
    DKEggFloat,
    DKEggDouble

} DKEggType;




// DKEggReader ===========================================================================
DKClassRef DKEggReaderClass( void );

DKEggReaderRef DKEggReaderCreate( DKDataRef data );

DKEggType   DKEggGetTypeOfKey( DKEggReaderRef _self, DKStringRef key );
size_t      DKEggGetLengthOfKey( DKEggReaderRef _self, DKStringRef key );

DKObjectRef DKEggReadObject( DKEggReaderRef _self, DKStringRef key );

void    DKEggReadCollection( DKEggReaderRef _self, DKStringRef key, DKApplierFunction callback, void * context );
void    DKEggReadKeyedCollection( DKEggReaderRef _self, DKStringRef key, DKKeyedApplierFunction callback, void * context );

int8_t  DKEggReadInt8( DKEggReaderRef _self, DKStringRef key );
int16_t DKEggReadInt16( DKEggReaderRef _self, DKStringRef key );
int32_t DKEggReadInt32( DKEggReaderRef _self, DKStringRef key );
int64_t DKEggReadInt64( DKEggReaderRef _self, DKStringRef key );
float   DKEggReadFloat( DKEggReaderRef _self, DKStringRef key );
double  DKEggReadDouble( DKEggReaderRef _self, DKStringRef key );

size_t  DKEggReadIntArray8( DKEggReaderRef _self, DKStringRef key, int8_t buffer[], size_t length );
size_t  DKEggReadIntArray16( DKEggReaderRef _self, DKStringRef key, int16_t buffer[], size_t length );
size_t  DKEggReadIntArray32( DKEggReaderRef _self, DKStringRef key, int32_t buffer[], size_t length );
size_t  DKEggReadIntArray64( DKEggReaderRef _self, DKStringRef key, int64_t buffer[], size_t length );
size_t  DKEggReadFloatArray( DKEggReaderRef _self, DKStringRef key, float buffer[], size_t length );
size_t  DKEggReadDoubleArray( DKEggReaderRef _self, DKStringRef key, double buffer[], size_t length );



// DKEggWriter ===========================================================================
DKClassRef DKEggWriterClass( void );

DKEggWriterRef DKEggWriterCreate( DKByteOrder byteOrder );

void DKEggWriteObject( DKEggWriterRef _self, DKStringRef key, DKObjectRef object );

void DKEggWriteCollection( DKEggWriterRef _self, DKStringRef key, DKObjectRef collection );
void DKEggWriteKeyedCollection( DKEggWriterRef _self, DKStringRef key, DKObjectRef collection );

void DKEggWriteInt8( DKEggWriterRef _self, DKStringRef key, int8_t x );
void DKEggWriteInt16( DKEggWriterRef _self, DKStringRef key, int16_t x );
void DKEggWriteInt32( DKEggWriterRef _self, DKStringRef key, int32_t x );
void DKEggWriteInt64( DKEggWriterRef _self, DKStringRef key, int64_t x );
void DKEggWriteFloat( DKEggWriterRef _self, DKStringRef key, float x );
void DKEggWriteDouble( DKEggWriterRef _self, DKStringRef key, double x );

void DKEggWriteIntArray8( DKEggWriterRef _self, DKStringRef key, const int8_t buffer[], size_t length );
void DKEggWriteIntArray16( DKEggWriterRef _self, DKStringRef key, const int16_t buffer[], size_t length );
void DKEggWriteIntArray32( DKEggWriterRef _self, DKStringRef key, const int32_t buffer[], size_t length );
void DKEggWriteIntArray64( DKEggWriterRef _self, DKStringRef key, const int64_t buffer[], size_t length );
void DKEggWriteFloatArray( DKEggWriterRef _self, DKStringRef key, const float buffer[], size_t length );
void DKEggWriteDoubleArray( DKEggWriterRef _self, DKStringRef key, const double buffer[], size_t length );



#endif // _DK_EGG_H_





