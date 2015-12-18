/*****************************************************************************************

  DKRuntime.c

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

#define DK_RUNTIME_PRIVATE 1

#include "DKPlatform.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"
#include "DKString.h"
#include "DKHashTable.h"
#include "DKArray.h"
#include "DKStream.h"
#include "DKEgg.h"
#include "DKAllocation.h"
#include "DKComparison.h"
#include "DKConversion.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKLocking.h"
#include "DKThread.h"



// Internal Types ========================================================================

static void DKClassFinalize( DKObjectRef _self );




// Root Classes ==========================================================================
static struct DKClass __DKRootClass__;
static struct DKClass __DKClassClass__;
static struct DKClass __DKSelectorClass__;
static struct DKClass __DKInterfaceClass__;
static struct DKClass __DKMsgHandlerClass__;
static struct DKClass __DKMetadataClass__;
static struct DKClass __DKObjectClass__;


DKClassRef DKRootClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKRootClass__;
}

DKClassRef DKClassClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKClassClass__;
}

DKClassRef DKSelectorClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKSelectorClass__;
}

DKClassRef DKInterfaceClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKInterfaceClass__;
}

DKClassRef DKMsgHandlerClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKMsgHandlerClass__;
}

DKClassRef DKMetadataClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKMetadataClass__;
}

DKClassRef DKObjectClass( void )
{
    DKRequire( DKRuntimeIsInitialized() );
    return &__DKObjectClass__;
}




// Interfaces Required for the Root Classes ==============================================
#define DKStaticSelectorInit( name )                                                    \
    static struct _DKSEL DKSelector_ ## name ##_StaticObject =                          \
    {                                                                                   \
        DKInitStaticObjectHeader( &__DKSelectorClass__ ),                               \
        NULL,                                                                           \
        DKStaticCache_ ## name                                                          \
    };                                                                                  \
                                                                                        \
    DKSEL DKSelector_ ## name( void )                                                   \
    {                                                                                   \
        return &DKSelector_ ## name ##_StaticObject;                                    \
    }


#define DKStaticInterfaceObject( sel )                                                  \
    {                                                                                   \
        DKInitStaticObjectHeader( &__DKInterfaceClass__ ),                              \
        sel                                                                             \
    }


DKStaticSelectorInit( Allocation );
DKStaticSelectorInit( Comparison );
DKStaticSelectorInit( Copying );
DKStaticSelectorInit( Description );
DKStaticSelectorInit( Locking );
DKStaticSelectorInit( Stream );
DKStaticSelectorInit( Egg );
DKStaticSelectorInit( Conversion );


// DefaultAllocation ---------------------------------------------------------------------
static struct DKAllocationInterface DKDefaultAllocation_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Allocation_StaticObject ),
    DKAllocObject,
    DKDeallocObject,
};

DKInterfaceRef DKDefaultAllocation( void )
{
    return &DKDefaultAllocation_StaticObject;
}


// DefaultComparison ---------------------------------------------------------------------
static struct DKComparisonInterface DKDefaultComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKPointerEqual,
    DKPointerCompare,
    DKPointerHash
};

DKInterfaceRef DKDefaultComparison( void )
{
    return &DKDefaultComparison_StaticObject;
}


// DefaultCopying ------------------------------------------------------------------------
static struct DKCopyingInterface DKDefaultCopying_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Copying_StaticObject ),
    DKRetain,
    (DKMutableCopyMethod)DKRetain
};

DKInterfaceRef DKDefaultCopying( void )
{
    return &DKDefaultCopying_StaticObject;
}


// DefaultDescription --------------------------------------------------------------------
static struct DKDescriptionInterface DKDefaultDescription_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Description_StaticObject ),
    DKDefaultGetDescription,
    DKDefaultGetSizeInBytes
};

DKInterfaceRef DKDefaultDescription( void )
{
    return &DKDefaultDescription_StaticObject;
}


// DefaultLocking ------------------------------------------------------------------------
static struct DKLockingInterface DKDefaultLocking_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Locking_StaticObject ),
    DKLockObject,
    DKUnlockObject
};

DKInterfaceRef DKDefaultLocking( void )
{
    return &DKDefaultLocking_StaticObject;
}


// Selector Comparison -------------------------------------------------------------------
static struct DKComparisonInterface DKSelectorComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    DKPointerEqual,
    DKPointerCompare,
    DKPointerHash
};

static DKInterfaceRef DKSelectorComparison( void )
{
    return &DKSelectorComparison_StaticObject;
}


// Interface Comparison ------------------------------------------------------------------
static int DKInterfaceEqual( DKInterface * a, DKInterface * b )
{
    return DKSelectorEqual( a->sel, b->sel );
}

static int DKInterfaceCompare( DKInterface * a, DKInterface * b )
{
    return DKSelectorCompare( a->sel, b->sel );
}

static DKHashCode DKInterfaceHash( DKInterface * a )
{
    return DKSelectorHash( a->sel );
}

static struct DKComparisonInterface DKInterfaceComparison_StaticObject =
{
    DKStaticInterfaceObject( &DKSelector_Comparison_StaticObject ),
    (DKEqualityMethod)DKInterfaceEqual,
    (DKCompareMethod)DKInterfaceCompare,
    (DKHashMethod)DKInterfaceHash
};

static DKInterfaceRef DKInterfaceComparison( void )
{
    return &DKInterfaceComparison_StaticObject;
}




// Runtime Init ==========================================================================
static bool _DKRuntimeIsInitialized = false;


///
//  DKRuntimeIsInitialized()
//
bool DKRuntimeIsInitialized( void )
{
    return _DKRuntimeIsInitialized;
}


///
//  InstallRootClassClassInterface()
//
static void InstallRootClassClassInterface( struct DKClass * _class, DKInterfaceRef interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKGenericHashTableInsert( &_class->classInterfaces.interfaces, &interface, DKInsertAlways );
}


///
//  InstallRootClassInstanceInterface()
//
static void InstallRootClassInstanceInterface( struct DKClass * _class, DKInterfaceRef interface )
{
    // Bypass the normal installation process here since the classes that allow it to
    // work haven't been fully initialized yet.
    DKGenericHashTableInsert( &_class->instanceInterfaces.interfaces, &interface, DKInsertAlways );
}


///
//  InitRootClass()
//
static void InitRootClass( struct DKClass * cls, struct DKClass * superclass, size_t structSize,
    uint32_t options, DKInitMethod init, DKFinalizeMethod finalize )
{
    memset( cls, 0, sizeof(struct DKClass) );
    
    DKObject * obj = (DKObject *)cls;
    obj->isa = &__DKRootClass__;
    obj->refcount = DKRefCountDisabledBit | 1;

    cls->name = NULL;
    cls->superclass = DKRetain( superclass );
    cls->structSize = (uint32_t)structSize;
    cls->options = options;
    cls->init = init;
    cls->finalize = finalize;

    DKInterfaceTableInit( &cls->classInterfaces, superclass ? &superclass->classInterfaces : NULL );
    DKInterfaceTableInit( &cls->instanceInterfaces, superclass ? &superclass->instanceInterfaces : NULL );

    InstallRootClassClassInterface( cls, DKDefaultAllocation() );
    InstallRootClassInstanceInterface( cls, DKDefaultComparison() );
    InstallRootClassInstanceInterface( cls, DKDefaultCopying() );
    InstallRootClassInstanceInterface( cls, DKDefaultDescription() );
    
    cls->propertiesLock = DKSpinLockInit;
}


///
//  SetRootClassName()
//
static void SetRootClassName( struct DKClass * _class, DKStringRef name )
{
    _class->name = DKCopy( name );
    DKNameDatabaseInsertClass( _class );
}


///
//  SetStaticSelectorName()
//
static void SetStaticSelectorName( struct _DKSEL * sel, DKStringRef name )
{
    sel->name = DKCopy( name );
    DKNameDatabaseInsertSelector( sel );
}


///
//  DKRuntimeInit()
//
void DKRuntimeInit( void )
{
    if( !_DKRuntimeIsInitialized )
    {
        _DKRuntimeIsInitialized = true;

        // Initialize the main thread context
        DKMainThreadContextInit();

        // Initialize the root classes
        InitRootClass( &__DKRootClass__,       NULL,                  sizeof(struct DKClass),   DKPreventSubclassing | DKAbstractBaseClass | DKDisableReferenceCounting, NULL, DKClassFinalize );
        InitRootClass( &__DKClassClass__,      NULL,                  sizeof(struct DKClass),   DKPreventSubclassing, NULL, DKClassFinalize );
        InitRootClass( &__DKSelectorClass__,   NULL,                  sizeof(struct _DKSEL),    DKPreventSubclassing, NULL, DKSelectorFinalize );
        InitRootClass( &__DKInterfaceClass__,  NULL,                  sizeof(DKInterface),      0, NULL, DKInterfaceFinalize );
        InitRootClass( &__DKMsgHandlerClass__, &__DKInterfaceClass__, sizeof(DKMsgHandler),     DKPreventSubclassing, NULL, NULL );
        InitRootClass( &__DKMetadataClass__,   NULL,                  sizeof(struct DKMetadata),DKPreventSubclassing, NULL, NULL );
        InitRootClass( &__DKObjectClass__,     NULL,                  sizeof(DKObject),         0, NULL, NULL );
        
        // Install custom comparison for selectors, interfaces and message handlers
        InstallRootClassInstanceInterface( &__DKSelectorClass__, DKSelectorComparison() );
        InstallRootClassInstanceInterface( &__DKInterfaceClass__, DKInterfaceComparison() );
        InstallRootClassInstanceInterface( &__DKMsgHandlerClass__, DKInterfaceComparison() );

        // Install default locking for instances of DKObject
        InstallRootClassInstanceInterface( &__DKObjectClass__, DKDefaultLocking() );

        // Initialize the name database
        DKNameDatabaseInit();

        // Initialize the base class names now that constant strings are available.
        SetRootClassName( &__DKRootClass__, DKSTR( "DKRootClass" ) );
        SetRootClassName( &__DKClassClass__, DKSTR( "DKClass" ) );
        SetRootClassName( &__DKSelectorClass__, DKSTR( "DKSelector" ) );
        SetRootClassName( &__DKInterfaceClass__, DKSTR( "DKInterface" ) );
        SetRootClassName( &__DKMsgHandlerClass__, DKSTR( "DKMsgHandler" ) );
        SetRootClassName( &__DKObjectClass__, DKSTR( "DKObject" ) );
        SetRootClassName( &__DKMetadataClass__, DKSTR( "DKMetadata" ) );

        SetStaticSelectorName( &DKSelector_Allocation_StaticObject, DKSTR( "Allocation" ) );
        SetStaticSelectorName( &DKSelector_Comparison_StaticObject, DKSTR( "Comparison" ) );
        SetStaticSelectorName( &DKSelector_Copying_StaticObject, DKSTR( "Copying" ) );
        SetStaticSelectorName( &DKSelector_Conversion_StaticObject, DKSTR( "Conversion" ) );
        SetStaticSelectorName( &DKSelector_Description_StaticObject, DKSTR( "Description" ) );
        SetStaticSelectorName( &DKSelector_Stream_StaticObject, DKSTR( "Stream" ) );
        SetStaticSelectorName( &DKSelector_Egg_StaticObject, DKSTR( "Egg" ) );
        
        SetRootClassName( (struct DKClass *)DKStringClass(), DKSTR( "DKString" ) );
        SetRootClassName( (struct DKClass *)DKConstantStringClass(), DKSTR( "DKConstantString" ) );

        // Initialize the main thread object
        DKThreadGetCurrentThread();
        
        // Initialize the object metadata table
        DKMetadataTableInit();
    }
}




// Classes ===============================================================================

///
//  DKNewClass()
//
DKClassRef DKNewClass( DKStringRef name, DKClassRef superclass, size_t structSize,
    uint32_t options, DKInitMethod init, DKFinalizeMethod finalize )
{
    if( superclass && ((superclass->options & DKPreventSubclassing) != 0) )
    {
        DKFatalError( "DKNewClass: Class '%s' does not allow subclasses.\n", DKStringGetCStringPtr( superclass->name ) );
        return NULL;
    }
    
    if( structSize == 0 )
        structSize = superclass ? superclass->structSize : sizeof(DKObject);

    struct DKClass * cls = DKNew( DKClassClass() );

    cls->name = DKCopy( name );
    cls->superclass = DKRetain( superclass );
    cls->structSize = (uint32_t)structSize;
    cls->options = options;
    cls->init = init;
    cls->finalize = finalize;

    DKInterfaceTableInit( &cls->classInterfaces, superclass ? &superclass->classInterfaces : NULL );
    DKInterfaceTableInit( &cls->instanceInterfaces, superclass ? &superclass->instanceInterfaces : NULL );
    
    cls->propertiesLock = DKSpinLockInit;
    
    // Insert the class into the name database
    DKNameDatabaseInsertClass( cls );
    
    return cls;
}


///
//  DKClassFinalize()
//
static void DKClassFinalize( DKObjectRef _untyped_self )
{
    DKClassRef _self = _untyped_self;
    
    DKAssert( _self->_obj.isa == &__DKClassClass__ );

    DKNameDatabaseRemoveClass( _self );

    DKPrintf( "Finalizing class %@\n", _self->name );
    
    // Note: The finalizer chain is still running at this point so make sure to set
    // the members to NULL to avoid accessing dangling pointers.
    
    DKRelease( _self->name );
    _self->name = NULL;
    
    DKRelease( _self->superclass );
    _self->superclass = NULL;

    DKInterfaceTableFinalize( &_self->classInterfaces );
    DKInterfaceTableFinalize( &_self->instanceInterfaces );
    
    // Release properties
    DKRelease( _self->properties );
    _self->properties = NULL;
}




// Objects ===============================================================================

///
//  DKAllocObject()
//
DKObjectRef DKAllocObject( DKClassRef cls, size_t extraBytes )
{
    if( !cls )
    {
        DKError( "DKAllocObject: Specified class object is NULL.\n" );
        return NULL;
    }
    
    if( cls->structSize < sizeof(DKObject) )
    {
        DKFatalError( "DKAllocObject: Requested struct size is smaller than DKObject.\n" );
        return NULL;
    }
    
    if( (cls->options & DKAbstractBaseClass) != 0 )
    {
        DKFatalError( "DKAllocObject: Class '%s' is an abstract base class.\n", DKStringGetCStringPtr( cls->name ) );
        return NULL;
    }
    
    // Allocate the structure + extra bytes
    DKObject * obj = dk_malloc( cls->structSize + extraBytes );
    
    // Zero the structure bytes
    memset( obj, 0, cls->structSize );
    
    // Setup the object header
    obj->isa = DKRetain( cls );
    
    if( (cls->options & DKDisableReferenceCounting) != 0 )
        obj->refcount = DKRefCountDisabledBit | 1;
    
    else
        obj->refcount = 1;
    
    return obj;
}


///
//  DKDeallocObject()
//
void DKDeallocObject( DKObjectRef _self )
{
    DKObject * obj = _self;
    DKClassRef cls = obj->isa;
    
    DKAssert( obj );
    DKAssert( ((obj->refcount & DKRefCountMask) == 0) || ((obj->refcount & DKRefCountDisabledBit) != 0) );

    // Deallocate
    dk_free( obj );
    
    // Finally release the class object
    DKRelease( cls );
}


///
//  DKAllocEx()
//
DKObjectRef DKAllocEx( DKClassRef _class, size_t extraBytes )
{
    DKObject * obj = NULL;
    
    if( _class )
    {
        DKAllocationInterfaceRef allocation = DKGetClassInterface( _class, DKSelector(Allocation) );
        obj = allocation->alloc( _class, extraBytes );
    }
    
    return obj;
}


///
//  DKDealloc()
//
void DKDealloc( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;
        
        DKAllocationInterfaceRef allocation = DKGetClassInterface( obj->isa, DKSelector(Allocation) );
        allocation->dealloc( obj );
    }
}


///
//  DKInit()
//
DKObjectRef DKInit( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;
        
        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls->init )
                return cls->init( _self );
        }
    }
    
    return _self;
}


///
//  DKSuperInit()
//
DKObjectRef DKSuperInit( DKObjectRef _self, DKClassRef superclass )
{
    if( _self && superclass )
    {
        DKAssertKindOfClass( _self, superclass );

        for( DKClassRef cls = superclass; cls != NULL; cls = cls->superclass )
        {
            if( cls->init )
                return cls->init( _self );
        }
    }
    
    return _self;
}


///
//  DKFinalize()
//
void DKFinalize( DKObjectRef _self )
{
    if( _self )
    {
        DKObject * obj = _self;

        for( DKClassRef cls = obj->isa; cls != NULL; cls = cls->superclass )
        {
            if( cls->finalize )
                cls->finalize( obj );
        }
    }
}


///
//  DKLockObject()
//
void DKLockObject( DKObjectRef _self )
{
    if( _self )
    {
        DKMetadataRef metadata = DKMetadataFindOrInsert( _self );
        DKSpinLockLock( &metadata->spinLock );
    }
}


///
//  DKUnlockObject()
//
void DKUnlockObject( DKObjectRef _self )
{
    if( _self )
    {
        DKMetadataRef metadata = DKMetadataFindOrInsert( _self );
        DKSpinLockUnlock( &metadata->spinLock );
    }
}





