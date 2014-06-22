/*****************************************************************************************

  DKRuntime.c

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

#include "DKPlatform.h"
#include "DKGenericArray.h"
#include "DKRuntime.h"
#include "DKString.h"
#include "DKHashTable.h"
#include "DKProperty.h"
#include "DKStream.h"


static void DKRuntimeInit( void );
static DKInterface * DKLookupInterface( const struct DKClass * cls, DKSEL sel );



// Internal Types ========================================================================

// Objects are at least 16 bytes long so there must exist a location in memory
// that is 16-byte aligned and inside the object. Given that, we can generate a
// hash code from the object pointer that strips out the uninteresting lower
// bits to make things a bit more random. This is particularly important in a
// hash table that uses hash % prime to derive an internal hash code.
#define ObjectUniqueHash( obj )     ((((uintptr_t)obj) + 15) >> 4)

// Get a dynamic cache index for a selector
#define GetDynamicCacheline( sel )  (int)((ObjectUniqueHash(sel) & (DKDynamicCacheSize-1)) + DKStaticCacheSize)



// DKClass
struct DKClass
{
    const DKObject  _obj;
    
    DKStringRef     name;
    DKClassRef      superclass;
    size_t          structSize;
    DKClassOptions  options;

    DKSpinLock      lock;

    DKInterface *   cache[DKStaticCacheSize + DKDynamicCacheSize];
    
    // Classes usually have fewer than 10 interfaces and selectors are compared by
    // pointer value (not name). It's hard to say whether a linear search on a small
    // array is faster or slower than a hash table lookup. The search result is also
    // cached, further mitigating any performance problems.
    DKGenericArray  interfaces;
    
    DKMutableHashTableRef properties;
};


// DKWeak
struct DKWeak
{
    DKObject        _obj;
    DKSpinLock      lock;
    DKObjectRef     target;
};




// Root Classes ==========================================================================
static struct DKClass __DKMetaClass__;
static struct DKClass __DKClassClass__;
static struct DKClass __DKSelectorClass__;
static struct DKClass __DKInterfaceClass__;
static struct DKClass __DKMsgHandlerClass__;
static struct DKClass __DKWeakClass__;
static struct DKClass __DKObjectClass__;


DKClassRef DKClassClass( void )
{
    DKRuntimeInit();
    return &__DKClassClass__;
}

DKClassRef DKSelectorClass( void )
{
    DKRuntimeInit();
    return &__DKSelectorClass__;
}

DKClassRef DKInterfaceClass( void )
{
    DKRuntimeInit();
    return &__DKInterfaceClass__;
}

DKClassRef DKMsgHandlerClass( void )
{
    DKRuntimeInit();
    return &__DKMsgHandlerClass__;
}

DKClassRef DKWeakClass( void )
{
    DKRuntimeInit();
    return &__DKWeakClass__;
}

DKClassRef DKObjectClass( void )
{
    DKRuntimeInit();
    return &__DKObjectClass__;
}




// Selectors Required for the Root Classes ===============================================
#define DKStaticSelectorInit( name )                                                    \
    static struct _DKSEL DKSelector_ ## name ##_StaticObject =                          \
    {                                                                                   \
        DKStaticObject( &__DKSelectorClass__ ),                                         \
        NULL,                                                                           \
        DKDynamicCache                                                                  \
    };                                                                                  \
                                                                                        \
    DKSEL DKSelector_ ## name( void )                                                   \
    {                                                                                   \
        return &DKSelector_ ## name ##_StaticObject;                                    \
    }


#define DKStaticFastSelectorInit( name )                                                \
    static struct _DKSEL DKSelector_ ## name ##_StaticObject =                          \
    {                                                                                   \
        DKStaticObject( &__DKSelectorClass__ ),                                         \
        NULL,                                                                           \
        DKStaticCache_ ## name                                                          \
    };                                                                                  \
                                                                                        \
    DKSEL DKSelector_ ## name( void )                                                   \
    {                                                                                   \
        return &DKSelector_ ## name ##_StaticObject;                                    \
    }


#define DKStaticInterfaceObject( sel )                                                  \
    {                                                                                   \
        DKStaticObject( &__DKInterfaceClass__ ),                                        \
        sel                                                                             \
    }


DKStaticFastSelectorInit( Allocation );
DKStaticFastSelectorInit( Comparison );
DKStaticSelectorInit( Copying );
DKStaticSelectorInit( Description );
DKStaticSelectorInit( Stream );




// Error Handling Interfaces =============================================================
#define DK_MAX_INTERFACE_SIZE   32

DKDeclareInterfaceSelector( InterfaceNotFound );
DKStaticSelectorInit( InterfaceNotFound );

DKDeclareMessageSelector( MsgHandlerNotFound );
DKStaticSelectorInit( MsgHandlerNotFound );

typedef void (*DKInterfaceNotFoundFunction)( DKObjectRef _self );

struct DKInterfaceNotFoundInterface
{
    DKInterface _interface;
    DKInterfaceNotFoundFunction func[DK_MAX_INTERFACE_SIZE];
};

static void DKInterfaceNotFoundCallback( DKObjectRef _self )
{
    // Note: '_self' is for debugging only and may not be valid. Most interface functions
    // take an object as a first arguement, but it's not actually required.

    // The only time this code is ever likely to be called is when calling an interface
    // function on a NULL object. We don't have the Objective-C luxury of tweaking the
    // call stack and return value in assembly for such cases.

    DKFatalError( "DKRuntime: Invalid interface call.\n" );
}

static intptr_t DKMsgHandlerNotFoundCallback( DKObjectRef _self, DKSEL sel )
{
    // This handles silently failing when sending messages to NULL objects.
    return 0;
}

DKThreadSafeSharedObjectInit( DKInterfaceNotFound, DKInterfaceRef )
{
    struct DKInterfaceNotFoundInterface * interface = DKAllocInterface( DKSelector(InterfaceNotFound), sizeof(struct DKInterfaceNotFoundInterface) );

    for( int i = 0; i < DK_MAX_INTERFACE_SIZE; i++ )
        interface->func[i] = DKInterfaceNotFoundCallback;
    
    return interface;
}

DKThreadSafeSharedObjectInit( DKMsgHandlerNotFound, DKMsgHandlerRef )
{
    struct DKMsgHandler * msgHandler = DKAllocInterface( DKSelector(MsgHandlerNotFound), sizeof(struct DKMsgHandler) );

    msgHandler->func = DKMsgHandlerNotFoundCallback;

    return msgHandler;
}




// Default Interfaces ====================================================================

// DefaultAllocation ---------------------------------------------------------------------
static struct DKAllocationInterface DKDefaultAllocation_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Allocation_StaticObject ),
    NULL,
    NULL,
    NULL,
    NULL
};

DKInterfaceRef DKDefaultAllocation( void )
{
    return &DKDefaultAllocation_StaticObject;
}


// DefaultComparison ---------------------------------------------------------------------
static struct DKComparisonInterface DKDefaultComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKPointerEqual,
    DKPointerCompare,
    DKPointerHash
};

DKInterfaceRef DKDefaultComparison( void )
{
    return &DKDefaultComparison_StaticObject;
}

int DKPointerEqual( DKObjectRef _self, DKObjectRef other )
{
    return _self == other;
}

int DKPointerCompare( DKObjectRef _self, DKObjectRef other )
{
    if( _self < other )
        return 1;
    
    if( _self > other )
        return -1;
    
    return 0;
}

DKHashCode DKPointerHash( DKObjectRef _self )
{
    return ObjectUniqueHash( _self );
}


// DefaultDescription --------------------------------------------------------------------
static struct DKDescriptionInterface DKDefaultDescription_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Description_StaticObject ),
    DKDefaultCopyDescription
};

DKInterfaceRef DKDefaultDescription( void )
{
    return &DKDefaultDescription_StaticObject;
}

DKStringRef DKDefaultCopyDescription( DKObjectRef _self )
{
    return DKCopy( DKGetClassName( _self ) );
}




// Root Class Interfaces =================================================================

// Class Allocation ----------------------------------------------------------------------
static void DKClassFinalize( DKObjectRef _self );

static struct DKAllocationInterface DKClassAllocation_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Allocation_StaticObject ),
    NULL,
    DKClassFinalize,
    NULL,
    NULL
};

static DKInterfaceRef DKClassAllocation( void )
{
    return &DKClassAllocation_StaticObject;
}


// Interface Allocation ------------------------------------------------------------------
static void DKInterfaceFinalize( DKObjectRef _self );

static struct DKAllocationInterface DKInterfaceAllocation_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Allocation_StaticObject ),
    NULL,
    DKInterfaceFinalize,
    NULL,
    NULL
};

static DKInterfaceRef DKInterfaceAllocation( void )
{
    return &DKInterfaceAllocation_StaticObject;
}


// Selector Allocation -------------------------------------------------------------------
static void DKSelectorFinalize( DKObjectRef _self );

static struct DKAllocationInterface DKSelectorAllocation_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Allocation_StaticObject ),
    NULL,
    DKSelectorFinalize,
    NULL,
    NULL
};

static DKInterfaceRef DKSelectorAllocation( void )
{
    return &DKSelectorAllocation_StaticObject;
}


// Selector Comparison -------------------------------------------------------------------
#define DKFastSelectorEqual( a, b )     (a == b)
#define DKSelectorEqual( a, b )         DKPointerEqual( a, b );
#define DKSelectorCompare( a, b )       DKPointerCompare( a, b );
#define DKSelectorHash( _self )         DKPointerHash( _self )

static struct DKComparisonInterface DKSelectorComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKPointerEqual,
    DKPointerCompare,
    DKPointerHash
};

static DKInterfaceRef DKSelectorComparison( void )
{
    return &DKSelectorComparison_StaticObject;
}


// Interface Comparison ------------------------------------------------------------------
static int DKInterfaceEqual( DKInterface * a, DKInterface * b )
{
    return DKSelectorEqual( a->sel, b->sel );
}

static int DKInterfaceCompare( DKInterface * a, DKInterface * b )
{
    return DKSelectorCompare( a->sel, b->sel );
}

static DKHashCode DKInterfaceHash( DKInterface * a )
{
    return DKSelectorHash( a->sel );
}

static struct DKComparisonInterface DKInterfaceComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    (DKEqualMethod)DKInterfaceEqual,
    (DKCompareMethod)DKInterfaceCompare,
    (DKHashMethod)DKInterfaceHash
};

static DKInterfaceRef DKInterfaceComparison( void )
{
    return &DKInterfaceComparison_StaticObject;
}




// Runtime Init ==========================================================================
static DKSpinLock DKRuntimeInitLock = DKSpinLockInit;
static bool DKRuntimeInitialized = false;


///
//  InitRootClass()
//
static void InitRootClass( struct DKClass * cls, struct DKClass * superclass, size_t structSize, DKClassOptions options )
{
    memset( cls, 0, sizeof(struct DKClass) );
    
    DKObject * obj = (DKObject *)cls;
    obj->isa = &__DKMetaClass__;
    obj->refcount = 1;

    cls->name = NULL;
    cls->superclass = DKRetain( superclass );
    cls->structSize = structSize;
    cls->options = options;
    cls->lock = DKSpinLockInit;
    
    DKGenericArrayInit( &cls->interfaces, sizeof(DKObjectRef) );
}


///
//  InstallRootClassInterface()
//
static void InstallRootClassInterface( struct DKClass * _class, DKInterfaceRef interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKGenericArrayAppendElements( &_class->interfaces, &interface, 1 );
}



///
//  DKRuntimeInit()
//
static void DKRuntimeInit( void )
{
    DKSpinLockLock( &DKRuntimeInitLock );
    
    if( !DKRuntimeInitialized )
    {
        DKRuntimeInitialized = true;

        InitRootClass( &__DKMetaClass__,       NULL,                  sizeof(struct DKClass), DKAbstractBaseClass | DKDisableReferenceCounting );
        InitRootClass( &__DKClassClass__,      NULL,                  sizeof(struct DKClass), 0 );
        InitRootClass( &__DKSelectorClass__,   NULL,                  sizeof(struct _DKSEL),  0 );
        InitRootClass( &__DKInterfaceClass__,  NULL,                  sizeof(DKInterface),    0 );
        InitRootClass( &__DKMsgHandlerClass__, &__DKInterfaceClass__, sizeof(DKMsgHandler),   0 );
        InitRootClass( &__DKWeakClass__,       NULL,                  sizeof(struct DKWeak),  0 );
        InitRootClass( &__DKObjectClass__,     NULL,                  sizeof(DKObject),       0 );
        
        InstallRootClassInterface( &__DKMetaClass__, DKClassAllocation() );
        InstallRootClassInterface( &__DKMetaClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKMetaClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKClassClass__, DKClassAllocation() );
        InstallRootClassInterface( &__DKClassClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKClassClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKSelectorClass__, DKDefaultAllocation() );
        InstallRootClassInterface( &__DKSelectorClass__, DKSelectorComparison() );
        InstallRootClassInterface( &__DKSelectorClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKInterfaceClass__, DKInterfaceAllocation() );
        InstallRootClassInterface( &__DKInterfaceClass__, DKInterfaceComparison() );
        InstallRootClassInterface( &__DKInterfaceClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKMsgHandlerClass__, DKDefaultAllocation() );
        InstallRootClassInterface( &__DKMsgHandlerClass__, DKInterfaceComparison() );
        InstallRootClassInterface( &__DKMsgHandlerClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKWeakClass__, DKDefaultAllocation() );
        InstallRootClassInterface( &__DKWeakClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKWeakClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKObjectClass__, DKDefaultAllocation() );
        InstallRootClassInterface( &__DKObjectClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKObjectClass__, DKDefaultDescription() );

        DKSpinLockUnlock( &DKRuntimeInitLock );
        
        // Initialize the base class names now that constant strings are available.
        __DKMetaClass__.name = DKRetain( DKSTR( "DKMetaClass" ) );
        __DKClassClass__.name = DKRetain( DKSTR( "DKClass" ) );
        __DKSelectorClass__.name = DKRetain( DKSTR( "DKSelector" ) );
        __DKInterfaceClass__.name = DKRetain( DKSTR( "DKInterface" ) );
        __DKMsgHandlerClass__.name = DKRetain( DKSTR( "DKMsgHandler" ) );
        __DKObjectClass__.name = DKRetain( DKSTR( "DKObject" ) );
        __DKWeakClass__.name = DKRetain( DKSTR( "DKWeak" ) );
        
        DKSelector_InterfaceNotFound_StaticObject.name = DKRetain( DKSTR( "InterfaceNotFound" ) );
        DKSelector_MsgHandlerNotFound_StaticObject.name = DKRetain( DKSTR( "MsgHandlerNotFound" ) );
        DKSelector_Allocation_StaticObject.name = DKRetain( DKSTR( "Allocation" ) );
        DKSelector_Comparison_StaticObject.name = DKRetain( DKSTR( "Comparison" ) );
        DKSelector_Copying_StaticObject.name = DKRetain( DKSTR( "Copying" ) );
        DKSelector_Description_StaticObject.name = DKRetain( DKSTR( "Description" ) );
        DKSelector_Stream_StaticObject.name = DKRetain( DKSTR( "Stream" ) );
        
        struct DKClass * stringClass = (struct DKClass *)DKStringClass();
        stringClass->name = DKRetain( DKSTR( "DKString" ) );

        struct DKClass * constantStringClass = (struct DKClass *)DKConstantStringClass();
        constantStringClass->name = DKRetain( DKSTR( "DKConstantString" ) );
    }
    
    else
    {
        DKSpinLockUnlock( &DKRuntimeInitLock );
    }
}




// Alloc/Free Objects ====================================================================

///
//  DKAllocObject()
//
void * DKAllocObject( DKClassRef cls, size_t extraBytes )
{
    if( !cls )
    {
        DKError( "DKAllocObject: Specified class object is NULL.\n" );
        return NULL;
    }
    
    if( cls->structSize < sizeof(DKObject) )
    {
        DKFatalError( "DKAllocObject: Requested struct size is smaller than DKObject.\n" );
        return NULL;
    }
    
    if( (cls->options & DKAbstractBaseClass) != 0 )
    {
        DKFatalError( "DKAllocObject: Class '%s' is an abstract base class.\n", DKStringGetCStringPtr( cls->name ) );
        return NULL;
    }
    
    // Allocate the structure + extra bytes
    DKAllocationInterfaceRef allocation = DKGetInterface( cls, DKSelector(Allocation) );
 
    DKObject * obj = NULL;
 
    if( allocation->alloc )
        obj = allocation->alloc( cls->structSize + extraBytes );
    
    else
        obj = dk_malloc( cls->structSize + extraBytes );
    
    // Zero the structure bytes
    memset( obj, 0, cls->structSize );
    
    // Setup the object header
    obj->isa = DKRetain( cls );
    obj->weakref = NULL;
    obj->refcount = 1;
    
    return obj;
}


///
//  DKDeallocObject()
//
void DKDeallocObject( DKObjectRef _self )
{
    DKObject * obj = (DKObject *)_self;
    
    DKAssert( obj );
    DKAssert( obj->refcount == 0 );
    DKAssert( obj->weakref == NULL );

    DKClassRef cls = obj->isa;

    // Deallocate
    DKAllocationInterfaceRef allocation = DKGetInterface( cls, DKSelector(Allocation) );
    
    if( allocation->free )
        allocation->free( obj );
    
    else
        dk_free( obj );
    
    // Finally release the class object
    DKRelease( cls );
}


///
//  DKInitializeObject()
//
static DKObjectRef DKInitializeObjectRecursive( DKObjectRef _self, DKClassRef cls )
{
    if( cls )
    {
        _self = DKInitializeObjectRecursive( _self, cls->superclass );

        if( _self )
        {
            DKAllocationInterfaceRef allocation = DKGetInterface( cls, DKSelector(Allocation) );
            
            if( allocation->initialize )
                _self = allocation->initialize( _self );
        }
    }
    
    return _self;
}

void * DKInitializeObject( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        
        return (void *)DKInitializeObjectRecursive( obj, obj->isa );
    }
    
    return NULL;
}


///
//  DKFinalizeObject()
//
void DKFinalizeObject( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;

        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            DKAllocationInterfaceRef allocation = DKGetInterface( cls, DKSelector(Allocation) );
            
            if( allocation->finalize )
                allocation->finalize( obj );
        }
    }
}




// Creating Classes ======================================================================

///
//  DKAllocClass()
//
DKClassRef DKAllocClass( DKStringRef name, DKClassRef superclass, size_t structSize, DKClassOptions options )
{
    if( superclass && ((superclass->options & DKPreventSubclassing) != 0) )
    {
        DKFatalError( "DKAllocClass: Class '%s' does not allow subclasses.\n", DKStringGetCStringPtr( superclass->name ) );
        return NULL;
    }

    struct DKClass * cls = DKCreate( DKClassClass() );

    cls->name = DKCopy( name );
    cls->superclass = DKRetain( superclass );
    cls->structSize = structSize;
    cls->options = options;

    memset( cls->cache, 0, sizeof(cls->cache) );
    
    DKGenericArrayInit( &cls->interfaces, sizeof(DKObjectRef) );
    
    // To ensure that all classes have an initializer/finalizer, install a default
    // allocation interface here. If we don't do this the base class versions can be
    // called multiple times.
    DKInstallInterface( cls, DKDefaultAllocation() );
    
    return cls;
}


///
//  DKClassFinalize()
//
static void DKClassFinalize( DKObjectRef _self )
{
    struct DKClass * cls = (struct DKClass *)_self;
    
    DKAssert( cls->_obj.isa == &__DKClassClass__ );
    
    DKRelease( cls->name );
    DKRelease( cls->superclass );

    // Release interfaces
    DKIndex count = cls->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * interface = DKGenericArrayGetElementAtIndex( &cls->interfaces, i, DKInterface * );
        DKRelease( interface );
    }
    
    DKGenericArrayFinalize( &cls->interfaces );
    
    // Release properties
    DKRelease( cls->properties );
}


///
//  DKAllocSelector()
//
DKSEL DKAllocSelector( DKStringRef name )
{
    struct _DKSEL * sel = DKAllocObject( DKSelectorClass(), 0 );
    sel = DKInitializeObject( sel );

    DKAssert( sel != NULL );

    sel->name = DKCopy( name );
    sel->cacheline = DKDynamicCache;

    return sel;
}


///
//  DKSelectorFinalize()
//
static void DKSelectorFinalize( DKObjectRef _self )
{
    DKSEL sel = _self;
    DKRelease( sel->name );
}


///
//  DKAllocInterface()
//
void * DKAllocInterface( DKSEL sel, size_t structSize )
{
    if( sel )
    {
        size_t extraBytes = structSize - sizeof(DKInterface);
        
        // DKGetInterface returns a generic interface filled with DK_MAX_INTERFACE_SIZE
        // pointers to DKInterfaceNotFound. The size needs to be large enough to deal
        // with the largest interface or it'll cause an access error at run time.
        DKAssert( (extraBytes / sizeof(void *)) <= DK_MAX_INTERFACE_SIZE );
        
        DKInterface * interface = DKAllocObject( DKInterfaceClass(), extraBytes );
        interface = DKInitializeObject( interface );

        DKAssert( interface != NULL );

        interface->sel = DKRetain( sel );
    
        // Init all the function pointers to NULL
        memset( interface + 1, 0, extraBytes );
    
        return interface;
    }
    
    return NULL;
}


///
//  DKInterfaceFinalize()
//
static void DKInterfaceFinalize( DKObjectRef _self )
{
    DKInterface * interface = (DKInterface *)_self;
    DKRelease( interface->sel );
}


///
//  DKInstallInterface()
//
void DKInstallInterface( DKClassRef _class, DKInterfaceRef _interface )
{
    DKAssertMemberOfClass( _class, DKClassClass() );
    DKAssertKindOfClass( _interface, DKInterfaceClass() );

    struct DKClass * cls = (struct DKClass *)_class;
    DKInterface * interface = (DKInterface *)_interface;

    // Retain the new interface
    DKRetain( interface );

    // Resolve the cache line from the selector
    int cacheline = interface->sel->cacheline;
    DKAssert( (cacheline >= 0) && (cacheline < DKStaticCacheSize) );

    if( cacheline == DKDynamicCache )
        cacheline = GetDynamicCacheline( interface->sel );
    
    DKAssert( (cacheline > 0) && (cacheline < (DKStaticCacheSize + DKDynamicCacheSize)) );
    
    // Lock while we make changes
    DKSpinLockLock( &cls->lock );
    
    // Invalidate the cache
    
    // *** WARNING ***
    // This doesn't invalidate the caches of any subclasses, so it's possible that
    // subclasses will still reference the old interface, or even crash if the old
    // interface is released (the cache doesn't maintain a reference).
    // *** WARNING ***
    
    cls->cache[cacheline] = NULL;

    // Replace the interface in the interface table
    DKIndex count = cls->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * oldInterface = DKGenericArrayGetElementAtIndex( &cls->interfaces, i, DKInterface * );
        
        if( DKEqual( oldInterface->sel, interface->sel ) )
        {
            DKGenericArrayGetElementAtIndex( &cls->interfaces, i, DKInterface * ) = interface;

            DKSpinLockUnlock( &cls->lock );

            // Release the old interface after unlocking
            DKRelease( oldInterface );
            return;
        }
    }
    
    // Add the interface to the interface table
    DKGenericArrayAppendElements( &cls->interfaces, &interface, 1 );

    DKSpinLockUnlock( &cls->lock );
}


///
//  DKInstallMsgHandler()
//
void DKInstallMsgHandler( DKClassRef _class, DKSEL sel, DKMsgFunction func )
{
    struct DKMsgHandler * msgHandler = DKAllocObject( DKMsgHandlerClass(), sizeof(void *) );
    msgHandler = DKInitializeObject( msgHandler );

    DKAssert( msgHandler != NULL );

    msgHandler->sel = DKRetain( sel );
    msgHandler->func = func;
    
    DKInstallInterface( _class, msgHandler );
    
    DKRelease( msgHandler );
}


///
//  DKInstallProperty()
//
void DKInstallProperty( DKClassRef _class, DKStringRef name, DKPropertyRef property )
{
    DKAssert( _class && property && name );
    
    struct DKClass * cls = (struct DKClass *)_class;
    
    DKSpinLockLock( &cls->lock );
    
    if( cls->properties == NULL )
        cls->properties = DKHashTableCreateMutable();
    
    DKHashTableInsertObject( cls->properties, name, property, DKInsertAlways );
    
    DKSpinLockUnlock( &cls->lock );
}




// Retrieving Interfaces, Message Handlers and Properties ================================

///
//  DKLookupInterface()
//
static DKInterface * DKLookupInterface( DKClassRef _class, DKSEL sel )
{
    DKAssert( (_class->_obj.isa == &__DKClassClass__) || (_class->_obj.isa == &__DKMetaClass__) );
    DKAssert( sel->_obj.isa == &__DKSelectorClass__ );

    struct DKClass * cls = (struct DKClass *)_class;

    // Get the static cache line from the selector
    int cacheline = sel->cacheline;
    DKAssert( (cacheline >= 0) && (cacheline < DKStaticCacheSize) );

    // Lock while we lookup the interface
    DKSpinLockLock( &cls->lock );
    
    // First check the static cache (line 0 will always be NULL)
    DKInterface * interface = cls->cache[cacheline];
    
    if( interface )
    {
        DKSpinLockUnlock( &cls->lock );
        return interface;
    }
    
    // Next check the dynamic cache
    if( cacheline == DKDynamicCache )
    {
        cacheline = GetDynamicCacheline( sel );
        DKAssert( (cacheline > 0) && (cacheline < (DKStaticCacheSize + DKDynamicCacheSize)) );
        
        interface = cls->cache[cacheline];
        
        if( interface && DKFastSelectorEqual( interface->sel, sel ) )
        {
            DKSpinLockUnlock( &cls->lock );
            return interface;
        }
    }

    // Search our interface table
    DKIndex count = cls->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * interface = DKGenericArrayGetElementAtIndex( &cls->interfaces, i, DKInterface * );
        DKAssert( interface != NULL );
        
        if( DKFastSelectorEqual( interface->sel, sel ) )
        {
            // Update the cache
            cls->cache[cacheline] = interface;

            DKSpinLockUnlock( &cls->lock );
            return interface;
        }
    }

    // Lookup the interface in our superclasses
    DKSpinLockUnlock( &cls->lock );
    
    if( cls->superclass )
    {
        interface = DKLookupInterface( cls->superclass, sel );

        // Update the cache
        if( interface )
        {
            DKSpinLockLock( &cls->lock );
            cls->cache[cacheline] = interface;
            DKSpinLockUnlock( &cls->lock );
        }
    }

    return interface;
}


///
//  DKGetInterface()
//
DKInterfaceRef DKGetInterface( DKObjectRef _self, DKSEL sel )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;
        
        DKInterfaceRef interface = DKLookupInterface( cls, sel );
        
        if( interface )
            return interface;

        DKFatalError( "DKRuntime: Interface '%s' not found on object '%s'\n", sel->name, cls->name );
    }

    return DKInterfaceNotFound();
}


///
//  DKQueryInterface()
//
int DKQueryInterface( DKObjectRef _self, DKSEL sel, DKInterfaceRef * _interface )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = _self;
        
        DKInterfaceRef interface = DKLookupInterface( cls, sel );

        if( interface )
        {
            if( _interface )
                *_interface = interface;
            
            return 1;
        }
    }
    
    return 0;
}


///
//  DKGetMsgHandler()
//
DKMsgHandlerRef DKGetMsgHandler( DKObjectRef _self, DKSEL sel )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;

        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKLookupInterface( cls, sel );

        if( msgHandler )
        {
            DKCheckKindOfClass( msgHandler, DKMsgHandlerClass(), DKMsgHandlerNotFound() );
            return msgHandler;
        }

        DKWarning( "DKRuntime: Message handler for '%s' not found on object '%s'\n", sel->name, obj->isa->name );
    }

    return DKMsgHandlerNotFound();
}


///
//  DKQueryMsgHandler()
//
int DKQueryMsgHandler( DKObjectRef _self, DKSEL sel, DKMsgHandlerRef * _msgHandler )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;

        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKLookupInterface( cls, sel );

        if( msgHandler )
        {
            DKCheckKindOfClass( msgHandler, DKMsgHandlerClass(), 0 );

            if( _msgHandler )
                *_msgHandler = msgHandler;
            
            return 1;
        }
    }

    return 0;
}


///
//  DKLookupProperty()
//
static DKPropertyRef DKLookupProperty( DKClassRef _class, DKStringRef name )
{
    DKAssert( (_class->_obj.isa == &__DKClassClass__) || (_class->_obj.isa == &__DKMetaClass__) );
    
    struct DKClass * cls = (struct DKClass *)_class;
    
    DKSpinLockLock( &cls->lock );

    DKPropertyRef property = DKHashTableGetObject( cls->properties, name );
    
    DKSpinLockUnlock( &cls->lock );
    
    return property;
}


///
//  DKGetProperty()
//
DKPropertyRef DKGetPropertyDefinition( DKObjectRef _self, DKStringRef name )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;

        return DKLookupProperty( cls, name );
    }
    
    return NULL;
}




// Reference Counting ====================================================================

DKObjectRef DKRetain( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = (DKObject *)_self;

        if( (obj->isa->options & DKDisableReferenceCounting) == 0 )
        {
            DKAtomicIncrement32( &obj->refcount );
        }
    }

    return _self;
}

void DKRelease( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = (DKObject *)_self;

        if( (obj->isa->options & DKDisableReferenceCounting) == 0 )
        {
            struct DKWeak * weakref = (struct DKWeak *)obj->weakref;
            
            int32_t n;
            
            if( weakref == NULL )
            {
                n = DKAtomicDecrement32( &obj->refcount );
                DKAssert( n >= 0 );
            }
            
            else
            {
                DKSpinLockLock( &weakref->lock );
                
                n = DKAtomicDecrement32( &obj->refcount );
                DKAssert( n >= 0 );

                if( n == 0 )
                {
                    obj->weakref = NULL;
                    weakref->target = NULL;
                }
                
                DKSpinLockUnlock( &weakref->lock );
            }
            
            if( n == 0 )
            {
                DKRelease( weakref );
                DKFinalizeObject( _self );
                DKDeallocObject( _self );
            }
        }
    }
}


///
//  DKRetainWeak()
//
DKWeakRef DKRetainWeak( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        
        // It doesn't make sense to get a weak reference to a weak reference.
        if( obj->isa == DKWeakClass() )
        {
            return DKRetain( obj );
        }
        
        if( !obj->weakref )
        {
            struct DKWeak * weakref = DKCreate( DKWeakClass() );
            
            weakref->lock = DKSpinLockInit;
            weakref->target = obj;
            
            if( !DKAtomicCmpAndSwapPtr( (void * volatile *)&obj->weakref, NULL, weakref ) )
                DKRelease( weakref );
        }
        
        return DKRetain( obj->weakref );
    }
    
    return NULL;
}


///
//  DKResolveWeak()
//
DKObjectRef DKResolveWeak( DKWeakRef weak_ref )
{
    if( weak_ref )
    {
        struct DKWeak * weakref = (struct DKWeak *)weak_ref;
        
        if( weakref->target )
        {
            DKSpinLockLock( &weakref->lock );
            
            DKObjectRef target = DKRetain( weakref->target );
            
            DKSpinLockUnlock( &weakref->lock );
            
            return target;
        }
    }
    
    return NULL;
}




// Reflection ============================================================================

///
//  DKGetClass()
//
DKClassRef DKGetClass( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        return obj->isa;
    }
    
    return NULL;
}


///
//  DKGetClassName()
//
DKStringRef DKGetClassName( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;
        
        return cls->name;
    }
    
    return DKSTR( "null" );
}


///
//  DKGetSuperclass()
//
DKClassRef DKGetSuperclass( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        return obj->isa->superclass;
    }
    
    return NULL;
}


///
//  DKIsMemberOfClass()
//
int DKIsMemberOfClass( DKObjectRef _self, DKClassRef _class )
{
    if( _self )
    {
        const DKObject * obj = _self;
        return obj->isa == _class;
    }
    
    return 0;
}


///
//  DKIsKindOfClass()
//
int DKIsKindOfClass( DKObjectRef _self, DKClassRef _class )
{
    if( _self )
    {
        const DKObject * obj = _self;
        
        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls == _class )
                return 1;
        }
    }
    
    return 0;
}


///
//  DKIsSubclass()
//
int DKIsSubclass( DKClassRef _class, DKClassRef otherClass )
{
    if( _class )
    {
        for( DKClassRef cls = _class; cls != NULL; cls = cls->superclass )
        {
            if( cls == otherClass )
                return 1;
        }
    }
    
    return 0;
}




// Polymorphic Wrappers ==================================================================

///
//  DKCreate()
//
void * DKCreate( DKClassRef _class )
{
    DKObjectRef obj = NULL;

    if( _class )
    {
        obj = DKAllocObject( _class, 0 );
        obj = DKInitializeObject( obj );
    }
    
    return (void *)obj;
}


///
//  DKEqual()
//
int DKEqual( DKObjectRef a, DKObjectRef b )
{
    if( a == b )
    {
        return 1;
    }

    if( a && b )
    {
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->equal( a, b );
    }
    
    return 0;
}


///
//  DKCompare()
//
int DKCompare( DKObjectRef a, DKObjectRef b )
{
    if( a == b )
    {
        return 0;
    }

    if( a && b )
    {
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->compare( a, b );
    }
    
    return a < b ? -1 : 1;
}


///
//  DKHash()
//
DKHashCode DKHash( DKObjectRef _self )
{
    if( _self )
    {
        DKComparisonInterfaceRef comparison = DKGetInterface( _self, DKSelector(Comparison) );
        return comparison->hash( _self );
    }
    
    return 0;
}


///
//  DKCopy()
//
DKObjectRef DKCopy( DKObjectRef _self )
{
    if( _self )
    {
        DKCopyingInterfaceRef copying = DKGetInterface( _self, DKSelector(Copying) );
        return copying->copy( _self );
    }

    return _self;
}


///
//  DKMutableCopy()
//
DKMutableObjectRef DKMutableCopy( DKObjectRef _self )
{
    if( _self )
    {
        DKCopyingInterfaceRef copying = DKGetInterface( _self, DKSelector(Copying) );
        return copying->mutableCopy( _self );
    }

    return NULL;
}


///
//  DKCopyDescription()
//
DKStringRef DKCopyDescription( DKObjectRef _self )
{
    if( _self )
    {
        DKDescriptionInterfaceRef description = DKGetInterface( _self, DKSelector(Description) );
        return description->copyDescription( _self );
    }
    
    return DKSTR( "null" );
}







