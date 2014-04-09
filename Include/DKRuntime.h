/*****************************************************************************************

  DKRuntime.h

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

#ifndef _DK_RUNTIME_H_
#define _DK_RUNTIME_H_

#include "DKPlatform.h"


// DKObject ==============================================================================
typedef struct DKObject
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
    
} DKObject;


// Use this macro when declaring a static instance of a DKObject. This will insulate your
// code from any changes to the DKObject structure. Your code is (obviously) responsible
// for properly initializing and finalizing static objects.
#define DKStaticObject( cls )   { cls, 1, 0, NULL }


// Get/Set the object tag.
#define DKGetObjectTag( obj )           (((DKObject *)(obj))->tag)
#define DKSetObjectTag( obj, value )    do{ ((DKObject *)(obj))->tag = (value); } while(0)




// DKSelector ============================================================================
typedef enum
{
    // Use the dynamic cache
    DKDynamicCache =            0,
    
    // 1-7 -- Static cache lines for Duck interfaces
    DKStaticCache_Allocation,
    DKStaticCache_Comparison,
    DKStaticCache_Collection,
    DKStaticCache_List,
    DKStaticCache_Dictionary,
    
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

struct _DKSEL
{
    const DKObject  _obj;
    
    // A "sorta" unique identifier. Selectors are compared by pointer value, so
    // this field is mostly for debugging, yet it should still be unique within
    // the context of the application.
    const char *    suid;
    
    // Controls how interfaces retrieved by this selector are cached.
    DKCacheUsage    cacheline;
};

typedef const struct _DKSEL * DKSEL;

#define DKSelector( name )      DKSelector_ ## name()




// DKInterface ===========================================================================
typedef struct DKInterface
{
    const DKObject  _obj;
    DKSEL           sel;
    
} DKInterface;

typedef const void * DKInterfaceRef;

// Declare an interface selector.
#define DKDeclareInterfaceSelector( name )                                              \
    DKSEL DKSelector_ ## name( void )

// A generic interface where calling any method causes a fatal error. Returned by
// DKGetMsgHandler() when a matching message handler cannot be located.
DKInterfaceRef DKInterfaceNotFound( void );




// DKMessage =============================================================================
typedef intptr_t (*DKMsgFunction)( DKObjectRef _self, DKSEL sel );

typedef struct DKMsgHandler
{
    const DKObject  _obj;
    DKSEL           sel;
    DKMsgFunction   func;
    
} DKMsgHandler;

typedef const struct DKMsgHandler * DKMsgHandlerRef;

// Declare a message handler selector. This also defines a callback type used by
// DKMsgSend() for type safety.
#define DKDeclareMessageSelector( name, ... )                                           \
    DKSEL DKSelector_ ## name( void );                                                  \
    typedef intptr_t (*DKMsgHandler_ ## name)( DKObjectRef, DKSEL , ## __VA_ARGS__ )

// A generic message handler that does nothing. Returned by DKGetMsgHandler() when a
// matching message handler cannot be located.
DKMsgHandlerRef DKMsgHandlerNotFound( void );




// Root Classes ==========================================================================
DKClassRef DKClassClass( void );
DKClassRef DKSelectorClass( void );
DKClassRef DKInterfaceClass( void );
DKClassRef DKMsgHandlerClass( void );
DKClassRef DKWeakClass( void );
DKClassRef DKObjectClass( void );



// Default Interfaces ====================================================================

// Allocation ----------------------------------------------------------------------------
DKDeclareInterfaceSelector( Allocation );

typedef DKObjectRef (*DKInitializeMethod)( DKObjectRef _self );
typedef void (*DKFinalizeMethod)( DKObjectRef _self );
typedef void * (*DKAllocMethod)( size_t size );
typedef void (*DKFreeMethod)( void * ptr );

struct DKAllocationInterface
{
    const DKInterface _interface;
 
    // All methods are optional -- specify NULL for the default behaviour
    
    // Initializers are called in order (superclass then subclass)
    DKInitializeMethod  initialize;
    
    // Finalizers are called in reverse order (subclass then superclass)
    DKFinalizeMethod    finalize;

    // Custom memory allocation
    DKAllocMethod       alloc;
    DKFreeMethod        free;
};

typedef const struct DKAllocationInterface * DKAllocationInterfaceRef;

DKInterfaceRef DKDefaultAllocation( void );




// Comparison ----------------------------------------------------------------------------
DKDeclareInterfaceSelector( Comparison );

typedef int (*DKEqualMethod)( DKObjectRef a, DKObjectRef b );
typedef int (*DKCompareMethod)( DKObjectRef a, DKObjectRef b );
typedef DKHashCode (*DKHashMethod)( DKObjectRef _self );

struct DKComparisonInterface
{
    const DKInterface _interface;
    
    DKEqualMethod   equal;
    DKCompareMethod compare;
    DKHashMethod    hash;
};

typedef const struct DKComparisonInterface * DKComparisonInterfaceRef;

DKInterfaceRef DKDefaultComparison( void );


// Pointer equality, comparison and hashing
int         DKPointerEqual( DKObjectRef _self, DKObjectRef other );
int         DKPointerCompare( DKObjectRef _self, DKObjectRef other );
DKHashCode  DKPointerHash( DKObjectRef ptr );




// Description ---------------------------------------------------------------------------
DKDeclareInterfaceSelector( Description );

typedef DKStringRef (*DKCopyDescriptionMethod)( DKObjectRef _self );

struct DKDescriptionInterface
{
    const DKInterface _interface;
    
    DKCopyDescriptionMethod copyDescription;
};

typedef const struct DKDescriptionInterface * DKDescriptionInterfaceRef;

DKInterfaceRef DKDefaultDescription( void );


// A default copyDescription method that returns a copy of the class name
DKStringRef DKDefaultCopyDescription( DKObjectRef _self );




// Alloc/Free Objects ====================================================================

// Allocates a new object Use 'extraBytes' to allocate memory beyond the 'structSize'
// specified in the class. The extra memory is not automatically zeroed for you.
void *      DKAllocObject( DKClassRef cls, size_t extraBytes );

// Deallocates an object created with DKAllocObject. You probably never need to call this
// directly unless creating an object that bypasses normal reference counting.
void        DKDeallocObject( DKObjectRef _self );

// Calls the object's initializer chain. The object returned by DKIntializeObject may not
// be the same as the object passed to it.
void *      DKInitializeObject( DKObjectRef _self );

// Calls the object's finalizer chain.
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
// Replacing interfaces after a class is in use (i.e. implementation swizzling) is not
// currently supported.
void        DKInstallInterface( DKClassRef cls, DKInterfaceRef interface );

// Install a message handler on a class.
//
// *** WARNING ***
// Replacing message handlers after a class is in use (i.e. implementation swizzling) is
// not currently supported.
void        DKInstallMsgHandler( DKClassRef cls, DKSEL sel, DKMsgFunction func );


// Install properties
//
// *** WARNING ***
// Replacing properties after a class is in use (i.e. implementation swizzling) is not
// currently supported.
void DKInstallProperty( DKClassRef _class, DKStringRef name, DKPropertyRef property );




// Retrieving Interfaces, Message Handlers and Properties ================================

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

// Retrieve an installed property
DKPropertyRef DKGetPropertyDefinition( DKObjectRef _self, DKStringRef name );



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

// Returns true if the object is a instance of the class.
int         DKIsMemberOfClass( DKObjectRef _self, DKClassRef _class );

// Returns true if the object is a instance of the class or one of its subclasses.
int         DKIsKindOfClass( DKObjectRef _self, DKClassRef _class );

// Returns true if the class is a subclass of (or equal to) another class
int         DKIsSubclass( DKClassRef _class, DKClassRef otherClass );



// Polymorphic Wrappers ==================================================================

// Wrapper for DKAllocObject + DKInitializeObject
void *      DKCreate( DKClassRef _class );

// Comparison Interface Wrappers
int         DKEqual( DKObjectRef a, DKObjectRef b );
int         DKCompare( DKObjectRef a, DKObjectRef b );
DKHashCode  DKHash( DKObjectRef _self );

// CopyDescription Interface Wrappers
DKStringRef DKCopyDescription( DKObjectRef _self );





// Message Passing =======================================================================

// This monstrosity makes method calling somewhat "pretty".
//
// DKMsgSend does three things:
//
// 1) Retrieve a DKMsgHandler object from REF using DKSelector(msg). This is equivalent to
//    the selector returned by DKSelector( METHOD ).
//
// 2) Cast the method implementation to the DKMethod_METHOD type defined by
//    DKDeclareMsgHandlerSelector( msg ). This provides a modicum of compile-time type
//    checking.
//
// 3) Call the imp function with _self, DKSelector(msg) and the remaining arguments.
//
//    Note that the GNU C Preprocessor concat operator ## has a special case when used
//    between a comma and __VA_ARGS__: if no variable arguments are supplied, the comma
//    is omitted as well.
//
//    The preprocesser used by Clang seems to support the special case ## syntax as well.
//
//    If the method isn't defined for the object, DKGetMsgHandler returns a generic
//    implementation that produces an error.

#define DKMsgSend( _self, msg, ... ) \
    ((DKMsgHandler_ ## msg)DKGetMsgHandler( _self, DKSelector(msg) )->func)( _self, DKSelector(msg) , ## __VA_ARGS__ )
    



// Thread-Safe Object Construction =======================================================

// These macros are for the thread-safe creation of shared object pointers exposed by a
// function accessor. The general strategy is to wrap the object construction in a
// secondary accessory and use an atomic swap to save the shared pointer in a thread-safe
// way.
//
// Shared objects are expected to be created once and never deallocated.
//

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


// Thread-safe initialization of shared class objects.
#define DKThreadSafeClassInit( accessor )                                               \
    DKThreadSafeSharedObjectInit( accessor, DKClassRef )


// Thread-safe initialization of selector objects.
#define DKThreadSafeSelectorInit( name )                                                \
    DKThreadSafeSharedObjectInit( DKSelector_ ## name, DKSEL )                          \
    {                                                                                   \
        struct _DKSEL * sel = DKAllocObject( DKSelectorClass(), 0 );                    \
        sel->suid = #name;                                                              \
        sel->cacheline = DKDynamicCache;                                                \
        return sel;                                                                     \
    }

// Thread-safe initialization of "fast" selectors. Each fast selector is assigned a
// unique, reserved cache line in the interface cache.
#define DKThreadSafeFastSelectorInit( name )                                            \
    DKThreadSafeSharedObjectInit( DKSelector_ ## name, DKSEL )                          \
    {                                                                                   \
        struct _DKSEL * sel = DKAllocObject( DKSelectorClass(), 0 );                    \
        sel->suid = #name;                                                              \
        sel->cacheline = DKStaticCache_ ## name;                                        \
        return sel;                                                                     \
    }


#endif // _DK_RUNTIME_H_






