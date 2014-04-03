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
    DKObjectHeader  _obj;
    
    DKStringRef     name;
    DKClassRef      superclass;
    size_t          structSize;

    DKSpinLock      lock;

    DKInterface *   vtable[DKVTableSize];
    
    DKPointerArray  interfaces;
    DKPointerArray  properties;
};


// DKWeak
struct DKWeak
{
    DKObjectHeader  _obj;
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
static struct DKClass __DKObjectClass__;
static struct DKClass __DKWeakClass__;


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

DKClassRef DKObjectClass( void )
{
    DKRuntimeInit();
    return &__DKObjectClass__;
}

static DKClassRef DKWeakClass( void )
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
DKStaticFastSelectorInit( LifeCycle );

static struct DKLifeCycle DKDefaultLifeCycle_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_LifeCycle_StaticObject ),
    NULL,
    NULL,
    NULL,
    NULL
};

DKInterfaceRef DKDefaultLifeCycle( void )
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

DKInterfaceRef DKDefaultComparison( void )
{
    return &DKDefaultComparison_StaticObject;
}

int DKDefaultEqual( DKObjectRef ref, DKObjectRef other )
{
    return ref == other;
}

int DKDefaultCompare( DKObjectRef ref, DKObjectRef other )
{
    if( ref < other )
        return 1;
    
    if( ref > other )
        return -1;
    
    return 0;
}

DKHashCode DKDefaultHash( DKObjectRef ref )
{
    // Just in case someone changes the size of DKHashCode
    DKAssert( sizeof(DKHashCode) == sizeof(DKObjectRef) );

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

DKInterfaceRef DKDefaultDescription( void )
{
    return &DKDefaultDescription_StaticObject;
}

DKStringRef DKDefaultCopyDescription( DKObjectRef ref )
{
    return DKGetClassName( ref );
}




// Meta-Class Interfaces =================================================================

// Class LifeCycle -----------------------------------------------------------------------
static void DKClassFinalize( DKObjectRef ref );

static struct DKLifeCycle DKClassLifeCycle_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_LifeCycle_StaticObject ),
    NULL,
    DKClassFinalize,
    NULL,
    NULL
};

static DKInterfaceRef DKClassLifeCycle( void )
{
    return &DKClassLifeCycle_StaticObject;
}


// Interface LifeCycle -------------------------------------------------------------------
static void DKInterfaceFinalize( DKObjectRef ref );

static struct DKLifeCycle DKInterfaceLifeCycle_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_LifeCycle_StaticObject ),
    NULL,
    DKInterfaceFinalize,
    NULL,
    NULL
};

static DKInterfaceRef DKInterfaceLifeCycle( void )
{
    return &DKInterfaceLifeCycle_StaticObject;
}


// Selector Comparison -------------------------------------------------------------------
static int          DKSelectorEqual( DKSEL a, DKSEL b );
static int          DKSelectorCompare( DKSEL a, DKSEL b );
#define             DKSelectorHash( ref )           DKDefaultHash( ref )
#define             DKFastSelectorEqual( a, b )     (a == b)

static struct DKComparison DKSelectorComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    (DKEqualMethod)DKSelectorEqual,
    (DKCompareMethod)DKSelectorCompare,
    DKDefaultHash
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

        InitRootClass( &__DKMetaClass__,       &__DKMetaClass__, NULL,                  sizeof(struct DKClass)      );
        InitRootClass( &__DKClassClass__,      &__DKMetaClass__, NULL,                  sizeof(struct DKClass)      );
        InitRootClass( &__DKSelectorClass__,   &__DKMetaClass__, NULL,                  sizeof(struct DKSEL)        );
        InitRootClass( &__DKInterfaceClass__,  &__DKMetaClass__, NULL,                  sizeof(DKInterface)         );
        InitRootClass( &__DKMsgHandlerClass__, &__DKMetaClass__, &__DKInterfaceClass__, sizeof(struct DKMsgHandler) );
        InitRootClass( &__DKPropertyClass__,   &__DKMetaClass__, NULL,                  0                           );
        InitRootClass( &__DKObjectClass__,     &__DKMetaClass__, NULL,                  sizeof(DKObjectHeader)      );
        InitRootClass( &__DKWeakClass__,       &__DKMetaClass__, NULL,                  sizeof(struct DKWeak)       );
        
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
void * DKAllocObject( DKClassRef cls, size_t extraBytes )
{
    if( !cls )
    {
        DKFatalError( "DKAlloc: Specified class object 'isa' is NULL." );
        return NULL;
    }
    
    if( cls->structSize < sizeof(struct DKObjectHeader) )
    {
        DKFatalError( "DKAlloc: Requested struct size is smaller than DKObjectHeader." );
        return NULL;
    }
    
    // Allocate the structure + extra bytes
    DKLifeCycle * lifeCycle = DKGetInterface( cls, DKSelector(LifeCycle) );

    struct DKObjectHeader * obj = NULL;

    if( lifeCycle->alloc )
        obj = lifeCycle->alloc( cls->structSize + extraBytes );
    
    else
        obj = dk_malloc( cls->structSize + extraBytes );
    
    // Zero the structure bytes
    memset( obj, 0, cls->structSize );
    
    // Setup the object header
    obj->isa = DKRetain( cls );
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
    for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
    {
        DKLifeCycle * lifeCycle = DKGetInterface( cls, DKSelector(LifeCycle) );
        
        if( lifeCycle->finalize )
            lifeCycle->finalize( obj );
    }
}


