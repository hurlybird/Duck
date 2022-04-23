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
#include "DKBuffer.h"
#include "DKStream.h"
#include "DKGenericHashTable.h"
#include "DKList.h"
#include "DKDictionary.h"
#include "DKArray.h"
#include "DKHashTable.h"
#include "DKEgg.h"
#include "DKAllocation.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKConversion.h"
#include "DKFile.h"

#include "icu/unicode/utf8.h"


struct DKString
{
    DKObject _obj;
    DKByteArray byteArray;
    DKHashCode hashCode;
    DKIndex cursor;
};

static void *       DKStringAllocPlaceholder( DKClassRef _class, size_t extraBytes );
static void         DKStringDealloc( DKStringRef _self );

static DKObjectRef  DKStringInit( DKObjectRef _self );
static void         DKStringFinalize( DKObjectRef _self );

static DKObjectRef  DKStringInitWithEgg( DKObjectRef _self, DKEggUnarchiverRef egg );
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
    description->getSizeInBytes = (DKGetSizeInBytesMethod)DKStringGetByteLength;
    
    DKInstallInterface( cls, description );
    DKRelease( description );
    
    // Buffer
    struct DKBufferInterface * buffer = DKNewInterface( DKSelector(Buffer), sizeof(struct DKBufferInterface) );
    buffer->getLength = (DKBufferGetLengthMethod)DKStringGetByteLength;
    buffer->getBytePtr = (DKBufferGetBytePtrMethod)DKStringGetBytePtr;
    buffer->setLength = (DKBufferSetLengthMethod)DKImmutableObjectAccessError;
    buffer->getMutableBytePtr = (DKBufferGetMutableBytePtrMethod)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, buffer );
    DKRelease( buffer );

    // Stream
    struct DKStreamInterface * stream = DKNewInterface( DKSelector(Stream), sizeof(struct DKStreamInterface) );
    stream->seek = (DKStreamSeekMethod)DKStringSeek;
    stream->tell = (DKStreamTellMethod)DKStringTell;
    stream->read = (DKStreamReadMethod)DKStringRead;
    stream->write = (DKStreamWriteMethod)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, stream );
    DKRelease( stream );

    // Egg
    struct DKEggInterface * egg = DKNewInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKStringInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKStringAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    // Conversion
    struct DKConversionInterface * conv = DKNewInterface( DKSelector(Conversion), sizeof(struct DKConversionInterface) );
    conv->getString = (DKGetStringMethod)DKGetSelf;
    conv->getBool = (DKGetBoolMethod)DKStringGetBool;
    conv->getInt32 = (DKGetInt32Method)DKStringGetInt32;
    conv->getInt64 = (DKGetInt64Method)DKStringGetInt64;
    conv->getFloat = (DKGetFloatMethod)DKStringGetFloat;
    conv->getDouble = (DKGetDoubleMethod)DKStringGetDouble;
    
    DKInstallInterface( cls, conv );
    DKRelease( conv );
    

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
    copying->copy = (DKCopyMethod)DKStringCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKStringMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Description
    struct DKDescriptionInterface * description = DKNewInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKMutableStringGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );
    
    // Buffer
    struct DKBufferInterface * buffer = DKNewInterface( DKSelector(Buffer), sizeof(struct DKBufferInterface) );
    buffer->getLength = (DKBufferGetLengthMethod)DKStringGetByteLength;
    buffer->getBytePtr = (DKBufferGetBytePtrMethod)DKStringGetBytePtr;
    buffer->setLength = (DKBufferSetLengthMethod)DKStringSetByteLength;
    buffer->getMutableBytePtr = (DKBufferGetMutableBytePtrMethod)DKStringGetMutableBytePtr;
    
    DKInstallInterface( cls, buffer );
    DKRelease( buffer );

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
//  DKStringSetCursor()
//
static void DKStringSetCursor( const struct DKString * string, DKIndex cursor )
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

    // Note: Add a DKChar32 '\0' to mimic the string termination done by DKByteArray
    struct DKString * str = DKAllocObject( DKStringClass(), byteRange.length + 4 );

    DKByteArrayInitWithExternalStorage( &str->byteArray, (const void *)(str + 1), byteRange.length );
        
    memcpy( str->byteArray.bytes, &cstr[byteRange.location], byteRange.length );
    str->byteArray.bytes[byteRange.length] = '\0';
    str->byteArray.bytes[byteRange.length+1] = '\0';
    str->byteArray.bytes[byteRange.length+2] = '\0';
    str->byteArray.bytes[byteRange.length+3] = '\0';

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
        DKFatalError( "DKStringInit: Trying to initialize a non-string object." );
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
static void DKStringFinalize( DKObjectRef _untyped_self )
{
    DKStringRef _self = _untyped_self;

    if( (_self == &DKPlaceholderString) || (_self == &DKPlaceholderConstantString) )
    {
        DKFatalError( "DKStringFinalize: Trying to finalize a string that was never initialized." );
        return;
    }

    DKByteArrayFinalize( &_self->byteArray );
}


