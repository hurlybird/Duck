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


// DKObject ==============================================================================
typedef const struct DKClass * DKClassRef;
typedef const struct DKWeak * DKWeakRef;


struct DKObject
{
    // The class of this object.
    DKClassRef isa;
    
    // Reference count. Never modify this directly.
    int32_t refcount;
    
    // The usage of the object tag is entirely up to individual classes. On 64-bit
    // platforms DKObject must be 8-byte aligned, so this field allows the use of the
    // leftover 4 bytes.
    int32_t tag;
    
    // The weak reference associated with this object. (This may be relocated.)
    DKWeakRef weakref;
};

typedef const struct DKObject DKObject;


// Use this macro when declaring a static instance of a DKObject. This will insulate your
// code from any changes to the DKObject structure. Your code is (obviously) responsible
// for properly initializing and finalizing static objects.
#define DKStaticObject( cls )   { cls, 1, 0, NULL }


// Get/Set the object tag.
#define DKGetObjectTag( obj )           (((DKObject *)(obj))->tag)
#define DKSetObjectTag( obj, value )    do{ ((struct DKObject *)(obj))->tag = (value); } while(0)




// DKSelector ============================================================================
typedef enum
{
    // Use the dynamic cache
    DKDynamicCache =            0,
    
    // 1-7 -- Static cache lines for Duck interfaces
    DKStaticCache_Allocation,
    DKStaticCache_Comparison,
    DKStaticCache_List,
    DKStaticCache_Dictionary,
    
    DKStaticCache_Reserved5,
    DKStaticCache_Reserved6,
    DKStaticCache_Reserved7,
    
    // 8-15 -- Static cache lines for user interfaces
    DKStaticCache_User1,
    DKStaticCache_User2,
    DKStaticCache_User3,
    DKStaticCache_User4,
    
    DKStaticCache_User5,
    DKStaticCache_User6,
    DKStaticCache_User7,
    DKStaticCache_User8,
    
    // Size of the static cache
    DKStaticCacheSize,
    
    // Size of the dynamic cache (must be a power of 2)
    DKDynamicCacheSize =        16
    
} DKCacheUsage;

struct DKSEL
{
    DKObject        _obj;
    const char *    suid;
    DKCacheUsage    cacheline;
};

typedef const struct DKSEL * DKSEL;

#define DKSelector( name )      DKSelector_ ## name()




// DKInterface ===========================================================================
struct DKInterface
{
    DKObject        _obj;
    DKSEL           sel;
};

typedef const struct DKInterface DKInterface;
typedef const void * DKInterfaceRef;

// Declare an interface selector.
#define DKDeclareInterfaceSelector( name )                                              \
    DKSEL DKSelector_ ## name( void )

// A generic interface where calling any method causes a fatal error. Returned by
// DKGetMsgHandler() when a matching message handler cannot be located.
DKInterfaceRef DKInterfaceNotFound( void );




// DKMessage =============================================================================
struct DKMsgHandler
{
    DKObject        _obj;
    DKSEL           sel;
    const void *    func;
};

typedef const struct DKMsgHandler DKMsgHandler;
typedef const struct DKMsgHandler * DKMsgHandlerRef;

