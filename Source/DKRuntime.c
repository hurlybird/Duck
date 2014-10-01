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

#define DK_RUNTIME_PRIVATE 1

#include "DKPlatform.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"
#include "DKString.h"
#include "DKHashTable.h"
#include "DKArray.h"
#include "DKStream.h"
#include "DKEgg.h"



// Internal Types ========================================================================

// DKWeak
struct DKWeak
{
    DKObject        _obj;
    DKSpinLock      lock;
    DKObjectRef     target;
};


static void DKClassFinalize( DKObjectRef _self );
static void DKSelectorFinalize( DKObjectRef _self );

static void DKInterfaceGroupInit( struct DKInterfaceGroup * interfaceGroup );
static void DKInterfaceGroupFinalize( struct DKInterfaceGroup * interfaceGroup );

static void DKInstallInterfaceInGroup( DKClassRef _class, DKInterfaceRef _interface, struct DKInterfaceGroup * interfaceGroup );
static DKInterface * DKLookupInterfaceInGroup( DKClassRef _class, DKSEL sel, struct DKInterfaceGroup * interfaceGroup );




// Root Classes ==========================================================================
static struct DKClass __DKRootClass__;
static struct DKClass __DKClassClass__;
static struct DKClass __DKSelectorClass__;
static struct DKClass __DKInterfaceClass__;
static struct DKClass __DKMsgHandlerClass__;
static struct DKClass __DKWeakClass__;
static struct DKClass __DKObjectClass__;


DKClassRef DKRootClass( void )
{
    DKFatal( DKRuntimeIsInitialized() );
    return &__DKRootClass__;
}

DKClassRef DKClassClass( void )
{
    DKFatal( DKRuntimeIsInitialized() );
    return &__DKClassClass__;
}

DKClassRef DKSelectorClass( void )
{
    DKFatal( DKRuntimeIsInitialized() );
    return &__DKSelectorClass__;
}

DKClassRef DKInterfaceClass( void )
{
    DKFatal( DKRuntimeIsInitialized() );
    return &__DKInterfaceClass__;
}

DKClassRef DKMsgHandlerClass( void )
{
    DKFatal( DKRuntimeIsInitialized() );
    return &__DKMsgHandlerClass__;
}

DKClassRef DKWeakClass( void )
{
    DKFatal( DKRuntimeIsInitialized() );
    return &__DKWeakClass__;
}

DKClassRef DKObjectClass( void )
{
    DKFatal( DKRuntimeIsInitialized() );
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
DKStaticFastSelectorInit( Copying );
DKStaticSelectorInit( Description );
DKStaticSelectorInit( Stream );
DKStaticSelectorInit( Egg );




// Error Handling Interfaces =============================================================
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

// DefaultComparison ---------------------------------------------------------------------
static struct DKComparisonInterface DKDefaultComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKPointerEqual,
    DKPointerEqual,
    DKPointerCompare,
    DKPointerHash
};

DKInterfaceRef DKDefaultComparison( void )
{
    return &DKDefaultComparison_StaticObject;
}

bool DKPointerEqual( DKObjectRef _self, DKObjectRef other )
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
    return DKObjectUniqueHash( _self );
}


// DefaultCopying ------------------------------------------------------------------------
static struct DKCopyingInterface DKDefaultCopying_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Copying_StaticObject ),
    DKRetain,
    (DKMutableCopyMethod)DKRetain
};

DKInterfaceRef DKDefaultCopying( void )
{
    return &DKDefaultCopying_StaticObject;
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

// Selector Comparison -------------------------------------------------------------------
#define DKSelectorEqual( a, b )         DKPointerEqual( a, b );
#define DKSelectorCompare( a, b )       DKPointerCompare( a, b );
#define DKSelectorHash( _self )         DKPointerHash( _self )

static struct DKComparisonInterface DKSelectorComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKPointerEqual,
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
    (DKEqualityMethod)DKInterfaceEqual,
    (DKEqualityMethod)DKInterfaceEqual,
    (DKCompareMethod)DKInterfaceCompare,
    (DKHashMethod)DKInterfaceHash
};

static DKInterfaceRef DKInterfaceComparison( void )
{
    return &DKInterfaceComparison_StaticObject;
}






// Runtime Init ==========================================================================
static bool _DKRuntimeIsInitialized = false;


///
//  DKRuntimeIsInitialized()
//
bool DKRuntimeIsInitialized( void )
{
    return _DKRuntimeIsInitialized;
}