///
//  DKStringInitWithString()
//
void * DKStringInitWithString( DKObjectRef _untyped_self, DKStringRef other )
{
    DKStringRef _self = _untyped_self;

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
void * DKStringInitWithCString( DKObjectRef _untyped_self, const char * cstr )
{
    DKStringRef _self = _untyped_self;

    if( !cstr )
        cstr = "";

    size_t length = strlen( cstr );
    return InitString( _self, cstr, length );
}


///
//  DKStringInitWithCStringNoCopy()
//
void * DKStringInitWithCStringNoCopy( DKObjectRef _untyped_self, const char * cstr )
{
    DKStringRef _self = _untyped_self;

    if( !cstr )
        cstr = "";

    DKIndex length = strlen( cstr );

    if( (_self == &DKPlaceholderString) || (_self == &DKPlaceholderConstantString) )
    {
        _self = DKAllocObject( DKStringClass(), 0 );
        DKByteArrayInitWithExternalStorage( &_self->byteArray, (const uint8_t *)cstr, length );
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKStringInitWithCStringNoCopy: Trying to initialize a non-immutable string object." );
    }

    return _self;
}


///
//  DKStringInitWithBytes()
//
DKObjectRef DKStringInitWithBytes( DKObjectRef _untyped_self, const void * bytes, DKIndex length )
{
    DKStringRef _self = _untyped_self;

    return InitString( _self, bytes, length );
}


///
//  DKStringInitWithFormat()
//
void * DKStringInitWithFormat( DKObjectRef _untyped_self, const char * format, ... )
{
    DKStringRef _self = _untyped_self;

    if( (_self == &DKPlaceholderString) || (_self == &DKPlaceholderConstantString)  )
    {
        _self = DKAllocObject( DKMutableStringClass(), 0 );
        DKByteArrayInit( &_self->byteArray );

        va_list arg_ptr;
        va_start( arg_ptr, format );
        
        DKVSPrintf( _self, format, arg_ptr );
        DKStringSetCursor( _self, 0 );

        va_end( arg_ptr );

        _self = DKStringMakeImmutable( _self );
    }
    
    else if( DKIsMemberOfClass( _self, DKMutableStringClass() ) )
    {
        DKByteArrayInit( &_self->byteArray );

        va_list arg_ptr;
        va_start( arg_ptr, format );
        
        DKVSPrintf( _self, format, arg_ptr );
        DKStringSetCursor( _self, 0 );

        va_end( arg_ptr );
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKStringInit: Trying to initialize a non-string object." );
    }
    
    return _self;
}


///
//  DKStringInitWithContentsOfFile()
//
DKObjectRef DKStringInitWithContentsOfFile( DKObjectRef _untyped_self, DKObjectRef file )
{
    DKStringRef _self = _untyped_self;

    if( (_self == &DKPlaceholderString) || (_self == &DKPlaceholderConstantString)  )
    {
        _self = DKAllocObject( DKStringClass(), 0 );
    }
    
    else if( DKIsMemberOfClass( _self, DKMutableStringClass() ) )
    {
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKStringInit: Trying to initialize a non-string object." );
    }
    
    if( _self )
    {
        DKByteArrayInit( &_self->byteArray );

        DKFileRef fileHandle = NULL;
        
        if( DKIsKindOfClass( file, DKStringClass() ) )
        {
            file = fileHandle = DKFileOpen( file, "rb" );
        }
        
        if( DKIsKindOfClass( file, DKFileClass() ) )
        {
            DKIndex length = DKFileGetLength( file );
        
            if( length > 0 )
            {
                DKByteArraySetLength( &_self->byteArray, length );

                void * buffer = DKByteArrayGetBytePtr( &_self->byteArray, 0 );
                DKFileRead( file, buffer, 1, length );
            }
            
            DKFileClose( fileHandle );
        }
        
        else if( file != NULL )
        {
            DKError( "DKStringInitWithContentsOfFile: '%@' is not a file.", file );
        }
    }
    
    return _self;
}


///
//  DKStringInitWithCapacity()
//
DKMutableStringRef DKStringInitWithCapacity( DKObjectRef _untyped_self, DKIndex capacity )
{
    DKMutableStringRef _self = DKInit( _untyped_self );

    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableStringClass() );
        DKByteArrayReserve( &_self->byteArray, capacity );
    }
    
    return _self;
}


