// =======================================================================================
//
// DKRuntime.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#define DK_RUNTIME_PRIVATE 1
#define DK_THREAD_PRIVATE  1

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"
#include "DKCollection.h"
#include "DKList.h"
#include "DKDictionary.h"
#include "DKArray.h"
#include "DKString.h"
#include "DKHashTable.h"
#include "DKBuffer.h"
#include "DKStream.h"
#include "DKEgg.h"
#include "DKAllocation.h"
#include "DKComparison.h"
#include "DKConversion.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKLocking.h"
#include "DKObjectPool.h"
#include "DKThread.h"
#include "DKMutex.h"



// Internal Types ========================================================================

static void DKClassFinalize( DKObjectRef _self );




// Root Classes ==========================================================================
static struct DKClass __DKRootClass__;
static struct DKClass __DKClassClass__;
static struct DKClass __DKSelectorClass__;
static struct DKClass __DKInterfaceClass__;
static struct DKClass __DKMsgHandlerClass__;
static struct DKClass __DKMetadataClass__;
static struct DKClass __DKObjectClass__;
static struct DKClass __DKZombieClass__;


DKClassRef DKRootClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKRootClass__;
}

DKClassRef DKClassClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKClassClass__;
}

DKClassRef DKSelectorClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKSelectorClass__;
}

DKClassRef DKInterfaceClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKInterfaceClass__;
}

DKClassRef DKMsgHandlerClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKMsgHandlerClass__;
}

DKClassRef DKMetadataClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKMetadataClass__;
}

DKClassRef DKObjectClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKObjectClass__;
}

DKClassRef DKZombieClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKZombieClass__;
}




// Interfaces Required for the Root Classes ==============================================
#define DKStaticSelectorInit( name, type, cacheline )                                   \
    static struct _DKSEL DKSelector_ ## name ##_StaticObject =                          \
    {                                                                                   \
        DKInitStaticObjectHeader( &__DKSelectorClass__ ),                               \
        NULL,                                                                           \
        NULL,                                                                           \
        cacheline,                                                                      \
        (unsigned int)DKInterfaceCountMethods( sizeof(type) )                           \
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


DKStaticSelectorInit( Allocation, struct DKAllocationInterface, DKStaticCache_Allocation );
DKStaticSelectorInit( Comparison, struct DKComparisonInterface, DKStaticCache_Comparison );
DKStaticSelectorInit( Copying, struct DKCopyingInterface, DKStaticCache_Copying );
DKStaticSelectorInit( Locking, struct DKLockingInterface, DKStaticCache_Locking );
DKStaticSelectorInit( Buffer, struct DKBufferInterface, DKStaticCache_Buffer );
DKStaticSelectorInit( Stream, struct DKStreamInterface, DKStaticCache_Stream );
DKStaticSelectorInit( Conversion, struct DKConversionInterface, DKStaticCache_Conversion );

// These need to be defined here since they're used by base classes, but they don't need
// to be assigned static cache lines.
DKStaticSelectorInit( Description, struct DKDescriptionInterface, DKStaticCacheSize + DKDynamicCacheSize - 1 );
DKStaticSelectorInit( Egg, struct DKEggInterface, DKStaticCacheSize + DKDynamicCacheSize - 2 );


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
    (DKCopyMethod)DKRetain,
    DKDefaultDeepCopy
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


// DefaultLocking ------------------------------------------------------------------------
static struct DKLockingInterface DKDefaultLocking_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Locking_StaticObject ),
    DKLockObject,
    DKUnlockObject
};

DKInterfaceRef DKDefaultLocking( void )
{
    return &DKDefaultLocking_StaticObject;
}


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




// Global Object Pools ===================================================================

#define DK_GLOBAL_OBJECT_POOL_BASE_SIZE     32
#define DK_NUM_GLOBAL_OBJECT_POOLS          4   // 32, 64, 128, 256

static DKObjectPool _GlobalObjectPools[DK_NUM_GLOBAL_OBJECT_POOLS];
static size_t _MaxSizeForGlobalObjectPool = 0;




// Statistics ============================================================================

#if DK_RUNTIME_STATS
static DKAtomicInt64 _ObjectAllocations = 0;
static DKAtomicInt64 _LiveObjectAllocations = 0;
#endif


