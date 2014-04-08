/*****************************************************************************************

  DKProperty.h

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

#ifndef _DK_PROPERTY_H_
#define _DK_PROPERTY_H_

#include "DKRuntime.h"
#include "DKNumber.h"


// Types
enum
{
    DKPropertyVoid =        0,
    
    // Object Types
    DKPropertyObject,
    
    // Struct Types
    DKPropertyStruct,
    
    // Numerical Types are defined in DKNumber.h
};


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


// The property definition object (visible for use by custom getters/setters)
struct DKProperty
{
    const DKObject  _obj;
    
    DKStringRef     name;
    int32_t         type;
    int32_t         attributes;
    size_t          offset;
    size_t          size;
    
    DKClassRef      requiredClass;
    DKSEL           requiredInterface;
    DKStringRef     requiredSemantic;

    DKPropertySetter setter;
    DKPropertyGetter getter;
};

// typedef const struct DKProperty * DKPropertyRef; -- Defined in DKPlatform.h

DKClassRef DKPropertyClass( void );


// Defining Properties
void DKInstallObjectProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    DKClassRef requiredClass,
    DKSEL requiredInterface,
    DKPropertySetter setter,
    DKPropertyGetter getter );

void DKInstallNumericalProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    DKNumberType type,
    DKPropertySetter setter,
    DKPropertyGetter getter );

void DKInstallStructProperty( DKClassRef _class,
    DKStringRef name,
    int32_t attributes,
    size_t offset,
    DKStringRef semantic,
    size_t size,
    DKPropertySetter setter,
    DKPropertyGetter getter );


// Set an object property. DKNumbers and DKStructs will be automatically unpacked if the
// property is stored as a number type or structure.
void        DKSetProperty( DKObjectRef _self, DKStringRef name, DKObjectRef object );

// Get an object property. If the property is stored as a number type or structure, the
// value will be automatically packaged in a DKNumber or DKStruct.

// *** YOU MUST RELEASE THE RETURNED OJBECT ***
DKObjectRef DKGetProperty( DKObjectRef _self, DKStringRef name );

// Get/Set a numerical property, with automatic conversion to/from storage as a DKNumber object.
void        DKSetNumericalProperty( DKObjectRef _self, DKStringRef name, const void * srcValue, DKNumberType srcType );
size_t      DKGetNumericalProperty( DKObjectRef _self, DKStringRef name, void * dstValue, DKNumberType dstType );

// Get/Set a struct property, with automatic conversion to/from storage as a DKStruct object.
void        DKSetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, const void * srcValue, size_t srcSize );
size_t      DKGetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, void * dstValue, size_t dstSize );

// Get/Set wrappers for basic types
void        DKSetIntegerProperty( DKObjectRef _self, DKStringRef name, int64_t x );
int64_t     DKGetIntegerProperty( DKObjectRef _self, DKStringRef name );

void        DKSetFloatProperty( DKObjectRef _self, DKStringRef name, double x );
double      DKGetFloatProperty( DKObjectRef _self, DKStringRef name );


#endif // _DK_PROPERTY_H_


