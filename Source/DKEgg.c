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
#include "DKByteArray.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"


DKThreadSafeSelectorInit( Egg );




// Egg File ==============================================================================

#define DKEggBinaryPrefix    "binegg\n" // 8 bytes including the '\0'
#define DKEggVersion         1


// EggHeader
// EggObject * header.objectTable.length
// EggAttribute * header.attributeTable.length
// Data

typedef struct
{
    uint32_t index;
    uint32_t length;

} DKEggRange;

typedef struct
{
    char prefix[8];
    
    uint8_t version;
    uint8_t encodingVersion;
    uint8_t byteOrder;
    uint8_t pad;

    DKEggRange objectTable;
    DKEggRange attributeTable;
    DKEggRange data;

} DKEggHeader;

typedef struct
{
    uint32_t className;
    DKEggRange attributes;

} DKEggObject;

typedef struct
{
    uint32_t key;
    uint32_t encoding;
    uint32_t value;

} DKEggAttribute;

typedef struct
{
    uint32_t key;
    uint32_t value;
    
} DKEggKVPair;




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






// DKEggArchiver =========================================================================

struct DKEggArchiver
{
    const DKObject _obj;
    
    DKByteOrder byteOrder;
    
    DKGenericArray stack;
    DKGenericArray archivedObjects;
    DKGenericHashTable visitedObjects;
    DKByteArray data;
};

struct ArchivedObject
{
    DKObjectRef object;
    uint32_t className;
    DKGenericArray attributes;
};

struct VisitedObjectsRow
{
    DKObjectRef object;
    DKIndex index;
};


// Visited List --------------------------------------------------------------------------

#define DELETED_VISTED_LIST_ROW ((void *)-1)

static DKRowStatus VisitedObjectsRowStatus( const void * _row )
{
    const struct VisitedObjectsRow * row = _row;

    if( row->object == NULL )
        return DKRowStatusEmpty;
    
    if( row->object == DELETED_VISTED_LIST_ROW )
        return DKRowStatusDeleted;
    
    return DKRowStatusActive;
}

static DKHashCode VisitedObjectsRowHash( const void * _row )
{
    const struct VisitedObjectsRow * row = _row;
    return DKPointerHash( row->object );
}

static bool VisitedObjectsRowEqual( const void * _row1, const void * _row2 )
{
    const struct VisitedObjectsRow * row1 = _row1;
    const struct VisitedObjectsRow * row2 = _row2;

    return DKPointerEqual( row1->object, row2->object );
}

static void VisitedObjectsRowInit( void * _row )
{
    struct VisitedObjectsRow * row = _row;
    
    row->object = NULL;
    row->index = 0;
}

static void VisitedObjectsRowUpdate( void * _row, const void * _src )
{
    struct VisitedObjectsRow * row = _row;
    const struct VisitedObjectsRow * src = _src;
    
    row->object = src->object;
    row->index = src->index;
}

static void VisitedObjectsRowDelete( void * _row )
{
    struct VisitedObjectsRow * row = _row;

    row->object = DELETED_VISTED_LIST_ROW;
}




// Methods -------------------------------------------------------------------------------

static DKObjectRef DKEggArchiverInitialize( DKObjectRef _self );
static void DKEggArchiverFinalize( DKObjectRef _self );

static DKIndex AddObject( DKEggArchiverRef _self, DKObjectRef object );

///
//  DKEggArchiverClass()
//
DKThreadSafeClassInit( DKEggArchiverClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKEggArchiver" ), DKObjectClass(), sizeof(struct DKEggArchiver), 0 );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->initialize = DKEggArchiverInitialize;
    allocation->finalize = DKEggArchiverFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    return cls;
}


