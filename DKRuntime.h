//
//  DKRuntime.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-20.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//
#ifndef _DK_RUNTIME_H_
#define _DK_RUNTIME_H_

#include "DKPlatform.h"


// DKObjectHeader ========================================================================
typedef const struct DKClass * DKClassRef;
typedef const struct DKWeak * DKWeakRef;

struct DKObjectHeader
{
    DKClassRef isa;
    DKWeakRef weakref;
    int32_t refcount;
};

typedef const struct DKObjectHeader DKObjectHeader;

#define DKStaticObjectHeader( cls )     { cls, NULL, 1 }




// DKClass ===============================================================================



// DKSelector ============================================================================
typedef enum
{
    DKVTableUnspecified =       0,
    
    DKVTable_LifeCycle,
    DKVTable_Comparison,
    DKVTable_List,
    DKVTable_Dictionary,
    
    DKVTableFirstUserIndex =    8,
    
    DKVTableSize =              16
    
} DKVTableIndex;

struct DKSEL
{
    DKObjectHeader  _obj;
    const char *    suid;
    DKVTableIndex   vidx;
};

typedef const struct DKSEL * DKSEL;

#define DKSelector( name )      DKSelector_ ## name()




// DKInterface ===========================================================================
struct DKInterface
{
    DKObjectHeader  _obj;
    DKSEL           sel;
};

typedef const struct DKInterface DKInterface;
typedef const void * DKInterfaceRef;



#define DKDeclareInterfaceSelector( name )                                              \
    DKSEL DKSelector_ ## name( void )

DKInterfaceRef DKInterfaceNotFound( void );




// DKMessage =============================================================================
struct DKMsgHandler
{
    DKObjectHeader  _obj;
    DKSEL           sel;
    const void *    func;
};

typedef const struct DKMsgHandler * DKMsgHandlerRef;

