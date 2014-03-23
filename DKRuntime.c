//
//  DKRuntime.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-20.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKEnv.h"
#include "DKElementArray.h"
#include "DKRuntime.h"
#include "DKLifeCycle.h"
#include "DKReferenceCounting.h"
#include "DKComparison.h"


// Internal Class Structure ==============================================================
#define MAX_CLASS_NAME_LENGTH   32

struct DKClass
{
    const DKObjectHeader    _obj;
    
    char                    name[MAX_CLASS_NAME_LENGTH];

    const struct DKClass *  superclass;

    DKTypeRef               fastLookupTable[DKFastLookupTableSize];
    
    DKElementArray          interfaces;
    DKElementArray          properties;
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


///
//  InitMetaClass()
//
static void InitMetaClass( struct DKClass * metaclass, struct DKClass * isa, struct DKClass * superclass )
{
    memset( metaclass, 0, sizeof(struct DKClass) );
    
    struct DKObjectHeader * header = (struct DKObjectHeader *)metaclass;
    header->isa = isa;
    header->refcount = 1;
    header->attributes = DKObjectIsStatic;

    // The superclass may not be initialized, so don't retain it in this case
    metaclass->superclass = superclass;

    DKElementArrayInit( &metaclass->interfaces, sizeof(DKTypeRef) );
    DKElementArrayReserve( &metaclass->interfaces, 8 );
    
    DKElementArrayInit( &metaclass->properties, sizeof(DKTypeRef) );
    DKElementArrayReserve( &metaclass->interfaces, 8 );

}


///
//  PrecacheMetaClassInterfaces()
//
static void PrecacheMetaClassInterfaces( struct DKClass * metaclass,
    DKReferenceCounting * referenceCounting, DKComparison * comparison )
{
    // Setup critical interfaces we need to construct the meta-classes. This is a bit
    // backwards since we're prepping the fast-lookup table so we can use the interfaces
    // before they're properly installed.
    metaclass->fastLookupTable[DKFastLookupReferenceCounting] = referenceCounting;
    metaclass->fastLookupTable[DKFastLookupComparison] = comparison;
}


///
//  InstallMetaClassInterfaces()
//
static void InstallMetaClassInterfaces( struct DKClass * metaclass,
    DKReferenceCounting * referenceCounting, DKComparison * comparison )
{
    DKInstallInterface( metaclass, DKDefaultLifeCycle() );
    DKInstallInterface( metaclass, referenceCounting );
    DKInstallInterface( metaclass, comparison );
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

        // Step 1: initialize all the class objects
        InitMetaClass( &__DKMetaClass__,      &__DKMetaClass__, NULL );
        InitMetaClass( &__DKClassClass__,     &__DKMetaClass__, NULL );
        InitMetaClass( &__DKSelectorClass__,  &__DKMetaClass__, NULL );
        InitMetaClass( &__DKInterfaceClass__, &__DKMetaClass__, NULL );
        InitMetaClass( &__DKMethodClass__,    &__DKMetaClass__, &__DKInterfaceClass__ );
        InitMetaClass( &__DKPropertyClass__,  &__DKMetaClass__, NULL );
        InitMetaClass( &__DKObjectClass__,    &__DKMetaClass__, NULL );
        
        // Step 2: Precache critical interfaces needed for step 3
        PrecacheMetaClassInterfaces( &__DKSelectorClass__,  DKStaticObjectReferenceCounting(), DKDefaultComparison() );
        PrecacheMetaClassInterfaces( &__DKInterfaceClass__, DKDefaultReferenceCounting(),      DKInterfaceComparison() );
        
        // Step 3: Install common interfaces
        InstallMetaClassInterfaces( &__DKMetaClass__,      DKStaticObjectReferenceCounting(), DKDefaultComparison() );
        InstallMetaClassInterfaces( &__DKClassClass__,     DKDefaultReferenceCounting(),      DKDefaultComparison() );
        InstallMetaClassInterfaces( &__DKSelectorClass__,  DKStaticObjectReferenceCounting(), DKDefaultComparison() );
        InstallMetaClassInterfaces( &__DKInterfaceClass__, DKDefaultReferenceCounting(),      DKInterfaceComparison() );
        InstallMetaClassInterfaces( &__DKMethodClass__,    DKDefaultReferenceCounting(),      DKInterfaceComparison() );
        InstallMetaClassInterfaces( &__DKPropertyClass__,  DKDefaultReferenceCounting(),      DKDefaultComparison() );
        InstallMetaClassInterfaces( &__DKObjectClass__,    DKDefaultReferenceCounting(),      DKDefaultComparison() );
    }
}




// Alloc/Free Objects ====================================================================

///
//  DKAllocObject()
//
DKTypeRef DKAllocObject( DKTypeRef _class, size_t size, int attributes )
{
    assert( _class );
    
    struct DKObjectHeader * obj = DKAlloc( size );
    obj->isa = _class;
    obj->refcount = 1;
    obj->attributes = attributes;
    
    return obj;
}


