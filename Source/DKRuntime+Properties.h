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
#include "DKPredicate.h"


// Attributes
enum
{
    // Read-Only property -- attempting to set it will raise a warning
    DKPropertyReadOnly =        (1 << 0),
    
    // The property is stored as a weak reference
    DKPropertyWeak =            (1 << 1),
    
    // The property is stored as a copy (via DKCopy)
    DKPropertyCopy =            (1 << 2)
};


// Custom Getter/Setter
typedef void (*DKPropertySetter)( DKObjectRef _self, DKPropertyRef property, DKObjectRef object );
typedef DKObjectRef (*DKPropertyGetter)( DKObjectRef _self, DKPropertyRef property );
typedef void (*DKPropertyObserver)( DKObjectRef _self, DKPropertyRef property );


// The property definition object (visible for use by custom getters/setters)
struct DKProperty
{
    const DKObject  _obj;
    
    DKStringRef     name;
    
    DKEncoding      encoding;
    int32_t         attributes;
    size_t          offset;
    
    DKStringRef     semantic;
    DKPredicateRef  predicate;

    DKPropertySetter setter;
    DKPropertyGetter getter;
    DKPropertyObserver willRead;
    DKPropertyObserver didWrite;
};

// typedef const struct DKProperty * DKPropertyRef; -- Defined in DKPlatform.h

DKClassRef DKPropertyClass( void );


// Interface for custom get/set property
DKDeclareInterfaceSelector( Property );

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

void DKInstallObjectProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    DKPredicateRef predicate,
    DKPropertySetter setter,
    DKPropertyGetter getter,
    DKPropertyObserver willRead,
    DKPropertyObserver didWrite );

void DKInstallNumberProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    DKEncoding encoding,
    DKStringRef semantic,
    DKPropertySetter setter,
    DKPropertyGetter getter,
    DKPropertyObserver willRead,
    DKPropertyObserver didWrite );

void DKInstallStructProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    size_t size,
    DKStringRef semantic,
    DKPropertySetter setter,
    DKPropertyGetter getter,
    DKPropertyObserver willRead,
    DKPropertyObserver didWrite );

// Retrieve installed properties
DKListRef   DKGetAllPropertyDefinitions( DKObjectRef _self );
DKPropertyRef DKGetPropertyDefinition( DKObjectRef _self, DKStringRef name );

// Set an object property. DKNumbers and DKStructs will be automatically unpacked if the
// property is stored as a number type or structure.
void        DKSetProperty( DKObjectRef _self, DKStringRef name, DKObjectRef object );
void        DKSetPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKObjectRef object );

// Get an object property. If the property is stored as a number type or structure, the
// value will be automatically packaged in a DKNumber or DKStruct.
DKObjectRef DKGetProperty( DKObjectRef _self, DKStringRef name );
DKObjectRef DKGetPropertyForKeyPath( DKObjectRef _self, DKStringRef path );

// Get/Set a numerical property, with automatic conversion to/from storage as a DKNumber object.
void        DKSetNumberProperty( DKObjectRef _self, DKStringRef name, const void * srcValue, DKEncoding srcEncoding );
void        DKSetNumberPropertyForKeyPath( DKObjectRef _self, DKStringRef path, const void * srcValue, DKEncoding srcEncoding );

size_t      DKGetNumberProperty( DKObjectRef _self, DKStringRef name, void * dstValue, DKEncoding dstEncoding );
size_t      DKGetNumberPropertyForKeyPath( DKObjectRef _self, DKStringRef path, void * dstValue, DKEncoding dstEncoding );

// Get/Set a struct property, with automatic conversion to/from storage as a DKStruct object.
void        DKSetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, const void * srcValue, size_t srcSize );
void        DKSetStructPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKStringRef semantic, const void * srcValue, size_t srcSize );

size_t      DKGetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, void * dstValue, size_t dstSize );
size_t      DKGetStructPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKStringRef semantic, void * dstValue, size_t dstSize );

// Get/Set wrappers for basic types
void        DKSetBoolProperty( DKObjectRef _self, DKStringRef name, bool x );
bool        DKGetBoolProperty( DKObjectRef _self, DKStringRef name );

void        DKSetIntegerProperty( DKObjectRef _self, DKStringRef name, int64_t x );
int64_t     DKGetIntegerProperty( DKObjectRef _self, DKStringRef name );

void        DKSetFloatProperty( DKObjectRef _self, DKStringRef name, double x );
double      DKGetFloatProperty( DKObjectRef _self, DKStringRef name );




// Private ===============================================================================
#if DK_RUNTIME_PRIVATE

#include "DKHashTable.h"

DKMutableHashTableRef DKCopyPropertiesTable( DKClassRef _class );


#endif // DK_RUNTIME_PRIVATE


#endif // _DK_PROPERTY_H_


