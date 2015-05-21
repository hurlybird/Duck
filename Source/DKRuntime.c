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
#include "DKAllocation.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKThread.h"



// Internal Types ========================================================================

static void DKClassFinalize( DKObjectRef _self );
static void DKInterfaceTableInit( struct DKInterfaceTable * interfaceTable, struct DKInterfaceTable * inheritedInterfaces );
static void DKInterfaceTableFinalize( struct DKInterfaceTable * interfaceTable );




// Thread Context ========================================================================
static pthread_key_t DKThreadContextKey;
static struct DKThreadContext * DKMainThreadContext = NULL;


///
//  DKFreeThreadContext()
//
static void DKFreeThreadContext( void * context )
{
    struct DKThreadContext * threadContext = context;
    
    DKRelease( threadContext->threadObject );
    
    DKFatal( threadContext->arpStack.top == -1 );
    
    for( int i = 0; i < DK_AUTORELEASE_POOL_STACK_SIZE; i++ )
        DKGenericArrayFinalize( &threadContext->arpStack.arp[i] );
    
    dk_free( threadContext );

    pthread_setspecific( DKThreadContextKey, NULL );
}


///
//  DKGetCurrentThreadContext()
//
struct DKThreadContext * DKGetCurrentThreadContext( void )
{
    struct DKThreadContext * threadContext = pthread_getspecific( DKThreadContextKey );

    if( !threadContext )
    {
        // Create a new stack
        threadContext = dk_malloc( sizeof(struct DKThreadContext) );
        memset( threadContext, 0, sizeof(struct DKThreadContext) );
        
        // Initialize the autorelease pool stack
        threadContext->arpStack.top = -1;

        for( int i = 0; i < DK_AUTORELEASE_POOL_STACK_SIZE; i++ )
            DKGenericArrayInit( &threadContext->arpStack.arp[i], sizeof(DKObjectRef) );
        
        // Save the stack to the current thread
        pthread_setspecific( DKThreadContextKey, threadContext );
    }

    return threadContext;
}


///
//  DKGetMainThreadContext()
//
struct DKThreadContext * DKGetMainThreadContext( void )
{
    return DKMainThreadContext;
}




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
        DKInitStaticObjectHeader( &__DKSelectorClass__ ),                               \
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
        DKInitStaticObjectHeader( &__DKInterfaceClass__ ),                              \
        sel                                                                             \
    }


DKStaticSelectorInit( Allocation );
DKStaticSelectorInit( Comparison );
DKStaticSelectorInit( Copying );
DKStaticSelectorInit( Description );
DKStaticSelectorInit( Stream );
DKStaticSelectorInit( Egg );




// Default Interfaces ====================================================================

// DefaultAllocation ---------------------------------------------------------------------
static struct DKAllocationInterface DKDefaultAllocation_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Allocation_StaticObject ),
    DKAllocObject,
    DKDeallocObject,
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
    DKDefaultGetDescription,
    DKDefaultGetSizeInBytes
};

DKInterfaceRef DKDefaultDescription( void )
{
    return &DKDefaultDescription_StaticObject;
}




// Root Class Interfaces =================================================================

// Selector Comparison -------------------------------------------------------------------
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
//  InstallRootClassClassInterface()
//
static void InstallRootClassClassInterface( struct DKClass * _class, DKInterfaceRef interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKGenericHashTableInsert( &_class->classInterfaces.interfaces, &interface, DKInsertAlways );
}


///
//  InstallRootClassInstanceInterface()
//
static void InstallRootClassInstanceInterface( struct DKClass * _class, DKInterfaceRef interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKGenericHashTableInsert( &_class->instanceInterfaces.interfaces, &interface, DKInsertAlways );
}


///
//  InitRootClass()
//
static void InitRootClass( struct DKClass * cls, struct DKClass * superclass, size_t structSize,
    uint32_t options, DKInitMethod init, DKFinalizeMethod finalize )
{
    memset( cls, 0, sizeof(struct DKClass) );
    
    DKObject * obj = (DKObject *)cls;
    obj->isa = &__DKRootClass__;
    obj->refcount = DKRefCountDisabledBit | 1;

    cls->name = NULL;
    cls->superclass = DKRetain( superclass );
    cls->structSize = (uint32_t)structSize;
    cls->options = options;
    cls->init = init;
    cls->finalize = finalize;

    DKInterfaceTableInit( &cls->classInterfaces, superclass ? &superclass->classInterfaces : NULL );
    DKInterfaceTableInit( &cls->instanceInterfaces, superclass ? &superclass->instanceInterfaces : NULL );

    InstallRootClassClassInterface( cls, DKDefaultAllocation() );
    InstallRootClassInstanceInterface( cls, DKDefaultComparison() );
    InstallRootClassInstanceInterface( cls, DKDefaultCopying() );
    InstallRootClassInstanceInterface( cls, DKDefaultDescription() );
    
    cls->propertiesLock = DKSpinLockInit;
}