///
//  DKEggArchiverInitialize()
//
static DKObjectRef DKEggArchiverInitialize( DKObjectRef _self )
{
    if( _self )
    {
        DKEggArchiverRef egg = (DKEggArchiverRef)_self;
    
        egg->byteOrder = DKByteOrderNative;
        
        DKGenericHashTableCallbacks callbacks =
        {
            VisitedObjectsRowStatus,
            VisitedObjectsRowHash,
            VisitedObjectsRowEqual,
            VisitedObjectsRowInit,
            VisitedObjectsRowUpdate,
            VisitedObjectsRowDelete
        };

        DKGenericArrayInit( &egg->stack, sizeof(DKIndex) );
        DKGenericArrayInit( &egg->archivedObjects, sizeof(struct ArchivedObject) );
        DKGenericHashTableInit( &egg->visitedObjects, sizeof(struct VisitedObjectsRow), &callbacks );
        DKByteArrayInit( &egg->data );

        // Push an anonymous root object
        struct ArchivedObject rootObject;
        rootObject.object = NULL;
        rootObject.className = 0;
        DKGenericArrayInit( &rootObject.attributes, sizeof(DKEggAttribute) );
        
        DKGenericArrayAppendElements( &egg->archivedObjects, &rootObject, 1 );

        DKGenericArrayAppendElements( &egg->stack, &(DKIndex){ 0 }, 1 );
    }

    return _self;
}


///
//  DKEggArchiverFinalize()
//
static void DKEggArchiverFinalize( DKObjectRef _self )
{
    DKEggArchiverRef egg = (DKEggArchiverRef)_self;
    
    DKAssert( DKGenericArrayGetLength( &egg->stack ) == 1 );
    
    DKIndex archivedObjectCount = DKGenericArrayGetLength( &egg->archivedObjects );
    
    for( DKIndex i = 0; i < archivedObjectCount; ++i )
    {
        struct ArchivedObject * archivedObject = DKGenericArrayGetPointerToElementAtIndex( &egg->archivedObjects, i );

        DKRelease( archivedObject->object );
        DKGenericArrayFinalize( &archivedObject->attributes );
    }
    
    DKGenericArrayFinalize( &egg->stack );
    DKGenericArrayFinalize( &egg->archivedObjects );
    DKGenericHashTableFinalize( &egg->visitedObjects );
    DKByteArrayFinalize( &egg->data );
}


///
//  DKEggCreateArchiver()
//
DKEggArchiverRef DKEggCreateArchiver( int options )
{
    DKEggArchiverRef egg = DKCreate( DKEggArchiverClass() );
    
    if( egg )
    {
    }
    
    return egg;
}


///
//  BuildHeader()
//
static void BuildHeader( DKEggArchiverRef _self, DKEggHeader * header )
{
    memset( header, 0, sizeof(DKEggHeader) );
    
    strncpy( header->prefix, DKEggBinaryPrefix, 8 );
    header->version = DKEggVersion;
    header->encodingVersion = DKEncodingVersion;
    header->byteOrder = _self->byteOrder;
    
    header->objectTable.index = sizeof(DKEggHeader);
    header->objectTable.length = (uint32_t)DKGenericArrayGetLength( &_self->archivedObjects );
    
    header->attributeTable.index = header->objectTable.index + (header->objectTable.length * sizeof(DKEggObject));
    header->attributeTable.length = 0;

    for( DKIndex i = 0; i < header->objectTable.length; ++i )
    {
        struct ArchivedObject * archivedObject = DKGenericArrayGetPointerToElementAtIndex( &_self->archivedObjects, i );
        header->attributeTable.length += DKGenericArrayGetLength( &archivedObject->attributes );
    }

    header->data.index = header->attributeTable.index + (header->attributeTable.length * sizeof(DKEggAttribute));
    header->data.length = (uint32_t)DKByteArrayGetLength( &_self->data );
}


///
//  WriteHeader()
//
static void WriteHeader( DKEggArchiverRef _self, const DKEggHeader * header, DKMutableObjectRef stream )
{
    DKWrite( stream, &header, sizeof(DKEggHeader), 1 );
}


///
//  WriteObjectTable()
//
static void WriteObjectTable( DKEggArchiverRef _self, const DKEggHeader * header, DKMutableObjectRef stream )
{
    DKIndex index = 0;

    for( DKIndex i = 0; i < header->objectTable.length; ++i )
    {
        struct ArchivedObject * archivedObject = DKGenericArrayGetPointerToElementAtIndex( &_self->archivedObjects, i );
        
        DKIndex count = DKGenericArrayGetLength( &archivedObject->attributes );
        
        DKEggObject object;
        object.className = archivedObject->className;
        object.attributes.index = (uint32_t)index;
        object.attributes.length = (uint32_t)count;
        
        DKWrite( stream, &object, sizeof(DKEggObject), 1 );
        
        index += count;
    }
}


