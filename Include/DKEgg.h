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
#include "DKStream.h"


typedef struct DKEggReader * DKEggReaderRef;
typedef struct DKEggArchiver * DKEggArchiverRef;


// Egg Interface - Adopted by any object that supports egg storage =======================
DKDeclareInterfaceSelector( Egg );

typedef DKObjectRef (*DKCreateFromEggMethod)( DKClassRef _class, DKEggReaderRef egg );
typedef void        (*DKAddToEggMethod)( DKObjectRef _self, DKEggArchiverRef egg );

struct DKEggInterface
{
    const DKInterface _interface;

    DKCreateFromEggMethod   createFromEgg;
    DKAddToEggMethod        addToEgg;
};

typedef const struct DKEggInterface * DKEggInterfaceRef;




// DKEggReader ===========================================================================
DKClassRef DKEggReaderClass( void );

DKEggReaderRef DKEggCreateReader( DKDataRef data );

int32_t DKEggGetTypeOfKey( DKEggReaderRef _self, DKStringRef key );
size_t  DKEggGetLengthOfKey( DKEggReaderRef _self, DKStringRef key );

DKObjectRef DKEggReadObject( DKEggReaderRef _self, DKStringRef key );

void    DKEggReadCollection( DKEggReaderRef _self, DKStringRef key, DKApplierFunction callback, void * context );
void    DKEggReadKeyedCollection( DKEggReaderRef _self, DKStringRef key, DKKeyedApplierFunction callback, void * context );

size_t  DKEggReadData( DKEggReaderRef _self, DKStringRef key, void * bytes, size_t elementSize, size_t elementCount );
size_t  DKEggGetDataPtr( DKEggReaderRef _self, DKStringRef key, const void ** bytes );




// DKEggArchiver =========================================================================
DKClassRef DKEggArchiverClass( void );

DKEggArchiverRef DKEggCreateArchiver( int options );

void DKEggArchiverWriteToStream( DKEggArchiverRef _self, DKMutableObjectRef stream );
DKDataRef DKEggArchiverCreateData( DKEggArchiverRef _self );

void DKEggAddObject( DKEggArchiverRef _self, DKStringRef key, DKObjectRef object );
void DKEggAddCollection( DKEggArchiverRef _self, DKStringRef key, DKObjectRef collection );
void DKEggAddKeyedCollection( DKEggArchiverRef _self, DKStringRef key, DKObjectRef collection );

void DKEggAddTextData( DKEggArchiverRef _self, DKStringRef key, const char * text, size_t length );
void DKEggAddBinaryData( DKEggArchiverRef _self, DKStringRef key, const void * bytes, size_t length );
void DKEggAddNumber( DKEggArchiverRef _self, DKStringRef key, DKEncoding encoding, const void * number );


#endif // _DK_EGG_H_





