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

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"
#include "DKBoolean.h"
#include "DKString.h"
#include "DKNumber.h"
#include "DKStruct.h"
#include "DKPredicate.h"
#include "DKEnum.h"
#include "DKCollection.h"
#include "DKList.h"
#include "DKDictionary.h"
#include "DKHashTable.h"
#include "DKPair.h"
#include "DKCopying.h"
#include "DKConversion.h"
#include "DKDescription.h"



static void DKPropertyFinalize( DKObjectRef _self );


DKThreadSafeClassInit( DKPropertyClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKProperty" ), DKObjectClass(), sizeof(struct DKProperty), 0, NULL, DKPropertyFinalize );

    return cls;
}

DKThreadSafeSelectorInit( Property, struct DKPropertyInterface );
DKThreadSafeSelectorInit( MissingProperty, DKMsgHandler );


///
//  DKPropertyFinalize()
//
static void DKPropertyFinalize( DKObjectRef _untyped_self )
{
    DKPropertyRef _self = _untyped_self;
    
    DKRelease( _self->name );
    DKRelease( _self->semantic );
    DKRelease( _self->predicate );
}




// Defining Properties ===================================================================

///
//  DKInstallProperty()
//
static void DKInstallProperty( DKClassRef _class, DKStringRef name, struct DKProperty * property )
{
    DKAssert( _class && property && name );
    
    DKSpinlockLock( &_class->propertiesLock );
    
    if( _class->properties == NULL )
        _class->properties = DKNewMutableHashTable();
    
    DKHashTableInsertObject( _class->properties, name, property, DKInsertAlways );
    
    DKSpinlockUnlock( &_class->propertiesLock );
}


///
//  DKCopyPropertiesTable()
//
DKMutableHashTableRef DKCopyPropertiesTable( DKClassRef _class )
{
    DKMutableHashTableRef copy = NULL;

    if( _class )
    {
        DKSpinlockLock( &_class->propertiesLock );
        
        if( _class->properties )
            copy = DKCopy( _class->properties );
        
        DKSpinlockUnlock( &_class->propertiesLock );
    }
    
    return copy;
}


///
//  DKInstallObjectProperty()
//
void DKInstallObjectProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    DKPredicateRef predicate,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didRead,
    DKPropertyObserver willWrite,
    DKPropertyObserver didWrite )
{
    DKRequire( (offset >= sizeof(DKObject)) || (getter != NULL) );
    DKRequire( (offset >= sizeof(DKObject)) || (setter != NULL) || (attributes & DKPropertyReadOnly) );
    DKRequire( (attributes & (DKPropertyWeak | DKPropertyNonNull)) != (DKPropertyWeak | DKPropertyNonNull) );

    struct DKProperty * property = DKNew( DKPropertyClass() );
    
    property->name = DKCopy( name );
    property->semantic = DKCopy( semantic );
    
    property->attributes = attributes;
    property->offset = offset;
    property->encoding = DKEncode( DKEncodingTypeObject, 1 );
    
    property->predicate = DKCopy( predicate );
    
    property->getter = getter;
    property->setter = setter;
    property->willRead = willRead;
    property->didRead = didRead;
    property->willWrite = willWrite;
    property->didWrite = didWrite;
    
    DKInstallProperty( _class, name, property );
    DKRelease( property );
}


///
//  DKInstallNumberProperty()
//
void DKInstallNumberProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    DKEncoding encoding,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didRead,
    DKPropertyObserver willWrite,
    DKPropertyObserver didWrite )
{
    DKRequire( DKEncodingIsNumber( encoding ) );
    DKRequire( (offset >= sizeof(DKObject)) || (getter != NULL) );
    DKRequire( (offset >= sizeof(DKObject)) || (setter != NULL) || (attributes & DKPropertyReadOnly) );
    
    struct DKProperty * property = DKNew( DKPropertyClass() );
    
    property->name = DKCopy( name );
    property->semantic = DKCopy( semantic );
    
    property->attributes = attributes;
    property->offset = offset;
    property->encoding = encoding;

    property->getter = getter;
    property->setter = setter;
    property->willRead = willRead;
    property->didRead = didRead;
    property->willWrite = willWrite;
    property->didWrite = didWrite;
    
    DKInstallProperty( _class, name, property );
    DKRelease( property );
}


