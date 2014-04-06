/*******************************************************************************

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

*******************************************************************************/

#include "DKPlatform.h"
#include "DKPointerArray.h"
#include "DKRuntime.h"
#include "DKString.h"
#include "DKHashTable.h"


static void DKRuntimeInit( void );
static DKInterface * DKLookupInterface( const struct DKClass * cls, DKSEL sel );


// Internal Types ========================================================================

// DKClass
#define MAX_CLASS_NAME_LENGTH   40

struct DKClass
{
    DKObject        _obj;
    
    DKStringRef     name;
    DKClassRef      superclass;
    size_t          structSize;
    DKClassOptions  options;

    DKSpinLock      lock;

    DKInterface *   cache[DKStaticCacheSize + DKDynamicCacheSize];
    
    DKPointerArray  interfaces;
    DKPointerArray  properties;
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
static struct DKClass __DKPropertyClass__;
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

DKClassRef DKPropertyClass( void )
{
    DKRuntimeInit();
    return &__DKPropertyClass__;
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




// Default Interfaces ====================================================================

#define DKStaticSelectorInit( name )                                                    \
    static struct DKSEL DKSelector_ ## name ##_StaticObject =                           \
    {                                                                                   \
        DKStaticObject( &__DKSelectorClass__ ),                                         \
        #name,                                                                          \
        DKDynamicCache                                                                  \
    };                                                                                  \
                                                                                        \
    DKSEL DKSelector_ ## name( void )                                                   \
    {                                                                                   \
        return &DKSelector_ ## name ##_StaticObject;                                    \
    }


#define DKStaticFastSelectorInit( name )                                                \
    static struct DKSEL DKSelector_ ## name ##_StaticObject =                           \
    {                                                                                   \
        DKStaticObject( &__DKSelectorClass__ ),                                         \
        #name,                                                                          \
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


// Not Found -----------------------------------------------------------------------------
#define DK_MAX_INTERFACE_SIZE   32

DKDeclareInterfaceSelector( InterfaceNotFound );
DKStaticSelectorInit( InterfaceNotFound );

DKDeclareMessageSelector( MsgHandlerNotFound );
DKStaticSelectorInit( MsgHandlerNotFound );

typedef void (*DKInterfaceNotFoundFunction)( struct DKObject * obj );

struct DKInterfaceNotFoundInterface
{
    DKInterface _interface;
    DKInterfaceNotFoundFunction func[DK_MAX_INTERFACE_SIZE];
};

static void DKInterfaceNotFoundCallback( struct DKObject * obj )
{
    // Note: 'obj' is for debugging only and may not be valid. Most interface functions
    // take an object as a first arguement, but it's not actually required.

    // The only time this code is ever likely to be called is when calling an interface
    // function on a NULL object. We don't have the Objective-C luxury of tweaking the
    // call stack and return value in assembly for such cases.

    DKFatalError( "DKRuntime: Invalid interface call.\n" );
}

static void DKMsgHandlerNotFoundCallback( struct DKObject * obj, DKSEL sel )
{
    // This handles silently failing when sending messages to NULL objects.
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


// LifeCycle -----------------------------------------------------------------------------
DKStaticFastSelectorInit( Allocation );

static struct DKAllocation DKDefaultAllocation_StaticObject =
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


// Comparison ----------------------------------------------------------------------------
DKStaticFastSelectorInit( Comparison );

static struct DKComparison DKDefaultComparison_StaticObject =
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
    // Just in case someone changes the size of DKHashCode
    DKAssert( sizeof(DKHashCode) == sizeof(DKObjectRef) );

    // Assuming object pointers are at least N-byte aligned, this will make hash codes
    // derived from pointers a bit more random. This is particularly important in a hash
    // table which uses (hashcode % prime) as an internal hash code.
#if __LP64__
    DKAssert( ((DKHashCode)_self & 0x7) == 0 );
    return ((DKHashCode)_self) >> 3;
#else
    DKAssert( ((DKHashCode)_self & 0x3) == 0 );
    return ((DKHashCode)_self) >> 2;
#endif
}


// Description ---------------------------------------------------------------------------
DKStaticSelectorInit( Description );

static struct DKDescription DKDefaultDescription_StaticObject =
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
    return DKRetain( DKGetClassName( _self ) );
}




// Meta-Class Interfaces =================================================================

// Class LifeCycle -----------------------------------------------------------------------
static void DKClassFinalize( DKObjectRef _self );

static struct DKAllocation DKClassAllocation_StaticObject =
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


// Interface LifeCycle -------------------------------------------------------------------
static void DKInterfaceFinalize( DKObjectRef _self );

static struct DKAllocation DKInterfaceAllocation_StaticObject =
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


// Selector Comparison -------------------------------------------------------------------
static int          DKSelectorEqual( DKSEL a, DKSEL b );
static int          DKSelectorCompare( DKSEL a, DKSEL b );
#define             DKSelectorHash( _self )         DKPointerHash( _self )
#define             DKFastSelectorEqual( a, b )     (a == b)

static struct DKComparison DKSelectorComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    (DKEqualMethod)DKSelectorEqual,
    (DKCompareMethod)DKSelectorCompare,
    DKPointerHash
};

static DKInterfaceRef DKSelectorComparison( void )
{
    return &DKSelectorComparison_StaticObject;
}

static int DKSelectorEqual( DKSEL a, DKSEL b )
{
    #if DK_RUNTIME_INTEGRITY_CHECKS
    // Do stuff here
    #endif

    return a == b;
}

static int DKSelectorCompare( DKSEL a, DKSEL b )
{
    #if DK_RUNTIME_INTEGRITY_CHECKS
    // Do stuff here
    #endif

    if( a < b )
        return 1;
    
    if( a > b )
        return -1;
    
    return 0;
}


// Interface Comparison ------------------------------------------------------------------
static int          DKInterfaceEqual( DKInterface * a, DKInterface * b );
static int          DKInterfaceCompare( DKInterface * a, DKInterface * b );
static DKHashCode   DKInterfaceHash( DKInterface * a );

static struct DKComparison DKInterfaceComparison_StaticObject =
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




// Runtime Init ==========================================================================

static DKSpinLock DKRuntimeInitLock = DKSpinLockInit;
static int32_t DKRuntimeInitialized = 0;

///
//  InitRootClass()
//
static void InitRootClass( struct DKClass * cls, struct DKClass * superclass, size_t structSize, DKClassOptions options )
{
    memset( cls, 0, sizeof(struct DKClass) );
    
    struct DKObject * obj = (struct DKObject *)cls;
    obj->isa = &__DKMetaClass__;
    obj->refcount = 1;

    cls->name = NULL;
    cls->superclass = DKRetain( superclass );
    cls->structSize = structSize;
    cls->options = options;
    cls->lock = DKSpinLockInit;
    
    DKPointerArrayInit( &cls->interfaces );
    DKPointerArrayInit( &cls->properties );
}


///
//  InstallRootClassInterface()
//
static void InstallRootClassInterface( struct DKClass * _class, DKInterface * interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKPointerArrayAppendPointer( &_class->interfaces, interface );
}



///
//  DKRuntimeInit()
//
static void DKRuntimeInit( void )
{
    DKSpinLockLock( &DKRuntimeInitLock );
    
    if( !DKRuntimeInitialized )
    {
        DKRuntimeInitialized = 1;

        InitRootClass( &__DKMetaClass__,       NULL,                  sizeof(struct DKClass), DKInstancesNeverAllocated | DKInstancesNeverDeallocated );
        InitRootClass( &__DKClassClass__,      NULL,                  sizeof(struct DKClass), 0 );
        InitRootClass( &__DKSelectorClass__,   NULL,                  sizeof(struct DKSEL),   0 );
        InitRootClass( &__DKInterfaceClass__,  NULL,                  sizeof(DKInterface),    0 );
        InitRootClass( &__DKMsgHandlerClass__, &__DKInterfaceClass__, sizeof(DKMsgHandler),   0 );
        InitRootClass( &__DKPropertyClass__,   NULL,                  0,                      0 );
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

        InstallRootClassInterface( &__DKPropertyClass__, DKDefaultAllocation() );
        InstallRootClassInterface( &__DKPropertyClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKPropertyClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKWeakClass__, DKDefaultAllocation() );
        InstallRootClassInterface( &__DKWeakClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKWeakClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKObjectClass__, DKDefaultAllocation() );
        InstallRootClassInterface( &__DKObjectClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKObjectClass__, DKDefaultDescription() );

        DKSpinLockUnlock( &DKRuntimeInitLock );

        // Both DKString and DKConstantString must be initialized to set the class names,
        // so do this after unlocking the spin lock. Because the names are all constant
        // strings, the worst that can happen is they get set multiple times.
        __DKMetaClass__.name = DKSTR( "DKMetaClass" );
        __DKClassClass__.name = DKSTR( "DKClass" );
        __DKSelectorClass__.name = DKSTR( "DKSelector" );
        __DKInterfaceClass__.name = DKSTR( "DKInterface" );
        __DKMsgHandlerClass__.name = DKSTR( "DKMsgHandler" );
        __DKPropertyClass__.name = DKSTR( "DKProperty" );
        __DKObjectClass__.name = DKSTR( "DKObject" );
        __DKWeakClass__.name = DKSTR( "DKWeak" );
        
        // Since DKString, DKConstantString, DKHashTable and DKMutableHashTable are all
        // involved in creating constant strings, the names for these classes are
        // initialized in DKRuntimeInit().
        struct DKClass * stringClass = (struct DKClass *)DKStringClass();
        stringClass->name = DKSTR( "DKString" );
        
        struct DKClass * constantStringClass = (struct DKClass *)DKConstantStringClass();
        constantStringClass->name = DKSTR( "DKConstantString" );

        struct DKClass * hashTableClass = (struct DKClass *)DKHashTableClass();
        hashTableClass->name = DKSTR( "DKHashTable" );

        struct DKClass * mutableHashTableClass = (struct DKClass *)DKMutableHashTableClass();
        mutableHashTableClass->name = DKSTR( "DKMutableHashTable" );
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
    
    if( cls->structSize < sizeof(struct DKObject) )
    {
        DKFatalError( "DKAllocObject: Requested struct size is smaller than DKObject.\n" );
        return NULL;
    }
    
    if( (cls->options & DKInstancesNeverAllocated) != 0 )
    {
        DKFatalError( "DKAllocObject: Class '%s' does not allow allocation of instances.\n", DKStringGetCStringPtr( cls->name ) );
        return NULL;
    }
    
    // Allocate the structure + extra bytes
    DKAllocation * allocation = DKGetInterface( cls, DKSelector(Allocation) );

    struct DKObject * obj = NULL;

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
    
    // Call the initializer chain
    obj = (struct DKObject *)DKInitializeObject( obj );
    
    return obj;
}


///
//  DKDeallocObject()
//
void DKDeallocObject( DKObjectRef _self )
{
    struct DKObject * obj = (struct DKObject *)_self;
    
    DKAssert( obj );
    DKAssert( obj->refcount == 0 );
    DKAssert( obj->weakref == NULL );

    DKClassRef cls = obj->isa;

    // Call the finalizer chain
    DKFinalizeObject( obj );
    
    // Deallocate
    DKAllocation * allocation = DKGetInterface( cls, DKSelector(Allocation) );
    
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
static DKObject * DKInitializeObjectRecursive( DKObject * obj, DKClassRef cls )
{
    if( cls )
    {
        obj = DKInitializeObjectRecursive( obj, cls->superclass );

        if( obj )
        {
            DKAllocation * allocation = DKGetInterface( cls, DKSelector(Allocation) );
            
            if( allocation->initialize )
                obj = (struct DKObject *)allocation->initialize( obj );
        }
    }
    
    return obj;
}

DKObjectRef DKInitializeObject( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;
        
        return DKInitializeObjectRecursive( obj, obj->isa );
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
        DKObject * obj = _self;

        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            DKAllocation * allocation = DKGetInterface( cls, DKSelector(Allocation) );
            
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

    struct DKClass * cls = (struct DKClass *)DKAllocObject( DKClassClass(), 0 );

    cls->name = DKRetain( name );
    cls->superclass = DKRetain( superclass );
    cls->structSize = structSize;
    cls->options = options;
    
    DKPointerArrayInit( &cls->interfaces );
    DKPointerArrayInit( &cls->properties );

    memset( cls->cache, 0, sizeof(cls->cache) );
    
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
        const DKInterface * interface = cls->interfaces.data[i];
        DKRelease( interface );
    }
    
    DKPointerArrayFinalize( &cls->interfaces );
    
    // Release properties
    DKPointerArrayFinalize( &cls->properties );
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
        
        struct DKInterface * interface = (struct DKInterface *)DKAllocObject( DKInterfaceClass(), extraBytes );

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
    DKInterface * interface = _self;
    DKRelease( interface->sel );
}


///
//  GetDynamicCacheline()
//
static int GetDynamicCacheline( DKSEL sel )
{
    int cacheline;
    
#if __LP64__
    DKAssert( ((uintptr_t)sel & 0x7) == 0 );
    cacheline = (int)((((uintptr_t)sel >> 3) & (DKDynamicCacheSize - 1)) + DKStaticCacheSize);
#else
    DKAssert( ((uintptr_t)sel & 0x3) == 0 );
    cacheline = (int)((((uintptr_t)sel >> 2) & (DKDynamicCacheSize - 1)) + DKStaticCacheSize);
#endif
    
    return cacheline;
}


///
//  DKInstallInterface()
//

void DKInstallInterface( DKClassRef _class, DKInterfaceRef _interface )
{
    DKAssertMemberOfClass( _class, DKClassClass() );
    DKAssertKindOfClass( _interface, DKInterfaceClass() );

    struct DKClass * cls = (struct DKClass *)_class;
    DKInterface * interface = _interface;

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
        DKInterface * oldInterface = cls->interfaces.data[i];
        
        if( DKEqual( oldInterface->sel, interface->sel ) )
        {
            cls->interfaces.data[i] = interface;

            DKSpinLockUnlock( &cls->lock );

            // Release the old interface after unlocking
            DKRelease( oldInterface );
            return;
        }
    }
    
    // Add the interface to the interface table
    DKPointerArrayAppendPointer( &cls->interfaces, interface );

    DKSpinLockUnlock( &cls->lock );
}


///
//  DKInstallMsgHandler()
//
void DKInstallMsgHandler( DKClassRef _class, DKSEL sel, const void * func )
{
    struct DKMsgHandler * msgHandler = (struct DKMsgHandler *)DKAllocObject( DKMsgHandlerClass(), sizeof(struct DKMsgHandler) );

    msgHandler->sel = DKRetain( sel );
    msgHandler->func = func;
    
    DKInstallInterface( _class, msgHandler );
    
    DKRelease( msgHandler );
}




// Retrieving Interfaces and Message Handlers ============================================

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
        DKInterface * interface = cls->interfaces.data[i];
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
        DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)_self;
        
        DKInterfaceRef interface = DKLookupInterface( cls, sel );
        
        if( interface )
            return interface;

        DKFatalError( "DKRuntime: Interface '%s' not found on object '%s'\n", sel->suid, cls->name );
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
        DKObject * obj = _self;
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
        DKObject * obj = _self;
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

        DKWarning( "DKRuntime: Message handler for '%s' not found on object '%s'\n", sel->suid, obj->isa->name );
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
        DKObject * obj = _self;
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





