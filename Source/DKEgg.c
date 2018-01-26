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
#include "DKComparison.h"


// The egg selector is initialized by DKRuntimeInit() so that constant strings can be
// used during initialization.
//DKThreadSafeSelectorInit( Egg );




// Egg File ==============================================================================

// The file prefix is a newline + null (\n\0) terminated string. The version number is
// included only for readability -- it is not functional. To maintain compatibility,
// future versions may change the content, but not the size of the prefix.

#define DKEggPrefix         "EGG-Version: 1.0\n"
#define DKEggPrefixSize     32
#define DKEggVersion        1


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
    char prefix[DKEggPrefixSize];
    
    uint8_t version;
    uint8_t encodingVersion;
    uint8_t byteOrder;
    uint8_t rootless;

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
    uint32_t hashkey;
    uint32_t encoding;
    uint32_t value;

} DKEggAttribute;

typedef struct
{
    uint32_t key;
    uint32_t value;
    
} DKEggKVPair;




// DKEggUnarchiver =======================================================================

struct DKEggUnarchiver
{
    const DKObject _obj;
    
    DKGenericArray unarchivedObjects;
    DKGenericArray stack;

    DKByteArray buffer;
    
    const DKEggHeader * header;
    const DKEggObject * objectTable;
    const DKEggAttribute * attributeTable;
    const uint8_t * data;
};


static void DKEggUnarchiverFinalize( DKObjectRef _self );


///
//  DKEggUnarchiverClass()
//
DKThreadSafeClassInit( DKEggUnarchiverClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKEggUnarchiver" ), DKObjectClass(), sizeof(struct DKEggUnarchiver),
        DKPreventSubclassing | DKNoImplicitInitializer, NULL, DKEggUnarchiverFinalize );

    return cls;
}


///
//  DKEggUnarchiverFinalize()
//
static void DKEggUnarchiverFinalize( DKObjectRef _untyped_self )
{
    DKEggUnarchiverRef _self = _untyped_self;

    DKIndex unarchivedObjectCount = DKGenericArrayGetLength( &_self->unarchivedObjects );
    
    for( DKIndex i = 0; i < unarchivedObjectCount; ++i )
    {
        DKObjectRef object = DKGenericArrayElementAtIndex( &_self->unarchivedObjects, i, DKObjectRef );
        DKRelease( object );
    }
    
    DKGenericArrayFinalize( &_self->unarchivedObjects );
    DKGenericArrayFinalize( &_self->stack );
    
    DKByteArrayFinalize( &_self->buffer );
}


///
//  SwizzleEggHeader()
//
static void SwizzleEggHeader( DKEggHeader * header )
{
    header->objectTable.index = DKSwapInt32( header->objectTable.index );
    header->objectTable.length = DKSwapInt32( header->objectTable.length );

    header->attributeTable.index = DKSwapInt32( header->attributeTable.index );
    header->attributeTable.length = DKSwapInt32( header->attributeTable.length );

    header->data.index = DKSwapInt32( header->data.index );
    header->data.length = DKSwapInt32( header->data.length );
}


///
//  SwizzleEggObjects()
//
static void SwizzleEggObjects( DKEggObject * objects, uint32_t count )
{
    for( uint32_t i = 0; i < count; ++i )
    {
        DKEggObject * obj = &objects[i];
        obj->className = DKSwapInt32( obj->className );
        obj->attributes.index = DKSwapInt32( obj->attributes.index );
        obj->attributes.length = DKSwapInt32( obj->attributes.length );
    }
}


///
//  SwizzleEggAttributes()
//
static void SwizzleEggAttributes( DKEggAttribute * attributes, uint32_t count )
{
    for( uint32_t i = 0; i < count; ++i )
    {
        DKEggAttribute * attr = &attributes[i];
        attr->hashkey = DKSwapInt32( attr->hashkey );
        attr->encoding = DKSwapInt32( attr->encoding );
        attr->value = DKSwapInt32( attr->value );
    }
}


