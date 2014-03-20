//
//  DKObject.c
//  DK
//
//  Created by Derek Nylen on 2013-04-11.
//  Copyright (c) 2013 Hurlybird Media. All rights reserved.
//

#include "DKObject.h"



// Base Classes ==========================================================================

const DKClass __DKClassClass__ =
{
    DK_STATIC_CLASS_OBJECT,
    
    DKEmptyInterfaceTable(),
    DKEmptyMethodTable(),
    DKEmptyPropertyTable(),
    
    DKObjectGetInterface,
    DKObjectGetMethod,
    
    DKDoNothingRetain,
    DKDoNothingRelease,
    
    DKDoNothingAllocate,
    DKDoNothingInitialize,
    DKDoNothingFinalize,
    
    DKObjectEqual,
    DKObjectCompare,
    DKObjectHash
};

const DKClass __DKSelectorClass__ =
{
    DK_STATIC_CLASS_OBJECT,

    DKEmptyInterfaceTable(),
    DKEmptyMethodTable(),
    DKEmptyPropertyTable(),
    
    DKObjectGetInterface,
    DKObjectGetMethod,
    
    DKDoNothingRetain,
    DKDoNothingRelease,
    
    DKDoNothingAllocate,
    DKDoNothingInitialize,
    DKDoNothingFinalize,

    DKObjectEqual,
    DKObjectCompare,
    DKObjectHash
};

const DKClass __DKInterfaceClass__ =
{
    DK_STATIC_CLASS_OBJECT,

    DKEmptyInterfaceTable(),
    DKEmptyMethodTable(),
    DKEmptyPropertyTable(),
    
    DKObjectGetInterface,
    DKObjectGetMethod,
    
    DKDoNothingRetain,
    DKDoNothingRelease,
    
    DKDoNothingAllocate,
    DKDoNothingInitialize,
    DKDoNothingFinalize,

    DKObjectEqual,
    DKObjectCompare,
    DKObjectHash
};

const DKClass __DKMethodClass__ =
{
    DK_STATIC_CLASS_OBJECT,

    DKEmptyInterfaceTable(),
    DKEmptyMethodTable(),
    DKEmptyPropertyTable(),
    
    DKObjectGetInterface,
    DKObjectGetMethod,
    
    DKDoNothingRetain,
    DKDoNothingRelease,
    
    DKDoNothingAllocate,
    DKDoNothingInitialize,
    DKDoNothingFinalize,

    DKObjectEqual,
    DKObjectCompare,
    DKObjectHash
};


const DKClass __DKObjectClass__ =
{
    DK_STATIC_CLASS_OBJECT,
    
    DKEmptyInterfaceTable(),
    DKEmptyMethodTable(),
    DKEmptyPropertyTable(),
    
    DKObjectGetInterface,
    DKObjectGetMethod,
    
    DKObjectRetain,
    DKObjectRelease,
    
    DKObjectAllocate,
    DKObjectInitialize,
    DKObjectFinalize,
    
    DKObjectEqual,
    DKObjectCompare,
    DKObjectHash
};




// DKObject ==============================================================================

///
//  DKObjectGetInterface()
//
DKTypeRef DKObjectGetInterface( DKTypeRef ref, DKSEL sel )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKClass * classObject = obj->isa;
        
        for( int i = 0; i < classObject->interfaceCount; ++i )
        {
            const DKInterface * interface = classObject->interfaces[i];
        
            if( interface->sel == sel )
                return interface;
        }

        for( int i = 0; i < classObject->interfaceCount; ++i )
        {
            const DKInterface * interface = classObject->interfaces[i];

            if( strcmp( interface->sel->suid, sel->suid ) == 0 )
                return interface;
        }
    }
    
    return NULL;
}


///
//  DKObjectGetMethod()
//
DKTypeRef DKObjectGetMethod( DKTypeRef ref, DKSEL sel )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKClass * classObject = obj->isa;
        
        for( int i = 0; i < classObject->methodCount; ++i )
        {
            const DKMethod * method = classObject->methods[i];
        
            if( method->sel == sel )
                return method;
        }

        for( int i = 0; i < classObject->methodCount; ++i )
        {
            const DKMethod * method = classObject->methods[i];

            if( strcmp( method->sel->suid, sel->suid ) == 0 )
                return method;
        }
    }
    
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
        DKAtomicIncrement( &obj->refcount );
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
        DKAtomicInt n = DKAtomicDecrement( &obj->refcount );
        
        assert( n >= 0 );
        
        if( n == 0 )
        {
            const DKClass * classObject = obj->isa;
            classObject->finalize( obj );
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
    return ref;
}


///
//  DKObjectFinalize()
//
void DKObjectFinalize( DKTypeRef ref )
{
}




// DKClass and DKInterface ===============================================================

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
//  DKDoNothingAllocate()
//
DKTypeRef DKDoNothingAllocate( void )
{
    return NULL;
}


///
//  DKDoNothingInitialize()
//
DKTypeRef DKDoNothingInitialize( DKTypeRef ref )
{
    return ref;
}


///
//  DKDoNothingFinalize()
//
void DKDoNothingFinalize( DKTypeRef ref )
{
}




// DKObject Polymorphic Wrappers =========================================================

///
//  DKNewObject()
//
DKTypeRef DKNewObject( DKTypeRef _class, size_t size, int attributes )
{
    assert( _class );
    
    DKObjectHeader * obj = DKAlloc( size );
    
    obj->isa = _class;
    obj->refcount = 1;
    obj->attributes = attributes;
    
    return obj;
}


///
//  DKCreate()
//
DKTypeRef DKCreate( DKTypeRef _class )
{
    const DKClass * classObject = _class;

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
        return obj->isa;
    }
    
    return NULL;
}


///
//  DKGetInterface()
//
DKTypeRef DKGetInterface( DKTypeRef ref, DKSEL sel )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKClass * classObject = obj->isa;
        return classObject->getInterface( ref, sel );
    }
    
    return NULL;
}


///
//  DKGetMethod()
//
static void DKMethodNotFoundProc( DKTypeRef ref, DKSEL sel )
{
    assert( 0 );
}

static const DKMethod DKMethodNotFound =
{
    { &__DKMethodClass__, 1 },
    NULL,
    DKMethodNotFoundProc
};

DKTypeRef DKGetMethod( DKTypeRef ref, DKSEL sel )
{
    DKTypeRef method = NULL;
    
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKClass * classObject = obj->isa;
        method = classObject->getMethod( ref, sel );
    }
    
    return method ? method : &DKMethodNotFound;
}


///
//  DKRetain()
//
DKTypeRef DKRetain( DKTypeRef ref )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKClass * classObject = obj->isa;
        return classObject->retain( obj );
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
        const DKClass * classObject = obj->isa;
        classObject->release( obj );
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
        const DKClass * classObject = obj->isa;
        return classObject->equal( a, b );
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
        const DKClass * classObject = obj->isa;
        return classObject->compare( a, b );
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
        const DKClass * classObject = obj->isa;
        return classObject->hash( ref );
    }
    
    return 0;
}


///
//  DKTestAttribute()
//
int DKTestAttribute( DKTypeRef ref, int attr )
{
    DKObjectHeader * obj = (DKObjectHeader *)ref;
    
    if( obj )
    {
        return (obj->attributes & attr) != 0;
    }
    
    return 0;
}







