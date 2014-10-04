/*****************************************************************************************

  DKRuntime+Properties.c

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

#define DK_RUNTIME_PRIVATE 1

#include "DKRuntime+Properties.h"
#include "DKString.h"
#include "DKNumber.h"
#include "DKStruct.h"
#include "DKCopying.h"
#include "DKDescription.h"



static void DKPropertyFinalize( DKObjectRef _self );


DKThreadSafeClassInit( DKPropertyClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKProperty" ), DKObjectClass(), sizeof(struct DKProperty), 0, NULL, DKPropertyFinalize );

    return cls;
}

DKThreadSafeSelectorInit( Property );


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
//  DKInstallProperty()
//
static void DKInstallProperty( DKClassRef _class, DKStringRef name, DKPropertyRef property )
{
    DKAssert( _class && property && name );
    
    struct DKClass * cls = (struct DKClass *)_class;
    
    DKSpinLockLock( &cls->propertiesLock );
    
    if( cls->properties == NULL )
        cls->properties = DKHashTableCreateMutable();
    
    DKHashTableInsertObject( cls->properties, name, property, DKInsertAlways );
    
    DKSpinLockUnlock( &cls->propertiesLock );
}


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
//  DKInstallNumberProperty()
//
void DKInstallNumberProperty( DKClassRef _class,
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


///
//  DKGetAllPropertyDefinitions()
//
DKListRef DKGetAllPropertyDefinitions( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own properties
        if( (cls == DKClassClass()) || (cls == DKRootClass()) )
            cls = (struct DKClass *)_self;

        DKSpinLockLock( &cls->propertiesLock );
        DKListRef properties = DKDictionaryGetAllObjects( cls->properties );
        DKSpinLockUnlock( &cls->propertiesLock );
        
        return properties;
    }
    
    return NULL;
}


///
//  DKGetPropertyDefinition()
//
DKPropertyRef DKGetPropertyDefinition( DKObjectRef _self, DKStringRef name )
{
    if( _self )
    {
        const DKObject * obj = _self;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own properties
        if( (cls == DKClassClass()) || (cls == DKRootClass()) )
            cls = (struct DKClass *)_self;

        DKSpinLockLock( &cls->propertiesLock );
        DKPropertyRef property = DKHashTableGetObject( cls->properties, name );
        DKSpinLockUnlock( &cls->propertiesLock );
        
        return property;
    }
    
    return NULL;
}




// Error Reporting =======================================================================

///
//  CheckPropertyDefined()
//
static void PropertyNotDefined( DKObjectRef _self, DKStringRef name )
{
    DKWarning( "DKProperty: Property '%s' is not defined for class '%s'.\n",
        DKStringGetCStringPtr( name ), DKStringGetCStringPtr( DKGetClassName( _self ) ) );
}

#define CheckPropertyIsDefined( obj, property, name, ... )                              \
    do                                                                                  \
    {                                                                                   \
        if( (property) == NULL )                                                        \
        {                                                                               \
            PropertyNotDefined( obj, name );                                            \
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
    DKWarning( "DKProperty: '%s' does not meet the requirements ('%s') for property '%s'.\n",
        DKStringGetCStringPtr( DKGetClassName( object ) ),
        DKStringGetCStringPtr( DKGetDescription( property->predicate ) ),
        DKStringGetCStringPtr( property->name ) );
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
            return DKAutorelease( DKResolveWeak( *ref ) );
        }
        
        else
        {
            DKObjectRef * ref = value;
            return *ref;
        }
    }
    
    // Automatic conversion of numerical types
    if( DKEncodingIsNumber( property->encoding ) )
    {
        DKNumberRef number = DKNumberCreate( value, property->encoding );
        return DKAutorelease( number );
    }

    // Automatic conversion of structure types
    if( property->semantic != NULL )
    {
        DKStructRef structure = DKStructCreate( property->semantic, value, DKEncodingGetSize( property->encoding ) );
        return DKAutorelease( structure );
    }

    DKWarning( "DKProperty: No available conversion for property '%s' to object.\n",
        DKStringGetCStringPtr( property->name ) );
    
    return NULL;
}


///
//  DKResolveTargetForKeyPath()
//
static bool DKResolveTargetForKeyPath( DKObjectRef root, DKStringRef path, DKObjectRef * target, DKStringRef * key )
{
    if( root )
    {
        DKListRef keys = DKStringCreateListBySeparatingStrings( path, DKSTR( "." ) );
        DKIndex count = DKListGetCount( keys );
        
        if( count == 0 )
        {
            DKRelease( keys );
            return false;
        }
        
        DKObjectRef currTarget = root;
        DKStringRef currKey = DKListGetObjectAtIndex( keys, 0 );
        
        for( DKIndex i = 1; i < count; i++ )
        {
            currTarget = DKGetProperty( currTarget, currKey );
            currKey = DKListGetObjectAtIndex( keys, i );
        }

        *target = currTarget; // Already retained
        *key = DKAutorelease( DKRetain( currKey ) );
        
        DKRelease( keys );
        
        return true;
    }
    
    return false;
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

        if( property == NULL )
        {
            DKPropertyInterfaceRef propertyInterface;
            
            if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
            {
                propertyInterface->setProperty( _self, name, object );
                return;
            }
            
            PropertyNotDefined( _self, name );
            return;
        }
        
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
//  DKSetPropertyForKeyPath()
//
void DKSetPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKObjectRef object )
{
    DKObjectRef target;
    DKStringRef key;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key ) )
        DKSetProperty( target, key, object );
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

        if( property == NULL )
        {
            DKPropertyInterfaceRef propertyInterface;
            
            if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
                return propertyInterface->getProperty( _self, name );
            
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
//  DKGetPropertyForKeyPath()
//
DKObjectRef DKGetPropertyForKeyPath( DKObjectRef _self, DKStringRef path )
{
    DKObjectRef target;
    DKStringRef key;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key ) )
        return DKGetProperty( target, key );

    return NULL;
}



///
//  DKSetNumberProperty()
//
void DKSetNumberProperty( DKObjectRef _self, DKStringRef name, const void * srcValue, DKEncoding srcEncoding )
{
    DKAssert( DKEncodingIsNumber( srcEncoding ) );

    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        if( property == NULL )
        {
            DKPropertyInterfaceRef propertyInterface;
            
            if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
            {
                DKNumberRef number = DKNumberCreate( srcValue, srcEncoding );
                propertyInterface->setProperty( _self, name, number );
                DKRelease( number );
                return;
            }
            
            PropertyNotDefined( _self, name );
            return;
        }

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
//  DKSetNumberPropertyForKeyPath()
//
void DKSetNumberPropertyForKeyPath( DKObjectRef _self, DKStringRef path, const void * srcValue, DKEncoding srcEncoding )
{
    DKObjectRef target;
    DKStringRef key;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key ) )
        DKSetNumberProperty( target, key, srcValue, srcEncoding );
}


///
//  DKGetNumberProperty()
//
static size_t UnpackNumber( DKObjectRef number, void * dstValue, DKEncoding dstEncoding )
{
    size_t result = 0;

    if( number == NULL )
    {
        memset( dstValue, 0, DKEncodingGetSize( dstEncoding ) );
        result = DKEncodingGetCount( dstEncoding );
    }
    
    if( DKIsKindOfClass( number, DKNumberClass() ) )
    {
        result = DKNumberCastValue( number, dstValue, dstEncoding );
    }
    
    return result;
}

size_t DKGetNumberProperty( DKObjectRef _self, DKStringRef name, void * dstValue, DKEncoding dstEncoding )
{
    size_t result = 0;

    DKAssert( DKEncodingIsNumber( dstEncoding ) );

    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        if( property == NULL )
        {
            DKPropertyInterfaceRef propertyInterface;
            
            if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
            {
                DKObjectRef number = propertyInterface->getProperty( _self, name );
                result = UnpackNumber( number, dstValue, dstEncoding );
                DKRelease( number );
                
                if( result != 0 )
                    return result;
            }
            
            else
            {
                PropertyNotDefined( _self, name );
                return 0;
            }
        }
        
        else
        {
            if( property->getter || (property->encoding == DKEncode( DKEncodingTypeObject, 1 )) )
            {
                DKObjectRef number = DKReadPropertyObject( _self, property );
                result = UnpackNumber( number, dstValue, dstEncoding );
                DKRelease( number );
                
                if( result != 0 )
                    return result;
            }
            
            else
            {
                void * srcValue = (uint8_t *)_self + property->offset;

                if( (result = DKNumberConvert( srcValue, property->encoding, dstValue, dstEncoding )) != 0 )
                    return result;
            }
        }

        DKWarning( "DKProperty: No available conversion for property '%s' to %s[%d].\n",
            DKStringGetCStringPtr( name ), DKEncodingGetTypeName( dstEncoding ), DKEncodingGetCount( dstEncoding ) );
    }
    
    return 0;
}


///
//  DKGetNumberPropertyForKeyPath()
//
size_t DKGetNumberPropertyForKeyPath( DKObjectRef _self, DKStringRef path, void * dstValue, DKEncoding dstEncoding )
{
    DKObjectRef target;
    DKStringRef key;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key ) )
        return DKGetNumberProperty( target, key, dstValue, dstEncoding );

    return 0;
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

        if( property == NULL )
        {
            DKPropertyInterfaceRef propertyInterface;
            
            if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
            {
                DKStructRef structure = DKStructCreate( semantic, srcValue, srcSize );
                propertyInterface->setProperty( _self, name, structure );
                DKRelease( structure );
                return;
            }
            
            PropertyNotDefined( _self, name );
            return;
        }

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
//  DKSetStructPropertyForKeyPath()
//
void DKSetStructPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKStringRef semantic, const void * srcValue, size_t srcSize )
{
    DKObjectRef target;
    DKStringRef key;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key ) )
        DKSetStructProperty( target, key, semantic, srcValue, srcSize );
}


///
//  DKGetStructProperty()
//
static size_t UnpackStructure( DKObjectRef structure, DKStringRef semantic, void * dstValue, size_t dstSize )
{
    size_t result = 0;

    if( structure == NULL )
    {
        memset( dstValue, 0, dstSize );
        result = dstSize;
    }
    
    if( DKIsKindOfClass( structure, DKStructClass() ) )
    {
        result = DKStructGetValue( structure, semantic, dstValue, dstSize );
    }
    
    return result;
}

size_t DKGetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, void * dstValue, size_t dstSize )
{
    size_t result = 0;

    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );
        
        if( property == NULL )
        {
            DKPropertyInterfaceRef propertyInterface;
            
            if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
            {
                DKObjectRef structure = propertyInterface->getProperty( _self, name );
                result = UnpackStructure( structure, semantic, dstValue, dstSize );
                DKRelease( structure );
                
                if( result != 0 )
                    return result;
            }
            
            else
            {
                PropertyNotDefined( _self, name );
                return 0;
            }
        }
        
        else
        {
            CheckSemanticRequirement( _self, property, semantic, 0 );
            
            if( property->getter || (property->encoding == DKEncode( DKEncodingTypeObject, 1 )) )
            {
                DKObjectRef structure = DKReadPropertyObject( _self, property );
                result = UnpackStructure( structure, semantic, dstValue, dstSize );
                DKRelease( structure );
                
                if( result != 0 )
                    return result;
            }

            if( property->encoding == DKEncode( DKEncodingTypeBinaryData, dstSize ) )
            {
                const void * srcValue = (uint8_t *)_self + property->offset;
                memcpy( dstValue, srcValue, dstSize );
                return dstSize;
            }
        }
        
        DKWarning( "DKProperty: No available conversion for property '%s' to structure (%s).\n",
            DKStringGetCStringPtr( name ), DKStringGetCStringPtr( semantic ) );
    }
    
    return 0;
}


///
//  DKGetStructPropertyForKeyPath()
//
size_t DKGetStructPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKStringRef semantic, void * dstValue, size_t dstSize )
{
    DKObjectRef target;
    DKStringRef key;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key ) )
        return DKGetStructProperty( target, key, semantic, dstValue, dstSize );

    return 0;
}



///
//  DKSetIntegerProperty()
//
void DKSetIntegerProperty( DKObjectRef _self, DKStringRef name, int64_t x )
{
    DKSetNumberProperty( _self, name, &x, DKNumberInt64 );
}


///
//  DKGetIntegerProperty()
//
int64_t DKGetIntegerProperty( DKObjectRef _self, DKStringRef name )
{
    int64_t x;
    
    if( DKGetNumberProperty( _self, name, &x, DKNumberInt64 ) )
        return x;
    
    return 0;
}


///
//  DKSetFloatProperty()
//
void DKSetFloatProperty( DKObjectRef _self, DKStringRef name, double x )
{
    DKSetNumberProperty( _self, name, &x, DKNumberDouble );
}


///
//  DKGetFloatProperty()
//
double DKGetFloatProperty( DKObjectRef _self, DKStringRef name )
{
    double x;
    
    if( DKGetNumberProperty( _self, name, &x, DKNumberDouble ) )
        return x;
    
    return 0;
}