///
//  DKInstallStructProperty()
//
void DKInstallStructProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    size_t size,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didRead,
    DKPropertyObserver willWrite,
    DKPropertyObserver didWrite )
{
    DKRequire( (offset >= sizeof(DKObject)) || (getter != NULL) );
    DKRequire( (offset >= sizeof(DKObject)) || (setter != NULL) || (attributes & DKPropertyReadOnly) );

    struct DKProperty * property = DKNew( DKPropertyClass() );
    
    property->name = DKCopy( name );
    property->semantic = DKCopy( semantic );

    property->attributes = attributes;
    property->offset = offset;
    property->encoding = DKEncode( DKEncodingTypeBinaryData, (uint32_t)size );

    property->getter = getter;
    property->setter = setter;
    property->willRead = willRead;
    property->didRead = didRead;
    property->willWrite = willWrite;
    property->didWrite = didWrite;
    
    DKInstallProperty( _class, name, property );
    DKRelease( property );
}


///
//  DKInstallEnumProperty()
//
void DKInstallEnumProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    DKEncoding encoding,
    DKEnumRef enumType,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didRead,
    DKPropertyObserver willWrite,
    DKPropertyObserver didWrite )
{
    DKRequire( DKEncodingIsNumber( encoding ) );
    DKRequire( (offset >= sizeof(DKObject)) || (getter != NULL) );
    DKRequire( (offset >= sizeof(DKObject)) || (setter != NULL) || (attributes & DKPropertyReadOnly) );
    
    struct DKProperty * property = DKNew( DKPropertyClass() );
    
    property->name = DKCopy( name );
    property->semantic = DKCopy( semantic );
    
    property->attributes = attributes;
    property->offset = offset;
    property->encoding = encoding;
    property->enumType = DKRetain( enumType );

    property->getter = getter;
    property->setter = setter;
    property->willRead = willRead;
    property->didRead = didRead;
    property->willWrite = willWrite;
    property->didWrite = didWrite;
    
    DKInstallProperty( _class, name, property );
    DKRelease( property );
}


///
//  DKGetDefinedPropertyNames()
//
DKListRef DKGetDefinedPropertyNames( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own properties
        if( (cls == DKClassClass()) || (cls == DKRootClass()) )
            cls = _self;

        DKSpinlockLock( &cls->propertiesLock );
        DKListRef names = DKDictionaryGetAllKeys( (DKDictionaryRef)cls->properties );
        DKSpinlockUnlock( &cls->propertiesLock );

        return names;
    }
    
    return NULL;
}


///
//  DKGetPropertyNames()
//
DKListRef DKGetPropertyNames( DKObjectRef _self )
{
    if( _self )
    {
        DKListRef definedNames = DKGetDefinedPropertyNames( _self );
        DKListRef dynamicNames = DKGetDynamicPropertyNames( _self );
        
        if( definedNames )
        {
            if( dynamicNames )
                return DKListByAppendingCollection( definedNames, dynamicNames );
                
            else
                return definedNames;
        }
        
        else if( dynamicNames )
        {
            return dynamicNames;
        }
    }
    
    return NULL;
}


///
//  DKGetPropertyDefinitions()
//
DKListRef DKGetPropertyDefinitions( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own properties
        if( (cls == DKClassClass()) || (cls == DKRootClass()) )
            cls = _self;

        DKSpinlockLock( &cls->propertiesLock );
        DKListRef properties = DKDictionaryGetAllObjects( (DKDictionaryRef)cls->properties );
        DKSpinlockUnlock( &cls->propertiesLock );
        
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
            cls = _self;

        DKSpinlockLock( &cls->propertiesLock );
        DKPropertyRef property = DKHashTableGetObject( cls->properties, name );
        DKSpinlockUnlock( &cls->propertiesLock );
        
        return property;
    }
    
    return NULL;
}


///
//  DKGetDynamicPropertyNames()
//
DKListRef DKGetDynamicPropertyNames( DKObjectRef _self )
{
    DKPropertyInterfaceRef propertyInterface;
    
    if( DKQueryInterface( _self, DKSelector(Property), (DKInterfaceRef *)&propertyInterface ) )
        return propertyInterface->getNames( _self );
    
    return NULL;
}


