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
    volatile int32_t refcount;
    uint32_t attributes;
};

typedef const struct DKObjectHeader DKObjectHeader;

enum
{
    // The lower byte is reserved for fast lookup indexes
    DKFastLookupIndexMask =         0xFF,
    
    DKFastLookupLifeCycle =         1,
    DKFastLookupReferenceCounting,
    DKFastLookupComparison,
    DKFastLookupList,
    DKFastLookupDictionary,
    
    DKFastLookupTableSize =         16,

    // The object is statically allocated
    DKObjectIsStatic =              (1 << 8),
    
    // The object is mutable
    DKObjectIsMutable =             (1 << 9),
    
    // The object content is allocated inline with the object structure
    DKObjectContentIsInline =       (1 << 10),
    
    // The object content is managed externally
    DKObjectContentIsExternal =     (1 << 11)
};

#define DKTestObjectAttribute( ref, attr )  ((((const DKObjectHeader *)(ref))->attributes & (attr)) != 0)
#define DKFastLookupIndex( ref )            (((const DKObjectHeader *)(ref))->attributes & DKFastLookupIndexMask)

void DKSetObjectAttribute( DKTypeRef ref, uint32_t attr, int value );


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

#define DKDeclareInterface( name )                                                      \
    extern struct DKSEL DKSelector_ ## name

#define DKDefineInterface( name )                                                       \
    struct DKSEL DKSelector_ ## name =                                                  \
    {                                                                                   \
        { &__DKSelectorClass__, 1, DKObjectIsStatic },                                  \
        #name,                                                                          \
        #name                                                                           \
    }

