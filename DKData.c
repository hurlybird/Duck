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


struct DKData
{
    DKObjectHeader _obj;
    DKByteArray byteArray;
    DKIndex cursor;
};


static DKTypeRef    DKDataAllocate( void );
static DKTypeRef    DKMutableDataAllocate( void );
static DKTypeRef    DKDataInitialize( DKTypeRef ref );
static void         DKDataFinalize( DKTypeRef ref );
static int          DKDataEqual( DKTypeRef a, DKTypeRef b );
static int          DKDataCompare( DKTypeRef a, DKTypeRef b );
static DKHashIndex  DKDataHash( DKTypeRef ref );



// Class Methods =========================================================================

///
//  DKDataClass()
//
DKTypeRef DKDataClass( void )
{
    static DKTypeRef dataClass = NULL;

    if( !dataClass )
    {
        dataClass = DKCreateClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKDataAllocate;
        lifeCycle->initialize = DKDataInitialize;
        lifeCycle->finalize = DKDataFinalize;

        DKInstallInterface( dataClass, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKRetain;
        copying->mutableCopy = DKDataCreateMutableCopy;
        
        DKInstallInterface( dataClass, copying );
        DKRelease( copying );

        // Stream
        struct DKStream * stream = (struct DKStream *)DKCreateInterface( DKSelector(Stream), sizeof(DKStream) );
        stream->seek = DKDataSeek;
        stream->tell = DKDataTell;
        stream->read = DKDataRead;
        stream->write = DKDataWrite;
        
        DKInstallInterface( dataClass, stream );
        DKRelease( stream );
    }
    
    return dataClass;
}


///
//  DKMutableDataClass()
//
DKTypeRef DKMutableDataClass( void )
{
    static DKTypeRef mutableDataClass = NULL;

    if( !mutableDataClass )
    {
        mutableDataClass = DKCreateClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKMutableDataAllocate;
        lifeCycle->initialize = DKDataInitialize;
        lifeCycle->finalize = DKDataFinalize;

        DKInstallInterface( mutableDataClass, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKDataCreateMutableCopy;
        copying->mutableCopy = DKDataCreateMutableCopy;
        
        DKInstallInterface( mutableDataClass, copying );
        DKRelease( copying );
        
        // Stream
        struct DKStream * stream = (struct DKStream *)DKCreateInterface( DKSelector(Stream), sizeof(DKStream) );
        stream->seek = DKDataSeek;
        stream->tell = DKDataTell;
        stream->read = DKDataRead;
        stream->write = DKDataWrite;
        
        DKInstallInterface( mutableDataClass, stream );
        DKRelease( stream );
    }
    
    return mutableDataClass;
}


///
//  DKDataAllocate()
//
static DKTypeRef DKDataAllocate( void )
{
    return DKAllocObject( DKDataClass(), sizeof(struct DKData), 0 );
}


///
//  DKMutableDataAllocate()
//
static DKTypeRef DKMutableDataAllocate( void )
{
    return DKAllocObject( DKMutableDataClass(), sizeof(struct DKData), DKObjectIsMutable );
}


///
//  DKDataInitialize()
//
static DKTypeRef DKDataInitialize( DKTypeRef ref )
{
    ref = DKObjectInitialize( ref );
    
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;
        DKByteArrayInit( &data->byteArray );
        data->cursor = 0;
    }
    
    return ref;
}


///
//  DKDataFinalize()
//
static void DKDataFinalize( DKTypeRef ref )
{
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;
        
        if( !DKTestObjectAttribute( data, (DKObjectContentIsInline | DKObjectContentIsExternal) ) )
        {
            DKByteArrayClear( &data->byteArray );
        }
    }
}


///
//  DKDataEqual()
//
static int DKDataEqual( DKTypeRef a, DKTypeRef b )
{
    return DKDataCompare( a, b ) == 0;
}


///
//  DKDataCompare()
//
static int DKDataCompare( DKTypeRef a, DKTypeRef b )
{
    DKTypeRef btype = DKGetClass( b );
    
    if( (btype == DKDataClass()) || (btype == DKMutableDataClass()) )
    {
        const struct DKData * da = a;
        const struct DKData * db = b;
        
        if( da->byteArray.length < db->byteArray.length )
            return -1;
        
        if( da->byteArray.length > db->byteArray.length )
            return 1;
        
        if( da->byteArray.length == 0 )
            return 0;

        return memcmp( da->byteArray.data, db->byteArray.data, da->byteArray.length );
    }
    
    return DKDefaultCompare( a, b );
}