///
//  DKCopyProperties()
//
struct DKCopyPropertyContext
{
    DKObjectRef toObject;
    DKObjectRef fromObject;
};

static int DKCopyPropertyCallback( DKObjectRef object, void * _context )
{
    DKStringRef name = object;
    struct DKCopyPropertyContext * context = _context;
    
    DKObjectRef value = DKTryGetProperty( context->fromObject, name, false );
    DKTrySetPropertyForKeyPath( context->toObject, name, value, false );
    
    return 0;
}

void DKCopyProperties( DKObjectRef toObject, DKObjectRef fromObject )
{
    if( toObject && fromObject && (toObject != fromObject) )
    {
        DKCheck( !DKIsClass( toObject ) && !DKIsClass( fromObject ) );
        
        struct DKCopyPropertyContext context;
        context.toObject = toObject;
        context.fromObject = fromObject;
    
        // Copy defined properties
        DKListRef definedNames = DKGetDefinedPropertyNames( fromObject );
        DKForeachObject( definedNames, DKCopyPropertyCallback, &context );
        
        // Copy dynamic properties
        DKListRef dynamicNames = DKGetDynamicPropertyNames( fromObject );
        DKForeachObject( dynamicNames, DKCopyPropertyCallback, &context );
    }
}




// Error Reporting =======================================================================

///
//  FailedToResolveKeyPath()
//
static void FailedToResolveKeyPath( DKObjectRef _self, DKStringRef path )
{
    DKWarning( "DKProperty: Failed to resolve property key path '%@' class '%@'.", path, DKGetClassName( _self ) );
}

