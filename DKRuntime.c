//
//  DKRuntime.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-20.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

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
    const DKObjectHeader    _obj;
    
    DKStringRef             name;
    const struct DKClass *  superclass;
    size_t                  structSize;

    DKSpinLock              lock;

    DKTypeRef               vtable[DKVTableSize];
    
    DKPointerArray          interfaces;
    DKPointerArray          properties;
};

typedef const struct DKClass DKClass;


// DKWeak
struct DKWeak
{
    DKObjectHeader  _obj;
    DKSpinLock      lock;
    DKTypeRef       target;
};

typedef const struct DKWeak DKWeak;




// Root Classes ==========================================================================
static struct DKClass __DKMetaClass__;
static struct DKClass __DKClassClass__;
static struct DKClass __DKSelectorClass__;
static struct DKClass __DKInterfaceClass__;
static struct DKClass __DKMsgHandlerClass__;
static struct DKClass __DKPropertyClass__;
static struct DKClass __DKObjectClass__;
static struct DKClass __DKWeakClass__;


DKTypeRef DKClassClass( void )
{
    DKRuntimeInit();
    return &__DKClassClass__;
}

DKTypeRef DKSelectorClass( void )
{
    DKRuntimeInit();
    return &__DKSelectorClass__;
}

DKTypeRef DKInterfaceClass( void )
{
    DKRuntimeInit();
    return &__DKInterfaceClass__;
}

DKTypeRef DKMsgHandlerClass( void )
{
    DKRuntimeInit();
    return &__DKMsgHandlerClass__;
}

DKTypeRef DKPropertyClass( void )
{
    DKRuntimeInit();
    return &__DKPropertyClass__;
}

DKTypeRef DKObjectClass( void )
{
    DKRuntimeInit();
    return &__DKObjectClass__;
}

static DKTypeRef DKWeakClass( void )
{
    DKRuntimeInit();
    return &__DKWeakClass__;
}




// Default Interfaces ====================================================================

#define DKStaticSelectorInit( name )                                                    \
    static struct DKSEL DKSelector_ ## name ##_StaticObject =                           \
    {                                                                                   \
        DKStaticObjectHeader( &__DKSelectorClass__ ),                                   \
        #name,                                                                          \
        0                                                                               \
    };                                                                                  \
                                                                                        \
    DKSEL DKSelector_ ## name( void )                                                   \
    {                                                                                   \
        return &DKSelector_ ## name ##_StaticObject;                                    \
    }


#define DKStaticFastSelectorInit( name )                                                \
    static struct DKSEL DKSelector_ ## name ##_StaticObject =                           \
    {                                                                                   \
        DKStaticObjectHeader( &__DKSelectorClass__ ),                                   \
        #name,                                                                          \
        DKVTable_ ## name                                                               \
    };                                                                                  \
                                                                                        \
    DKSEL DKSelector_ ## name( void )                                                   \
    {                                                                                   \
        return &DKSelector_ ## name ##_StaticObject;                                    \
    }


#define DKStaticInterfaceObject( sel )                                                  \
    {                                                                                   \
        DKStaticObjectHeader( &__DKInterfaceClass__ ),                                  \
        sel                                                                             \
    }


// Not Found -----------------------------------------------------------------------------
#define DK_MAX_INTERFACE_SIZE   32

DKDeclareInterfaceSelector( InterfaceNotFound );
DKStaticSelectorInit( InterfaceNotFound );

DKDeclareMessageSelector( MsgHandlerNotFound );
DKStaticSelectorInit( MsgHandlerNotFound );

typedef void (*DKInterfaceNotFoundFunction)( struct DKObjectHeader * obj );

struct DKInterfaceNotFoundInterface
{
    DKInterface _interface;
    DKInterfaceNotFoundFunction func[DK_MAX_INTERFACE_SIZE];
};

