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
#include "DKStream.h"
#include "DKGenericHashTable.h"
#include "DKList.h"
#include "DKArray.h"
#include "DKEgg.h"
#include "DKAllocation.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKFile.h"

#include "icu/unicode/utf8.h"


struct DKString
{
    DKObject _obj;          // 24 bytes
    DKByteArray byteArray;  // 24 bytes
    DKHashCode hashCode;    // 8 bytes
    DKIndex cursor;         // 8 bytes
};

static void *       DKStringAllocPlaceholder( DKClassRef _class, size_t extraBytes );
static void         DKStringDealloc( DKStringRef _self );

static DKObjectRef  DKStringInit( DKObjectRef _self );
static void         DKStringFinalize( DKObjectRef _self );

static DKObjectRef  DKStringInitWithEgg( DKStringRef _self, DKEggUnarchiverRef egg );
static void         DKStringAddToEgg( DKStringRef _self, DKEggArchiverRef egg );

static DKStringRef  DKMutableStringGetDescription( DKMutableStringRef _self );

static struct DKString DKPlaceholderString =
{
    DKInitStaticObjectHeader( NULL ),
    { NULL, 0, 0 },
    0,
    0
};

static struct DKString DKPlaceholderConstantString =
{
    DKInitStaticObjectHeader( NULL ),
    { NULL, 0, 0 },
    0,
    0
};




// Class Methods =========================================================================

///
//  DKStringClass()
//
DKThreadSafeClassInit( DKStringClass )
{
    // Since constant strings are used for class and selector names, the name fields of
    // DKString and DKConstantString are initialized in DKRuntimeInit().
    DKClassRef cls = DKNewClass( NULL, DKObjectClass(), sizeof(struct DKString), DKImmutableInstances, DKStringInit, DKStringFinalize );

    // Allocation
    struct DKAllocationInterface * allocation = DKNewInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->alloc = (DKAllocMethod)DKStringAllocPlaceholder;
    allocation->dealloc = (DKDeallocMethod)DKStringDealloc;

    DKInstallClassInterface( cls, allocation );
    DKRelease( allocation );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKNewInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKStringEqual;
    comparison->compare = (DKCompareMethod)DKStringCompare;
    comparison->hash = (DKHashMethod)DKStringHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );
    
    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKStringMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Description
    struct DKDescriptionInterface * description = DKNewInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKGetSelf;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );
    
    // Stream
    struct DKStreamInterface * stream = DKNewInterface( DKSelector(Stream), sizeof(struct DKStreamInterface) );
    stream->seek = (DKStreamSeekMethod)DKStringSeek;
    stream->tell = (DKStreamTellMethod)DKStringTell;
    stream->read = (DKStreamReadMethod)DKStringRead;
    stream->write = (void *)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );

    // Egg
    struct DKEggInterface * egg = DKNewInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKStringInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKStringAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKConstantStringClass()
//
DKThreadSafeClassInit( DKConstantStringClass )
{
    // Since constant strings are used for class and selector names, the name fields of
    // DKString and DKConstantString are initialized in DKRuntimeInit().
    DKClassRef cls = DKNewClass( NULL, DKStringClass(), sizeof(struct DKString), DKPreventSubclassing | DKDisableReferenceCounting | DKImmutableInstances, NULL, NULL );
    
    return cls;
}


///
//  DKMutableStringClass()
//
DKThreadSafeClassInit( DKMutableStringClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKMutableString" ), DKStringClass(), sizeof(struct DKString), 0, NULL, NULL );
    
    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = (DKCopyMethod)DKStringMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKStringMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Description
    struct DKDescriptionInterface * description = DKNewInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKMutableStringGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );
    
    // Stream
    struct DKStreamInterface * stream = DKNewInterface( DKSelector(Stream), sizeof(struct DKStreamInterface) );
    stream->seek = (DKStreamSeekMethod)DKStringSeek;
    stream->tell = (DKStreamTellMethod)DKStringTell;
    stream->read = (DKStreamReadMethod)DKStringRead;
    stream->write = (DKStreamWriteMethod)DKStringWrite;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );
    
    return cls;
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
//  UpdateHash()
//
static void UpdateHash( DKStringRef _self )
{
    if( _self->hashCode == 0 )
    {
        if( _self->byteArray.length > 0 )
            _self->hashCode = dk_strhash( (const char *)_self->byteArray.bytes );
    }
}