///
//  InitRootClass()
//
static void InitRootClass( struct DKClass * cls, struct DKClass * superclass, size_t structSize,
    DKClassOptions options, DKInitMethod init, DKFinalizeMethod finalize )
{
    memset( cls, 0, sizeof(struct DKClass) );
    
    DKObject * obj = (DKObject *)cls;
    obj->isa = &__DKRootClass__;
    obj->refcount = 1;

    cls->name = NULL;
    cls->superclass = DKRetain( superclass );
    cls->structSize = structSize;
    cls->options = options;
    cls->init = init;
    cls->finalize = finalize;

    cls->classInterfaces.lock = DKSpinLockInit;
    DKGenericArrayInit( &cls->classInterfaces.interfaces, sizeof(DKObjectRef) );

    cls->instanceInterfaces.lock = DKSpinLockInit;
    DKGenericArrayInit( &cls->instanceInterfaces.interfaces, sizeof(DKObjectRef) );
    
    cls->propertiesLock = DKSpinLockInit;
}


///
//  InstallRootClassInterface()
//
static void InstallRootClassInstanceInterface( struct DKClass * _class, DKInterfaceRef interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKGenericArrayAppendElements( &_class->instanceInterfaces.interfaces, &interface, 1 );
}


///
//  SetRootClassName()
//
static void SetRootClassName( struct DKClass * _class, DKStringRef name )
{
    _class->name = DKCopy( name );
    _class->hash = DKStringHash( name );
    
    DKNameDatabaseInsertClass( _class );
}


///
//  SetStaticSelectorName()
//
static void SetStaticSelectorName( struct _DKSEL * sel, DKStringRef name )
{
    sel->name = DKCopy( name );
    sel->hash = DKStringHash( name );
    
    DKNameDatabaseInsertSelector( sel );
}