///
//  DKStringInitWithEgg()
//
static DKObjectRef DKStringInitWithEgg( DKObjectRef _untyped_self, DKEggUnarchiverRef egg )
{
    DKStringRef _self = _untyped_self;

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

        if( other )
        {
            DKAssertKindOfClass( other, DKStringClass() );
    
            if( _self == other )
                return 0;

            return dk_ustrcmp( (const char *)_self->byteArray.bytes, (const char *)other->byteArray.bytes );
        }

        return 1;
    }
    
    return -1;
}


///
//  DKStringCompareCString()
//
int DKStringCompareCString( DKStringRef _self, const char * cstr )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( cstr )
            return dk_ustrcmp( (const char *)_self->byteArray.bytes, cstr );

        return 1;
    }
    
    return -1;
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
//  DKStringIsEmptyString()
//
bool DKStringIsEmptyString( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        return _self->byteArray.length == 0;
    }
    
    return true;
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
//  DKStringSetByteLength()
//
void DKStringSetByteLength( DKMutableStringRef _self, DKIndex length )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );

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
//  DKStringGetCharacterAtIndex()
//
DKChar32 DKStringGetCharacterAtIndex( DKStringRef _self, DKIndex index, DKChar8 * utf8 )
{
    if( _self )
    {
        const char * str = (const char *)_self->byteArray.bytes;

        for( DKIndex i = 0; *str; i++ )
        {
            // Get the next character
            DKChar32 utf32;
            size_t bytes = dk_ustrscan( str, &utf32 );
            
            if( i == index )
            {
                // Copy the code point
                if( utf8 )
                {
                    // This size check is pedantic since UTF8 code points are
                    // always < 6 bytes long, but it squashes a warning in some
                    // versions of GCC GCC
                    if( bytes < sizeof(DKChar8) )
                    {
                        for( size_t j = 0; j < bytes; j++ )
                            utf8->s[j] = str[j];
                    
                        utf8->s[bytes] = '\0';
                    }

                    else
                    {
                        DKAssert( bytes < sizeof(DKChar8) );
                        utf8->s[0] = '\0';
                    }
                }
                
                return utf32;
            }
            
            str += bytes;
        }
    }
    
    if( utf8 )
        utf8->s[0] = '\0';

    return '\0';
}


///
//  DKStringGetBytePtr()
//
const void * DKStringGetBytePtr( DKStringRef _self, DKIndex index )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        DKCheckIndex( index, _self->byteArray.length, NULL );
        
        return &_self->byteArray.bytes[index];
    }
    
    return NULL;
}


///
//  DKStringGetMutableBytePtr()
//
void * DKStringGetMutableBytePtr( DKMutableStringRef _self, DKIndex index )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass(), NULL );
        DKCheckIndex( index, _self->byteArray.length, NULL );
        
        return &_self->byteArray.bytes[index];
    }
    
    return NULL;
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

            DKError( "%s: Range %ld,%ld is outside 0,%ld", __func__,
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

        DKError( "%s: Index %ld is outside 0,%ld", __func__,
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

        DKError( "%s: Index %ld is outside 0,%ld", __func__,
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
                    DKError( "%s: Index %ld is outside 0,%ld", __func__,
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
//  DKStringSplit()
//
DKListRef DKStringSplit( DKStringRef _self, DKStringRef separator )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        DKAssertKindOfClass( separator, DKStringClass() );

        DKMutableArrayRef array = DKMutableArray();

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

        return (DKListRef)array;
    }
    
    return NULL;
}


///
//  DKStringWrap()
//
DKListRef DKStringWrap( DKStringRef _self, size_t glyphsPerLine )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        DKMutableArrayRef array = DKMutableArray();
        DKMutableStringRef line = DKMutableString();

        const char * str = (const char *)_self->byteArray.bytes;
        const char * cur = str;

        size_t lineLength = 0;

        while( *cur )
        {
            size_t wordLengthInBytes = strcspn( cur, " \n\r\t" );
            size_t wordLength = dk_ustrnlen( cur, wordLengthInBytes );

            if( (lineLength > 0) && ((lineLength + wordLength) > glyphsPerLine) )
            {
                DKArrayAppendObject( array, DKStringByTrimmingWhitespace( line ) );
                DKStringSetByteLength( line, 0 );
                lineLength = 0;
            }


            DKStringAppendBytes( line, cur, wordLength );
            lineLength += wordLength;
            cur += wordLength;

            if( *cur == '\r' )
            {
                cur++;
            }

            else if( *cur == '\n' )
            {
                if( lineLength > 0 )
                {
                    DKArrayAppendObject( array, DKStringByTrimmingWhitespace( line ) );
                    DKStringSetByteLength( line, 0 );
                    lineLength = 0;
                }

                DKArrayAppendObject( array, DKSTR( "" ) );

                cur++;
            }

            else
            {
                size_t spaceLength = strspn( cur, " \t" );

                DKStringAppendBytes( line, cur, spaceLength );
                lineLength += spaceLength;
                cur += spaceLength;
            }
        }

        if( lineLength > 0 )
        {
            DKArrayAppendObject( array, DKStringByTrimmingWhitespace( line ) );
        }

        return (DKListRef)array;
    }

    return NULL;
}