///
//  TestEquality()
//
static bool TestEquality( DKStringRef _self, DKStringRef other )
{
    if( _self == other )
        return true;

    UpdateHash( _self );
    UpdateHash( other );

    if( _self->hashCode != other->hashCode )
        return false;
    
    return strcmp( (const char *)_self->byteArray.bytes, (const char *)other->byteArray.bytes ) == 0;
}


///
//  AppendBytes()
//
static void AppendBytes( DKStringRef _self, const void * bytes, DKIndex length )
{
    DKByteArrayAppendBytes( &_self->byteArray, bytes, length );
    _self->hashCode = 0;
}


///
//  ReplaceBytes()
//
static void ReplaceBytes( DKStringRef _self, DKRange range, const void * bytes, DKIndex length )
{
    DKByteArrayReplaceBytes( &_self->byteArray, range, bytes, length );
    _self->hashCode = 0;
}


///
//  CopySubstring()
//
static DKStringRef CopySubstring( const char * cstr, DKRange byteRange )
{
    if( (cstr == NULL) || (byteRange.length == 0) )
        return DKSTR( "" );

    struct DKString * str = DKAllocObject( DKStringClass(), byteRange.length + 1 );

    DKByteArrayInitWithExternalStorage( &str->byteArray, (const void *)(str + 1), byteRange.length );
        
    memcpy( str->byteArray.bytes, &cstr[byteRange.location], byteRange.length );
    str->byteArray.bytes[byteRange.length] = '\0';

    return str;
}


///
//  InitString()
//
static void * InitString( DKStringRef _self, const char * cstr, DKIndex length )
{
    if( _self == &DKPlaceholderString  )
    {
        _self = CopySubstring( cstr, DKRangeMake( 0, length ) );
    }
    
    else if( _self == &DKPlaceholderConstantString )
    {
        // Note: constant strings must be externally allocated, which we can't
        // guarantee in this case
        _self = __DKStringGetConstantString( cstr, false );

        if( !_self )
            _self = CopySubstring( cstr, DKRangeMake( 0, length ) );
    }
    
    else if( _self && (_self->_obj.isa == DKMutableStringClass_SharedObject) )
    {
        DKByteArrayInit( (DKByteArray *)&_self->byteArray );
        AppendBytes( _self, (const uint8_t *)cstr, length );
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKStringInit: Trying to initialize a non-string object.\n" );
    }

    return _self;
}





// DKString Interface ====================================================================

///
//  DKStringAllocPlaceholder()
//
static void * DKStringAllocPlaceholder( DKClassRef _class, size_t extraBytes )
{
    if( _class == DKStringClass_SharedObject )
    {
        DKPlaceholderString._obj.isa = DKStringClass_SharedObject;
        return &DKPlaceholderString;
    }
    
    else if( _class == DKConstantStringClass_SharedObject )
    {
        DKPlaceholderConstantString._obj.isa = DKConstantStringClass_SharedObject;
        return &DKPlaceholderConstantString;
    }
    
    else if( _class == DKMutableStringClass_SharedObject )
    {
        return DKAllocObject( _class, 0 );
    }
    
    DKAssert( 0 );
    return NULL;
}


///
//  DKStringDealloc()
//
static void DKStringDealloc( DKStringRef _self )
{
    if( (_self == &DKPlaceholderString) || (_self == &DKPlaceholderConstantString) )
        return;
    
    DKDeallocObject( _self );
}


///
//  DKStringInit()
//
static DKObjectRef DKStringInit( DKObjectRef _self )
{
    return InitString( _self, NULL, 0 );
}


///
//  DKStringFinalize()
//
static void DKStringFinalize( DKObjectRef _self )
{
    if( (_self == &DKPlaceholderString) || (_self == &DKPlaceholderConstantString) )
    {
        DKFatalError( "DKStringFinalize: Trying to finalize a string that was never initialized.\n" );
        return;
    }

    struct DKString * string = (struct DKString *)_self;
    DKByteArrayFinalize( &string->byteArray );
}


///
//  DKStringInitWithString()
//
void * DKStringInitWithString( DKStringRef _self, DKStringRef other )
{
    const char * cstr = "";
    DKIndex length = 0;
    DKHashCode hashCode = 0;
    
    if( other )
    {
        DKAssertKindOfClass( other, DKStringClass() );
        cstr = (const char *)other->byteArray.bytes;
        length = other->byteArray.length;
        hashCode = other->hashCode;
    }
    
    _self = InitString( _self, cstr, length );
    _self->hashCode = hashCode;
    
    return _self;
}