///
//  WriteAttributeTable()
//
static void WriteAttributeTable( DKEggArchiverRef _self, const DKEggHeader * header, DKMutableObjectRef stream )
{
    for( DKIndex i = 0; i < header->objectTable.length; ++i )
    {
        struct ArchivedObject * archivedObject = DKGenericArrayGetPointerToElementAtIndex( &_self->archivedObjects, i );
        
        DKIndex count = DKGenericArrayGetLength( &archivedObject->attributes );
        
        if( count > 0 )
        {
            DKEggAttribute * attributes = DKGenericArrayGetPointerToElementAtIndex( &archivedObject->attributes, 0 );
            DKWrite( stream, attributes, sizeof(DKEggAttribute), count );
        }
    }
}


///
//  WriteData()
//
static void WriteData( DKEggArchiverRef _self, const DKEggHeader * header, DKMutableObjectRef stream )
{
    DKIndex length = DKByteArrayGetLength( &_self->data );
    
    if( length > 0 )
    {
        DKWrite( stream, DKByteArrayGetPtr( &_self->data ), 1, length );
    }
}


///
//  DKEggArchiverWriteToStream()
//
void DKEggArchiverWriteToStream( DKEggArchiverRef _self, DKMutableObjectRef stream )
{
    DKEggHeader header;
    BuildHeader( _self, &header );
    
    WriteHeader( _self, &header, stream );
    WriteObjectTable( _self, &header, stream );
    WriteAttributeTable( _self, &header, stream );
    WriteData( _self, &header, stream );
}


///
//  DKEggArchiverCreateData()
//
DKDataRef DKEggArchiverCreateData( DKEggArchiverRef _self )
{
    DKEggHeader header;
    BuildHeader( _self, &header );

    DKIndex size = header.data.index + header.data.length;
    DKMutableDataRef data = DKDataCreateMutable( size );
    
    WriteHeader( _self, &header, data );
    WriteObjectTable( _self, &header, data );
    WriteAttributeTable( _self, &header, data );
    WriteData( _self, &header, data );
    
    return data;
}


///
//  StackPush()
//
static void StackPush( DKEggArchiverRef _self, DKObjectRef object )
{
    // Add a new object table entry
    struct ArchivedObject archivedObject;
    archivedObject.object = DKRetain( object );
    archivedObject.className = (uint32_t)AddObject( _self, DKGetClassName( object ) );
    DKGenericArrayInit( &archivedObject.attributes, sizeof(DKEggAttribute) );
    
    DKIndex index = DKGenericArrayGetLength( &_self->archivedObjects );
    DKGenericArrayAppendElements( &_self->archivedObjects, &archivedObject, 1 );

    // Add the object to the visited list
    struct VisitedObjectsRow visitedObject;
    visitedObject.object = object;
    visitedObject.index = index;
    
    DKGenericHashTableInsert( &_self->visitedObjects, &visitedObject, DKInsertAlways );

    // Push the object context onto the stack
    DKGenericArrayAppendElements( &_self->stack, &index, 1 );
}


///
//  StackPop()
//
static void StackPop( DKEggArchiverRef _self )
{
    DKIndex length = DKGenericArrayGetLength( &_self->stack );
    DKGenericArrayReplaceElements( &_self->stack, DKRangeMake( length - 1, 1 ), NULL, 0 );
}



///
//  StackTop()
//
static struct ArchivedObject * StackTop( DKEggArchiverRef _self )
{
    DKIndex length = DKGenericArrayGetLength( &_self->stack );
    DKAssert( length > 0 );
    
    DKIndex index = DKGenericArrayGetElementAtIndex( &_self->stack, length - 1, DKIndex );
    DKAssert( (index >= 0) && (index < DKGenericArrayGetLength( &_self->archivedObjects )) );
    
    return DKGenericArrayGetPointerToElementAtIndex( &_self->archivedObjects, index );
}


///
//  AddObject()
//
static DKIndex AddObject( DKEggArchiverRef _self, DKObjectRef object )
{
    // Make sure we can serialize the object
    DKEggInterfaceRef eggInterface;
    
    if( !DKQueryInterface( object, DKSelector(Egg), (const void **)&eggInterface ) )
    {
        DKError( "DKEggWriteObject: %s does not implement .egg file archiving.\n",
            DKStringGetCStringPtr( DKGetClassName( object ) ) );
        
        return -1;
    }

    // Have we already seen this object?
    struct VisitedObjectsRow key = { object, 0 };
    const struct VisitedObjectsRow * visitedObject = DKGenericHashTableFind( &_self->visitedObjects, &key );
    
    if( !visitedObject )
    {
        StackPush( _self, object );
        eggInterface->addToEgg( object, _self );
        StackPop( _self );
        
        visitedObject = DKGenericHashTableFind( &_self->visitedObjects, &key );
        DKAssert( visitedObject );
    }
    
    return visitedObject->index;
}


