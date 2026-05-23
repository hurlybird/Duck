// =======================================================================================
//
// DKBoolean.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKBoolean.h"
#include "DKNumber.h"
#include "DKString.h"
#include "DKStream.h"
#include "DKAllocation.h"
#include "DKDescription.h"
#include "DKEgg.h"



// DKBoolean =============================================================================

struct DKBoolean
{
    DKObject _obj;
    bool value; // This matches the layout of DKNumber
};

static struct DKBoolean DKBooleanTrue =
{
    DKInitStaticObjectHeader( NULL ),
    1
};

static struct DKBoolean DKBooleanFalse =
{
    DKInitStaticObjectHeader( NULL ),
    0
};


static void * DKBooleanAlloc( DKClassRef _class, size_t extraBytes );
static void DKBooleanDealloc( DKNumberRef _self );

static DKObjectRef DKBooleanInitWithEgg( DKBooleanRef _self, DKEggUnarchiverRef egg );
static void DKBooleanAddToEgg( DKBooleanRef _self, DKEggArchiverRef egg );


///
//  DKBooleanClass()
//
DKThreadSafeClassInit( DKBooleanClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKBoolean" ), DKNumberClass(), sizeof(struct DKBoolean),
        DKImmutableInstances | DKDisableReferenceCounting, NULL, NULL );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKNewInterface( DKSelector(Allocation) );
    allocation->alloc = (DKAllocMethod)DKBooleanAlloc;
    allocation->dealloc = (DKDeallocMethod)DKBooleanDealloc;

    DKInstallClassInterface( cls, allocation );
    DKRelease( allocation );
    
    // Description
    struct DKDescriptionInterface * description = DKNewInterface( DKSelector(Description) );
    description->getDescription = (DKGetDescriptionMethod)DKBooleanGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Egg
    struct DKEggInterface * egg = DKNewInterface( DKSelector(Egg) );
    egg->initWithEgg = (DKInitWithEggMethod)DKBooleanInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKBooleanAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKBooleanAlloc()
//
static void * DKBooleanAlloc( DKClassRef _class, size_t extraBytes )
{
    if( _class == DKBooleanClass_SharedObject )
        return DKFalse();
    
    DKAssert( 0 );
    return NULL;
}


///
//  DKBooleanDealloc()
//
static void DKBooleanDealloc( DKNumberRef _self )
{
}


///
//  DKTrue()
//
DKBooleanRef DKTrue( void )
{
    if( DKBooleanTrue._obj.isa == NULL )
    {
        DKSetObjectTag( &DKBooleanTrue, DKNumberBoolean );
        DKBooleanTrue._obj.isa = DKBooleanClass();
    }
    
    return &DKBooleanTrue;
}


///
//  DKFalse()
//
DKBooleanRef DKFalse( void )
{
    if( DKBooleanFalse._obj.isa == NULL )
    {
        DKSetObjectTag( &DKBooleanFalse, DKNumberBoolean );
        DKBooleanFalse._obj.isa = DKBooleanClass();
    }

    return &DKBooleanFalse;
}


///
//  DKBooleanInitWithEgg()
//
static DKObjectRef DKBooleanInitWithEgg( DKBooleanRef _self, DKEggUnarchiverRef egg )
{
    DKAssert( DKEggGetEncoding( egg, DKSTR( "value" ) ) == DKNumberBoolean );

    uint8_t value;
    DKEggGetNumberData( egg, DKSTR( "value" ), &value );
    
    return DKBoolean( value );
}


///
//  DKBooleanAddToEgg()
//
static void DKBooleanAddToEgg( DKBooleanRef _self, DKEggArchiverRef egg )
{
    DKEggAddNumberData( egg, DKSTR( "value" ), DKNumberBoolean, &_self->value );
}


///
//  DKBooleanGetDescription()
//
DKStringRef DKBooleanGetDescription( DKBooleanRef _self )
{
    if( _self == DKTrue() )
        return DKSTR( "true" );

    return DKSTR( "false" );
}