///
//  DKStringInitWithCString()
//
void * DKStringInitWithCString( DKStringRef _self, const char * cstr )
{
    size_t length = strlen( cstr );
    return InitString( _self, cstr, length );
}


///
//  DKStringInitWithCStringNoCopy()
//
void * DKStringInitWithCStringNoCopy( DKStringRef _self, const char * cstr )
{
    DKIndex length = strlen( cstr );

    if( (_self == &DKPlaceholderString) || (_self == &DKPlaceholderConstantString) )
    {
        _self = DKAllocObject( DKStringClass(), 0 );
        DKByteArrayInitWithExternalStorage( &_self->byteArray, (const uint8_t *)cstr, length );
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKStringInitWithCStringNoCopy: Trying to initialize a non-immutable string object.\n" );
    }

    return _self;
}


///
//  DKStringInitWithFormat()
//
void * DKStringInitWithFormat( DKStringRef _self, const char * format, ... )
{
    if( (_self == &DKPlaceholderString) || (_self == &DKPlaceholderConstantString)  )
    {
        _self = DKAllocObject( DKMutableStringClass(), 0 );
        DKByteArrayInit( &_self->byteArray );

        va_list arg_ptr;
        va_start( arg_ptr, format );
        
        DKVSPrintf( _self, format, arg_ptr );
        SetCursor( _self, 0 );

        va_end( arg_ptr );

        _self = DKStringMakeImmutable( _self );
    }
    
    else if( DKIsMemberOfClass( _self, DKMutableStringClass() ) )
    {
        DKByteArrayInit( &_self->byteArray );

        va_list arg_ptr;
        va_start( arg_ptr, format );
        
        DKVSPrintf( _self, format, arg_ptr );
        SetCursor( _self, 0 );

        va_end( arg_ptr );
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKStringInit: Trying to initialize a non-string object.\n" );
    }
    
    return _self;
}


///
//  DKStringInitWithContentsOfFile()
//
DKObjectRef DKStringInitWithContentsOfFile( DKStringRef _self, DKStringRef filename )
{
    if( (_self == &DKPlaceholderString) || (_self == &DKPlaceholderConstantString)  )
    {
        _self = DKAllocObject( DKStringClass(), 0 );
    }
    
    else if( DKIsMemberOfClass( _self, DKMutableStringClass() ) )
    {
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKStringInit: Trying to initialize a non-string object.\n" );
    }
    
    if( _self )
    {
        DKByteArrayInit( &_self->byteArray );

        DKFileRef file = DKFileOpen( filename, "r" );
        
        if( file )
        {
            DKIndex length = DKFileGetLength( file );
        
            if( length > 0 )
            {
                DKByteArraySetLength( &_self->byteArray, length );

                void * buffer = DKByteArrayGetBytePtr( &_self->byteArray, 0 );
                DKFileRead( file, buffer, 1, length );
            }

            DKFileClose( file );
        }
    }
    
    return _self;
}


///
//  DKStringInitWithEgg()
//
static DKObjectRef DKStringInitWithEgg( DKStringRef _self, DKEggUnarchiverRef egg )
{
    size_t length = 0;
    const char * cstr = DKEggGetTextDataPtr( egg, DKSTR( "str" ), &length );

    return InitString( _self, cstr, length );
}


///
//  DKStringAddToEgg()
//
static void DKStringAddToEgg( DKStringRef _self, DKEggArchiverRef egg )
{
    DKIndex length = _self->byteArray.length;
    
    if( length > 0 )
        DKEggAddTextData( egg, DKSTR( "str" ), (const char *)_self->byteArray.bytes, length );
}


///
//  DKStringMakeImmutable()
//
DKStringRef DKStringMakeImmutable( DKMutableStringRef _self )
{
    if( DKIsMemberOfClass( _self, DKMutableStringClass() ) )
    {
        DKRelease( _self->_obj.isa );
        _self->_obj.isa = DKRetain( DKStringClass() );
    }
    
    return _self;
}


///
//  DKMutableStringGetDescription()
//
static DKStringRef DKMutableStringGetDescription( DKMutableStringRef _self )
{
    return DKAutorelease( DKStringCopy( _self ) );
}


