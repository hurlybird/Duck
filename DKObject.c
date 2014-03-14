//
//  DKObject.c
//  DK
//
//  Created by Derek Nylen on 2013-04-11.
//  Copyright (c) 2013 Hurlybird Media. All rights reserved.
//

#include "DKObject.h"


DKDefineSUID( DKObjectInterfaceID );


static DKTypeRef    DKClassGetInterface( DKTypeRef ref, DKSUID suid );
static DKTypeRef    DKInterfaceGetInterface( DKTypeRef ref, DKSUID suid );



// Base Classes ==========================================================================
const DKObjectInterface __DKClassClass__ =
{
    DK_CLASS_OBJECT,

    DKClassGetInterface,
    
    DKDoNothingRetain,
    DKDoNothingRelease,
    
    DKDisallowAllocate,
    DKDisallowInitialize,
    DKDisallowFinalize,
    
    DKPtrEqual,
    DKPtrCompare,
    DKPtrHash
};

static const DKObjectInterface __DKObjectClass__ =
{
    DK_CLASS_OBJECT,

    DKObjectGetInterface,
    
    DKObjectRetain,
    DKObjectRelease,
    
    DKObjectAllocate,
    DKObjectInitialize,
    DKObjectFinalize,
    
    DKPtrEqual,
    DKPtrCompare,
    DKPtrHash
};

const DKObjectInterface __DKInterfaceClass__ =
{
    DK_CLASS_OBJECT,

    DKInterfaceGetInterface,
    
    DKDoNothingRetain,
    DKDoNothingRelease,
    
    DKDisallowAllocate,
    DKDisallowInitialize,
    DKDisallowFinalize,

    DKPtrEqual,
    DKPtrCompare,
    DKPtrHash
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
    return DKNewObject( DKObjectClass(), sizeof(DKObjectHeader), 0 );
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
        assert( DKGetClass( ref ) == &__DKClassClass__ );
        
        // This makes class.getInterface() the same as calling instance.getInterface()
        const DKObjectInterface * objectInterface = ref;
        return objectInterface->getInterface( ref, suid );
    }

    return NULL;
}


///
//  DKInterfaceGetInterface()
//
static DKTypeRef DKInterfaceGetInterface( DKTypeRef ref, DKSUID suid )
{
    return NULL;
}


///
//  DKDoNothingRetain()
//
DKTypeRef DKDoNothingRetain( DKTypeRef ref )
{
    return ref;
}


///
//  DKDoNothingRelease()
//
void DKDoNothingRelease( DKTypeRef ref )
{
}


///
//  DKDisallowAllocate()
//
DKTypeRef DKDisallowAllocate( void )
{
    assert( 0 );
    return NULL;
}


///
//  DKDisallowInitialize()
//
DKTypeRef DKDisallowInitialize( DKTypeRef ref )
{
    assert( 0 );
    return ref;
}


///
//  DKDisallowFinalize()
//
void DKDisallowFinalize( DKTypeRef ref )
{
    assert( 0 );
}




// DKObject Polymorphic Wrappers =========================================================

///
//  DKNewObject()
//
DKTypeRef DKNewObject( DKTypeRef _class, size_t size, int flags )
{
    assert( _class );
    
    DKObjectHeader * obj = DKAlloc( size );
    
    obj->_isa = _class;
    obj->_refcount = 1;
    obj->_flags = flags;
    
    return obj;
}


///
//  DKCreate()
//
DKTypeRef DKCreate( DKTypeRef _class )
{
    const DKObjectInterface * classObject = _class;

    if( classObject )
    {
        DKTypeRef obj = classObject->allocate();
        return classObject->initialize( obj );
    }
    
    return NULL;
}


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
            if( obj->_isa == &__DKClassClass__ )
                return obj;
            
            return obj->_isa;
        }

        const DKObjectInterface * objectInterface = obj->_isa;
        return objectInterface->getInterface( obj, suid );
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
    if( a == b )
    {
        return 1;
    }

    const DKObjectHeader * obj = a;

    if( obj )
    {
        const DKObjectInterface * objectInterface = obj->_isa;
        return objectInterface->equal( a, b );
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

    const DKObjectHeader * obj = a;

    if( obj )
    {
        const DKObjectInterface * objectInterface = obj->_isa;
        return objectInterface->compare( a, b );
    }
    
    return a < b ? -1 : 1;
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
//  DKTestFlag()
//
int DKTestFlag( DKTypeRef ref, int flag )
{
    DKObjectHeader * obj = (DKObjectHeader *)ref;
    
    if( obj )
    {
        return (obj->_flags & flag) != 0;
    }
    
    return 0;
}