static void DKInterfaceNotFoundCallback( struct DKObjectHeader * obj )
{
    // Note: 'obj' is for debugging only and may not be valid. Most interface functions
    // take an object as a first arguement, but it's not actually required.

    // The only time this code is ever likely to be called is when calling an interface
    // function on a NULL object. We don't have the Objective-C luxury of tweaking the
    // call stack and return value in assembly for such cases.

    DKFatalError( "DKRuntime: Invalid interface call.\n" );
}

static void DKMsgHandlerNotFoundCallback( struct DKObjectHeader * obj, DKSEL sel )
{
    // This handles silently failing when sending messages to NULL objects.
}

DKThreadSafeSharedObjectInit( DKInterfaceNotFound, DKTypeRef )
{
    struct DKInterfaceNotFoundInterface * interface = DKAllocInterface( DKSelector(InterfaceNotFound), sizeof(struct DKInterfaceNotFoundInterface) );

    for( int i = 0; i < DK_MAX_INTERFACE_SIZE; i++ )
        interface->func[i] = DKInterfaceNotFoundCallback;
    
    return interface;
}

DKThreadSafeSharedObjectInit( DKMsgHandlerNotFound, DKTypeRef )
{
    struct DKMsgHandler * msgHandler = DKAllocInterface( DKSelector(MsgHandlerNotFound), sizeof(struct DKMsgHandler) );

    msgHandler->func = DKMsgHandlerNotFoundCallback;

    return msgHandler;
}


// LifeCycle -----------------------------------------------------------------------------
DKStaticFastSelectorInit( LifeCycle );

static struct DKLifeCycle DKDefaultLifeCycle_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_LifeCycle_StaticObject ),
    NULL,
    NULL,
    NULL,
    NULL
};

DKTypeRef DKDefaultLifeCycle( void )
{
    return &DKDefaultLifeCycle_StaticObject;
}


// Comparison ----------------------------------------------------------------------------
DKStaticFastSelectorInit( Comparison );

static struct DKComparison DKDefaultComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKDefaultEqual,
    DKDefaultCompare,
    DKDefaultHash
};

DKTypeRef DKDefaultComparison( void )
{
    return &DKDefaultComparison_StaticObject;
}

int DKDefaultEqual( DKTypeRef ref, DKTypeRef other )
{
    return ref == other;
}

int DKDefaultCompare( DKTypeRef ref, DKTypeRef other )
{
    if( ref < other )
        return 1;
    
    if( ref > other )
        return -1;
    
    return 0;
}

DKHashCode DKDefaultHash( DKTypeRef ref )
{
    // Just in case someone changes the size of DKHashCode
    DKAssert( sizeof(DKHashCode) == sizeof(DKTypeRef) );

    // Make sure object pointers are 4-byte aligned
    DKAssert( ((DKHashCode)ref & 0x3) == 0 );
    
    // Assuming object pointers are at least 4-byte aligned, this will make hash codes
    // derived from pointers a bit more random. This is particularly important in a hash
    // table which uses (hashcode % prime) as an internal hash code.
    return ((DKHashCode)ref) >> 2;
}


// Description ---------------------------------------------------------------------------
DKStaticSelectorInit( Description );

static struct DKDescription DKDefaultDescription_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Description_StaticObject ),
    DKDefaultCopyDescription
};

DKTypeRef DKDefaultDescription( void )
{
    return &DKDefaultDescription_StaticObject;
}

DKStringRef DKDefaultCopyDescription( DKTypeRef ref )
{
    return DKGetClassName( ref );
}




// Meta-Class Interfaces =================================================================

// Class LifeCycle -----------------------------------------------------------------------
static void DKClassFinalize( DKTypeRef ref );

static struct DKLifeCycle DKClassLifeCycle_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_LifeCycle_StaticObject ),
    NULL,
    DKClassFinalize,
    NULL,
    NULL
};

static DKTypeRef DKClassLifeCycle( void )
{
    return &DKClassLifeCycle_StaticObject;
}


