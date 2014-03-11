//
//  DKObject.c
//  DK
//
//  Created by Derek Nylen on 2013-04-11.
//  Copyright (c) 2013 Hurlybird Media. All rights reserved.
//

#include "DKObject.h"



DKDefineSUID( DKObjectTypeID );
DKDefineSUID( DKObjectInterfaceID );
DKDefineSUID( DKClassTypeID );
DKDefineSUID( DKInterfaceTypeID );


static DKTypeRef    DKClassGetInterface( DKTypeRef ref, DKSUID suid );
static DKSUID       DKClassGetTypeID( DKTypeRef ref );
static DKTypeRef    DKInterfaceGetInterface( DKTypeRef ref, DKSUID suid );
static DKSUID       DKInterfaceGetTypeID( DKTypeRef ref );
static DKTypeRef    DKDoNothingRetain( DKTypeRef ref );
static void         DKDoNothingRelease( DKTypeRef ref );
static DKTypeRef    DKDisallowAllocate( void );
static DKTypeRef    DKDisallowInitialize( DKTypeRef ref );
static void         DKDisallowFinalize( DKTypeRef ref );




// Base Classes ==========================================================================
const DKObjectInterface __DKClassClass__ =
{
    DK_CLASS_OBJECT,

    DKClassGetInterface,
    DKClassGetTypeID,
    
    DKDoNothingRetain,
    DKDoNothingRelease,
    DKDisallowAllocate,
    DKDisallowInitialize,
    DKDisallowFinalize,
    
    DKObjectEqual,
    DKObjectCompare,
    DKObjectHash
};

static const DKObjectInterface __DKObjectClass__ =
{
    DK_CLASS_OBJECT,

    DKObjectGetInterface,
    DKObjectGetTypeID,
    
    DKObjectRetain,
    DKObjectRelease,
    DKObjectAllocate,
    DKObjectInitialize,
    DKObjectFinalize,

    DKObjectEqual,
    DKObjectCompare,
    DKObjectHash
};

const DKObjectInterface __DKInterfaceClass__ =
{
    DK_CLASS_OBJECT,

    DKInterfaceGetInterface,
    DKInterfaceGetTypeID,
    
    DKDoNothingRetain,
    DKDoNothingRelease,
    DKDisallowAllocate,
    DKDisallowInitialize,
    DKDisallowFinalize,

    DKObjectEqual,
    DKObjectCompare,
    DKObjectHash
};




// DKObject ==============================================================================

///
//  DKObjectClass()
//
DKTypeRef DKObjectClass( void )
{
    return &__DKObjectClass__;
}


///
//  DKObjectGetInterface()
//
DKTypeRef DKObjectGetInterface( DKTypeRef ref, DKSUID suid )
{
    if( suid == DKObjectInterfaceID )
        return &__DKObjectClass__;
        
    return NULL;
}


///
//  DKObjectGetTypeID()
//
DKSUID DKObjectGetTypeID( DKTypeRef ref )
{
    return DKObjectTypeID;
}


///
//  DKObjectRetain()
//
DKTypeRef DKObjectRetain( DKTypeRef ref )
{
    DKObjectHeader * obj = (DKObjectHeader *)ref;
    
    if( obj )
    {
        DKAtomicIncrement( &obj->_refcount );
    }

    return ref;
}


///
//  DKObjectRelease()
//
void DKObjectRelease( DKTypeRef ref )
{
    DKObjectHeader * obj = (DKObjectHeader *)ref;

    if( obj )
    {
        DKAtomicInt n = DKAtomicDecrement( &obj->_refcount );
        
        assert( n >= 0 );
        
        if( n == 0 )
        {
            const DKObjectInterface * objectInterface = obj->_isa;
            objectInterface->finalize( obj );
            DKFree( obj );
        }
    }
}


///
//  DKObjectAllocate()
//
DKTypeRef DKObjectAllocate( void )
{
    return DKAllocAndZero( sizeof(DKObjectHeader) );
}


///
//  DKObjectInitialize()
//
DKTypeRef DKObjectInitialize( DKTypeRef ref )
{
    // Nothing to do here
    return ref;
}


///
//  DKObjectFinalize()
//
void DKObjectFinalize( DKTypeRef ref )
{
    // Nothing to do here
}


///
//  DKObjectEqual()
//
int DKObjectEqual( DKTypeRef a, DKTypeRef b )
{
    return a == b;
}


///
//  DKObjectCompare()
//
int DKObjectCompare( DKTypeRef a, DKTypeRef b )
{
    if( a < b )
        return 1;
    
    if( a > b )
        return -1;
    
    return 0;
}


///
//  DKObjectHash()
//
DKHashIndex DKObjectHash( DKTypeRef ref )
{
    assert( sizeof(DKHashIndex) == sizeof(DKTypeRef) );
    return (DKHashIndex)ref;
}




// DKClass and DKInterface ===============================================================

