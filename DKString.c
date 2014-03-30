//
//  DKData.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#include "DKString.h"
#include "DKByteArray.h"
#include "DKCopying.h"
#include "DKStream.h"


struct DKString
{
    DKObjectHeader _obj;
    DKByteArray byteArray;
    DKIndex cursor;
};


static DKTypeRef    DKStringInitialize( DKTypeRef ref );
static void         DKStringFinalize( DKTypeRef ref );




// Class Methods =========================================================================

///
//  DKStringClass()
//
DKTypeRef DKStringClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKString", DKObjectClass(), sizeof(struct DKString) );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->initialize = DKStringInitialize;
        lifeCycle->finalize = DKStringFinalize;

        DKInstallInterface( SharedClassObject, lifeCycle );
        DKRelease( lifeCycle );

        // Comparison
        struct DKComparison * comparison = (struct DKComparison *)DKCreateInterface( DKSelector(Comparison), sizeof(DKComparison) );
        comparison->equal = DKStringEqual;
        comparison->compare = DKStringCompare;
        comparison->hash = DKStringHash;

        DKInstallInterface( SharedClassObject, comparison );
        DKRelease( comparison );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKRetain;
        copying->mutableCopy = DKStringCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );

        // Stream
        /*
        struct DKStream * stream = (struct DKStream *)DKCreateInterface( DKSelector(Stream), sizeof(DKStream) );
        stream->seek = DKStringSeek;
        stream->tell = DKStringTell;
        stream->read = DKStringRead;
        stream->write = DKStringWrite;
        
        DKInstallInterface( SharedClassObject, stream );
        DKRelease( stream );
        */
    }
    
    return SharedClassObject;
}


///
//  DKMutableStringClass()
//
DKTypeRef DKMutableStringClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKMutableString", DKStringClass(), sizeof(struct DKString) );
        
        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKStringCreateMutableCopy;
        copying->mutableCopy = DKStringCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );
        
        // Stream
        /*
        struct DKStream * stream = (struct DKStream *)DKCreateInterface( DKSelector(Stream), sizeof(DKStream) );
        stream->seek = DKStringSeek;
        stream->tell = DKStringTell;
        stream->read = DKStringRead;
        stream->write = DKStringWrite;
        
        DKInstallInterface( SharedClassObject, stream );
        DKRelease( stream );
        */
    }
    
    return SharedClassObject;
}


///
//  DKStringInitialize()
//
static DKTypeRef DKStringInitialize( DKTypeRef ref )
{
    struct DKString * string = (struct DKString *)ref;
    DKByteArrayInit( &string->byteArray );
    string->cursor = 0;
    
    return ref;
}


///
//  DKStringFinalize()
//
static void DKStringFinalize( DKTypeRef ref )
{
    struct DKString * String = (struct DKString *)ref;
    DKByteArrayFinalize( &String->byteArray );
}


///
//  DKStringEqual()
//
int DKStringEqual( DKStringRef a, DKTypeRef b )
{
    if( DKIsKindOfClass( b, DKStringClass() ) )
        return DKStringCompare( a, b ) == 0;
    
    return 0;
}


///
//  DKStringCompare()
//
int DKStringCompare( DKStringRef a, DKStringRef b )
{
    if( a )
    {
        DKVerifyKindOfClass( a, DKStringClass(), 0 );
        DKVerifyKindOfClass( b, DKStringClass(), 0 );
    
        const struct DKString * sa = a;
        const struct DKString * sb = b;
        
        if( sa->byteArray.data )
        {
            if( sb->byteArray.data )
                return dk_ustrcmp( (const char *)sa->byteArray.data, (const char *)sb->byteArray.data, 0 );
            
            return dk_ustrcmp( (const char *)sa->byteArray.data, "", 0 );
        }
        
        else if( sb->byteArray.data )
        {
            return dk_ustrcmp( "", (const char *)sb->byteArray.data, 0 );
        }
    }
    
    return 0;
}


///
//  DKStringHash()
//
DKHashCode DKStringHash( DKTypeRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), 0 );

        const struct DKString * string = ref;
        
        if( string->byteArray.data )
            return dk_strhash( (const char *)string->byteArray.data );
    }
    
    return 0;
}


///
//  DKStringUpdateCursor()
//
static void DKStringUpdateCursor( DKStringRef ref, DKIndex cursor )
{
    struct DKString * string = (struct DKString *)ref;
    
    if( cursor < 0 )
        string->cursor = 0;
    
    else if( cursor > string->byteArray.length )
        string->cursor = string->byteArray.length;
    
    else
        string->cursor = cursor;
}