///
//  DKStringCombine()
//
DKStringRef DKStringCombine( DKListRef list, DKStringRef separator )
{
    DKMutableStringRef combinedString = DKMutableString();

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
//  DKStringByTrimmingWhitespace()
//
DKStringRef DKStringByTrimmingWhitespace( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        const char * str = (const char *)_self->byteArray.bytes;
        DKIndex len = 0;

        DKRange byteRange = DKRangeMake( 0, 0 );
        
        while( *str )
        {
            DKChar32 ch;
            size_t bytes = dk_ustrscan( str, &ch );

            str += bytes;
            
            if( !isspace( ch ) )
            {
                len = bytes;
                byteRange.length = bytes;
                break;
            }
            
            byteRange.location += bytes;
        }
        
        
        while( *str )
        {
            DKChar32 ch;
            size_t bytes = dk_ustrscan( str, &ch );

            str += bytes;
            len += bytes;
            
            if( !isspace( ch ) )
                byteRange.length = len;
        }
        
        if( byteRange.length != _self->byteArray.length )
        {
            return DKAutorelease( CopySubstring( (const char *)_self->byteArray.bytes, byteRange ) );
        }
    }

    return DKAutorelease( DKCopy( _self ) );
}


///
//  DKStringByFilteringString()
//
DKStringRef DKStringByFilteringString( DKStringRef _self, ZLStringFilterFunction filter, void * context )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        DKMutableStringRef filteredString = DKMutableString();

        const char * str = (const char *)_self->byteArray.bytes;
        DKIndex rdx = 0;
        DKIndex wrx = 0;

        while( *str )
        {
            // Get the next character
            DKChar32 utf32;
            size_t bytes = dk_ustrscan( str, &utf32 );
            
            // Copy the code point
            DKChar8 utf8;
            DKAssert( bytes < sizeof(DKChar8) );
            
            for( size_t i = 0; i < bytes; i++ )
                utf8.s[i] = str[i];
            
            utf8.s[bytes] = '\0';
            
            // Filter the character
            DKFilterAction action = filter( rdx, wrx, utf8, utf32, context );
            
            if( action & DKFilterKeep )
            {
                DKByteArrayAppendBytes( &filteredString->byteArray, (void *)utf8.s, bytes );
                wrx++;
            }
            
            if( action & DKFilterStop )
            {
                break;
            }

            // Move the cursor
            str += bytes;
            rdx += 1;
        }
        
        DKStringMakeImmutable( filteredString );
        return filteredString;
    }

    return _self;
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

        DKStringSetCursor( _self, _self->byteArray.length );
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

        DKStringSetCursor( _self, _self->byteArray.length );
    }
}


///
//  DKStringSetBytes()
//
void DKStringSetBytes( DKMutableStringRef _self, const void * bytes, DKIndex length )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );

        DKRange range = DKRangeMake( 0, _self->byteArray.length );
        ReplaceBytes( _self, range, bytes, length );

        DKStringSetCursor( _self, _self->byteArray.length );
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
        
        DKStringSetCursor( _self, _self->byteArray.length );
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

        DKStringSetCursor( _self, _self->byteArray.length );

        va_list arg_ptr;
        va_start( arg_ptr, format );
        
        DKVSPrintf( _self, format, arg_ptr );
        
        va_end( arg_ptr );
    }
}


///
//  DKStringAppendFormatv()
//
void DKStringAppendFormatv( DKMutableStringRef _self, const char * format, va_list arg_ptr )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );

        DKStringSetCursor( _self, _self->byteArray.length );

        DKVSPrintf( _self, format, arg_ptr );
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
        
        DKStringSetCursor( _self, _self->byteArray.length );
    }
}


///
//  DKStringAppendBytes()
//
void DKStringAppendBytes( DKMutableStringRef _self, const void * bytes, DKIndex length )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );

        DKRange range = DKRangeMake( _self->byteArray.length, 0 );
        ReplaceBytes( _self, range, bytes, length );
        
        DKStringSetCursor( _self, _self->byteArray.length );
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
static void INTERNAL_DKStringReplaceOccurrencesOfString( DKMutableStringRef _self, DKStringRef pattern, DKStringRef replacement )
{
    DKRange range = DKStringGetRangeOfString( _self, pattern, 0 );
    DKIndex length = DKStringGetLength( replacement );
    
    while( range.location != DKNotFound )
    {
        DKStringReplaceSubstring( _self, range, replacement );
        range = DKStringGetRangeOfString( _self, pattern, range.location + length );
    }
}