///
//  SetRootClassName()
//
static void SetRootClassName( struct DKClass * _class, DKStringRef name )
{
    _class->name = DKCopy( name );
    DKNameDatabaseInsertClass( _class );
}


///
//  SetStaticSelectorName()
//
static void SetStaticSelectorName( struct _DKSEL * sel, DKStringRef name )
{
    sel->name = DKCopy( name );
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

        // Initialize the main thread context
        pthread_key_create( &DKThreadContextKey, DKFreeThreadContext );
        DKMainThreadContext = DKGetCurrentThreadContext();

        // Initialize the root classes
        InitRootClass( &__DKRootClass__,       NULL,                  sizeof(struct DKClass), DKAbstractBaseClass | DKDisableReferenceCounting, NULL, DKClassFinalize );
        InitRootClass( &__DKClassClass__,      NULL,                  sizeof(struct DKClass), 0, NULL, DKClassFinalize );
        InitRootClass( &__DKSelectorClass__,   NULL,                  sizeof(struct _DKSEL),  0, NULL, DKSelectorFinalize );
        InitRootClass( &__DKInterfaceClass__,  NULL,                  sizeof(DKInterface),    0, NULL, DKInterfaceFinalize );
        InitRootClass( &__DKMsgHandlerClass__, &__DKInterfaceClass__, sizeof(DKMsgHandler),   0, NULL, NULL );
        InitRootClass( &__DKWeakClass__,       NULL,                  sizeof(struct DKWeak),  0, NULL, NULL );
        InitRootClass( &__DKObjectClass__,     NULL,                  sizeof(DKObject),       0, NULL, NULL );
        
        // Install custom comparison for selectors, interfaces and message handlers
        InstallRootClassInstanceInterface( &__DKSelectorClass__, DKSelectorComparison() );
        InstallRootClassInstanceInterface( &__DKInterfaceClass__, DKInterfaceComparison() );
        InstallRootClassInstanceInterface( &__DKMsgHandlerClass__, DKInterfaceComparison() );

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

        SetStaticSelectorName( &DKSelector_Allocation_StaticObject, DKSTR( "Allocation" ) );
        SetStaticSelectorName( &DKSelector_Comparison_StaticObject, DKSTR( "Comparison" ) );
        SetStaticSelectorName( &DKSelector_Copying_StaticObject, DKSTR( "Copying" ) );
        SetStaticSelectorName( &DKSelector_Description_StaticObject, DKSTR( "Description" ) );
        SetStaticSelectorName( &DKSelector_Stream_StaticObject, DKSTR( "Stream" ) );
        SetStaticSelectorName( &DKSelector_Egg_StaticObject, DKSTR( "Egg" ) );
        
        SetRootClassName( (struct DKClass *)DKStringClass(), DKSTR( "DKString" ) );
        SetRootClassName( (struct DKClass *)DKConstantStringClass(), DKSTR( "DKConstantString" ) );

        // Initialize the main thread object
        DKThreadGetCurrentThread();
        
        // Initialize the weak reference table
        DKWeakReferenceTableInit();
    }
}




// DKInterfaceTable ======================================================================

// Hash Table Callbacks
static DKRowStatus InterfaceTableRowStatus( const void * _row )
{
    DKInterface * const * row = _row;
    return (DKRowStatus)DK_HASHTABLE_ROW_STATUS( *row );
}

static DKHashCode InterfaceTableRowHash( const void * _row )
{
    DKInterface * const * row = _row;
    return DKObjectUniqueHash( (*row)->sel );
}

static bool InterfaceTableRowEqual( const void * _row1, const void * _row2 )
{
    DKInterface * const * row1 = _row1;
    DKInterface * const * row2 = _row2;

    return DKSelectorEqual( (*row1)->sel, (*row2)->sel );
}

