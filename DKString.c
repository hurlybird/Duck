//
//  DKData.c
//  Duck
//
//  Created by Derek Nylen on 11-12-06.
//  Copyright (c) 2011 Derek W. Nylen. All rights reserved.
//
#include "DKString.h"
#include "DKUnicode.h"
#include "DKByteArray.h"
#include "DKCopying.h"
#include "DKStream.h"
#include "DKHashTable.h"
#include "DKList.h"
#include "DKArray.h"


struct DKString
{
    DKObjectHeader _obj;
    DKByteArray byteArray;
    DKIndex cursor;
};


static DKTypeRef    DKStringInitialize( DKTypeRef ref );
static void         DKStringFinalize( DKTypeRef ref );

static DKIndex      DKImmutableStringWrite( DKMutableStringRef ref, const void * buffer, DKIndex size, DKIndex count );



// Class Methods =========================================================================

///
//  DKStringClass()
//
DKThreadSafeClassInit( DKStringClass )
{
    DKTypeRef cls = DKCreateClass( "DKString", DKObjectClass(), sizeof(struct DKString) );
    
    // LifeCycle
    struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
    lifeCycle->initialize = DKStringInitialize;
    lifeCycle->finalize = DKStringFinalize;

    DKInstallInterface( cls, lifeCycle );
    DKRelease( lifeCycle );

    // Comparison
    struct DKComparison * comparison = (struct DKComparison *)DKCreateInterface( DKSelector(Comparison), sizeof(DKComparison) );
    comparison->equal = DKStringEqual;
    comparison->compare = DKStringCompare;
    comparison->hash = DKStringHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );
    
    // Description
    struct DKDescription * description = (struct DKDescription *)DKCreateInterface( DKSelector(Description), sizeof(DKDescription) );
    description->copyDescription = DKRetain;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Copying
    struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = DKRetain;
    copying->mutableCopy = DKStringCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Stream
    struct DKStream * stream = (struct DKStream *)DKCreateInterface( DKSelector(Stream), sizeof(DKStream) );
    stream->seek = DKStringSeek;
    stream->tell = DKStringTell;
    stream->read = DKStringRead;
    stream->write = DKImmutableStringWrite;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
}

