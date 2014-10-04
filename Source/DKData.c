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

#include "DKData.h"
#include "DKByteArray.h"
#include "DKStream.h"
#include "DKString.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKEgg.h"


struct DKData
{
    DKObject _obj;
    DKByteArray byteArray;
    DKIndex cursor;
};


static DKObjectRef  DKDataInitialize( DKObjectRef _self );
static void         DKDataFinalize( DKObjectRef _self );

static DKObjectRef  DKDataInitWithEgg( DKDataRef _self, DKEggUnarchiverRef egg );
static void         DKDataAddToEgg( DKDataRef _self, DKEggArchiverRef egg );




// Class Methods =========================================================================

///
//  DKDataClass()
//
DKThreadSafeClassInit( DKDataClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKData" ), DKObjectClass(), sizeof(struct DKData), 0, DKDataInitialize, DKDataFinalize );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKDataEqual;
    comparison->like = (DKEqualityMethod)DKDataEqual;
    comparison->compare = (DKCompareMethod)DKDataCompare;
    comparison->hash = (DKHashMethod)DKDataHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKDataMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Stream
    struct DKStreamInterface * stream =DKAllocInterface( DKSelector(Stream), sizeof(struct DKStreamInterface) );
    stream->seek = (DKStreamSeekMethod)DKDataSeek;
    stream->tell = (DKStreamTellMethod)DKDataTell;
    stream->read = (DKStreamReadMethod)DKDataRead;
    stream->write = (void *)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    // Egg
    struct DKEggInterface * egg = DKAllocInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKDataInitWithEgg;
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
    DKClassRef cls = DKAllocClass( DKSTR( "DKMutableData" ), DKDataClass(), sizeof(struct DKData), 0, NULL, NULL );
    
    // Copying
    struct DKCopyingInterface * copying = DKAllocInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = (DKCopyMethod)DKDataMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKDataMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Stream
    struct DKStreamInterface * stream = DKAllocInterface( DKSelector(Stream), sizeof(struct DKStreamInterface) );
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
//  SetCursor()
//
static void SetCursor( const struct DKData * data, DKIndex cursor )
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
DKObjectRef DKDataInitialize( DKObjectRef _self )
{
    _self = DKSuperInit( _self, DKObjectClass() );

    if( _self )
    {
        struct DKData * data = (struct DKData *)_self;
        DKByteArrayInit( &data->byteArray );
        data->cursor = 0;
    }
    
    return _self;
}


///
//  DKDataFinalize()
//
static void DKDataFinalize( DKObjectRef _self )
{
    struct DKData * data = (struct DKData *)_self;
    DKByteArrayFinalize( &data->byteArray );
}


///
//  DKDataInitWithBytes()
//
DKDataRef DKDataInitWithBytes( DKDataRef _self, const void * bytes, DKIndex length )
{
    _self = DKInit( _self );

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
DKDataRef DKDataInitWithBytesNoCopy( DKDataRef _self, const void * bytes, DKIndex length )
{
    _self = DKInit( _self );

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
DKDataRef DKDataInitWithLength( DKDataRef _self, DKIndex length )
{
    _self = DKInit( _self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        DKByteArrayAppendBytes( (DKByteArray *)&_self->byteArray, NULL, length );
    }
    
    return _self;
}


///
//  DKDataInitWithCapacity()
//
DKMutableDataRef DKDataInitWithCapacity( DKMutableDataRef _self, DKIndex capacity )
{
    _self = DKInit( _self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableDataClass() );
        DKByteArrayReserve( (DKByteArray *)&_self->byteArray, capacity );
    }
    
    return _self;
}


///
//  DKDataInitWithEgg()
//
static DKObjectRef DKDataInitWithEgg( DKDataRef _self, DKEggUnarchiverRef egg )
{
    size_t length = 0;
    const void * bytes = DKEggGetBinaryDataPtr( egg, DKSTR( "data" ), &length );

    return DKDataInitWithBytes( _self, bytes, length );
}


///
//  DKDataAddToEgg()
//
static void DKDataAddToEgg( DKDataRef _self, DKEggArchiverRef egg )
{
    DKIndex length = _self->byteArray.length;
    
    if( length > 0 )
        DKEggAddBinaryData( egg, DKSTR( "data" ), _self->byteArray.bytes, length );
}


///
//  DKDataEqual()
//
bool DKDataEqual( DKDataRef _self, DKObjectRef other )
{
    DKAssertKindOfClass( _self, DKDataClass() );

    if( DKIsKindOfClass( _self, DKDataClass() ) )
        return DKDataCompare( _self, other ) == 0;
    
    return 0;
}


///
//  DKDataCompare()
//
int DKDataCompare( DKDataRef _self, DKDataRef other )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        
        // DKCompare requires that the objects have some strict ordering property useful
        // for comparison, yet has no way of checking if the objects actually meet that
        // requirement.
        DKCheckKindOfClass( other, DKDataClass(), DKPointerCompare( _self, other ) );

        if( _self->byteArray.length < other->byteArray.length )
            return 1;
        
        if( _self->byteArray.length > other->byteArray.length )
            return -1;
        
        if( _self->byteArray.length == 0 )
            return 0;

        return memcmp( _self->byteArray.bytes, other->byteArray.bytes, _self->byteArray.length );
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
        return DKDataCreateWithBytes( DKGetClass( _self ), _self->byteArray.bytes, _self->byteArray.length );
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
        return (DKMutableDataRef)DKDataCreateWithBytes( DKMutableDataClass(), _self->byteArray.bytes, _self->byteArray.length );
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
        DKAssertKindOfClass( _self, DKMutableDataClass() );

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
        DKAssertKindOfClass( _self, DKMutableDataClass() );

        DKRange range = DKRangeMake( _self->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, length );
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
//  DKDataGetMutableBytePtr()
//
void * DKDataGetMutableBytePtr( DKMutableDataRef _self, DKIndex index )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableDataClass() );
        DKCheckIndex( index, _self->byteArray.length, NULL );
        
        return _self->byteArray.bytes;
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
        DKAssertKindOfClass( _self, DKMutableDataClass() );
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
//  DKDataAppendBytes()
//
void DKDataAppendBytes( DKMutableDataRef _self, const void * bytes, DKIndex length )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableDataClass() );

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
        DKAssertKindOfClass( _self, DKMutableDataClass() );
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
        DKAssertKindOfClass( _self, DKMutableDataClass() );
        DKCheckRange( range, _self->byteArray.length );

        DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, 0 );
    }
}