///
//  DKRuntimePrintStats()
//
void DKRuntimePrintStats( void )
{
#if DK_RUNTIME_STATS
    uint64_t total = _ObjectAllocations;
    uint64_t live = _LiveObjectAllocations;
    uint64_t pooled = 0;

    if( _MaxSizeForGlobalObjectPool > 0 )
    {
        for( int i = 0; i < DK_NUM_GLOBAL_OBJECT_POOLS; i++ )
        {
            DKObjectPool * pool = &_GlobalObjectPools[i];
            pooled += DKObjectPoolGetAllocatedCount( pool );
        }
    }

    printf( "DKRuntime Statistics:\n" );
    printf( "  Object allocations:        %" PRId64 "\n", total );
    printf( "  Live Object allocations:   %" PRId64 "\n", live );
    printf( "  Pooled Object allocations: %" PRId64 ", (%0.1lf%%)\n", pooled, ((double)pooled / (double)live) * 100.0 );

    if( _MaxSizeForGlobalObjectPool > 0 )
    {
        for( int i = 0; i < DK_NUM_GLOBAL_OBJECT_POOLS; i++ )
        {
            DKObjectPool * pool = &_GlobalObjectPools[i];
            
            size_t reserved = DKObjectPoolGetReservedCount( pool );
            size_t allocated = DKObjectPoolGetAllocatedCount( pool );
            
            printf( "  Pool %d (%3zu bytes):        %zu / %zu  (%0.2lf%%)\n", i + 1,
                DKObjectPoolGetObjectSize( pool ), allocated, reserved,
                ((double)allocated / (double)reserved) * 100.0 );
        }
    }
#endif
}





// Runtime Init ==========================================================================
static bool _DKRuntimeIsInitialized = false;
static bool _DKRuntimeEnableZombieObjects = false;


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
static void InstallRootClassClassInterface( struct DKClass * _class, DKInterfaceRef _interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKInterface * interface = _interface;
    
    struct DKInterfaceTableRow row;
    row.sel = interface->sel;
    row.interface = interface;
    
    DKGenericHashTableInsert( &_class->classInterfaces.interfaces, &row, DKInsertAlways );
    _class->classInterfaces.cache[interface->sel->cacheline] = interface;
}