///
//  CheckPropertyDefined()
//
static void PropertyNotDefined( DKObjectRef _self, DKStringRef name )
{
    DKWarning( "DKProperty: Property '%@' is not defined for class '%@'.", name, DKGetClassName( _self ) );
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
    DKWarning( "DKProperty: Property '%@' is not read-write.", property->name );
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
//  CheckNonNullRequirement()
//
static void FailedNonNullRequirement( DKObjectRef _self, DKPropertyRef property )
{
    DKWarning( "DKProperty: Property '%@' cannot be NULL.", property->name );
}

#define CheckNonNullRequirement( obj, property, object, ... )                           \
    do                                                                                  \
    {                                                                                   \
        if( ((property)->attributes & DKPropertyNonNull) && (object == NULL) )          \
        {                                                                               \
            FailedNonNullRequirement( obj, property );                                  \
            return __VA_ARGS__;                                                         \
        }                                                                               \
    } while( 0 )



///
//  CheckPredicateRequirement()
//
static void FailedPredicateRequirement( DKObjectRef _self, DKPropertyRef property, DKObjectRef object )
{
    DKWarning( "DKProperty: '%@' does not meet the requirements ('%@') for property '%@'.",
        DKGetClassName( object ), DKGetDescription( property->predicate ), property->name );
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
    DKWarning( "DKProperty: '%@' does not meet the semantic requirement ('%@') for property '%s'.",
        semantic, property->semantic, property->name );
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
//  DKWillReadProperty()
//
static inline void DKWillReadProperty( DKObjectRef _self, DKPropertyRef property )
{
    if( property->willRead )
        property->willRead( _self, property );
}


///
//  DKDidReadProperty()
//
static inline void DKDidReadProperty( DKObjectRef _self, DKPropertyRef property )
{
    if( property->didRead )
        property->didRead( _self, property );
}

///
//  DKWillWriteProperty()
//
static inline void DKWillWriteProperty( DKObjectRef _self, DKPropertyRef property )
{
    if( property->willWrite )
        property->willWrite( _self, property );
}


///
//  DKDidWriteProperty()
//
static inline void DKDidWriteProperty( DKObjectRef _self, DKPropertyRef property )
{
    if( property->didWrite )
        property->didWrite( _self, property );
}


///
//  DKWritePropertyObject()
//
static void DKWritePropertyObject( DKObjectRef _self, DKPropertyRef property, DKObjectRef object )
{
    void * value = (uint8_t *)_self + property->offset;
    
    CheckNonNullRequirement( _self, property, object );
    CheckPredicateRequirement( _self, property, object );

    // Custom setter
    if( property->setter )
    {
        DKWillWriteProperty( _self, property );
        property->setter( _self, property, object );
        DKDidWriteProperty( _self, property );
        return;
    }
    
    // Object types
    if( property->encoding == DKEncode( DKEncodingTypeObject, 1 ) )
    {
        DKObjectRef * ref = value;
        
        DKWillWriteProperty( _self, property );
        
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
        
        DKDidWriteProperty( _self, property );
        return;
    }
    
    // Automatic conversion of enum types from strings
    if( property->enumType )
    {
        if( DKIsKindOfClass( object, DKStringClass() ) )
        {
            int enumValue = DKEnumFromString( property->enumType, object );

            DKWillWriteProperty( _self, property );
            bool success = DKNumberConvert( &enumValue, DKEncodeIntegerType(int), value, property->encoding );
            DKDidWriteProperty( _self, property );

            if( success )
                return;
        }
    }
    
    // Automatic conversion of numerical types, enums and booleans
    if( DKIsKindOfClass( object, DKNumberClass() ) )
    {
        DKWillWriteProperty( _self, property );
        bool success = DKNumberCastValue( object, value, property->encoding );
        DKDidWriteProperty( _self, property );
        
        if( success )
            return;
    }
    
    // Automatic conversion of structure types
    if( DKIsKindOfClass( object, DKStructClass() ) )
    {
        DKWillWriteProperty( _self, property );
        bool success = DKStructGetValue( object, property->semantic, value, DKEncodingGetSize( property->encoding ) );
        DKDidWriteProperty( _self, property );

        if( success )
            return;
    }

    DKWarning( "DKProperty: No available conversion for property '%@' from %@.", property->name, DKGetClassName( object ) );
}


///
//  DKReadPropertyObject()
//
static DKObjectRef DKReadPropertyObject( DKObjectRef _self, DKPropertyRef property )
{
    void * value = (uint8_t *)_self + property->offset;

    // Custom getter
    if( property->getter )
    {
        DKWillReadProperty( _self, property );
        DKObjectRef object = property->getter( _self, property );
        DKDidReadProperty( _self, property );
        
        return object;
    }
    
    // Object types
    else if( property->encoding == DKEncode( DKEncodingTypeObject, 1 ) )
    {
        DKWillReadProperty( _self, property );
        DKObjectRef object;

        if( (*((DKObjectRef *)value) == NULL) && (property->attributes & DKPropertyNonNull) )
        {
            DKMsgSend( _self, MissingProperty, property );
            
            if( *((DKObjectRef *)value) == NULL )
                FailedNonNullRequirement( _self, property );
        }

        if( property->attributes & DKPropertyWeak )
        {
            DKWeakRef * ref = value;
            object = DKAutorelease( DKResolveWeak( *ref ) );
        }
        
        else
        {
            DKObjectRef * ref = value;
            object = DKAutorelease( DKRetain( *ref ) );
        }

        DKDidReadProperty( _self, property );
        
        return object;
    }
    
    // Automatic conversion of enum types to strings
    if( property->enumType )
    {
        int enumValue;
        
        DKWillReadProperty( _self, property );
        bool success = DKNumberConvert( value, property->encoding, &enumValue, DKEncodeIntegerType(int) );
        DKDidReadProperty( _self, property );
        
        if( success )
            return DKStringFromEnum( property->enumType, enumValue );
    }
    
    // Automatic conversion of numerical types
    if( DKEncodingIsNumber( property->encoding ) )
    {
        DKWillReadProperty( _self, property );
        DKNumberRef number = DKNewNumber( value, property->encoding );
        DKDidReadProperty( _self, property );
        
        return DKAutorelease( number );
    }

    // Automatic conversion of structure types
    if( property->semantic != NULL )
    {
        DKWillReadProperty( _self, property );
        DKStructRef structure = DKNewStruct( property->semantic, value, DKEncodingGetSize( property->encoding ) );
        DKDidReadProperty( _self, property );
        
        return DKAutorelease( structure );
    }

    DKWarning( "DKProperty: No available conversion for property '%@' to object.", property->name );
    
    return NULL;
}


///
//  DKResolveTargetForKeyPath()
//
static bool DKResolveTargetForKeyPath( DKObjectRef root, DKStringRef path, DKObjectRef * target, DKStringRef * key, DKPropertyRef * property )
{
    if( root )
    {
        DKObjectRef currTarget = root;
        DKStringRef currKey = path;
        
        while( currTarget && currKey )
        {
            // If the target has a property matching the key, use it. This lets us resolve
            // dotted keys at the end of a key path. We need to check for both an explicit
            // property definition (that may contain NULL), and a dynamic property.
            DKPropertyRef foundProperty = DKGetPropertyDefinition( currTarget, currKey );
            
            if( foundProperty )
            {
                *target = currTarget; // Already retained
                *key = DKAutorelease( DKRetain( currKey ) );
                *property = foundProperty;
                
                return true;
            }

            else
            {
                DKPropertyInterfaceRef propertyInterface;
                
                if( DKQueryInterface( currTarget, DKSelector(Property), (void *)&propertyInterface ) )
                {
                    if( propertyInterface->getProperty( currTarget, currKey ) )
                    {
                        *target = currTarget; // Already retained
                        *key = DKAutorelease( DKRetain( currKey ) );
                        *property = NULL;
                        return true;
                    }
                }
                
                else
                {
                    PropertyNotDefined( currTarget, currKey );
                }
            }

            DKPairRef components = DKStringSplitFirst( currKey, DKSTR( "." ) );
            DKStringRef nextKey = DKPairGetSecondObject( components );
            
            // If we're at the end of the key path, use the current target and key. This
            // lets us add keys to a dynamic key-store.
            if( !nextKey )
            {
                *target = currTarget; // Already retained
                *key = DKAutorelease( DKRetain( currKey ) );
                *property = NULL;
                
                return true;
            }
            
            // Otherwise, step into the target and follow the remainder of the key path
            currTarget = DKGetProperty( currTarget, DKPairGetFirstObject( components ) );
            currKey = nextKey;
        }
    }
    
    return false;
}


///
//  DKTrySetProperty()
//
static bool DKTrySetPropertyInternal( DKObjectRef _self, DKStringRef name, DKPropertyRef property, DKObjectRef object, bool warnIfNotFound )
{
    if( property )
    {
        CheckPropertyIsReadWrite( _self, property, false );
        
        DKWritePropertyObject( _self, property, object );
        
        return true;
    }

    DKPropertyInterfaceRef propertyInterface;
    
    if( DKQueryInterface( _self, DKSelector(Property), (DKInterfaceRef *)&propertyInterface ) )
    {
        if( propertyInterface->setProperty( _self, name, object ) )
            return true;
    }
        
    if( warnIfNotFound )
    {
        PropertyNotDefined( _self, name );
    }
    
    return false;
}

bool DKTrySetProperty( DKObjectRef _self, DKStringRef name, DKObjectRef object, bool warnIfNotFound )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        return DKTrySetPropertyInternal( _self, name, property, object, warnIfNotFound );
    }
    
    return false;
}