// Interface LifeCycle -------------------------------------------------------------------
static void DKInterfaceFinalize( DKTypeRef ref );

static struct DKLifeCycle DKInterfaceLifeCycle_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_LifeCycle_StaticObject ),
    NULL,
    DKInterfaceFinalize,
    NULL,
    NULL
};

static DKTypeRef DKInterfaceLifeCycle( void )
{
    return &DKInterfaceLifeCycle_StaticObject;
}


// Selector Comparison -------------------------------------------------------------------
static int          DKSelectorEqual( DKTypeRef ref, DKTypeRef other );
static int          DKSelectorCompare( DKTypeRef ref, DKTypeRef other );
#define             DKSelectorHash( ref )           DKDefaultHash( ref )
#define             DKFastSelectorEqual( a, b )     (a == b)

static struct DKComparison DKSelectorComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKSelectorEqual,
    DKSelectorCompare,
    DKDefaultHash
};

static DKTypeRef DKSelectorComparison( void )
{
    return &DKSelectorComparison_StaticObject;
}

static int DKSelectorEqual( DKTypeRef ref, DKTypeRef other )
{
    #if DK_RUNTIME_INTEGRITY_CHECKS
    // Do stuff here
    #endif

    return ref == other;
}

static int DKSelectorCompare( DKTypeRef ref, DKTypeRef other )
{
    #if DK_RUNTIME_INTEGRITY_CHECKS
    // Do stuff here
    #endif

    if( ref < other )
        return 1;
    
    if( ref > other )
        return -1;
    
    return 0;
}


// Interface Comparison ------------------------------------------------------------------
static int          DKInterfaceEqual( DKTypeRef ref, DKTypeRef other );
static int          DKInterfaceCompare( DKTypeRef ref, DKTypeRef other );
static DKHashCode   DKInterfaceHash( DKTypeRef ref );

static struct DKComparison DKInterfaceComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKInterfaceEqual,
    DKInterfaceCompare,
    DKInterfaceHash
};

static DKTypeRef DKInterfaceComparison( void )
{
    return &DKInterfaceComparison_StaticObject;
}

static int DKInterfaceEqual( DKTypeRef ref, DKTypeRef other )
{
    const DKInterface * thisInterface = ref;
    const DKInterface * otherInterface = other;

    return DKSelectorEqual( thisInterface->sel, otherInterface->sel );
}

static int DKInterfaceCompare( DKTypeRef ref, DKTypeRef other )
{
    const DKInterface * thisInterface = ref;
    const DKInterface * otherInterface = other;

    return DKSelectorCompare( thisInterface->sel, otherInterface->sel );
}

static DKHashCode DKInterfaceHash( DKTypeRef ref )
{
    const DKInterface * interface = ref;
    
    return DKSelectorHash( interface->sel );
}




// Runtime Init ==========================================================================

static DKSpinLock DKRuntimeInitLock = DKSpinLockInit;
static int32_t DKRuntimeInitialized = 0;

///
//  InitRootClass()
//
static void InitRootClass( struct DKClass * _class, struct DKClass * isa, struct DKClass * superclass, size_t structSize )
{
    memset( _class, 0, sizeof(struct DKClass) );
    
    struct DKObjectHeader * header = (struct DKObjectHeader *)_class;
    header->isa = DKRetain( isa );
    header->refcount = 1;

    _class->name = NULL;
    _class->superclass = DKRetain( superclass );
    _class->structSize = structSize;
    _class->lock = DKSpinLockInit;
    
    DKPointerArrayInit( &_class->interfaces );
    DKPointerArrayInit( &_class->properties );
}