///
//  DKDeallocObject()
//
void DKDeallocObject( DKObjectRef ref )
{
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    
    DKAssert( obj );
    DKAssert( obj->refcount == 0 );
    DKAssert( obj->weakref == NULL );

    DKClassRef cls = obj->isa;

    // Call the finalizer chain
    DKFinalizeObject( obj );
    
    // Deallocate
    DKLifeCycle * lifeCycle = DKGetInterface( cls, DKSelector(LifeCycle) );
    
    if( lifeCycle->free )
        lifeCycle->free( obj );
    
    else
        dk_free( obj );
    
    // Finally release the class object
    DKRelease( cls );
}


///
//  DKAllocClass()
//
DKClassRef DKAllocClass( DKStringRef name, DKClassRef superclass, size_t structSize )
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
static void DKClassFinalize( DKObjectRef ref )
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
static void DKInterfaceFinalize( DKObjectRef ref )
{
    DKInterface * interface = ref;
    DKRelease( interface->sel );
}


///
//  DKInstallInterface()
//
void DKInstallInterface( DKClassRef _class, DKInterfaceRef interface )
{
    DKAssertMemberOfClass( _class, DKClassClass() );
    DKAssertKindOfClass( interface, DKInterfaceClass() );

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
void DKInstallMsgHandler( DKClassRef _class, DKSEL sel, const void * func )
{
    struct DKMsgHandler * msgHandler = (struct DKMsgHandler *)DKAllocObject( DKMsgHandlerClass(), sizeof(struct DKMsgHandler) );

    msgHandler->sel = DKRetain( sel );
    msgHandler->func = func;
    
    DKInstallInterface( _class, msgHandler );
    
    DKRelease( msgHandler );
}


///
//  DKLookupInterface()
//
static DKInterface * DKSearchForInterface( DKClassRef cls, DKSEL sel )
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

static DKInterface * DKLookupInterface( DKClassRef cls, DKSEL sel )
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
int DKHasInterface( DKObjectRef ref, DKSEL sel )
{
    if( ref )
    {
        DKObjectHeader * obj = ref;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = ref;
        
        DKInterface * interface = DKLookupInterface( cls, sel );

        return interface != NULL;
    }
    
    return 0;
}


///
//  DKGetInterface()
//
DKInterfaceRef DKGetInterface( DKObjectRef ref, DKSEL sel )
{
    if( ref )
    {
        DKObjectHeader * obj = ref;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;
        
        DKInterfaceRef interface = DKLookupInterface( cls, sel );
        
        if( interface )
            return interface;

        DKFatalError( "DKRuntime: Interface '%s' not found on object '%s'\n", sel->suid, cls->name );
    }

    return DKInterfaceNotFound();
}


///
//  DKHasMsgHandler()
//
int DKHasMsgHandler( DKObjectRef ref, DKSEL sel )
{
    return DKGetMsgHandler( ref, sel ) != DKMsgHandlerNotFound();
}


///
//  DKGetMsgHandler()
//
DKMsgHandlerRef DKGetMsgHandler( DKObjectRef ref, DKSEL sel )
{
    if( ref )
    {
        DKObjectHeader * obj = ref;
        DKClassRef cls = obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;

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




// Reference Counting ====================================================================

DKObjectRef DKRetain( DKObjectRef ref )
{
    if( ref )
    {
        struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
        DKAtomicIncrement32( &obj->refcount );
    }

    return ref;
}

void DKRelease( DKObjectRef ref )
{
    if( ref )
    {
        struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
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
            DKDeallocObject( ref );
        }
    }
}


///
//  DKRetainWeak()
//
DKWeakRef DKRetainWeak( DKObjectRef ref )
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
//  DKGetClass()
//
DKClassRef DKGetClass( DKObjectRef ref )
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
DKStringRef DKGetClassName( DKObjectRef ref )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        DKClassRef cls = obj->isa;
        
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;
        
        return cls->name;
    }
    
    return DKSTR( "null" );
}


///
//  DKGetSuperclass()
//
DKClassRef DKGetSuperclass( DKObjectRef ref )
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
int DKIsMemberOfClass( DKObjectRef ref, DKClassRef _class )
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
int DKIsKindOfClass( DKObjectRef ref, DKClassRef _class )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        
        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
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
DKHashCode DKHash( DKObjectRef ref )
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
DKStringRef DKCopyDescription( DKObjectRef ref )
{
    if( ref )
    {
        DKDescription * description = DKGetInterface( ref, DKSelector(Description) );
        return description->copyDescription( ref );
    }
    
    return DKSTR( "null" );
}



