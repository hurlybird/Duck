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


static DKTypeRef    DKDataInitialize( DKTypeRef ref );
static void         DKDataFinalize( DKTypeRef ref );




// Class Methods =========================================================================

///
//  DKDataClass()
//
DKTypeRef DKDataClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKData", DKObjectClass(), sizeof(struct DKData) );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->initialize = DKDataInitialize;
        lifeCycle->finalize = DKDataFinalize;

        DKInstallInterface( SharedClassObject, lifeCycle );
        DKRelease( lifeCycle );

        // Comparison
        struct DKComparison * comparison = (struct DKComparison *)DKCreateInterface( DKSelector(Comparison), sizeof(DKComparison) );
        comparison->equal = DKDataEqual;
        comparison->compare = DKDataCompare;
        comparison->hash = DKDataHash;

        DKInstallInterface( SharedClassObject, comparison );
        DKRelease( comparison );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKRetain;
        copying->mutableCopy = DKDataCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );

        // Stream
        struct DKStream * stream = (struct DKStream *)DKCreateInterface( DKSelector(Stream), sizeof(DKStream) );
        stream->seek = DKDataSeek;
        stream->tell = DKDataTell;
        stream->read = DKDataRead;
        stream->write = DKDataWrite;
        
        DKInstallInterface( SharedClassObject, stream );
        DKRelease( stream );
    }
    
    return SharedClassObject;
}


///
//  DKMutableDataClass()
//
DKTypeRef DKMutableDataClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKMutableData", DKDataClass(), sizeof(struct DKData) );
        
        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKDataCreateMutableCopy;
        copying->mutableCopy = DKDataCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );
        
        // Stream
        struct DKStream * stream = (struct DKStream *)DKCreateInterface( DKSelector(Stream), sizeof(DKStream) );
        stream->seek = DKDataSeek;
        stream->tell = DKDataTell;
        stream->read = DKDataRead;
        stream->write = DKDataWrite;
        
        DKInstallInterface( SharedClassObject, stream );
        DKRelease( stream );
    }
    
    return SharedClassObject;
}


///
//  DKDataInitialize()
//
static DKTypeRef DKDataInitialize( DKTypeRef ref )
{
    struct DKData * data = (struct DKData *)ref;
    DKByteArrayInit( &data->byteArray );
    data->cursor = 0;
    
    return ref;
}


///
//  DKDataFinalize()
//
static void DKDataFinalize( DKTypeRef ref )
{
    struct DKData * data = (struct DKData *)ref;
    DKByteArrayFinalize( &data->byteArray );
}


///
//  DKDataEqual()
//
int DKDataEqual( DKDataRef a, DKTypeRef b )
{
    if( DKIsKindOfClass( b, DKDataClass() ) )
        return DKDataCompare( a, b ) == 0;
    
    return 0;
}


///
//  DKDataCompare()
//
int DKDataCompare( DKDataRef a, DKDataRef b )
{
    if( a )
    {
        DKVerifyKindOfClass( a, DKDataClass(), DKDefaultCompare( a, b ) );
        DKVerifyKindOfClass( b, DKDataClass(), DKDefaultCompare( a, b ) );

        const struct DKData * da = a;
        const struct DKData * db = b;
        
        if( da->byteArray.length < db->byteArray.length )
            return 1;
        
        if( da->byteArray.length > db->byteArray.length )
            return -1;
        
        if( da->byteArray.length == 0 )
            return 0;

        return memcmp( da->byteArray.data, db->byteArray.data, da->byteArray.length );
    }
    
    return DKDefaultCompare( a, b );
}


///
//  DKDataHash()
//
DKHashCode DKDataHash( DKDataRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKDataClass(), 0 );
    
        const struct DKData * data = ref;
        
        if( data->byteArray.length > 0 )
            return dk_memhash( data->byteArray.data, data->byteArray.length );
    }
    
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
        data->cursor = cursor;
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
        DKVerifyKindOfClass( src, DKDataClass(), NULL );

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
DKIndex DKDataGetLength( DKDataRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKDataClass(), 0 );

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
        DKVerifyKindOfClass( ref, DKMutableDataClass() );

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
        DKVerifyKindOfClass( ref, DKMutableDataClass() );

        struct DKData * data = (struct DKData *)ref;

        DKRange range = DKRangeMake( data->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &data->byteArray, range, NULL, length );
    }
}


///
//  DKDataGetBytePtr()
//
const void * DKDataGetBytePtr( DKDataRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKDataClass(), NULL );

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
        DKVerifyKindOfClass( ref, DKDataClass(), NULL );

        const struct DKData * data = ref;

        DKVerifyRange( range, data->byteArray.length, NULL );

        // DKVerifyRange allows a 0-length range at the end of the sequence
        if( range.location < data->byteArray.length )
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
        DKVerifyKindOfClass( ref, DKMutableDataClass(), NULL );

        const struct DKData * data = ref;
        
        if( data->byteArray.length > 0 )
            return data->byteArray.data;
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
        DKVerifyKindOfClass( ref, DKMutableDataClass(), NULL );

        const struct DKData * data = ref;

        DKVerifyRange( range, data->byteArray.length, NULL );

        // DKVerifyRange allows a 0-length range at the end of the sequence
        if( range.location < data->byteArray.length )
            return data->byteArray.data + range.location;
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
        DKVerifyKindOfClass( ref, DKMutableDataClass() );

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
        DKVerifyKindOfClass( ref, DKMutableDataClass() );

        struct DKData * data = (struct DKData *)ref;

        DKVerifyRange( range, data->byteArray.length );

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
        DKVerifyKindOfClass( ref, DKMutableDataClass() );

        struct DKData * data = (struct DKData *)ref;

        DKVerifyRange( range, data->byteArray.length );

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
        DKVerifyKindOfClass( ref, DKDataClass(), -1 );

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
        DKVerifyKindOfClass( ref, DKDataClass(), -1 );

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
        DKVerifyKindOfClass( ref, DKDataClass(), 0 );

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
        DKVerifyKindOfClass( ref, DKMutableDataClass(), 0 );

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




