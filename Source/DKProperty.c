/*****************************************************************************************

  DKProperty.c

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

#include "DKProperty.h"
#include "DKString.h"
#include "DKNumber.h"
#include "DKStruct.h"
#include "DKCopying.h"



static void DKPropertyFinalize( DKObjectRef _self );


DKThreadSafeClassInit( DKPropertyClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKProperty" ), DKObjectClass(), sizeof(struct DKProperty), 0 );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->finalize = DKPropertyFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    return cls;
}


///
//  DKPropertyFinalize()
//
static void DKPropertyFinalize( DKObjectRef _self )
{
    struct DKProperty * property = (struct DKProperty *)_self;
    
    DKRelease( property->name );
    DKRelease( property->requiredClass );
    DKRelease( property->requiredInterface );
    DKRelease( property->requiredSemantic );
}




// Defining Properties ===================================================================

///
//  DKInstallObjectProperty()
//
void DKInstallObjectProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    DKClassRef requiredClass,
    DKSEL requiredInterface,
    DKPropertySetter setter,
    DKPropertyGetter getter )
{
    struct DKProperty * property = DKCreate( DKPropertyClass() );
    
    property->name = DKRetain( name );
    property->attributes = attributes;
    property->offset = offset;
    property->type = DKPropertyObject;
    property->size = sizeof(DKObjectRef);
    
    property->requiredClass = DKRetain( requiredClass );
    property->requiredInterface = DKRetain( requiredInterface );
    
    property->setter = setter;
    property->getter = getter;
    
    DKInstallProperty( _class, name, property );
    DKRelease( property );
}


///
//  DKInstallNumericalProperty()
//
void DKInstallNumericalProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    DKNumberType type,
    DKPropertySetter setter,
    DKPropertyGetter getter )
{
    DKAssert( DKNumberTypeIsValid( type ) );
    
    size_t size = DKNumberGetComponentSize( type );
    size_t count = DKNumberGetComponentCount( type );
    
    struct DKProperty * property = DKCreate( DKPropertyClass() );
    
    property->name = DKRetain( name );
    property->attributes = attributes;
    property->offset = offset;
    property->type = type;
    property->size = size * count;
    
    property->setter = setter;
    property->getter = getter;
    
    DKInstallProperty( _class, name, property );
    DKRelease( property );
}


///
//  DKInstallStructProperty()
//
void DKInstallStructProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    DKStringRef semantic,
    size_t size,
    DKPropertySetter setter,
    DKPropertyGetter getter )
{
    struct DKProperty * property = DKCreate( DKPropertyClass() );
    
    property->name = DKRetain( name );
    property->attributes = attributes;
    property->offset = offset;
    property->type = DKPropertyStruct;
    property->size = size;
    
    property->requiredSemantic = DKRetain( semantic );

    property->setter = setter;
    property->getter = getter;
    
    DKInstallProperty( _class, name, property );
    DKRelease( property );
}




// Error Reporting =======================================================================

///
//  CheckPropertyDefined()
//
static void PropertyNotDefined( DKObjectRef _self, DKPropertyRef property, DKStringRef name )
{
    DKWarning( "DKProperty: Property '%s' is not defined for class '%s'.\n",
        DKStringGetCStringPtr( name ), DKStringGetCStringPtr( DKGetClassName( _self ) ) );
}

#define CheckPropertyIsDefined( obj, property, name, ... )                              \
    do                                                                                  \
    {                                                                                   \
        if( (property) == NULL )                                                        \
        {                                                                               \
            PropertyNotDefined( obj, property, name );                                  \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )


///
//  CheckPropertyReadWrite()
//
static void PropertyNotReadWrite( DKObjectRef _self, DKPropertyRef property )
{
    DKWarning( "DKProperty: Property '%s' is not read-write.\n",
        DKStringGetCStringPtr( property->name ) );
}

#define CheckPropertyIsReadWrite( obj, property, ... )                                  \
    do                                                                                  \
    {                                                                                   \
        if( (property)->attributes & DKPropertyReadOnly )                               \
        {                                                                               \
            PropertyNotReadWrite( obj, property );                                      \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )



///
//  CheckClassRequirement()
//
static void FailedClassRequirement( DKObjectRef _self, DKPropertyRef property, DKObjectRef object )
{
    DKWarning( "DKProperty: '%s' does not meet the class requirement ('%s') for property '%s'.\n",
        DKStringGetCStringPtr( DKGetClassName( object ) ),
        DKStringGetCStringPtr( DKGetClassName( property->requiredClass ) ),
        DKStringGetCStringPtr( property->name ) );
}

#define CheckClassRequirement( obj, property, object, ... )                             \
    do                                                                                  \
    {                                                                                   \
        if( (property->requiredClass != NULL) &&                                        \
            !DKIsKindOfClass( object, property->requiredClass ) )                       \
        {                                                                               \
            FailedClassRequirement( obj, property, object );                            \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )


///
//  CheckInterfaceRequirement()
//
static void FailedInterfaceRequirement( DKObjectRef _self, DKPropertyRef property, DKObjectRef object )
{
    DKWarning( "DKProperty: '%s' does not meet the interface requirement ('%s') for property '%s'.\n",
        DKStringGetCStringPtr( DKGetClassName( object ) ),
        property->requiredInterface->suid,
        DKStringGetCStringPtr( property->name ) );
}

#define CheckInterfaceRequirement( obj, property, object, ... )                         \
    do                                                                                  \
    {                                                                                   \
        if( (property->requiredInterface != NULL) &&                                    \
            !DKQueryInterface( object, property->requiredInterface, NULL ) )            \
        {                                                                               \
            FailedInterfaceRequirement( obj, property, object );                        \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )


///
//  CheckSemanticRequirement()
//
static void FailedSemanticRequirement( DKObjectRef _self, DKPropertyRef property, DKStringRef semantic )
{
    DKWarning( "DKProperty: '%s' does not meet the semantic requirement ('%s') for property '%s'.\n",
        DKStringGetCStringPtr( semantic ),
        DKStringGetCStringPtr( property->requiredSemantic ),
        DKStringGetCStringPtr( property->name ) );
}

#define CheckSemanticRequirement( obj, property, semantic, ... )                        \
    do                                                                                  \
    {                                                                                   \
        if( (property->requiredSemantic != NULL) &&                                     \
            !DKStringEqual( semantic, property->requiredSemantic ) )                    \
        {                                                                               \
            FailedSemanticRequirement( obj, property, semantic );                       \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )





// Get/Set Properties ====================================================================

///
//  DKWritePropertyObject()
//
static void DKWritePropertyObject( DKObjectRef _self, DKPropertyRef property, DKObjectRef object )
{
    void * value = (uint8_t *)_self + property->offset;
    
    // Object types
    if( property->type == DKPropertyObject )
    {
        CheckClassRequirement( _self, property, object );
        CheckInterfaceRequirement( _self, property, object );

        DKObjectRef * ref = value;
        
        if( property->attributes & DKPropertyCopy )
        {
            DKObjectRef copy = DKCopy( object );
            DKRelease( *ref );
            *ref = copy;
        }
        
        else if( property->attributes & DKPropertyWeak )
        {
            DKWeakRef weakref = DKRetainWeak( object );
            DKRelease( *ref );
            *ref = weakref;
        }
        
        else
        {
            DKRetain( object );
            DKRelease( *ref );
            *ref = object;
        }
        
        return;
    }
    
    // Automatic conversion of numerical types
    if( DKIsKindOfClass( object, DKNumberClass() ) )
    {
        if( DKNumberCastValue( object, value, property->type ) )
            return;
    }
    
    // Automatic conversion of structure types
    if( DKIsKindOfClass( object, DKStructClass() ) )
    {
        if( DKStructGetValue( object, property->requiredSemantic, value, property->size ) )
            return;
    }

    DKWarning( "DKProperty: No available conversion for property '%s' from %s.\n",
        DKStringGetCStringPtr( property->name ), DKStringGetCStringPtr( DKGetClassName( object ) ) );
}


///
//  DKReadPropertyObject()
//
static DKObjectRef DKReadPropertyObject( DKObjectRef _self, DKPropertyRef property )
{
    void * value = (uint8_t *)_self + property->offset;
    
    // Object types
    if( property->type == DKPropertyObject )
    {
        if( property->attributes & DKPropertyWeak )
        {
            DKWeakRef * ref = value;
            return DKResolveWeak( *ref );
        }
        
        else
        {
            DKObjectRef * ref = value;
            return DKRetain( *ref );
        }
    }
    
    // Automatic conversion of numerical types
    if( DKNumberTypeIsValid( property->type ) )
    {
        DKNumberRef number = DKNumberCreate( value, property->type );
        return number;
    }

    // Automatic conversion of structure types
    if( property->requiredSemantic != NULL )
    {
        DKStructRef structure = DKStructCreate( property->requiredSemantic, value, property->size );
        return structure;
    }

    DKWarning( "DKProperty: No available conversion for property '%s' to object.\n",
        DKStringGetCStringPtr( property->name ) );
    
    return NULL;
}


///
//  DKSetProperty()
//
void DKSetProperty( DKObjectRef _self, DKStringRef name, DKObjectRef object )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );
        
        CheckPropertyIsDefined( _self, property, name );
        CheckPropertyIsReadWrite( _self, property );
        
        if( property->setter )
        {
            property->setter( _self, property, object );
            return;
        }
        
        DKWritePropertyObject( _self, property, object );
    }
}


///
//  DKGetProperty()
//
DKObjectRef DKGetProperty( DKObjectRef _self, DKStringRef name )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        CheckPropertyIsDefined( _self, property, name, NULL );
        
        if( property->getter )
        {
            return property->getter( _self, property );
        }
        
        return DKReadPropertyObject( _self, property );
    }
    
    return NULL;
}


///
//  DKSetNumericalProperty()
//
void DKSetNumericalProperty( DKObjectRef _self, DKStringRef name, const void * srcValue, DKNumberType srcType )
{
    DKAssert( DKNumberTypeIsValid( srcType ) );

    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        CheckPropertyIsDefined( _self, property, name );
        CheckPropertyIsReadWrite( _self, property );
        
        if( property->setter || (property->type == DKPropertyObject) )
        {
            DKNumberRef number = DKNumberCreate( srcValue, srcType );
            DKWritePropertyObject( _self, property, number );
            DKRelease( number );
            return;
        }

        else
        {
            void * dstValue = (uint8_t *)_self + property->offset;
            
            if( DKNumberConvert( srcValue, srcType, dstValue, property->type ) )
                return;
        }
        
        DKWarning( "DKProperty: No available conversion for property '%s' from %s[%d].\n",
            DKStringGetCStringPtr( name ), DKNumberGetComponentName( srcType ), DKNumberGetComponentCount( srcType ) );
    }
}


///
//  DKGetNumericalProperty()
//
size_t DKGetNumericalProperty( DKObjectRef _self, DKStringRef name, void * dstValue, DKNumberType dstType )
{
    size_t result = 0;

    DKAssert( DKNumberTypeIsValid( dstType ) );

    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        CheckPropertyIsDefined( _self, property, name, 0 );
        
        if( property->getter || (property->type == DKPropertyObject) )
        {
            DKObjectRef object = DKReadPropertyObject( _self, property );

            if( object == NULL )
            {
                size_t size = DKNumberGetComponentSize( dstType );
                size_t count = DKNumberGetComponentCount( dstType );
                
                memset( dstValue, 0, size * count );
                
                return count;
            }
            
            if( DKIsKindOfClass( object, DKNumberClass() ) )
            {
                if( (result = DKNumberCastValue( object, dstValue, dstType )) != 0 )
                {
                    DKRelease( object );
                    return result;
                }
            }

            DKRelease( object );
        }
        
        else
        {
            void * srcValue = (uint8_t *)_self + property->offset;

            if( (result = DKNumberConvert( srcValue, property->type, dstValue, dstType )) != 0 )
                return result;
        }

        DKWarning( "DKProperty: No available conversion for property '%s' to %s[%d].\n",
            DKStringGetCStringPtr( name ), DKNumberGetComponentName( dstType ), DKNumberGetComponentCount( dstType ) );
    }
    
    return result;
}


///
//  DKSetStructProperty()
//
void DKSetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, const void * srcValue, size_t srcSize )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );
        
        CheckPropertyIsDefined( _self, property, name );
        CheckPropertyIsReadWrite( _self, property );
        CheckSemanticRequirement( _self, property, semantic );
        
        if( property->setter || (property->type == DKPropertyObject) )
        {
            DKStructRef structure = DKStructCreate( semantic, srcValue, srcSize );
            DKWritePropertyObject( _self, property, structure );
            DKRelease( structure );
            return;
        }

        if( (property->type == DKPropertyStruct) && (property->size == srcSize) )
        {
            void * dstValue = (uint8_t *)_self + property->offset;
            memcpy( dstValue, srcValue, srcSize );
            return;
        }
        
        DKWarning( "DKProperty: No available conversion for property '%s' from structure (%s).\n",
            DKStringGetCStringPtr( name ), DKStringGetCStringPtr( semantic ) );
    }
}


///
//  DKGetStructProperty()
//
size_t DKGetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, void * dstValue, size_t dstSize )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );
        
        CheckPropertyIsDefined( _self, property, name, 0 );
        CheckSemanticRequirement( _self, property, semantic, 0 );
        
        if( property->getter || (property->type == DKPropertyObject) )
        {
            DKObjectRef object = DKReadPropertyObject( _self, property );

            if( object == NULL )
            {
                memset( dstValue, 0, dstSize );
                return 1;
            }
            
            if( DKIsKindOfClass( object, DKStructClass() ) )
            {
                if( DKStructGetValue( object, semantic, dstValue, dstSize ) )
                {
                    DKRelease( object );
                    return 1;
                }
            }

            DKRelease( object );
        }

        if( (property->type == DKPropertyStruct) && (property->size == dstSize) )
        {
            const void * srcValue = (uint8_t *)_self + property->offset;
            memcpy( dstValue, srcValue, dstSize );
            return 1;
        }
        
        DKWarning( "DKProperty: No available conversion for property '%s' to structure (%s).\n",
            DKStringGetCStringPtr( name ), DKStringGetCStringPtr( semantic ) );
    }
    
    return 0;
}


///
//  DKSetIntegerProperty()
//
void DKSetIntegerProperty( DKObjectRef _self, DKStringRef name, int64_t x )
{
    DKSetNumericalProperty( _self, name, &x, DKNumberInt64 );
}


///
//  DKGetIntegerProperty()
//
int64_t DKGetIntegerProperty( DKObjectRef _self, DKStringRef name )
{
    int64_t x;
    
    if( DKGetNumericalProperty( _self, name, &x, DKNumberInt64 ) )
        return x;
    
    return 0;
}


///
//  DKSetFloatProperty()
//
void DKSetFloatProperty( DKObjectRef _self, DKStringRef name, double x )
{
    DKSetNumericalProperty( _self, name, &x, DKNumberDouble );
}


///
//  DKGetFloatProperty()
//
double DKGetFloatProperty( DKObjectRef _self, DKStringRef name )
{
    double x;
    
    if( DKGetNumericalProperty( _self, name, &x, DKNumberDouble ) )
        return x;
    
    return 0;
}


