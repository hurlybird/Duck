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


static void DKRuntimeInit( void );
static DKInterface * DKLookupInterface( const struct DKClass * cls, DKSEL sel );


// Internal Types ========================================================================


// DKClass
#define MAX_CLASS_NAME_LENGTH   40

struct DKClass
{
    const DKObjectHeader    _obj;
    
    char                    name[MAX_CLASS_NAME_LENGTH];
    DKStringRef             nameString;

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
       struct DKClass __DKSelectorClass__; // Selectors are statically initialized
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

#define DKStaticInterfaceObject( sel )                                                  \
    {                                                                                   \
        DKStaticObjectHeader( &__DKInterfaceClass__ ),                                  \
        DKSelector( sel )                                                               \
    }


// Not Found -----------------------------------------------------------------------------
#define DK_MAX_INTERFACE_SIZE   32

static struct DKSEL DKSelector_InterfaceNotFound =
{
    DKStaticObjectHeader( &__DKSelectorClass__ ),
    "InterfaceNotFound",
    "InterfaceNotFound"
};

static struct DKSEL DKSelector_MsgHandlerNotFound =
{
    DKStaticObjectHeader( &__DKSelectorClass__ ),
    "MsgHandlerNotFound",
    "DKMsgHandlerHotFound( ??? )"
};

typedef void (*DKInterfaceNotFoundFunction)( struct DKObjectHeader * obj );

struct DKInterfaceNotFoundInterface
{
    DKInterface _interface;
    DKInterfaceNotFoundFunction func[DK_MAX_INTERFACE_SIZE];
};

static void DKInterfaceNotFound( struct DKObjectHeader * obj )
{
    // Note: 'obj' is for debugging only and may not be valid. Most interface functions
    // take an object as a first arguement, but it's not actually required.

    // The only time this code is ever likely to be called is when calling an interface
    // function on a NULL object. We don't have the Objective-C luxury of tweaking the
    // call stack and return value in assembly for such cases.

    DKFatalError( "DKRuntime: Invalid interface call.\n" );
}

static void DKMsgHandlerNotFound( struct DKObjectHeader * obj, DKSEL sel )
{
    // This handles silently failing when sending messages to NULL objects.
}

static struct DKInterfaceNotFoundInterface __DKInterfaceNotFound__ =
{
    DKStaticInterfaceObject( InterfaceNotFound ),
    // Function pointers initialized in DKRuntimeInit()
};

static DKMsgHandler __DKMsgHandlerNotFound__ =
{
    DKStaticObjectHeader( &__DKMsgHandlerClass__ ),
    DKSelector( MsgHandlerNotFound ),
    DKMsgHandlerNotFound
};


// LifeCycle -----------------------------------------------------------------------------
DKDefineFastLookupInterface( LifeCycle );

static DKLifeCycle __DKDefaultLifeCycle__ =
{
    DKStaticInterfaceObject( LifeCycle ),
    NULL,
    NULL,
    NULL,
    NULL
};

DKLifeCycle * DKDefaultLifeCycle( void )
{
    return &__DKDefaultLifeCycle__;
}


// Comparison ----------------------------------------------------------------------------
DKDefineFastLookupInterface( Comparison );

static DKComparison __DKDefaultComparison__ =
{
    DKStaticInterfaceObject( Comparison ),
    DKDefaultEqual,
    DKDefaultCompare,
    DKDefaultHash
};

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

DKComparison * DKDefaultComparison( void )
{
    return &__DKDefaultComparison__;
}


// Description ---------------------------------------------------------------------------
DKDefineInterface( Description );

static DKDescription __DKDefaultDescription__ =
{
    DKStaticInterfaceObject( Description ),
    DKDefaultCopyDescription,
};

DKStringRef DKDefaultCopyDescription( DKTypeRef ref )
{
    return DKGetClassName( ref );
}

DKDescription * DKDefaultDescription( void )
{
    return &__DKDefaultDescription__;
}




// Meta-Class Interfaces =================================================================
static void         DKClassFinalize( DKTypeRef ref );
static void         DKInterfaceFinalize( DKTypeRef ref );

static int          DKSelectorEqual( DKTypeRef ref, DKTypeRef other );
static int          DKSelectorCompare( DKTypeRef ref, DKTypeRef other );
#define             DKSelectorHash DKDefaultHash

static int          DKInterfaceEqual( DKTypeRef ref, DKTypeRef other );
static int          DKInterfaceCompare( DKTypeRef ref, DKTypeRef other );
static DKHashCode   DKInterfaceHash( DKTypeRef ref );


static DKLifeCycle __DKClassLifeCycle__ =
{
    DKStaticInterfaceObject( LifeCycle ),
    NULL,
    DKClassFinalize,
    NULL,
    NULL
};

static DKLifeCycle __DKInterfaceLifeCycle__ =
{
    DKStaticInterfaceObject( LifeCycle ),
    NULL,
    DKInterfaceFinalize,
    NULL,
    NULL
};

static DKComparison __DKSelectorComparison__ =
{
    DKStaticInterfaceObject( Comparison ),
    DKSelectorEqual,
    DKSelectorCompare,
    DKSelectorHash
};

static DKComparison __DKInterfaceComparison__ =
{
    DKStaticInterfaceObject( Comparison ),
    DKInterfaceEqual,
    DKInterfaceCompare,
    DKInterfaceHash
};


// Selector Comparison
#define DKFastSelectorEqual( a, b )     (a == b)

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


// Interface Comparison
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
static void InitRootClass( struct DKClass * _class, const char * name,
    struct DKClass * isa, struct DKClass * superclass, size_t structSize,
    DKLifeCycle * lifeCycle, DKComparison * comparison )
{
    memset( _class, 0, sizeof(struct DKClass) );
    
    struct DKObjectHeader * header = (struct DKObjectHeader *)_class;
    header->isa = DKRetain( isa );
    header->refcount = 1;

    strncpy( _class->name, name, MAX_CLASS_NAME_LENGTH - 1 );

    _class->nameString = NULL;
    _class->superclass = DKRetain( superclass );
    _class->structSize = structSize;
    _class->lock = DKSpinLockInit;
    
    DKPointerArrayInit( &_class->interfaces );
    DKPointerArrayInit( &_class->properties );

    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKRetain( lifeCycle );
    _class->vtable[DKVTable_LifeCycle] = lifeCycle;
    DKPointerArrayAppendPointer( &_class->interfaces, lifeCycle );
    
    DKRetain( comparison );
    _class->vtable[DKVTable_Comparison] = comparison;
    DKPointerArrayAppendPointer( &_class->interfaces, comparison );
    
    DKRetain( &__DKDefaultDescription__ );
    DKPointerArrayAppendPointer( &_class->interfaces, &__DKDefaultDescription__ );
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

        // Init error handling
        for( int i = 0; i < DK_MAX_INTERFACE_SIZE; i++ )
            __DKInterfaceNotFound__.func[i] = DKInterfaceNotFound;

        // Init root classes
        InitRootClass( &__DKMetaClass__,       "Root",       &__DKMetaClass__, NULL,                  sizeof(DKClass),        &__DKClassLifeCycle__,     &__DKDefaultComparison__ );
        InitRootClass( &__DKClassClass__,      "Class",      &__DKMetaClass__, NULL,                  sizeof(DKClass),        &__DKClassLifeCycle__,     &__DKDefaultComparison__ );
        InitRootClass( &__DKSelectorClass__,   "Selector",   &__DKMetaClass__, NULL,                  sizeof(struct DKSEL),   &__DKDefaultLifeCycle__,   &__DKSelectorComparison__ );
        InitRootClass( &__DKInterfaceClass__,  "Interface",  &__DKMetaClass__, NULL,                  sizeof(DKInterface),    &__DKInterfaceLifeCycle__, &__DKInterfaceComparison__ );
        InitRootClass( &__DKMsgHandlerClass__, "MsgHandler", &__DKMetaClass__, &__DKInterfaceClass__, sizeof(DKMsgHandler),   &__DKDefaultLifeCycle__,   &__DKInterfaceComparison__ );
        InitRootClass( &__DKPropertyClass__,   "Property",   &__DKMetaClass__, NULL,                  0,                      &__DKDefaultLifeCycle__,   &__DKDefaultComparison__ );
        InitRootClass( &__DKObjectClass__,     "Object",     &__DKMetaClass__, NULL,                  sizeof(DKObjectHeader), &__DKDefaultLifeCycle__,   &__DKDefaultComparison__ );
        InitRootClass( &__DKWeakClass__,       "Weak",       &__DKMetaClass__, NULL,                  sizeof(DKWeak),         &__DKDefaultLifeCycle__,   &__DKDefaultComparison__ );
    }