// Declare a message handler selector. This also defines a callback type used by
// DKMsgSend() for type safety.
#define DKDeclareMessageSelector( name, ... )                                           \
    DKSEL DKSelector_ ## name( void );                                                  \
    typedef void (*DKMsgHandler_ ## name)( DKObjectRef, DKSEL , ## __VA_ARGS__ )

// A generic message handler that does nothing. Returned by DKGetMsgHandler() when a
// matching message handler cannot be located.
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
    DKObject        _obj;
    
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
DKClassRef DKWeakClass( void );
DKClassRef DKObjectClass( void );



// Default Interfaces ====================================================================

// Allocation ----------------------------------------------------------------------------
DKDeclareInterfaceSelector( Allocation );

typedef DKObjectRef (*DKInitializeMethod)( DKObjectRef _self );
typedef void (*DKFinalizeMethod)( DKObjectRef _self );
typedef void * (*DKAllocMethod)( size_t size );
typedef void (*DKFreeMethod)( void * ptr );

struct DKAllocation
{
    DKInterface _interface;
 
    // All methods are optional -- specify NULL for the default behaviour
    
    // Initializers are called in order (superclass then subclass)
    DKInitializeMethod  initialize;
    
    // Finalizers are called in reverse order (subclass then superclass)
    DKFinalizeMethod    finalize;

    // Custom memory allocation
    DKAllocMethod       alloc;
    DKFreeMethod        free;
};

typedef const struct DKAllocation DKAllocation;

DKInterfaceRef DKDefaultAllocation( void );




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

DKInterfaceRef DKDefaultComparison( void );


// Pointer equality, comparison and hashing
int         DKPointerEqual( DKObjectRef _self, DKObjectRef other );
int         DKPointerCompare( DKObjectRef _self, DKObjectRef other );
DKHashCode  DKPointerHash( DKObjectRef ptr );




// Description ---------------------------------------------------------------------------
DKDeclareInterfaceSelector( Description );

typedef DKStringRef (*DKCopyDescriptionMethod)( DKObjectRef _self );

struct DKDescription
{
    DKInterface _interface;
    
    DKCopyDescriptionMethod copyDescription;
};

typedef const struct DKDescription DKDescription;

DKInterfaceRef DKDefaultDescription( void );


// A default copyDescription method that returns a copy of the class name
DKStringRef DKDefaultCopyDescription( DKObjectRef _self );




// Alloc/Free Objects ====================================================================

// Allocates a new object and calls its intializer chain. Use 'extraBytes' to allocate
// memory in addition to the 'structSize' specified in the class. The extra memory is not
// automatically zeroed for you.
void *      DKAllocObject( DKClassRef cls, size_t extraBytes );

// Calls the object's finalizer chain and deallocates it. You probably never need to call
// this directly unless creating an object that bypasses normal reference counting.
void        DKDeallocObject( DKObjectRef _self );

// Calls the object's initializer chain. The object returned by DKIntializeObject may not
// be the same as the object passed to it. In normal use this is called by DKAllocObject().
DKObjectRef DKInitializeObject( DKObjectRef _self );

// Calls the object's finalizer chain. In normal use this is called by DKDeallocObject().
void        DKFinalizeObject( DKObjectRef _self );




// Creating Classes ======================================================================
enum
{
    // Instances are never allocated
    DKInstancesNeverAllocated =     (1 << 0),

    // Instances are never deallocated (disables reference counting)
    DKInstancesNeverDeallocated =   (1 << 1),
    
    // This class cannot be subclassed
    DKPreventSubclassing =          (1 << 2)
};

typedef uint32_t DKClassOptions;

// Allocate a new class object.
DKClassRef  DKAllocClass( DKStringRef name, DKClassRef superclass, size_t structSize, DKClassOptions options );

// Allocate a new interface object.
void *      DKAllocInterface( DKSEL sel, size_t structSize );

// Install an interface on a class.
//
// *** WARNING ***
// Installing interfaces after a class is in use (i.e. implementation swizzling) is not
// currently supported.
void        DKInstallInterface( DKClassRef cls, DKInterfaceRef interface );

// Install a message handler on a class.
//
// *** WARNING ***
// Installing message handlers after a class is in use (i.e. implementation swizzling) is
// not currently supported.
void        DKInstallMsgHandler( DKClassRef cls, DKSEL sel, const void * func );




// Retrieving Interfaces and Message Handlers ============================================

// Retrieve an installed interface. If a matching interface cannot be found on the class
// or any of its superclasses, DKGetInterace() will report an error and return the
// DKInterfaceNotFound() interface.
DKInterfaceRef DKGetInterface( DKObjectRef _self, DKSEL sel );

// Check to see if an interface is available for an object.
int         DKQueryInterface( DKObjectRef _self, DKSEL sel, DKInterfaceRef * interface );

// Retrieve an installed message handler. If a matching message handler cannot be found on
// the class or any of its superclasses, DKGetMsgHandler() will report a warning and
// return the DKMsgHandlerNotFound() message handler.
DKMsgHandlerRef DKGetMsgHandler( DKObjectRef _self, DKSEL sel );

// Check to see if a message handler is available for an object.
int         DKQueryMsgHandler( DKObjectRef _self, DKSEL sel, DKMsgHandlerRef * msgHandler );




// Reference Counting ====================================================================

DKObjectRef DKRetain( DKObjectRef _self );
void        DKRelease( DKObjectRef _self );

// Get a weak reference to an object. The weak reference itself must be released when the
// caller is finished with it.
DKWeakRef   DKRetainWeak( DKObjectRef _self );

// Resolve a weak reference into a strong reference. The returned object must be released
// when the caller is finished with it. This will return NULL if the object has been
// deallocated.
DKObjectRef DKResolveWeak( DKWeakRef weak_ref );




// Reflection ============================================================================

// Retrieve the class, superclass and class name. These functions return the same values
// for classes and instances (i.e. DKGetClass(DKObjectClass()) == DKObjectClass()).
DKClassRef  DKGetClass( DKObjectRef _self );
DKStringRef DKGetClassName( DKObjectRef _self );
DKClassRef  DKGetSuperclass( DKObjectRef _self );

// Returns TRUE if the object is a member of the class.
int         DKIsMemberOfClass( DKObjectRef _self, DKClassRef _class );

// Returns TRUE if the object is a member or a subclass of the class.
int         DKIsKindOfClass( DKObjectRef _self, DKClassRef _class );




// Polymorphic Wrappers ==================================================================

// Create an instance of a class (identical to DKAllocObject( _class, 0 )).
DKObjectRef DKCreate( DKClassRef _class );

int         DKEqual( DKObjectRef a, DKObjectRef b );
int         DKCompare( DKObjectRef a, DKObjectRef b );
DKHashCode  DKHash( DKObjectRef _self );

DKStringRef DKCopyDescription( DKObjectRef _self );




// Message Passing =======================================================================

// This monstrosity makes method calling somewhat "pretty".
//
// DKMsgSend does three things:
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




// Thread-Safe Object Construction =======================================================

// Thread-safe initialization of shared objects.
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


// Thread-safe creation/initialization of shared class objects.
#define DKThreadSafeClassInit( accessor )                                               \
    DKThreadSafeSharedObjectInit( accessor, DKClassRef )


// Thread-safe creation/initialization of selector objects.
#define DKThreadSafeSelectorInit( name )                                                \
    DKThreadSafeSharedObjectInit( DKSelector_ ## name, DKSEL )                          \
    {                                                                                   \
        struct DKSEL * sel = DKAllocObject( DKSelectorClass(), 0 );                     \
        sel->suid = #name;                                                              \
        sel->cacheline = DKDynamicCache;                                                \
        return sel;                                                                     \
    }

#define DKThreadSafeFastSelectorInit( name )                                            \
    DKThreadSafeSharedObjectInit( DKSelector_ ## name, DKSEL )                          \
    {                                                                                   \
        struct DKSEL * sel = DKAllocObject( DKSelectorClass(), 0 );                     \
        sel->suid = #name;                                                              \
        sel->cacheline = DKStaticCache_ ## name;                                        \
        return sel;                                                                     \
    }


#endif // _DK_RUNTIME_H_