///
//  DKTrySetPropertyForKeyPath()
//
bool DKTrySetPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKObjectRef object, bool warnIfNotFound )
{
    DKObjectRef target;
    DKStringRef key;
    DKPropertyRef property;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key, &property ) )
        return DKTrySetPropertyInternal( target, key, property, object, warnIfNotFound );
        
    if( warnIfNotFound )
        FailedToResolveKeyPath( _self, path );
        
    return false;
}


///
//  DKGetProperty()
//
static DKObjectRef DKTryGetPropertyInternal( DKObjectRef _self, DKStringRef name, DKPropertyRef property, bool warnIfNotFound )
{
    if( property )
    {
        return DKReadPropertyObject( _self, property );
    }

    else
    {
        DKPropertyInterfaceRef propertyInterface;
        
        if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
            return propertyInterface->getProperty( _self, name );
        
        if( warnIfNotFound )
            PropertyNotDefined( _self, name );
    }
    
    return NULL;
}

DKObjectRef DKTryGetProperty( DKObjectRef _self, DKStringRef name, bool warnIfNotFound )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        return DKTryGetPropertyInternal( _self, name, property, warnIfNotFound );
    }
    
    return NULL;
}


///
//  DKGetPropertyForKeyPath()
//
DKObjectRef DKTryGetPropertyForKeyPath( DKObjectRef _self, DKStringRef path, bool warnIfNotFound )
{
    DKObjectRef target;
    DKStringRef key;
    DKPropertyRef property;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key, &property ) )
        return DKTryGetPropertyInternal( target, key, property, warnIfNotFound );

    if( warnIfNotFound )
        FailedToResolveKeyPath( _self, path );

    return NULL;
}



