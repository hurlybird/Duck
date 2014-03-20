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


struct DKSEL
{
    const DKObjectHeader    obj;
    const char *            name;
    const char *            suid;
};

typedef const struct DKSEL * DKSEL;

#define DKSelector( name )  &(DKSelector_ ## name)


typedef struct
{
    const DKObjectHeader    obj;
    DKSEL                   sel;
    
} DKInterface;

#define DKDeclareInterface( name )              \
    extern struct DKSEL DKSelector_ ## name

#define DKDefineInterface( name )               \
    struct DKSEL DKSelector_ ## name =          \
    {                                           \
        { &__DKSelectorClass__, 1 },            \
        #name,                                  \
        #name                                   \
    }


typedef struct
{
    const DKObjectHeader    obj;
    DKSEL                   sel;
    const void *            imp;
    
} DKMethod;

#define DKDeclareMethod( ret, name, ... )               \
    extern struct DKSEL DKSelector_ ## name;            \
    typedef ret (*name)( DKTypeRef, DKSEL , ## __VA_ARGS__ )

#define DKDefineMethod( ret, name, ... )                \
    struct DKSEL DKSelector_ ## name =                  \
    {                                                   \
        { &__DKSelectorClass__, 1 },                    \
        #name,                                          \
        #ret " " #name "( " #__VA_ARGS__ " )"           \
    }

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
    const DKObjectHeader    _obj;
    
    const char *            name;
    DKPropertyType          type;
    int32_t                 attributes;
    size_t                  offset;
    size_t                  size;
    size_t                  count;
    DKSEL                   interface;

    void (*setter)( DKTypeRef ref, const struct DKProperty * property, const void * value );
    void (*getter)( DKTypeRef ref, const struct DKProperty * property, void * value );

} DKProperty;


typedef struct
{
    const DKObjectHeader _obj;
    
    DKTypeRef * interfaces;
    size_t      interfaceCount;
    
    DKTypeRef * methods;
    size_t      methodCount;
    
    DKTypeRef * properties;
    size_t      propertyCount;
    
    // Get interfaces/methods/properties
    DKTypeRef   (* const getInterface)( DKTypeRef ref, DKSEL sel );
    DKTypeRef   (* const getMethod)( DKTypeRef ref, DKSEL sel );
    
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
#define DK_STATIC_CLASS_OBJECT              { &__DKClassClass__, 1 }
#define DKClassClass()                      ((DKTypeRef)&__DKClassClass__)


extern const DKClass __DKSelectorClass__;


extern const DKClass __DKInterfaceClass__;
#define DKStaticInterfaceObject( interface )    { { &__DKInterfaceClass__, 1 }, &(DKSelector_ ## interface) }
#define DKInterfaceClass()                      ((DKTypeRef)&__DKInterfaceClass__)


extern const DKClass __DKMethodClass__;
#define DKStaticMethodObject( method )          { &__DKMethodClass__, 1 }, &(DKSelector_ ## method)
#define DKMethodClass()                         ((DKTypeRef)&__DKMethodClass__)


extern const DKClass __DKObjectClass__;
#define DKObjectClass()                     ((DKTypeRef)&__DKObjectClass__)


#define DKInterfaceTable( table )   table, sizeof(table) / sizeof(DKTypeRef)
#define DKEmptyInterfaceTable()     NULL, 0


#define DKMethodTable( table )      table, sizeof(table) / sizeof(DKTypeRef)
#define DKEmptyMethodTable()        NULL, 0


#define DKPropertyTable( table )    table, sizeof(table) / sizeof(DKTypeRef)
#define DKEmptyPropertyTable()      NULL, 0


// Concrete DKObjectInterface Implementation
DKTypeRef   DKObjectGetInterface( DKTypeRef ref, DKSEL sel );
DKTypeRef   DKObjectGetMethod( DKTypeRef ref, DKSEL sel );

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
DKTypeRef   DKGetInterface( DKTypeRef ref, DKSEL sel );
DKTypeRef   DKGetMethod( DKTypeRef ref, DKSEL sel );

DKTypeRef   DKRetain( DKTypeRef ref );
void        DKRelease( DKTypeRef ref );

int         DKEqual( DKTypeRef a, DKTypeRef b );
int         DKCompare( DKTypeRef a, DKTypeRef b );
DKHashIndex DKHash( DKTypeRef ref );

// Flags
int         DKTestAttribute( DKTypeRef ref, int attr );
#define     DKIsMutable( ref )  DKTestAttribute( (ref), DKObjectMutable )


#define DKCallMethod( ref, method, ... )   ((method)((const DKMethod *)DKGetMethod( ref, &DKSelector_ ## method ))->imp)( ref, &DKSelector_ ## method , ## __VA_ARGS__ )



#endif // DK_OBJECT_H
