///
//  DKEggUnarchiverInitWithStream()
//
DKEggUnarchiverRef DKEggUnarchiverInitWithStream( DKEggUnarchiverRef _self, DKObjectRef stream )
{
    _self = DKSuperInit( _self, DKObjectClass() );
    
    if( _self )
    {
        DKGenericArrayInit( &_self->unarchivedObjects, sizeof(DKObjectRef) );
        DKGenericArrayInit( &_self->stack, sizeof(DKIndex) );
        DKByteArrayInit( &_self->buffer );

        // Read the header into the buffer
        DKByteArrayReserve( &_self->buffer, sizeof(DKEggHeader) );
        
        if( DKRead( stream, DKByteArrayGetBytePtr( &_self->buffer, 0 ), sizeof(DKEggHeader), 1 ) != 1 )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }

        _self->header = DKByteArrayGetBytePtr( &_self->buffer, 0 );

        // Check the header
        if( memcmp( _self->header->prefix, DKEggPrefix, DKEggPrefixSize ) != 0 )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }
        
        if( _self->header->version != DKEggVersion )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }

        if( _self->header->encodingVersion != DKEncodingVersion )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }
        
        if( _self->header->byteOrder != DKByteOrderNative )
        {
            DKWarning( "DKEggUnarchiver: Reading an archive with a non-native byte order is untested.\n" );
        
            // Note: This is safe since we've copied the header into our own buffer
            SwizzleEggHeader( (DKEggHeader *)_self->header );
        }

        DKIndex archiveLength = _self->header->data.index + _self->header->data.length;
        DKIndex remainingLength = archiveLength - sizeof(DKEggHeader);
        
        // Read the rest of the archive into the buffer
        DKByteArrayReserve( &_self->buffer, archiveLength );
        
        if( DKRead( stream, DKByteArrayGetBytePtr( &_self->buffer, sizeof(DKEggHeader) ), 1, remainingLength ) != remainingLength )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }
        
        // Set the data pointers and update the header pointer since memory may have shifted
        _self->header = DKByteArrayGetBytePtr( &_self->buffer, 0 );
        _self->objectTable = DKByteArrayGetBytePtr( &_self->buffer, sizeof(DKEggHeader) );
        _self->attributeTable = DKByteArrayGetBytePtr( &_self->buffer, sizeof(DKEggHeader) +
            (_self->header->objectTable.length * sizeof(DKEggObject)) );
        _self->data = DKByteArrayGetBytePtr( &_self->buffer, sizeof(DKEggHeader) +
            (_self->header->objectTable.length * sizeof(DKEggObject)) +
            (_self->header->attributeTable.length * sizeof(DKEggAttribute)) );

        if( _self->header->byteOrder != DKByteOrderNative )
        {
            // Note: This is safe since we've copied the tables into our own buffer
            SwizzleEggObjects( (DKEggObject *)_self->objectTable, _self->header->objectTable.length );
            SwizzleEggAttributes( (DKEggAttribute *)_self->attributeTable, _self->header->attributeTable.length );
        }

        // Setup the unarchived object table
        DKGenericArrayAppendElements( &_self->unarchivedObjects, NULL, _self->header->objectTable.length );

        // For rootless archives push the anonymous root object
        if( _self->header->rootless )
        {
            DKGenericArrayPush( &_self->stack, &(DKIndex){ 0 } );
        }
    }
    
    return _self;
}


