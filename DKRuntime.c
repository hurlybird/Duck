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


// Internal Class Structure ==============================================================
#define MAX_CLASS_NAME_LENGTH   32

struct DKClass
{
    const DKObjectHeader    _obj;
    
    char                    name[MAX_CLASS_NAME_LENGTH];

    const struct DKClass *  superclass;

    DKTypeRef               fastLookupTable[DKFastLookupTableSize];
    
    DKPointerArray          interfaces;
    DKPointerArray          properties;
};

typedef const struct DKClass DKClass;




// Root Classes ==========================================================================
static int MetaClassesInitialized = 0;

static void InitMetaClasses( void );

static struct DKClass __DKMetaClass__;
static struct DKClass __DKClassClass__;
       struct DKClass __DKSelectorClass__;
static struct DKClass __DKInterfaceClass__;
static struct DKClass __DKMethodClass__;
static struct DKClass __DKPropertyClass__;
static struct DKClass __DKObjectClass__;


DKTypeRef DKClassClass( void )
{
    InitMetaClasses();
    return &__DKClassClass__;
}

DKTypeRef DKSelectorClass( void )
{
    InitMetaClasses();
    return &__DKSelectorClass__;
}

DKTypeRef DKInterfaceClass( void )
{
    InitMetaClasses();
    return &__DKInterfaceClass__;
}

DKTypeRef DKMethodClass( void )
{
    InitMetaClasses();
    return &__DKMethodClass__;
}

DKTypeRef DKPropertyClass( void )
{
    InitMetaClasses();
    return &__DKPropertyClass__;
}

DKTypeRef DKObjectClass( void )
{
    InitMetaClasses();
    return &__DKObjectClass__;
}




// Default Interfaces ====================================================================

#define DKStaticInterfaceObject( sel )                                                  \
    {                                                                                   \
        { &__DKInterfaceClass__, 1, DKObjectIsStatic },                                 \
        DKSelector( sel )                                                               \
    }


// LifeCycle -----------------------------------------------------------------------------
DKDefineFastLookupInterface( LifeCycle );

static DKLifeCycle __DKDefaultLifeCycle__ =
{
    DKStaticInterfaceObject( LifeCycle ),
    DKDefaultAllocate,
    DKDefaultInitialize,
    DKDefaultFinalize
};

DKTypeRef DKDefaultAllocate( void )
{
    DKError( "DKLifeCycle: The allocate interface is undefined." );
    return NULL;
}

DKTypeRef DKDefaultInitialize( DKTypeRef ref )
{
    return ref;
}

void DKDefaultFinalize( DKTypeRef ref )
{
}

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

DKHashIndex DKDefaultHash( DKTypeRef ref )
{
    DKAssert( sizeof(DKHashIndex) == sizeof(DKTypeRef) );
    return (DKHashIndex)ref;
}

DKComparison * DKDefaultComparison( void )
{
    return &__DKDefaultComparison__;
}




// Meta-Class Interfaces =================================================================
static void DKClassFinalize( DKTypeRef ref );
static void DKInterfaceFinalize( DKTypeRef ref );

static DKTypeRef DKStaticObjectRetain( DKTypeRef ref );
static void DKStaticObjectRelease( DKTypeRef ref );

static int DKInterfaceEqual( DKTypeRef ref, DKTypeRef other );
static int DKInterfaceCompare( DKTypeRef ref, DKTypeRef other );
static DKHashIndex DKInterfaceHash( DKTypeRef ref );


static DKLifeCycle __DKClassLifeCycle__ =
{
    DKStaticInterfaceObject( LifeCycle ),
    DKDefaultAllocate,
    DKDefaultInitialize,
    DKClassFinalize
};