///
//  DKSetNumberProperty()
//
static void DKSetNumberPropertyInternal( DKObjectRef _self, DKStringRef name, DKPropertyRef property, const void * srcValue, DKEncoding srcEncoding )
{
    DKAssert( DKEncodingIsNumber( srcEncoding ) );

    if( property == NULL )
    {
        DKPropertyInterfaceRef propertyInterface;
        
        if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
        {
            DKNumberRef number = DKNewNumber( srcValue, srcEncoding );
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
        DKNumberRef number = DKNewNumber( srcValue, srcEncoding );
        DKWritePropertyObject( _self, property, number );
        DKRelease( number );
        return;
    }

    else
    {
        void * dstValue = (uint8_t *)_self + property->offset;

        DKWillWriteProperty( _self, property );
        bool success = DKNumberConvert( srcValue, srcEncoding, dstValue, property->encoding );
        DKDidWriteProperty( _self, property );
        
        if( success )
            return;
    }
    
    DKWarning( "DKProperty: No available conversion for property '%@' from %s[%d].",
        name, DKEncodingGetTypeName( srcEncoding ), DKEncodingGetCount( srcEncoding ) );
}

void DKSetNumberProperty( DKObjectRef _self, DKStringRef name, const void * srcValue, DKEncoding srcEncoding )
{
    DKAssert( DKEncodingIsNumber( srcEncoding ) );

    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        DKSetNumberPropertyInternal( _self, name, property, srcValue, srcEncoding );
    }
}


///
//  DKSetNumberPropertyForKeyPath()
//
void DKSetNumberPropertyForKeyPath( DKObjectRef _self, DKStringRef path, const void * srcValue, DKEncoding srcEncoding )
{
    DKObjectRef target;
    DKStringRef key;
    DKPropertyRef property;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key, &property ) )
        DKSetNumberPropertyInternal( target, key, property, srcValue, srcEncoding );
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

static size_t DKGetNumberPropertyInternal( DKObjectRef _self, DKStringRef name, DKPropertyRef property, void * dstValue, DKEncoding dstEncoding )
{
    size_t result;

    DKAssert( DKEncodingIsNumber( dstEncoding ) );

    if( property == NULL )
    {
        DKPropertyInterfaceRef propertyInterface;
        
        if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
        {
            DKObjectRef number = propertyInterface->getProperty( _self, name );
            result = UnpackNumber( number, dstValue, dstEncoding );
            
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
            
            if( result != 0 )
                return result;
        }
        
        else
        {
            void * srcValue = (uint8_t *)_self + property->offset;

            DKWillReadProperty( _self, property );
            result = DKNumberConvert( srcValue, property->encoding, dstValue, dstEncoding );
            DKDidReadProperty( _self, property );
            
            if( result != 0 )
                return result;
        }
    }

    DKWarning( "DKProperty: No available conversion for property '%@' to %s[%d].",
        name, DKEncodingGetTypeName( dstEncoding ), DKEncodingGetCount( dstEncoding ) );
    
    return 0;
}

size_t DKGetNumberProperty( DKObjectRef _self, DKStringRef name, void * dstValue, DKEncoding dstEncoding )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        return DKGetNumberPropertyInternal( _self, name, property, dstValue, dstEncoding );
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
    DKPropertyRef property;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key, &property ) )
        return DKGetNumberPropertyInternal( target, key, property, dstValue, dstEncoding );

    return 0;
}