///
//  DKEggUnarchiverInitWithData()
//
DKEggUnarchiverRef DKEggUnarchiverInitWithData( DKEggUnarchiverRef _self, DKDataRef data )
{
    _self = DKSuperInit( _self, DKObjectClass() );
    
    if( _self )
    {
        DKGenericArrayInit( &_self->unarchivedObjects, sizeof(DKObjectRef) );
        DKGenericArrayInit( &_self->stack, sizeof(DKIndex) );
        DKByteArrayInit( &_self->buffer );

        // Read the header
        if( DKDataGetLength( data ) < sizeof(DKEggHeader) )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }
    
        _self->header = DKDataGetBytePtr( data, 0 );
        
        // Check the header
        if( strcmp( _self->header->prefix, DKEggPrefix ) != 0 )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }
        
        if( _self->header->version != DKEggVersion )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }

        if( _self->header->encodingVersion != DKEncodingVersion )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }
        
        if( _self->header->byteOrder != DKByteOrderNative )
        {
            // Copy the header, object table and attribute table into our buffer
            uint32_t objectTableLength = DKSwapInt32( _self->header->objectTable.length );
            uint32_t attributeTableLength = DKSwapInt32( _self->header->attributeTable.length );

            size_t bytelength = sizeof(DKEggHeader) +
                (sizeof(DKEggObject) * objectTableLength) +
                (sizeof(DKEggAttribute) * attributeTableLength);

            if( DKDataGetLength( data ) < bytelength )
            {
                // *** ERROR ***
                
                DKRelease( _self );
                return NULL;
            }

            DKByteArrayAppendBytes( &_self->buffer, DKDataGetBytePtr( data, 0 ), bytelength );
            
            // Update the header pointer to the copied version
            _self->header = DKByteArrayGetBytePtr( &_self->buffer, 0 );
            
            // Swizzle the copied header
            SwizzleEggHeader( (DKEggHeader *)_self->header );
        }

        if( DKDataGetLength( data ) < (_self->header->data.index + _self->header->data.length) )
        {
            // *** ERROR ***
            
            DKRelease( _self );
            return NULL;
        }
        
        // Set the other data pointers
        if( _self->header->byteOrder != DKByteOrderNative )
        {
            _self->objectTable = DKByteArrayGetBytePtr( &_self->buffer, sizeof(DKEggHeader) );
            _self->attributeTable = DKByteArrayGetBytePtr( &_self->buffer, sizeof(DKEggHeader) +
                (_self->header->objectTable.length * sizeof(DKEggObject)) );

            SwizzleEggObjects( (DKEggObject *)_self->objectTable, _self->header->objectTable.length );
            SwizzleEggAttributes( (DKEggAttribute *)_self->attributeTable, _self->header->attributeTable.length );
        }
        
        else
        {
            _self->objectTable = DKDataGetBytePtr( data, sizeof(DKEggHeader) );
            _self->attributeTable = DKDataGetBytePtr( data, sizeof(DKEggHeader) +
                (_self->header->objectTable.length * sizeof(DKEggObject)) );
        }

        _self->data = DKDataGetBytePtr( data, sizeof(DKEggHeader) +
            (_self->header->objectTable.length * sizeof(DKEggObject)) +
            (_self->header->attributeTable.length * sizeof(DKEggAttribute)) );
        
        // Setup the unarchived object table
        DKGenericArrayAppendElements( &_self->unarchivedObjects, NULL, _self->header->objectTable.length );

        // For rootless archives push the anonymous root object
        if( _self->header->rootless )
        {
            DKGenericArrayPush( &_self->stack, &(DKIndex){ 0 } );
        }
    }
    
    return _self;
}


///
//  GetAttribute()
//
static const DKEggAttribute * GetAttribute( DKEggUnarchiverRef _self, DKStringRef key )
{
    uint32_t hashkey = dk_strhash32( DKStringGetCStringPtr( key ) );
    
    DKIndex index = DKGenericArrayLastElement( &_self->stack, DKIndex );
    const DKEggObject * object = &_self->objectTable[index];
    
    for( uint32_t i = 0; i < object->attributes.length; ++i )
    {
        const DKEggAttribute * attribute = &_self->attributeTable[object->attributes.index + i];
        
        if( attribute->hashkey == hashkey )
            return attribute;
    }
    
    return NULL;
}


///
//  GetClass()
//
static DKClassRef GetClass( DKEggUnarchiverRef _self, uint32_t offset )
{
    const char * cstr = (const char *)&_self->data[offset];
    DKStringRef className = DKStringInitWithCString( DKAlloc( DKStringClass() ), cstr );
    DKClassRef cls = DKClassFromString( className );
    DKRelease( className );
    
    if( !cls )
    {
        DKError( "DKEggUnarchiver: %s is not a recognized class name.\n", cstr );
    }

    return cls;
}


///
//  GetCSelector()
//
static DKSEL GetSelector( DKEggUnarchiverRef _self, uint32_t offset )
{
    const char * cstr = (const char *)&_self->data[offset];
    DKStringRef selectorName = DKStringInitWithCString( DKAlloc( DKStringClass() ), cstr );
    DKSEL sel = DKSelectorFromString( selectorName );
    DKRelease( selectorName );
    
    if( !sel )
    {
        DKError( "DKEggUnarchiver: %s is not a recognized selector name.\n", cstr );
    }

    return sel;
}


///
//  GetObject()
//
static DKObjectRef GetObject( DKEggUnarchiverRef _self, DKIndex index )
{
    DKAssert( index >= 0 );
    DKCheckIndex( index, _self->header->objectTable.length, NULL );
    
    DKObjectRef object = DKGenericArrayElementAtIndex( &_self->unarchivedObjects, index, DKObjectRef );
    
    if( object == NULL )
    {
        const DKEggObject * archivedObject = &_self->objectTable[index];
        
        // Get the class
        DKClassRef cls = GetClass( _self, archivedObject->className );
        
        if( !cls )
            return NULL;
        
        // Allocate the object
        object = DKAlloc( cls );
        
        // Make sure we can deserialize the object
        DKEggInterfaceRef eggInterface;
        
        if( !DKQueryInterface( object, DKSelector(Egg), (DKInterfaceRef *)&eggInterface ) )
        {
            DKError( "DKEggUnarchiver: %@ does not implement .egg file archiving.\n", DKGetClassName( cls ) );
            
            DKRelease( object );
            
            return NULL;
        }

        // Add the object to the array
        DKGenericArrayReplaceElements( &_self->unarchivedObjects, DKRangeMake( index, 1 ), &object, 1 );

        // Unarchive the object
        DKGenericArrayPush( &_self->stack, &index );
        object = eggInterface->initWithEgg( object, _self );
        DKGenericArrayPop( &_self->stack );
        
        // Re-add the object in case it was replaced by the unarchiving method
        DKGenericArrayReplaceElements( &_self->unarchivedObjects, DKRangeMake( index, 1 ), &object, 1 );
    }
    
    return object;
}