///
//  DKDataHash()
//
static DKHashIndex DKDataHash( DKTypeRef ref )
{
    const struct DKData * data = ref;
    
    if( data->byteArray.length > 0 )
        return DKMemHash( data->byteArray.data, data->byteArray.length );
    
    return 0;
}


///
//  DKDataUpdateCursor()
//
static void DKDataSetCursor( DKDataRef ref, DKIndex cursor )
{
    struct DKData * data = (struct DKData *)ref;
    
    if( cursor < 0 )
        data->cursor = 0;
    
    else if( cursor > data->byteArray.length )
        data->cursor = data->byteArray.length;
    
    else
        data = cursor;
}




// DKData Interface ======================================================================

///
//  DKDataCreate()
//
DKDataRef DKDataCreate( const void * bytes, DKIndex length )
{
    DKDataRef ref = DKAllocObject( DKDataClass(), sizeof(struct DKData) + length, DKObjectContentIsInline );

    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;
        
        DKByteArrayInit( &data->byteArray );

        data->byteArray.data = (void *)(data + 1);
        data->byteArray.length = length;
        data->byteArray.maxLength = length;
        
        memcpy( data->byteArray.data, bytes, length );
    }
    
    return ref;
}


///
//  DKDataCreateCopy()
//
DKDataRef DKDataCreateCopy( DKDataRef src )
{
    if( src )
    {
        const void * srcBytes = DKDataGetBytePtr( src );
        DKIndex srcLength = DKDataGetLength( src );
        
        return DKDataCreate( srcBytes, srcLength );
    }
    
    else
    {
        return DKDataCreate( NULL, 0 );
    }
}


///
//  DKDataCreateWithBytesNoCopy()
//
DKDataRef DKDataCreateWithBytesNoCopy( const void * bytes, DKIndex length )
{
    DKDataRef ref = DKAllocObject( DKDataClass(), sizeof(struct DKData), DKObjectContentIsExternal );

    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;
        
        DKByteArrayInit( &data->byteArray );

        data->byteArray.data = (void *)bytes;
        data->byteArray.length = length;
        data->byteArray.maxLength = length;
    }
    
    return ref;
}


///
//  DKDataCreateMutable()
//
DKMutableDataRef DKDataCreateMutable( void )
{
    return (DKMutableDataRef)DKCreate( DKMutableDataClass() );
}


///
//  DKDataCreateMutableCopy()
//
DKMutableDataRef DKDataCreateMutableCopy( DKDataRef src )
{
    struct DKData * data = (struct DKData *)DKDataCreateMutable();

    if( src )
    {
        const void * srcBytes = DKDataGetBytePtr( src );
        DKIndex srcLength = DKDataGetLength( src );

        DKByteArrayReplaceBytes( &data->byteArray, DKRangeMake( 0, 0 ), srcBytes, srcLength );
    }

    return data;
}


///
//  DKDataGetLength()
//
DKIndex DKDataGetLength( DKDataRef ref )
{
    if( ref )
    {
        const struct DKData * data = ref;
        return data->byteArray.length;
    }
    
    return 0;
}


///
//  DKDataSetLength()
//
void DKDataSetLength( DKMutableDataRef ref, DKIndex length )
{
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;

        if( length > data->byteArray.length )
        {
            DKRange range = DKRangeMake( data->byteArray.length, 0 );
            DKByteArrayReplaceBytes( &data->byteArray, range, NULL, length - data->byteArray.length );
        }
        
        else if( length < data->byteArray.length )
        {
            DKRange range = DKRangeMake( length, data->byteArray.length - length );
            DKByteArrayReplaceBytes( &data->byteArray, range, NULL, 0 );
            
            DKDataSetCursor( ref, data->cursor );
        }
    }
}


///
//  DKDataIncreaseLength()
//
void DKDataIncreaseLength( DKMutableDataRef ref, DKIndex length )
{
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;
        DKDataSetLength( data, data->byteArray.length + length );
    }
}


