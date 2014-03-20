//
//  DKObject.c
//  DK
//
//  Created by Derek Nylen on 2013-04-11.
//  Copyright (c) 2013 Hurlybird Media. All rights reserved.
//

#include "DKObject.h"



// Base Classes ==========================================================================

const DKInterface __DKEmptyInterfaceTable__[] =
{
    DK_INTERFACE_TABLE_END
};

const DKMethod __DKEmptyMethodTable__[] =
{
    DK_METHOD_TABLE_END
};

const DKProperty __DKEmptyPropertyTable__[] =
{
    DK_PROPERTY_TABLE_END
};

const DKClass __DKClassClass__ =
{
    DK_STATIC_CLASS_OBJECT,
    
    DK_EMPTY_INTERFACE_TABLE,
    DK_EMPTY_METHOD_TABLE,
    DK_EMPTY_PROPERTY_TABLE,
    
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

    DK_EMPTY_INTERFACE_TABLE,
    DK_EMPTY_METHOD_TABLE,
    DK_EMPTY_PROPERTY_TABLE,
    
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

    DK_EMPTY_INTERFACE_TABLE,
    DK_EMPTY_METHOD_TABLE,
    DK_EMPTY_PROPERTY_TABLE,
    
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
    
    DK_EMPTY_INTERFACE_TABLE,
    DK_EMPTY_METHOD_TABLE,
    DK_EMPTY_PROPERTY_TABLE,
    
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
DKTypeRef DKObjectGetInterface( DKTypeRef ref, DKSUID suid )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKClass * classObject = obj->isa;
        const DKInterface * interfaces = classObject->interfaces;
        
        for( int i = 0; interfaces[i].suid != NULL; ++i )
        {
            if( interfaces[i].suid == suid )
                return interfaces[i].interface;
        }

        for( int i = 0; interfaces[i].suid != NULL; ++i )
        {
            if( strcmp( interfaces[i].suid->selector, suid->selector ) == 0 )
                return interfaces[i].interface;
        }
    }
    
    return NULL;
}


///
//  DKObjectGetMethod()
//
DKTypeRef DKObjectGetMethod( DKTypeRef ref, DKSUID suid )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKClass * classObject = obj->isa;
        const DKMethod * methods = classObject->methods;
        
        for( int i = 0; methods[i].suid != NULL; ++i )
        {
            if( methods[i].suid == suid )
                return methods[i].method;
        }

        for( int i = 0; methods[i].suid != NULL; ++i )
        {
            if( strcmp( methods[i].suid->selector, suid->selector ) == 0 )
                return methods[i].method;
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
DKTypeRef DKGetInterface( DKTypeRef ref, DKSUID suid )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKClass * classObject = obj->isa;
        return classObject->getInterface( ref, suid );
    }
    
    return NULL;
}


///
//  DKGetMethod()
//
DKTypeRef DKGetMethod( DKTypeRef ref, DKSUID suid )
{
    const DKObjectHeader * obj = ref;
    
    if( obj )
    {
        const DKClass * classObject = obj->isa;
        return classObject->getMethod( ref, suid );
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