///
//  AddAttribute()
//
static void AddAttribute( DKEggArchiverRef _self, DKStringRef key, DKEncoding encoding, DKIndex value )
{
    DKAssert( value <= 0xffffffff );

    struct ArchivedObject * archivedObject = StackTop( _self );
    
    DKEggAttribute attribute;
    attribute.key = dk_strhash32( DKStringGetCStringPtr( key ) );
    attribute.encoding = encoding;
    attribute.value = (uint32_t)value;
    
    DKGenericArrayAppendElements( &archivedObject->attributes, &attribute, 1 );
}


///
//  AddEncodedData()
//
static DKIndex AddEncodedData( DKEggArchiverRef _self, DKEncoding encoding, const void * data )
{
    DKIndex count = DKEncodingGetCount( encoding );
    DKIndex size = DKEncodingGetTypeSize( encoding );

    DKByteArrayAlignLength( &_self->data, size );

    DKIndex offset = DKByteArrayGetLength( &_self->data );
    
    DKByteArrayAppendBytes( &_self->data, NULL, size * count );

    if( (size == 1) || (_self->byteOrder == DKByteOrderNative) )
    {
        memcpy( &_self->data.bytes[offset], data, size * count );
    }

    else if( size == 2 )
    {
        memcpy( &_self->data.bytes[offset], data, size * count );
    }

    else if( size == 4 )
    {
        memcpy( &_self->data.bytes[offset], data, size * count );
    }

    else if( size == 8 )
    {
        memcpy( &_self->data.bytes[offset], data, size * count );
    }
    
    else
    {
        DKAssert( 0 );
    }
    
    return offset;
}


///
//  DKEggAddObject()
//
void DKEggAddObject( DKEggArchiverRef _self, DKStringRef key, DKObjectRef object )
{
    DKAssertKindOfClass( _self, DKEggArchiverClass() );

    if( object == NULL )
        return;

    // Make sure we can serialize the object
    DKEggInterfaceRef eggInterface;
    
    if( !DKQueryInterface( object, DKSelector(Egg), (const void **)&eggInterface ) )
    {
        DKError( "DKEggWriteObject: %s does not implement .egg file archiving.\n",
            DKStringGetCStringPtr( DKGetClassName( object ) ) );
        return;
    }

    DKIndex index = AddObject( _self, object );
    
    // Negative indexes are obviously an error and index 0 is the root object
    if( index > 0 )
    {
        AddAttribute( _self, key, DKEncode( DKEncodingTypeObject, 1 ), (uint32_t)index );
    }
}


///
//  DKEggAddCollection()
//
struct DKEggAddCollectionContext
{
    DKEggArchiverRef eggArchiver;
    uint32_t * indexes;
    DKIndex count;
};

static int DKEggAddCollectionCallback( DKObjectRef object, void * context )
{
    struct DKEggAddCollectionContext * ctx = context;
    
    DKIndex index = AddObject( ctx->eggArchiver, object );
    
    if( index > 0 )
    {
        ctx->indexes[ctx->count] = (uint32_t)index;
        ctx->count++;
    }
    
    return 0;
}

void DKEggAddCollection( DKEggArchiverRef _self, DKStringRef key, DKObjectRef collection )
{
    DKIndex count = DKGetCount( collection );
    DKEncoding encoding = DKEncode( DKEncodingTypeObject, (uint32_t)count );
 
    // A one-object collection is encoded the same as a single object
    if( count == 1 )
    {
        DKObjectRef object = DKGetAnyObject( collection );
        
        if( object )
        {
            DKIndex index = AddObject( _self, object );
            
            if( index > 0 )
            {
                AddAttribute( _self, key, DKEncode( DKEncodingTypeObject, 1 ), (uint32_t)index );
            }
        }
    }
    
    else
    {
        // Add data storage for the collection
        DKByteArrayAlignLength( &_self->data, 4 );

        DKIndex offset = DKByteArrayGetLength( &_self->data );
        
        DKByteArrayAppendBytes( &_self->data, NULL, sizeof(uint32_t) * count );
        
        // Add the objects in the collection
        struct DKEggAddCollectionContext ctx;
        ctx.eggArchiver = _self;
        ctx.indexes = (uint32_t *)&_self->data.bytes[offset];
        ctx.count = 0;
        
        DKForeachObject( collection, DKEggAddCollectionCallback, &ctx );

        // Add an attribute for the collection
        AddAttribute( _self, key, encoding, (uint32_t)offset );
    }
}