///
//  DKDataGetBytePtr()
//
const void * DKDataGetBytePtr( DKDataRef ref )
{
    if( ref )
    {
        const struct DKData * data = ref;
        
        if( data->byteArray.length > 0 )
            return data->byteArray.data;
    }
    
    return NULL;
}


///
//  DKDataGetByteRange()
//
const void * DKDataGetByteRange( DKDataRef ref, DKRange range )
{
    if( ref )
    {
        const struct DKData * data = ref;

        if( (range.location < data->byteArray.length) && (DKRangeEnd( range ) <= data->byteArray.length) )
            return data->byteArray.data + range.location;
    }
    
    return NULL;
}


///
//  DKDataGetMutableBytePtr()
//
void * DKDataGetMutableBytePtr( DKMutableDataRef ref )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKDataGetMutableBytePtr: Trying to modify an immutable object." );
            return NULL;
        }
        
        return (void *)DKDataGetBytePtr( ref );
    }
    
    return NULL;
}


///
//  DKDataGetMutableByteRange()
//
void * DKDataGetMutableByteRange( DKMutableDataRef ref, DKRange range )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKDataGetMutableByteRange: Trying to modify an immutable object." );
            return NULL;
        }
        
        return (void *)DKDataGetByteRange( ref, range );
    }
    
    return NULL;
}


///
//  DKDataGetBytes()
//
DKIndex DKDataGetBytes( DKDataRef ref, DKRange range, void * buffer )
{
    if( buffer )
    {
        const void * src = DKDataGetByteRange( ref, range );
        
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
void DKDataAppendBytes( DKMutableDataRef ref, const void * bytes, DKIndex length )
{
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;

        DKRange range = DKRangeMake( data->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &data->byteArray, range, bytes, length );
    }
}


///
//  DKDataReplaceBytes()
//
void DKDataReplaceBytes( DKMutableDataRef ref, DKRange range, const void * bytes, DKIndex length )
{
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;

        DKByteArrayReplaceBytes( &data->byteArray, range, bytes, length );
        DKDataSetCursor( data, data->cursor );
    }
}


///
//  DKDataDeleteBytes()
//
void DKDataDeleteBytes( DKMutableDataRef ref, DKRange range )
{
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;

        DKByteArrayReplaceBytes( &data->byteArray, range, NULL, 0 );
        DKDataSetCursor( data, data->cursor );
    }
}


///
//  DKDataSeek()
//
int DKDataSeek( DKDataRef ref, DKIndex offset, int origin )
{
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;
        
        DKIndex cursor = data->cursor;
        
        if( origin == DKSeekSet )
            cursor = offset;
        
        else if( origin == DKSeekCur )
            cursor += offset;
        
        else
            cursor = data->byteArray.length + cursor;

        DKDataSetCursor( data, cursor );
    }
    
    return -1;
}


///
//  DKDataTell()
//
DKIndex DKDataTell( DKDataRef ref )
{
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;
        return data->cursor;
    }
    
    return -1;
}


///
//  DKDataRead()
//
DKIndex DKDataRead( DKDataRef ref, void * buffer, DKIndex size, DKIndex count )
{
    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;

        DKAssert( (data->cursor >= 0) && (data->cursor <= data->byteArray.length) );
        
        DKRange range = DKRangeMake( data->cursor, size * count );
        
        if( range.length > (data->byteArray.length - data->cursor) )
            range.length = data->byteArray.length - data->cursor;
        
        DKDataGetBytes( data, range, buffer );
        
        return range.length / size;
    }
    
    return 0;
}


///
//  DKDataWrite()
//
DKIndex DKDataWrite( DKDataRef ref, const void * buffer, DKIndex size, DKIndex count )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKDataWrite: Trying to modify an immutable object." );
            return 0;
        }

        struct DKData * data = (struct DKData *)ref;

        DKAssert( (data->cursor >= 0) && (data->cursor <= data->byteArray.length) );
        
        DKRange range = DKRangeMake( data->cursor, size * count );
        
        if( range.length > (data->byteArray.length - data->cursor) )
            range.length = data->byteArray.length - data->cursor;
        
        DKDataReplaceBytes( data, range, buffer, size * count );
        
        return count;
    }
    
    return 0;
}