///
//  InstallRootClassInterface()
//
static void InstallRootClassInterface( struct DKClass * _class, DKInterface * interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.

    if( interface->sel->vidx )
        _class->vtable[interface->sel->vidx] = interface;
    
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

        InitRootClass( &__DKMetaClass__,       &__DKMetaClass__, NULL,                  sizeof(DKClass)        );
        InitRootClass( &__DKClassClass__,      &__DKMetaClass__, NULL,                  sizeof(DKClass)        );
        InitRootClass( &__DKSelectorClass__,   &__DKMetaClass__, NULL,                  sizeof(struct DKSEL)   );
        InitRootClass( &__DKInterfaceClass__,  &__DKMetaClass__, NULL,                  sizeof(DKInterface)    );
        InitRootClass( &__DKMsgHandlerClass__, &__DKMetaClass__, &__DKInterfaceClass__, sizeof(DKMsgHandler)   );
        InitRootClass( &__DKPropertyClass__,   &__DKMetaClass__, NULL,                  0                      );
        InitRootClass( &__DKObjectClass__,     &__DKMetaClass__, NULL,                  sizeof(DKObjectHeader) );
        InitRootClass( &__DKWeakClass__,       &__DKMetaClass__, NULL,                  sizeof(DKWeak)         );
        
        InstallRootClassInterface( &__DKMetaClass__, DKClassLifeCycle() );
        InstallRootClassInterface( &__DKMetaClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKMetaClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKClassClass__, DKClassLifeCycle() );
        InstallRootClassInterface( &__DKClassClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKClassClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKSelectorClass__, DKDefaultLifeCycle() );
        InstallRootClassInterface( &__DKSelectorClass__, DKSelectorComparison() );
        InstallRootClassInterface( &__DKSelectorClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKInterfaceClass__, DKInterfaceLifeCycle() );
        InstallRootClassInterface( &__DKInterfaceClass__, DKInterfaceComparison() );
        InstallRootClassInterface( &__DKInterfaceClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKMsgHandlerClass__, DKDefaultLifeCycle() );
        InstallRootClassInterface( &__DKMsgHandlerClass__, DKInterfaceComparison() );
        InstallRootClassInterface( &__DKMsgHandlerClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKPropertyClass__, DKDefaultLifeCycle() );
        InstallRootClassInterface( &__DKPropertyClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKPropertyClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKObjectClass__, DKDefaultLifeCycle() );
        InstallRootClassInterface( &__DKObjectClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKObjectClass__, DKDefaultDescription() );

        InstallRootClassInterface( &__DKWeakClass__, DKDefaultLifeCycle() );
        InstallRootClassInterface( &__DKWeakClass__, DKDefaultComparison() );
        InstallRootClassInterface( &__DKWeakClass__, DKDefaultDescription() );

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
//  DKInitializeObject()
//
static struct DKObjectHeader * DKInitializeObject( struct DKObjectHeader * obj, const struct DKClass * cls )
{
    if( cls->superclass )
    {
        obj = DKInitializeObject( obj, cls->superclass );
    }
    
    if( obj )
    {
        DKLifeCycle * lifeCycle = DKGetInterface( cls, DKSelector(LifeCycle) );
        
        if( lifeCycle->initialize )
            obj = (struct DKObjectHeader *)lifeCycle->initialize( obj );
    }
    
    return obj;
}


///
//  DKAllocObject()
//
void * DKAllocObject( DKTypeRef isa, size_t extraBytes )
{
    if( !isa )
    {
        DKFatalError( "DKAlloc: Specified class object 'isa' is NULL." );
        return NULL;
    }
    
    const struct DKClass * cls = isa;
    
    if( cls->structSize < sizeof(struct DKObjectHeader) )
    {
        DKFatalError( "DKAlloc: Requested struct size is smaller than DKObjectHeader." );
        return NULL;
    }
    
    // Allocate the structure + extra bytes
    DKLifeCycle * lifeCycle = DKGetInterface( isa, DKSelector(LifeCycle) );

    struct DKObjectHeader * obj = NULL;

    if( lifeCycle->alloc )
        obj = lifeCycle->alloc( cls->structSize + extraBytes );
    
    else
        obj = dk_malloc( cls->structSize + extraBytes );
    
    // Zero the structure bytes
    memset( obj, 0, cls->structSize );
    
    // Setup the object header
    obj->isa = DKRetain( isa );
    obj->weakref = NULL;
    obj->refcount = 1;
    
    // Call the initializer chain
    obj = DKInitializeObject( obj, cls );
    
    return obj;
}


///
//  DKFinalizeObject()
//
static void DKFinalizeObject( struct DKObjectHeader * obj )
{
    for( const struct DKClass * cls = obj->isa; cls != NULL; cls = cls->superclass )
    {
        DKLifeCycle * lifeCycle = DKGetInterface( cls, DKSelector(LifeCycle) );
        
        if( lifeCycle->finalize )
            lifeCycle->finalize( obj );
    }
}


///
//  DKDeallocObject()
//
void DKDeallocObject( DKTypeRef ref )
{
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    
    DKAssert( obj );
    DKAssert( obj->refcount == 0 );
    DKAssert( obj->weakref == NULL );

    const struct DKClass * isa = obj->isa;

    // Call the finalizer chain
    DKFinalizeObject( obj );
    
    // Deallocate
    DKLifeCycle * lifeCycle = DKGetInterface( isa, DKSelector(LifeCycle) );
    
    if( lifeCycle->free )
        lifeCycle->free( obj );
    
    else
        dk_free( obj );
    
    // Finally release the class object
    DKRelease( isa );
}


///
//  DKAllocClass()
//
void * DKAllocClass( DKStringRef name, DKTypeRef superclass, size_t structSize )
{
    struct DKClass * cls = (struct DKClass *)DKAllocObject( DKClassClass(), 0 );

    cls->name = DKRetain( name );
    cls->superclass = DKRetain( superclass );
    cls->structSize = structSize;
    
    DKPointerArrayInit( &cls->interfaces );
    DKPointerArrayInit( &cls->properties );

    memset( cls->vtable, 0, sizeof(cls->vtable) );
    
    // Install a default life-cycle interface so lookups on this class
    // always resolve to something
    DKInstallInterface( cls, DKDefaultLifeCycle() );
    
    return cls;
}


///
//  DKClassFinalize()
//
static void DKClassFinalize( DKTypeRef ref )
{
    struct DKClass * cls = (struct DKClass *)ref;
    
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
static void DKInterfaceFinalize( DKTypeRef ref )
{
    DKInterface * interface = ref;
    DKRelease( interface->sel );
}


///
//  DKInstallInterface()
//
void DKInstallInterface( DKTypeRef _class, DKTypeRef interface )
{
    DKVerifyMemberOfClass( _class, DKClassClass() );
    DKVerifyKindOfClass( interface, DKInterfaceClass() );

    struct DKClass * cls = (struct DKClass *)_class;
    DKInterface * interfaceObject = interface;

    // Retain the interface
    DKRetain( interfaceObject );

    // Update the fast-lookup table
    int vtableIndex = interfaceObject->sel->vidx;
    DKAssert( (vtableIndex >= 0) && (vtableIndex < DKVTableSize) );

    if( vtableIndex )
    {
        // If we're replacing a fast-lookup entry, make sure the selectors match
        DKInterface * oldInterface = cls->vtable[vtableIndex];

        if( (oldInterface != NULL) && !DKEqual( interface, oldInterface ) )
        {
            DKRetain( interfaceObject );

            // This likely means that two interfaces are trying to use the same fast lookup index
            DKFatalError( "DKInstallInterface: Selector '%s' doesn't match the current vtable entry '%s'.\n",
                interfaceObject->sel->suid, oldInterface->sel->suid );
            
            return;
        }
    
        cls->vtable[vtableIndex] = interface;
    }

    // Invalidate the interface cache
    // Do stuff here...

    // Replace the interface in the interface table
    DKIndex count = cls->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        const DKInterface * oldInterfaceObject = cls->interfaces.data[i];
        
        if( DKEqual( oldInterfaceObject->sel, interfaceObject->sel ) )
        {
            DKRelease( oldInterfaceObject );
            cls->interfaces.data[i] = interfaceObject;
            return;
        }
    }
    
    // Add the interface to the interface table
    DKPointerArrayAppendPointer( &cls->interfaces, interfaceObject );
}


///
//  DKInstallMsgHandler()
//
void DKInstallMsgHandler( DKTypeRef _class, DKSEL sel, const void * func )
{
    struct DKMsgHandler * msgHandler = (struct DKMsgHandler *)DKAllocObject( DKMsgHandlerClass(), sizeof(DKMsgHandler) );

    msgHandler->sel = DKRetain( sel );
    msgHandler->func = func;
    
    DKInstallInterface( _class, msgHandler );
    
    DKRelease( msgHandler );
}


///
//  DKLookupInterface()
//
static DKInterface * DKSearchForInterface( const struct DKClass * cls, DKSEL sel )
{
    // Search our interface table for the selector
    DKIndex count = cls->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * interface = cls->interfaces.data[i];
        
        // DKEqual( interface->sel, sel );
        if( DKFastSelectorEqual( interface->sel, sel ) )
            return interface;
    }
    
    // Try to lookup the interface in our superclass
    if( cls->superclass )
        return DKLookupInterface( cls->superclass, sel );
    
    return NULL;
}