///
//  DKEggGetRootObject()
//
DKObjectRef DKEggGetRootObject( DKEggUnarchiverRef _self )
{
    return GetObject( _self, 0 );
}


///
//  DKEggGetEncoding()
//
DKEncoding DKEggGetEncoding( DKEggUnarchiverRef _self, DKStringRef key )
{
    const DKEggAttribute * attribute = GetAttribute( _self, key );
    
    if( attribute )
        return attribute->encoding;
    
    return DKEncodingNull;
}


///
//  DKEggGetObject()
//
DKObjectRef DKEggGetObject( DKEggUnarchiverRef _self, DKStringRef key )
{
    const DKEggAttribute * attribute = GetAttribute( _self, key );
    
    if( attribute )
    {
        if( attribute->encoding == DKEncode( DKEncodingTypeObject, 1 ) )
            return GetObject( _self, attribute->value );
        
        else if( attribute->encoding == DKEncode( DKEncodingTypeClass, 1 ) )
            return GetClass( _self, attribute->value );
        
        else if( attribute->encoding == DKEncode( DKEncodingTypeSelector, 1 ) )
            return GetSelector( _self, attribute->value );
    }
    
    return NULL;
}


///
//  DKEggGetCollection()
//
void DKEggGetCollection( DKEggUnarchiverRef _self, DKStringRef key, DKApplierFunction callback, void * context )
{
    const DKEggAttribute * attribute = GetAttribute( _self, key );
    
    if( attribute )
    {
        if( DKEncodingGetType( attribute->encoding ) == DKEncodingTypeObject )
        {
            uint32_t count = DKEncodingGetCount( attribute->encoding );
            
            if( count == 1 )
            {
                DKObjectRef object = (attribute->value > 0) ? GetObject( _self, attribute->value ) : NULL;
                callback( object, context );
            }
            
            else
            {
                const uint32_t * indexes = (const uint32_t *)&_self->data[attribute->value];
            
                for( uint32_t i = 0; i < count; ++i )
                {
                    uint32_t index = indexes[i];
                    
                    if( _self->header->byteOrder != DKByteOrderNative )
                        index = DKSwapInt32( index );
                
                    DKObjectRef object = (indexes[i] > 0) ? GetObject( _self, index ) : NULL;
                    callback( object, context );
                }
            }
        }
    }
}


///
//  DKEggGetKeyedCollection()
//
void DKEggGetKeyedCollection( DKEggUnarchiverRef _self, DKStringRef key, DKKeyedApplierFunction callback, void * context )
{
    const DKEggAttribute * attribute = GetAttribute( _self, key );
    
    if( attribute )
    {
        if( DKEncodingGetType( attribute->encoding ) == DKEncodingTypeKeyedObject )
        {
            uint32_t count = DKEncodingGetCount( attribute->encoding );
            
            const DKEggKVPair * indexes = (const DKEggKVPair *)&_self->data[attribute->value];
        
            for( uint32_t i = 0; i < count; ++i )
            {
                uint32_t k = indexes[i].key;
                uint32_t v = indexes[i].value;
                
                if( _self->header->byteOrder != DKByteOrderNative )
                {
                    k = DKSwapInt32( k );
                    v = DKSwapInt32( v );
                }
            
                DKObjectRef key = GetObject( _self, k );
                
                if( key )
                {
                    DKObjectRef object = v ? GetObject( _self, v ) : NULL;
                    callback( key, object, context );
                }
            }
        }
    }
}


///
//  DKEggGetTextData()
//
const char * DKEggGetTextDataPtr( DKEggUnarchiverRef _self, DKStringRef key, size_t * length )
{
    const DKEggAttribute * attribute = GetAttribute( _self, key );
    
    if( attribute )
    {
        if( DKEncodingGetType( attribute->encoding ) == DKEncodingTypeTextData )
        {
            *length = DKEncodingGetSize( attribute->encoding );
            
            return (const char *)&_self->data[attribute->value];
        }
    }
    
    return NULL;
}

