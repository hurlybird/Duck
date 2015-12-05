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
DKClassRef DKRootClass( void );
DKClassRef DKClassClass( void );
DKClassRef DKSelectorClass( void );
DKClassRef DKInterfaceClass( void );
DKClassRef DKMsgHandlerClass( void );
DKClassRef DKMetadataClass( void );
DKClassRef DKObjectClass( void );



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
    DKImmutableInstances =          (1 << 3)
};

typedef DKObjectRef (*DKInitMethod)( DKObjectRef _self );
typedef void (*DKFinalizeMethod)( DKObjectRef _self );

// Create a new class object.
DKClassRef  DKNewClass( DKStringRef name, DKClassRef superclass, size_t structSize,
    uint32_t options, DKInitMethod init, DKFinalizeMethod finalize );





// Objects ===============================================================================

// These functions implement the default allocator for objects. You should never need to
// call them outside of a custom allocation scheme. Use DKAlloc and DKDealloc instead.
DKObjectRef DKAllocObject( DKClassRef cls, size_t extraBytes );
void        DKDeallocObject( DKObjectRef _self );

// Allocates a new object using the Allocation interface of its class. Use 'extraBytes' to
// allocate memory beyond the 'structSize' specified by the class. The extra memory is not
// automatically zeroed for you.
#define     DKAlloc( _class )   DKAllocEx( _class, 0 )
DKObjectRef DKAllocEx( DKClassRef _class, size_t extraBytes );

// Deallocates an object using the Allocation interface of its class. You should never
// need to call this directly unless dealing with an object that bypasses normal reference
// counting.
void        DKDealloc( DKObjectRef _self );

// Call the default initializer specified by the object's class. The object returned may
// not be the same as the object passed to it.
DKObjectRef DKInit( DKObjectRef _self );

// Call the default initializer specified by 'superclass'.
DKObjectRef DKSuperInit( DKObjectRef _self, DKClassRef superclass );

// Call the object's finalizer chain. You should never need to call this directly unless
// dealing with an object that bypasses normal reference counting.
void        DKFinalize( DKObjectRef _self );

// Wrapper for DKAlloc + DKInit
#define     DKNew( _class )  DKInit( DKAllocEx( _class, 0 ) )

// Simple object synchronization
void        DKLockObject( DKObjectRef _self );
bool        DKTryLockObject( DKObjectRef _self );
void        DKUnlockObject( DKObjectRef _self );




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


// Thread-safe initialization of shared class objects.
#define DKThreadSafeClassInit( accessor )                                               \
    DKThreadSafeSharedObjectInit( accessor, DKClassRef )




// Submodules ============================================================================
#include "DKRuntime+RefCount.h"
#include "DKRuntime+Interfaces.h"
#include "DKRuntime+Properties.h"
#include "DKRuntime+Reflection.h"
#include "DKRuntime+Metadata.h"




// Private ===============================================================================
#if DK_RUNTIME_PRIVATE

#include "DKGenericArray.h"
#include "DKHashTable.h"


// DKClass -------------------------------------------------------------------------------
struct DKClass
{
    const DKObject          _obj;

    // The name database requires that the name field of DKClass and DKSEL is
    // in the same position in the structure (i.e. right after the object header).
    DKStringRef             name;
    
    DKClassRef              superclass;
    uint32_t                structSize;
    uint32_t                options;
    
    DKInitMethod            init;
    DKFinalizeMethod        finalize;

    struct DKInterfaceTable classInterfaces;
    struct DKInterfaceTable instanceInterfaces;
    
    DKSpinLock              propertiesLock;
    DKMutableHashTableRef   properties;
};


// DKThreadContext -----------------------------------------------------------------------
struct DKThreadContext
{
    DKObjectRef threadObject;

    struct
    {
        DKIndex top;
        DKGenericArray arp[DK_AUTORELEASE_POOL_STACK_SIZE];
        
    } arpStack;
};

struct DKThreadContext * DKGetCurrentThreadContext( void );
struct DKThreadContext * DKGetMainThreadContext( void );



#endif // DK_RUNTIME_PRIVATE


#endif // _DK_RUNTIME_H_