static void InterfaceTableRowInit( void * _row )
{
    DKInterface ** row = _row;
    *row = DK_HASHTABLE_EMPTY_KEY;
}

static void InterfaceTableRowUpdate( void * _row, const void * _src )
{
    DKInterface ** row = _row;
    DKInterface * const * src = _src;
    
    DKRetain( *src );
    
    if( DK_HASHTABLE_IS_POINTER( *row ) )
        DKRelease( *row );
        
    *row = *src;
}

static void InterfaceTableRowDelete( void * _row )
{
    DKInterface ** row = _row;
    
    DKRelease( *row );
    *row = DK_HASHTABLE_DELETED_KEY;
}

static void InterfaceTableForeachRowCallback( const void * _row, void * context )
{
    struct DKInterfaceTable * interfaceTable = context;
    DKGenericHashTableInsert( &interfaceTable->interfaces, _row, DKInsertAlways );
}


///
//  DKInterfaceTableInit()
//
static void DKInterfaceTableInit( struct DKInterfaceTable * interfaceTable, struct DKInterfaceTable * inheritedInterfaces )
{
    DKGenericHashTableCallbacks callbacks =
    {
        InterfaceTableRowStatus,
        InterfaceTableRowHash,
        InterfaceTableRowEqual,
        InterfaceTableRowInit,
        InterfaceTableRowUpdate,
        InterfaceTableRowDelete
    };

    memset( interfaceTable->cache, 0, sizeof(interfaceTable->cache) );

    interfaceTable->lock = DKSpinLockInit;

    DKGenericHashTableInit( &interfaceTable->interfaces, sizeof(DKObjectRef), &callbacks );
    
    if( inheritedInterfaces )
        DKGenericHashTableForeachRow( &inheritedInterfaces->interfaces, InterfaceTableForeachRowCallback, interfaceTable );
}


///
//  DKInterfaceTableFinalize()
//
static void DKInterfaceTableFinalize( struct DKInterfaceTable * interfaceTable )
{
    DKGenericHashTableFinalize( &interfaceTable->interfaces );
}


///
//  DKInterfaceTableInsert()
//
void DKInterfaceTableInsert( DKClassRef _class, struct DKInterfaceTable * interfaceTable, DKInterfaceRef _interface )
{
    // *** WARNING ***
    // Swizzling interfaces on base classes isn't fully supported. In order to do so we
    // would need to update the interfaces tables of all subclasses.
    // *** WARNING ***
    
    DKAssertMemberOfClass( _class, DKClassClass() );
    DKAssertKindOfClass( _interface, DKInterfaceClass() );

    DKInterface * interface = (DKInterface *)_interface;

    // Get the cache line from the selector
    unsigned int cacheline = interface->sel->cacheline;
    DKAssert( cacheline < (DKStaticCacheSize + DKDynamicCacheSize) );
    
    // Invalidate the cache
    interfaceTable->cache[cacheline] = NULL;

    // Replace the interface in the interface table
    DKSpinLockLock( &interfaceTable->lock );
    DKGenericHashTableInsert( &interfaceTable->interfaces, &interface, DKInsertAlways );
    DKSpinLockUnlock( &interfaceTable->lock );
}


///
//  DKInterfaceTableFind()
//
DKInterface * DKInterfaceTableFind( DKClassRef _class, struct DKInterfaceTable * interfaceTable, DKSEL sel, DKInterfaceNotFoundCallback interfaceNotFound )
{
    DKAssert( sel->_obj.isa == DKSelectorClass() );

    // Get the cache line from the selector
    unsigned int cacheline = sel->cacheline;
    DKAssert( cacheline < (DKStaticCacheSize + DKDynamicCacheSize) );

    // We shoudn't need to acquire the spin lock while reading and writing to the cache
    // since the worst that can happen is doing an extra lookup after reading a stale
    // cache line.

    // Check the cached interface
    DKInterface * interface = interfaceTable->cache[cacheline];
    
    if( interface && DKSelectorEqual( interface->sel, sel ) )
        return interface;

    // Search our interface table
    DKInterface _key;
    _key.sel = sel;
    
    DKInterface * key = &_key;

    DKSpinLockLock( &interfaceTable->lock );
    DKInterface ** entry = (DKInterface **)DKGenericHashTableFind( &interfaceTable->interfaces, &key );
    DKSpinLockUnlock( &interfaceTable->lock );
    
    if( entry )
    {
        // Update the cache
        interfaceTable->cache[cacheline] = *entry;

        return *entry;
    }

    return interfaceNotFound( _class, sel );
}