size_t DKEggGetTextData( DKEggUnarchiverRef _self, DKStringRef key, char * text )
{
    size_t length = 0;
    
    const char * src = DKEggGetTextDataPtr( _self, key, &length );
    
    if( src )
    {
        memcpy( text, src, length );
        return length - 1;
    }
    
    return 0;
}


///
//  DKEggGetBinaryData()
//
const void * DKEggGetBinaryDataPtr( DKEggUnarchiverRef _self, DKStringRef key, size_t * length )
{
    const DKEggAttribute * attribute = GetAttribute( _self, key );
    
    if( attribute )
    {
        if( DKEncodingGetType( attribute->encoding ) == DKEncodingTypeBinaryData )
        {
            *length = DKEncodingGetSize( attribute->encoding );
            
            return &_self->data[attribute->value];
        }
    }
    
    return 0;
}

size_t DKEggGetBinaryData( DKEggUnarchiverRef _self, DKStringRef key, void * bytes )
{
    size_t length = 0;
    
    const char * src = DKEggGetBinaryDataPtr( _self, key, &length );
    
    if( src )
        memcpy( bytes, src, length );
    
    return length;
}


///
//  DKEggGetNumberData()
//
size_t DKEggGetNumberData( DKEggUnarchiverRef _self, DKStringRef key, void * number )
{
    const DKEggAttribute * attribute = GetAttribute( _self, key );
    
    if( attribute )
    {
        if( DKEncodingIsNumber( attribute->encoding ) )
        {
            size_t size = DKEncodingGetTypeSize( attribute->encoding );
            size_t count = DKEncodingGetCount( attribute->encoding );
            
            if( (size == 1) || (_self->header->byteOrder == DKByteOrderNative) )
            {
                const void * src = &_self->data[attribute->value];
                memcpy( number, src, size * count );
            }
            
            else if( size == 2 )
            {
                uint16_t * dst = number;
                const uint16_t * src = (uint16_t *)&_self->data[attribute->value];
                
                for( DKIndex i = 0; i < count; ++i )
                    dst[i] = DKSwapInt16( src[i] );
            }
            
            else if( size == 4 )
            {
                uint32_t * dst = number;
                const uint32_t * src = (uint32_t *)&_self->data[attribute->value];
                
                for( DKIndex i = 0; i < count; ++i )
                    dst[i] = DKSwapInt32( src[i] );
            }
            
            else if( size == 8 )
            {
                uint64_t * dst = number;
                const uint64_t * src = (uint64_t *)&_self->data[attribute->value];
                
                for( DKIndex i = 0; i < count; ++i )
                    dst[i] = DKSwapInt64( src[i] );
            }
            
            else
            {
                DKAssert( 0 );
            }
            
            return count;
        }
    }
    
    return 0;
}




// DKEggArchiver =========================================================================

struct DKEggArchiver
{
    const DKObject _obj;
    
    DKByteOrder byteOrder;
    bool rootless;
    
    DKGenericArray stack;
    DKGenericArray archivedObjects;
    DKGenericHashTable visitedObjects;
    DKGenericHashTable symbolTable;
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

static DKRowStatus VisitedObjectsRowStatus( const void * _row )
{
    const struct VisitedObjectsRow * row = _row;
    return (DKRowStatus)(row->object);
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
    
    row->object = DKRowStatusEmpty;
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

    row->object = DKRowStatusDeleted;
}




// Methods -------------------------------------------------------------------------------

static DKObjectRef DKEggArchiverInit( DKObjectRef _self );
static void DKEggArchiverFinalize( DKObjectRef _self );

static DKIndex AddSymbol( DKEggArchiverRef _self, DKStringRef symbol );
static DKIndex AddObject( DKEggArchiverRef _self, DKObjectRef object );

///
//  DKEggArchiverClass()
//
DKThreadSafeClassInit( DKEggArchiverClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKEggArchiver" ), DKObjectClass(), sizeof(struct DKEggArchiver),
        DKPreventSubclassing, DKEggArchiverInit, DKEggArchiverFinalize );

    return cls;
}


///
//  DKEggArchiverInit()
//
static DKObjectRef DKEggArchiverInit( DKObjectRef _self )
{
    return DKEggArchiverInitWithObject( _self, NULL );
}


