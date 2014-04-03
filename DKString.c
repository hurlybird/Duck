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


static DKObjectRef  DKStringInitialize( DKObjectRef ref );
static void         DKStringFinalize( DKObjectRef ref );

static DKIndex      DKImmutableStringWrite( DKMutableObjectRef ref, const void * buffer, DKIndex size, DKIndex count );



// Class Methods =========================================================================

///
//  DKStringClass()
//
DKThreadSafeClassInit( DKStringClass )
{
    // Since DKString, DKConstantString, DKHashTable and DKMutableHashTable are all
    // involved in creating constant strings, the names for these classes are
    // initialized in DKRuntimeInit().
    DKClassRef cls = DKAllocClass( NULL, DKObjectClass(), sizeof(struct DKString) );
    
    // LifeCycle
    struct DKLifeCycle * lifeCycle = DKAllocInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
    lifeCycle->initialize = DKStringInitialize;
    lifeCycle->finalize = DKStringFinalize;

    DKInstallInterface( cls, lifeCycle );
    DKRelease( lifeCycle );

    // Comparison
    struct DKComparison * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(DKComparison) );
    comparison->equal = (DKEqualMethod)DKStringEqual;
    comparison->compare = (DKCompareMethod)DKStringCompare;
    comparison->hash = (DKHashMethod)DKStringHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );
    
    // Description
    struct DKDescription * description = DKAllocInterface( DKSelector(Description), sizeof(DKDescription) );
    description->copyDescription = (DKCopyDescriptionMethod)DKRetain;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKStringCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Stream
    struct DKStream * stream = DKAllocInterface( DKSelector(Stream), sizeof(DKStream) );
    stream->seek = (DKStreamSeekMethod)DKStringSeek;
    stream->tell = (DKStreamTellMethod)DKStringTell;
    stream->read = (DKStreamReadMethod)DKStringRead;
    stream->write = DKImmutableStringWrite;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
}


///
//  DKConstantStringClass()
//
DKThreadSafeClassInit( DKConstantStringClass )
{
    // Since DKString, DKConstantString, DKHashTable and DKMutableHashTable are all
    // involved in creating constant strings, the names for these classes are
    // initialized in DKRuntimeInit().
    DKClassRef cls = DKAllocClass( NULL, DKStringClass(), sizeof(struct DKString) );
    
    return cls;
}


///
//  DKMutableStringClass()
//
DKThreadSafeClassInit( DKMutableStringClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKMutableString" ), DKStringClass(), sizeof(struct DKString) );
    
    // Description
    struct DKDescription * description = DKAllocInterface( DKSelector(Description), sizeof(DKDescription) );
    description->copyDescription = (DKCopyDescriptionMethod)DKStringCreateCopy;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = (DKCopyMethod)DKStringCreateMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKStringCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Stream
    // Stream
    struct DKStream * stream = DKAllocInterface( DKSelector(Stream), sizeof(DKStream) );
    stream->seek = (DKStreamSeekMethod)DKStringSeek;
    stream->tell = (DKStreamTellMethod)DKStringTell;
    stream->read = (DKStreamReadMethod)DKStringRead;
    stream->write = (DKStreamWriteMethod)DKStringWrite;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
}


///
//  DKStringInitialize()
//
static DKObjectRef DKStringInitialize( DKObjectRef ref )
{
    struct DKString * string = (struct DKString *)ref;
    DKByteArrayInit( &string->byteArray );
    string->cursor = 0;
    
    return ref;
}


///
//  DKStringFinalize()
//
static void DKStringFinalize( DKObjectRef ref )
{
    struct DKString * String = (struct DKString *)ref;
    DKByteArrayFinalize( &String->byteArray );
}


