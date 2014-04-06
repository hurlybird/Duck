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
//  DKSetCopyObjects()
//
static int DKSetCopyObjectsCallback( DKObjectRef object, void * context )
{
    DKListAppendObject( context, object );
    return 0;
}

DKListRef DKSetCopyObjects( DKDictionaryRef _self )
{
    DKMutableListRef list = (DKMutableListRef)DKCreate( DKMutableArrayClass() );
    
    DKSetApplyFunction( _self, DKSetCopyObjectsCallback, (void *)list );
    
    return list;
}


///
//  DKSetApplyFunction()
//
int DKSetApplyFunction( DKSetRef _self, DKSetApplierFunction callback, void * context )
{
    if( _self )
    {
        DKSetInterfaceRef set = DKGetInterface( _self, DKSelector(Set) );
        return set->applyFunction( _self, callback, context );
    }
    
    return 0;
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
    return !DKSetApplyFunction( _self, DKSetIsSubsetOfSetCallback, (void *)otherSet );
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
    return !DKSetApplyFunction( _self, DKSetIntersectsSetCallback, (void *)otherSet );
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
    DKSetApplyFunction( otherSet, DKSetUnionCallback, _self );
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
    DKSetApplyFunction( otherSet, DKSetMinusCallback, _self );
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

        DKArrayRef objects = DKSetCopyObjects( _self );
        
        DKIndex count = DKArrayGetCount( objects );
        
        for( DKIndex i = 0; i < count; i++ )
        {
            DKObjectRef object;
            DKArrayGetObjects( objects, DKRangeMake( i, 1 ), &object );
            
            if( other->getMember( otherSet, object ) )
                set->addObject( _self, object );
        }
        
        DKRelease( objects );
    }
}


///
//  DKSetCopyDescription()
//
struct PrintDescriptionContext
{
    DKMutableObjectRef stream;
    DKIndex n;
};

static int PrintDescriptionCallback( DKObjectRef object, void * context )
{
    struct PrintDescriptionContext * ctx = context;

    const char * prefix = (ctx->n == 0) ? "\n    " : ",\n    ";

    if( DKIsKindOfClass( object, DKStringClass() ) )
        DKSPrintf( ctx->stream, "%s\"%@\" = ", prefix, object );
    
    else
        DKSPrintf( ctx->stream, "%s%@ = ", prefix, object );

    ctx->n++;
    
    return 0;
}

DKStringRef DKSetCopyDescription( DKListRef _self )
{
    DKMutableStringRef desc = DKStringCreateMutable();
    
    DKIndex count = DKListGetCount( _self );
    
    DKSPrintf( desc, "%@, %d objects = [", DKGetClassName( _self ), count );
    
    struct PrintDescriptionContext context = { desc, 0 };
    DKSetApplyFunction( _self, PrintDescriptionCallback, &context );
    
    DKSPrintf( desc, "\n]\n" );
    
    return desc;
}




