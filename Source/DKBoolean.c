/*****************************************************************************************

  DKBoolean.c

  Copyright (c) 2014 Derek W. Nylen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*****************************************************************************************/

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
    uint8_t value;
};

static struct DKBoolean DKBooleanTrue =
{
    DKInitObjectHeader( NULL ),
    1
};

static struct DKBoolean DKBooleanFalse =
{
    DKInitObjectHeader( NULL ),
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
    DKClassRef cls = DKAllocClass( DKSTR( "DKBoolean" ), DKNumberClass(), sizeof(struct DKBoolean),
        DKImmutableInstances | DKDisableReferenceCounting, NULL, NULL );
    
    // Allocation
    struct DKAllocationInterface * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(struct DKAllocationInterface) );
    allocation->alloc = (DKAllocMethod)DKBooleanAlloc;
    allocation->dealloc = (DKDeallocMethod)DKBooleanDealloc;

    DKInstallClassInterface( cls, allocation );
    DKRelease( allocation );
    
    // Description
    struct DKDescriptionInterface * description = DKAllocInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKBooleanGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Egg
    struct DKEggInterface * egg = DKAllocInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
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
        DKBooleanTrue._obj.isa = DKBooleanClass();
        DKSetObjectTag( &DKBooleanTrue, DKNumberUInt8 );
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
        DKBooleanFalse._obj.isa = DKBooleanClass();
        DKSetObjectTag( &DKBooleanFalse, DKNumberUInt8 );
    }

    return &DKBooleanFalse;
}


///
//  DKBooleanInitWithEgg()
//
static DKObjectRef DKBooleanInitWithEgg( DKBooleanRef _self, DKEggUnarchiverRef egg )
{
    DKAssert( DKEggGetEncoding( egg, DKSTR( "value" ) ) == DKNumberUInt8 );

    uint8_t value;
    DKEggGetNumberData( egg, DKSTR( "value" ), &value );
    
    return DKBoolean( value );
}


///
//  DKBooleanAddToEgg()
//
static void DKBooleanAddToEgg( DKBooleanRef _self, DKEggArchiverRef egg )
{
    DKEggAddNumberData( egg, DKSTR( "value" ), DKNumberUInt8, &_self->value );
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







