//
//  DKData.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#include "DKData.h"
#include "DKCopying.h"


struct DKData
{
    DKObjectHeader _obj;
    DKMemorySegment segment;
};


static DKTypeRef    DKDataAllocate( void );
static DKTypeRef    DKMutableDataAllocate( void );
static DKTypeRef    DKDataInitialize( DKTypeRef ref );
static void         DKDataFinalize( DKTypeRef ref );
static DKTypeRef    DKDataCopy( DKTypeRef ref );
static DKTypeRef    DKMutableDataCopy( DKTypeRef ref );
static DKTypeRef    DKDataMutableCopy( DKTypeRef ref );
int                 DKDataEqual( DKTypeRef a, DKTypeRef b );
int                 DKDataCompare( DKTypeRef a, DKTypeRef b );
DKHashIndex         DKDataHash( DKTypeRef ref );




// DKData Class ==========================================================================
static const DKCopyingInterface __DKDataCopyingInterface__ =
{
    DK_STATIC_INTERFACE_OBJECT,
    
    DKDataCopy,
    DKDataMutableCopy
};

static const DKInterface __DKDataInterfaces__[] =
{
    { &DKCopyingInterfaceID, &__DKDataCopyingInterface__ },
    DK_INTERFACE_TABLE_END
};

static const DKClass __DKDataClass__ =
{
    DK_STATIC_CLASS_OBJECT,

    __DKDataInterfaces__,
    DK_EMPTY_METHOD_TABLE,
    DK_EMPTY_PROPERTY_TABLE,
    
    DKObjectGetInterface,
    DKObjectGetMethod,
    
    DKObjectRetain,
    DKObjectRelease,
    
    DKDataAllocate,
    DKDataInitialize,
    DKDataFinalize,
    
    DKDataEqual,
    DKDataCompare,
    DKDataHash
};




// DKMutableData Class ===================================================================
static const DKCopyingInterface __DKMutableDataCopyingInterface__ =
{
    DK_STATIC_INTERFACE_OBJECT,
    
    DKMutableDataCopy,
    DKDataMutableCopy
};

static const DKInterface __DKMutableDataInterfaces__[] =
{
    { &DKCopyingInterfaceID, &__DKMutableDataCopyingInterface__ },
    DK_INTERFACE_TABLE_END
};

static const DKClass __DKMutableDataClass__ =
{
    DK_STATIC_CLASS_OBJECT,

    __DKMutableDataInterfaces__,
    DK_EMPTY_METHOD_TABLE,
    DK_EMPTY_PROPERTY_TABLE,
    
    DKObjectGetInterface,
    DKObjectGetMethod,
    
    DKObjectRetain,
    DKObjectRelease,
    
    DKMutableDataAllocate,
    DKDataInitialize,
    DKDataFinalize,

    DKDataEqual,
    DKDataCompare,
    DKDataHash
};




// Class Methods =========================================================================

///
//  DKDataClass()
//
DKTypeRef DKDataClass( void )
{
    return &__DKDataClass__;
}


///
//  DKMutableDataClass()
//
DKTypeRef DKMutableDataClass( void )
{
    return &__DKMutableDataClass__;
}


///
//  DKDataAllocate()
//
static DKTypeRef DKDataAllocate( void )
{
    return DKNewObject( DKDataClass(), sizeof(struct DKData), 0 );
}


///
//  DKMutableDataAllocate()
//
static DKTypeRef DKMutableDataAllocate( void )
{
    return DKNewObject( DKMutableDataClass(), sizeof(struct DKData), DKObjectMutable );
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
        DKMemorySegmentInit( &data->segment );
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
        
        if( !DKTestAttribute( data, DKObjectExternalStorage ) )
        {
            DKMemorySegmentClear( &data->segment );
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
int DKDataEqual( DKTypeRef a, DKTypeRef b )
{
    return DKDataCompare( a, b ) == 0;
}


///
//  DKDataCompare()
//
int DKDataCompare( DKTypeRef a, DKTypeRef b )
{
    DKTypeRef btype = DKGetClass( b );
    
    if( (btype == DKDataClass()) || (btype == DKMutableDataClass()) )
    {
        DKDataRef da = a;
        DKDataRef db = b;
        
        if( da->segment.length < db->segment.length )
            return -1;
        
        if( da->segment.length > db->segment.length )
            return 1;
        
        if( da->segment.length == 0 )
            return 0;

        return memcmp( da->segment.data, db->segment.data, da->segment.length );
    }
    
    return DKPtrCompare( a, b );
}


///
//  DKDataHash()
//
DKHashIndex DKDataHash( DKTypeRef ref )
{
    DKDataRef data = ref;
    
    if( data->segment.length > 0 )
        return DKMemHash( data->segment.data, data->segment.length );
    
    return 0;
}




// DKData Interface ======================================================================

///
//  DKDataCreate()
//
DKDataRef DKDataCreate( const void * bytes, DKIndex length )
{
    DKDataRef ref = DKNewObject( DKDataClass(), sizeof(struct DKData) + length, DKObjectExternalStorage );

    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;
        
        DKMemorySegmentInit( &data->segment );

        data->segment.data = (void *)(data + 1);
        data->segment.length = length;
        data->segment.maxLength = length;
        
        memcpy( data->segment.data, bytes, length );
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
    DKDataRef ref = DKNewObject( DKDataClass(), sizeof(struct DKData), DKObjectExternalStorage );

    if( ref )
    {
        struct DKData * data = (struct DKData *)ref;
        
        DKMemorySegmentInit( &data->segment );

        data->segment.data = (void *)bytes;
        data->segment.length = length;
        data->segment.maxLength = length;
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

        DKMemorySegmentReplaceBytes( &ref->segment, DKRangeMake( 0, 0 ), srcBytes, srcLength );
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
        return ref->segment.length;
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
        if( length > ref->segment.length )
        {
            DKMemorySegmentReplaceBytes( &ref->segment, DKRangeMake( ref->segment.length, 0 ), NULL, length - ref->segment.length );
        }
        
        else if( length < ref->segment.length )
        {
            DKMemorySegmentReplaceBytes( &ref->segment, DKRangeMake( length, ref->segment.length - length ), NULL, 0 );
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
        DKDataSetLength( ref, ref->segment.length + length );
    }
}


///
//  DKDataGetBytePtr()
//
const void * DKDataGetBytePtr( DKDataRef ref )
{
    if( ref && (ref->segment.length > 0) )
    {
        return ref->segment.data;
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
        if( (range.location < ref->segment.length) && (DKRangeEnd( range ) <= ref->segment.length) )
            return ref->segment.data + range.location;
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
        if( !DKTestAttribute( ref, DKObjectMutable ) )
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
        if( !DKTestAttribute( ref, DKObjectMutable ) )
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
        DKMemorySegmentReplaceBytes( &ref->segment, DKRangeMake( ref->segment.length, 0 ), bytes, length );
    }
}


///
//  DKDataReplaceBytes()
//
void DKDataReplaceBytes( DKMutableDataRef ref, DKRange range, const void * bytes, DKIndex length )
{
    if( ref )
    {
        DKMemorySegmentReplaceBytes( &ref->segment, range, bytes, length );
    }
}


///
//  DKDataDeleteBytes()
//
void DKDataDeleteBytes( DKMutableDataRef ref, DKRange range )
{
    if( ref )
    {
        DKMemorySegmentReplaceBytes( &ref->segment, range, NULL, 0 );
    }
}