///
//  DKClassGetInterface()
//
static DKTypeRef DKClassGetInterface( DKTypeRef ref, DKSUID suid )
{
    // We should never be able to call this on the root class in normal usage
    assert( ref != &__DKClassClass__ );

    if( ref )
    {
        assert( DKGetTypeID( ref ) == DKClassTypeID );
        
        // This makes class.getInterface() the same as calling instance.getInterface()
        const DKObjectInterface * objectInterface = ref;
        return objectInterface->getInterface( ref, suid );
    }

    return NULL;
}


///
//  DKClassGetTypeID()
//
static DKSUID DKClassGetTypeID( DKTypeRef ref )
{
    return DKClassTypeID;
}


///
//  DKInterfaceGetInterface()
//
static DKTypeRef DKInterfaceGetInterface( DKTypeRef ref, DKSUID suid )
{
    return NULL;
}


///
//  DKInterfaceGetTypeID()
//
static DKSUID DKInterfaceGetTypeID( DKTypeRef ref )
{
    return DKInterfaceTypeID;
}


///
//  DKDoNothingRetain()
//
static DKTypeRef DKDoNothingRetain( DKTypeRef ref )
{
    return ref;
}


///
//  DKDoNothingRelease()
//
static void DKDoNothingRelease( DKTypeRef ref )
{
}


///
//  DKDisallowAllocate()
//
static DKTypeRef DKDisallowAllocate( void )
{
    assert( 0 );
    return NULL;
}


///
//  DKDisallowInitialize()
//
static DKTypeRef DKDisallowInitialize( DKTypeRef ref )
{
    assert( 0 );
    return ref;
}


///
//  DKDisallowFinalize()
//
static void DKDisallowFinalize( DKTypeRef ref )
{
    assert( 0 );
}




// DKObject Polymorphic Wrappers =========================================================

///
//  DKObjectHeaderInit()
//
void DKObjectHeaderInit( DKTypeRef ref, DKTypeRef _class, DKOptionFlags flags )
{
    DKObjectHeader * obj = (DKObjectHeader *)ref;
    
    if( obj )
    {
        assert( _class );
        assert( obj->_isa == NULL );
        assert( obj->_refcount == 0 );
        assert( obj->_flags == 0 );
        
        obj->_isa = _class;
        obj->_refcount = 1;
        obj->_flags = flags;
    }
}


///
//  DKCreate()
//
DKTypeRef DKCreate( DKTypeRef _class )
{
    const DKObjectInterface * classObject = _class;

    if( classObject )
    {
        DKObjectHeader * obj = (DKObjectHeader *)classObject->allocate();
        DKObjectHeaderInit( obj, classObject, 0 );
        
        return classObject->initialize( obj );
    }
    
    return NULL;
}


///
//  DKGetClass()
//
DKTypeRef DKGetClass( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        return obj->_isa;
    }
    
    return NULL;
}


///
//  DKGetInterface()
//
DKTypeRef DKGetInterface( DKTypeRef ref, DKSUID suid )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        if( suid == DKObjectInterfaceID )
        {
            if( DKGetTypeID( obj ) == DKClassTypeID )
                return obj;
            
            return obj->_isa;
        }

        const DKObjectInterface * objectInterface = obj->_isa;
        return objectInterface->getInterface( obj, suid );
    }
    
    return NULL;
}


///
//  DKGetTypeID()
//
DKSUID DKGetTypeID( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKObjectInterface * objectInterface = obj->_isa;
        return objectInterface->getTypeID( obj );
    }
    
    return NULL;
}


///
//  DKRetain()
//
DKTypeRef DKRetain( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKObjectInterface * objectInterface = obj->_isa;
        return objectInterface->retain( obj );
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
        const DKObjectInterface * objectInterface = obj->_isa;
        objectInterface->release( obj );
    }
}


///
//  DKEqual()
//
int DKEqual( DKTypeRef a, DKTypeRef b )
{
    const DKObjectHeader * obj = a;

    if( obj )
    {
        const DKObjectInterface * objectInterface = obj->_isa;
        return objectInterface->equal( a, b );
    }
    
    return a == b;
}


///
//  DKCompare()
//
int DKCompare( DKTypeRef a, DKTypeRef b )
{
    const DKObjectHeader * obj = a;

    if( obj )
    {
        const DKObjectInterface * objectInterface = obj->_isa;
        return objectInterface->compare( a, b );
    }
    
    return a < b;
}


///
//  DKHash()
//
DKHashIndex DKHash( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;

    if( obj )
    {
        const DKObjectInterface * objectInterface = obj->_isa;
        return objectInterface->hash( ref );
    }
    
    return 0;
}


///
//  DKGetFlag()
//
int DKGetFlag( DKTypeRef ref, int flag )
{
    DKObjectHeader * obj = (DKObjectHeader *)ref;
    
    if( obj )
    {
        return (obj->_flags & (1 << flag)) != 0;
    }
    
    return 0;
}


///
//  DKSetFlag()
//
void DKSetFlag( DKTypeRef ref, int flag, int value )
{
    DKObjectHeader * obj = (DKObjectHeader *)ref;
    
    if( obj )
    {
        if( value )
            obj->_flags |= (1 << flag);
        
        else
            obj->_flags &= ~(1 << flag);
    }
}