///
//  DKStringCopy()
//
DKStringRef DKStringCopy( DKStringRef _self )
{
    return DKStringInitWithString( DKAlloc( DKStringClass() ), _self );
}


///
//  DKStringMutableCopy()
//
DKMutableStringRef DKStringMutableCopy( DKStringRef _self )
{
    return DKStringInitWithString( DKAlloc( DKMutableStringClass() ), _self );
}


///
//  DKStringEqual()
//
bool DKStringEqual( DKStringRef _self, DKObjectRef other )
{
    if( DKIsKindOfClass( other, DKStringClass() ) )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        return TestEquality( _self, other );
    }
    
    return false;
}


///
//  DKStringCompare()
//
int DKStringCompare( DKStringRef _self, DKObjectRef other )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        // DKCompare requires that the objects have some strict ordering property useful
        // for comparison, yet has no way of checking if the objects actually meet that
        // requirement.
        if( DKIsKindOfClass( other, DKStringClass() ) )
        {
            DKStringRef otherString = other;
            return dk_ustrcmp( (const char *)_self->byteArray.bytes, (const char *)otherString->byteArray.bytes );
        }
    }
    
    return DKPointerCompare( _self, other );
}


///
//  DKStringEqualToString()
//
bool DKStringEqualToString( DKStringRef _self, DKStringRef other )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        DKAssertKindOfClass( other, DKStringClass() );

        return TestEquality( _self, other );
    }
    
    return false;
}


///
//  DKStringCompareString()
//
int DKStringCompareString( DKStringRef _self, DKStringRef other )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        DKAssertKindOfClass( other, DKStringClass() );
    
        if( _self == other )
            return 0;

        return dk_ustrcmp( (const char *)_self->byteArray.bytes, (const char *)other->byteArray.bytes );
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

        UpdateHash( _self );
        
        return _self->hashCode;
    }
    
    return 0;
}


///
//  DKStringHasPrefix()
//
bool DKStringHasPrefix( DKStringRef _self, DKStringRef other )
{
    DKRange substring = DKStringGetRangeOfString( _self, other, 0 );
    return substring.location == 0;
}


///
//  DKStringHasSuffix()
//
bool DKStringHasSuffix( DKStringRef _self, DKStringRef other )
{
    DKIndex a = DKStringGetLength( _self );
    DKIndex b = DKStringGetLength( other );
    
    if( a >= b )
    {
        DKRange substring = DKStringGetRangeOfString( _self, other, a - b );
        return substring.location != DKNotFound;
    }
    
    return false;
}


///
//  DKStringHasSubstring()
//
bool DKStringHasSubstring( DKStringRef _self, DKStringRef other )
{
    DKRange substring = DKStringGetRangeOfString( _self, other, 0 );
    return substring.location != DKNotFound;
}