static DKLifeCycle __DKInterfaceLifeCycle__ =
{
    DKStaticInterfaceObject( LifeCycle ),
    DKDefaultAllocate,
    DKDefaultInitialize,
    DKInterfaceFinalize
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

static DKHashIndex DKInterfaceHash( DKTypeRef ref )
{
    const DKInterface * interface = ref;
    
    return DKDefaultHash( interface->sel );
}




// Meta-Class Init =======================================================================

///
//  InitMetaClass()
//
static void InitMetaClass( struct DKClass * metaclass, struct DKClass * isa, struct DKClass * superclass )
{
    memset( metaclass, 0, sizeof(struct DKClass) );
    
    // NOTE: We don't retain the 'isa' or 'superclass' objects here since the reference
    // counting interfaces haven't been installed yet. It doesn't really matter because
    // the meta-classes are statically allocated anyway.

    struct DKObjectHeader * header = (struct DKObjectHeader *)metaclass;
    header->isa = isa;
    header->refcount = 1;
    header->attributes = DKObjectIsStatic;

    metaclass->superclass = superclass;

    DKPointerArrayInit( &metaclass->interfaces );
    DKPointerArrayInit( &metaclass->properties );
}


///
//  InstallMetaClassInterfaces()
//
static void InstallMetaClassInterfaces( struct DKClass * metaclass, DKLifeCycle * lifeCycle,
    DKReferenceCounting * referenceCounting, DKComparison * comparison )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    metaclass->fastLookupTable[DKFastLookupLifeCycle] = lifeCycle;
    DKPointerArrayAppendPointer( &metaclass->interfaces, lifeCycle );
    
    metaclass->fastLookupTable[DKFastLookupReferenceCounting] = referenceCounting;
    DKPointerArrayAppendPointer( &metaclass->interfaces, referenceCounting );
    
    metaclass->fastLookupTable[DKFastLookupComparison] = comparison;
    DKPointerArrayAppendPointer( &metaclass->interfaces, comparison );
}



///
//  InitMetaClasses()
//
static void InitMetaClasses( void )
{
    // *** SPIN LOCK HERE ***

    if( !MetaClassesInitialized )
    {
        MetaClassesInitialized = 1;

        InitMetaClass( &__DKMetaClass__,      &__DKMetaClass__, NULL );
        InitMetaClass( &__DKClassClass__,     &__DKMetaClass__, NULL );
        InitMetaClass( &__DKSelectorClass__,  &__DKMetaClass__, NULL );
        InitMetaClass( &__DKInterfaceClass__, &__DKMetaClass__, NULL );
        InitMetaClass( &__DKMethodClass__,    &__DKMetaClass__, &__DKInterfaceClass__ );
        InitMetaClass( &__DKPropertyClass__,  &__DKMetaClass__, NULL );
        InitMetaClass( &__DKObjectClass__,    &__DKMetaClass__, NULL );
        
        InstallMetaClassInterfaces( &__DKMetaClass__,      &__DKClassLifeCycle__,     &__DKStaticObjectReferenceCounting__, &__DKDefaultComparison__ );
        InstallMetaClassInterfaces( &__DKClassClass__,     &__DKClassLifeCycle__,     &__DKDefaultReferenceCounting__,      &__DKDefaultComparison__ );
        InstallMetaClassInterfaces( &__DKSelectorClass__,  &__DKDefaultLifeCycle__,   &__DKStaticObjectReferenceCounting__, &__DKDefaultComparison__ );
        InstallMetaClassInterfaces( &__DKInterfaceClass__, &__DKInterfaceLifeCycle__, &__DKDefaultReferenceCounting__,      &__DKInterfaceComparison__ );
        InstallMetaClassInterfaces( &__DKMethodClass__,    &__DKInterfaceLifeCycle__, &__DKDefaultReferenceCounting__,      &__DKInterfaceComparison__ );
        InstallMetaClassInterfaces( &__DKPropertyClass__,  &__DKDefaultLifeCycle__,   &__DKDefaultReferenceCounting__,      &__DKDefaultComparison__ );
        InstallMetaClassInterfaces( &__DKObjectClass__,    &__DKDefaultLifeCycle__,   &__DKDefaultReferenceCounting__,      &__DKDefaultComparison__ );
    }
}



// Alloc/Free Objects ====================================================================

///
//  DKAllocObject()
//
DKTypeRef DKAllocObject( DKTypeRef _class, size_t size, int attributes )
{
    if( _class )
    {
        struct DKObjectHeader * obj = DKAlloc( size );
        
        obj->isa = DKRetain( _class );
        obj->refcount = 1;
        obj->attributes = attributes;

        return obj;
    }
    
    return NULL;
}