void DKStringReplaceOccurrencesOfString( DKMutableStringRef _self, DKStringRef pattern, DKStringRef replacement )
{
    if( _self )
    {
        INTERNAL_DKStringReplaceOccurrencesOfString( _self, pattern, replacement );
    }
}


///
//  DKStringEscape()
//
static int DKStringEscapeCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    DKMutableStringRef str = context;
    INTERNAL_DKStringReplaceOccurrencesOfString( str, key, object );
    
    return 0;
}

void DKStringEscape( DKMutableStringRef _self, DKDictionaryRef patterns )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );
        DKForeachKeyAndObject( patterns, DKStringEscapeCallback, _self );
    }
}


///
//  DKStringRemovePercentEscapes()
//
static int DKStringUnescapeCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    DKMutableStringRef str = context;
    INTERNAL_DKStringReplaceOccurrencesOfString( str, object, key );
    
    return 0;
}

void DKStringUnescape( DKMutableStringRef _self, DKDictionaryRef patterns )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass() );
        DKForeachKeyAndObject( patterns, DKStringUnescapeCallback, _self );
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
bool DKStringIsAbsolutePath( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        return (_self->byteArray.length > 0) && (_self->byteArray.bytes[0] == DKPathComponentSeparator);
    }
    
    return false;
}

///
//  DKStringGetLastPathComponent()
//
DKStringRef DKStringGetLastPathComponent( DKStringRef _self )
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
        
        return DKAutorelease( CopySubstring( path, DKRangeMake( start, end - start ) ) );
    }
    
    return NULL;
}


///
//  DKStringHasPathExtension()
//
bool DKStringHasPathExtension( DKStringRef _self, DKStringRef extension )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        DKAssertKindOfClass( extension, DKStringClass() );
        
        const char * path = (const char *)_self->byteArray.bytes;
        const char * ext = dk_ustrrchr( path, DKPathExtensionSeparator );
        
        // No extension
        if( ext == NULL )
            return extension->byteArray.length == 0;
        
        const char * lastSlash = dk_ustrrchr( path, DKPathComponentSeparator );
        
        // No extension in last path component
        if( lastSlash && (ext < lastSlash) )
            return extension->byteArray.length == 0;
        
        // Step over the '.'
        DKChar32 ch;
        ext += dk_ustrscan( ext, &ch );
        
        return strcmp( ext, (const char *)extension->byteArray.bytes ) == 0;
    }
    
    return false;
}


///
//  DKStringGetPathExtension()
//
DKStringRef DKStringGetPathExtension( DKStringRef _self )
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
        
        return DKAutorelease( CopySubstring( path, range ) );
    }
    
    return NULL;
}


///
//  DKStringByAppendingPathComponent()
//
DKStringRef DKStringByAppendingPathComponent( DKStringRef _self, DKStringRef pathComponent )
{
    if( _self )
    {
        DKMutableStringRef copy = DKStringMutableCopy( _self );
        DKStringAppendPathComponent( copy, pathComponent );
        DKStringMakeImmutable( copy );
        return DKAutorelease( copy );
    }
    
    return NULL;
}


///
//  DKStringByAppendingPathExtension()
//
DKStringRef DKStringByAppendingPathExtension( DKStringRef _self, DKStringRef extension )
{
    if( _self )
    {
        DKMutableStringRef copy = DKStringMutableCopy( _self );
        DKStringAppendPathExtension( copy, extension );
        DKStringMakeImmutable( copy );
        return DKAutorelease( copy );
    }
    
    return NULL;
}


///
//  DKStringByDeletingLastPathComponent()
//
DKStringRef DKStringByDeletingLastPathComponent( DKStringRef _self )
{
    if( _self )
    {
        DKMutableStringRef copy = DKStringMutableCopy( _self );
        DKStringDeleteLastPathComponent( copy );
        DKStringMakeImmutable( copy );
        return DKAutorelease( copy );
    }
    
    return NULL;
}


