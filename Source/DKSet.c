//
//  DKSet.c
//  Duck
//
//  Created by Derek Nylen on 2014-04-06.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKSet.h"
#include "DKHashTable.h"
#include "DKArray.h"
#include "DKString.h"
#include "DKStream.h"


DKThreadSafeSelectorInit( Set );


struct DKSet
{
    const DKObject * _obj;
};


///
//  DKSetClass()
//
static DKClassRef DefaultSetClass = NULL;

DKClassRef DKSetClass( void )
{
    if( DefaultSetClass )
        return DefaultSetClass;
    
    return DKHashTableClass();
}

void DKSetDefaultSetClass( DKClassRef _self )
{
    DefaultSetClass = _self;
}


///
//  DKMutableSetClass()
//
static DKClassRef DefaultMutableSetClass = NULL;

DKClassRef DKMutableSetClass( void )
{
    if( DefaultMutableSetClass )
        return DefaultMutableSetClass;
    
    return DKMutableHashTableClass();
}

void DKSetDefaultMutableSetClass( DKClassRef _self )
{
    DefaultMutableSetClass = _self;
}



///
//  DKSetInitWithObject()
//
DKObjectRef DKSetInitWithObject( DKSetRef _self, DKObjectRef object )
{
    return DKSetInitWithCArray( _self, &object, 1 );
}


///
//  DKSetInitWithObjects()
//
DKObjectRef DKSetInitWithObjects( DKSetRef _self, ... )
{
    va_list arg_ptr;
    va_start( arg_ptr, _self );
    
    _self = DKSetInitWithVAObjects( _self, arg_ptr );
    
    va_end( arg_ptr );
    
    return _self;
}


///
//  DKSetInitWithVAObjects()
//
DKObjectRef DKSetInitWithVAObjects( DKSetRef _self, va_list objects )
{
    if( _self )
    {
        DKSetInterfaceRef setInterface = DKGetInterface( _self, DKSelector(Set) );
        _self = setInterface->initWithVAObjects( _self, objects );
    }
    
    return _self;
}


///
//  DKSetInitWithCArray()
//
DKObjectRef DKSetInitWithCArray( DKSetRef _self, DKObjectRef objects[], DKIndex count )
{
    if( _self )
    {
        DKSetInterfaceRef setInterface = DKGetInterface( _self, DKSelector(Set) );
        _self = setInterface->initWithCArray( _self, objects, count );
    }
    
    return _self;
}


///
//  DKSetInitWithCollection()
//
DKObjectRef DKSetInitWithCollection( DKSetRef _self, DKObjectRef srcCollection )
{
    if( _self )
    {
        DKSetInterfaceRef setInterface = DKGetInterface( _self, DKSelector(Set) );
        _self = setInterface->initWithCollection( _self, srcCollection );
    }
    
    return _self;
}


///
//  DKSetGetCount()
//
DKIndex DKSetGetCount( DKSetRef _self )
{
    if( _self )
    {
        DKSetInterfaceRef set = DKGetInterface( _self, DKSelector(Set) );
        return set->getCount( _self );
    }
    
    return 0;
}


///
//  DKSetGetMember()
//
DKObjectRef DKSetGetMember( DKSetRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKSetInterfaceRef set = DKGetInterface( _self, DKSelector(Set) );
        return set->getMember( _self, object );
    }
    
    return NULL;
}


///
//  DKSetContainsObject()
//
int DKSetContainsObject( DKSetRef _self, DKObjectRef object )
{
    return DKSetGetMember( _self, object ) != NULL;
}


///
//  DKDSetAddObject()
//
void DKSetAddObject( DKMutableSetRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKSetInterfaceRef set = DKGetInterface( _self, DKSelector(Set) );
        return set->addObject( _self, object );
    }
}


///
//  DKDSetRemoveObject()
//
void DKSetRemoveObject( DKMutableSetRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKSetInterfaceRef set = DKGetInterface( _self, DKSelector(Set) );
        return set->removeObject( _self, object );
    }
}


///
//  DKSetRemoveAllObjects()
//
void DKSetRemoveAllObjects( DKMutableSetRef _self )
{
    if( _self )
    {
        DKSetInterfaceRef set = DKGetInterface( _self, DKSelector(Set) );
        return set->removeAllObjects( _self );
    }
}


///
//  DKSetEqualToSet()
//
bool DKSetEqualToSet( DKSetRef _self, DKSetRef otherSet )
{
    return (DKSetGetCount( _self ) == DKSetGetCount( otherSet )) &&
        DKSetIsSubsetOfSet( _self, otherSet );
}


///
//  DKSetIsSubsetOfSet()
//
static int DKSetIsSubsetOfSetCallback( DKObjectRef object, void * otherSet )
{
    return !DKSetContainsObject( otherSet, object );
}

int DKSetIsSubsetOfSet( DKSetRef _self, DKSetRef otherSet )
{
    DKAssertInterface( _self, DKSelector(Set) );
    
    return !DKForeachObject( _self, DKSetIsSubsetOfSetCallback, otherSet );
}


///
//  DKSetIntersectsSet()
//
static int DKSetIntersectsSetCallback( DKObjectRef object, void * otherSet )
{
    return DKSetContainsObject( otherSet, object );
}

int DKSetIntersectsSet( DKSetRef _self, DKSetRef otherSet )
{
    DKAssertInterface( _self, DKSelector(Set) );

    return !DKForeachObject( _self, DKSetIntersectsSetCallback, otherSet );
}


///
//  DKSetUnion()
//
static int DKSetUnionCallback( DKObjectRef object, void * unionSet )
{
    DKSetAddObject( unionSet, object );
    return 0;
}

void DKSetUnion( DKMutableSetRef _self, DKSetRef otherSet )
{
    DKForeachObject( otherSet, DKSetUnionCallback, _self );
}


///
//  DKSetMinus()
//
static int DKSetMinusCallback( DKObjectRef object, void * minusSet )
{
    DKSetRemoveObject( minusSet, object );
    return 0;
}

void DKSetMinus( DKMutableSetRef _self, DKSetRef otherSet )
{
    DKForeachObject( otherSet, DKSetMinusCallback, _self );
}


///
//  DKSetIntersect()
//
void DKSetIntersect( DKMutableSetRef _self, DKSetRef otherSet )
{
    if( _self )
    {
        DKSetInterfaceRef set = DKGetInterface( _self, DKSelector(Set) );
        DKSetInterfaceRef other = DKGetInterface( _self, DKSelector(Set) );

        DKArrayRef objects = DKArrayCreateWithCollection( DKArrayClass(), _self );
        
        DKIndex count = DKArrayGetCount( objects );
        
        for( DKIndex i = 0; i < count; i++ )
        {
            DKObjectRef object = DKArrayGetObjectAtIndex( objects, i );
            
            if( other->getMember( otherSet, object ) )
                set->addObject( _self, object );
        }
        
        DKRelease( objects );
    }
}