// Reference Counting ====================================================================

DKObjectRef DKRetain( DKObjectRef _self )
{
    if( _self )
    {
        struct DKObject * obj = (struct DKObject *)_self;

        if( (obj->isa->options & DKInstancesNeverDeallocated) == 0 )
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
        struct DKObject * obj = (struct DKObject *)_self;

        if( (obj->isa->options & DKInstancesNeverDeallocated) == 0 )
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
        struct DKObject * obj = (struct DKObject *)_self;
        
        if( !obj->weakref )
        {
            struct DKWeak * weakref = (struct DKWeak *)DKAllocObject( DKWeakClass(), 0 );
            
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
        DKObject * obj = _self;
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
        DKObject * obj = _self;
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
        DKObject * obj = _self;
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
        DKObject * obj = _self;
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
        DKObject * obj = _self;
        
        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls == _class )
                return 1;
        }
    }
    
    return 0;
}




// Polymorphic Wrappers ==================================================================

///
//  DKCreate()
//
DKObjectRef DKCreate( DKClassRef _class )
{
    if( _class )
    {
        return DKAllocObject( _class, 0 );
    }
    
    return NULL;
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
        DKComparison * comparison = DKGetInterface( a, DKSelector(Comparison) );
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

    if( a )
    {
        DKComparison * comparison = DKGetInterface( a, DKSelector(Comparison) );
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
        DKComparison * comparison = DKGetInterface( _self, DKSelector(Comparison) );
        return comparison->hash( _self );
    }
    
    return 0;
}


///
//  DKCopyDescription()
//
DKStringRef DKCopyDescription( DKObjectRef _self )
{
    if( _self )
    {
        DKDescription * description = DKGetInterface( _self, DKSelector(Description) );
        return description->copyDescription( _self );
    }
    
    return DKSTR( "null" );
}



