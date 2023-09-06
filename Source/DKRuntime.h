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

#ifdef __cplusplus
extern "C"
{
#endif


// DKRuntime =============================================================================

enum
{
    DKRuntimeOptionEnableZombieObjects = (1 << 0)
};

// Initialize the library
DK_API void DKRuntimeInit( int options );
DK_API bool DKRuntimeIsInitialized( void );




// DKObject ==============================================================================
typedef struct
{
    // The class of this object.
    DKClassRef isa;
    
    // Reference count. Never modify this directly.
    int32_t refcount;
    
    // The usage of the object tag is entirely up to individual classes. On 64-bit
    // platforms DKObject should be 8-byte aligned anyway, so this field allows the use
    // of the leftover 4 bytes.
    int32_t tag;
    
    // Force a 16 byte struct size on 32-bit systems.
    #if !__LP64__
    int32_t pad;
    #endif
    
} DKObject;


typedef struct
{
    DKObject _obj;
    DKClassRef wasa;
    
} DKZombie;


// Flags that get stored in the 'refcount' field
enum
{
    // Used for reference count overflow checks
    DKRefCountOverflowBit = 0x10000000,
    
    // Reference counting is disabled for the object
    DKRefCountDisabledBit = 0x20000000,
    
    // The object has an associated metadata entry
    DKRefCountMetadataBit = 0x40000000,
    
    // Reserved for future use
    DKRefCountReservedBit = 0x80000000,
    
    // The bits containing the actual reference count
    DKRefCountMask =        0x0fffffff
};


// Use this macro when declaring a static instance of a DKObject to insulate your code
// from any changes to the DKObject structure. Your code is (obviously) responsible for
// properly initializing and finalizing static objects.
#define DKInitStaticObjectHeader( cls ) { cls, DKRefCountDisabledBit | 1, 0 }


// Objects are at least 16 bytes long so there must exist a location in memory that is
// both 16-byte aligned and inside the object. Given that, we can generate a hash code
// from the object pointer that strips out the uninteresting lower bits to make things a
// bit more random. This is particularly important in a hash table that uses hash % prime
// to derive an internal hash code.
#define DKObjectUniqueHash( obj )       ((((uintptr_t)obj) + 15) >> 4)


// Get/Set the object tag.
#define DKGetObjectTag( obj )           (((DKObject *)(obj))->tag)
#define DKSetObjectTag( obj, value )    do{ ((DKObject *)(obj))->tag = (value); } while(0)




// Root Classes ==========================================================================
DK_API DKClassRef DKRootClass( void );
DK_API DKClassRef DKClassClass( void );
DK_API DKClassRef DKSelectorClass( void );
DK_API DKClassRef DKInterfaceClass( void );
DK_API DKClassRef DKMsgHandlerClass( void );
DK_API DKClassRef DKMetadataClass( void );
DK_API DKClassRef DKObjectClass( void );
DK_API DKClassRef DKZombieClass( void );



// Classes ===============================================================================

// Class Options
enum
{
    // Disable allocating of instances
    DKAbstractBaseClass =           (1 << 0),

    // Disable reference counting for instances
    DKDisableReferenceCounting =    (1 << 1),
    
    // The class cannot be subclassed
    DKPreventSubclassing =          (1 << 2),
    
    // Instances of the class are considered immutable
    DKImmutableInstances =          (1 << 3),
    
    // Raise an error if the implicit initializer is called via DKInit() or DKSuperInit()
    DKNoImplicitInitializer =       (1 << 4)
};

typedef DKObjectRef (*DKInitMethod)( DKObjectRef _self );
typedef void (*DKFinalizeMethod)( DKObjectRef _self );

// Create a new class object.
DK_API DKClassRef DKNewClass( DKStringRef name, DKClassRef superclass, size_t structSize,
    uint32_t options, DKInitMethod init, DKFinalizeMethod finalize );





// Objects ===============================================================================

// These functions implement the default allocator for objects. You should never need to
// call them outside of a custom allocation scheme. Use DKAlloc and DKDealloc instead.
DK_API DKObjectRef DKAllocObject( DKClassRef cls, size_t extraBytes );
DK_API void        DKDeallocObject( DKObjectRef _self );

// Allocates a new object using the Allocation interface of its class. Use 'extraBytes' to
// allocate memory beyond the 'structSize' specified by the class. The extra memory is not
// automatically zeroed for you.
#define            DKAlloc( _class )   DKAllocEx( _class, 0 )
DK_API DKObjectRef DKAllocEx( DKClassRef _class, size_t extraBytes );

// Deallocates an object using the Allocation interface of its class. You should never
// need to call this directly unless dealing with an object that bypasses normal reference
// counting.
DK_API void        DKDealloc( DKObjectRef _self );

// Call the default initializer specified by the object's class. The object returned may
// not be the same as the object passed to it.
DK_API DKObjectRef DKInit( DKObjectRef _self );

// Call the default initializer specified by 'superclass'.
DK_API DKObjectRef DKSuperInit( DKObjectRef _self, DKClassRef superclass );

// Call the object's finalizer chain. You should never need to call this directly unless
// dealing with an object that bypasses normal reference counting.
DK_API void        DKFinalize( DKObjectRef _self );

// Wrapper for DKAlloc + DKInit
#define            DKNew( _class )  DKInit( DKAllocEx( _class, 0 ) )

// Simple object synchronization
DK_API void        DKLockObject( DKObjectRef _self );
DK_API void        DKUnlockObject( DKObjectRef _self );




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
    static DKSpinLock accessor ## _SharedObjectLock = DKSpinLockInit;                   \
    static type accessor ## _Create( void );                                            \
                                                                                        \
    type accessor( void )                                                               \
    {                                                                                   \
        if( accessor ## _SharedObject != NULL )                                         \
            return accessor ## _SharedObject;                                           \
                                                                                        \
        DKSpinLockLock( &accessor ## _SharedObjectLock );                               \
                                                                                        \
        if( accessor ## _SharedObject == NULL )                                         \
            accessor ## _SharedObject = accessor ## _Create();                          \
                                                                                        \
        DKSpinLockUnlock( &accessor ## _SharedObjectLock );                             \
                                                                                        \
        return accessor ## _SharedObject;                                               \
    }                                                                                   \
                                                                                        \
    static type accessor ## _Create( void )


// Static accessor version of the previous macro
#define DKThreadSafeStaticObjectInit( accessor, type )                                  \
    static type accessor ## _SharedObject = NULL;                                       \
    static DKSpinLock accessor ## _SharedObjectLock = DKSpinLockInit;                   \
    static type accessor ## _Create( void );                                            \
                                                                                        \
    static type accessor( void )                                                        \
    {                                                                                   \
        if( accessor ## _SharedObject != NULL )                                         \
            return accessor ## _SharedObject;                                           \
                                                                                        \
        DKSpinLockLock( &accessor ## _SharedObjectLock );                               \
                                                                                        \
        if( accessor ## _SharedObject == NULL )                                         \
            accessor ## _SharedObject = accessor ## _Create();                          \
                                                                                        \
        DKSpinLockUnlock( &accessor ## _SharedObjectLock );                             \
                                                                                        \
        return accessor ## _SharedObject;                                               \
    }                                                                                   \
                                                                                        \
    static type accessor ## _Create( void )


// Thread-safe initialization of shared class objects.
#define DKThreadSafeClassInit( accessor )                                               \
    DKThreadSafeSharedObjectInit( accessor, DKClassRef )

#define DKThreadSafeStaticClassInit( accessor )                                         \
    DKThreadSafeStaticObjectInit( accessor, DKClassRef )




// DKSelector ============================================================================

// Selector cache configuration
enum
{
    // 0 - Allocation (Class), Copying (Instances)
    DKStaticCache_Allocation =      0,
    DKStaticCache_Copying =         0,

    // 1 - Comparison
    DKStaticCache_Comparison =      1,
    
    // 2 - Locking
    DKStaticCache_Locking =         2,
    
    // 3-4 - Collections
    DKStaticCache_Collection =      3,
    DKStaticCache_KeyedCollection = 4,
    
    // 5-7 - Containers
    DKStaticCache_List =            5,
    DKStaticCache_Dictionary =      6,
    DKStaticCache_Set =             7,
    
    // 8-12 - I/O and static selectors used by root classes. These don't really need to
    // use the static cache and may be relocated if/when dynamic cache lines are assigned
    // in a different way.
    DKStaticCache_Buffer =          8,
    DKStaticCache_Stream =          9,
    DKStaticCache_Conversion =      10,
    DKStaticCache_Description =     11,
    DKStaticCache_Egg =             12,
    
    // 13-15 - Reserved
    
    // Size of the static cache
    DKStaticCacheSize =             16,
    
    // Size of the dynamic cache
    DKDynamicCacheSize =            16
};


struct _DKSEL
{
    const DKObject  _obj;

    // Selectors are typically compared by pointer value, but the name is required to
    // look up a selector by name.

    // The name database requires that the name field of DKClass and DKSEL is
    // in the same position in the structure (i.e. right after the object header).
    DKStringRef     name;
    
    // Controls how interfaces retrieved by this selector are cached.
    unsigned int    cacheline;
};

typedef struct _DKSEL * DKSEL;


// Offset of the instance interface table for fast selector lookups
// i.e. offsetof( struct DKClass, instanceInterfaceTable )
#if __LP64__
#define DK_INTERFACE_TABLE_OFFSET   0x18
#else
#define DK_INTERFACE_TABLE_OFFSET   0x14
#endif


// A friendly macro for accessing selector objects.
#define DKSelector( name )      DKSelector_ ## name()
#define DKFastSelector( name )  DKStaticCache_ ## name

// A friendly macro for accessing the function prototype for message handlers
#define DKSelectorFunc( name )  DKMsgHandler_ ## name

#define DKSelectorEqual( a, b )         ((a) == (b))
#define DKSelectorCompare( a, b )       DKPointerCompare( a, b )
#define DKSelectorHash( _self )         DKPointerHash( _self )



// Allocate a new selector object.
DK_API DKSEL DKAllocSelector( DKStringRef name );


// Thread-safe initialization of selector objects.
#define DKThreadSafeSelectorInit( name )                                                \
    DKThreadSafeSharedObjectInit( DKSelector_ ## name, DKSEL )                          \
    {                                                                                   \
        return DKAllocSelector( DKSTR( #name ) );                                       \
    }

// Thread-safe initialization of selector objects.
#define DKThreadSafeStaticSelectorInit( name )                                          \
    DKThreadSafeStaticObjectInit( DKSelector_ ## name, DKSEL )                          \
    {                                                                                   \
        return DKAllocSelector( DKSTR( #name ) );                                       \
    }

// Thread-safe initialization of "fast" selectors. Each fast selector is assigned a
// unique, reserved cache line in the interface cache.
#define DKThreadSafeFastSelectorInit( name )                                            \
    DKThreadSafeSharedObjectInit( DKSelector_ ## name, DKSEL )                          \
    {                                                                                   \
        struct _DKSEL * sel = (struct _DKSEL *)DKAllocSelector( DKSTR( #name ) );       \
        sel->cacheline = DKStaticCache_ ## name;                                        \
        return sel;                                                                     \
    }




// Reference Counting ====================================================================

DK_API DKObjectRef DKRetain( DKObjectRef _self );
DK_API DKObjectRef DKRelease( DKObjectRef _self ); // Always returns NULL

// If a call to DKRelease would free the object, do so and return NULL. Otherwise return
// the object unchanged. (This is mainly useful for implementing object caches.)
DK_API DKObjectRef DKTryRelease( DKObjectRef _self );

// Get a weak reference to an object. The weak reference itself must be released when the
// caller is finished with it.
DK_API DKWeakRef   DKRetainWeak( DKObjectRef _self );

// Resolve a weak reference into a strong reference. The returned object must be released
// when the caller is finished with it. This will return NULL if the object has been
// deallocated.
DK_API DKObjectRef DKResolveWeak( DKWeakRef weakref );

// Drain the current autorelease pool
DK_API void        DKDrainAutoreleasePool( void );

// Push/Pop the current autorelease pool
DK_API void        DKPushAutoreleasePool( void );
DK_API void        DKPopAutoreleasePool( void );

DK_API DKObjectRef DKAutorelease( DKObjectRef _self );




// DKInterface ===========================================================================

typedef struct _DKInterface
{
    const DKObject  _obj;
    DKSEL           sel;
    size_t          methodCount;
    // void *       methods[?];
    
} DKInterface;

typedef void * DKInterfaceRef;

// Declare an interface selector.
#define DKDeclareInterfaceSelector( name )                                              \
    DKSEL DKSelector_ ## name( void )

#define DKDeclareFastInterfaceSelector( name, cacheline )                               \
    DKSEL DKSelector_ ## name( void );                                                  \
    enum { DKStaticCache_ ##name = cacheline };

// Get the interface method count from the structure size
#define DKInterfaceCountMethods( structSize )    (((structSize) - sizeof(DKInterface)) / sizeof(void *))

// Get the interface method table
#define DKInterfaceGetMethodTable( _interface )   (void **)(((uint8_t *)(_interface)) + sizeof(DKInterface))

// Create a new interface object.
DK_API DKInterfaceRef DKNewInterface( DKSEL sel, size_t structSize );

// Inherit undefined methods from another class. This is automatically done for the
// superclass of '_class' when installing the interface.
DK_API void DKInterfaceInheritMethods( DKInterfaceRef _interface, DKClassRef _class );

// Install an interface on a class.
//
// *** WARNING ***
// Replacing interfaces after a class is in use (i.e. implementation swizzling) is not
// currently supported.
DK_API void DKInstallInterface( DKClassRef cls, DKInterfaceRef _interface );
DK_API void DKInstallClassInterface( DKClassRef _class, DKInterfaceRef _interface );

// Retrieve an installed interface. If a matching interface cannot be found on the class
// or any of its superclasses, DKGetInterace() will report an error and return the
// DKInterfaceNotFound() interface.
DK_API DKInterfaceRef DKGetInterface( DKObjectRef _self, DKSEL sel );
DK_API DKInterfaceRef DKGetClassInterface( DKClassRef _class, DKSEL sel );

// Check to see if an interface is available for an object.
DK_API bool DKQueryInterface( DKObjectRef _self, DKSEL sel, DKInterfaceRef * _interface );
DK_API bool DKQueryClassInterface( DKClassRef _class, DKSEL sel, DKInterfaceRef * _interface );

// Retrieve a static interface fast by bypassing all safety checks
static inline DKInterfaceRef DKGetStaticInterfaceFastUnsafe( DKObjectRef _self, unsigned int cacheline )
{
    DKAssert( _self && (cacheline < DKStaticCacheSize) );

    DKObject * obj = (DKObject *)_self;
    DKClassRef cls = obj->isa;
    DKInterfaceRef * table = (DKInterfaceRef *)((uint8_t *)cls + DK_INTERFACE_TABLE_OFFSET);
    
    DKAssert( table[cacheline] );
    return table[cacheline];
}




// DKMsgHandler ==========================================================================

// Common Message Handler Prototypes
typedef intptr_t (*DKMsgFunction)( DKObjectRef _self, DKSEL sel );
typedef intptr_t (*DKMsgFunction1)( DKObjectRef _self, DKSEL sel, DKObjectRef obj );
typedef intptr_t (*DKMsgFunction2)( DKObjectRef _self, DKSEL sel, DKObjectRef obj1, DKObjectRef obj2 );

typedef struct DKMsgHandler
{
    const DKObject  _obj;
    DKSEL           sel;
    size_t          methodCount;
    DKMsgFunction   func;
    
} DKMsgHandler;

typedef struct DKMsgHandler * DKMsgHandlerRef;

// Declare a message handler selector. This also defines a callback type used by
// DKMsgSend() for type safety.
#define DKDeclareMessageSelector( name, ... )                                           \
    DKSEL DKSelector_ ## name( void );                                                  \
    typedef intptr_t (*DKMsgHandler_ ## name)( DKObjectRef, DKSEL , ## __VA_ARGS__ )

#define DKDeclareFastMessageSelector( name, cacheline, ... )                            \
    enum { DKStaticCache_ ##name = cacheline };                                         \
    DKSEL DKSelector_ ## name( void );                                                  \
    typedef intptr_t (*DKMsgHandler_ ## name)( DKObjectRef, DKSEL , ## __VA_ARGS__ )

// Return true if an object will respond to a message though dynamic message handling. The
// dynamic message handler for an object WILL NOT be called if this message returns false.
DK_API DKDeclareMessageSelector( DKRespondsToDynamicMsg, DKSEL msg );

// Selector for dynamic message handling. If defined for a class, the dynamic message
// handler is returned by DKGetMsgHandler or DKQueryMsgHandler when a message handler
// cannot be located and DKRespondsToDynamicMsg return true.
DK_API DKDeclareMessageSelector( DKDynamicMsgHandler );

// A generic message handler that does nothing. Returned by DKGetMsgHandler() when a
// matching message handler cannot be located.
DK_API DKMsgHandlerRef DKMsgHandlerNotFound( void );

// Install a message handler on a class.
//
// *** WARNING ***
// Replacing message handlers after a class is in use (i.e. implementation swizzling) is
// not currently supported.
DK_API void DKInstallMsgHandler( DKClassRef cls, DKSEL sel, DKMsgFunction func );
DK_API void DKInstallClassMsgHandler( DKClassRef cls, DKSEL sel, DKMsgFunction func );

// Retrieve an installed message handler. If a matching message handler cannot be found on
// the class or any of its superclasses, DKGetMsgHandler() will report a warning and
// return the DKMsgHandlerNotFound() message handler.
DK_API DKMsgHandlerRef DKGetMsgHandler( DKObjectRef _self, DKSEL sel );
DK_API DKMsgHandlerRef DKGetClassMsgHandler( DKClassRef _class, DKSEL sel );

// Check to see if a message handler is available for an object.
DK_API bool DKQueryMsgHandler( DKObjectRef _self, DKSEL sel, DKMsgHandlerRef * msgHandler );
DK_API bool DKQueryClassMsgHandler( DKClassRef _class, DKSEL sel, DKMsgHandlerRef * msgHandler );

// Retrieve a static interface fast by bypassing all safety checks
static inline DKMsgHandlerRef DKGetStaticMsgHandlerFastUnsafe( DKObjectRef _self, unsigned int cacheline )
{
    DKAssert( _self && (cacheline < DKStaticCacheSize) );

    DKObject * obj = (DKObject *)_self;
    DKClassRef cls = obj->isa;
    DKInterfaceRef * table = (DKInterfaceRef *)((uint8_t *)cls + DK_INTERFACE_TABLE_OFFSET);

    DKAssert( table[cacheline] );
    return (DKMsgHandlerRef)table[cacheline];
}




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


#define DKMsgSendf( ftype, _self, sel, ... ) \
    ((ftype)(DKGetMsgHandler( _self, sel )->func))( _self, sel , ## __VA_ARGS__ )




// DKProperty ============================================================================

// Attributes
enum
{
    // Read-Only property -- attempting to set it will raise a warning
    DKPropertyReadOnly =        (1 << 0),
    
    // The property is stored as a weak reference
    DKPropertyWeak =            (1 << 1),
    
    // The property is stored as a copy (via DKCopy)
    DKPropertyCopy =            (1 << 2),
    
    // The property should not be archived or serialized
    DKPropertyTransient =      (1 << 3)
};


// Custom Getter/Setter
typedef DKObjectRef (*DKPropertyGetter)( DKObjectRef _self, DKPropertyRef property );
typedef void (*DKPropertySetter)( DKObjectRef _self, DKPropertyRef property, DKObjectRef object );
typedef void (*DKPropertyObserver)( DKObjectRef _self, DKPropertyRef property );


// The property definition object (visible for use by custom getters/setters)
struct DKProperty
{
    const DKObject  _obj;
    
    DKStringRef     name;
    DKStringRef     semantic;
    
    DKEncoding      encoding;
    int32_t         attributes;
    size_t          offset;
    
    DKPredicateRef  predicate;
    DKEnumRef       enumType;

    DKPropertyGetter getter;
    DKPropertySetter setter;
    
    DKPropertyObserver willRead;
    DKPropertyObserver didRead;
    
    DKPropertyObserver willWrite;
    DKPropertyObserver didWrite;
};

// typedef const struct DKProperty * DKPropertyRef; -- Defined in DKPlatform.h

DK_API DKClassRef DKPropertyClass( void );


// Interface for custom get/set property
DK_API DKDeclareInterfaceSelector( Property );

typedef DKObjectRef (*DKGetPropertyMethod)( DKObjectRef _self, DKStringRef name );
typedef void        (*DKSetPropertyMethod)( DKObjectRef _self, DKStringRef name, DKObjectRef object );

struct DKPropertyInterface
{
    const DKInterface _interface;

    DKGetPropertyMethod getProperty;
    DKSetPropertyMethod setProperty;
};

typedef const struct DKPropertyInterface * DKPropertyInterfaceRef;


// Parameter macros to make using the following functions a bit more readable
#define DK_NO_XETTERS           NULL, NULL
#define DK_NO_READ_OBSERVERS    NULL, NULL
#define DK_NO_WRITE_OBSERVERS   NULL, NULL
#define DK_NO_OBSERVERS         NULL, NULL, NULL, NULL

// Install properties
//
// *** WARNING ***
// Replacing properties after a class is in use (i.e. implementation swizzling) is not
// currently supported.

DK_API void DKInstallObjectProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    DKPredicateRef predicate,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didRead,
    DKPropertyObserver willWrite,
    DKPropertyObserver didWrite );

DK_API void DKInstallNumberProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    DKEncoding encoding,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didRead,
    DKPropertyObserver willWrite,
    DKPropertyObserver didWrite );

DK_API void DKInstallStructProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    size_t size,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didRead,
    DKPropertyObserver willWrite,
    DKPropertyObserver didWrite );

DK_API void DKInstallEnumProperty( DKClassRef _class,
    DKStringRef name,
    DKStringRef semantic,
    int32_t attributes,
    size_t offset,
    DKEncoding encoding,
    DKEnumRef enumType,
    DKPropertyGetter getter,
    DKPropertySetter setter,
    DKPropertyObserver willRead,
    DKPropertyObserver didRead,
    DKPropertyObserver willWrite,
    DKPropertyObserver didWrite );

// Retrieve installed properties
DK_API DKListRef   DKGetAllPropertyDefinitions( DKObjectRef _self );
DK_API DKPropertyRef DKGetPropertyDefinition( DKObjectRef _self, DKStringRef name );

// Set an object property. DKNumbers and DKStructs will be automatically unpacked if the
// property is stored as a number type or structure.
DK_API void        DKTrySetProperty( DKObjectRef _self, DKStringRef name, DKObjectRef object, bool warnIfNotFound );
DK_API void        DKTrySetPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKObjectRef object, bool warnIfNotFound );

#define DKSetProperty( _self, name, object )            DKTrySetProperty( _self, name, object, true )
#define DKSetPropertyForKeyPath( _self, path, object )  DKTrySetPropertyForKeyPath( _self, path, object, true )

// Get an object property. If the property is stored as a number type or structure, the
// value will be automatically packaged in a DKNumber or DKStruct.
DK_API DKObjectRef DKTryGetProperty( DKObjectRef _self, DKStringRef name, bool warnIfNotFound );
DK_API DKObjectRef DKTryGetPropertyForKeyPath( DKObjectRef _self, DKStringRef path, bool warnIfNotFound );

#define DKGetProperty( _self, name )            DKTryGetProperty( _self, name, true )
#define DKGetPropertyForKeyPath( _self, path )  DKTryGetPropertyForKeyPath( _self, path, true )

// Get/Set a numerical property, with automatic conversion to/from storage as a DKNumber object.
DK_API void        DKSetNumberProperty( DKObjectRef _self, DKStringRef name, const void * srcValue, DKEncoding srcEncoding );
DK_API void        DKSetNumberPropertyForKeyPath( DKObjectRef _self, DKStringRef path, const void * srcValue, DKEncoding srcEncoding );

DK_API size_t      DKGetNumberProperty( DKObjectRef _self, DKStringRef name, void * dstValue, DKEncoding dstEncoding );
DK_API size_t      DKGetNumberPropertyForKeyPath( DKObjectRef _self, DKStringRef path, void * dstValue, DKEncoding dstEncoding );

// Get/Set a struct property, with automatic conversion to/from storage as a DKStruct object.
DK_API void        DKSetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, const void * srcValue, size_t srcSize );
DK_API void        DKSetStructPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKStringRef semantic, const void * srcValue, size_t srcSize );

DK_API size_t      DKGetStructProperty( DKObjectRef _self, DKStringRef name, DKStringRef semantic, void * dstValue, size_t dstSize );
DK_API size_t      DKGetStructPropertyForKeyPath( DKObjectRef _self, DKStringRef path, DKStringRef semantic, void * dstValue, size_t dstSize );

// Get/Set wrappers for basic types
DK_API void        DKSetBoolProperty( DKObjectRef _self, DKStringRef name, bool x );
DK_API bool        DKGetBoolProperty( DKObjectRef _self, DKStringRef name );

DK_API void        DKSetIntegerProperty( DKObjectRef _self, DKStringRef name, int64_t x );
DK_API int64_t     DKGetIntegerProperty( DKObjectRef _self, DKStringRef name );

DK_API void        DKSetFloatProperty( DKObjectRef _self, DKStringRef name, double x );
DK_API double      DKGetFloatProperty( DKObjectRef _self, DKStringRef name );

DK_API void        DKSetEnumProperty( DKObjectRef _self, DKStringRef name, int x );
DK_API int         DKGetEnumProperty( DKObjectRef _self, DKStringRef name );




// Reflection ============================================================================

// Returns the object itself (useful for callbacks).
DK_API DKObjectRef DKGetSelf( DKObjectRef _self );

// Returns false if the object's class was created with the DKImmutableInstances option.
DK_API bool        DKIsMutable( DKObjectRef _self );

// Retrieve the class, superclass and class name. These functions return the same values
// for classes and instances (i.e. DKGetClass(DKObjectClass()) == DKObjectClass()).
DK_API DKClassRef  DKGetClass( DKObjectRef _self );
DK_API DKStringRef DKGetClassName( DKObjectRef _self );
DK_API DKClassRef  DKGetSuperclass( DKObjectRef _self );

// Returns true if the object is a instance of the class.
DK_API bool        DKIsMemberOfClass( DKObjectRef _self, DKClassRef _class );

// Returns true if the object is a instance of the class or one of its subclasses.
DK_API bool        DKIsKindOfClass( DKObjectRef _self, DKClassRef _class );

// Returns true if the class is a subclass of (or equal to) another class
DK_API bool        DKIsSubclass( DKClassRef _class, DKClassRef otherClass );

// Convert between classes and strings
DK_API DKClassRef  DKClassFromString( DKStringRef className );
DK_API DKClassRef  DKClassFromCString( const char * className );
DK_API DKStringRef DKStringFromClass( DKClassRef _class );

// Convert between selectors and strings
DK_API DKSEL       DKSelectorFromString( DKStringRef name );
DK_API DKStringRef DKStringFromSelector( DKSEL sel );




// Private ===============================================================================
#if DK_RUNTIME_PRIVATE

// DKInterfaceTable
struct DKInterfaceTable
{
    struct _DKInterface *   cache[DKStaticCacheSize + DKDynamicCacheSize];

    DKSpinLock              lock;
    DKGenericHashTable      interfaces;
};

typedef DKInterfaceRef (*DKInterfaceNotFoundCallback)( DKObjectRef object, DKClassRef _class, DKSEL sel );


// DKClass
struct DKClass
{
    const DKObject          _obj;

    // The name database requires that the name field of DKClass and DKSEL is
    // in the same position in the structure (i.e. right after the object header).
    DKStringRef             name;
    
    struct DKInterfaceTable instanceInterfaces;
    struct DKInterfaceTable classInterfaces;
    
    DKInitMethod            init;
    DKFinalizeMethod        finalize;

    DKClassRef              superclass;
    uint32_t                structSize;
    uint32_t                options;

    DKSpinLock              propertiesLock;
    DKMutableHashTableRef   properties;
};


// DKMetadata
struct DKMetadata
{
    DKObject        _obj;

    // Owner of this metadata
    DKObjectRef     owner;
    
    // Weak referencing
    DKObjectRef     weakTarget;
    DKSpinLock      weakLock;
    
    // Thread Synchronization
    DKObjectRef     mutex;
};

typedef struct DKMetadata * DKMetadataRef;


void DKRuntimeInitSymbols( void );

void DKInterfaceTableInit( struct DKInterfaceTable * interfaceTable, struct DKInterfaceTable * inheritedInterfaces );
void DKInterfaceTableFinalize( struct DKInterfaceTable * interfaceTable );
void DKInterfaceTableInsert( DKClassRef _class, struct DKInterfaceTable * interfaceTable, DKInterfaceRef _interface );
DKInterface * DKInterfaceTableFind( DKObjectRef object, DKClassRef _class, struct DKInterfaceTable * interfaceTable, DKSEL sel,
    DKInterfaceNotFoundCallback interfaceNotFound );

void DKSelectorFinalize( DKObjectRef _self );
void DKInterfaceFinalize( DKObjectRef _self );

void DKNameDatabaseInit( void );
void DKNameDatabaseInsertClass( DKClassRef _class );
void DKNameDatabaseRemoveClass( DKClassRef _class );
void DKNameDatabaseInsertSelector( DKSEL sel );
void DKNameDatabaseRemoveSelector( DKSEL sel );

DKMutableHashTableRef DKCopyPropertiesTable( DKClassRef _class );

void DKMetadataTableInit( void );

DKMetadataRef DKMetadataFindOrInsert( DKObject * obj );
void DKMetadataRemove( DKMetadataRef metadata );
void DKMetadataFinalize( DKObjectRef _self );


#endif // DK_RUNTIME_PRIVATE


#ifdef __cplusplus
}
#endif

#endif // _DK_RUNTIME_H_






