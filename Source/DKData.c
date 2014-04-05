//
//  DKData.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#include "DKData.h"
#include "DKByteArray.h"
#include "DKCopying.h"
#include "DKStream.h"
#include "DKString.h"


struct DKData
{
    DKObject _obj;
    DKByteArray byteArray;
    DKIndex cursor;
};


static DKObjectRef  DKDataInitialize( DKObjectRef _self );
static void         DKDataFinalize( DKObjectRef _self );

static DKIndex DKImmutableDataWrite( DKMutableDataRef _self, const void * buffer, DKIndex size, DKIndex count );



// Class Methods =========================================================================

///
//  DKDataClass()
//
DKThreadSafeClassInit( DKDataClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKData" ), DKObjectClass(), sizeof(struct DKData), 0 );
    
    // Allocation
    struct DKAllocation * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(DKAllocation) );
    allocation->initialize = DKDataInitialize;
    allocation->finalize = DKDataFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    // Comparison
    struct DKComparison * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(DKComparison) );
    comparison->equal = (DKEqualMethod)DKDataEqual;
    comparison->compare = (DKCompareMethod)DKDataCompare;
    comparison->hash = (DKHashMethod)DKDataHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKDataCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Stream
    struct DKStream * stream =DKAllocInterface( DKSelector(Stream), sizeof(DKStream) );
    stream->seek = (DKStreamSeekMethod)DKDataSeek;
    stream->tell = (DKStreamTellMethod)DKDataTell;
    stream->read = (DKStreamReadMethod)DKDataRead;
    stream->write = (DKStreamWriteMethod)DKImmutableDataWrite;
    
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
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = (DKCopyMethod)DKDataCreateMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKDataCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Stream
    struct DKStream * stream = DKAllocInterface( DKSelector(Stream), sizeof(DKStream) );
    stream->seek = (DKStreamSeekMethod)DKDataSeek;
    stream->tell = (DKStreamTellMethod)DKDataTell;
    stream->read = (DKStreamReadMethod)DKDataRead;
    stream->write = (DKStreamWriteMethod)DKDataWrite;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
}


///
//  DKDataInitialize()
//
static DKObjectRef DKDataInitialize( DKObjectRef _self )
{
    struct DKData * data = (struct DKData *)_self;
    DKByteArrayInit( &data->byteArray );
    data->cursor = 0;
    
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

        return memcmp( _self->byteArray.data, other->byteArray.data, _self->byteArray.length );
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
            return dk_memhash( _self->byteArray.data, _self->byteArray.length );
    }
    
    return 0;
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
//  DKDataCreate()
//
DKDataRef DKDataCreate( void )
{
    return DKAllocObject( DKDataClass(), 0 );
}


///
//  DKDataCreateCopy()
//
DKDataRef DKDataCreateCopy( DKDataRef src )
{
    const void * srcBytes = NULL;
    DKIndex srcLength = 0;

    if( src )
    {
        DKAssertKindOfClass( src, DKDataClass() );

        srcBytes = DKDataGetBytePtr( src );
        srcLength = DKDataGetLength( src );
    }
    
    return DKDataCreateWithBytes( srcBytes, srcLength );
}


///
//  DKDataCreateWithBytes()
//
DKDataRef DKDataCreateWithBytes( const void * bytes, DKIndex length )
{
    if( bytes && (length > 0) )
    {
        struct DKData * data = (struct DKData *)DKAllocObject( DKDataClass(), length );
        
        DKByteArrayInitWithExternalStorage( &data->byteArray, (void *)(data + 1), length );
        
        memcpy( data->byteArray.data, bytes, length );
        
        return data;
    }
    
    return DKDataCreate();
}


///
//  DKDataCreateWithBytesNoCopy()
//
DKDataRef DKDataCreateWithBytesNoCopy( const void * bytes, DKIndex length )
{
    if( bytes && (length > 0) )
    {
        struct DKData * data = (struct DKData *)DKAllocObject( DKDataClass(), 0 );
        
        DKByteArrayInitWithExternalStorage( &data->byteArray, bytes, length );
        
        return data;
    }
    
    return DKDataCreate();
}


///
//  DKDataCreateMutable()
//
DKMutableDataRef DKDataCreateMutable( void )
{
    return (DKMutableDataRef)DKAllocObject( DKMutableDataClass(), 0 );
}


///
//  DKDataCreateMutableCopy()
//
DKMutableDataRef DKDataCreateMutableCopy( DKDataRef src )
{
    struct DKData * data = (struct DKData *)DKDataCreateMutable();

    const void * srcBytes = DKDataGetBytePtr( src );
    DKIndex srcLength = DKDataGetLength( src );

    DKByteArrayReplaceBytes( &data->byteArray, DKRangeMake( 0, 0 ), srcBytes, srcLength );

    return data;
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
            return _self->byteArray.data;
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
            return _self->byteArray.data + range.location;
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
            return _self->byteArray.data;
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
            return _self->byteArray.data + range.location;
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

        memcpy( buffer, &data->byteArray.data[range.location], range.length );
        
        SetCursor( data, data->cursor + range.length );
        
        return range.length / size;
    }
    
    return 0;
}


///
//  DKDataWrite()
//
static DKIndex DKImmutableDataWrite( DKMutableDataRef _self, const void * buffer, DKIndex size, DKIndex count )
{
    DKError( "DKDataWrite: Trying to modify an immutable object." );
    return 0;
}

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