///
//  DKDeallocObject()
//
void DKDeallocObject( DKTypeRef ref )
{
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    
    DKAssert( obj );
    DKAssert( obj->refcount == 0 );
    DKAssert( !DKTestObjectAttribute( obj, DKObjectIsStatic ) );
    
    const struct DKClass * classObject = obj->isa;
    
    for( const struct DKClass * cls = classObject; cls != NULL; cls = cls->superclass )
    {
        DKLifeCycle * lifeCycle = cls->fastLookupTable[DKFastLookupLifeCycle];
        lifeCycle->finalize( obj );
        
        classObject = cls->superclass;
    }
    
    DKRelease( obj->isa );
    DKFree( obj );
}


///
//  DKCreateClass()
//
DKTypeRef DKCreateClass( DKTypeRef superclass )
{
    struct DKClass * classObject = (struct DKClass *)DKAllocObject( DKClassClass(), sizeof(struct DKClass), 0 );

    classObject->superclass = DKRetain( superclass );
    
    DKPointerArrayInit( &classObject->interfaces );
    DKPointerArrayInit( &classObject->properties );

    memset( classObject->fastLookupTable, 0, sizeof(classObject->fastLookupTable) );
    
    return classObject;
}

static void DKClassFinalize( DKTypeRef ref )
{
    struct DKClass * classObject = (struct DKClass *)ref;
    
    DKRelease( classObject->superclass );

    // Release interfaces
    DKIndex count = classObject->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        const DKInterface * interface = classObject->interfaces.data[i];
        DKRelease( interface );
    }
    
    DKPointerArrayClear( &classObject->interfaces );
    
    // Release properties
    DKPointerArrayClear( &classObject->properties );
}


///
//  DKCreateInterface()
//
DKTypeRef DKCreateInterface( DKSEL sel, size_t size )
{
    if( sel )
    {
        struct DKInterface * interface = (struct DKInterface *)DKAllocObject( DKInterfaceClass(), size, 0 );
        interface->sel = DKRetain( sel );
    
        return interface;
    }
    
    return NULL;
}

static void DKInterfaceFinalize( DKTypeRef ref )
{
    struct DKInterface * interface = (struct DKInterface *)ref;
    DKRelease( interface->sel );
}


///
//  DKInstallInterface()
//
void DKInstallInterface( DKTypeRef _class, DKTypeRef interface )
{
    if( !_class )
    {
        DKError( "DKInstallInterface: Trying to install an interface on a NULL class object." );
        return;
    }
    
    if( !interface )
    {
        DKError( "DKInstallInterface: Trying to install a NULL interface." );
        return;
    }

    struct DKClass * classObject = (struct DKClass *)_class;
    DKInterface * interfaceObject = interface;

    DKAssert( (classObject->_obj.isa == &__DKClassClass__) || (classObject->_obj.isa == &__DKMetaClass__) );
    DKAssert( (interfaceObject->_obj.isa == &__DKInterfaceClass__) || (interfaceObject->_obj.isa == &__DKMethodClass__) );

    // Retain the interface
    DKRetain( interfaceObject );
    
    // Update the fast-lookup table
    int fastLookupIndex = DKFastLookupIndex( interfaceObject->sel );
    DKAssert( (fastLookupIndex >= 0) && (fastLookupIndex < DKFastLookupTableSize) );
    
    if( fastLookupIndex )
    {
        // If we're replacing a fast-lookup entry, make sure the selectors match
        DKTypeRef oldInterface = classObject->fastLookupTable[fastLookupIndex];

        if( (oldInterface != NULL) && !DKEqual( interface, oldInterface ) )
        {
            // This likely means that two interfaces are trying to use the same fast lookup index
            DKFatalError( "DKInstallInterface: Fast-Lookup selector doesn't match the installed interface." );
            return;
        }
    
        classObject->fastLookupTable[fastLookupIndex] = interface;
    }
    
    // Invalidate the interface cache
    // Do stuff here...

    // Replace the interface in the interface table
    DKIndex count = classObject->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        const DKInterface * oldInterfaceObject = classObject->interfaces.data[i];
        
        if( DKEqual( oldInterfaceObject->sel, interfaceObject->sel ) )
        {
            DKRelease( oldInterfaceObject );
            classObject->interfaces.data[i] = interfaceObject;
            return;
        }
    }
    
    // Add the interface to the interface table
    DKPointerArrayAppendPointer( &classObject->interfaces, interfaceObject );
}


///
//  DKInstallMethod()
//
void DKInstallMethod( DKTypeRef _class, DKSEL sel, const void * imp )
{
    struct DKMethod * method = (struct DKMethod *)DKAllocObject( DKMethodClass(), sizeof(DKMethod), 0 );

    method->sel = sel;
    method->imp = imp;
    
    DKInstallInterface( _class, method );
    
    DKRelease( method );
}


