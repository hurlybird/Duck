/*****************************************************************************************

  DKStruct.c

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

#include "DKStruct.h"
#include "DKString.h"
#include "DKStream.h"
#include "DKAllocation.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKEgg.h"


struct DKStruct
{
    DKObject _obj;
    DKStringRef semantic;
    uint8_t value[1];       // variable size
};


static struct DKStruct DKPlaceholderStruct =
{
    DKInitObjectHeader( NULL ),
    NULL
};


static DKObjectRef  DKStructAllocPlaceholder( DKClassRef _class, size_t extraBytes );
static void         DKStructDealloc( DKStructRef _self );

static void         DKStructFinalize( DKStructRef _self );
static DKStringRef  DKStructGetDescription( DKStructRef _self );

static DKObjectRef  DKStructInitWithEgg( DKStructRef _self, DKEggUnarchiverRef egg );
static void         DKStructAddToEgg( DKStructRef _self, DKEggArchiverRef egg );


DKThreadSafeClassInit( DKStructClass )
{
    // NOTE: The value field of DKStruct is dynamically sized, and not included in the
    // base instance structure size.
    DKClassRef cls = DKAllocClass( DKSTR( "DKStruct" ), DKObjectClass(), sizeof(struct DKStruct) - 1, DKImmutableInstances, NULL, (DKFinalizeMethod)DKStructFinalize );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->alloc = (DKAllocMethod)DKStructAllocPlaceholder;
    allocation->dealloc = (DKDeallocMethod)DKStructDealloc;

    DKInstallClassInterface( cls, allocation );
    DKRelease( allocation );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKAllocInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKStructEqual;
    comparison->compare = (DKCompareMethod)DKStructCompare;
    comparison->hash = (DKHashMethod)DKStructHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );
    
    // Description
    struct DKDescriptionInterface * description = DKAllocInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKStructGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Egg
    struct DKEggInterface * egg = DKAllocInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKStructInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKStructAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}



///
//  DKStructAllocPlaceholder()
//
static DKObjectRef DKStructAllocPlaceholder( DKClassRef _class, size_t extraBytes )
{
    if( _class == DKStructClass_SharedObject )
    {
        DKPlaceholderStruct._obj.isa = DKStructClass_SharedObject;
        return &DKPlaceholderStruct;
    }
    
    DKAssert( 0 );
    return NULL;
}


///
//  DKStructDealloc()
//
static void DKStructDealloc( DKStructRef _self )
{
    if( _self == &DKPlaceholderStruct )
        return;
    
    DKDeallocObject( _self );
}


///
//  DKStructFinalize()
//
static void DKStructFinalize( DKStructRef _self )
{
    DKRelease( _self->semantic );
}


///
//  DKStructInit()
//
DKStructRef DKStructInit( DKStructRef _self, DKStringRef semantic, const void * bytes, size_t size )
{
    // The real size limit is MAX_INT, but > 64K in a structure is almost certainly an error
    DKAssert( size < (64 * 1024) );

    if( _self == &DKPlaceholderStruct  )
    {
        if( (bytes == NULL) || (size == 0) )
            return NULL;
        
        _self = DKAllocObject( DKStructClass(), size );
        
        _self->semantic = DKCopy( semantic );
        
        DKSetObjectTag( _self, (int32_t)size );
        
        memcpy( _self->value, bytes, size );
    }
    
    else if( _self != NULL )
    {
        DKFatalError( "DKStructInit: Trying to initialize a non-struct object.\n" );
    }

    return _self;
}


///
//  DKStructInitWithEgg()
//
static DKObjectRef DKStructInitWithEgg( DKStructRef _self, DKEggUnarchiverRef egg )
{
    DKStringRef semantic = DKEggGetObject( egg, DKSTR( "semantic" ) );

    size_t length = 0;
    const void * bytes = DKEggGetBinaryDataPtr( egg, DKSTR( "value" ), &length );

    return DKStructInit( _self, semantic, bytes, length );
}


///
//  DKStructAddToEgg()
//
static void DKStructAddToEgg( DKStructRef _self, DKEggArchiverRef egg )
{
    DKEggAddObject( egg, DKSTR( "semantic" ), _self->semantic );

    size_t size = (size_t)DKGetObjectTag( _self );
    DKEggAddBinaryData( egg, DKSTR( "value" ), _self->value, size );
}


///
//  DKStructEqual()
//
bool DKStructEqual( DKStructRef _self, DKStructRef other )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStructClass() );
        
        if( DKIsKindOfClass( other, DKStructClass() ) )
        {
            if( DKStringEqualToString( _self->semantic, other->semantic ) )
            {
                size_t size1 = (size_t)DKGetObjectTag( _self );
                size_t size2 = (size_t)DKGetObjectTag( other );
                
                if( size1 == size2 )
                    return memcmp( _self->value, other->value, size1 ) == 0;
            }
        }
    }
    
    return false;
}


///
//  DKStructCompare()
//
int DKStructCompare( DKStructRef _self, DKStructRef other )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStructClass() );
        
        // DKCompare requires that the objects have some strict ordering property useful
        // for comparison, yet has no way of checking if the objects actually meet that
        // requirement.
        if( DKIsKindOfClass( other, DKStructClass() ) )
        {
            int cmp = DKStringCompare( _self->semantic, other->semantic );

            if( cmp != 0 )
                return cmp;

            size_t size1 = (size_t)DKGetObjectTag( _self );
            size_t size2 = (size_t)DKGetObjectTag( other );
            
            if( size1 < size2 )
                return 1;
            
            if( size1 > size2 )
                return -1;
            
            if( size1 == 0 )
                return 0;

            return memcmp( _self->value, other->value, size1 );
        }
    }
    
    return DKPointerCompare( _self, other );
}


///
//  DKStructHash()
//
DKHashCode DKStructHash( DKStructRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStructClass() );

        size_t size = (size_t)DKGetObjectTag( _self );
        
        return dk_memhash( _self->value, size );
    }
    
    return 0;
}


///
//  DKStructGetSemantic()
//
DKStringRef DKStructGetSemantic( DKStructRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStructClass() );
        return _self->semantic;
    }
    
    return NULL;
}


///
//  DKStructGetSize()
//
size_t DKStructGetSize( DKStructRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStructClass() );
        return (size_t)DKGetObjectTag( _self );
    }
    
    return 0;
}


///
//  DKStructGetValue()
//
size_t DKStructGetValue( DKStructRef _self, DKStringRef semantic, void * bytes, size_t size )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKStructClass() );
    
        size_t structSize = (size_t)DKGetObjectTag( _self );
    
        if( size == structSize )
        {
            if( DKEqual( _self->semantic, semantic ) )
            {
                memcpy( bytes, _self->value, size );
            
                return size;
            }
            
            else
            {
                DKWarning( "DKStructGetValue: Semantic mismatch '%s' != '%s'.\n",
                    DKStringGetCStringPtr( _self->semantic ),
                    DKStringGetCStringPtr( semantic ) );
            }
        }
        
        else
        {
            DKError( "DKStructGetValue: Size mismatch %u != %u.\n",
                (unsigned int)DKGetObjectTag( _self ), (unsigned int)size );
        }
    }
    
    return 0;
}


///
//  DKStructGetDescription()
//
static DKStringRef DKStructGetDescription( DKStructRef _self )
{
    if( _self )
    {
        DKMutableStringRef desc = DKAutorelease( DKStringCreateMutable() );
        
        DKSPrintf( desc, "%@ (%@)", DKGetClassName( _self ), _self->semantic );
        
        return desc;
    }
    
    return NULL;
}