///
//  InstallRootClassInstanceInterface()
//
static void InstallRootClassInstanceInterface( struct DKClass * _class, DKInterfaceRef _interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKInterface * interface = _interface;

    struct DKInterfaceTableRow row;
    row.sel = interface->sel;
    row.interface = interface;

    DKGenericHashTableInsert( &_class->instanceInterfaces.interfaces, &row, DKInsertAlways );
    _class->instanceInterfaces.cache[interface->sel->cacheline] = interface;
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
    
    cls->propertiesLock = DKSpinlockInit;
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
void DKRuntimeInit( int options )
{
    if( !_DKRuntimeIsInitialized )
    {
        _DKRuntimeIsInitialized = true;
        _DKRuntimeEnableZombieObjects = (options & DKRuntimeOptionEnableZombieObjects) != 0;

        // Sanity Checks
        //DKRequire( offsetof(struct DKClass, instanceInterfaces) == DK_INTERFACE_TABLE_OFFSET );
        static_assert( offsetof(struct DKClass, instanceInterfaces) == DK_INTERFACE_TABLE_OFFSET, "DK_INTERFACE_TABLE_OFFSET doesn't match the DKClass structure" );

        // Initialize the main thread context
        DKMainThreadContextInit();

        // Initialize the global object pools
        if( options & DKRuntimeOptionUseGlobalObjectPools )
        {
            for( size_t i = 0; i < DK_NUM_GLOBAL_OBJECT_POOLS; i++ )
            {
                size_t blockSize = (size_t)(DK_GLOBAL_OBJECT_POOL_BASE_SIZE) << i;
                DKObjectPoolInit( &_GlobalObjectPools[i], blockSize, DK_GLOBAL_OBJECT_POOL_RESERVE );
                
                _MaxSizeForGlobalObjectPool = blockSize;
            }
        }

        // Initialize the root classes
        InitRootClass( &__DKRootClass__,       NULL,                  sizeof(struct DKClass),   DKPreventSubclassing | DKAbstractBaseClass | DKDisableReferenceCounting, NULL, DKClassFinalize );
        InitRootClass( &__DKClassClass__,      NULL,                  sizeof(struct DKClass),   DKPreventSubclassing, NULL, DKClassFinalize );
        InitRootClass( &__DKSelectorClass__,   NULL,                  sizeof(struct _DKSEL),    DKPreventSubclassing, NULL, DKSelectorFinalize );
        InitRootClass( &__DKInterfaceClass__,  NULL,                  sizeof(DKInterface),      0, NULL, DKInterfaceFinalize );
        InitRootClass( &__DKMsgHandlerClass__, &__DKInterfaceClass__, sizeof(DKMsgHandler),     DKPreventSubclassing, NULL, NULL );
        InitRootClass( &__DKMetadataClass__,   NULL,                  sizeof(struct DKMetadata),DKPreventSubclassing, NULL, DKMetadataFinalize );
        InitRootClass( &__DKObjectClass__,     NULL,                  sizeof(DKObject),         0, NULL, NULL );
        InitRootClass( &__DKZombieClass__,     NULL,                  sizeof(DKObject),         0, NULL, NULL );
        
        // Install custom comparison for selectors, interfaces and message handlers
        InstallRootClassInstanceInterface( &__DKSelectorClass__, DKSelectorComparison() );
        InstallRootClassInstanceInterface( &__DKInterfaceClass__, DKInterfaceComparison() );
        InstallRootClassInstanceInterface( &__DKMsgHandlerClass__, DKInterfaceComparison() );

        // Install default locking for instances of DKObject
        InstallRootClassInstanceInterface( &__DKObjectClass__, DKDefaultLocking() );

        // Initialize the name database
        DKNameDatabaseInit();

        // Initialize the base class names now that constant strings are available.
        SetRootClassName( &__DKRootClass__, DKSTR( "DKRootClass" ) );
        SetRootClassName( &__DKClassClass__, DKSTR( "DKClass" ) );
        SetRootClassName( &__DKSelectorClass__, DKSTR( "DKSelector" ) );
        SetRootClassName( &__DKInterfaceClass__, DKSTR( "DKInterface" ) );
        SetRootClassName( &__DKMsgHandlerClass__, DKSTR( "DKMsgHandler" ) );
        SetRootClassName( &__DKObjectClass__, DKSTR( "DKObject" ) );
        SetRootClassName( &__DKMetadataClass__, DKSTR( "DKMetadata" ) );
        SetRootClassName( &__DKZombieClass__, DKSTR( "DKZombie" ) );

        SetStaticSelectorName( &DKSelector_Allocation_StaticObject, DKSTR( "Allocation" ) );
        SetStaticSelectorName( &DKSelector_Comparison_StaticObject, DKSTR( "Comparison" ) );
        SetStaticSelectorName( &DKSelector_Copying_StaticObject, DKSTR( "Copying" ) );
        SetStaticSelectorName( &DKSelector_Conversion_StaticObject, DKSTR( "Conversion" ) );
        SetStaticSelectorName( &DKSelector_Description_StaticObject, DKSTR( "Description" ) );
        SetStaticSelectorName( &DKSelector_Buffer_StaticObject, DKSTR( "Buffer" ) );
        SetStaticSelectorName( &DKSelector_Stream_StaticObject, DKSTR( "Stream" ) );
        SetStaticSelectorName( &DKSelector_Egg_StaticObject, DKSTR( "Egg" ) );
        
        SetRootClassName( (struct DKClass *)DKStringClass(), DKSTR( "DKString" ) );
        SetRootClassName( (struct DKClass *)DKConstantStringClass(), DKSTR( "DKConstantString" ) );

        // Initialize the main thread object
        DKThreadGetCurrentThread();
        
        // Initialize the object metadata table
        DKMetadataTableInit();
        
        // Init class/selector symbols
        DKRuntimeInitSymbols();
    }
}




// Classes ===============================================================================

///
//  DKNewClass()
//
DKClassRef DKNewClass( DKStringRef name, DKClassRef superclass, size_t structSize,
    uint32_t options, DKInitMethod init, DKFinalizeMethod finalize )
{
    if( superclass && ((superclass->options & DKPreventSubclassing) != 0) )
    {
        DKFatalError( "DKNewClass: Class '%@' does not allow subclasses.", superclass->name );
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
    
    cls->propertiesLock = DKSpinlockInit;
    cls->properties = DKCopyPropertiesTable( superclass );
    
    // Insert the class into the name database
    DKNameDatabaseInsertClass( cls );
    
    return cls;
}


///
//  DKClassFinalize()
//
static void DKClassFinalize( DKObjectRef _untyped_self )
{
    DKClassRef _self = _untyped_self;
    
    DKAssert( _self->_obj.isa == &__DKClassClass__ );

    DKNameDatabaseRemoveClass( _self );

    DKDebug( "Finalizing class %@\n", _self->name );
    
    // Note: The finalizer chain is still running at this point so make sure to set
    // the members to NULL to avoid accessing dangling pointers.
    
    DKRelease( _self->name );
    _self->name = NULL;
    
    DKRelease( _self->superclass );
    _self->superclass = NULL;

    DKInterfaceTableFinalize( &_self->classInterfaces );
    DKInterfaceTableFinalize( &_self->instanceInterfaces );
    
    // Release properties
    DKRelease( _self->properties );
    _self->properties = NULL;
}




// Objects ===============================================================================

///
//  DKAllocObject()
//
static DKObjectRef DKInitObjectMemory( DKObject * obj, DKClassRef cls, int poolIndex )
{
    // Zero the structure bytes
    memset( obj, 0, cls->structSize );
    
    // Setup the object header
    obj->isa = DKRetain( cls );
    
    if( (cls->options & DKDisableReferenceCounting) != 0 )
        obj->refcount = (poolIndex << DKRefCountPoolShift) | DKRefCountDisabledBit | 1;
    
    else
        obj->refcount = (poolIndex << DKRefCountPoolShift) | 1;
    
    return obj;
}

DKObjectRef DKAllocObject( DKClassRef cls, size_t extraBytes )
{
    if( !cls )
    {
        DKWarning( "DKAllocObject: Specified class object is NULL." );
        return NULL;
    }
    
    if( cls->structSize < sizeof(DKObject) )
    {
        DKFatalError( "DKAllocObject: Requested struct size is smaller than DKObject." );
    }
    
    if( (cls->options & DKAbstractBaseClass) != 0 )
    {
        DKFatalError( "DKAllocObject: Class '%@' is an abstract base class", cls->name );
    }

#if DK_RUNTIME_STATS
    DKAtomicIncrement64( &_ObjectAllocations );
    DKAtomicIncrement64( &_LiveObjectAllocations );
#endif

    // Allocate the structure + extra bytes
    size_t allocSize = cls->structSize + extraBytes;

    if( allocSize <= _MaxSizeForGlobalObjectPool )
    {
        for( int i = 0; i < DK_NUM_GLOBAL_OBJECT_POOLS; i++ )
        {
            DKObjectPool * pool = &_GlobalObjectPools[i];
            
            if( allocSize <= DKObjectPoolGetObjectSize( pool ) )
            {
                DKObject * obj = DKObjectPoolThreadSafeAlloc( pool );
                return DKInitObjectMemory( obj, cls, i + 1 );
            }
        }
    }
    
    DKObject * obj = dk_malloc( allocSize );
    return DKInitObjectMemory( obj, cls, 0 );
}


///
//  DKDeallocObject()
//
void DKDeallocObject( DKObjectRef _self )
{
    DKObject * obj = _self;
    DKClassRef cls = obj->isa;
    
    DKAssert( obj );

#if DK_RUNTIME_STATS
    DKAtomicDecrement64( &_LiveObjectAllocations );
#endif

    int32_t rc = DKAtomicLoad32( &obj->refcount );
    DKAssert( ((rc & DKRefCountMask) == 0) || ((rc & DKRefCountDisabledBit) != 0) );

    // Deallocate
    if( _DKRuntimeEnableZombieObjects )
    {
        obj->isa = DKZombieClass();

        if( cls->structSize >= sizeof(DKZombie) )
        {
            DKZombie * zombie = _self;
            zombie->wasa = cls;
        }
        
        else
        {
            DKRelease( cls );
        }
    }

    else
    {
        int poolIndex = DKObjectGetPoolIndex( rc );
        DKAssert( (poolIndex >= 0) && (poolIndex <= DK_NUM_GLOBAL_OBJECT_POOLS) );
        
        if( poolIndex )
        {
            DKObjectPool * pool = &_GlobalObjectPools[poolIndex - 1];
            DKObjectPoolThreadSafeFree( pool, obj );
        }
        
        else
        {
            dk_free( obj );
        }

        DKRelease( cls );
    }
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
            {
                return cls->init( _self );
            }
            
            if( cls->options & DKNoImplicitInitializer )
            {
                DKFatalError( "DKInit: Class '%@' has no implicit initializer.", cls->name );
            }
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
            {
                return cls->init( _self );
            }

            if( cls->options & DKNoImplicitInitializer )
            {
                DKFatalError( "DKSuperInit: Class '%@' has no implicit initializer.", cls->name );
            }
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


///
//  DKLockObject()
//
void DKLockObject( DKObjectRef _self )
{
    if( _self )
    {
        DKMetadataRef metadata = DKMetadataFindOrInsert( _self );
        
        if( metadata->mutex == NULL )
        {
            DKMutexRef mutex = DKNewMutex();
            
            void * _null = NULL;
            
            if( !DKAtomicCompareAndSwapPtr( &metadata->mutex, &_null, mutex ) )
                DKRelease( mutex );
        }
        
        DKMutexLock( metadata->mutex );
    }
}


///
//  DKUnlockObject()
//
void DKUnlockObject( DKObjectRef _self )
{
    if( _self )
    {
        DKMetadataRef metadata = DKMetadataFindOrInsert( _self );
        DKMutexUnlock( metadata->mutex );
    }
}