///
//  DKLookupInterface()
//
DKTypeRef DKLookupInterface( DKTypeRef ref, DKSEL sel )
{
    if( !ref )
        return NULL;

    DKObjectHeader * obj = ref;
    struct DKClass * classObject = (struct DKClass *)obj->isa;
    
    if( classObject == &__DKClassClass__ )
        classObject = (struct DKClass *)ref;

    // First check the fast lookup table
    int fastLookupIndex = DKFastLookupIndex( sel );
    DKAssert( (fastLookupIndex >= 0) && (fastLookupIndex < DKFastLookupTableSize) );
    
    DKInterface * interface = classObject->fastLookupTable[fastLookupIndex];
    
    if( interface )
        return interface;
    
    // Next check the interface cache
    // Do stuff here...
    
    // Finally search the interface tables for the selector
    for( DKClass * cls = classObject; cls != NULL; cls = cls->superclass )
    {
        DKIndex count = cls->interfaces.length;
        
        for( DKIndex i = 0; i < count; ++i )
        {
            interface = cls->interfaces.data[i];
            
            if( DKEqual( interface->sel, sel ) )
            {
                // Update the fast lookup table
                if( fastLookupIndex > 0 )
                {
                    classObject->fastLookupTable[fastLookupIndex] = interface;
                }
            
                // Update the interface cache
                // Do stuff here...
            
                return interface;
            }
        }
    }
    
    return NULL;
}


///
//  DKLookupMethod()
//
static struct DKSEL DKMethodNotFoundSelector =
{
    { &__DKSelectorClass__, 1, DKObjectIsStatic },
    "DKMethodNotFound",
    "void DKMethodNotFound( ??? )"
};

static void DKMethodNotFoundImp( DKTypeRef ref, DKSEL sel )
{
    DKAssert( ref && sel );
    DKFatalError( "DKMethodNotFound: Method '%s' not found on object 'FIX THIS'", sel->name );
}

static DKMethod DKMethodNotFound =
{
    { &__DKMethodClass__, 1, DKObjectIsStatic },
    &DKMethodNotFoundSelector,
    DKMethodNotFoundImp
};

DKTypeRef DKLookupMethod( DKTypeRef ref, DKSEL sel )
{
    DKMethod * method = DKLookupInterface( ref, sel );
    
    if( method )
    {
        DKAssertMsg( DKIsMemberOfClass( method, DKMethodClass() ), "DKLookupMethod: Installed interface '%s' is not a method.", sel->name );
        return method;
    }
    
    return &DKMethodNotFound;
}




// Polymorphic Wrappers ==================================================================

///
//  DKCreate()
//
DKTypeRef DKCreate( DKTypeRef _class )
{
    if( _class )
    {
        DKLifeCycle * lifeCycle = DKLookupInterface( _class, DKSelector(LifeCycle) );

        DKTypeRef ref = lifeCycle->allocate();
        
        return lifeCycle->initialize( ref );
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
        const struct DKClass * classObject = obj->isa;
        
        do
        {
            if( classObject == _class )
                return 1;
            
            classObject = classObject->superclass;
        }
        while( classObject != NULL );
    }
    
    return 0;
}


///
//  DKRetain()
//
DKTypeRef DKRetain( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        DKReferenceCounting * referenceCounting = DKLookupInterface( ref, DKSelector(ReferenceCounting) );
        return referenceCounting->retain( obj );
    }

    return ref;
}


///
//  DKRelease()
//
void DKRelease( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        DKReferenceCounting * referenceCounting = DKLookupInterface( ref, DKSelector(ReferenceCounting) );
        referenceCounting->release( obj );
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
        DKComparison * comparison = DKLookupInterface( a, DKSelector(Comparison) );
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
        DKComparison * comparison = DKLookupInterface( a, DKSelector(Comparison) );
        return comparison->compare( a, b );
    }
    
    return a < b ? -1 : 1;
}


///
//  DKHash()
//
DKHashIndex DKHash( DKTypeRef ref )
{
    if( ref )
    {
        DKComparison * comparison = DKLookupInterface( ref, DKSelector(Comparison) );
        return comparison->hash( ref );
    }
    
    return 0;
}



