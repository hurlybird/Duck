//
//  DKRuntime.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-20.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//
#ifndef _DK_RUNTIME_H_
#define _DK_RUNTIME_H_

#include "DKEnv.h"


// DKObjectHeader ========================================================================
struct DKObjectHeader
{
    const struct DKClass * isa;
    DKAtomicInt refcount;
    DKAtomicInt attributes;
};

typedef const struct DKObjectHeader DKObjectHeader;

enum
{
    // The objec is statically allocated
    DKObjectIsStatic =          (1 << 0),
    
    // The object is mutable
    DKObjectIsMutable =         (1 << 1),
    
    // The object content is allocated inline with the object structure
    DKObjectContentIsInline =   (1 << 2),
    
    // The object content is managed externally
    DKObjectContentIsExternal = (1 << 3)
};

#define DKTestObjectAttribute( ref, attr )  ((((const DKObjectHeader *)(ref))->attributes & (attr)) != 0)




// DKSelector ============================================================================
struct DKSEL
{
    DKObjectHeader  _obj;
    const char *    name;
    const char *    suid;
};

typedef const struct DKSEL * DKSEL;

extern struct DKClass __DKSelectorClass__;

#define DKSelector( name )  &(DKSelector_ ## name)




// DKInterface ===========================================================================
struct DKInterface
{
    DKObjectHeader  _obj;
    DKSEL           sel;
};

typedef const struct DKInterface DKInterface;

#define DKDeclareInterface( name )                      \
    extern struct DKSEL DKSelector_ ## name

#define DKDefineInterface( name )                       \
    struct DKSEL DKSelector_ ## name =                  \
    {                                                   \
        { &__DKSelectorClass__, 1, DKObjectIsStatic },  \
        #name,                                          \
        #name                                           \
    }




// DKMethod ==============================================================================
struct DKMethod
{
    DKObjectHeader  _obj;
    DKSEL           sel;
    const void *    imp;
};

typedef const struct DKMethod DKMethod;

#define DKDeclareMethod( ret, name, ... )                                               \
    extern struct DKSEL DKSelector_ ## name;                                            \
    typedef ret (*DKMethod_ ## name)( DKTypeRef, DKSEL , ## __VA_ARGS__ )

#define DKDefineMethod( ret, name, ... )                                                \
    struct DKSEL DKSelector_ ## name =                                                  \
    {                                                                                   \
        { &__DKSelectorClass__, 1, DKObjectIsStatic },                                  \
        #name,                                                                          \
        #ret " " #name "( " #__VA_ARGS__ " )"                                           \
    }




// Root Classes ==========================================================================
DKTypeRef   DKClassClass( void );
DKTypeRef   DKSelectorClass( void );
DKTypeRef   DKInterfaceClass( void );
DKTypeRef   DKMethodClass( void );
DKTypeRef   DKPropertyClass( void );
DKTypeRef   DKObjectClass( void );




// Alloc/Free Objects ====================================================================
DKTypeRef   DKAllocObject( DKTypeRef _class, size_t size, int flags );
void        DKFreeObject( DKTypeRef ref );

DKTypeRef   DKAllocClass( DKTypeRef superclass );
DKTypeRef   DKAllocInterface( DKSEL sel, size_t size );

//void        DKRegisterClass( DKTypeRef classObject );

void        DKInstallInterface( DKTypeRef _class, DKTypeRef interface );
DKTypeRef   DKLookupInterface( DKTypeRef ref, DKSEL sel );

void        DKInstallMethod( DKTypeRef _class, DKTypeRef method );
DKTypeRef   DKLookupMethod( DKTypeRef ref, DKSEL sel );




// DKObject themed default implementations ===============================================
#define DKObjectInitialize( ref )   DKDefaultInitializeImp( ref )

#define DKObjectEqual( a, b )       DKDefaultEqualImp( a, b )
#define DKObjectCompare( a, b )     DKDefaultCompareImp( a, b )
#define DKObjectHash( ref )         DKDefaultHashImp( a, b )




// Polymorphic Wrappers ==================================================================

// Allocate a new object and call its default initializer
DKTypeRef   DKCreate( DKTypeRef _class );

DKTypeRef   DKGetClass( DKTypeRef ref );
int         DKIsMemberOfClass( DKTypeRef ref, DKTypeRef _class );
int         DKIsKindOfClass( DKTypeRef ref, DKTypeRef _class );

DKTypeRef   DKRetain( DKTypeRef ref );
void        DKRelease( DKTypeRef ref );

DKTypeRef   DKQueryInterface( DKTypeRef ref, DKSEL sel );
DKTypeRef   DKQueryMethod( DKTypeRef ref, DKSEL sel );

int         DKEqual( DKTypeRef a, DKTypeRef b );
int         DKCompare( DKTypeRef a, DKTypeRef b );
DKHashIndex DKHash( DKTypeRef ref );



// Method Calling ========================================================================

// This is a monstrosity. It's also necessary to make method calling somewhat "pretty".
//
// DKCallMethod does three things:
//
// 1) Retrieve a DKMethod object from REF using the selector DKSelector( METHOD ).
//
// 2) Cast the method implementation to the DKMethod_METHOD type defined by
//    DKDefineSelector( METHOD ). This provides a modicum of compile-time type checking.
//
// 3) Call the imp function with REF, DKSelector( METHOD) and the remaining arguments.
//
//    Note that the GNU C Preprocessor concat operator ## has a special case when used
//    between a comma and __VA_ARGS__: if no variable arguments are supplied, the comma
//    is omitted as well.
//    The preprocesser used by Clang seems to support this syntax as well.
//
//    If the method isn't defined for the object, DKQueryMethod returns a generic
//    implementation that produces an error.

#define DKCallMethod( ref, method, ... ) \
    ((DKMethod_ ## method)((const DKMethod *)DKQueryMethod( ref, &DKSelector_ ## method ))->imp)( ref, &DKSelector_ ## method , ## __VA_ARGS__ )




#endif // _DK_RUNTIME_H_