static DKInterface * DKLookupInterface( const struct DKClass * cls, DKSEL sel )
{
    // First check the fast lookup table
    int vtableIndex = sel->vidx;
    DKAssert( (vtableIndex >= 0) && (vtableIndex < DKVTableSize) );
    
    DKInterface * interface = cls->vtable[vtableIndex];
    
    if( interface )
        return interface;
    
    // Next check the interface cache
    // Do stuff here...
    
    // Search the interface tables
    interface = DKSearchForInterface( cls, sel );

    // Update the caches
    if( interface )
    {
        struct DKClass * _cls = (struct DKClass *)cls;
        
        // Update the vtable
        if( vtableIndex > 0 )
        {
            _cls->vtable[vtableIndex] = interface;
        }

        // Update the interface cache
        // Do stuff here...
    }

    return interface;
}


///
//  DKHasInterface()
//
int DKHasInterface( DKTypeRef ref, DKSEL sel )
{
    if( ref )
    {
        DKObjectHeader * obj = ref;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;
        
        DKInterface * interface = DKLookupInterface( cls, sel );

        return interface != NULL;
    }
    
    return 0;
}


///
//  DKGetInterface()
//
DKTypeRef DKGetInterface( DKTypeRef ref, DKSEL sel )
{
    if( ref )
    {
        DKObjectHeader * obj = ref;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;
        
        DKInterface * interface = DKLookupInterface( cls, sel );
        
        if( interface )
            return interface;

        DKFatalError( "DKRuntime: Interface '%s' not found on object '%s'\n", sel->suid, cls->name );
    }

    return DKInterfaceNotFound();
}


