//
//  DKProperty.h
//  Duck
//
//  Created by Derek Nylen on 2014-04-07.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

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
    DKPropertyReadOnly =        (1 << 0),
    DKPropertyWeak =            (1 << 1),
    DKPropertyCopy =            (1 << 2)
};


// Getter/Setter
typedef void (*DKPropertySetter)( DKObjectRef _self, DKPropertyRef property, DKObjectRef object );
typedef DKObjectRef (*DKPropertyGetter)( DKObjectRef _self, DKPropertyRef property );


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
    DKStringRef     structHint;

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
    size_t size,
    DKStringRef hint,
    DKPropertySetter setter,
    DKPropertyGetter getter );


// Get/Set Properties
void        DKSetProperty( DKObjectRef _self, DKStringRef name, DKObjectRef object );
DKObjectRef DKGetProperty( DKObjectRef _self, DKStringRef name );

void        DKSetNumericalProperty( DKObjectRef _self, DKStringRef name, const void * srcValue, DKNumberType srcType );
size_t      DKGetNumericalProperty( DKObjectRef _self, DKStringRef name, void * dstValue, DKNumberType dstType );

void        DKSetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef hint, const void * srcValue, size_t srcSize );
size_t      DKGetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef hint, void * dstValue, size_t dstSize );

void        DKSetIntegerProperty( DKObjectRef _self, DKStringRef name, int64_t x );
int64_t     DKGetIntegerProperty( DKObjectRef _self, DKStringRef name );
void        DKSetFloatProperty( DKObjectRef _self, DKStringRef name, double x );
double      DKGetFloatProperty( DKObjectRef _self, DKStringRef name );


#endif // _DK_PROPERTY_H_