///
//  DKMutableStringClass()
//
DKThreadSafeClassInit( DKMutableStringClass )
{
    DKTypeRef cls = DKCreateClass( "DKMutableString", DKStringClass(), sizeof(struct DKString) );
    
    // Description
    struct DKDescription * description = (struct DKDescription *)DKCreateInterface( DKSelector(Description), sizeof(DKDescription) );
    description->copyDescription = DKStringCreateCopy;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Copying
    struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = DKStringCreateMutableCopy;
    copying->mutableCopy = DKStringCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Stream
    struct DKStream * stream = (struct DKStream *)DKCreateInterface( DKSelector(Stream), sizeof(DKStream) );
    stream->seek = DKStringSeek;
    stream->tell = DKStringTell;
    stream->read = DKStringRead;
    stream->write = DKStringWrite;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
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




// Internals =============================================================================

///
//  SetCursor()
//
static void SetCursor( const struct DKString * string, DKIndex cursor )
{
    struct DKString * _string = (struct DKString *)string;
    
    if( cursor < 0 )
        _string->cursor = 0;
    
    else if( cursor > _string->byteArray.length )
        _string->cursor = _string->byteArray.length;
    
    else
        _string->cursor = cursor;
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
        
        DKByteArrayReserve( &substring->byteArray, byteRange.length );
        DKByteArrayReplaceBytes( &substring->byteArray, DKRangeMake( 0, 0 ), src, byteRange.length );
        
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
        DKIndex length = strlen( str );

        DKStringRef ref = DKAllocObject( DKStringClass(), length + 1 );

        if( ref )
        {
            struct DKString * string = (struct DKString *)ref;
            
            DKByteArrayInitWithExternalStorage( &string->byteArray, (const void *)(string + 1), length );
            
            memcpy( string->byteArray.data, str, length + 1 );
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
        
        DKIndex length = strlen( str );
        
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
            return dk_ustrlen( (const char *)string->byteArray.data );
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
        return string->byteArray.length;
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
            if( string->byteArray.length > 0 )
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
            if( string->byteArray.length > 0 )
            {
                const uint8_t * loc = (const uint8_t *)dk_ustridx( (const char *)string->byteArray.data, index );
                    
                if( loc )
                {
                    DKRange byteRange;
                    byteRange.location = loc = string->byteArray.data;
                    byteRange.length = string->byteArray.length - byteRange.location;

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
            if( string->byteArray.length > 0 )
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

        if( string->byteArray.length > 0 )
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
//  DKStringCreateListBySeparatingStrings()
//
DKTypeRef DKStringCreateListBySeparatingStrings( DKStringRef ref, DKStringRef separator )
{
    DKListRef array = DKArrayCreateMutable();

    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), array );
        DKVerifyKindOfClass( separator, DKStringClass(), array );
        
        DKRange prev = DKRangeMake( 0, 0 );
        DKRange next = DKStringGetRangeOfString( ref, separator, 0 );
        DKRange copyRange;
        
        while( next.location != DKNotFound )
        {
            copyRange.location = prev.location + prev.length;
            copyRange.length = next.location - copyRange.location;
            
            if( copyRange.length > 0 )
            {
                DKStringRef copy = DKStringCopySubstring( ref, copyRange );
                DKListAppendObject( array, copy );
                DKRelease( copy );
            }
            
            prev = next;
            next = DKStringGetRangeOfString( ref, separator, next.location + next.length );
        }
        
        copyRange.location = prev.location + prev.length;
        copyRange.length = DKStringGetLength( ref ) - copyRange.location;
        
        if( copyRange.length > 0 )
        {
            DKStringRef copy = DKStringCopySubstring( ref, copyRange );
            DKListAppendObject( array, copy );
            DKRelease( copy );
        }
    }
    
    return array;
}


///
//  DKStringCreateByCombiningStrings()
//
DKStringRef DKStringCreateByCombiningStrings( DKTypeRef list, DKStringRef separator )
{
    DKMutableStringRef combinedString = DKStringCreateMutable();

    if( list )
    {
        DKVerifyInterface( list, DKSelector(List), combinedString );
        
        DKIndex count = DKListGetCount( list );
     
        if( count > 0 )
        {
            for( DKIndex i = 0; i < count; ++i )
            {
                DKStringRef str = DKListGetObjectAtIndex( list, i );
                DKVerifyKindOfClass( str, DKStringClass(), combinedString );
                
                DKStringAppendString( combinedString, str );
                
                if( separator && (i < (count - 1)) )
                    DKStringAppendString( combinedString, separator );
            }
            
            return combinedString;
        }
    }
    
    return combinedString;
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


///
//  DKStringAppendString()
//
void DKStringAppendString( DKMutableStringRef ref, DKStringRef str )
{
    if( ref && str )
    {
        DKVerifyKindOfClass( ref, DKMutableStringClass() );
        DKVerifyKindOfClass( str, DKStringClass() );
        
        struct DKString * dst = (struct DKString *)ref;
        const struct DKString * src = str;
        
        DKRange dstRange = DKRangeMake( dst->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &dst->byteArray, dstRange, src->byteArray.data, src->byteArray.length );
    }
}


///
//  DKStringAppendFormat()
//
void DKStringAppendFormat( DKMutableStringRef ref, const char * format, ... )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableStringClass() );

        const struct DKString * string = (struct DKString *)ref;
        SetCursor( string, string->byteArray.length );

        va_list arg_ptr;
        va_start( arg_ptr, format );
        
        DKVSPrintf( ref, format, arg_ptr );
        
        va_end( arg_ptr );
    }
}


///
//  DKStringReplaceSubstring()
//
void DKStringReplaceSubstring( DKMutableStringRef ref, DKRange range, DKStringRef str )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableStringClass() );
        DKVerifyKindOfClass( str, DKStringClass() );

        struct DKString * dst = (struct DKString *)ref;
        const struct DKString * src = (struct DKString *)str;
        
        DKVerifyRange( range, dst->byteArray.length );
        
        DKByteArrayReplaceBytes( &dst->byteArray, range, src->byteArray.data, src->byteArray.length );
    }
}


///
//  DKStringReplaceOccurrencesOfString()
//
void DKStringReplaceOccurrencesOfString( DKMutableStringRef ref, DKStringRef pattern, DKStringRef replacement )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableStringClass() );
        
        DKRange range = DKStringGetRangeOfString( ref, pattern, 0 );
        DKIndex length = DKStringGetLength( replacement );
        
        while( range.location != DKNotFound )
        {
            DKStringReplaceSubstring( ref, range, replacement );
            range = DKStringGetRangeOfString( ref, pattern, range.location + length );
        }
    }
}


///
//  DKStringDeleteSubstring()
//
void DKStringDeleteSubstring( DKMutableStringRef ref, DKRange range )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableStringClass() );

        struct DKString * dst = (struct DKString *)ref;
        
        DKVerifyRange( range, dst->byteArray.length );
        
        DKByteArrayReplaceBytes( &dst->byteArray, range, NULL, 0 );
    }
}


///
//  DKStringSeek()
//
int DKStringSeek( DKStringRef ref, DKIndex offset, int origin )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), -1 );

        struct DKString * string = (struct DKString *)ref;
        
        DKIndex cursor = string->cursor;
        
        if( origin == DKSeekSet )
            cursor = offset;
        
        else if( origin == DKSeekCur )
            cursor += offset;
        
        else
            cursor = string->byteArray.length + cursor;

        SetCursor( string, cursor );
        
        return 0;
    }
    
    return -1;
}