///
//  DKDataSeek()
//
int DKDataSeek( DKDataRef _self, DKIndex offset, int origin )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );

        struct DKData * data = (struct DKData *)_self;
        
        DKIndex cursor = data->cursor;
        
        if( origin == DKSeekSet )
            cursor = offset;
        
        else if( origin == DKSeekCur )
            cursor += offset;
        
        else
            cursor = data->byteArray.length + cursor;

        SetCursor( data, cursor );
        
        return 0;
    }
    
    return -1;
}


///
//  DKDataTell()
//
DKIndex DKDataTell( DKDataRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );

        struct DKData * data = (struct DKData *)_self;
        return data->cursor;
    }
    
    return -1;
}


///
//  DKDataRead()
//
DKIndex DKDataRead( DKDataRef _self, void * buffer, DKIndex size, DKIndex count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );

        struct DKData * data = (struct DKData *)_self;

        SetCursor( data, data->cursor );
        
        DKRange range = DKRangeMake( data->cursor, size * count );
        
        if( range.length > (data->byteArray.length - data->cursor) )
            range.length = data->byteArray.length - data->cursor;

        memcpy( buffer, &data->byteArray.bytes[range.location], range.length );
        
        SetCursor( data, data->cursor + range.length );
        
        return range.length / size;
    }
    
    return 0;
}


///
//  DKDataWrite()
//
DKIndex DKDataWrite( DKMutableDataRef _self, const void * buffer, DKIndex size, DKIndex count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableDataClass() );

        struct DKData * data = (struct DKData *)_self;

        SetCursor( data, data->cursor );
        
        DKRange range = DKRangeMake( data->cursor, size * count );
        
        if( range.length > (data->byteArray.length - data->cursor) )
            range.length = data->byteArray.length - data->cursor;
        
        DKByteArrayReplaceBytes( &data->byteArray, range, buffer, size * count );

        SetCursor( data, data->cursor + (size * count) );
        
        return count;
    }
    
    return 0;
}




