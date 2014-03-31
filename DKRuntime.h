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
};

typedef const struct DKObjectHeader DKObjectHeader;




// DKSelector ============================================================================
typedef enum
{
    DKVTableUnspecified =       0,
    
    DKVTable_LifeCycle,
    DKVTable_ReferenceCounting,
    DKVTable_Comparison,
    DKVTable_List,
    DKVTable_Dictionary,
    
    DKVTableFirstUserIndex =    8,
    
    DKVTableSize =              16
    
} DKVTableIndex;

struct DKSEL
{
    DKObjectHeader  _obj;
    const char *    name;
    const char *    suid;
    DKVTableIndex   vidx;
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
        { &__DKSelectorClass__, 1 },                                                    \
        #name,                                                                          \
        #name,                                                                          \
        0                                                                               \
    }

#define DKDefineFastLookupInterface( name )                                             \
    struct DKSEL DKSelector_ ## name =                                                  \
    {                                                                                   \
        { &__DKSelectorClass__, 1 },                                                    \
        #name,                                                                          \
        #name,                                                                          \
        DKVTable_ ## name                                                               \
    }




// DKMessage =============================================================================
struct DKMsgHandler
{
    DKObjectHeader  _obj;
    DKSEL           sel;
    const void *    func;
};

typedef const struct DKMsgHandler DKMsgHandler;