///
//  DKEggArchiverInitWithObject()
//
DKObjectRef DKEggArchiverInitWithObject( DKObjectRef _untyped_self, DKObjectRef object )
{
    DKEggArchiverRef _self = DKSuperInit( _untyped_self, DKObjectClass() );

    if( _self )
    {
        _self->byteOrder = DKByteOrderNative;
        
        DKGenericHashTableCallbacks callbacks =
        {
            VisitedObjectsRowStatus,
            VisitedObjectsRowHash,
            VisitedObjectsRowEqual,
            VisitedObjectsRowInit,
            VisitedObjectsRowUpdate,
            VisitedObjectsRowDelete
        };

        DKGenericArrayInit( &_self->stack, sizeof(DKIndex) );
        DKGenericArrayInit( &_self->archivedObjects, sizeof(struct ArchivedObject) );
        DKGenericHashTableInit( &_self->visitedObjects, sizeof(struct VisitedObjectsRow), &callbacks );
        DKGenericHashTableInit( &_self->symbolTable, sizeof(struct VisitedObjectsRow), &callbacks );
        DKByteArrayInit( &_self->data );

        // Rooted archive, add the root object
        if( object )
        {
            _self->rootless = false;
            
            AddObject( _self, object );
        }
        
        // Rootless archive, push an anonymous root dictionary
        else
        {
            _self->rootless = true;

            struct ArchivedObject rootObject;
            rootObject.object = NULL;
            rootObject.className = (uint32_t)AddSymbol( _self, DKSTR( "DKDictionary" ) );
            DKGenericArrayInit( &rootObject.attributes, sizeof(DKEggAttribute) );
            
            DKGenericArrayAppendElements( &_self->archivedObjects, &rootObject, 1 );

            DKGenericArrayPush( &_self->stack, &(DKIndex){ 0 } );
        }
    }

    return _self;
}


///
//  DKEggArchiverFinalize()
//
static void DKEggArchiverFinalize( DKObjectRef _untyped_self )
{
    DKEggArchiverRef _self = _untyped_self;
    
    DKIndex archivedObjectCount = DKGenericArrayGetLength( &_self->archivedObjects );
    
    for( DKIndex i = 0; i < archivedObjectCount; ++i )
    {
        struct ArchivedObject * archivedObject = DKGenericArrayGetPointerToElementAtIndex( &_self->archivedObjects, i );

        DKRelease( archivedObject->object );
        DKGenericArrayFinalize( &archivedObject->attributes );
    }
    
    DKGenericArrayFinalize( &_self->stack );
    DKGenericArrayFinalize( &_self->archivedObjects );
    DKGenericHashTableFinalize( &_self->visitedObjects );
    DKGenericHashTableFinalize( &_self->symbolTable );
    DKByteArrayFinalize( &_self->data );
}


///
//  BuildHeader()
//
static void BuildHeader( DKEggArchiverRef _self, DKEggHeader * header )
{
    memset( header, 0, sizeof(DKEggHeader) );
    
    strcpy( header->prefix, DKEggPrefix );
    header->version = DKEggVersion;
    header->encodingVersion = DKEncodingVersion;
    header->byteOrder = _self->byteOrder;
    header->rootless = _self->rootless;
    
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
static void WriteHeader( DKEggArchiverRef _self, const DKEggHeader * header, DKObjectRef stream )
{
    DKWrite( stream, header, sizeof(DKEggHeader), 1 );
}


///
//  WriteObjectTable()
//
static void WriteObjectTable( DKEggArchiverRef _self, const DKEggHeader * header, DKObjectRef stream )
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
static void WriteAttributeTable( DKEggArchiverRef _self, const DKEggHeader * header, DKObjectRef stream )
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
static void WriteData( DKEggArchiverRef _self, const DKEggHeader * header, DKObjectRef stream )
{
    DKIndex length = DKByteArrayGetLength( &_self->data );
    
    if( length > 0 )
    {
        DKWrite( stream, DKByteArrayGetBytePtr( &_self->data, 0 ), 1, length );
    }
}


///
//  DKEggArchiverWriteToStream()
//
void DKEggArchiverWriteToStream( DKEggArchiverRef _self, DKObjectRef stream )
{
    DKEggHeader header;
    BuildHeader( _self, &header );
    
    WriteHeader( _self, &header, stream );
    WriteObjectTable( _self, &header, stream );
    WriteAttributeTable( _self, &header, stream );
    WriteData( _self, &header, stream );
}


///
//  DKEggArchiverCopyData()
//
DKDataRef DKEggArchiverCopyData( DKEggArchiverRef _self )
{
    DKEggHeader header;
    BuildHeader( _self, &header );

    DKIndex size = header.data.index + header.data.length;
    DKMutableDataRef data = DKDataInitWithCapacity( DKAlloc( DKMutableDataClass() ), size );
    
    WriteHeader( _self, &header, data );
    WriteObjectTable( _self, &header, data );
    WriteAttributeTable( _self, &header, data );
    WriteData( _self, &header, data );
    
    return data;
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
        uint16_t * dst = (uint16_t *)&_self->data.bytes[offset];
        const uint16_t * src = data;
        
        for( DKIndex i = 0; i < count; ++i )
            dst[i] = DKSwapInt16( src[i] );
    }

    else if( size == 4 )
    {
        uint32_t * dst = (uint32_t *)&_self->data.bytes[offset];
        const uint32_t * src = data;
        
        for( DKIndex i = 0; i < count; ++i )
            dst[i] = DKSwapInt32( src[i] );
    }

    else if( size == 8 )
    {
        uint64_t * dst = (uint64_t *)&_self->data.bytes[offset];
        const uint64_t * src = data;
        
        for( DKIndex i = 0; i < count; ++i )
            dst[i] = DKSwapInt64( src[i] );
    }
    
    else
    {
        DKAssert( 0 );
    }
    
    return offset;
}