///
//  DKStringGetLength()
//
DKIndex DKStringGetLength( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        return dk_ustrlen( (const char *)_self->byteArray.bytes );
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
        
        return _self->byteArray.length;
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
        return (const char *)_self->byteArray.bytes;
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
                const uint8_t * loc = (const uint8_t *)dk_ustridx( (const char *)_self->byteArray.bytes, range.location );
                
                if( loc )
                {
                    const uint8_t * end = (const uint8_t *)dk_ustridx( (const char *)loc, range.length );
                    
                    if( end )
                    {
                        DKRange byteRange;
                        byteRange.location = loc - _self->byteArray.bytes;
                        byteRange.length = end - loc;

                        return CopySubstring( (const char *)_self->byteArray.bytes, byteRange );
                    }
                }
            }

            DKError( "%s: Range %ld,%ld is outside 0,%ld\n", __func__,
                range.location, range.length, dk_ustrlen( (const char *)_self->byteArray.bytes ) );
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
                const uint8_t * loc = (const uint8_t *)dk_ustridx( (const char *)_self->byteArray.bytes, index );
                    
                if( loc )
                {
                    DKRange byteRange;
                    byteRange.location = loc - _self->byteArray.bytes;
                    byteRange.length = _self->byteArray.length - byteRange.location;

                    return CopySubstring( (const char *)_self->byteArray.bytes, byteRange );
                }
            }
        }

        DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
            index, dk_ustrlen( (const char *)_self->byteArray.bytes ) );
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
                const uint8_t * end = (const uint8_t *)dk_ustridx( (const char *)_self->byteArray.bytes, index );
                    
                if( end )
                {
                    DKRange byteRange;
                    byteRange.location = 0;
                    byteRange.length = end - _self->byteArray.bytes;

                    return CopySubstring( (const char *)_self->byteArray.bytes, byteRange );
                }
            }
        }

        DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
            index, dk_ustrlen( (const char *)_self->byteArray.bytes ) );
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

        if( _self->byteArray.length > 0 )
        {
            const char * s = (const char *)_self->byteArray.bytes;
        
            if( startLoc > 0 )
            {
                s = dk_ustridx( s, startLoc );
                
                if( s == NULL )
                {
                    DKError( "%s: Index %ld is outside 0,%ld\n", __func__,
                        startLoc, dk_ustrlen( (const char *)_self->byteArray.bytes ) );
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
    DKMutableArrayRef array = DKNew( DKMutableArrayClass() );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        DKAssertKindOfClass( separator, DKStringClass() );
        
        const char * str = (const char *)_self->byteArray.bytes;
        const char * sep = (const char *)separator->byteArray.bytes;
        size_t len = strlen( sep );
        
        const char * a = str;
        const char * b = dk_ustrstr( a, sep );
        
        while( a < b )
        {
            DKStringRef copy = CopySubstring( a, DKRangeMake( 0, b - a ) );
            DKArrayAppendCArray( array, (DKObjectRef *)&copy, 1 );
            DKRelease( copy );
            
            a = b + len;
            b = dk_ustrstr( a, sep );
        }
        
        if( *a != '\0' )
        {
            DKStringRef copy = CopySubstring( a, DKRangeMake( 0, strlen( a ) ) );
            DKArrayAppendCArray( array, (DKObjectRef *)&copy, 1 );
            DKRelease( copy );
        }
    }
    
    return (DKListRef)array;
}


///
//  DKStringCreateByCombiningStrings()
//
DKStringRef DKStringCreateByCombiningStrings( DKListRef list, DKStringRef separator )
{
    DKMutableStringRef combinedString = DKNewMutableString();

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
        DKCheckKindOfClass( _self, DKMutableStringClass() );
        
        DKRange range = DKRangeMake( 0, _self->byteArray.length );
        
        if( str )
        {
            DKAssertKindOfClass( str, DKStringClass() );
            const struct DKString * src = str;

            ReplaceBytes( _self, range, src->byteArray.bytes, src->byteArray.length );
        }
        
        else
        {
            ReplaceBytes( _self, range, NULL, 0 );
        }
    }
}


///
//  DKStringSetCString()
//
void DKStringSetCString( DKMutableStringRef _self, const char * cstr )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );

        DKRange range = DKRangeMake( 0, _self->byteArray.length );
        
        if( cstr )
        {
            size_t length = strlen( cstr );
            ReplaceBytes( _self, range, cstr, length );
        }
        
        else
        {
            ReplaceBytes( _self, range, NULL, 0 );
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
        DKCheckKindOfClass( _self, DKMutableStringClass() );
        DKAssertKindOfClass( str, DKStringClass() );
        
        DKRange range = DKRangeMake( _self->byteArray.length, 0 );
        ReplaceBytes( _self, range, str->byteArray.bytes, str->byteArray.length );
        
        SetCursor( _self, _self->byteArray.length );
    }
}


///
//  DKStringAppendFormat()
//
void DKStringAppendFormat( DKMutableStringRef _self, const char * format, ... )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );

        SetCursor( _self, _self->byteArray.length );

        va_list arg_ptr;
        va_start( arg_ptr, format );
        
        DKVSPrintf( _self, format, arg_ptr );
        
        va_end( arg_ptr );
    }
}


///
//  DKStringAppendCString()
//
void DKStringAppendCString( DKMutableStringRef _self, const char * cstr )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );

        size_t length = strlen( cstr );
        DKRange range = DKRangeMake( _self->byteArray.length, 0 );
        ReplaceBytes( _self, range, cstr, length );
        
        SetCursor( _self, _self->byteArray.length );
    }
}


///
//  DKStringReplaceSubstring()
//
void DKStringReplaceSubstring( DKMutableStringRef _self, DKRange range, DKStringRef str )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );
        DKAssertKindOfClass( str, DKStringClass() );
        DKCheckRange( range, _self->byteArray.length );
        
        ReplaceBytes( _self, range, str->byteArray.bytes, str->byteArray.length );
    }
}