///
//  DKStringEqual()
//
int DKStringEqual( DKStringRef a, DKObjectRef b )
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
        DKAssertKindOfClass( a, DKStringClass() );

        // DKCompare doesn't require that a and b be the same type, so if b isn't a string
        // object, fall back to comparing pointers.
        DKCheckKindOfClass( b, DKStringClass(), DKDefaultCompare( a, b ) );
    
        if( a->byteArray.data )
        {
            if( b->byteArray.data )
                return dk_ustrcmp( (const char *)a->byteArray.data, (const char *)b->byteArray.data, 0 );
            
            return dk_ustrcmp( (const char *)a->byteArray.data, "", 0 );
        }
        
        else if( b->byteArray.data )
        {
            return dk_ustrcmp( "", (const char *)b->byteArray.data, 0 );
        }
    }
    
    return 0;
}


///
//  DKStringHash()
//
DKHashCode DKStringHash( DKStringRef ref )
{
    if( ref )
    {
        DKAssertKindOfClass( ref, DKStringClass() );

        if( ref->byteArray.data )
            return dk_strhash( (const char *)ref->byteArray.data );
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
static DKStringRef CopySubstring( const char * str, DKRange byteRange )
{
    DKAssert( str );

    DKStringRef ref = DKAllocObject( DKStringClass(), byteRange.length + 1 );

    if( ref )
    {
        struct DKString * string = (struct DKString *)ref;
        
        DKByteArrayInitWithExternalStorage( &string->byteArray, (const void *)(string + 1), byteRange.length );
        
        memcpy( string->byteArray.data, &str[byteRange.location], byteRange.length );
        string->byteArray.data[byteRange.length] = '\0';
    }

    return ref;
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
    if( str )
    {
        DKIndex length = strlen( str );

        return CopySubstring( str, DKRangeMake( 0, length ) );
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
    return DKAllocObject( DKMutableStringClass(), 0 );
}


///
//  DKStringCreateMutableCopy()
//
DKMutableStringRef DKStringCreateMutableCopy( DKStringRef src )
{
    DKMutableStringRef ref = DKAllocObject( DKMutableStringClass(), 0 );
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
        DKAssertKindOfClass( ref, DKStringClass() );
        
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
        DKAssertKindOfClass( ref, DKStringClass() );
        
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
        DKAssertKindOfClass( ref, DKStringClass() );

        const struct DKString * string = ref;
        
        if( string->byteArray.data )
            return (const char *)string->byteArray.data;
    }
    
    return "";
}


///
//  DKStringCopySubstring()
//
DKStringRef DKStringCopySubstring( DKStringRef ref, DKRange range )
{
    if( ref )
    {
        DKAssertKindOfClass( ref, DKStringClass() );

        if( range.length > 0 )
        {
            if( ref->byteArray.length > 0 )
            {
                const uint8_t * loc = (const uint8_t *)dk_ustridx( (const char *)ref->byteArray.data, range.location );
                
                if( loc )
                {
                    const uint8_t * end = (const uint8_t *)dk_ustridx( (const char *)loc, range.length );
                    
                    if( end )
                    {
                        DKRange byteRange;
                        byteRange.location = loc - ref->byteArray.data;
                        byteRange.length = end - loc;

                        return CopySubstring( (const char *)ref->byteArray.data, byteRange );
                    }
                }
            }

            DKError( "%s: Range %ld,%ld is outside 0,%ld\n", __func__,
                range.location, range.length, dk_ustrlen( (const char *)ref->byteArray.data ) );
        }
    }
    
    return NULL;
}


///
//  DKStringCopySubstringFromIndex()
//
DKStringRef DKStringCopySubstringFromIndex( DKStringRef ref, DKIndex index )
{
    if( ref )
    {
        DKAssertKindOfClass( ref, DKStringClass() );

        if( index >= 0 )
        {
            if( ref->byteArray.length > 0 )
            {
                const uint8_t * loc = (const uint8_t *)dk_ustridx( (const char *)ref->byteArray.data, index );
                    
                if( loc )
                {
                    DKRange byteRange;
                    byteRange.location = loc = ref->byteArray.data;
                    byteRange.length = ref->byteArray.length - byteRange.location;

                    return CopySubstring( (const char *)ref->byteArray.data, byteRange );
                }
            }
        }

        DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
            index, dk_ustrlen( (const char *)ref->byteArray.data ) );
    }
    
    return NULL;
}


///
//  DKStringCopySubstringToIndex()
//
DKStringRef DKStringCopySubstringToIndex( DKStringRef ref, DKIndex index )
{
    if( ref )
    {
        DKAssertKindOfClass( ref, DKStringClass() );

        if( index >= 0 )
        {
            if( ref->byteArray.length > 0 )
            {
                const uint8_t * end = (const uint8_t *)dk_ustridx( (const char *)ref->byteArray.data, index );
                    
                if( end )
                {
                    DKRange byteRange;
                    byteRange.location = 0;
                    byteRange.length = end - ref->byteArray.data;

                    return CopySubstring( (const char *)ref->byteArray.data, byteRange );
                }
            }
        }

        DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
            index, dk_ustrlen( (const char *)ref->byteArray.data ) );
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
        DKAssertKindOfClass( ref, DKStringClass() );

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
DKListRef DKStringCreateListBySeparatingStrings( DKStringRef ref, DKStringRef separator )
{
    DKMutableArrayRef array = DKArrayCreateMutable();

    if( ref )
    {
        DKAssertKindOfClass( ref, DKStringClass() );
        DKAssertKindOfClass( separator, DKStringClass() );
        
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
DKStringRef DKStringCreateByCombiningStrings( DKListRef list, DKStringRef separator )
{
    DKMutableStringRef combinedString = DKStringCreateMutable();

    if( list )
    {
        DKIndex count = DKListGetCount( list );
     
        if( count > 0 )
        {
            for( DKIndex i = 0; i < count; ++i )
            {
                DKStringRef str = DKListGetObjectAtIndex( list, i );
                DKAssertKindOfClass( str, DKStringClass() );
                
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
        DKAssertKindOfClass( ref, DKMutableStringClass() );
        
        DKRange range = DKRangeMake( 0, ref->byteArray.length );
        
        if( str )
        {
            DKAssertKindOfClass( str, DKStringClass() );
            const struct DKString * src = str;

            DKByteArrayReplaceBytes( &ref->byteArray, range, src->byteArray.data, src->byteArray.length );
        }
        
        else
        {
            DKByteArrayReplaceBytes( &ref->byteArray, range, NULL, 0 );
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
        DKAssertKindOfClass( ref, DKMutableStringClass() );
        DKAssertKindOfClass( str, DKStringClass() );
        
        DKRange range = DKRangeMake( ref->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &ref->byteArray, range, str->byteArray.data, str->byteArray.length );
    }
}


///
//  DKStringAppendFormat()
//
void DKStringAppendFormat( DKMutableStringRef ref, const char * format, ... )
{
    if( ref )
    {
        DKAssertKindOfClass( ref, DKMutableStringClass() );

        SetCursor( ref, ref->byteArray.length );

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
        DKAssertKindOfClass( ref, DKMutableStringClass() );
        DKAssertKindOfClass( str, DKStringClass() );
        DKCheckRange( range, ref->byteArray.length );
        
        DKByteArrayReplaceBytes( &ref->byteArray, range, str->byteArray.data, str->byteArray.length );
    }
}


///
//  DKStringReplaceOccurrencesOfString()
//
void DKStringReplaceOccurrencesOfString( DKMutableStringRef ref, DKStringRef pattern, DKStringRef replacement )
{
    if( ref )
    {
        DKAssertKindOfClass( ref, DKMutableStringClass() );
        
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
        DKAssertKindOfClass( ref, DKMutableStringClass() );
        DKCheckRange( range, ref->byteArray.length );
        
        DKByteArrayReplaceBytes( &ref->byteArray, range, NULL, 0 );
    }
}


///
//  DKStringSeek()
//
int DKStringSeek( DKStringRef ref, DKIndex offset, int origin )
{
    if( ref )
    {
        DKAssertKindOfClass( ref, DKStringClass() );
        
        DKIndex cursor = ref->cursor;
        
        if( origin == DKSeekSet )
            cursor = offset;
        
        else if( origin == DKSeekCur )
            cursor += offset;
        
        else
            cursor = ref->byteArray.length + cursor;

        SetCursor( ref, cursor );
        
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
        DKAssertKindOfClass( ref, DKStringClass() );
        return ref->cursor;
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
        DKAssertKindOfClass( ref, DKStringClass() );

        SetCursor( (struct DKString *)ref, ref->cursor );
        
        DKRange range = DKRangeMake( ref->cursor, size * count );
        
        if( range.length > (ref->byteArray.length - ref->cursor) )
            range.length = ref->byteArray.length - ref->cursor;
        
        memcpy( buffer, &ref->byteArray.data[range.location], range.length );
        
        SetCursor( (struct DKString *)ref, ref->cursor + range.length );
        
        return range.length / size;
    }
    
    return 0;
}


///
//  DKStringWrite()
//
static DKIndex DKImmutableStringWrite( DKMutableObjectRef ref, const void * buffer, DKIndex size, DKIndex count )
{
    DKError( "DKStringWrite: Trying to modify an immutable object." );
    return 0;
}

DKIndex DKStringWrite( DKMutableStringRef ref, const void * buffer, DKIndex size, DKIndex count )
{
    if( ref )
    {
        DKAssertKindOfClass( ref, DKMutableStringClass() );

        SetCursor( ref, ref->cursor );
        
        DKRange range = DKRangeMake( ref->cursor, size * count );
        
        if( range.length > (ref->byteArray.length - ref->cursor) )
            range.length = ref->byteArray.length - ref->cursor;
        
        DKByteArrayReplaceBytes( &ref->byteArray, range, buffer, size * count );

        SetCursor( ref, ref->cursor + (size * count) );
        
        return count;
    }
    
    return 0;
}




// Constant Strings ======================================================================
static DKMutableHashTableRef DKConstantStringTable = NULL;
static DKSpinLock DKConstantStringTableLock = DKSpinLockInit;

///
//  __DKStringDefineConstantString()
//
DKStringRef __DKStringDefineConstantString( const char * str )
{
    if( DKConstantStringTable == NULL )
    {
        DKMutableHashTableRef table = DKHashTableCreateMutable();
        
        DKSpinLockLock( &DKConstantStringTableLock );
        
        if( DKConstantStringTable == NULL )
            DKConstantStringTable = table;
        
        DKSpinLockUnlock( &DKConstantStringTableLock );

        if( DKConstantStringTable != table )
            DKRelease( table );
    }

    // Create a temporary stack object for the table lookup
    DKClassRef constantStringClass = DKConstantStringClass();
    
    struct DKString key =
    {
        DKStaticObjectHeader( constantStringClass ),
    };
    
    DKIndex length = strlen( str );
    DKByteArrayInitWithExternalStorage( &key.byteArray, (const uint8_t *)str, length );
    
    // Check the table for an existing version
    DKSpinLockLock( &DKConstantStringTableLock );
    DKStringRef constantString = DKHashTableGetObject( DKConstantStringTable, &key );
    DKSpinLockUnlock( &DKConstantStringTableLock );
    
    if( !constantString )
    {
        // Create a new constant string
        DKStringRef newConstantString = DKAllocObject( constantStringClass, 0 );
        DKByteArrayInitWithExternalStorage( &((struct DKString *)newConstantString)->byteArray, (const uint8_t *)str, length );

        // Try to insert it in the table
        DKSpinLockLock( &DKConstantStringTableLock );
        DKIndex count = DKHashTableGetCount( DKConstantStringTable );
        DKHashTableInsertObject( DKConstantStringTable, newConstantString, newConstantString, DKDictionaryInsertIfNotFound );
        
        // Did someone sneak in and insert it before us?
        if( DKHashTableGetCount( DKConstantStringTable ) == count )
            constantString = DKHashTableGetObject( DKConstantStringTable, newConstantString );
        
        else
            constantString = newConstantString;
        
        DKSpinLockUnlock( &DKConstantStringTableLock );

        // This either removes the extra retain on an added entry, or releases the object
        // if the insert failed
        DKRelease( newConstantString );
    }
    
    return constantString;
}


