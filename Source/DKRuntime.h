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



// DKRuntime =============================================================================

// Initialize the library
void DKRuntimeInit( void );
bool DKRuntimeIsInitialized( void );




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
    DKStaticCache_Comparison,
    DKStaticCache_Collection,
    DKStaticCache_KeyedCollection,
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

    // Selectors are typically compared by pointer value, but the name is required to
    // look up a selector by name

    // The name database requires that the name and hash fields of DKClass and DKSEL are
    // in the same position in the structure (i.e. right after the object header).
    DKStringRef     name;
    DKHashCode      hash;
    
    // Controls how interfaces retrieved by this selector are cached.
    DKCacheUsage    cacheline;
};

typedef const struct _DKSEL * DKSEL;

#define DKSelector( name )      DKSelector_ ## name()




// Root Classes ==========================================================================
DKClassRef DKRootClass( void );
DKClassRef DKClassClass( void );
DKClassRef DKSelectorClass( void );
DKClassRef DKInterfaceClass( void );
DKClassRef DKMsgHandlerClass( void );
DKClassRef DKWeakClass( void );
DKClassRef DKObjectClass( void );



// Alloc/Free Objects ====================================================================

// These functions implement the default allocator for objects. You should never need to
// call them outside of a custom allocation scheme. Use DKAlloc and DKDealloc instead.
void *      DKAllocObject( DKClassRef cls, size_t extraBytes );
void        DKDeallocObject( DKObjectRef _self );




// Creating Classes ======================================================================
enum
{
    // Disable allocating of instances
    DKAbstractBaseClass =     (1 << 0),

    // Disable reference counting for instances
    DKDisableReferenceCounting =   (1 << 1),
    
    // This class cannot be subclassed
    DKPreventSubclassing =          (1 << 2)
};

typedef uint32_t DKClassOptions;

typedef DKObjectRef (*DKInitMethod)( DKObjectRef _self );
typedef void (*DKFinalizeMethod)( DKObjectRef _self );

// Allocate a new class object.
DKClassRef  DKAllocClass( DKStringRef name, DKClassRef superclass, size_t structSize,
    DKClassOptions options, DKInitMethod init, DKFinalizeMethod finalize );

// Allocate a new selector object.
DKSEL       DKAllocSelector( DKStringRef name );





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




// Polymorphic Wrappers ==================================================================

// Allocates a new object. Use 'extraBytes' to allocate memory beyond the 'structSize'
// specified in the class. The extra memory is not automatically zeroed for you.
void *      DKAlloc( DKClassRef _class, size_t extraBytes );

// Deallocates an object created. You should never need to call this directly unless
// dealing with an object that bypasses normal reference counting.
void        DKDealloc( DKObjectRef _self );

// Call the object's default initializer. The object returned by DKIntializeObject may
// not be the same as the object passed to it.
void *      DKInit( DKObjectRef _self );

// Call the object's superclass initializer.
void *      DKSuperInit( DKObjectRef _self, DKClassRef superclass );

// Call the object's finalizer chain. You should never need to call this directly unless
// dealing with an object that bypasses normal reference counting.
void        DKFinalize( DKObjectRef _self );

// Wrapper for DKAlloc + DKInit
#define     DKCreate( _class )  DKInit( DKAlloc( _class, 0 ) )

// Comparison Interface Wrappers
bool        DKEqual( DKObjectRef a, DKObjectRef b );
bool        DKLike( DKObjectRef a, DKObjectRef b );
int         DKCompare( DKObjectRef a, DKObjectRef b );
DKHashCode  DKHash( DKObjectRef _self );

// Copying Interface Wrappers
DKObjectRef DKCopy( DKObjectRef _self );
DKMutableObjectRef DKMutableCopy( DKObjectRef _self );

// CopyDescription Interface Wrappers
DKStringRef DKCopyDescription( DKObjectRef _self );





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




// Submodules ============================================================================
#include "DKRuntime+Interfaces.h"
#include "DKRuntime+Properties.h"
#include "DKRuntime+Reflection.h"




// Private ===============================================================================
#if DK_RUNTIME_PRIVATE

#include "DKGenericArray.h"
#include "DKHashTable.h"


// Objects are at least 16 bytes long so there must exist a location in memory
// that is 16-byte aligned and inside the object. Given that, we can generate a
// hash code from the object pointer that strips out the uninteresting lower
// bits to make things a bit more random. This is particularly important in a
// hash table that uses hash % prime to derive an internal hash code.
#define DKObjectUniqueHash( obj )   ((((uintptr_t)obj) + 15) >> 4)


struct DKInterfaceGroup
{
    DKSpinLock      lock;

    struct _DKInterface * cache[DKStaticCacheSize + DKDynamicCacheSize];
    
    // Classes usually have fewer than 10 interfaces and selectors are compared by
    // pointer value (not name). It's hard to say whether a linear search on a small
    // array is faster or slower than a hash table lookup. The search result is also
    // cached, further mitigating any performance problems.
    DKGenericArray  interfaces;
};


struct DKClass
{
    const DKObject          _obj;

    // The name database requires that the name and hash fields of DKClass and DKSEL are
    // in the same position in the structure (i.e. right after the object header).
    DKStringRef             name;
    DKHashCode              hash;
    
    DKClassRef              superclass;
    size_t                  structSize;
    DKClassOptions          options;
    
    DKInitMethod            init;
    DKFinalizeMethod        finalize;

    struct DKInterfaceGroup classInterfaces;
    struct DKInterfaceGroup instanceInterfaces;
    
    DKSpinLock              propertiesLock;
    DKMutableHashTableRef   properties;
};

#endif // DK_RUNTIME_PRIVATE


#endif // _DK_RUNTIME_H_