///
//  DKStringReplaceOccurrencesOfString()
//
void DKStringReplaceOccurrencesOfString( DKMutableStringRef _self, DKStringRef pattern, DKStringRef replacement )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );
        
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
        DKCheckKindOfClass( _self, DKMutableStringClass() );
        DKCheckRange( range, _self->byteArray.length );
        
        ReplaceBytes( _self, range, NULL, 0 );
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
        
        return (_self->byteArray.length > 0) && (_self->byteArray.bytes[0] == DKPathComponentSeparator);
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

        const char * path = (const char *)_self->byteArray.bytes;

        // Empty or root path
        if( (_self->byteArray.length == 0) || ((_self->byteArray.length == 1) && (path[0] == DKPathComponentSeparator)) )
            return DKSTR( "" );
        
        // Skip over trailing slashes
        int32_t end = (int32_t)_self->byteArray.length;
        DKChar32 ch;
        
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
            DKCopyingInterfaceRef copying = DKGetInterface( _self, DKSelector(Copying) );
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

        const char * path = (const char *)_self->byteArray.bytes;
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
        DKCheckKindOfClass( _self, DKMutableStringClass() );
        DKAssertKindOfClass( pathComponent, DKStringClass() );

        if( _self->byteArray.length > 0 )
        {
            const char separator[2] = { DKPathComponentSeparator, '\0' };
            AppendBytes( _self, (const uint8_t *)separator, 1 );
        }

        AppendBytes( _self, pathComponent->byteArray.bytes, pathComponent->byteArray.length );

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
        DKCheckKindOfClass( _self, DKMutableStringClass() );

        DKStringStandardizePath( _self );

        const char * path = (const char *)_self->byteArray.bytes;

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
        
        ReplaceBytes( _self, range, NULL, 0 );
    }
}


///
//  DKStringAppendPathExtension()
//
void DKStringAppendPathExtension( DKMutableStringRef _self, DKStringRef extension )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );
        DKAssertKindOfClass( extension, DKStringClass() );

        DKStringStandardizePath( _self );
        
        const char * path = (const char *)_self->byteArray.bytes;
        
        // Empty or root path
        if( (_self->byteArray.length == 0) || ((_self->byteArray.length == 1) && (path[0] == DKPathComponentSeparator)) )
        {
            DKError( "%s: Cannot append path extension '%s' to path '%s'\n",
                __func__, (const char *)extension->byteArray.bytes, path );
            return;
        }

        const char separator[2] = { DKPathExtensionSeparator, '\0' };
        AppendBytes( _self, (const uint8_t *)separator, 1 );
        
        AppendBytes( _self, extension->byteArray.bytes, extension->byteArray.length );
    }
}


///
//  DKStringRemovePathExtension()
//
void DKStringRemovePathExtension( DKMutableStringRef _self )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );

        DKStringStandardizePath( _self );

        const char * path = (const char *)_self->byteArray.bytes;
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
        
        ReplaceBytes( _self, range, NULL, 0 );
    }
}


///
//  DKStringStandardizePath()
//
void DKStringStandardizePath( DKMutableStringRef _self )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );

        // Remove redundant slashes
        char * dst = (char *)_self->byteArray.bytes;
        const char * src = dst;
        int wasslash = 0;
        
        DKChar32 ch;
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
        range.location = (uint8_t *)dst - _self->byteArray.bytes;
        
        if( wasslash && (range.location > 1) )
            range.location--;
        
        range.length = _self->byteArray.length - range.location;
        
        ReplaceBytes( _self, range, NULL, 0 );
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

        SetCursor( _self, _self->cursor );
        
        DKRange range = DKRangeMake( _self->cursor, size * count );
        
        if( range.length > (_self->byteArray.length - _self->cursor) )
            range.length = _self->byteArray.length - _self->cursor;
        
        memcpy( buffer, &_self->byteArray.bytes[range.location], range.length );
        
        SetCursor( _self, _self->cursor + range.length );
        
        return range.length / size;
    }
    
    return 0;
}


///
//  DKStringWrite()
//
DKIndex DKStringWrite( DKMutableStringRef _self, const void * buffer, DKIndex size, DKIndex count )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass(), 0 );

        SetCursor( _self, _self->cursor );
        
        DKRange range = DKRangeMake( _self->cursor, size * count );
        
        if( range.length > (_self->byteArray.length - _self->cursor) )
            range.length = _self->byteArray.length - _self->cursor;
        
        ReplaceBytes( _self, range, buffer, size * count );

        SetCursor( _self, _self->cursor + (size * count) );
        
        return count;
    }
    
    return 0;
}




