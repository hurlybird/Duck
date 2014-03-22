//
//  DKRuntime.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-20.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKEnv.h"
#include "DKMemory.h"
#include "DKRuntime.h"
#include "DKCommonInterfaces.h"


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
static void InitMetaClass( struct DKClass * metaclass, struct DKClass * isa,
    DKReferenceCounting * referenceCounting, DKComparison * comparison )
{
    memset( metaclass, 0, sizeof(struct DKClass) );
    
    struct DKObjectHeader * header = (struct DKObjectHeader *)metaclass;
    header->isa = isa;
    header->refcount = 1;
    header->attributes = DKObjectIsStatic;

    metaclass->superclass = NULL;

    DKElementArrayInit( &metaclass->interfaces, sizeof(DKTypeRef) );
    DKElementArrayReserve( &metaclass->interfaces, 4 );
    
    DKElementArrayInit( &metaclass->properties, sizeof(DKTypeRef) );
    DKElementArrayReserve( &metaclass->interfaces, 4 );

    // Just set the fast-lookup pointers here -- they'll be properly installed once all
    // the base classes are initialized
    metaclass->fastLookupTable[DKFastLookupLifeCycle] = DKDefaultLifeCycle();
    metaclass->fastLookupTable[DKFastLookupReferenceCounting] = referenceCounting;
    metaclass->fastLookupTable[DKFastLookupIntrospection] = DKDefaultIntrospection();
    metaclass->fastLookupTable[DKFastLookupComparison] = comparison;
}


///
//  InitMetaClassInterfaceTable()
//
static void InitMetaClassInterfaceTable( struct DKClass * metaclass )
{
    // Use the fast-lookup pointers set above to properly install the interfaces
    for( int i = 1; i < DKFastLookupTableSize; i++ )
    {
        DKTypeRef interface = metaclass->fastLookupTable[i];
        
        if( interface )
            DKInstallInterface( metaclass, interface );
    }
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

        InitMetaClass( &__DKMetaClass__,      &__DKMetaClass__, DKStaticObjectReferenceCounting(), DKDefaultComparison() );
        InitMetaClass( &__DKClassClass__,     &__DKMetaClass__, DKDefaultReferenceCounting(),      DKDefaultComparison() );
        InitMetaClass( &__DKSelectorClass__,  &__DKMetaClass__, DKStaticObjectReferenceCounting(), DKDefaultComparison() );
        InitMetaClass( &__DKInterfaceClass__, &__DKMetaClass__, DKDefaultReferenceCounting(),      DKInterfaceComparison() );
        InitMetaClass( &__DKMethodClass__,    &__DKMetaClass__, DKDefaultReferenceCounting(),      DKInterfaceComparison() );
        InitMetaClass( &__DKPropertyClass__,  &__DKMetaClass__, DKDefaultReferenceCounting(),      DKDefaultComparison() );
        InitMetaClass( &__DKObjectClass__,    &__DKMetaClass__, DKDefaultReferenceCounting(),      DKDefaultComparison() );
        
        InitMetaClassInterfaceTable( &__DKMetaClass__ );
        InitMetaClassInterfaceTable( &__DKClassClass__ );
        InitMetaClassInterfaceTable( &__DKSelectorClass__ );
        InitMetaClassInterfaceTable( &__DKInterfaceClass__ );
        InitMetaClassInterfaceTable( &__DKMethodClass__ );
        InitMetaClassInterfaceTable( &__DKPropertyClass__ );
        InitMetaClassInterfaceTable( &__DKObjectClass__ );
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
    
    while( classObject )
    {
        DKLifeCycle * lifeCycle = classObject->fastLookupTable[DKFastLookupLifeCycle];
        lifeCycle->finalize( obj );
        
        classObject = classObject->superclass;
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
    
    classObject->superclass = superclass;
    
    DKElementArrayInit( &classObject->interfaces, sizeof(DKTypeRef) );
    DKElementArrayReserve( &classObject->interfaces, 16 );

    DKElementArrayInit( &classObject->properties, sizeof(DKTypeRef) );
    DKElementArrayReserve( &classObject->properties, 16 );

    DKInstallInterface( classObject, DKDefaultLifeCycle() );
    DKInstallInterface( classObject, DKDefaultReferenceCounting() );
    DKInstallInterface( classObject, DKDefaultIntrospection() );
    DKInstallInterface( classObject, DKDefaultComparison() );
    
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
    const DKInterface * interfaceObject = interface;

    assert( (classObject->_obj.isa == &__DKClassClass__) || (classObject->_obj.isa == &__DKMetaClass__) );
    assert( interfaceObject->_obj.isa == &__DKInterfaceClass__ );

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
//  DKLookupInterface()
//
DKTypeRef DKLookupInterface( DKTypeRef ref, DKSEL sel )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        const struct DKClass * classObject = obj->isa;
        
        if( classObject == DKClassClass() )
        {
            classObject = ref;
        }

        // First check the fast lookup table
        int fastLookupIndex = DKFastLookupIndex( sel );
        assert( (fastLookupIndex >= 0) && (fastLookupIndex < DKFastLookupTableSize) );
        
        if( fastLookupIndex )
        {
            return classObject->fastLookupTable[fastLookupIndex];
        }
        
        // Next check the interface cache
        // Do stuff here...
        
        // Finally search the interface table for the interface
        DKIndex count = DKElementArrayGetCount( &classObject->interfaces );
        
        for( DKIndex i = 0; i < count; ++i )
        {
            const DKInterface * interface = DKElementArrayGetElementAtIndex( &classObject->interfaces, i, void * );
            
            if( DKEqual( interface->sel, sel ) )
                return interface;
        }
    }
    
    return NULL;
}


