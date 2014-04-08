//
//  DKProperty.c
//  Duck
//
//  Created by Derek Nylen on 2014-04-07.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKProperty.h"
#include "DKString.h"
#include "DKNumber.h"



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
    DKRelease( property->structHint );
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
    size_t size,
    DKStringRef hint,
    DKPropertySetter setter,
    DKPropertyGetter getter )
{
    struct DKProperty * property = DKCreate( DKPropertyClass() );
    
    property->name = DKRetain( name );
    property->attributes = attributes;
    property->offset = offset;
    property->type = DKPropertyStruct;
    property->size = size;
    property->structHint = DKRetain( hint );

    property->setter = setter;
    property->getter = getter;
    
    DKInstallProperty( _class, name, property );
    DKRelease( property );
}




// Get/Set Properties ====================================================================

///
//  PropertyNotDefined()
//
static void PropertyNotDefined( DKObjectRef obj, DKStringRef name )
{
    DKWarning( "DKSetProperty: Property '%s' is not defined for class '%s'.\n",
        DKStringGetCStringPtr( name ), DKStringGetCStringPtr( DKGetClassName( obj ) ) );
}


///
//  DKWritePropertyObject()
//
static void DKWritePropertyObject( DKObjectRef _self, DKPropertyRef property, DKObjectRef object )
{
    void * value = (uint8_t *)_self + property->offset;
    
    // Object types
    if( property->type == DKPropertyObject )
    {
        if( (property->requiredClass != NULL) && !DKIsKindOfClass( object, property->requiredClass ) )
        {
            DKWarning( "DKSetProperty: '%s' does not meet the class requirement ('%s') for property '%s'.\n",
                DKStringGetCStringPtr( DKGetClassName( object ) ),
                DKStringGetCStringPtr( DKGetClassName( property->requiredClass ) ),
                DKStringGetCStringPtr( property->name ) );
            return;
        }
        
        if( (property->requiredInterface != NULL) && !DKQueryInterface( object, property->requiredInterface, NULL ) )
        {
            DKWarning( "DKSetProperty: '%s' does not meet the interface requirement ('%s') for property '%s'.\n",
                DKStringGetCStringPtr( DKGetClassName( object ) ),
                property->requiredInterface->suid,
                DKStringGetCStringPtr( property->name ) );
            return;
        }

        DKObjectRef * ref = value;
        
        DKRetain( object );
        DKRelease( *ref );
        *ref = object;
        
        return;
    }
    
    // Automatic conversion of numerical types
    if( DKIsKindOfClass( object, DKNumberClass() ) )
    {
        if( DKNumberCastValue( object, value, property->type ) )
            return;
    }
    
    // Automatic conversion of structure types
    // DO STUFF HERE

    DKWarning( "DKSetProperty: No available conversion for property '%s' from '%s'.\n",
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
        DKObjectRef * ref = value;
        return DKRetain( *ref );
    }
    
    // Automatic conversion of numerical types
    if( DKNumberTypeIsValid( property->type ) )
    {
        DKNumberRef number = DKNumberCreate( value, property->type );
        return number;
    }

    // Automatic conversion of structure types
    // DO STUFF HERE

    DKWarning( "DKSetProperty: No available conversion for property '%s' to object.\n",
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
        
        if( !property )
        {
            PropertyNotDefined( _self, name );
            return;
        }
        
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
        
        if( !property )
        {
            PropertyNotDefined( _self, name );
            return NULL;
        }
        
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
        
        if( !property )
        {
            PropertyNotDefined( _self, name );
            return;
        }
        
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
        
        DKWarning( "DKSetProperty: No available conversion for property '%s' from %s[%d].\n",
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
        
        if( !property )
        {
            PropertyNotDefined( _self, name );
            return 0;
        }
        
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

        DKWarning( "DKSetProperty: No available conversion for property '%s' to %s[%d].\n",
            DKStringGetCStringPtr( name ), DKNumberGetComponentName( dstType ), DKNumberGetComponentCount( dstType ) );
    }
    
    return result;
}


///
//  DKSetStructProperty()
//
void DKSetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef hint, const void * srcValue, size_t srcSize )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );
        
        if( !property )
        {
            PropertyNotDefined( _self, name );
            return;
        }
        
        if( property->setter || (property->type == DKPropertyObject) )
        {
            DKAssert( 0 );
            //DKStructRef structure = DKStructCreate( hint, srcValue, srcSize );
            //DKWritePropertyObject( _self, property, structure );
            //DKRelease( structure );
            return;
        }

        if( (property->type == DKPropertyStruct) && (property->size == srcSize) )
        {
            void * dstValue = (uint8_t *)_self + property->offset;
            memcpy( dstValue, srcValue, srcSize );
            return;
        }
        
        DKWarning( "DKSetProperty: No available conversion for property '%s' from structure '%s'.\n",
            DKStringGetCStringPtr( name ), DKStringGetCStringPtr( hint ) );
    }
}


///
//  DKGetStructProperty()
//
size_t DKGetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef hint, void * dstValue, size_t dstSize )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );
        
        if( !property )
        {
            PropertyNotDefined( _self, name );
            return 0;
        }
        
        if( property->getter || (property->type == DKPropertyObject) )
        {
            DKAssert( 0 );
            /*
            DKObjectRef object = DKReadPropertyObject( _self, property );

            if( object == NULL )
            {
                memset( dstValue, 0, dstSize );
                return 1;
            }
            
            if( DKIsKindOfClass( object, DKStructClass() ) )
            {
                if( DKStructGetValue( DKStringRef hint, dstValue, dstSize ) )
                {
                    DKRelease( object );
                    return 1;
                }
            }

            DKRelease( object );
            */
        }

        if( (property->type == DKPropertyStruct) && (property->size == dstSize) )
        {
            const void * srcValue = (uint8_t *)_self + property->offset;
            memcpy( dstValue, srcValue, dstSize );
            return 1;
        }
        
        DKWarning( "DKSetProperty: No available conversion for property '%s' to structure '%s'.\n",
            DKStringGetCStringPtr( name ), DKStringGetCStringPtr( hint ) );
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