// Constant Strings ======================================================================
static DKGenericHashTable * DKConstantStringTable = NULL;
static DKSpinLock DKConstantStringTableLock = DKSpinLockInit;


static DKRowStatus DKConstantStringTableRowStatus( const void * _row )
{
    struct DKString * const * row = _row;
    return (DKRowStatus)DK_HASHTABLE_ROW_STATUS( *row );
}

static DKHashCode DKConstantStringTableRowHash( const void * _row )
{
    struct DKString * const * row = _row;
    return (*row)->hashCode;
}

static bool DKConstantStringTableRowEqual( const void * _row1, const void * _row2 )
{
    struct DKString * const * row1 = _row1;
    struct DKString * const * row2 = _row2;

    return DKStringEqualToString( *row1, *row2 );
}

static void DKConstantStringTableRowInit( void * _row )
{
    struct DKString ** row = _row;
    *row = DK_HASHTABLE_EMPTY_KEY;
}

static void DKConstantStringTableRowUpdate( void * _row, const void * _src )
{
    struct DKString ** row = _row;
    struct DKString * const * src = _src;
    *row = *src;
}

static void DKConstantStringTableRowDelete( void * _row )
{
    struct DKString ** row = _row;
    *row = DK_HASHTABLE_DELETED_KEY;
}


///
//  __DKStringGetConstantString()
//
DKStringRef __DKStringGetConstantString( const char * str, bool insert )
{
    if( DKConstantStringTable == NULL )
    {
        DKGenericHashTable * table = dk_malloc( sizeof(DKGenericHashTable) );

        DKGenericHashTableCallbacks callbacks =
        {
            DKConstantStringTableRowStatus,
            DKConstantStringTableRowHash,
            DKConstantStringTableRowEqual,
            DKConstantStringTableRowInit,
            DKConstantStringTableRowUpdate,
            DKConstantStringTableRowDelete
        };

        DKGenericHashTableInit( table, sizeof(DKStringRef), &callbacks );
        
        DKSpinLockLock( &DKConstantStringTableLock );
        
        if( DKConstantStringTable == NULL )
            DKConstantStringTable = table;
        
        DKSpinLockUnlock( &DKConstantStringTableLock );

        if( DKConstantStringTable != table )
        {
            DKGenericHashTableFinalize( table );
            dk_free( table );
        }
    }

    // Get the length and hash code of the string
    if( str == NULL )
        str = "";

    DKIndex length = strlen( str );
    DKHashCode hashCode = dk_strhash( str );

    // Create a temporary stack object for the table lookup
    DKClassRef constantStringClass = DKConstantStringClass();
    
    struct DKString _lookupString =
    {
        DKInitStaticObjectHeader( constantStringClass ),
    };
    
    struct DKString * lookupString = &_lookupString;
    
    DKByteArrayInitWithExternalStorage( &lookupString->byteArray, (const uint8_t *)str, length );
    lookupString->hashCode = hashCode;

    // Lookup the string in the hash table
    DKStringRef constantString;

    DKSpinLockLock( &DKConstantStringTableLock );
    const DKStringRef * entry = DKGenericHashTableFind( DKConstantStringTable, &lookupString );
    
    if( entry )
    {
        constantString = *entry;

        DKSpinLockUnlock( &DKConstantStringTableLock );

        return constantString;
    }
    
    DKSpinLockUnlock( &DKConstantStringTableLock );
    
    if( !insert )
        return NULL;

    // Create a new constant string
    DKStringRef newConstantString = DKAllocObject( constantStringClass, 0 );
    DKByteArrayInitWithExternalStorage( &newConstantString->byteArray, (const uint8_t *)str, length );
    newConstantString->hashCode = hashCode;

    // Try to insert it in the table
    DKSpinLockLock( &DKConstantStringTableLock );
    
    if( DKGenericHashTableInsert( DKConstantStringTable, &newConstantString, DKInsertIfNotFound ) )
    {
        constantString = newConstantString;
    }
    
    else
    {
        entry = DKGenericHashTableFind( DKConstantStringTable, &lookupString );
        constantString = *entry;
    }

    DKSpinLockUnlock( &DKConstantStringTableLock );

    // Discard the new string if we're not using it
    if( constantString != newConstantString )
        DKDeallocObject( newConstantString );
    
    return constantString;
}