///
//  DKHasMsgHandler()
//
int DKHasMsgHandler( DKTypeRef ref, DKSEL sel )
{
    if( ref )
    {
        DKObjectHeader * obj = ref;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;

        DKMsgHandler * msgHandler = (DKMsgHandler *)DKLookupInterface( cls, sel );

        if( msgHandler )
        {
            DKVerifyKindOfClass( msgHandler, DKMsgHandlerClass(), 0 );
            return 1;
        }
    }

    return 0;
}


///
//  DKGetMsgHandler()
//
DKTypeRef DKGetMsgHandler( DKTypeRef ref, DKSEL sel )
{
    if( ref )
    {
        DKObjectHeader * obj = ref;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;

        DKMsgHandler * msgHandler = (DKMsgHandler *)DKLookupInterface( cls, sel );

        if( msgHandler )
        {
            DKVerifyKindOfClass( msgHandler, DKMsgHandlerClass(), DKMsgHandlerNotFound() );
            return msgHandler;
        }

        DKWarning( "DKRuntime: Message handler for '%s' not found on object '%s'\n", sel->suid, obj->isa->name );
    }

    return DKMsgHandlerNotFound();
}




// Reference Counting ====================================================================

DKTypeRef DKRetain( DKTypeRef ref )
{
    if( ref )
    {
        struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
        DKAtomicIncrement32( &obj->refcount );
    }

    return ref;
}

