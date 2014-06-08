/*****************************************************************************************

  DKEgg.c

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

#include "DKEgg.h"
#include "DKString.h"
#include "DKHashTable.h"


DKThreadSafeSelectorInit( Egg );




// Egg File ==============================================================================

#define EGG_PREFIX          "EGG\n"
#define EGG_BYTE_ORDER      ((((uint32_t)'1') << 24) | (((uint32_t)'2') << 16) | (((uint32_t)'3') << 8) | ((uint32_t)'4'))
#define EGG_VERSION         1


// EggHeader
// EggObjectTableRow * header.objectTable.length
// String Data
// Binary Data

typedef struct
{
    uint32_t offset;
    uint32_t length;

} EggRange;

typedef struct
{
    int8_t prefix[4];
    uint32_t byteOrder;
    uint32_t version;

    EggRange objectTable;
    EggRange stringData;
    EggRange binaryData;

} EggHeader;

typedef struct
{
    uint32_t className;
    DKUUID   uuid;
    EggRange keyTable;

} ObjectTableRow;

typedef struct
{
    uint32_t key;
    uint32_t dataType;
    EggRange data;

} KeyTableRow;




// DKEggReader ===========================================================================

struct DKEggReader
{
    const DKObject _obj;
};


static DKObjectRef DKEggReaderInitialize( DKObjectRef _self );
static void DKEggReaderFinalize( DKObjectRef _self );


///
//  DKEggReaderClass()
//
DKThreadSafeClassInit( DKEggReaderClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKEggReader" ), DKObjectClass(), sizeof(struct DKEggReader), 0 );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->initialize = DKEggReaderInitialize;
    allocation->finalize = DKEggReaderFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    return cls;
}


///
//  DKEggReaderInitialize()
//
static DKObjectRef DKEggReaderInitialize( DKObjectRef _self )
{
    return _self;
}


///
//  DKEggReaderFinalize()
//
static void DKEggReaderFinalize( DKObjectRef _self )
{
}






// DKEggWriter ===========================================================================

struct DKEggWriter
{
    const DKObject _obj;
    
    DKByteOrder byteOrder;
    
    DKMutableHashTableRef objectTable;
};


static DKObjectRef DKEggWriterInitialize( DKObjectRef _self );
static void DKEggWriterFinalize( DKObjectRef _self );


///
//  DKEggReaderClass()
//
DKThreadSafeClassInit( DKEggWriterClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKEggWriter" ), DKObjectClass(), sizeof(struct DKEggWriter), 0 );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->initialize = DKEggWriterInitialize;
    allocation->finalize = DKEggWriterFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    return cls;
}


///
//  DKEggWriterInitialize()
//
static DKObjectRef DKEggWriterInitialize( DKObjectRef _self )
{
    if( _self )
    {
        DKEggWriterRef egg = (DKEggWriterRef)_self;
    
        egg->byteOrder = DKByteOrderNative;
        egg->objectTable = DKHashTableCreateWithHashFunction( DKPointerHash, DKPointerEqual );
    }

    return _self;
}


///
//  DKEggWriterFinalize()
//
static void DKEggWriterFinalize( DKObjectRef _self )
{
    DKEggWriterRef egg = (DKEggWriterRef)_self;
    
    DKRelease( egg->objectTable );
}


///
//  DKEggCreateWriter()
//
DKEggWriterRef DKEggCreateWriter( DKByteOrder byteOrder )
{
    DKEggWriterRef egg = DKCreate( DKEggWriterClass() );
    
    if( egg )
    {
        egg->byteOrder = byteOrder;
    }
    
    return egg;
}


///
//  DKEggCompile()
//
DKDataRef DKEggCompile( DKEggWriterRef _self )
{
    return NULL;
}


///
//  DKEggWriteObject()
//
void DKEggWriteObject( DKEggWriterRef _self, DKStringRef key, DKObjectRef object )
{
    if( (_self == NULL) || (object == NULL) )
        return;

    // Have we already seen this object?
    if( !DKHashTableGetObject( _self->objectTable, object ) )
    {
        // Make sure we can serialize the object
        DKEggInterfaceRef eggInterface;
        
        if( !DKQueryInterface( object, DKSelector(Egg), (const void **)&eggInterface ) )
        {
            DKError( "DKEggWriteObject: %s does not implement .egg file archiving.\n",
                DKStringGetCStringPtr( DKGetClassName( object ) ) );
            return;
        }
    
        // Add the object to our table
        DKHashTableInsertObject( _self->objectTable, object, object, DKInsertAlways );
        
        // Push a new context for the object
        
        // Archive the object
        eggInterface->writeToEgg( object, _self );
        
        // Pop the object context
    }
    
    // Add the object to the current context
}


///
//  DKEggWriteCollection()
//
void DKEggWriteCollection( DKEggWriterRef _self, DKStringRef key, DKObjectRef collection )
{
}


///
//  DKEggWriteKeyedCollection()
//
void DKEggWriteKeyedCollection( DKEggWriterRef _self, DKStringRef key, DKObjectRef collection )
{
}


///
//  DKEggWriteNumber()
//
void DKEggWriteNumber( DKEggWriterRef _self, DKStringRef key, const void * src, DKNumberType srcType, size_t count )
{
}


///
//  DKEggWriteBytes()
//
void DKEggWriteBytes( DKEggWriterRef _self, DKStringRef key, const void * bytes, size_t length )
{
}











