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



static void DKPropertyFinalize( DKObjectRef _self );


DKThreadSafeClassInit( DKPropertyClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKProperty" ), DKObjectClass(), sizeof(struct DKProperty), 0, NULL, DKPropertyFinalize );

    return cls;
}


///
//  DKPropertyFinalize()
//
static void DKPropertyFinalize( DKObjectRef _self )
{
    struct DKProperty * property = (struct DKProperty *)_self;
    
    DKRelease( property->name );
    DKRelease( property->semantic );
    DKRelease( property->predicate );
}




// Defining Properties ===================================================================

///
//  DKInstallObjectProperty()
//
void DKInstallObjectProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    DKPredicateRef predicate,
    DKPropertySetter setter,
    DKPropertyGetter getter )
{
    struct DKProperty * property = DKCreate( DKPropertyClass() );
    
    property->name = DKCopy( name );
    
    property->attributes = attributes;
    property->offset = offset;
    property->encoding = DKEncode( DKEncodingTypeObject, 1 );
    
    property->predicate = DKCopy( predicate );
    
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
    DKEncoding encoding,
    DKStringRef semantic,
    DKPropertySetter setter,
    DKPropertyGetter getter )
{
    DKAssert( DKEncodingIsNumber( encoding ) );
    
    struct DKProperty * property = DKCreate( DKPropertyClass() );
    
    property->name = DKCopy( name );
    
    property->attributes = attributes;
    property->offset = offset;
    property->encoding = encoding;

    property->semantic = DKCopy( semantic );
    
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
    DKStringRef semantic,
    DKPropertySetter setter,
    DKPropertyGetter getter )
{
    struct DKProperty * property = DKCreate( DKPropertyClass() );
    
    property->name = DKCopy( name );
    
    property->attributes = attributes;
    property->offset = offset;
    property->encoding = DKEncode( DKEncodingTypeBinaryData, (uint32_t)size );

    property->semantic = DKCopy( semantic );

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
//  CheckPredicateRequirement()
//
static void FailedPredicateRequirement( DKObjectRef _self, DKPropertyRef property, DKObjectRef object )
{
    DKStringRef desc = DKCopyDescription( property->predicate );

    DKWarning( "DKProperty: '%s' does not meet the requirements ('%s') for property '%s'.\n",
        DKStringGetCStringPtr( DKGetClassName( object ) ),
        DKStringGetCStringPtr( desc ),
        DKStringGetCStringPtr( property->name ) );
    
    DKRelease( desc );
}

#define CheckPredicateRequirement( obj, property, object, ... )                         \
    do                                                                                  \
    {                                                                                   \
        if( (property->predicate != NULL) &&                                            \
            !DKPredicateEvaluateWithObject( property->predicate, object ) )             \
        {                                                                               \
            FailedPredicateRequirement( obj, property, object );                        \
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
        DKStringGetCStringPtr( property->semantic ),
        DKStringGetCStringPtr( property->name ) );
}

#define CheckSemanticRequirement( obj, property, semantic, ... )                        \
    do                                                                                  \
    {                                                                                   \
        if( (property->semantic != NULL) &&                                             \
            !DKStringEqualToString( semantic, property->semantic ) )                    \
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
    if( property->encoding == DKEncode( DKEncodingTypeObject, 1 ) )
    {
        CheckPredicateRequirement( _self, property, object );

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
        if( DKNumberCastValue( object, value, property->encoding ) )
            return;
    }
    
    // Automatic conversion of structure types
    if( DKIsKindOfClass( object, DKStructClass() ) )
    {
        if( DKStructGetValue( object, property->semantic, value, DKEncodingGetSize( property->encoding ) ) )
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
    if( property->encoding == DKEncode( DKEncodingTypeObject, 1 ) )
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
    if( DKEncodingIsNumber( property->encoding ) )
    {
        DKNumberRef number = DKNumberCreate( value, property->encoding );
        return number;
    }

    // Automatic conversion of structure types
    if( property->semantic != NULL )
    {
        DKStructRef structure = DKStructCreate( property->semantic, value, DKEncodingGetSize( property->encoding ) );
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
void DKSetNumericalProperty( DKObjectRef _self, DKStringRef name, const void * srcValue, DKEncoding srcEncoding )
{
    DKAssert( DKEncodingIsNumber( srcEncoding ) );

    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        CheckPropertyIsDefined( _self, property, name );
        CheckPropertyIsReadWrite( _self, property );
        
        if( property->setter || (property->encoding == DKEncode( DKEncodingTypeObject, 1 )) )
        {
            DKNumberRef number = DKNumberCreate( srcValue, srcEncoding );
            DKWritePropertyObject( _self, property, number );
            DKRelease( number );
            return;
        }

        else
        {
            void * dstValue = (uint8_t *)_self + property->offset;
            
            if( DKNumberConvert( srcValue, srcEncoding, dstValue, property->encoding ) )
                return;
        }
        
        DKWarning( "DKProperty: No available conversion for property '%s' from %s[%d].\n",
            DKStringGetCStringPtr( name ), DKEncodingGetTypeName( srcEncoding ), DKEncodingGetCount( srcEncoding ) );
    }
}


///
//  DKGetNumericalProperty()
//
size_t DKGetNumericalProperty( DKObjectRef _self, DKStringRef name, void * dstValue, DKEncoding dstEncoding )
{
    size_t result = 0;

    DKAssert( DKEncodingIsNumber( dstEncoding ) );

    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        CheckPropertyIsDefined( _self, property, name, 0 );
        
        if( property->getter || (property->encoding == DKEncode( DKEncodingTypeObject, 1 )) )
        {
            DKObjectRef object = DKReadPropertyObject( _self, property );

            if( object == NULL )
            {
                memset( dstValue, 0, DKEncodingGetSize( dstEncoding ) );
                
                return DKEncodingGetCount( dstEncoding );
            }
            
            if( DKIsKindOfClass( object, DKNumberClass() ) )
            {
                if( (result = DKNumberCastValue( object, dstValue, dstEncoding )) != 0 )
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

            if( (result = DKNumberConvert( srcValue, property->encoding, dstValue, dstEncoding )) != 0 )
                return result;
        }

        DKWarning( "DKProperty: No available conversion for property '%s' to %s[%d].\n",
            DKStringGetCStringPtr( name ), DKEncodingGetTypeName( dstEncoding ), DKEncodingGetCount( dstEncoding ) );
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
        
        if( property->setter || (property->encoding == DKEncode( DKEncodingTypeObject, 1 )) )
        {
            DKStructRef structure = DKStructCreate( semantic, srcValue, srcSize );
            DKWritePropertyObject( _self, property, structure );
            DKRelease( structure );
            return;
        }

        if( property->encoding == DKEncode( DKEncodingTypeBinaryData, srcSize ) )
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
        
        if( property->getter || (property->encoding == DKEncode( DKEncodingTypeObject, 1 )) )
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

        if( property->encoding == DKEncode( DKEncodingTypeBinaryData, dstSize ) )
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