///
//  DKStringByDeletingPathExtension()
//
DKStringRef DKStringByDeletingPathExtension( DKStringRef _self )
{
    if( _self )
    {
        DKMutableStringRef copy = DKStringMutableCopy( _self );
        DKStringDeletePathExtension( copy );
        DKStringMakeImmutable( copy );
        return DKAutorelease( copy );
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
//  DKStringDeleteLastPathComponent()
//
void DKStringDeleteLastPathComponent( DKMutableStringRef _self )
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
            DKError( "%s: Cannot append path extension '%s' to path '%s'",
                __func__, (const char *)extension->byteArray.bytes, path );
            return;
        }

        const char separator[2] = { DKPathExtensionSeparator, '\0' };
        AppendBytes( _self, (const uint8_t *)separator, 1 );
        
        AppendBytes( _self, extension->byteArray.bytes, extension->byteArray.length );
    }
}


///
//  DKStringDeletePathExtension()
//
void DKStringDeletePathExtension( DKMutableStringRef _self )
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
//  DKStringURLPercentEscapePatterns()
//
DKThreadSafeSharedObjectInit( DKStringURLPercentEscapePatterns, DKDictionaryRef )
{
    return DKDictionaryInitWithKeysAndObjects( DKAlloc( DKHashTableClass() ),
        DKSTR( " " ), DKSTR( "%20" ),
        DKSTR( "#" ), DKSTR( "%23" ),
        DKSTR( "$" ), DKSTR( "%24" ),
        DKSTR( "%" ), DKSTR( "%25" ),
        DKSTR( "&" ), DKSTR( "%26" ),
        DKSTR( "@" ), DKSTR( "%40" ),
        DKSTR( "`" ), DKSTR( "%60" ),
        DKSTR( "/" ), DKSTR( "%2F" ),
        DKSTR( ":" ), DKSTR( "%3A" ),
        DKSTR( ";" ), DKSTR( "%3B" ),
        DKSTR( "<" ), DKSTR( "%3C" ),
        DKSTR( "=" ), DKSTR( "%3D" ),
        DKSTR( ">" ), DKSTR( "%3E" ),
        DKSTR( "?" ), DKSTR( "%3F" ),
        DKSTR( "[" ), DKSTR( "%5B" ),
        DKSTR( "\\" ),DKSTR( "%5C" ),
        DKSTR( "]" ), DKSTR( "%5D" ),
        DKSTR( "^" ), DKSTR( "%5E" ),
        DKSTR( "{" ), DKSTR( "%7B" ),
        DKSTR( "|" ), DKSTR( "%7C" ),
        DKSTR( "}" ), DKSTR( "%7D" ),
        DKSTR( "~" ), DKSTR( "%7E" ),
        DKSTR( "“" ), DKSTR( "%22" ),
        DKSTR( "‘" ), DKSTR( "%27" ),
        DKSTR( "+" ), DKSTR( "%2B" ),
        DKSTR( "," ), DKSTR( "%2C" ),
        NULL );
}


///
//  DKStringByAddingPercentEscapes()
//
DKStringRef DKStringByAddingURLPercentEscapes( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        DKMutableStringRef copy = DKStringMutableCopy( _self );
        DKStringEscape( copy, DKStringURLPercentEscapePatterns() );
        DKStringMakeImmutable( copy );
        
        return DKAutorelease( copy );
    }
    
    return NULL;
}


///
//  DKStringByRemovingPercentEscapes()
//
DKStringRef DKStringByRemovingURLPercentEscapes( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        DKMutableStringRef copy = DKStringMutableCopy( _self );
        DKStringUnescape( copy, DKStringURLPercentEscapePatterns() );
        DKStringMakeImmutable( copy );
        
        return DKAutorelease( copy );
    }

    return NULL;
}


///
//  DKStringSplitQueryParameters()
//
static int DKStringSplitQueryParametersCallback( DKObjectRef object, void * context )
{
    DKMutableDictionaryRef queryParameters = context;
    
    DKListRef keyValuePair = DKStringSplit( object, DKSTR( "=" ) );
    DKIndex count = DKListGetCount( keyValuePair );
    DKAssert( count > 0 );

    DKStringRef escapedKey = DKListGetObjectAtIndex( keyValuePair, 0 );
    DKStringRef key = DKStringByRemovingURLPercentEscapes( escapedKey );
    DKStringRef value = NULL;
    
    if( count > 1 )
    {
        DKStringRef escapedValue = DKListGetObjectAtIndex( keyValuePair, 1 );
        value = DKStringByRemovingURLPercentEscapes( escapedValue );
    }

    DKDictionarySetObject( queryParameters, key, value );
    
    return 0;
}

DKDictionaryRef DKStringSplitQueryParameters( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        if( _self->byteArray.length > 0 )
        {
            DKMutableDictionaryRef queryParameters = DKMutableDictionary();
        
            DKListRef keyValuePairs = DKStringSplit( _self, DKSTR( "&" ) );
            DKForeachObject( keyValuePairs, DKStringSplitQueryParametersCallback, queryParameters );
            
            return queryParameters;
        }
    }
    
    return NULL;
}


