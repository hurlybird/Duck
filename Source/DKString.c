/*****************************************************************************************

  DKString.c

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

#include "DKString.h"
#include "DKUnicode.h"
#include "DKByteArray.h"
#include "DKCopying.h"
#include "DKStream.h"
#include "DKHashTable.h"
#include "DKList.h"
#include "DKArray.h"

#include "icu/unicode/utf8.h"


struct DKString
{
    DKObject _obj;
    DKByteArray byteArray;
    DKIndex cursor;
};


static DKObjectRef  DKStringInitialize( DKObjectRef _self );
static void         DKStringFinalize( DKObjectRef _self );

static DKIndex      DKImmutableStringWrite( DKMutableObjectRef _self, const void * buffer, DKIndex size, DKIndex count );



// Class Methods =========================================================================

///
//  DKStringClass()
//
DKThreadSafeClassInit( DKStringClass )
{
    // Since DKString, DKConstantString, DKHashTable and DKMutableHashTable are all
    // involved in creating constant strings, the names for these classes are
    // initialized in DKRuntimeInit().
    DKClassRef cls = DKAllocClass( NULL, DKObjectClass(), sizeof(struct DKString), 0 );
    
    // Allocation
    struct DKAllocation * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(DKAllocation) );
    allocation->initialize = DKStringInitialize;
    allocation->finalize = DKStringFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

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
    DKClassRef cls = DKAllocClass( NULL, DKStringClass(), sizeof(struct DKString), DKPreventSubclassing | DKInstancesNeverDeallocated );
    
    return cls;
}


///
//  DKMutableStringClass()
//
DKThreadSafeClassInit( DKMutableStringClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKMutableString" ), DKStringClass(), sizeof(struct DKString), 0 );
    
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
static DKObjectRef DKStringInitialize( DKObjectRef _self )
{
    struct DKString * string = (struct DKString *)_self;
    DKByteArrayInit( &string->byteArray );
    string->cursor = 0;
    
    return _self;
}


///
//  DKStringFinalize()
//
static void DKStringFinalize( DKObjectRef _self )
{
    struct DKString * String = (struct DKString *)_self;
    DKByteArrayFinalize( &String->byteArray );
}


///
//  DKStringEqual()
//
int DKStringEqual( DKStringRef _self, DKObjectRef other )
{
    if( DKIsKindOfClass( other, DKStringClass() ) )
        return DKStringCompare( _self, other ) == 0;
    
    return 0;
}


///
//  DKStringCompare()
//
int DKStringCompare( DKStringRef _self, DKStringRef other )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        // DKCompare requires that the objects have some strict ordering property useful
        // for comparison, yet has no way of checking if the objects actually meet that
        // requirement.
        DKCheckKindOfClass( other, DKStringClass(), DKPointerCompare( _self, other ) );
    
        if( _self->byteArray.data )
        {
            if( _self->byteArray.data )
                return dk_ustrcmp( (const char *)_self->byteArray.data, (const char *)other->byteArray.data );
            
            return dk_ustrcmp( (const char *)_self->byteArray.data, "" );
        }
        
        else if( other->byteArray.data )
        {
            return dk_ustrcmp( "", (const char *)other->byteArray.data );
        }
    }
    
    return 0;
}


///
//  DKStringHash()
//
DKHashCode DKStringHash( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( _self->byteArray.data )
            return dk_strhash( (const char *)_self->byteArray.data );
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

    DKStringRef _self = DKAllocObject( DKStringClass(), byteRange.length + 1 );

    if( _self )
    {
        struct DKString * string = (struct DKString *)_self;
        
        DKByteArrayInitWithExternalStorage( &string->byteArray, (const void *)(string + 1), byteRange.length );
        
        memcpy( string->byteArray.data, &str[byteRange.location], byteRange.length );
        string->byteArray.data[byteRange.length] = '\0';
    }

    return _self;
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
    DKStringRef _self = DKAllocObject( DKStringClass(), 0 );

    if( _self )
    {
        struct DKString * string = (struct DKString *)_self;
        
        DKIndex length = strlen( str );
        
        DKByteArrayInitWithExternalStorage( &string->byteArray, (const uint8_t *)str, length );
    }
    
    return _self;
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
    DKMutableStringRef _self = DKAllocObject( DKMutableStringClass(), 0 );
    DKStringSetString( _self, src );
    
    return _self;
}


///
//  DKStringGetLength()
//
DKIndex DKStringGetLength( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        const struct DKString * string = _self;
        
        if( string->byteArray.data )
            return dk_ustrlen( (const char *)string->byteArray.data );
    }
    
    return 0;
}


///
//  DKStringGetByteLength()
//
DKIndex DKStringGetByteLength( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        const struct DKString * string = _self;
        return string->byteArray.length;
    }
    
    return 0;
}


///
//  DKStringGetCStringPtr()
//
const char * DKStringGetCStringPtr( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        // Note: ByteArray data is never null.
        return (const char *)_self->byteArray.data;
    }
    
    return "";
}


///
//  DKStringCopySubstring()
//
DKStringRef DKStringCopySubstring( DKStringRef _self, DKRange range )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( range.length > 0 )
        {
            if( _self->byteArray.length > 0 )
            {
                const uint8_t * loc = (const uint8_t *)dk_ustridx( (const char *)_self->byteArray.data, range.location );
                
                if( loc )
                {
                    const uint8_t * end = (const uint8_t *)dk_ustridx( (const char *)loc, range.length );
                    
                    if( end )
                    {
                        DKRange byteRange;
                        byteRange.location = loc - _self->byteArray.data;
                        byteRange.length = end - loc;

                        return CopySubstring( (const char *)_self->byteArray.data, byteRange );
                    }
                }
            }

            DKError( "%s: Range %ld,%ld is outside 0,%ld\n", __func__,
                range.location, range.length, dk_ustrlen( (const char *)_self->byteArray.data ) );
        }
    }
    
    return NULL;
}


///
//  DKStringCopySubstringFromIndex()
//
DKStringRef DKStringCopySubstringFromIndex( DKStringRef _self, DKIndex index )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( index >= 0 )
        {
            if( _self->byteArray.length > 0 )
            {
                const uint8_t * loc = (const uint8_t *)dk_ustridx( (const char *)_self->byteArray.data, index );
                    
                if( loc )
                {
                    DKRange byteRange;
                    byteRange.location = loc = _self->byteArray.data;
                    byteRange.length = _self->byteArray.length - byteRange.location;

                    return CopySubstring( (const char *)_self->byteArray.data, byteRange );
                }
            }
        }

        DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
            index, dk_ustrlen( (const char *)_self->byteArray.data ) );
    }
    
    return NULL;
}


///
//  DKStringCopySubstringToIndex()
//
DKStringRef DKStringCopySubstringToIndex( DKStringRef _self, DKIndex index )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( index >= 0 )
        {
            if( _self->byteArray.length > 0 )
            {
                const uint8_t * end = (const uint8_t *)dk_ustridx( (const char *)_self->byteArray.data, index );
                    
                if( end )
                {
                    DKRange byteRange;
                    byteRange.location = 0;
                    byteRange.length = end - _self->byteArray.data;

                    return CopySubstring( (const char *)_self->byteArray.data, byteRange );
                }
            }
        }

        DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
            index, dk_ustrlen( (const char *)_self->byteArray.data ) );
    }
    
    return NULL;
}


///
//  DKStringGetRangeOfString()
//
DKRange DKStringGetRangeOfString( DKStringRef _self, DKStringRef str, DKIndex startLoc )
{
    DKRange range = DKRangeMake( DKNotFound, 0 );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        const struct DKString * string = _self;

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
                range = dk_ustrstr_range( s, search );
                
                if( range.location != DKNotFound )
                    range.location += startLoc;
                
                return range;
            }
        }
    }

    return range;
}


///
//  DKStringCreateListBySeparatingStrings()
//
DKListRef DKStringCreateListBySeparatingStrings( DKStringRef _self, DKStringRef separator )
{
    DKMutableArrayRef array = DKArrayCreateMutable();

    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        DKAssertKindOfClass( separator, DKStringClass() );
        
        DKRange prev = DKRangeMake( 0, 0 );
        DKRange next = DKStringGetRangeOfString( _self, separator, 0 );
        DKRange copyRange;
        
        while( next.location != DKNotFound )
        {
            copyRange.location = prev.location + prev.length;
            copyRange.length = next.location - copyRange.location;
            
            if( copyRange.length > 0 )
            {
                DKStringRef copy = DKStringCopySubstring( _self, copyRange );
                DKListAppendObject( array, copy );
                DKRelease( copy );
            }
            
            prev = next;
            next = DKStringGetRangeOfString( _self, separator, next.location + next.length );
        }
        
        copyRange.location = prev.location + prev.length;
        copyRange.length = DKStringGetLength( _self ) - copyRange.location;
        
        if( copyRange.length > 0 )
        {
            DKStringRef copy = DKStringCopySubstring( _self, copyRange );
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
void DKStringSetString( DKMutableStringRef _self, DKStringRef str )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );
        
        DKRange range = DKRangeMake( 0, _self->byteArray.length );
        
        if( str )
        {
            DKAssertKindOfClass( str, DKStringClass() );
            const struct DKString * src = str;

            DKByteArrayReplaceBytes( &_self->byteArray, range, src->byteArray.data, src->byteArray.length );
        }
        
        else
        {
            DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, 0 );
        }
    }
}


///
//  DKStringAppendString()
//
void DKStringAppendString( DKMutableStringRef _self, DKStringRef str )
{
    if( _self && str )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );
        DKAssertKindOfClass( str, DKStringClass() );
        
        DKRange range = DKRangeMake( _self->byteArray.length, 0 );
        DKByteArrayReplaceBytes( &_self->byteArray, range, str->byteArray.data, str->byteArray.length );
    }
}


///
//  DKStringAppendFormat()
//
void DKStringAppendFormat( DKMutableStringRef _self, const char * format, ... )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );

        SetCursor( _self, _self->byteArray.length );

        va_list arg_ptr;
        va_start( arg_ptr, format );
        
        DKVSPrintf( _self, format, arg_ptr );
        
        va_end( arg_ptr );
    }
}


///
//  DKStringReplaceSubstring()
//
void DKStringReplaceSubstring( DKMutableStringRef _self, DKRange range, DKStringRef str )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );
        DKAssertKindOfClass( str, DKStringClass() );
        DKCheckRange( range, _self->byteArray.length );
        
        DKByteArrayReplaceBytes( &_self->byteArray, range, str->byteArray.data, str->byteArray.length );
    }
}


///
//  DKStringReplaceOccurrencesOfString()
//
void DKStringReplaceOccurrencesOfString( DKMutableStringRef _self, DKStringRef pattern, DKStringRef replacement )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );
        
        DKRange range = DKStringGetRangeOfString( _self, pattern, 0 );
        DKIndex length = DKStringGetLength( replacement );
        
        while( range.location != DKNotFound )
        {
            DKStringReplaceSubstring( _self, range, replacement );
            range = DKStringGetRangeOfString( _self, pattern, range.location + length );
        }
    }
}


///
//  DKStringDeleteSubstring()
//
void DKStringDeleteSubstring( DKMutableStringRef _self, DKRange range )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );
        DKCheckRange( range, _self->byteArray.length );
        
        DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, 0 );
    }
}


///
//  DKStringIsAbsolutePath()
//
int DKStringIsAbsolutePath( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        return (_self->byteArray.length > 0) && (_self->byteArray.data[0] == DKPathComponentSeparator);
    }
    
    return 0;
}

///
//  DKStringCopyLastPathComponent()
//
DKStringRef DKStringCopyLastPathComponent( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        const char * path = (const char *)_self->byteArray.data;

        // Empty or root path
        if( (_self->byteArray.length == 0) || ((_self->byteArray.length == 1) && (path[0] == DKPathComponentSeparator)) )
            return DKSTR( "" );
        
        // Skip over trailing slashes
        int32_t end = (int32_t)_self->byteArray.length;
        char32_t ch;
        
        while( end > 0 )
        {
            int32_t i = end;
            
            U8_PREV( path, 0, i, ch );
            
            if( ch != DKPathComponentSeparator )
                break;
            
            end = i;
        }

        // Find the last slash before end
        int32_t start = end;
        
        while( start > 0 )
        {
            int32_t i = start;
            
            U8_PREV( path, 0, i, ch );
            
            if( ch == DKPathComponentSeparator )
                break;
            
            start = i;
        }
        
        // No separator
        if( end == 0 )
        {
            // Use the copying interface here because immutable strings can be retained
            // rather than copied
            DKCopying * copying = DKGetInterface( _self, DKSelector(Copying) );
            return copying->copy( _self );
        }
        
        return CopySubstring( path, DKRangeMake( start, end - start ) );
    }
    
    return NULL;
}


///
//  DKStringCopyPathExtension()
//
DKStringRef DKStringCopyPathExtension( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        const char * path = (const char *)_self->byteArray.data;
        const char * ext = dk_ustrrchr( path, DKPathExtensionSeparator );
        
        // No extension
        if( ext == NULL )
            return DKSTR( "" );
        
        const char * lastSlash = dk_ustrrchr( path, DKPathComponentSeparator );
        
        // No extension in last path component
        if( lastSlash && (ext < lastSlash) )
            return DKSTR( "" );
        
        DKRange range;
        range.location = ext - path + 1;
        range.length = _self->byteArray.length - range.location;
        
        return CopySubstring( path, range );
    }
    
    return NULL;
}


///
//  DKStringAppendPathComponent()
//
void DKStringAppendPathComponent( DKMutableStringRef _self, DKStringRef pathComponent )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );
        DKAssertKindOfClass( pathComponent, DKStringClass() );

        if( _self->byteArray.length > 0 )
        {
            const char separator[2] = { DKPathComponentSeparator, '\0' };
            DKByteArrayAppendBytes( &_self->byteArray, (const uint8_t *)separator, 1 );
        }

        DKByteArrayAppendBytes( &_self->byteArray, pathComponent->byteArray.data, pathComponent->byteArray.length );

        DKStringStandardizePath( _self );
    }
}


///
//  DKStringRemoveLastPathComponent()
//
void DKStringRemoveLastPathComponent( DKMutableStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );

        DKStringStandardizePath( _self );

        const char * path = (const char *)_self->byteArray.data;

        // Empty or root path
        if( (_self->byteArray.length == 0) || ((_self->byteArray.length == 1) && (path[0] == DKPathComponentSeparator)) )
            return;
        
        const char * lastSlash = dk_ustrrchr( path, DKPathComponentSeparator );

        DKRange range;
        
        // No separator
        if( lastSlash == NULL )
        {
            range = DKRangeMake( 0, _self->byteArray.length );
        }
            
        // Keep the first slash of an absolute path
        else if( lastSlash == path )
        {
            range.location = lastSlash - path + 1;
            range.length = _self->byteArray.length - range.location;
        }
        
        else
        {
            range.location = lastSlash - path;
            range.length = _self->byteArray.length - range.location;
        }
        
        DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, 0 );
    }
}


///
//  DKStringAppendPathExtension()
//
void DKStringAppendPathExtension( DKMutableStringRef _self, DKStringRef extension )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );
        DKAssertKindOfClass( extension, DKStringClass() );

        DKStringStandardizePath( _self );
        
        const char * path = (const char *)_self->byteArray.data;
        
        // Empty or root path
        if( (_self->byteArray.length == 0) || ((_self->byteArray.length == 1) && (path[0] == DKPathComponentSeparator)) )
        {
            DKError( "%s: Cannot append path extension '%s' to path '%s'\n",
                __func__, (const char *)extension->byteArray.data, path );
            return;
        }

        const char separator[2] = { DKPathExtensionSeparator, '\0' };
        DKByteArrayAppendBytes( &_self->byteArray, (const uint8_t *)separator, 1 );
        
        DKByteArrayAppendBytes( &_self->byteArray, extension->byteArray.data, extension->byteArray.length );
    }
}


///
//  DKStringRemovePathExtension()
//
void DKStringRemovePathExtension( DKMutableStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );

        DKStringStandardizePath( _self );

        const char * path = (const char *)_self->byteArray.data;
        const char * ext = dk_ustrrchr( path, DKPathExtensionSeparator );
        
        // No extension
        if( ext == NULL )
            return;
        
        const char * lastSlash = dk_ustrrchr( path, DKPathComponentSeparator );
        
        // No extension in last path component
        if( lastSlash && (ext < lastSlash) )
            return;
        
        DKRange range;
        range.location = ext - path;
        range.length = _self->byteArray.length - range.location;
        
        DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, 0 );
    }
}


///
//  DKStringStandardizePath()
//
void DKStringStandardizePath( DKMutableStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );

        // Remove redundant slashes
        char * dst = (char *)_self->byteArray.data;
        const char * src = dst;
        int wasslash = 0;
        
        char32_t ch;
        size_t n;
        
        while( (n = dk_ustrscan( src, &ch )) != 0 )
        {
            int isslash = (ch == DKPathComponentSeparator);
            
            if( !wasslash || !isslash )
            {
                for( size_t i = 0; i < n; ++i )
                    *dst++ = src[i];
            }
            
            wasslash = isslash;
            src += n;
        }
        
        DKRange range;
        range.location = (uint8_t *)dst - _self->byteArray.data;
        
        if( wasslash && (range.location > 1) )
            range.location--;
        
        range.length = _self->byteArray.length - range.location;
        
        DKByteArrayReplaceBytes( &_self->byteArray, range, NULL, 0 );
    }
}


///
//  DKStringSeek()
//
int DKStringSeek( DKStringRef _self, DKIndex offset, int origin )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        DKIndex cursor = _self->cursor;
        
        if( origin == DKSeekSet )
            cursor = offset;
        
        else if( origin == DKSeekCur )
            cursor += offset;
        
        else
            cursor = _self->byteArray.length + cursor;

        SetCursor( _self, cursor );
        
        return 0;
    }
    
    return -1;
}


///
//  DKStringTell()
//
DKIndex DKStringTell( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        return _self->cursor;
    }
    
    return -1;
}


///
//  DKStringRead()
//
DKIndex DKStringRead( DKStringRef _self, void * buffer, DKIndex size, DKIndex count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        SetCursor( (struct DKString *)_self, _self->cursor );
        
        DKRange range = DKRangeMake( _self->cursor, size * count );
        
        if( range.length > (_self->byteArray.length - _self->cursor) )
            range.length = _self->byteArray.length - _self->cursor;
        
        memcpy( buffer, &_self->byteArray.data[range.location], range.length );
        
        SetCursor( (struct DKString *)_self, _self->cursor + range.length );
        
        return range.length / size;
    }
    
    return 0;
}


///
//  DKStringWrite()
//
static DKIndex DKImmutableStringWrite( DKMutableObjectRef _self, const void * buffer, DKIndex size, DKIndex count )
{
    DKError( "DKStringWrite: Trying to modify an immutable object." );
    return 0;
}

DKIndex DKStringWrite( DKMutableStringRef _self, const void * buffer, DKIndex size, DKIndex count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );

        SetCursor( _self, _self->cursor );
        
        DKRange range = DKRangeMake( _self->cursor, size * count );
        
        if( range.length > (_self->byteArray.length - _self->cursor) )
            range.length = _self->byteArray.length - _self->cursor;
        
        DKByteArrayReplaceBytes( &_self->byteArray, range, buffer, size * count );

        SetCursor( _self, _self->cursor + (size * count) );
        
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
        DKStaticObject( constantStringClass ),
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
        DKHashTableInsertObject( DKConstantStringTable, newConstantString, newConstantString, DKInsertIfNotFound );
        
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


