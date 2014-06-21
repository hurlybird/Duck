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
//  DKSetCreateWithObjects()
//
DKObjectRef DKSetCreateWithObjects( DKClassRef _class, DKObjectRef firstObject, ... )
{
    if( _class == NULL )
        _class = DKSetClass();
    
    DKSetInterfaceRef set = DKGetInterface( _class, DKSelector(Set) );
    
    va_list arg_ptr;
    va_start( arg_ptr, firstObject );
    
    DKObjectRef obj = set->createWithVAObjects( _class, arg_ptr );
    
    va_end( arg_ptr );
    
    return obj;
}


///
//  DKSetCreateWithVAObjects()
//
DKObjectRef DKSetCreateWithVAObjects( DKClassRef _class, va_list objects )
{
    if( _class == NULL )
        _class = DKSetClass();
    
    DKSetInterfaceRef set = DKGetInterface( _class, DKSelector(Set) );
    
    return set->createWithVAObjects( _class, objects );
}


///
//  DKSetCreateWithCArray()
//
DKObjectRef DKSetCreateWithCArray( DKClassRef _class, DKObjectRef objects[], DKIndex count )
{
    if( _class == NULL )
        _class = DKSetClass();
    
    DKSetInterfaceRef set = DKGetInterface( _class, DKSelector(Set) );

    return set->createWithCArray( _class, objects, count );
}


///
//  DKSetCreateWithCollection()
//
DKObjectRef DKSetCreateWithCollection( DKClassRef _class, DKObjectRef srcCollection )
{
    if( _class == NULL )
        _class = DKSetClass();
    
    DKSetInterfaceRef set = DKGetInterface( _class, DKSelector(Set) );

    return set->createWithCollection( _class, srcCollection );
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
DKObjectRef DKSetGetMember( DKDictionaryRef _self, DKObjectRef object )
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
int DKSetContainsObject( DKDictionaryRef _self, DKObjectRef object )
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
//  DKSetIsSubsetOfSet()
//
static int DKSetIsSubsetOfSetCallback( DKObjectRef object, void * otherSet )
{
    return !DKSetContainsObject( otherSet, object );
}

int DKSetIsSubsetOfSet( DKSetRef _self, DKSetRef otherSet )
{
    return !DKForeachObject( _self, DKSetIsSubsetOfSetCallback, (void *)otherSet );
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
    return !DKForeachObject( _self, DKSetIntersectsSetCallback, (void *)otherSet );
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