///
//  DKStringCombineQueryParameters()
//
static int DKStringCombineQueryParametersCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    DKMutableStringRef str = context;

    DKStringRef escapedKey = DKStringByAddingURLPercentEscapes( key );
    
    if( object )
    {
        DKStringRef escapedValue = DKStringByAddingURLPercentEscapes( object );
        const char * format = (str->byteArray.length > 0) ? "&%s=%s" : "%s=%s";
        
        DKSPrintf( str, format, (const char *)escapedKey->byteArray.bytes, (const char *)escapedValue->byteArray.bytes );
    }
    
    else
    {
        const char * format = (str->byteArray.length > 0) ? "&%s" : "%s";
        DKSPrintf( str, format, (const char *)escapedKey->byteArray.bytes );
    }
    
    return 0;
}

DKStringRef DKStringCombineQueryParameters( DKDictionaryRef queryParameters )
{
    if( queryParameters )
    {
        DKMutableStringRef str = DKMutableString();
        
        DKForeachKeyAndObject( queryParameters, DKStringCombineQueryParametersCallback, str );
        DKStringMakeImmutable( str );
        
        return str;
    }

    return DKSTR( "" );
}


///
//  DKStringSplitCSSStyles()
//
static int DKStringSplitCSSStylesCallback( DKObjectRef object, void * context )
{
    DKMutableDictionaryRef styles = context;
    
    DKListRef keyValuePair = DKStringSplit( object, DKSTR( ":" ) );
    DKIndex count = DKListGetCount( keyValuePair );

    if( count == 2 )
    {
        DKStringRef key = DKStringByTrimmingWhitespace( DKListGetObjectAtIndex( keyValuePair, 0 ) );
        DKStringRef value = DKStringByTrimmingWhitespace( DKListGetObjectAtIndex( keyValuePair, 1 ) );

        DKDictionarySetObject( styles, key, value );
    }
    
    return 0;
}

DKDictionaryRef DKStringSplitCSSStyles( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        if( _self->byteArray.length > 0 )
        {
            DKMutableDictionaryRef styles = DKMutableDictionary();
        
            DKListRef keyValuePairs = DKStringSplit( _self, DKSTR( ";" ) );
            DKForeachObject( keyValuePairs, DKStringSplitCSSStylesCallback, styles );
            
            return styles;
        }
    }
    
    return NULL;
}


///
//  DKStringSeek()
//
int DKStringSeek( DKStringRef _self, long offset, int origin )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        DKIndex cursor = _self->cursor;
        
        if( origin == SEEK_SET )
            cursor = offset;
        
        else if( origin == SEEK_CUR )
            cursor += offset;
        
        else
            cursor = _self->byteArray.length + cursor;

        DKStringSetCursor( _self, cursor );
        
        return 0;
    }
    
    return -1;
}


///
//  DKStringTell()
//
long DKStringTell( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        return (long)_self->cursor;
    }
    
    return -1;
}


///
//  DKStringRead()
//
size_t DKStringRead( DKStringRef _self, void * buffer, size_t size, size_t count )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        DKStringSetCursor( _self, _self->cursor );
        
        DKRange range = DKRangeMake( _self->cursor, size * count );
        
        if( range.length > (_self->byteArray.length - _self->cursor) )
            range.length = _self->byteArray.length - _self->cursor;
        
        memcpy( buffer, &_self->byteArray.bytes[range.location], range.length );
        
        DKStringSetCursor( _self, _self->cursor + range.length );
        
        return range.length / size;
    }
    
    return 0;
}


///
//  DKStringWrite()
//
size_t DKStringWrite( DKMutableStringRef _self, const void * buffer, size_t size, size_t count )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableStringClass(), 0 );

        DKStringSetCursor( _self, _self->cursor );
        
        DKRange range = DKRangeMake( _self->cursor, size * count );
        
        if( range.length > (_self->byteArray.length - _self->cursor) )
            range.length = _self->byteArray.length - _self->cursor;
        
        ReplaceBytes( _self, range, buffer, size * count );

        DKStringSetCursor( _self, _self->cursor + (size * count) );
        
        return count;
    }
    
    return 0;
}


///
//  DKStringGetBool()
//
bool DKStringGetBool( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );
        
        if( _self->byteArray.length > 0 )
        {
            // Note: this uses the same logic as the getBool method of NSString

            int c = _self->byteArray.bytes[0];

            if( isdigit( c ) && (c != '0') )
                return true;
            
            if( (c == 't') || (c == 'T') || (c == 'y') || (c == 'Y') )
                return true;
        }
    }
    
    return false;
}