///
//  CopySubstring()
//
static DKTypeRef CopySubstring( const struct DKString * string, DKRange byteRange )
{
    DKStringRef ref = DKAllocObject( DKStringClass(), 0 );

    if( ref )
    {
        struct DKString * substring = (struct DKString *)ref;

        DKByteArrayInit( &substring->byteArray );

        const uint8_t * src = &string->byteArray.data[byteRange.location];
        
        DKByteArrayReserve( &substring->byteArray, byteRange.length + 1 );
        DKByteArrayReplaceBytes( &substring->byteArray, DKRangeMake( 0, 0 ), src, byteRange.length + 1 );
        substring->byteArray.data[byteRange.length] = '\0';
        
        return substring;
    }

    return NULL;
}




// DKString Interface ====================================================================

///
//  DKStringCreate()
//
DKStringRef DKStringCreate( void )
{
    return DKAllocObject( DKStringClass(), 0 );
}


///
//  DKStringCreateCopy()
//
DKStringRef DKStringCreateCopy( DKStringRef src )
{
    if( src )
    {
        DKAssert( DKIsKindOfClass( src, DKStringClass() ) );

        const char * srcString = DKStringGetCStringPtr( src );
        
        return DKStringCreateWithCString( srcString );
    }
    
    return DKStringCreate();
}


///
//  DKStringCreateWithCString()
//
DKStringRef DKStringCreateWithCString( const char * str )
{
    if( str && (*str != '\0') )
    {
        DKIndex bytes = strlen( str ) + 1;

        DKStringRef ref = DKAllocObject( DKStringClass(), bytes );

        if( ref )
        {
            struct DKString * string = (struct DKString *)ref;
            
            DKByteArrayInitWithExternalStorage( &string->byteArray, (const void *)(string + 1), bytes );
            
            memcpy( string->byteArray.data, str, bytes );
        }

        return ref;
    }
    
    return DKStringCreate();
}


///
//  DKStringCreateWithCStringNoCopy()
//
DKStringRef DKStringCreateWithCStringNoCopy( const char * str )
{
    DKStringRef ref = DKAllocObject( DKStringClass(), 0 );

    if( ref )
    {
        struct DKString * string = (struct DKString *)ref;
        
        DKIndex length = strlen( str ) + 1;
        
        DKByteArrayInitWithExternalStorage( &string->byteArray, (const uint8_t *)str, length );
    }
    
    return ref;
}


///
//  DKStringCreateMutable()
//
DKMutableStringRef DKStringCreateMutable( void )
{
    return (DKMutableStringRef)DKCreate( DKMutableStringClass() );
}


///
//  DKStringCreateMutableCopy()
//
DKMutableStringRef DKStringCreateMutableCopy( DKStringRef src )
{
    DKStringRef ref = DKStringCreateMutable();
    DKStringSetString( ref, src );
    
    return ref;
}


///
//  DKStringGetLength()
//
DKIndex DKStringGetLength( DKStringRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), 0 );
        
        const struct DKString * string = ref;
        
        if( string->byteArray.data )
            return strlen( (const char *)string->byteArray.data );
    }
    
    return 0;
}


///
//  DKStringGetByteLength()
//
DKIndex DKStringGetByteLength( DKStringRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), 0 );
        
        const struct DKString * string = ref;
        
        // The byte length doesn't include the NULL terminator
        if( string->byteArray.length > 1 )
            return string->byteArray.length - 1;
    }
    
    return 0;
}


///
//  DKStringGetCStringPtr()
//
const char * DKStringGetCStringPtr( DKStringRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), 0 );

        const struct DKString * string = ref;
        
        if( string->byteArray.data )
            return (const char *)string->byteArray.data;
    }
    
    return "";
}


///
//  DKStringCopySubstring()
//
DKTypeRef DKStringCopySubstring( DKStringRef ref, DKRange range )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), NULL );

        const struct DKString * string = ref;

        if( range.length > 0 )
        {
            if( string->byteArray.length > 1 )
            {
                const uint8_t * loc = (const uint8_t *)dk_ustridx( (const char *)string->byteArray.data, range.location );
                
                if( loc )
                {
                    const uint8_t * end = (const uint8_t *)dk_ustridx( (const char *)loc, range.length );
                    
                    if( end )
                    {
                        DKRange byteRange;
                        byteRange.location = loc - string->byteArray.data;
                        byteRange.length = end - loc;

                        return CopySubstring( string, byteRange );
                    }
                }
            }

            DKError( "%s: Range %ld,%ld is outside 0,%ld\n", __func__,
                range.location, range.length, dk_ustrlen( (const char *)string->byteArray.data ) );
        }
    }
    
    return NULL;
}