///
//  DKEggAddKeyedCollection()
//
struct DKEggAddKeyedCollectionContext
{
    DKEggArchiverRef eggArchiver;
    DKEggKVPair * kvpairs;
    DKIndex count;
};

static int DKEggAddKeyedCollectionCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    struct DKEggAddKeyedCollectionContext * ctx = context;
    
    DKIndex keyIndex = AddObject( ctx->eggArchiver, key );
    
    if( keyIndex > 0 )
    {
        DKIndex objectIndex = AddObject( ctx->eggArchiver, object );
    
        if( objectIndex > 0 )
        {
            ctx->kvpairs[ctx->count].key = (uint32_t)keyIndex;
            ctx->kvpairs[ctx->count].value = (uint32_t)objectIndex;
            ctx->count++;
        }
    }
    
    return 0;
}

void DKEggAddKeyedCollection( DKEggArchiverRef _self, DKStringRef key, DKObjectRef collection )
{
    DKIndex count = DKGetCount( collection );
    DKEncoding encoding = DKEncode( DKEncodingTypeKeyedObject, (uint32_t)count );
 
    // Add data storage for the collection
    DKByteArrayAlignLength( &_self->data, 4 );

    DKIndex offset = DKByteArrayGetLength( &_self->data );
    
    DKByteArrayAppendBytes( &_self->data, NULL, sizeof(uint32_t) * count * 2 );
    
    // Add the key+object pairs in the collection
    struct DKEggAddKeyedCollectionContext ctx;
    ctx.eggArchiver = _self;
    ctx.kvpairs = (DKEggKVPair *)&_self->data.bytes[offset];
    ctx.count = 0;
    
    DKForeachKeyAndObject( collection, DKEggAddKeyedCollectionCallback, &ctx );
    
    // Add an attribute for the collection
    AddAttribute( _self, key, encoding, (uint32_t)offset );
}


///
//  DKEggAddTextData()
//
void DKEggAddTextData( DKEggArchiverRef _self, DKStringRef key, const char * text, size_t length )
{
    if( length == 0 )
    {
        length = strlen( text );
    }

    if( (length + 1) > DKMaxEncodingSize )
    {
        DKError( "DKEggWriteTextData: %lu exceeds the maximum encoded data size (%dM)",
            length, DKMaxEncodingSize / (1024 * 1024) );
        return;
    }
    
    if( text[length + 1] != '\0' )
    {
        DKError( "DKEggWriteTextData: the text string is not terminated by a null byte." );
        return;
    }

    DKEncoding encoding = DKEncode( DKEncodingTypeTextData, (uint32_t)(length + 1) );
    DKIndex offset = AddEncodedData( _self, encoding, text );

    AddAttribute( _self, key, encoding, offset );
}


///
//  DKEggAddBinaryData()
//
void DKEggAddBinaryData( DKEggArchiverRef _self, DKStringRef key, const void * bytes, size_t length )
{
    if( length > DKMaxEncodingSize )
    {
        DKError( "DKEggWriteBinaryData: %lu exceeds the maximum encoded data size (%dM)",
            length, DKMaxEncodingSize / (1024 * 1024) );
        return;
    }

    DKEncoding encoding = DKEncode( DKEncodingTypeBinaryData, (uint32_t)length );
    DKIndex offset = AddEncodedData( _self, encoding, bytes );

    AddAttribute( _self, key, encoding, offset );
}


///
//  DKEggAddNumber()
//
void DKEggAddNumber( DKEggArchiverRef _self, DKStringRef key, DKEncoding encoding, const void * number )
{
    DKAssert( DKEncodingIsNumber( encoding ) );

    DKIndex offset = AddEncodedData( _self, encoding, number );

    AddAttribute( _self, key, encoding, offset );
}