    DKSpinLockUnlock( &DKRuntimeInitLock );
}



// Alloc/Free Objects ====================================================================

///
//  DKAllocObject()
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

DKTypeRef DKAllocObject( DKTypeRef isa, size_t extraBytes )
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
//  DKDeallocObject()
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
//  DKCreateClass()
//
DKTypeRef DKCreateClass( const char * name, DKTypeRef superclass, size_t structSize )
{
    struct DKClass * cls = (struct DKClass *)DKAllocObject( DKClassClass(), 0 );

    strncpy( cls->name, name, MAX_CLASS_NAME_LENGTH - 1 );

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

static void DKClassFinalize( DKTypeRef ref )
{
    struct DKClass * cls = (struct DKClass *)ref;
    
    DKAssert( cls->_obj.isa == &__DKClassClass__ );
    
    DKRelease( cls->nameString );
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
//  DKCreateInterface()
//
DKTypeRef DKCreateInterface( DKSEL sel, size_t structSize )
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
    if( !_class )
    {
        DKError( "DKInstallInterface: Trying to install an interface on a NULL class object.\n" );
        return;
    }
    
    if( !interface )
    {
        DKError( "DKInstallInterface: Trying to install a NULL interface.\n" );
        return;
    }

    struct DKClass * cls = (struct DKClass *)_class;
    DKInterface * interfaceObject = interface;

    // *** ARE THESE ASSERTS CORRECT? ***

    DKAssert( (cls->_obj.isa == &__DKClassClass__) || (cls->_obj.isa == &__DKMetaClass__) );
    DKAssert( (interfaceObject->_obj.isa == &__DKInterfaceClass__) || (interfaceObject->_obj.isa == &__DKMsgHandlerClass__) );

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
                interfaceObject->sel->name, oldInterface->sel->name );
            
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

        DKFatalError( "DKRuntime: Interface '%s' not found on object '%s'\n", sel->name, cls->name );
    }

    return &__DKInterfaceNotFound__;
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
            DKVerifyKindOfClass( msgHandler, DKMsgHandlerClass(), &__DKMsgHandlerNotFound__ );
            return msgHandler;
        }

        DKWarning( "DKRuntime: Message handler for '%s' not found on object '%s'\n", sel->name, obj->isa->name );
    }

    return &__DKMsgHandlerNotFound__;
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
            
            if( !OSAtomicCmpAndSwapPtr( &obj->weakref, NULL, weakref ) )
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
        
        if( cls->nameString == NULL )
        {
            DKStringRef nameString = DKStringCreateWithCStringNoCopy( cls->name );
            
            DKSpinLockLock( &cls->lock );
            
            if( cls->nameString == NULL )
                cls->nameString = nameString;
            
            DKSpinLockUnlock( &cls->lock );
            
            if( cls->nameString != nameString )
                DKRelease( nameString );
        }
        
        return cls->nameString;
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