///
//  DKFastLookupInterface()
//
DKTypeRef DKFastLookupInterface( DKTypeRef ref, int index )
{
    assert( (index >= 0) && (index < DKFastLookupTableSize) );

    if( ref )
    {
        const DKObjectHeader * obj = ref;
        const struct DKClass * classObject = obj->isa;
        
        if( classObject == DKClassClass() )
            classObject = ref;
        
        return classObject->fastLookupTable[index];
    }
    
    return NULL;
}




// Polymorphic Wrappers ==================================================================

///
//  DKCreate()
//
DKTypeRef DKCreate( DKTypeRef _class )
{
    const struct DKClass * classObject = _class;

    if( classObject )
    {
        DKLifeCycle * lifeCycle = classObject->fastLookupTable[DKFastLookupLifeCycle];

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
        const struct DKClass * classObject = obj->isa;
        DKReferenceCounting * referenceCounting = classObject->fastLookupTable[DKFastLookupReferenceCounting];

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
        const struct DKClass * classObject = obj->isa;
        DKReferenceCounting * referenceCounting = classObject->fastLookupTable[DKFastLookupReferenceCounting];

        referenceCounting->release( obj );
    }
}


///
//  DKQueryInterface()
//
DKTypeRef DKQueryInterface( DKTypeRef ref, DKSEL sel )
{
    if( ref )
    {
        const DKObjectHeader * obj = ref;
        const struct DKClass * classObject = obj->isa;
        DKIntrospection * introspection = classObject->fastLookupTable[DKFastLookupIntrospection];
        
        return introspection->queryInterface( ref, sel );
    }
    
    return NULL;
}


///
//  DKQueryMethod()
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

DKTypeRef DKQueryMethod( DKTypeRef ref, DKSEL sel )
{
    DKMethod * method = DKQueryInterface( ref, sel );
    
    if( method )
    {
        assert( DKIsMemberOfClass( method, DKMethodClass() ) );
        return method;
    }
    
    return &DKMethodNotFound;
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
        const DKObjectHeader * obj = a;
        const struct DKClass * classObject = obj->isa;
        DKComparison * comparison = classObject->fastLookupTable[DKFastLookupComparison];

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
        const DKObjectHeader * obj = a;
        const struct DKClass * classObject = obj->isa;
        DKComparison * comparison = classObject->fastLookupTable[DKFastLookupComparison];

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
        const DKObjectHeader * obj = ref;
        const struct DKClass * classObject = obj->isa;
        DKComparison * comparison = classObject->fastLookupTable[DKFastLookupComparison];
        
        return comparison->hash( ref );
    }
    
    return 0;
}