// Creating Classes ======================================================================

///
//  DKAllocClass()
//
DKClassRef DKAllocClass( DKStringRef name, DKClassRef superclass, size_t structSize,
    uint32_t options, DKInitMethod init, DKFinalizeMethod finalize )
{
    if( superclass && ((superclass->options & DKPreventSubclassing) != 0) )
    {
        DKFatalError( "DKAllocClass: Class '%s' does not allow subclasses.\n", DKStringGetCStringPtr( superclass->name ) );
        return NULL;
    }
    
    if( structSize == 0 )
        structSize = superclass ? superclass->structSize : sizeof(DKObject);

    struct DKClass * cls = DKNew( DKClassClass() );

    cls->name = DKCopy( name );
    cls->superclass = DKRetain( superclass );
    cls->structSize = (uint32_t)structSize;
    cls->options = options;
    cls->init = init;
    cls->finalize = finalize;

    DKInterfaceTableInit( &cls->classInterfaces, superclass ? &superclass->classInterfaces : NULL );
    DKInterfaceTableInit( &cls->instanceInterfaces, superclass ? &superclass->instanceInterfaces : NULL );
    
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
    struct DKClass * cls = _self;
    
    DKAssert( cls->_obj.isa == &__DKClassClass__ );

    DKNameDatabaseRemoveClass( cls );

    DKPrintf( "Finalizing class %@\n", cls->name );
    
    // Note: The finalizer chain is still running at this point so make sure to set
    // the members to NULL to avoid accessing dangling pointers.
    
    DKRelease( cls->name );
    cls->name = NULL;
    
    DKRelease( cls->superclass );
    cls->superclass = NULL;

    DKInterfaceTableFinalize( &cls->classInterfaces );
    DKInterfaceTableFinalize( &cls->instanceInterfaces );
    
    // Release properties
    DKRelease( cls->properties );
    cls->properties = NULL;
}




// Alloc/Free Objects ====================================================================

///
//  DKAllocObject()
//
DKObjectRef DKAllocObject( DKClassRef cls, size_t extraBytes )
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
    
    if( (cls->options & DKDisableReferenceCounting) != 0 )
        obj->refcount = DKRefCountDisabledBit | 1;
    
    else
        obj->refcount = 1;
    
    return obj;
}


///
//  DKDeallocObject()
//
void DKDeallocObject( DKObjectRef _self )
{
    DKObject * obj = _self;
    DKClassRef cls = obj->isa;
    
    DKAssert( obj );
    DKAssert( ((obj->refcount & DKRefCountMask) == 0) || ((obj->refcount & DKRefCountDisabledBit) != 0) );

    // Deallocate
    dk_free( obj );
    
    // Finally release the class object
    DKRelease( cls );
}


///
//  DKAllocEx()
//
DKObjectRef DKAllocEx( DKClassRef _class, size_t extraBytes )
{
    DKObject * obj = NULL;
    
    if( _class )
    {
        DKAllocationInterfaceRef allocation = DKGetClassInterface( _class, DKSelector(Allocation) );
        obj = allocation->alloc( _class, extraBytes );
    }
    
    return obj;
}


///
//  DKDealloc()
//
void DKDealloc( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;
        
        DKAllocationInterfaceRef allocation = DKGetClassInterface( obj->isa, DKSelector(Allocation) );
        allocation->dealloc( obj );
    }
}


///
//  DKInit()
//
DKObjectRef DKInit( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;
        
        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls->init )
                return cls->init( _self );
        }
    }
    
    return _self;
}


///
//  DKSuperInit()
//
DKObjectRef DKSuperInit( DKObjectRef _self, DKClassRef superclass )
{
    if( _self && superclass )
    {
        DKAssertKindOfClass( _self, superclass );

        for( DKClassRef cls = superclass; cls != NULL; cls = cls->superclass )
        {
            if( cls->init )
                return cls->init( _self );
        }
    }
    
    return _self;
}


///
//  DKFinalize()
//
void DKFinalize( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;

        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls->finalize )
                cls->finalize( obj );
        }
    }
}