#define DKDeclareMessage( name, ... )                                                   \
    extern struct DKSEL DKSelector_ ## name;                                            \
    typedef void (*DKMsgHandler_ ## name)( DKTypeRef, DKSEL , ## __VA_ARGS__ )

#define DKDefineMessage( name, ... )                                                    \
    struct DKSEL DKSelector_ ## name =                                                  \
    {                                                                                   \
        { &__DKSelectorClass__, 1 },                                                    \
        #name,                                                                          \
        #name "( " #__VA_ARGS__ " )",                                                   \
        0                                                                               \
    }

#define DKDefineFastLookupMessage( name, ... )                                          \
    struct DKSEL DKSelector_ ## name =                                                  \
    {                                                                                   \
        { &__DKSelectorClass__, 1 },                                                    \
        #name,                                                                          \
        #name "( " #__VA_ARGS__ " )",                                                   \
        DKVTable_ ## name                                                               \
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
DKTypeRef DKMsgHandlerClass( void );
DKTypeRef DKPropertyClass( void );
DKTypeRef DKObjectClass( void );




// Default Interfaces ====================================================================

// LifeCycle -----------------------------------------------------------------------------
DKDeclareInterface( LifeCycle );

struct DKLifeCycle
{
    DKInterface _interface;
 
    // All life-cycle callbacks are optional -- specify NULL for the default behaviour
    
    // Initializers are called in order (superclass then subclass)
    DKTypeRef   (*initialize)( DKTypeRef ref );
    
    // Finalizers are called in reverse order (subclass then superclass)
    void        (*finalize)( DKTypeRef ref );

    // Custom memory allocation
    void *      (*alloc)( size_t size );
    void        (*free)( void * ptr );
};

typedef const struct DKLifeCycle DKLifeCycle;

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

DKTypeRef   DKStaticRetain( DKTypeRef ref );
void        DKStaticRelease( DKTypeRef ref );

DKReferenceCounting * DKDefaultReferenceCounting( void );
DKReferenceCounting * DKStaticReferenceCounting( void );


// Comparison ----------------------------------------------------------------------------
DKDeclareInterface( Comparison );

struct DKComparison
{
    DKInterface _interface;
    
    int         (*equal)( DKTypeRef ref, DKTypeRef other );
    int         (*compare)( DKTypeRef ref, DKTypeRef other );
    DKHashCode  (*hash)( DKTypeRef ref );
};

typedef const struct DKComparison DKComparison;

int         DKDefaultEqual( DKTypeRef ref, DKTypeRef other );
int         DKDefaultCompare( DKTypeRef ref, DKTypeRef other );
DKHashCode  DKDefaultHash( DKTypeRef ptr );

DKComparison * DKDefaultComparison( void );


// Description ---------------------------------------------------------------------------
DKDeclareInterface( Description );

struct DKDescription
{
    DKInterface _interface;
    
    DKStringRef (*copyDescription)( DKTypeRef ref );
};

typedef const struct DKDescription DKDescription;

DKStringRef DKDefaultCopyDescription( DKTypeRef ref );

DKDescription * DKDefaultDescription( void );




// Alloc/Free Objects ====================================================================
DKTypeRef   DKAllocObject( DKTypeRef isa, size_t extraBytes );
void        DKDeallocObject( DKTypeRef ref );

DKTypeRef   DKCreateClass( const char * name, DKTypeRef superclass, size_t structSize );


//void        DKRegisterClass( DKTypeRef classObject );

DKTypeRef   DKCreateInterface( DKSEL sel, size_t structSize );
void        DKInstallInterface( DKTypeRef _class, DKTypeRef interface );

void        DKInstallMsgHandler( DKTypeRef _class, DKSEL sel, const void * func );

int         DKHasInterface( DKTypeRef ref, DKSEL sel );
DKTypeRef   DKGetInterface( DKTypeRef ref, DKSEL sel );

int         DKHasMsgHandler( DKTypeRef ref, DKSEL sel );
DKTypeRef   DKGetMsgHandler( DKTypeRef ref, DKSEL sel );




// DKObject themed default implementations ===============================================
#define DKObjectInitialize( ref )   DKDefaultInitialize( ref )

#define DKObjectEqual( a, b )       DKDefaultEqual( a, b )
#define DKObjectCompare( a, b )     DKDefaultCompare( a, b )
#define DKObjectHash( ref )         DKDefaultHash( a, b )




// Polymorphic Wrappers ==================================================================

DKTypeRef   DKCreate( DKTypeRef _class );

DKTypeRef   DKGetClass( DKTypeRef ref );
DKStringRef DKGetClassName( DKTypeRef ref );
DKTypeRef   DKGetSuperclass( DKTypeRef ref );

int         DKIsMemberOfClass( DKTypeRef ref, DKTypeRef _class );
int         DKIsKindOfClass( DKTypeRef ref, DKTypeRef _class );

DKTypeRef   DKRetain( DKTypeRef ref );
void        DKRelease( DKTypeRef ref );

int         DKEqual( DKTypeRef a, DKTypeRef b );
int         DKCompare( DKTypeRef a, DKTypeRef b );
DKHashCode  DKHash( DKTypeRef ref );

DKStringRef DKCopyDescription( DKTypeRef ref );


// Message Passing =======================================================================

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

#define DKMsgSend( ref, msg, ... ) \
    ((DKMsgHandler_ ## msg)((const DKMsgHandler *)DKGetMsgHandler( ref, &DKSelector_ ## msg ))->func)( ref, &DKSelector_ ## msg , ## __VA_ARGS__ )

#define DKMsgSendSuper( ref, msg, ... ) \
    ((DKMsgHandler_ ## msg)((const DKMsgHandler *)DKGetMsgHandler( DKGetSuperclass( ref ), &DKSelector_ ## msg ))->func)( ref, &DKSelector_ ## msg , ## __VA_ARGS__ )




// Thread-Safe Class Construction ========================================================
#define DKThreadSafeClassInit( name )                                                   \
    static DKTypeRef  name ## _SharedObject = NULL;                                     \
    static DKTypeRef  name ## _Create( void );                                          \
    static DKSpinLock name ## _Lock = DKSpinLockInit;                                   \
                                                                                        \
    DKTypeRef name( void )                                                              \
    {                                                                                   \
        if( name ## _SharedObject == NULL )                                             \
        {                                                                               \
            DKTypeRef cls = name ## _Create();                                          \
                                                                                        \
            DKSpinLockLock( &name ## _Lock );                                           \
                                                                                        \
            if( name ## _SharedObject == NULL )                                         \
                name ## _SharedObject = cls;                                            \
                                                                                        \
            DKSpinLockUnlock( &name ## _Lock );                                         \
                                                                                        \
            if( name ## _SharedObject != cls )                                          \
                DKRelease( cls );                                                       \
        }                                                                               \
                                                                                        \
        return name ## _SharedObject;                                                   \
    }                                                                                   \
                                                                                        \
    static DKTypeRef name ## _Create( void )


#endif // _DK_RUNTIME_H_