void DKRelease( DKTypeRef ref )
{
    if( ref )
    {
        struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
        struct DKWeak * weakref = (struct DKWeak *)obj->weakref;
        
        int32_t n;
        
        if( weakref )
        {
            DKSpinLockLock( &weakref->lock );
            
            n = DKAtomicDecrement32( &obj->refcount );

            if( n == 0 )
            {
                obj->weakref = NULL;
                weakref->target = NULL;
            }
            
            DKSpinLockUnlock( &weakref->lock );
        }
        
        else
        {
            n = DKAtomicDecrement32( &obj->refcount );
        }
        
        DKAssert( n >= 0 );
        
        if( n == 0 )
        {
            DKRelease( weakref );
            DKDeallocObject( ref );
        }
    }
}


///
//  DKRetainWeak()
//
DKWeakRef DKRetainWeak( DKTypeRef ref )
{
    if( ref )
    {
        struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
        
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
DKTypeRef DKResolveWeak( DKWeakRef weak_ref )
{
    if( weak_ref )
    {
        struct DKWeak * weakref = (struct DKWeak *)weak_ref;
        
        if( weakref->target )
        {
            DKSpinLockLock( &weakref->lock );
            
            DKTypeRef target = DKRetain( weakref->target );
            
            DKSpinLockUnlock( &weakref->lock );
            
            return target;
        }
    }
    
    return NULL;
}




// Polymorphic Wrappers ==================================================================

///
//  DKCreate()
//
DKTypeRef DKCreate( DKTypeRef _class )
{
    if( _class )
    {
        return DKAllocObject( _class, 0 );
    }
    
    return NULL;
}


///
//  DKGetClass()
//
DKTypeRef DKGetClass( DKTypeRef ref )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        return obj->isa;
    }
    
    return NULL;
}


///
//  DKGetClassName()
//
DKStringRef DKGetClassName( DKTypeRef ref )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;
        
        return cls->name;
    }
    
    return DKSTR( "null" );
}


///
//  DKGetSuperclass()
//
DKTypeRef DKGetSuperclass( DKTypeRef ref )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        return obj->isa->superclass;
    }
    
    return NULL;
}


///
//  DKIsMemberOfClass()
//
int DKIsMemberOfClass( DKTypeRef ref, DKTypeRef _class )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        return obj->isa == _class;
    }
    
    return 0;
}


///
//  DKIsKindOfClass()
//
int DKIsKindOfClass( DKTypeRef ref, DKTypeRef _class )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        
        for( const struct DKClass * cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls == _class )
                return 1;
        }
    }
    
    return 0;
}


///
//  DKEqual()
//
int DKEqual( DKTypeRef a, DKTypeRef b )
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
int DKCompare( DKTypeRef a, DKTypeRef b )
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
DKHashCode DKHash( DKTypeRef ref )
{
    if( ref )
    {
        DKComparison * comparison = DKGetInterface( ref, DKSelector(Comparison) );
        return comparison->hash( ref );
    }
    
    return 0;
}


///
//  DKCopyDescription()
//
DKStringRef DKCopyDescription( DKTypeRef ref )
{
    if( ref )
    {
        DKDescription * description = DKGetInterface( ref, DKSelector(Description) );
        return description->copyDescription( ref );
    }
    
    return DKSTR( "null" );
}



