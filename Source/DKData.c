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


struct DKData
{
    DKObject _obj;
    DKByteArray byteArray;
    DKIndex cursor;
};


static DKObjectRef DKDataInitialize( DKObjectRef _self );
static void        DKDataFinalize( DKObjectRef _self );



// Class Methods =========================================================================

///
//  DKDataClass()
//
DKThreadSafeClassInit( DKDataClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKData" ), DKObjectClass(), sizeof(struct DKData), 0 );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->initialize = DKDataInitialize;
    allocation->finalize = DKDataFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    // Comparison
    struct DKComparisonInterface * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualMethod)DKDataEqual;
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
    
    return cls;
}


///
//  DKMutableDataClass()
//
DKThreadSafeClassInit( DKMutableDataClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKMutableData" ), DKDataClass(), sizeof(struct DKData), 0 );
    
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
//  DKDataEqual()
//
int DKDataEqual( DKDataRef _self, DKObjectRef other )
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
//  DKDataCreateWithBytes()
//
DKDataRef DKDataCreateWithBytes( DKClassRef _class, const void * bytes, DKIndex length )
{
    DKAssert( (_class == NULL) || DKIsSubclass( _class, DKDataClass() ) );

    struct DKData * data = NULL;
    
    if( _class == DKMutableDataClass() )
    {
        data = DKCreate( DKMutableDataClass() );
        
        DKByteArrayAppendBytes( &data->byteArray, bytes, length );
        
        return data;
    }

    else
    {
        data = DKAllocObject( DKDataClass(), length );
        data = DKInitializeObject( data );
    
        if( bytes && (length > 0) )
        {
            DKByteArrayInitWithExternalStorage( &data->byteArray, (void *)(data + 1), length );
            memcpy( data->byteArray.bytes, bytes, length );
        }
        
        return data;
    }
}


///
//  DKDataCreateWithBytesNoCopy()
//
DKDataRef DKDataCreateWithBytesNoCopy( /* DKClassRef _class, */ const void * bytes, DKIndex length )
{
    if( bytes && (length > 0) )
    {
        struct DKData * data = (struct DKData *)DKAllocObject( DKDataClass(), 0 );
        data = DKInitializeObject( data );
        
        DKByteArrayInitWithExternalStorage( &data->byteArray, bytes, length );
        
        return data;
    }
    
    return DKDataCreateEmpty();
}


///
//  DKDataCopy()
//
DKDataRef DKDataCopy( DKDataRef _self )
{
    if( _self )
        return DKDataCreateWithBytes( DKGetClass( _self ), _self->byteArray.bytes, _self->byteArray.length );
    
    return NULL;
}


///
//  DKDataMutableCopy()
//
DKMutableDataRef DKDataMutableCopy( DKDataRef _self )
{
    if( _self )
        return (DKMutableDataRef)DKDataCreateWithBytes( DKMutableDataClass(), _self->byteArray.bytes, _self->byteArray.length );
    
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
const void * DKDataGetBytePtr( DKDataRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKDataClass() );
        
        if( _self->byteArray.length > 0 )
            return _self->byteArray.bytes;
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
void * DKDataGetMutableBytePtr( DKMutableDataRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableDataClass() );
        
        if( _self->byteArray.length > 0 )
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