///
//  DKStringGetInt32()
//
int32_t DKStringGetInt32( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( _self->byteArray.length > 0 )
            return dk_strtoi32( (const char *)_self->byteArray.bytes, NULL, 10 );
    }
    
    return 0;
}


///
//  DKStringGetInt64()
//
int64_t DKStringGetInt64( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( _self->byteArray.length > 0 )
            return dk_strtoi64( (const char *)_self->byteArray.bytes, NULL, 10 );
    }
    
    return 0;
}


///
//  DKStringGetFloat()
//
float DKStringGetFloat( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( _self->byteArray.length > 0 )
            return dk_strtof32( (const char *)_self->byteArray.bytes, NULL );
    }
    
    return 0.0f;
}


///
//  DKStringGetDouble()
//
double DKStringGetDouble( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( _self->byteArray.length > 0 )
            return dk_strtof64( (const char *)_self->byteArray.bytes, NULL );
    }
    
    return 0.0;
}




// Wide Character String Conversion on Windows ==========================================
#if DK_PLATFORM_WINDOWS

///
//  DKStringInitWithWString()
//
DKStringRef DKStringInitWithWString( DKObjectRef _untyped_self, const WCHAR * wstr )
{
    int length = WideCharToMultiByte( CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL );

    char * cstr = dk_malloc( length );

    WideCharToMultiByte( CP_UTF8, 0, wstr, -1, cstr, length, NULL, NULL );

    DKStringRef _self = DKStringInitWithCString( _untyped_self, cstr );

    dk_free( cstr );

    return _self;
}


///
//  DKStringGetWString()
//
const WCHAR * DKStringGetWString( DKStringRef _self )
{
    static WCHAR emptyString = 0;

    DKDataRef wstrData = DKStringGetWStringAsData( _self );

    if( wstrData )
        return DKDataGetBytePtr( wstrData, 0 );

    return &emptyString;
}


///
//  DKStringGetWStringAsData()
//
DK_API DKDataRef DKStringGetWStringAsData( DKStringRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStringClass() );

        if( _self->byteArray.length == 0 )
            return NULL;

        const char * cstr = (const char *)_self->byteArray.bytes;

        int length = MultiByteToWideChar( CP_UTF8, 0, cstr, -1, NULL, 0 );

        if( length <= 0 )
            return NULL;

        DKMutableDataRef wstrData = DKMutableDataWithCapacity( length * sizeof(WCHAR) );
        WCHAR * wstr = DKDataGetMutableBytePtr( wstrData, 0 );

	    if( MultiByteToWideChar( CP_UTF8, 0, cstr, -1, wstr, length ) != length )
            return NULL;

        DKDataMakeImmutable( wstrData );

        return wstrData;
    }

    return NULL;
}

#endif




// Constant Strings ======================================================================
static DKGenericHashTable * DKConstantStringTable = NULL;
static DKSpinLock DKConstantStringTableLock = DKSpinLockInit;


static DKRowStatus DKConstantStringTableRowStatus( const void * _row, void * not_used )
{
    DKStringRef * row = (void *)_row;
    return (DKRowStatus)(*row);
}

static DKHashCode DKConstantStringTableRowHash( const void * _row, void * not_used )
{
    DKStringRef * row = (void *)_row;
    return (*row)->hashCode;
}

static bool DKConstantStringTableRowEqual( const void * _row1, const void * _row2, void * not_used )
{
    DKStringRef * row1 = (void *)_row1;
    DKStringRef * row2 = (void *)_row2;

    return DKStringEqualToString( *row1, *row2 );
}

static void DKConstantStringTableRowInit( void * _row, void * not_used )
{
    DKStringRef * row = _row;
    *row = DKRowStatusEmpty;
}

static void DKConstantStringTableRowUpdate( void * _row, const void * _src, void * not_used )
{
    DKStringRef * row = _row;
    DKStringRef * src = (void *)_src;
    *row = *src;
}

static void DKConstantStringTableRowDelete( void * _row, void * not_used )
{
    DKStringRef * row = _row;
    *row = DKRowStatusDeleted;
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

        DKGenericHashTableInit( table, sizeof(DKStringRef), &callbacks, NULL );
        
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
    DKStringRef * entry = (DKStringRef *)DKGenericHashTableFind( DKConstantStringTable, &lookupString );
    
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
        entry = (DKStringRef *)DKGenericHashTableFind( DKConstantStringTable, &lookupString );
        constantString = *entry;
    }

    DKSpinLockUnlock( &DKConstantStringTableLock );

    // Discard the new string if we're not using it
    if( constantString != newConstantString )
        DKDeallocObject( newConstantString );
    
    return constantString;
}