///
//  DKStringTell()
//
DKIndex DKStringTell( DKStringRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), -1 );

        struct DKString * string = (struct DKString *)ref;
        return string->cursor;
    }
    
    return -1;
}


///
//  DKStringRead()
//
DKIndex DKStringRead( DKStringRef ref, void * buffer, DKIndex size, DKIndex count )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKStringClass(), 0 );

        struct DKString * string = (struct DKString *)ref;

        SetCursor( string, string->cursor );
        
        DKRange range = DKRangeMake( string->cursor, size * count );
        
        if( range.length > (string->byteArray.length - string->cursor) )
            range.length = string->byteArray.length - string->cursor;
        
        memcpy( buffer, &string->byteArray.data[range.location], range.length );
        
        SetCursor( string, string->cursor + range.length );
        
        return range.length / size;
    }
    
    return 0;
}


///
//  DKStringWrite()
//
static DKIndex DKImmutableStringWrite( DKMutableStringRef ref, const void * buffer, DKIndex size, DKIndex count )
{
    DKError( "DKStringWrite: Trying to modify an immutable object." );
    return 0;
}

DKIndex DKStringWrite( DKMutableStringRef ref, const void * buffer, DKIndex size, DKIndex count )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableStringClass(), 0 );

        struct DKString * string = (struct DKString *)ref;

        SetCursor( string, string->cursor );
        
        DKRange range = DKRangeMake( string->cursor, size * count );
        
        if( range.length > (string->byteArray.length - string->cursor) )
            range.length = string->byteArray.length - string->cursor;
        
        DKByteArrayReplaceBytes( &string->byteArray, range, buffer, size * count );

        SetCursor( string, string->cursor + (size * count) );
        
        return count;
    }
    
    return 0;
}




// Constant Strings ======================================================================

static volatile DKSpinLock DKConstantStringLock = DKSpinLockInit;
static DKTypeRef DKConstantStringClassObject = NULL;
static DKTypeRef DKConstantStringTable = NULL;


///
//  __DKStringDefineConstantString()
//
DKStringRef __DKStringDefineConstantString( const char * str )
{
    if( DKConstantStringClassObject == NULL )
    {
        // Create the objects
        DKTypeRef cls = DKCreateClass( "DKConstantString", DKStringClass(), sizeof(struct DKString) );
        DKInstallInterface( cls, DKStaticReferenceCounting() );
        
        DKTypeRef table = DKHashTableCreateMutable();
        
        // Store the objects while locked
        DKSpinLockLock( &DKConstantStringLock );
        
        if( DKConstantStringClassObject == NULL )
            DKConstantStringClassObject = cls;
        
        if( DKConstantStringTable == NULL )
            DKConstantStringTable = table;
        
        DKSpinLockUnlock( &DKConstantStringLock );

        // If the objects weren't stored, release them
        if( DKConstantStringClassObject != cls )
            DKRelease( cls );
        
        if( DKConstantStringTable != table )
            DKRelease( table );
    }

    // Create a temporary stack object for the table lookup
    struct DKString key =
    {
        { DKConstantStringClassObject, 1 }, // isa, refcount
    };
    
    DKIndex length = strlen( str );
    DKByteArrayInitWithExternalStorage( &key.byteArray, (const uint8_t *)str, length );
    
    // Check the table for an existing version
    DKSpinLockLock( &DKConstantStringLock );
    DKTypeRef constantString = DKHashTableGetObject( DKConstantStringTable, &key );
    DKSpinLockUnlock( &DKConstantStringLock );
    
    if( !constantString )
    {
        // Create a new constant string
        DKStringRef newConstantString = DKAllocObject( DKConstantStringClassObject, 0 );
        DKByteArrayInitWithExternalStorage( &((struct DKString *)newConstantString)->byteArray, (const uint8_t *)str, length );

        // Try to insert it in the table
        DKSpinLockLock( &DKConstantStringLock );
        DKIndex count = DKHashTableGetCount( DKConstantStringTable );
        DKHashTableInsertObject( DKConstantStringTable, newConstantString, newConstantString, DKDictionaryInsertIfNotFound );
        
        // Did someone sneak in and insert it before us?
        if( DKHashTableGetCount( DKConstantStringTable ) == count )
            constantString = DKHashTableGetObject( DKConstantStringTable, newConstantString );
        
        else
            constantString = newConstantString;
        
        DKSpinLockUnlock( &DKConstantStringLock );

        // This either removes the extra retain on an added entry, or releases the object
        // if the insert failed
        DKRelease( newConstantString );
    }
    
    return constantString;
}


