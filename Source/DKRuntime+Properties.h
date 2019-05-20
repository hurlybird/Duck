/*****************************************************************************************

  DKRuntime+Properties.h

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

#ifndef _DK_RUNTIME_PROPERTIES_H_
#define _DK_RUNTIME_PROPERTIES_H_

#include "DKEncoding.h"
#include "DKNumber.h"
#include "DKEnum.h"
#include "DKPredicate.h"


// Attributes
enum
{
    // Read-Only property -- attempting to set it will raise a warning
    DKPropertyReadOnly =        (1 << 0),
    
    // The property is stored as a weak reference
    DKPropertyWeak =            (1 << 1),
    
    // The property is stored as a copy (via DKCopy)
    DKPropertyCopy =            (1 << 2),
    
    // The property should not be archived or serialized
    DKPropertyTransient =      (1 << 3)
};


// Custom Getter/Setter
typedef DKObjectRef (*DKPropertyGetter)( DKObjectRef _self, DKPropertyRef property );
typedef void (*DKPropertySetter)( DKObjectRef _self, DKPropertyRef property, DKObjectRef object );
typedef void (*DKPropertyObserver)( DKObjectRef _self, DKPropertyRef property );


// The property definition object (visible for use by custom getters/setters)
struct DKProperty
{
    const DKObject  _obj;
    
    DKStringRef     name;
    DKStringRef     semantic;
    
    DKEncoding      encoding;
    int32_t         attributes;
    size_t          offset;
    
    DKPredicateRef  predicate;
    DKEnumRef       enumType;

    DKPropertyGetter getter;
    DKPropertySetter setter;
    DKPropertyObserver willRead;
    DKPropertyObserver didWrite;
};

// typedef const struct DKProperty * DKPropertyRef; -- Defined in DKPlatform.h

DK_API DKClassRef DKPropertyClass( void );


// Interface for custom get/set property
DK_API DKDeclareInterfaceSelector( Property );

typedef DKObjectRef (*DKGetPropertyMethod)( DKObjectRef _self, DKStringRef name );
typedef void        (*DKSetPropertyMethod)( DKObjectRef _self, DKStringRef name, DKObjectRef object );

struct DKPropertyInterface
{
    const DKInterface _interface;

    DKGetPropertyMethod getProperty;
    DKSetPropertyMethod setProperty;
};

typedef const struct DKPropertyInterface * DKPropertyInterfaceRef;



// Install properties
//
// *** WARNING ***
// Replacing properties after a class is in use (i.e. implementation swizzling) is not
// currently supported.

DK_API void DKInstallObjectProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    DKPredicateRef predicate,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didWrite );

DK_API void DKInstallNumberProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    DKEncoding encoding,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didWrite );

DK_API void DKInstallStructProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    size_t size,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didWrite );

DK_API void DKInstallEnumProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    DKEncoding encoding,
    DKEnumRef enumType,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didWrite );

// Retrieve installed properties
DK_API DKListRef   DKGetAllPropertyDefinitions( DKObjectRef _self );
DK_API DKPropertyRef DKGetPropertyDefinition( DKObjectRef _self, DKStringRef name );

// Set an object property. DKNumbers and DKStructs will be automatically unpacked if the
// property is stored as a number type or structure.
DK_API void        DKTrySetProperty( DKObjectRef _self, DKStringRef name, DKObjectRef object, bool warnIfNotFound );
DK_API void        DKTrySetPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKObjectRef object, bool warnIfNotFound );

#define DKSetProperty( _self, name, object )            DKTrySetProperty( _self, name, object, true )
#define DKSetPropertyForKeyPath( _self, path, object )  DKTrySetPropertyForKeyPath( _self, path, object, true )

// Get an object property. If the property is stored as a number type or structure, the
// value will be automatically packaged in a DKNumber or DKStruct.
DK_API DKObjectRef DKTryGetProperty( DKObjectRef _self, DKStringRef name, bool warnIfNotFound );
DK_API DKObjectRef DKTryGetPropertyForKeyPath( DKObjectRef _self, DKStringRef path, bool warnIfNotFound );

#define DKGetProperty( _self, name )            DKTryGetProperty( _self, name, true )
#define DKGetPropertyForKeyPath( _self, path )  DKTryGetPropertyForKeyPath( _self, path, true )

// Get/Set a numerical property, with automatic conversion to/from storage as a DKNumber object.
DK_API void        DKSetNumberProperty( DKObjectRef _self, DKStringRef name, const void * srcValue, DKEncoding srcEncoding );
DK_API void        DKSetNumberPropertyForKeyPath( DKObjectRef _self, DKStringRef path, const void * srcValue, DKEncoding srcEncoding );

DK_API size_t      DKGetNumberProperty( DKObjectRef _self, DKStringRef name, void * dstValue, DKEncoding dstEncoding );
DK_API size_t      DKGetNumberPropertyForKeyPath( DKObjectRef _self, DKStringRef path, void * dstValue, DKEncoding dstEncoding );

// Get/Set a struct property, with automatic conversion to/from storage as a DKStruct object.
DK_API void        DKSetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, const void * srcValue, size_t srcSize );
DK_API void        DKSetStructPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKStringRef semantic, const void * srcValue, size_t srcSize );

DK_API size_t      DKGetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, void * dstValue, size_t dstSize );
DK_API size_t      DKGetStructPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKStringRef semantic, void * dstValue, size_t dstSize );

// Get/Set wrappers for basic types
DK_API void        DKSetBoolProperty( DKObjectRef _self, DKStringRef name, bool x );
DK_API bool        DKGetBoolProperty( DKObjectRef _self, DKStringRef name );

DK_API void        DKSetIntegerProperty( DKObjectRef _self, DKStringRef name, int64_t x );
DK_API int64_t     DKGetIntegerProperty( DKObjectRef _self, DKStringRef name );

DK_API void        DKSetFloatProperty( DKObjectRef _self, DKStringRef name, double x );
DK_API double      DKGetFloatProperty( DKObjectRef _self, DKStringRef name );

DK_API void        DKSetEnumProperty( DKObjectRef _self, DKStringRef name, int x );
DK_API int         DKGetEnumProperty( DKObjectRef _self, DKStringRef name );



// Private ===============================================================================
#if DK_RUNTIME_PRIVATE

#include "DKHashTable.h"

DKMutableHashTableRef DKCopyPropertiesTable( DKClassRef _class );


#endif // DK_RUNTIME_PRIVATE


#endif // _DK_PROPERTY_H_