#define DKDeclareMessageSelector( name, ... )                                           \
    DKSEL DKSelector_ ## name( void );                                                  \
    typedef void (*DKMsgHandler_ ## name)( DKObjectRef, DKSEL , ## __VA_ARGS__ )

DKMsgHandlerRef DKMsgHandlerNotFound( void );




// DKProperty ============================================================================
typedef enum
{
    DKPropertyType_void =       0,
    
    // Number Types
    DKPropertyInt32,
    DKPropertyInt64,
    DKPropertyUInt32,
    DKPropertyUInt64,
    DKPropertyFloat,
    DKPropertyDouble,

    // Object Types
    DKPropertyObject,
    DKPropertyString,
    
    // Unretained Pointers
    DKPropertyPointer,
    
    // Arbitrary Structures
    DKPropertyStruct,

} DKPropertyType;

enum
{
    DKPropertyReadOnly =        (1 << 0),
    DKPropertyWeak =            (1 << 1),
    DKPropertyCopy =            (1 << 2)
};

typedef const struct DKProperty * DKPropertyRef;

struct DKProperty
{
    DKObjectHeader  _obj;
    
    DKStringRef     name;
    DKPropertyType  type;
    int32_t         attributes;
    size_t          offset;
    size_t          size;
    size_t          count;
    
    DKClassRef      requiredClass;
    DKSEL           requiredInterface;

    void (*setter)( DKObjectRef _self, DKPropertyRef property, const void * value );
    void (*getter)( DKObjectRef _self, DKPropertyRef property, void * value );
};

typedef const struct DKProperty DKProperty;




// Root Classes ==========================================================================
DKClassRef DKClassClass( void );
DKClassRef DKSelectorClass( void );
DKClassRef DKInterfaceClass( void );
DKClassRef DKMsgHandlerClass( void );
DKClassRef DKPropertyClass( void );
DKClassRef DKObjectClass( void );




// Default Interfaces ====================================================================

// LifeCycle -----------------------------------------------------------------------------
DKDeclareInterfaceSelector( LifeCycle );

typedef DKObjectRef (*DKInitializeMethod)( DKObjectRef _self );
typedef void (*DKFinalizeMethod)( DKObjectRef _self );
typedef void * (*DKAllocMethod)( size_t size );
typedef void (*DKFreeMethod)( void * ptr );

struct DKLifeCycle
{
    DKInterface _interface;
 
    // All life-cycle methods are optional -- specify NULL for the default behaviour
    
    // Initializers are called in order (superclass then subclass)
    DKInitializeMethod initialize;
    
    // Finalizers are called in reverse order (subclass then superclass)
    DKFinalizeMethod finalize;

    // Custom memory allocation
    DKAllocMethod alloc;
    DKFreeMethod free;
};

typedef const struct DKLifeCycle DKLifeCycle;

DKInterfaceRef DKDefaultLifeCycle( void );


// Comparison ----------------------------------------------------------------------------
DKDeclareInterfaceSelector( Comparison );

typedef int (*DKEqualMethod)( DKObjectRef a, DKObjectRef b );
typedef int (*DKCompareMethod)( DKObjectRef a, DKObjectRef b );
typedef DKHashCode (*DKHashMethod)( DKObjectRef _self );

struct DKComparison
{
    DKInterface _interface;
    
    DKEqualMethod   equal;
    DKCompareMethod compare;
    DKHashMethod    hash;
};

typedef const struct DKComparison DKComparison;

int         DKDefaultEqual( DKObjectRef _self, DKObjectRef other );
int         DKDefaultCompare( DKObjectRef _self, DKObjectRef other );
DKHashCode  DKDefaultHash( DKObjectRef ptr );

DKInterfaceRef DKDefaultComparison( void );


// Description ---------------------------------------------------------------------------
DKDeclareInterfaceSelector( Description );

typedef DKStringRef (*DKCopyDescriptionMethod)( DKObjectRef _self );

struct DKDescription
{
    DKInterface _interface;
    
    DKCopyDescriptionMethod copyDescription;
};

typedef const struct DKDescription DKDescription;

DKStringRef DKDefaultCopyDescription( DKObjectRef _self );

DKInterfaceRef DKDefaultDescription( void );




// Alloc/Free Objects ====================================================================
void *      DKAllocObject( DKClassRef cls, size_t extraBytes );
void        DKDeallocObject( DKObjectRef _self );

DKClassRef  DKAllocClass( DKStringRef name, DKClassRef superclass, size_t structSize );
void *      DKAllocInterface( DKSEL sel, size_t structSize );

void        DKInstallInterface( DKClassRef cls, DKInterfaceRef interface );
void        DKInstallMsgHandler( DKClassRef cls, DKSEL sel, const void * func );

int         DKHasInterface( DKObjectRef _self, DKSEL sel );
DKInterfaceRef DKGetInterface( DKObjectRef _self, DKSEL sel );

int         DKHasMsgHandler( DKObjectRef _self, DKSEL sel );
DKMsgHandlerRef DKGetMsgHandler( DKObjectRef _self, DKSEL sel );




// Reference Counting ====================================================================

DKObjectRef DKRetain( DKObjectRef _self );
void        DKRelease( DKObjectRef _self );

DKWeakRef   DKRetainWeak( DKObjectRef _self );
DKObjectRef DKResolveWeak( DKWeakRef weak_ref );




// Polymorphic Wrappers ==================================================================

DKObjectRef DKCreate( DKClassRef _class );

DKClassRef  DKGetClass( DKObjectRef _self );
DKStringRef DKGetClassName( DKObjectRef _self );
DKClassRef  DKGetSuperclass( DKObjectRef _self );

int         DKIsMemberOfClass( DKObjectRef _self, DKClassRef _class );
int         DKIsKindOfClass( DKObjectRef _self, DKClassRef _class );

int         DKEqual( DKObjectRef a, DKObjectRef b );
int         DKCompare( DKObjectRef a, DKObjectRef b );
DKHashCode  DKHash( DKObjectRef _self );

DKStringRef DKCopyDescription( DKObjectRef _self );


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

#define DKMsgSend( _self, msg, ... ) \
    ((DKMsgHandler_ ## msg)DKGetMsgHandler( _self, DKSelector(msg) )->func)( _self, DKSelector(msg) , ## __VA_ARGS__ )

#define DKMsgSendSuper( _self, msg, ... ) \
    ((DKMsgHandler_ ## msg)DKGetMsgHandler( DKGetSuperclass( _self ), DKSelector(msg) )->func)( _self, DKSelector(msg) , ## __VA_ARGS__ )




// Thread-Safe Class Construction ========================================================
#define DKThreadSafeSharedObjectInit( accessor, type )                                  \
    static type accessor ## _SharedObject = NULL;                                       \
    static type accessor ## _Create( void );                                            \
                                                                                        \
    type accessor( void )                                                               \
    {                                                                                   \
        if( accessor ## _SharedObject == NULL )                                         \
        {                                                                               \
            type tmp = accessor ## _Create();                                           \
                                                                                        \
            if( !DKAtomicCmpAndSwapPtr( (void * volatile *)&(accessor ## _SharedObject), NULL, (void *)tmp ) ) \
                DKRelease( tmp );                                                       \
        }                                                                               \
                                                                                        \
        return accessor ## _SharedObject;                                               \
    }                                                                                   \
                                                                                        \
    static type accessor ## _Create( void )


#define DKThreadSafeClassInit( accessor )                                               \
    DKThreadSafeSharedObjectInit( accessor, DKClassRef )


#define DKThreadSafeSelectorInit( name )                                                \
    DKThreadSafeSharedObjectInit( DKSelector_ ## name, DKSEL )                          \
    {                                                                                   \
        struct DKSEL * sel = DKAllocObject( DKSelectorClass(), 0 );                     \
        sel->suid = #name;                                                              \
        sel->vidx = 0;                                                                  \
        return sel;                                                                     \
    }

#define DKThreadSafeFastSelectorInit( name )                                            \
    DKThreadSafeSharedObjectInit( DKSelector_ ## name, DKSEL )                          \
    {                                                                                   \
        struct DKSEL * sel = DKAllocObject( DKSelectorClass(), 0 );                     \
        sel->suid = #name;                                                              \
        sel->vidx = DKVTable_ ## name;                                                  \
        return sel;                                                                     \
    }


/*

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
*/


#endif // _DK_RUNTIME_H_