///
//  DKRuntimeInit()
//
void DKRuntimeInit( void )
{
    if( !_DKRuntimeIsInitialized )
    {
        _DKRuntimeIsInitialized = true;

        InitRootClass( &__DKRootClass__,       NULL,                  sizeof(struct DKClass), DKAbstractBaseClass | DKDisableReferenceCounting, NULL, DKClassFinalize );
        InitRootClass( &__DKClassClass__,      NULL,                  sizeof(struct DKClass), 0, NULL, DKClassFinalize );
        InitRootClass( &__DKSelectorClass__,   NULL,                  sizeof(struct _DKSEL),  0, NULL, DKSelectorFinalize );
        InitRootClass( &__DKInterfaceClass__,  NULL,                  sizeof(DKInterface),    0, NULL, DKInterfaceFinalize );
        InitRootClass( &__DKMsgHandlerClass__, &__DKInterfaceClass__, sizeof(DKMsgHandler),   0, NULL, NULL );
        InitRootClass( &__DKWeakClass__,       NULL,                  sizeof(struct DKWeak),  0, NULL, NULL );
        InitRootClass( &__DKObjectClass__,     NULL,                  sizeof(DKObject),       0, NULL, NULL );
        
        InstallRootClassInstanceInterface( &__DKRootClass__, DKDefaultComparison() );
        InstallRootClassInstanceInterface( &__DKRootClass__, DKDefaultDescription() );

        InstallRootClassInstanceInterface( &__DKClassClass__, DKDefaultComparison() );
        InstallRootClassInstanceInterface( &__DKClassClass__, DKDefaultCopying() );
        InstallRootClassInstanceInterface( &__DKClassClass__, DKDefaultDescription() );

        InstallRootClassInstanceInterface( &__DKSelectorClass__, DKSelectorComparison() );
        InstallRootClassInstanceInterface( &__DKSelectorClass__, DKDefaultCopying() );
        InstallRootClassInstanceInterface( &__DKSelectorClass__, DKDefaultDescription() );

        InstallRootClassInstanceInterface( &__DKInterfaceClass__, DKInterfaceComparison() );
        InstallRootClassInstanceInterface( &__DKInterfaceClass__, DKDefaultCopying() );
        InstallRootClassInstanceInterface( &__DKInterfaceClass__, DKDefaultDescription() );

        InstallRootClassInstanceInterface( &__DKMsgHandlerClass__, DKInterfaceComparison() );
        InstallRootClassInstanceInterface( &__DKMsgHandlerClass__, DKDefaultCopying() );
        InstallRootClassInstanceInterface( &__DKMsgHandlerClass__, DKDefaultDescription() );

        InstallRootClassInstanceInterface( &__DKWeakClass__, DKDefaultComparison() );
        InstallRootClassInstanceInterface( &__DKWeakClass__, DKDefaultCopying() );
        InstallRootClassInstanceInterface( &__DKWeakClass__, DKDefaultDescription() );

        InstallRootClassInstanceInterface( &__DKObjectClass__, DKDefaultComparison() );
        InstallRootClassInstanceInterface( &__DKObjectClass__, DKDefaultCopying() );
        InstallRootClassInstanceInterface( &__DKObjectClass__, DKDefaultDescription() );

        // Initialize the name database
        DKNameDatabaseInit();

        // Initialize the base class names now that constant strings are available.
        SetRootClassName( &__DKRootClass__, DKSTR( "DKRootClass" ) );
        SetRootClassName( &__DKClassClass__, DKSTR( "DKClass" ) );
        SetRootClassName( &__DKSelectorClass__, DKSTR( "DKSelector" ) );
        SetRootClassName( &__DKInterfaceClass__, DKSTR( "DKInterface" ) );
        SetRootClassName( &__DKMsgHandlerClass__, DKSTR( "DKMsgHandler" ) );
        SetRootClassName( &__DKObjectClass__, DKSTR( "DKObject" ) );
        SetRootClassName( &__DKWeakClass__, DKSTR( "DKWeak" ) );

        SetStaticSelectorName( &DKSelector_InterfaceNotFound_StaticObject, DKSTR( "InterfaceNotFound" ) );
        SetStaticSelectorName( &DKSelector_MsgHandlerNotFound_StaticObject, DKSTR( "MsgHandlerNotFound" ) );
        SetStaticSelectorName( &DKSelector_Allocation_StaticObject, DKSTR( "Allocation" ) );
        SetStaticSelectorName( &DKSelector_Comparison_StaticObject, DKSTR( "Comparison" ) );
        SetStaticSelectorName( &DKSelector_Copying_StaticObject, DKSTR( "Copying" ) );
        SetStaticSelectorName( &DKSelector_Description_StaticObject, DKSTR( "Description" ) );
        SetStaticSelectorName( &DKSelector_Stream_StaticObject, DKSTR( "Stream" ) );
        SetStaticSelectorName( &DKSelector_Egg_StaticObject, DKSTR( "Egg" ) );
        
        SetRootClassName( (struct DKClass *)DKStringClass(), DKSTR( "DKString" ) );
        SetRootClassName( (struct DKClass *)DKConstantStringClass(), DKSTR( "DKConstantString" ) );
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
    DKObject * obj = dk_malloc( cls->structSize + extraBytes );
    
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
    dk_free( obj );
    
    // Finally release the class object
    DKRelease( cls );
}




// Creating Classes ======================================================================

///
//  DKInterfaceGroupInit()
//
static void DKInterfaceGroupInit( struct DKInterfaceGroup * interfaceGroup )
{
    interfaceGroup->lock = DKSpinLockInit;
    memset( interfaceGroup->cache, 0, sizeof(interfaceGroup->cache) );
    DKGenericArrayInit( &interfaceGroup->interfaces, sizeof(DKObjectRef) );
}


///
//  DKInterfaceGroupFinalize()
//
static void DKInterfaceGroupFinalize( struct DKInterfaceGroup * interfaceGroup )
{
    DKIndex count = interfaceGroup->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * interface = DKGenericArrayGetElementAtIndex( &interfaceGroup->interfaces, i, DKInterface * );
        DKRelease( interface );
    }
    
    DKGenericArrayFinalize( &interfaceGroup->interfaces );
}


///
//  DKAllocClass()
//
DKClassRef DKAllocClass( DKStringRef name, DKClassRef superclass, size_t structSize,
    DKClassOptions options, DKInitMethod init, DKFinalizeMethod finalize )
{
    if( superclass && ((superclass->options & DKPreventSubclassing) != 0) )
    {
        DKFatalError( "DKAllocClass: Class '%s' does not allow subclasses.\n", DKStringGetCStringPtr( superclass->name ) );
        return NULL;
    }

    struct DKClass * cls = DKCreate( DKClassClass() );

    cls->name = DKCopy( name );
    cls->hash = DKStringHash( name );
    cls->superclass = DKRetain( superclass );
    cls->structSize = structSize;
    cls->options = options;
    cls->init = init;
    cls->finalize = finalize;

    DKInterfaceGroupInit( &cls->classInterfaces );
    DKInterfaceGroupInit( &cls->instanceInterfaces );
    
    cls->propertiesLock = DKSpinLockInit;
    
    // Insert the class into the name database
    DKNameDatabaseInsertClass( cls );
    
    return cls;
}