///
//  DKFreeObject()
//
void DKFreeObject( DKTypeRef ref )
{
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    
    assert( obj );
    assert( obj->refcount == 0 );
    assert( !DKTestObjectAttribute( obj, DKObjectIsStatic ) );
    
    const struct DKClass * classObject = obj->isa;
    
    for( const struct DKClass * cls = classObject; cls != NULL; cls = cls->superclass )
    {
        DKLifeCycle * lifeCycle = cls->fastLookupTable[DKFastLookupLifeCycle];
        lifeCycle->finalize( obj );
        
        classObject = cls->superclass;
    }
    
    DKFree( obj );
}


///
//  DKAllocClass()
//
DKTypeRef DKAllocClass( DKTypeRef superclass )
{
    struct DKClass * classObject = (struct DKClass *)DKAllocObject( DKClassClass(), sizeof(struct DKClass), 0 );

    struct DKObjectHeader * header = (struct DKObjectHeader *)classObject;
    header->isa = DKClassClass();
    header->refcount = 1;
    
    classObject->superclass = DKRetain( superclass );
    
    DKElementArrayInit( &classObject->interfaces, sizeof(DKTypeRef) );
    DKElementArrayReserve( &classObject->interfaces, 16 );

    DKElementArrayInit( &classObject->properties, sizeof(DKTypeRef) );
    DKElementArrayReserve( &classObject->properties, 16 );

    memset( classObject->fastLookupTable, 0, sizeof(classObject->fastLookupTable) );
    
    return classObject;
}


///
//  DKAllocInterface()
//
DKTypeRef DKAllocInterface( DKSEL sel, size_t size )
{
    assert( sel );

    struct DKInterface * interface = (struct DKInterface *)DKAllocObject( DKInterfaceClass(), size, 0 );
    interface->sel = sel;
    
    return interface;
}


///
//  DKInstallInterface()
//
void DKInstallInterface( DKTypeRef _class, DKTypeRef interface )
{
    assert( _class && interface );

    struct DKClass * classObject = (struct DKClass *)_class;
    DKInterface * interfaceObject = interface;

    assert( (classObject->_obj.isa == &__DKClassClass__) || (classObject->_obj.isa == &__DKMetaClass__) );
    assert( (interfaceObject->_obj.isa == &__DKInterfaceClass__) || (interfaceObject->_obj.isa == &__DKMethodClass__) );

    // Retain the interface
    DKRetain( interfaceObject );
    
    // Update the fast-lookup table
    int fastLookupIndex = DKFastLookupIndex( interfaceObject->sel );
    assert( (fastLookupIndex >= 0) && (fastLookupIndex < DKFastLookupTableSize) );
    
    if( fastLookupIndex )
    {
        // If we're replacing a fast-lookup entry, make sure the selectors match
        DKTypeRef oldInterface = classObject->fastLookupTable[fastLookupIndex];

        if( (oldInterface != NULL) && !DKEqual( interface, oldInterface ) )
        {
            // This likely means that two interfaces are trying to use the same fast lookup index
            assert( 0 );
            return;
        }
    
        classObject->fastLookupTable[fastLookupIndex] = interface;
    }
    
    // Invalidate the interface cache
    // Do stuff here...

    // Replace the interface in the interface table
    DKIndex count = DKElementArrayGetCount( &classObject->interfaces );
    
    for( DKIndex i = 0; i < count; ++i )
    {
        const DKInterface * oldInterfaceObject = DKElementArrayGetElementAtIndex( &classObject->interfaces, i, void * );
        
        if( DKEqual( oldInterfaceObject->sel, interfaceObject->sel ) )
        {
            DKRelease( oldInterfaceObject );
            DKElementArraySetElementAtIndex( &classObject->interfaces, i, &interfaceObject );
            return;
        }
    }
    
    // Add the interface to the interface table
    DKElementArrayAppendElement( &classObject->interfaces, &interfaceObject );
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
    assert( (fastLookupIndex >= 0) && (fastLookupIndex < DKFastLookupTableSize) );
    
    DKInterface * interface = classObject->fastLookupTable[fastLookupIndex];
    
    if( interface )
        return interface;
    
    // Next check the interface cache
    // Do stuff here...
    
    // Finally search the interface tables for the selector
    for( DKClass * cls = classObject; cls != NULL; cls = cls->superclass )
    {
        DKIndex count = DKElementArrayGetCount( &cls->interfaces );
        
        for( DKIndex i = 0; i < count; ++i )
        {
            interface = DKElementArrayGetElementAtIndex( &cls->interfaces, i, DKInterface * );
            
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
    assert( 0 );
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
        assert( DKIsMemberOfClass( method, DKMethodClass() ) );
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