///
//  DKSetStructProperty()
//
static void DKSetStructPropertyInternal( DKObjectRef _self, DKStringRef name, DKStringRef semantic, DKPropertyRef property, const void * srcValue, size_t srcSize )
{
    if( property == NULL )
    {
        DKPropertyInterfaceRef propertyInterface;
        
        if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
        {
            DKStructRef structure = DKNewStruct( semantic, srcValue, srcSize );
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
        DKStructRef structure = DKNewStruct( semantic, srcValue, srcSize );
        DKWritePropertyObject( _self, property, structure );
        DKRelease( structure );
        return;
    }

    if( property->encoding == DKEncode( DKEncodingTypeBinaryData, srcSize ) )
    {
        void * dstValue = (uint8_t *)_self + property->offset;

        DKWillWriteProperty( _self, property );
        memcpy( dstValue, srcValue, srcSize );
        DKDidWriteProperty( _self, property );

        return;
    }
    
    DKWarning( "DKProperty: No available conversion for property '%@' from structure (%@).", name, semantic );
}

void DKSetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, const void * srcValue, size_t srcSize )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );

        DKSetStructPropertyInternal( _self, name, semantic, property, srcValue, srcSize );
    }
}


///
//  DKSetStructPropertyForKeyPath()
//
void DKSetStructPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKStringRef semantic, const void * srcValue, size_t srcSize )
{
    DKObjectRef target;
    DKStringRef key;
    DKPropertyRef property;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key, &property ) )
        DKSetStructPropertyInternal( target, key, semantic, property, srcValue, srcSize );
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

static size_t DKGetStructPropertyInternal( DKObjectRef _self, DKStringRef name, DKStringRef semantic, DKPropertyRef property, void * dstValue, size_t dstSize )
{
    size_t result;

    if( property == NULL )
    {
        DKPropertyInterfaceRef propertyInterface;
        
        if( DKQueryInterface( _self, DKSelector(Property), (void *)&propertyInterface ) )
        {
            DKObjectRef structure = propertyInterface->getProperty( _self, name );
            result = UnpackStructure( structure, semantic, dstValue, dstSize );
            
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
            
            if( result != 0 )
                return result;
        }

        if( property->encoding == DKEncode( DKEncodingTypeBinaryData, dstSize ) )
        {
            const void * srcValue = (uint8_t *)_self + property->offset;

            DKWillReadProperty( _self, property );
            memcpy( dstValue, srcValue, dstSize );
            DKDidReadProperty( _self, property );
            
            return dstSize;
        }
    }
    
    DKWarning( "DKProperty: No available conversion for property '%@' to structure (%@).", name, semantic );
    
    return 0;
}

size_t DKGetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, void * dstValue, size_t dstSize )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKPropertyRef property = DKGetPropertyDefinition( obj->isa, name );
        
        return DKGetStructPropertyInternal( _self, name, semantic, property, dstValue, dstSize );
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
    DKPropertyRef property;
    
    if( DKResolveTargetForKeyPath( _self, path, &target, &key, &property ) )
        return DKGetStructPropertyInternal( target, key, semantic, property, dstValue, dstSize );

    return 0;
}


///
//  DKSetBoolProperty()
//
void DKSetBoolProperty( DKObjectRef _self, DKStringRef name, bool x )
{
    DKSetProperty( _self, name, DKBoolean( x ) );
}


///
//  DKGetBoolProperty()
//
bool DKGetBoolProperty( DKObjectRef _self, DKStringRef name )
{
    DKObjectRef x = DKGetProperty( _self, name );

    if( x )
    {
        if( x == DKFalse() )
            return false;
        
        if( x == DKTrue() )
            return true;
        
        DKConversionInterfaceRef conversion;
        
        if( DKQueryInterface( x, DKSelector(Conversion), (DKInterfaceRef *)&conversion) )
            return conversion->getBool( x );
    }
    
    return false;
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


///
//  DKSetEnumProperty()
//
void DKSetEnumProperty( DKObjectRef _self, DKStringRef name, int x )
{
    DKSetNumberProperty( _self, name, &x, DKEncodeIntegerType(int) );
}


///
//  DKGetEnumProperty()
//
int DKGetEnumProperty( DKObjectRef _self, DKStringRef name )
{
    int x;
    
    if( DKGetNumberProperty( _self, name, &x, DKEncodeIntegerType(int) ) )
        return x;
    
    return 0;
}