///
//  DKClassFinalize()
//
static void DKClassFinalize( DKObjectRef _self )
{
    struct DKClass * cls = (struct DKClass *)_self;
    
    DKAssert( cls->_obj.isa == &__DKClassClass__ );

    DKNameDatabaseRemoveClass( cls );

    DKPrintf( "Finalizing class %@\n", cls->name );
    
    // Note: The finalizer chain is still running at this point so make sure to set
    // the members to NULL to avoid accessing dangling pointers.
    
    DKRelease( cls->name );
    cls->name = NULL;
    
    DKRelease( cls->superclass );
    cls->superclass = NULL;

    DKInterfaceGroupFinalize( &cls->classInterfaces );
    DKInterfaceGroupFinalize( &cls->instanceInterfaces );
    
    // Release properties
    DKRelease( cls->properties );
    cls->properties = NULL;
}


///
//  DKAllocSelector()
//
DKSEL DKAllocSelector( DKStringRef name )
{
    struct _DKSEL * sel = DKInit( DKAlloc( DKSelectorClass(), 0 ) );

    DKAssert( sel != NULL );

    sel->name = DKCopy( name );
    sel->cacheline = DKDynamicCache;

    DKNameDatabaseInsertSelector( sel );

    return sel;
}


///
//  DKSelectorFinalize()
//
static void DKSelectorFinalize( DKObjectRef _self )
{
    DKSEL sel = _self;

    DKNameDatabaseRemoveSelector( sel );

    DKRelease( sel->name );
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
                DKFinalize( _self );
                DKDealloc( _self );
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




// Polymorphic Wrappers ==================================================================

///
//  DKAlloc()
//
void * DKAlloc( DKClassRef _class, size_t extraBytes )
{
    DKObject * obj = NULL;
    
    if( _class )
    {
        DKAllocationInterfaceRef allocation;
        
        if( DKQueryClassInterface( _class, DKSelector(Allocation), (DKInterfaceRef *)&allocation ) )
            obj = allocation->alloc( _class, extraBytes );
        
        else
            obj = DKAllocObject( _class, extraBytes );
    }
    
    return obj;
}


///
//  DKDealloc()
//
void DKDealloc( DKObjectRef _self )
{
    DKObject * obj = (DKObject *)_self;
    
    DKAssert( obj );
    DKAssert( obj->refcount == 0 );
    DKAssert( obj->weakref == NULL );

    DKAllocationInterfaceRef allocation;

    if( DKQueryClassInterface( obj->isa, DKSelector(Allocation), (DKInterfaceRef *)&allocation ) )
        allocation->dealloc( obj );
    
    else
        DKDeallocObject( obj );
}


///
//  DKInit()
//
void * DKInit( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        
        if( obj->isa->init )
            _self = obj->isa->init( _self );
        
        else
            _self = DKSuperInit( _self, DKGetSuperclass( _self ) );
    }
    
    return (void *)_self;
}


///
//  DKSuperInit()
//
void * DKSuperInit( DKObjectRef _self, DKClassRef superclass )
{
    if( _self && superclass )
    {
        DKAssertKindOfClass( _self, superclass );
    
        if( superclass->init )
            _self = superclass->init( _self );
        
        else
            _self = DKSuperInit( _self, superclass->superclass );
    }
    
    return (void *)_self;
}


///
//  DKFinalize()
//
void DKFinalize( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;

        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls->finalize )
                cls->finalize( obj );
        }
    }
}


///
//  DKEqual()
//
bool DKEqual( DKObjectRef a, DKObjectRef b )
{
    if( a && b )
    {
        if( a == b )
            return true;
        
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->equal( a, b );
    }
    
    return false;
}


///
//  DKLike()
//
bool DKLike( DKObjectRef a, DKObjectRef b )
{
    if( a && b )
    {
        if( a == b )
            return true;
        
        DKComparisonInterfaceRef comparison = DKGetInterface( a, DKSelector(Comparison) );
        return comparison->like( a, b );
    }
    
    return false;
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







