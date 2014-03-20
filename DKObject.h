//
//  DKObject.h
//  Duck
//
//  Created by Derek Nylen on 2013-04-11.
//  Copyright (c) 2013 Hurlybird Media. All rights reserved.
//
#ifndef _DK_OBJECT_H_
#define _DK_OBJECT_H_

#include "DKEnv.h"


typedef struct
{
    DKTypeRef   isa;
    DKAtomicInt refcount;
    DKAtomicInt attributes;
    
} DKObjectHeader;


enum
{
    DKObjectMutable =           (1 << 0),
    DKObjectExternalStorage =   (1 << 1)
};


typedef struct
{
    DKSUID      suid;
    DKTypeRef   interface;
    
} DKInterface;


typedef struct
{
    DKSUID      suid;
    DKTypeRef   method;
    
} DKMethod;

typedef struct
{
    const DKObjectHeader _obj;
    const void * function;

} DKMethodImp;

#define DKDeclareMethod( ret, name, ... )       \
    extern struct DKSUID DKSelector_ ## name;   \
    typedef ret (*name)( __VA_ARGS__ )

#define DKDefineMethod( ret, name, ... )        \
    struct DKSUID DKSelector_ ## name = { #name, #ret " " #name "( " #__VA_ARGS__ " )" }

typedef enum
{
    DKPropertyUndefined =       0,
    DKPropertyObject,
    DKPropertyString,
    DKPropertyInt32,
    DKPropertyInt64,
    DKPropertyUnsignedInt32,
    DKPropertyUnsignedInt64,
    DKPropertyFloat32,
    DKPropertyFloat64,

} DKPropertyType;


enum
{
    DKPropertyReadOnly =        (1 << 0),
    DKPropertyWeak =            (1 << 1),
    DKPropertyCopy =            (1 << 2)
};


typedef struct DKProperty
{
    const char *            name;
    DKPropertyType          type;
    int32_t                 attributes;
    size_t                  offset;
    size_t                  size;
    size_t                  count;
    DKSUID                  interface;

    void (*setter)( DKTypeRef ref, const struct DKProperty * property, const void * value );
    void (*getter)( DKTypeRef ref, const struct DKProperty * property, void * value );

} DKProperty;


typedef struct
{
    const DKObjectHeader _obj;
    
    const DKInterface * interfaces;
    const DKMethod * methods;
    const DKProperty * properties;
    
    // Get interfaces/methods/properties
    DKTypeRef   (* const getInterface)( DKTypeRef ref, DKSUID suid );
    DKTypeRef   (* const getMethod)( DKTypeRef ref, DKSUID suid );
    
    // Life-Cycle
    DKTypeRef   (* const retain)( DKTypeRef ref );
    void        (* const release)( DKTypeRef ref );
    
    DKTypeRef   (* const allocate)( void );
    DKTypeRef   (* const initialize)( DKTypeRef ref );
    void        (* const finalize)( DKTypeRef ref );
    
    // Comparison
    int         (* const equal)( DKTypeRef a, DKTypeRef b );
    int         (* const compare)( DKTypeRef a, DKTypeRef b );
    DKHashIndex (* const hash)( DKTypeRef ref );

} DKClass;


extern const DKClass __DKClassClass__;
#define DK_STATIC_CLASS_OBJECT      { &__DKClassClass__, 1 }
#define DKClassClass()              ((DKTypeRef)&__DKClassClass__)


extern const DKClass __DKInterfaceClass__;
#define DK_STATIC_INTERFACE_OBJECT  { &__DKInterfaceClass__, 1 }
#define DKInterfaceClass()          ((DKTypeRef)&__DKInterfaceClass__)


extern const DKClass __DKMethodClass__;
#define DK_STATIC_METHOD_OBJECT     { &__DKMethodClass__, 1 }
#define DKMethodClass()             ((DKTypeRef)&__DKMethodClass__)


extern const DKClass __DKObjectClass__;
#define DKObjectClass()             ((DKTypeRef)&__DKObjectClass__)


extern const DKInterface __DKEmptyInterfaceTable__[];
#define DK_EMPTY_INTERFACE_TABLE    __DKEmptyInterfaceTable__
#define DK_INTERFACE_TABLE_END      { NULL, NULL }


extern const DKMethod __DKEmptyMethodTable__[];
#define DK_EMPTY_METHOD_TABLE       __DKEmptyMethodTable__
#define DK_METHOD_TABLE_END         { NULL, NULL }


extern const DKProperty __DKEmptyPropertyTable__[];
#define DK_EMPTY_PROPERTY_TABLE     __DKEmptyPropertyTable__
#define DK_PROPERTY_TABLE_END       { NULL, }


// Concrete DKObjectInterface Implementation
DKTypeRef   DKObjectGetInterface( DKTypeRef ref, DKSUID suid );
DKTypeRef   DKObjectGetMethod( DKTypeRef ref, DKSUID suid );

DKTypeRef   DKObjectRetain( DKTypeRef ref );
void        DKObjectRelease( DKTypeRef ref );

DKTypeRef   DKObjectAllocate( void );
DKTypeRef   DKObjectInitialize( DKTypeRef ref );
void        DKObjectFinalize( DKTypeRef ref );

#define     DKObjectEqual   DKPtrEqual
#define     DKObjectCompare DKPtrCompare
#define     DKObjectHash    DKPtrHash


// No-Op versions for statically allocated objects
DKTypeRef   DKDoNothingRetain( DKTypeRef ref );
void        DKDoNothingRelease( DKTypeRef ref );
DKTypeRef   DKDoNothingAllocate( void );
DKTypeRef   DKDoNothingInitialize( DKTypeRef ref );
void        DKDoNothingFinalize( DKTypeRef ref );


// Allocate a new object
DKTypeRef   DKNewObject( DKTypeRef _class, size_t size, int flags );


// Polymorphic DKObjectInterface Wrappers
DKTypeRef   DKCreate( DKTypeRef _class );

DKTypeRef   DKGetClass( DKTypeRef ref );
DKTypeRef   DKGetInterface( DKTypeRef ref, DKSUID suid );
DKTypeRef   DKGetMethod( DKTypeRef ref, DKSUID suid );

DKTypeRef   DKRetain( DKTypeRef ref );
void        DKRelease( DKTypeRef ref );

int         DKEqual( DKTypeRef a, DKTypeRef b );
int         DKCompare( DKTypeRef a, DKTypeRef b );
DKHashIndex DKHash( DKTypeRef ref );

// Flags
int         DKTestAttribute( DKTypeRef ref, int attr );
#define     DKIsMutable( ref )  DKTestAttribute( (ref), DKObjectMutable )


#define DKCallMethod( ref, method, ... )   ((method)((DKMethodImp *)DKGetMethod( ref, &DKSelector_ ## method ))->function)( ref , ## __VA_ARGS__ )



#endif // DK_OBJECT_H
















