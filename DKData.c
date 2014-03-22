//
//  DKData.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#include "DKData.h"
#include "DKCommonInterfaces.h"
#include "DKCopying.h"


struct DKData
{
    DKObjectHeader _obj;
    DKByteArray byteArray;
};


static DKTypeRef    DKDataAllocate( void );
static DKTypeRef    DKMutableDataAllocate( void );
static DKTypeRef    DKDataInitialize( DKTypeRef ref );
static void         DKDataFinalize( DKTypeRef ref );
static DKTypeRef    DKDataCopy( DKTypeRef ref );
static DKTypeRef    DKMutableDataCopy( DKTypeRef ref );
static DKTypeRef    DKDataMutableCopy( DKTypeRef ref );
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
        dataClass = DKAllocClass( DKObjectClass() );
        
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKDataAllocate;
        lifeCycle->initialize = DKDataInitialize;
        lifeCycle->finalize = DKDataFinalize;

        DKInstallInterface( dataClass, lifeCycle );
        DKRelease( lifeCycle );

        struct DKCopying * copying = (struct DKCopying *)DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKDataCopy;
        copying->mutableCopy = DKDataMutableCopy;
        
        DKInstallInterface( dataClass, copying );
        DKRelease( copying );
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
        mutableDataClass = DKAllocClass( DKObjectClass() );
        
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKMutableDataAllocate;
        lifeCycle->initialize = DKDataInitialize;
        lifeCycle->finalize = DKDataFinalize;

        DKInstallInterface( mutableDataClass, lifeCycle );
        DKRelease( lifeCycle );

        struct DKCopying * copying = (struct DKCopying *)DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKMutableDataCopy;
        copying->mutableCopy = DKDataMutableCopy;
        
        DKInstallInterface( mutableDataClass, copying );
        DKRelease( copying );
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
//  DKDataCopy()
//
static DKTypeRef DKDataCopy( DKTypeRef ref )
{
    return DKRetain( ref );
}


///
//  DKMutableDataCopy()
//
static DKTypeRef DKMutableDataCopy( DKTypeRef ref )
{
    return DKDataCreateCopy( ref );
}


///
//  DKDataMutableCopy()
//
static DKTypeRef DKDataMutableCopy( DKTypeRef ref )
{
    return DKDataCreateMutableCopy( ref );
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
        DKDataRef da = a;
        DKDataRef db = b;
        
        if( da->byteArray.length < db->byteArray.length )
            return -1;
        
        if( da->byteArray.length > db->byteArray.length )
            return 1;
        
        if( da->byteArray.length == 0 )
            return 0;

        return memcmp( da->byteArray.data, db->byteArray.data, da->byteArray.length );
    }
    
    return DKDefaultCompareImp( a, b );
}


///
//  DKDataHash()
//
static DKHashIndex DKDataHash( DKTypeRef ref )
{
    DKDataRef data = ref;
    
    if( data->byteArray.length > 0 )
        return DKMemHash( data->byteArray.data, data->byteArray.length );
    
    return 0;
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
    DKMutableDataRef ref = DKDataCreateMutable();

    if( src )
    {
        const void * srcBytes = DKDataGetBytePtr( src );
        DKIndex srcLength = DKDataGetLength( src );

        DKByteArrayReplaceBytes( &ref->byteArray, DKRangeMake( 0, 0 ), srcBytes, srcLength );
    }

    return ref;
}


///
//  DKDataGetLength()
//
DKIndex DKDataGetLength( DKDataRef ref )
{
    if( ref )
    {
        return ref->byteArray.length;
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
        if( length > ref->byteArray.length )
        {
            DKRange range = DKRangeMake( ref->byteArray.length, 0 );
            DKByteArrayReplaceBytes( &ref->byteArray, range, NULL, length - ref->byteArray.length );
        }
        
        else if( length < ref->byteArray.length )
        {
            DKRange range = DKRangeMake( length, ref->byteArray.length - length );
            DKByteArrayReplaceBytes( &ref->byteArray, range, NULL, 0 );
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
        DKDataSetLength( ref, ref->byteArray.length + length );
    }
}


///
//  DKDataGetBytePtr()
//
const void * DKDataGetBytePtr( DKDataRef ref )
{
    if( ref && (ref->byteArray.length > 0) )
    {
        return ref->byteArray.data;
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
        if( (range.location < ref->byteArray.length) && (DKRangeEnd( range ) <= ref->byteArray.length) )
            return ref->byteArray.data + range.location;
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
            assert( 0 );
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
            assert( 0 );
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
    const void * src = DKDataGetByteRange( ref, range );
    
    if( src )
    {
        assert( buffer );
        memcpy( buffer, src, range.length );
        return range.length;
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
        DKRange range = DKRangeMake( ref->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &ref->byteArray, range, bytes, length );
    }
}


///
//  DKDataReplaceBytes()
//
void DKDataReplaceBytes( DKMutableDataRef ref, DKRange range, const void * bytes, DKIndex length )
{
    if( ref )
    {
        DKByteArrayReplaceBytes( &ref->byteArray, range, bytes, length );
    }
}


///
//  DKDataDeleteBytes()
//
void DKDataDeleteBytes( DKMutableDataRef ref, DKRange range )
{
    if( ref )
    {
        DKByteArrayReplaceBytes( &ref->byteArray, range, NULL, 0 );
    }
}