///
//  AddSymbol()
//
static DKIndex AddSymbol( DKEggArchiverRef _self, DKStringRef symbol )
{
    // Non-constant symbols require different hash table callbacks than the visited list
    DKAssertKindOfClass( symbol, DKConstantStringClass() );

    // Check the symbol table first
    struct VisitedObjectsRow key = { symbol, 0 };
    const struct VisitedObjectsRow * visitedObject = DKGenericHashTableFind( &_self->symbolTable, &key );
    
    if( visitedObject )
        return visitedObject->index;

    // Add the symbol to the encoded data
    const char * str = DKStringGetCStringPtr( symbol );
    DKIndex length = DKStringGetByteLength( symbol );
    
    DKEncoding encoding = DKEncode( DKEncodingTypeTextData, (uint32_t)(length + 1) );
    DKIndex index = AddEncodedData( _self, encoding, str );
    
    // Add the symbol to the symbol table
    key.index = index;
    DKGenericHashTableInsert( &_self->symbolTable, &key, DKInsertAlways );
    
    return index;
}


///
//  AddObject()
//
static DKIndex AddObject( DKEggArchiverRef _self, DKObjectRef object )
{
    // Make sure we can serialize the object
    DKEggInterfaceRef eggInterface;
    
    if( !DKQueryInterface( object, DKSelector(Egg), (DKInterfaceRef *)&eggInterface ) )
    {
        DKError( "DKEggArchiver: %@ does not implement .egg file archiving.\n", DKGetClassName( object ) );
        
        return -1;
    }

    // Have we already seen this object?
    struct VisitedObjectsRow key = { object, 0 };
    const struct VisitedObjectsRow * visitedObject = DKGenericHashTableFind( &_self->visitedObjects, &key );
    
    if( visitedObject )
        return visitedObject->index;
    
    // Remember the index of the new object
    DKIndex index = DKGenericArrayGetLength( &_self->archivedObjects );

    // Add a new archived object
    struct ArchivedObject newArchivedObject;
    newArchivedObject.object = DKRetain( object );
    newArchivedObject.className = (uint32_t)AddSymbol( _self, DKGetClassName( object ) );
    DKGenericArrayInit( &newArchivedObject.attributes, sizeof(DKEggAttribute) );
    
    DKGenericArrayAppendElements( &_self->archivedObjects, &newArchivedObject, 1 );

    // Add the object to the visited list
    key.index = index;
    DKGenericHashTableInsert( &_self->visitedObjects, &key, DKInsertAlways );

    // Archive the object
    DKGenericArrayPush( &_self->stack, &index );
    eggInterface->addToEgg( object, _self );
    DKGenericArrayPop( &_self->stack );
    
    return index;
}


///
//  AddAttribute()
//
static void AddAttribute( DKEggArchiverRef _self, DKStringRef key, DKEncoding encoding, DKIndex value )
{
    DKAssert( value <= 0xffffffff );

    DKIndex index = DKGenericArrayLastElement( &_self->stack, DKIndex );
    struct ArchivedObject * archivedObject = DKGenericArrayGetPointerToElementAtIndex( &_self->archivedObjects, index );
    
    DKEggAttribute attribute;
    attribute.hashkey = dk_strhash32( DKStringGetCStringPtr( key ) );
    attribute.encoding = encoding;
    attribute.value = (uint32_t)value;
    
    DKGenericArrayAppendElements( &archivedObject->attributes, &attribute, 1 );
}