#define DKDefineFastLookupInterface( name )                                             \
    struct DKSEL DKSelector_ ## name =                                                  \
    {                                                                                   \
        { &__DKSelectorClass__, 1, DKObjectIsStatic | (DKFastLookup ## name) },         \
        #name,                                                                          \
        #name                                                                           \
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

#define DKDefineFastLookupMethod( ret, name, ... )                                      \
    struct DKSEL DKSelector_ ## name =                                                  \
    {                                                                                   \
        { &__DKSelectorClass__, 1, DKObjectIsStatic | (DKFastLookup ## name) },         \
        #name,                                                                          \
        #ret " " #name "( " #__VA_ARGS__ " )"                                           \
    }




// DKProperty ============================================================================
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
    DKPropertyPointer,
    DKPropertyStruct,

} DKPropertyType;

enum
{
    DKPropertyReadOnly =        (1 << 0),
    DKPropertyWeak =            (1 << 1),
    DKPropertyCopy =            (1 << 2)
};

struct DKProperty
{
    DKObjectHeader  _obj;
    
    const char *    name;
    DKPropertyType  type;
    int32_t         attributes;
    size_t          offset;
    size_t          size;
    size_t          count;
    DKSEL           interface;

    void (*setter)( DKTypeRef ref, const struct DKProperty * property, const void * value );
    void (*getter)( DKTypeRef ref, const struct DKProperty * property, void * value );
};

typedef const struct DKProperty DKProperty;




// Root Classes ==========================================================================
DKTypeRef DKClassClass( void );
DKTypeRef DKSelectorClass( void );
DKTypeRef DKInterfaceClass( void );
DKTypeRef DKMethodClass( void );
DKTypeRef DKPropertyClass( void );
DKTypeRef DKObjectClass( void );




// Default Interfaces ====================================================================

// LifeCycle -----------------------------------------------------------------------------
DKDeclareInterface( LifeCycle );

struct DKLifeCycle
{
    DKInterface _interface;
    
    DKTypeRef   (*initialize)( DKTypeRef ref );
    void        (*finalize)( DKTypeRef ref );

    // Custom memory allocation (Optional - these may be NULL)
    void *      (*alloc)( size_t size );
    void        (*free)( void * ptr );
};

typedef const struct DKLifeCycle DKLifeCycle;

DKTypeRef   DKDefaultInitialize( DKTypeRef ref );
void        DKDefaultFinalize( DKTypeRef ref );

DKLifeCycle * DKDefaultLifeCycle( void );


// ReferenceCounting ---------------------------------------------------------------------
DKDeclareInterface( ReferenceCounting );

struct DKReferenceCounting
{
    DKInterface _interface;
    
    DKTypeRef   (*retain)( DKTypeRef ref );
    void        (*release)( DKTypeRef ref );
};

typedef const struct DKReferenceCounting DKReferenceCounting;

DKTypeRef   DKDefaultRetain( DKTypeRef ref );
void        DKDefaultRelease( DKTypeRef ref );

DKReferenceCounting * DKDefaultReferenceCounting( void );


// Comparison ----------------------------------------------------------------------------
DKDeclareInterface( Comparison );

struct DKComparison
{
    DKInterface _interface;
    
    int         (*equal)( DKTypeRef ref, DKTypeRef other );
    int         (*compare)( DKTypeRef ref, DKTypeRef other );
    DKHashIndex (*hash)( DKTypeRef ref );
};

typedef const struct DKComparison DKComparison;

int         DKDefaultEqual( DKTypeRef ref, DKTypeRef other );
int         DKDefaultCompare( DKTypeRef ref, DKTypeRef other );
DKHashIndex DKDefaultHash( DKTypeRef ptr );

DKComparison * DKDefaultComparison( void );




// Alloc/Free Objects ====================================================================
DKTypeRef   DKAllocObject( DKTypeRef isa, size_t extraBytes, uint32_t attributes );
void        DKDeallocObject( DKTypeRef ref );

DKTypeRef   DKCreateClass( DKTypeRef superclass, size_t structSize );
DKTypeRef   DKCreateInterface( DKSEL sel, size_t structSize );


//void        DKRegisterClass( DKTypeRef classObject );

void        DKInstallInterface( DKTypeRef _class, DKTypeRef interface );
void        DKInstallMethod( DKTypeRef _class, DKSEL sel, const void * imp );

DKTypeRef   DKLookupInterface( DKTypeRef ref, DKSEL sel );
DKTypeRef   DKLookupMethod( DKTypeRef ref, DKSEL sel );




// DKObject themed default implementations ===============================================
#define DKObjectInitialize( ref )   DKDefaultInitialize( ref )

#define DKObjectEqual( a, b )       DKDefaultEqual( a, b )
#define DKObjectCompare( a, b )     DKDefaultCompare( a, b )
#define DKObjectHash( ref )         DKDefaultHash( a, b )




// Polymorphic Wrappers ==================================================================

DKTypeRef   DKAlloc( DKTypeRef _class );
DKTypeRef   DKInit( DKTypeRef ref );

#define     DKCreate( _class )  DKInit( DKAlloc( _class ) )

DKTypeRef   DKGetClass( DKTypeRef ref );
int         DKIsMemberOfClass( DKTypeRef ref, DKTypeRef _class );
int         DKIsKindOfClass( DKTypeRef ref, DKTypeRef _class );

DKTypeRef   DKRetain( DKTypeRef ref );
void        DKRelease( DKTypeRef ref );

int         DKEqual( DKTypeRef a, DKTypeRef b );
int         DKCompare( DKTypeRef a, DKTypeRef b );
DKHashIndex DKHash( DKTypeRef ref );



// Method Calling ========================================================================

// This is a monstrosity. It's also necessary to make method calling somewhat "pretty".
//
// DKCallMethod does three things:
//
// 1) Retrieve a DKMethod object from REF using DKSelector_METHOD. This is equivalent to
//    the selector returned by DKSelector( METHOD ).
//
// 2) Cast the method implementation to the DKMethod_METHOD type defined by
//    DKDefineSelector( METHOD ). This provides a modicum of compile-time type checking.
//
// 3) Call the imp function with REF, DKSelector_METHOD and the remaining arguments.
//
//    Note that the GNU C Preprocessor concat operator ## has a special case when used
//    between a comma and __VA_ARGS__: if no variable arguments are supplied, the comma
//    is omitted as well.
//
//    The preprocesser used by Clang seems to support the special case ## syntax as well.
//
//    If the method isn't defined for the object, DKLookupMethod returns a generic
//    implementation that produces an error.

#define DKCallMethod( ref, method, ... ) \
    ((DKMethod_ ## method)((const DKMethod *)DKLookupMethod( ref, &DKSelector_ ## method ))->imp)( ref, &DKSelector_ ## method , ## __VA_ARGS__ )




#endif // _DK_RUNTIME_H_