///
//  DKStringCopySubstringFromIndex()
//
DKTypeRef DKStringCopySubstringFromIndex( DKStringRef ref, DKIndex index )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), NULL );

        const struct DKString * string = ref;

        if( index >= 0 )
        {
            if( string->byteArray.length > 1 )
            {
                const uint8_t * loc = (const uint8_t *)dk_ustridx( (const char *)string->byteArray.data, index );
                    
                if( loc )
                {
                    DKRange byteRange;
                    byteRange.location = loc = string->byteArray.data;
                    byteRange.length = string->byteArray.length - byteRange.location - 1;

                    return CopySubstring( string, byteRange );
                }
            }
        }

        DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
            index, dk_ustrlen( (const char *)string->byteArray.data ) );
    }
    
    return NULL;
}


///
//  DKStringCopySubstringToIndex()
//
DKTypeRef DKStringCopySubstringToIndex( DKStringRef ref, DKIndex index )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), NULL );

        const struct DKString * string = ref;

        if( index >= 0 )
        {
            if( string->byteArray.length > 1 )
            {
                const uint8_t * end = (const uint8_t *)dk_ustridx( (const char *)string->byteArray.data, index );
                    
                if( end )
                {
                    DKRange byteRange;
                    byteRange.location = 0;
                    byteRange.length = end - string->byteArray.data;

                    return CopySubstring( string, byteRange );
                }
            }
        }

        DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
            index, dk_ustrlen( (const char *)string->byteArray.data ) );
    }
    
    return NULL;
}


///
//  DKStringGetRangeOfString()
//
DKRange DKStringGetRangeOfString( DKStringRef ref, DKStringRef str, DKIndex startLoc )
{
    DKRange range = DKRangeMake( DKNotFound, 0 );

    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), range );

        const struct DKString * string = ref;

        if( string->byteArray.length > 1 )
        {
            const char * s = (const char *)string->byteArray.data;
        
            if( startLoc > 0 )
            {
                s = dk_ustridx( s, startLoc );
                
                if( s == NULL )
                {
                    DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
                        startLoc, dk_ustrlen( (const char *)string->byteArray.data ) );
                }
            }
            
            const char * search = DKStringGetCStringPtr( str );

            if( search[0] != '\0' )
            {
                const char * ss = strstr( s, search );
                
                if( ss )
                {
                    range.location = 0;
                    range.length = dk_ustrlen( search );
                    
                    const char * cur = (const char *)string->byteArray.data;
                    
                    while( cur != ss )
                    {
                        char32_t ch;
                        cur += dk_ustrscan( cur, &ch );
                        range.location++;
                    }
                    
                    return range;
                }
            }
        }
    }

    return range;
}


///
//  DKStringSetString()
//
void DKStringSetString( DKMutableStringRef ref, DKStringRef str )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableStringClass() );
        
        struct DKString * dst = (struct DKString *)ref;
        DKRange dstRange = DKRangeMake( 0, dst->byteArray.length );
        
        if( str )
        {
            DKVerifyKindOfClass( str, DKStringClass() );
            const struct DKString * src = str;

            DKByteArrayReplaceBytes( &dst->byteArray, dstRange, src->byteArray.data, src->byteArray.length );
        }
        
        else
        {
            DKByteArrayReplaceBytes( &dst->byteArray, dstRange, NULL, 0 );
        }
    }
}









#if 0

///
//  DKDataGetBytePtr()
//
const void * DKDataGetBytePtr( DKDataRef ref )
{
    if( ref )
    {
        DKAssert( DKIsKindOfClass( ref, DKDataClass() ) );

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
        DKAssert( DKIsKindOfClass( ref, DKDataClass() ) );

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
        if( !DKIsKindOfClass( ref, DKMutableDataClass() ) )
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
        if( !DKIsKindOfClass( ref, DKMutableDataClass() ) )
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
        if( !DKIsKindOfClass( ref, DKMutableDataClass() ) )
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

#endif


