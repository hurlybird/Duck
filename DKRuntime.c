//
//  DKRuntime.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-20.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKEnv.h"
#include "DKPointerArray.h"
#include "DKRuntime.h"


static void DKRuntimeInit( void );
static DKInterface * DKLookupInterface( const struct DKClass * cls, DKSEL sel );


// Internal Class Structure ==============================================================
#define MAX_CLASS_NAME_LENGTH   40

struct DKClass
{
    const DKObjectHeader    _obj;
    
    char                    name[MAX_CLASS_NAME_LENGTH];

    const struct DKClass *  superclass;
    size_t                  structSize;

    DKTypeRef               vtable[DKVTableSize];
    
    DKPointerArray          interfaces;
    DKPointerArray          properties;
};

typedef const struct DKClass DKClass;




// Root Classes ==========================================================================

static struct DKClass __DKMetaClass__;
static struct DKClass __DKClassClass__;
       struct DKClass __DKSelectorClass__; // Selectors are statically initialized
static struct DKClass __DKInterfaceClass__;
static struct DKClass __DKMethodClass__;
static struct DKClass __DKPropertyClass__;
static struct DKClass __DKObjectClass__;


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

DKTypeRef DKMethodClass( void )
{
    DKRuntimeInit();
    return &__DKMethodClass__;
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




// Default Interfaces ====================================================================

#define DKStaticInterfaceObject( sel )                                                  \
    {                                                                                   \
        { &__DKInterfaceClass__, 1 },                                                   \
        DKSelector( sel )                                                               \
    }

#define DKStaticMethodObject( sel )                                                     \
    { &__DKMethodClass__, 1 },                                                          \
    DKSelector( sel )


// Not Found -----------------------------------------------------------------------------
#define DK_MAX_INTERFACE_SIZE   32

static struct DKSEL DKSelector_InterfaceNotFound =
{
    { &__DKSelectorClass__, 1 },
    "InterfaceNotFound",
    "void DKInterfaceNotFound( ??? )"
};

typedef void (*DKInterfaceNotFoundFunction)( struct DKObjectHeader * obj, DKSEL sel );

struct DKInterfaceNotFoundInterface
{
    DKInterface _interface;
    DKInterfaceNotFoundFunction func[DK_MAX_INTERFACE_SIZE];
};

static void DKInterfaceNotFound( struct DKObjectHeader * obj, DKSEL sel )
{
    // Note: 'obj' and 'sel' are for debugging only and may not be valid. Methods require
    // both, and most interface functions take an object as a first arguement, but it's
    // not actually required.

    // The only time this code is ever likely to be called is when calling an interface
    // function or method on a NULL object. We don't have the Objective-C luxury of
    // tweaking the call stack and return value in assembly for such cases.

    DKFatalError( "DKRuntime: Invalid interface/method call.\n" );
}

static struct DKInterfaceNotFoundInterface __DKInterfaceNotFound__ =
{
    DKStaticInterfaceObject( InterfaceNotFound ),
    // Function pointers initialized in DKRuntimeInit()
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


// ReferenceCounting ---------------------------------------------------------------------
DKDefineFastLookupInterface( ReferenceCounting );

static DKReferenceCounting __DKDefaultReferenceCounting__ =
{
    DKStaticInterfaceObject( ReferenceCounting ),
    DKDefaultRetain,
    DKDefaultRelease
};

DKTypeRef DKDefaultRetain( DKTypeRef ref )
{
    if( ref )
    {
        struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
        DKAtomicIncrement32( &obj->refcount );
    }

    return ref;
}

void DKDefaultRelease( DKTypeRef ref )
{
    if( ref )
    {
        struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
        int32_t n = DKAtomicDecrement32( &obj->refcount );
        
        DKAssert( n >= 0 );
        
        if( n == 0 )
        {
            DKDeallocObject( ref );
        }
    }
}

DKReferenceCounting * DKDefaultReferenceCounting( void )
{
    return &__DKDefaultReferenceCounting__;
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
    DKAssert( sizeof(DKHashCode) == sizeof(DKTypeRef) );
    return ((DKHashCode)ref) >> 2;
}

DKComparison * DKDefaultComparison( void )
{
    return &__DKDefaultComparison__;
}




// Meta-Class Interfaces =================================================================
static void         DKClassFinalize( DKTypeRef ref );
static void         DKInterfaceFinalize( DKTypeRef ref );

static DKTypeRef    DKStaticObjectRetain( DKTypeRef ref );
static void         DKStaticObjectRelease( DKTypeRef ref );

static int          DKInterfaceEqual( DKTypeRef ref, DKTypeRef other );
static int          DKInterfaceCompare( DKTypeRef ref, DKTypeRef other );
static DKHashCode  DKInterfaceHash( DKTypeRef ref );


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

static DKReferenceCounting __DKStaticObjectReferenceCounting__ =
{
    DKStaticInterfaceObject( ReferenceCounting ),
    DKStaticObjectRetain,
    DKStaticObjectRelease
};

static DKComparison __DKInterfaceComparison__ =
{
    DKStaticInterfaceObject( Comparison ),
    DKInterfaceEqual,
    DKInterfaceCompare,
    DKInterfaceHash
};


// Static Object Reference Counting
static DKTypeRef DKStaticObjectRetain( DKTypeRef ref )
{
    return ref;
}

static void DKStaticObjectRelease( DKTypeRef ref )
{
}

// Interface Comparison
static int DKInterfaceEqual( DKTypeRef ref, DKTypeRef other )
{
    const DKInterface * thisInterface = ref;
    const DKInterface * otherInterface = other;

    return DKDefaultEqual( thisInterface->sel, otherInterface->sel );
}

static int DKInterfaceCompare( DKTypeRef ref, DKTypeRef other )
{
    const DKInterface * thisInterface = ref;
    const DKInterface * otherInterface = other;

    return DKDefaultCompare( thisInterface->sel, otherInterface->sel );
}

static DKHashCode DKInterfaceHash( DKTypeRef ref )
{
    const DKInterface * interface = ref;
    
    return DKDefaultHash( interface->sel );
}




// Runtime Init ==========================================================================

static volatile int32_t DKRuntimeInitialized = 0;

///
//  InitRootClass()
//
static void InitRootClass( struct DKClass * metaclass, const char * name, struct DKClass * isa,
    struct DKClass * superclass, size_t structSize, DKLifeCycle * lifeCycle,
    DKReferenceCounting * referenceCounting, DKComparison * comparison )
{
    memset( metaclass, 0, sizeof(struct DKClass) );
    
    // NOTE: We don't retain the 'isa' or 'superclass' objects here since the reference
    // counting interfaces haven't been installed yet. It doesn't really matter because
    // the meta-classes are statically allocated anyway.

    struct DKObjectHeader * header = (struct DKObjectHeader *)metaclass;
    header->isa = isa;
    header->refcount = 1;

    strncpy( metaclass->name, name, MAX_CLASS_NAME_LENGTH - 1 );

    metaclass->superclass = superclass;
    metaclass->structSize = structSize;
    
    DKPointerArrayInit( &metaclass->interfaces );
    DKPointerArrayInit( &metaclass->properties );

    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    metaclass->vtable[DKVTable_LifeCycle] = lifeCycle;
    DKPointerArrayAppendPointer( &metaclass->interfaces, lifeCycle );
    
    metaclass->vtable[DKVTable_ReferenceCounting] = referenceCounting;
    DKPointerArrayAppendPointer( &metaclass->interfaces, referenceCounting );
    
    metaclass->vtable[DKVTable_Comparison] = comparison;
    DKPointerArrayAppendPointer( &metaclass->interfaces, comparison );
}

///
//  DKRuntimeInit()
//
static void DKRuntimeInit( void )
{
    // *** SPIN LOCK HERE ***
    
    if( !DKRuntimeInitialized )
    {
        DKRuntimeInitialized = 1;

        // Init error handling
        for( int i = 0; i < DK_MAX_INTERFACE_SIZE; i++ )
            __DKInterfaceNotFound__.func[i] = DKInterfaceNotFound;

        // Init root classes
        InitRootClass( &__DKMetaClass__,      "Root",      &__DKMetaClass__, NULL,                  sizeof(struct DKClass), &__DKClassLifeCycle__,     &__DKStaticObjectReferenceCounting__, &__DKDefaultComparison__ );
        InitRootClass( &__DKClassClass__,     "Class",     &__DKMetaClass__, NULL,                  sizeof(struct DKClass), &__DKClassLifeCycle__,     &__DKDefaultReferenceCounting__,      &__DKDefaultComparison__ );
        InitRootClass( &__DKSelectorClass__,  "Selector",  &__DKMetaClass__, NULL,                  sizeof(struct DKSEL),   &__DKDefaultLifeCycle__,   &__DKStaticObjectReferenceCounting__, &__DKDefaultComparison__ );
        InitRootClass( &__DKInterfaceClass__, "Interface", &__DKMetaClass__, NULL,                  sizeof(DKInterface),    &__DKInterfaceLifeCycle__, &__DKDefaultReferenceCounting__,      &__DKInterfaceComparison__ );
        InitRootClass( &__DKMethodClass__,    "Method",    &__DKMetaClass__, &__DKInterfaceClass__, sizeof(DKMethod),       &__DKInterfaceLifeCycle__, &__DKDefaultReferenceCounting__,      &__DKInterfaceComparison__ );
        InitRootClass( &__DKPropertyClass__,  "Property",  &__DKMetaClass__, NULL,                  0,                      &__DKDefaultLifeCycle__,   &__DKDefaultReferenceCounting__,      &__DKDefaultComparison__ );
        InitRootClass( &__DKObjectClass__,    "Object",    &__DKMetaClass__, NULL,                  sizeof(DKObjectHeader), &__DKDefaultLifeCycle__,   &__DKDefaultReferenceCounting__,      &__DKDefaultComparison__ );
    }
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

    DKAssert( (cls->_obj.isa == &__DKClassClass__) || (cls->_obj.isa == &__DKMetaClass__) );
    DKAssert( (interfaceObject->_obj.isa == &__DKInterfaceClass__) || (interfaceObject->_obj.isa == &__DKMethodClass__) );

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
//  DKInstallMethod()
//
void DKInstallMethod( DKTypeRef _class, DKSEL sel, const void * imp )
{
    struct DKMethod * method = (struct DKMethod *)DKAllocObject( DKMethodClass(), sizeof(DKMethod) );

    method->sel = sel;
    method->imp = imp;
    
    DKInstallInterface( _class, method );
    
    DKRelease( method );
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
        if( interface->sel == sel )
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
//  DKHasMethod()
//
int DKHasMethod( DKTypeRef ref, DKSEL sel )
{
    if( ref )
    {
        DKObjectHeader * obj = ref;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;

        DKMethod * method = (DKMethod *)DKLookupInterface( cls, sel );

        if( method )
        {
            DKVerifyKindOfClass( method, DKMethodClass(), 0 );
            return 1;
        }
    }

    return 0;
}


///
//  DKGetMethod()
//
DKTypeRef DKGetMethod( DKTypeRef ref, DKSEL sel )
{
    if( ref )
    {
        DKObjectHeader * obj = ref;
        struct DKClass * cls = (struct DKClass *)obj->isa;
        
        // If this object is a class, look in its own interfaces
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = (struct DKClass *)ref;

        DKMethod * method = (DKMethod *)DKLookupInterface( cls, sel );

        if( method )
        {
            DKVerifyKindOfClass( method, DKMethodClass(), &__DKInterfaceNotFound__ );
            return method;
        }

        DKFatalError( "DKRuntime: Method '%s' not found on object '%s'\n", sel->name, obj->isa->name );
    }

    return &__DKInterfaceNotFound__;
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


const char * DKGetClassName( DKTypeRef ref )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        const struct DKClass * cls = obj->isa;
        
        if( (cls == &__DKClassClass__) || (cls == &__DKMetaClass__) )
            cls = ref;
        
        return cls->name;
    }
    
    return "";
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
//  DKRetain()
//
DKTypeRef DKRetain( DKTypeRef ref )
{
    if( ref )
    {
        DKReferenceCounting * referenceCounting = DKGetInterface( ref, DKSelector(ReferenceCounting) );
        return referenceCounting->retain( ref );
    }

    return ref;
}


///
//  DKRelease()
//
void DKRelease( DKTypeRef ref )
{
    if( ref )
    {
        DKReferenceCounting * referenceCounting = DKGetInterface( ref, DKSelector(ReferenceCounting) );
        referenceCounting->release( ref );
    }
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



