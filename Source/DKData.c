/*****************************************************************************************

  DKData.c

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

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKData.h"
#include "DKByteArray.h"
#include "DKStream.h"
#include "DKString.h"
#include "DKBuffer.h"
#include "DKComparison.h"
#include "DKDescription.h"
#include "DKCopying.h"
#include "DKEgg.h"
#include "DKFile.h"


struct DKData
{
    DKObject _obj;
    DKByteArray byteArray;
    DKIndex cursor;
};


static DKObjectRef  DKDataInitialize( DKObjectRef _self );
static void         DKDataFinalize( DKObjectRef _self );

static DKObjectRef  DKDataInitWithEgg( DKObjectRef _self, DKEggUnarchiverRef egg );
static void         DKDataAddToEgg( DKDataRef _self, DKEggArchiverRef egg );




// Class Methods =========================================================================

///
//  DKDataClass()
//
DKThreadSafeClassInit( DKDataClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKData" ), DKObjectClass(), sizeof(struct DKData), DKImmutableInstances, DKDataInitialize, DKDataFinalize );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKNewInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKDataEqual;
    comparison->compare = (DKCompareMethod)DKDataCompare;
    comparison->hash = (DKHashMethod)DKDataHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKDataMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Description
    struct DKDescriptionInterface * description = DKNewInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKDataGetDescription;
    description->getSizeInBytes = (DKGetSizeInBytesMethod)DKDataGetLength;
    
    DKInstallInterface( cls, description );
    DKRelease( description );
    
    // Buffer
    struct DKBufferInterface * buffer = DKNewInterface( DKSelector(Buffer), sizeof(struct DKBufferInterface) );
    buffer->getLength = (DKBufferGetLengthMethod)DKDataGetLength;
    buffer->getBytePtr = (DKBufferGetBytePtrMethod)DKDataGetBytePtr;
    buffer->setLength = (DKBufferSetLengthMethod)DKImmutableObjectAccessError;
    buffer->getMutableBytePtr = (DKBufferGetMutableBytePtrMethod)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, buffer );
    DKRelease( buffer );

    // Stream
    struct DKStreamInterface * stream =DKNewInterface( DKSelector(Stream), sizeof(struct DKStreamInterface) );
    stream->seek = (DKStreamSeekMethod)DKDataSeek;
    stream->tell = (DKStreamTellMethod)DKDataTell;
    stream->read = (DKStreamReadMethod)DKDataRead;
    stream->write = (DKStreamWriteMethod)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    // Egg
    struct DKEggInterface * egg = DKNewInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = DKDataInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKDataAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKMutableDataClass()
//
DKThreadSafeClassInit( DKMutableDataClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKMutableData" ), DKDataClass(), sizeof(struct DKData), 0, NULL, NULL );
    
    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = (DKCopyMethod)DKDataCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKDataMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Buffer
    struct DKBufferInterface * buffer = DKNewInterface( DKSelector(Buffer), sizeof(struct DKBufferInterface) );
    buffer->getLength = (DKBufferGetLengthMethod)DKDataGetLength;
    buffer->getBytePtr = (DKBufferGetBytePtrMethod)DKDataGetBytePtr;
    buffer->setLength = (DKBufferSetLengthMethod)DKDataSetLength;
    buffer->getMutableBytePtr = (DKBufferGetMutableBytePtrMethod)DKDataGetMutableBytePtr;
    
    DKInstallInterface( cls, buffer );
    DKRelease( buffer );

    // Stream
    struct DKStreamInterface * stream = DKNewInterface( DKSelector(Stream), sizeof(struct DKStreamInterface) );
    stream->seek = (DKStreamSeekMethod)DKDataSeek;
    stream->tell = (DKStreamTellMethod)DKDataTell;
    stream->read = (DKStreamReadMethod)DKDataRead;
    stream->write = (DKStreamWriteMethod)DKDataWrite;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
}




// Internals =============================================================================

///
//  DKDataSetCursor()
//
static void DKDataSetCursor( const struct DKData * data, DKIndex cursor )
{
    struct DKData * _data = (struct DKData *)data;
    
    if( cursor < 0 )
        _data->cursor = 0;
    
    else if( cursor > _data->byteArray.length )
        _data->cursor = _data->byteArray.length;
    
    else
        _data->cursor = cursor;
}




// DKData Interface ======================================================================

///
//  DKDataInitialize()
//
DKObjectRef DKDataInitialize( DKObjectRef _untyped_self )
{
    DKDataRef _self = DKSuperInit( _untyped_self, DKObjectClass() );

    if( _self )
    {
        DKByteArrayInit( &_self->byteArray );
        _self->cursor = 0;
    }
    
    return _self;
}


///
//  DKDataFinalize()
//
static void DKDataFinalize( DKObjectRef _untyped_self )
{
    DKDataRef _self = _untyped_self;
    DKByteArrayFinalize( &_self->byteArray );
}


///
//  DKDataInitWithBytes()
//
DKDataRef DKDataInitWithBytes( DKObjectRef _untyped_self, const void * bytes, DKIndex length )
{
    DKDataRef _self = DKInit( _untyped_self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        DKByteArrayAppendBytes( (DKByteArray *)&_self->byteArray, bytes, length );
    }
    
    return _self;
}


///
//  DKDataInitWithBytesNoCopy()
//
DKDataRef DKDataInitWithBytesNoCopy( DKObjectRef _untyped_self, const void * bytes, DKIndex length )
{
    DKDataRef _self = DKInit( _untyped_self );

    if( _self )
    {
        DKAssertMemberOfClass( _self, DKDataClass() );
        DKByteArrayInitWithExternalStorage( (DKByteArray *)&_self->byteArray, bytes, length );
    }
    
    return _self;
}


///
//  DKDataInitWithLength()
//
DKDataRef DKDataInitWithLength( DKObjectRef _untyped_self, DKIndex length )
{
    DKDataRef _self = DKInit( _untyped_self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        DKByteArrayAppendBytes( (DKByteArray *)&_self->byteArray, NULL, length );
    }
    
    return _self;
}


///
//  DKDataInitWithContentsOfFile()
//
DKDataRef DKDataInitWithContentsOfFile( DKObjectRef _untyped_self, DKObjectRef file )
{
    DKDataRef _self = DKInit( _untyped_self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        
        DKFileRef fileHandle = NULL;
        
        if( DKIsKindOfClass( file, DKStringClass() ) )
        {
            file = fileHandle = DKFileOpen( file, "rb" );
        }
        
        if( DKIsKindOfClass( file, DKFileClass() ) )
        {
            DKIndex length = DKFileGetLength( file );
        
            if( length > 0 )
            {
                DKByteArraySetLength( &_self->byteArray, length );

                void * buffer = DKByteArrayGetBytePtr( &_self->byteArray, 0 );
                DKFileRead( file, buffer, 1, length );
            }
            
            DKFileClose( fileHandle );
        }
        
        else if( file != NULL )
        {
            DKError( "DKDataInitWithContentsOfFile: '%@' is not a file.", file );
        }
    }
    
    return _self;
}


///
//  DKDataInitWithCapacity()
//
DKMutableDataRef DKDataInitWithCapacity( DKObjectRef _untyped_self, DKIndex capacity )
{
    DKMutableDataRef _self = DKInit( _untyped_self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableDataClass() );
        DKByteArrayReserve( &_self->byteArray, capacity );
    }
    
    return _self;
}


///
//  DKDataInitWithEgg()
//
static DKObjectRef DKDataInitWithEgg( DKObjectRef _untyped_self, DKEggUnarchiverRef egg )
{
    DKEncoding encoding = DKEggGetEncoding( egg, DKSTR( "data" ) );

    if( DKEncodingIsNumber( encoding ) )
    {
        DKDataRef _self = DKDataInitialize( _untyped_self );

        if( _self )
        {
            DKEncodingType encodingType = DKEncodingGetType( encoding );
            DKSetObjectTag( _self, encodingType );
            
            size_t length = DKEncodingGetSize( encoding );

            DKByteArraySetLength( &_self->byteArray, length );
            DKEggGetNumberData( egg, DKSTR( "data" ), DKByteArrayGetBytePtr( &_self->byteArray, 0 ) );
        }
        
        return _self;
    }
    
    else
    {
        size_t length = 0;
        const void * bytes = DKEggGetBinaryDataPtr( egg, DKSTR( "data" ), &length );

        return DKDataInitWithBytes( _untyped_self, bytes, length );
    }
}


///
//  DKDataAddToEgg()
//
static void DKDataAddToEgg( DKDataRef _self, DKEggArchiverRef egg )
{
    DKEncodingType encodingType = DKGetObjectTag( _self );

    if( DKEncodingTypeIsNumber( encodingType ) )
    {
        size_t size = DKEncodingTypeGetSize( encodingType );
        unsigned int count = (unsigned int)(_self->byteArray.length / size);
        DKAssert( (size_t)_self->byteArray.length == (size * count) );

        DKEncoding encoding = DKEncode( encodingType, count );

        DKEggAddNumberData( egg, DKSTR( "data" ), encoding, DKByteArrayGetBytePtr( &_self->byteArray, 0 ) );
    }

    else
    {
        DKIndex length = _self->byteArray.length;
        
        if( length > 0 )
            DKEggAddBinaryData( egg, DKSTR( "data" ), _self->byteArray.bytes, length );
    }
}


///
//  DKDataMakeImmutable()
//
DKDataRef DKDataMakeImmutable( DKMutableDataRef _self )
{
    if( DKIsMemberOfClass( _self, DKMutableDataClass() ) )
    {
        DKRelease( _self->_obj.isa );
        _self->_obj.isa = DKRetain( DKDataClass() );
    }
    
    return _self;
}


///
//  DKDataGetDescription()
//
DKStringRef DKDataGetDescription( DKDataRef _self )
{
    if( _self )
    {
        return DKStringWithFormat( "%@ (%" PRId64 " bytes)", DKGetClassName( _self ), _self->byteArray.length );
    }
    
    return NULL;
}


///
//  DKDataEqual()
//
bool DKDataEqual( DKDataRef _self, DKObjectRef other )
{
    DKAssertKindOfClass( _self, DKDataClass() );

    if( DKIsKindOfClass( _self, DKDataClass() ) )
        return DKDataCompare( _self, other ) == 0;
    
    return false;
}


///
//  DKDataCompare()
//
int DKDataCompare( DKDataRef _self, DKObjectRef other )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        
        // DKCompare requires that the objects have some strict ordering property useful
        // for comparison, yet has no way of checking if the objects actually meet that
        // requirement.
        if( DKIsKindOfClass( other, DKDataClass() ) )
        {
            DKDataRef otherData = other;
        
            DKIndex length1 = _self->byteArray.length;
            DKIndex length2 = otherData->byteArray.length;
            
            DKIndex length = (length1 < length2) ? length1 : length2;
            
            int cmp = memcmp( _self->byteArray.bytes, otherData->byteArray.bytes, length );
            
            if( cmp != 0 )
                return cmp;
            
            if( length1 < length2 )
                return -1;
            
            else if( length1 > length2 )
                return 1;
            
            return 0;
        }
    }
    
    return DKPointerCompare( _self, other );
}


///
//  DKDataHash()
//
DKHashCode DKDataHash( DKDataRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
    
        if( _self->byteArray.length > 0 )
            return dk_memhash( _self->byteArray.bytes, _self->byteArray.length );
    }
    
    return 0;
}


///
//  DKDataCopy()
//
DKDataRef DKDataCopy( DKDataRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        return DKDataInitWithBytes( DKAlloc( DKGetClass( _self ) ), _self->byteArray.bytes, _self->byteArray.length );
    }
    
    return NULL;
}


///
//  DKDataMutableCopy()
//
DKMutableDataRef DKDataMutableCopy( DKDataRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        return DKDataInitWithBytes( DKAlloc( DKMutableDataClass() ), _self->byteArray.bytes, _self->byteArray.length );
    }
    
    return NULL;
}


///
//  DKDataGetLength()
//
DKIndex DKDataGetLength( DKDataRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        return _self->byteArray.length;
    }
    
    return 0;
}


///
//  DKDataSetLength()
//
void DKDataSetLength( DKMutableDataRef _self, DKIndex length )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableDataClass() );

        if( length > _self->byteArray.length )
        {
            DKRange range = DKRangeMake( _self->byteArray.length, 0 );
            DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, length - _self->byteArray.length );
        }
        
        else if( length < _self->byteArray.length )
        {
            DKRange range = DKRangeMake( length, _self->byteArray.length - length );
            DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, 0 );
        }
    }
}


///
//  DKDataIncreaseLength()
//
void DKDataIncreaseLength( DKMutableDataRef _self, DKIndex length )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableDataClass() );

        DKRange range = DKRangeMake( _self->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, length );
    }
}


DKEncodingType DKDataGetEncodingType( DKDataRef _self )
{
    if( _self )
    {
        return DKGetObjectTag( _self );
    }
    
    return 0;
}


void DKDataSetEncodingType( DKDataRef _self, DKEncodingType type )
{
    if( _self )
    {
        DKSetObjectTag( _self, type );
    }
}


///
//  DKDataGetBytePtr()
//
const void * DKDataGetBytePtr( DKDataRef _self, DKIndex index )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        DKCheckIndex( index, _self->byteArray.length, NULL );
        
        return &_self->byteArray.bytes[index];
    }
    
    return NULL;
}


///
//  DKDataGetByteRange()
//
const void * DKDataGetByteRange( DKDataRef _self, DKRange range )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        DKCheckRange( range, _self->byteArray.length, NULL );

        // DKCheckRange allows a 0-length range at the end of the sequence
        if( range.location < _self->byteArray.length )
            return _self->byteArray.bytes + range.location;
    }
    
    return NULL;
}


///
//  DKDataGetByteEnd()
//
const void * DKDataGetByteEnd( DKDataRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        
        return &_self->byteArray.bytes[_self->byteArray.length];
    }
    
    return NULL;
}


///
//  DKDataGetMutableBytePtr()
//
void * DKDataGetMutableBytePtr( DKMutableDataRef _self, DKIndex index )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableDataClass(), NULL );
        DKCheckIndex( index, _self->byteArray.length, NULL );
        
        return &_self->byteArray.bytes[index];
    }
    
    return NULL;
}


///
//  DKDataGetMutableByteRange()
//
void * DKDataGetMutableByteRange( DKMutableDataRef _self, DKRange range )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableDataClass(), NULL );
        DKCheckRange( range, _self->byteArray.length, NULL );

        // DKCheckRange allows a 0-length range at the end of the sequence
        if( range.location < _self->byteArray.length )
            return _self->byteArray.bytes + range.location;
    }
    
    return NULL;
}


///
//  DKDataGetBytes()
//
DKIndex DKDataGetBytes( DKDataRef _self, DKRange range, void * buffer )
{
    if( buffer )
    {
        const void * src = DKDataGetByteRange( _self, range );

        if( src )
        {
            memcpy( buffer, src, range.length );
            return range.length;
        }
    }
    
    else
    {
        DKError( "DKDataGetBytes: Trying to copy to a NULL buffer." );
    }
    
    return 0;
}


///
//  DKDataAppendData()
//
void DKDataAppendData( DKMutableDataRef _self, DKDataRef data )
{
    if( _self && data )
    {
        DKCheckKindOfClass( _self, DKMutableDataClass() );
        DKCheckKindOfClass( data, DKDataClass() );

        DKRange range = DKRangeMake( _self->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &_self->byteArray, range, data->byteArray.bytes, data->byteArray.length );
    }
}


///
//  DKDataAppendBytes()
//
void DKDataAppendBytes( DKMutableDataRef _self, const void * bytes, DKIndex length )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableDataClass() );

        DKRange range = DKRangeMake( _self->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &_self->byteArray, range, bytes, length );
    }
}


///
//  DKDataReplaceBytes()
//
void DKDataReplaceBytes( DKMutableDataRef _self, DKRange range, const void * bytes, DKIndex length )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableDataClass() );
        DKCheckRange( range, _self->byteArray.length );

        DKByteArrayReplaceBytes( &_self->byteArray, range, bytes, length );
    }
}


///
//  DKDataDeleteBytes()
//
void DKDataDeleteBytes( DKMutableDataRef _self, DKRange range )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableDataClass() );
        DKCheckRange( range, _self->byteArray.length );

        DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, 0 );
    }
}


///
//  DKDataSeek()
//
int DKDataSeek( DKDataRef _self, long offset, int origin )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );

        DKIndex cursor = _self->cursor;
        
        if( origin == SEEK_SET )
            cursor = offset;
        
        else if( origin == SEEK_CUR )
            cursor += offset;
        
        else
            cursor = _self->byteArray.length + cursor;

        DKDataSetCursor( _self, cursor );
        
        return 0;
    }
    
    return -1;
}


///
//  DKDataTell()
//
long DKDataTell( DKDataRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );

        return (long)_self->cursor;
    }
    
    return -1;
}


///
//  DKDataRead()
//
size_t DKDataRead( DKDataRef _self, void * buffer, size_t size, size_t count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );

        DKDataSetCursor( _self, _self->cursor );
        
        DKRange range = DKRangeMake( _self->cursor, size * count );
        
        if( range.length > (_self->byteArray.length - _self->cursor) )
            range.length = _self->byteArray.length - _self->cursor;

        memcpy( buffer, &_self->byteArray.bytes[range.location], range.length );
        
        DKDataSetCursor( _self, _self->cursor + range.length );
        
        return range.length / size;
    }
    
    return 0;
}


///
//  DKDataWrite()
//
size_t DKDataWrite( DKMutableDataRef _self, const void * buffer, size_t size, size_t count )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableDataClass(), 0 );

        DKDataSetCursor( _self, _self->cursor );
        
        DKRange range = DKRangeMake( _self->cursor, size * count );
        
        if( range.length > (_self->byteArray.length - _self->cursor) )
            range.length = _self->byteArray.length - _self->cursor;
        
        DKByteArrayReplaceBytes( &_self->byteArray, range, buffer, size * count );

        DKDataSetCursor( _self, _self->cursor + (size * count) );
        
        return count;
    }
    
    return 0;
}