///
//  DKEggAddObject()
//
void DKEggAddObject( DKEggArchiverRef _self, DKStringRef key, DKObjectRef object )
{
    DKAssertKindOfClass( _self, DKEggArchiverClass() );

    if( object == NULL )
        return;

    DKIndex index;
    DKEncoding encoding;

    if( DKIsKindOfClass( object, DKClassClass() ) )
    {
        index = AddSymbol( _self, DKGetClassName( object ) );
        encoding = DKEncode( DKEncodingTypeClass, 1 );
    }
    
    else if( DKIsKindOfClass( object, DKSelectorClass() ) )
    {
        index = AddSymbol( _self, ((DKSEL)object)->name );
        encoding = DKEncode( DKEncodingTypeSelector, 1 );
    }

    else
    {
        index = AddObject( _self, object );
        encoding = DKEncode( DKEncodingTypeObject, 1 );
    }
    
    // Negative indexes are obviously an error and index 0 is the root object
    if( index > 0 )
    {
        AddAttribute( _self, key, encoding, (uint32_t)index );
    }
}


///
//  DKEggAddCollection()
//
struct DKEggAddCollectionContext
{
    DKEggArchiverRef eggArchiver;
    DKIndex offset;
    DKIndex count;
};

static int DKEggAddCollectionCallback( DKObjectRef object, void * context )
{
    struct DKEggAddCollectionContext * ctx = context;
    
    if( object )
    {
        DKIndex index = object ? AddObject( ctx->eggArchiver, object ) : 0;
        
        if( index >= 0 )
        {
            uint32_t * indexes = (uint32_t *)&ctx->eggArchiver->data.bytes[ctx->offset];
            
            indexes[ctx->count] = (uint32_t)index;
            ctx->count++;
        }
    }
    
    return 0;
}

void DKEggAddCollection( DKEggArchiverRef _self, DKStringRef key, DKObjectRef collection )
{
    DKIndex count = DKGetCount( collection );
    DKEncoding encoding = DKEncode( DKEncodingTypeObject, (uint32_t)count );
 
    if( count == 0 )
    {
        // Nothing to do here
    }
 
    // A one-object collection is encoded the same as a single object
    else if( count == 1 )
    {
        DKObjectRef object = DKGetAnyObject( collection );
        DKIndex index = object ? AddObject( _self, object ) : 0;
            
        if( index >= 0 )
            AddAttribute( _self, key, encoding, (uint32_t)index );
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
        ctx.offset = offset;
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
    DKIndex offset;
    DKIndex count;
};

static int DKEggAddKeyedCollectionCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    struct DKEggAddKeyedCollectionContext * ctx = context;
    
    DKIndex keyIndex = AddObject( ctx->eggArchiver, key );
    
    if( keyIndex > 0 )
    {
        DKIndex objectIndex = object ? AddObject( ctx->eggArchiver, object ) : 0;
        
        if( objectIndex >= 0 )
        {
            DKEggKVPair * kvpairs = (DKEggKVPair *)&ctx->eggArchiver->data.bytes[ctx->offset];
            
            kvpairs[ctx->count].key = (uint32_t)keyIndex;
            kvpairs[ctx->count].value = (uint32_t)objectIndex;
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
    ctx.offset = offset;
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
        DKError( "DKEggArchiver: %lu exceeds the maximum encoded data size (%dM)",
            length, DKMaxEncodingSize / (1024 * 1024) );
        return;
    }
    
    if( text[length] != '\0' )
    {
        DKError( "DKEggArchiver: the text string is not terminated by a null byte." );
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
        DKError( "DKEggArchiver: %lu exceeds the maximum encoded data size (%dM)",
            length, DKMaxEncodingSize / (1024 * 1024) );
        return;
    }

    DKEncoding encoding = DKEncode( DKEncodingTypeBinaryData, (uint32_t)length );
    DKIndex offset = AddEncodedData( _self, encoding, bytes );

    AddAttribute( _self, key, encoding, offset );
}


///
//  DKEggAddNumberData()
//
void DKEggAddNumberData( DKEggArchiverRef _self, DKStringRef key, DKEncoding encoding, const void * number )
{
    DKAssert( DKEncodingIsNumber( encoding ) );

    DKIndex offset = AddEncodedData( _self, encoding, number );

    AddAttribute( _self, key, encoding, offset );
}


