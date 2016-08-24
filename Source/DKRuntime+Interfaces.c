/*****************************************************************************************

  DKRuntime+Interfaces.c

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

#include "DKRuntime.h"
#include "DKString.h"
#include "DKComparison.h"
#include "DKCopying.h"



// Dynamic Message Handling ==============================================================
DKThreadSafeSelectorInit( DKDynamicMsgHandler );




// Error Handling ========================================================================

// Called when an interface vtable hasn't been properly initialized.
static void DKUninitializedMethodError( DKObjectRef _self )
{
    // Note: The '_self' pointer is for debugging only -- it may not be valid since
    // interface methods do not require it.

    DKFatalError( "DKRuntime: Calling an uninitialized interface method\n" );
}


// This handles sending messages to NULL objects.
DKDeclareMessageSelector( MsgHandlerNotFound );
DKThreadSafeSelectorInit( MsgHandlerNotFound );

static intptr_t DKMsgHandlerNotFoundMethod( DKObjectRef _self, DKSEL sel )
{
    return 0;
}

DKThreadSafeSharedObjectInit( DKMsgHandlerNotFound, DKMsgHandlerRef )
{
    struct DKMsgHandler * msgHandler = DKNewInterface( DKSelector(MsgHandlerNotFound), sizeof(struct DKMsgHandler) );

    msgHandler->func = DKMsgHandlerNotFoundMethod;

    return msgHandler;
}




// DKInterfaceTable ======================================================================

// Hash Table Callbacks
static DKRowStatus InterfaceTableRowStatus( const void * _row )
{
    DKInterface * const * row = _row;
    return (DKRowStatus)DK_HASHTABLE_ROW_STATUS( *row );
}

static DKHashCode InterfaceTableRowHash( const void * _row )
{
    DKInterface * const * row = _row;
    return DKObjectUniqueHash( (*row)->sel );
}

static bool InterfaceTableRowEqual( const void * _row1, const void * _row2 )
{
    DKInterface * const * row1 = _row1;
    DKInterface * const * row2 = _row2;

    return DKSelectorEqual( (*row1)->sel, (*row2)->sel );
}

static void InterfaceTableRowInit( void * _row )
{
    DKInterface ** row = _row;
    *row = DK_HASHTABLE_EMPTY_KEY;
}

static void InterfaceTableRowUpdate( void * _row, const void * _src )
{
    DKInterface ** row = _row;
    DKInterface * const * src = _src;
    
    DKRetain( *src );
    
    if( DK_HASHTABLE_IS_POINTER( *row ) )
        DKRelease( *row );
        
    *row = *src;
}

static void InterfaceTableRowDelete( void * _row )
{
    DKInterface ** row = _row;
    
    DKRelease( *row );
    *row = DK_HASHTABLE_DELETED_KEY;
}

static void InterfaceTableForeachRowCallback( const void * _row, void * context )
{
    struct DKInterfaceTable * interfaceTable = context;
    DKGenericHashTableInsert( &interfaceTable->interfaces, _row, DKInsertAlways );
}


///
//  DKInterfaceTableInit()
//
void DKInterfaceTableInit( struct DKInterfaceTable * interfaceTable, struct DKInterfaceTable * inheritedInterfaces )
{
    DKGenericHashTableCallbacks callbacks =
    {
        InterfaceTableRowStatus,
        InterfaceTableRowHash,
        InterfaceTableRowEqual,
        InterfaceTableRowInit,
        InterfaceTableRowUpdate,
        InterfaceTableRowDelete
    };

    memset( interfaceTable->cache, 0, sizeof(interfaceTable->cache) );

    interfaceTable->lock = DKSpinLockInit;

    DKGenericHashTableInit( &interfaceTable->interfaces, sizeof(DKObjectRef), &callbacks );
    
    if( inheritedInterfaces )
        DKGenericHashTableForeachRow( &inheritedInterfaces->interfaces, InterfaceTableForeachRowCallback, interfaceTable );
}


///
//  DKInterfaceTableFinalize()
//
void DKInterfaceTableFinalize( struct DKInterfaceTable * interfaceTable )
{
    DKGenericHashTableFinalize( &interfaceTable->interfaces );
}


///
//  DKInterfaceTableInsert()
//
void DKInterfaceTableInsert( DKClassRef _class, struct DKInterfaceTable * interfaceTable, DKInterfaceRef _interface )
{
    // *** WARNING ***
    // Swizzling interfaces on base classes isn't fully supported. In order to do so we
    // would need to update the interfaces tables of all subclasses.
    // *** WARNING ***
    
    DKAssertMemberOfClass( _class, DKClassClass() );
    DKAssertKindOfClass( _interface, DKInterfaceClass() );

    DKInterface * interface = (DKInterface *)_interface;

    // Get the cache line from the selector
    unsigned int cacheline = interface->sel->cacheline;
    DKAssert( cacheline < (DKStaticCacheSize + DKDynamicCacheSize) );
    
    // Invalidate the cache
    interfaceTable->cache[cacheline] = NULL;

    // Replace the interface in the interface table
    DKSpinLockLock( &interfaceTable->lock );
    DKGenericHashTableInsert( &interfaceTable->interfaces, &interface, DKInsertAlways );
    DKSpinLockUnlock( &interfaceTable->lock );
}


///
//  DKInterfaceTableFind()
//
DKInterface * DKInterfaceTableFind( DKClassRef _class, struct DKInterfaceTable * interfaceTable, DKSEL sel, DKInterfaceNotFoundCallback interfaceNotFound )
{
    DKAssert( sel->_obj.isa == DKSelectorClass() );

    // Get the cache line from the selector
    unsigned int cacheline = sel->cacheline;
    DKAssert( cacheline < (DKStaticCacheSize + DKDynamicCacheSize) );

    // We shoudn't need to acquire the spin lock while reading and writing to the cache
    // since the worst that can happen is doing an extra lookup after reading a stale
    // cache line.
    
    // The extra NULL check could be skipped by using a sentinal interface object instead
    // of NULL for empty cache lines, however doing so -might- result in worse CPU cache
    // behaviour by touching the sentinel's memory location.

    // Check the cached interface
    DKInterface * interface = interfaceTable->cache[cacheline];
    
    if( interface && DKSelectorEqual( interface->sel, sel ) )
        return interface;

    // Search our interface table
    DKInterface _key;
    _key.sel = sel;
    
    DKInterface * key = &_key;

    DKSpinLockLock( &interfaceTable->lock );
    DKInterface ** entry = (DKInterface **)DKGenericHashTableFind( &interfaceTable->interfaces, &key );
    DKSpinLockUnlock( &interfaceTable->lock );
    
    if( entry )
    {
        // Update the cache
        interfaceTable->cache[cacheline] = *entry;

        return *entry;
    }

    return interfaceNotFound( _class, sel );
}




// DKSelector ============================================================================

static DKSpinLock NextCacheLineSpinLock = DKSpinLockInit;
static unsigned int NextCacheLine = 0;


///
//  DKAllocSelector()
//
DKSEL DKAllocSelector( DKStringRef name )
{
    struct _DKSEL * sel = DKInit( DKAlloc( DKSelectorClass() ) );

    DKAssert( sel != NULL );

    sel->name = DKCopy( name );
    
    DKSpinLockLock( &NextCacheLineSpinLock );
    sel->cacheline = DKStaticCacheSize + (NextCacheLine % DKDynamicCacheSize);
    NextCacheLine++;
    DKSpinLockUnlock( &NextCacheLineSpinLock );

    DKNameDatabaseInsertSelector( sel );

    return sel;
}


///
//  DKSelectorFinalize()
//
void DKSelectorFinalize( DKObjectRef _untyped_self )
{
    DKSEL _self = _untyped_self;

    DKNameDatabaseRemoveSelector( _self );

    DKRelease( _self->name );
}




// DKInterface ===========================================================================

///
//  DKNewInterface()
//
DKInterfaceRef DKNewInterface( DKSEL sel, size_t structSize )
{
    if( sel )
    {
        size_t extraBytes = structSize - sizeof(DKInterface);
        
        DKInterface * interface = DKInit( DKAllocEx( DKInterfaceClass(), extraBytes ) );

        interface->sel = DKRetain( sel );
    
        // Init all the function pointers
        interface->methodCount = DKInterfaceCountMethods( structSize );
        void ** methods = DKInterfaceGetMethodTable( interface );
        
        for( size_t i = 0; i < interface->methodCount; i++ )
            methods[i] = DKUninitializedMethodError;
    
        return interface;
    }
    
    return NULL;
}


///
//  DKInterfaceInheritMethods()
//
static DKInterfaceRef DKSourceInterfaceNotFound( DKClassRef _class, DKSEL sel )
{
    return NULL;
}

void DKInterfaceInheritMethods( DKInterfaceRef interface, DKClassRef _class )
{
    DKAssert( interface && _class );
    
    DKInterface * dstInterface = interface;
    const DKInterface * srcInterface = DKInterfaceTableFind( _class, &_class->instanceInterfaces, dstInterface->sel, DKSourceInterfaceNotFound );

    if( srcInterface )
    {
        DKRequire( dstInterface->methodCount == srcInterface->methodCount );
        
        void ** dstMethods = DKInterfaceGetMethodTable( dstInterface );
        void ** srcMethods = DKInterfaceGetMethodTable( srcInterface );
        
        for( size_t i = 0; i < dstInterface->methodCount; i++ )
        {
            if( (dstMethods == NULL) || (dstMethods[i] == DKUninitializedMethodError) )
                dstMethods[i] = srcMethods[i];
        }
    }
}


///
//  DKInstallInterface()
//
void DKInstallInterface( DKClassRef _class, DKInterfaceRef _interface )
{
    DKInterfaceInheritMethods( _interface, _class->superclass );
    DKInterfaceTableInsert( _class, &_class->instanceInterfaces, _interface );
}


///
//  DKInterfaceFinalize()
//
void DKInterfaceFinalize( DKObjectRef _untyped_self )
{
    DKInterface * _self = _untyped_self;
    DKRelease( _self->sel );
}


///
//  DKInstallClassInterface()
//
void DKInstallClassInterface( DKClassRef _class, DKInterfaceRef _interface )
{
    DKInterfaceTableInsert( _class, &_class->classInterfaces, _interface );
}


///
//  DKGetInterface()
//
static DKInterfaceRef DKGetInterfaceNotFound( DKClassRef _class, DKSEL sel )
{
    DKFatalError( "DKRuntime: Interface '%@' not found on object '%@'\n", sel->name, _class->name );
    return NULL;
}

DKInterfaceRef DKGetInterface( DKObjectRef _self, DKSEL sel )
{
    // The NULL checks on the arguments are skipped here (and in the related functions
    // below) to eliminate branching in the interface lookup. Also, interface calls are
    // typically done inside a wrapper function that already does a NULL check of its own.

    DKAssert( (_self != NULL) && (sel != NULL) );

    const DKObject * obj = _self;
    DKClassRef cls = obj->isa;
    
    return DKInterfaceTableFind( cls, &cls->instanceInterfaces, sel, DKGetInterfaceNotFound );
}


///
//  DKGetClassInterface()
//
static DKInterfaceRef DKGetClassInterfaceNotFound( DKClassRef _class, DKSEL sel )
{
    DKFatalError( "DKRuntime: Class interface '%@' not found on object '%@'\n", sel->name, _class->name );
    return NULL;
}

DKInterfaceRef DKGetClassInterface( DKClassRef _class, DKSEL sel )
{
    DKAssert( (_class != NULL) && (sel != NULL) );
    DKAssert( (_class->_obj.isa == DKClassClass()) || (_class->_obj.isa == DKRootClass()) );

    return DKInterfaceTableFind( _class, &_class->classInterfaces, sel, DKGetClassInterfaceNotFound );
}


///
//  DKQueryInterface()
//
static DKInterfaceRef DKQueryInterfaceNotFound( DKClassRef _class, DKSEL sel )
{
    return NULL;
}

bool DKQueryInterface( DKObjectRef _self, DKSEL sel, DKInterfaceRef * _interface )
{
    DKAssert( (_self != NULL) && (sel != NULL) );

    const DKObject * obj = _self;
    DKClassRef cls = obj->isa;
    
    DKInterfaceRef interface = DKInterfaceTableFind( cls, &cls->instanceInterfaces, sel, DKQueryInterfaceNotFound );

    if( _interface )
        *_interface = interface;
    
    return interface != NULL;
}


///
//  DKQueryClassInterface()
//
bool DKQueryClassInterface( DKClassRef _class, DKSEL sel, DKInterfaceRef * _interface )
{
    DKAssert( (_class != NULL) && (sel != NULL) );
    DKAssert( (_class->_obj.isa == DKClassClass()) || (_class->_obj.isa == DKRootClass()) );
    
    DKInterfaceRef interface = DKInterfaceTableFind( _class, &_class->classInterfaces, sel, DKQueryInterfaceNotFound );

    if( _interface )
        *_interface = interface;
    
    return interface != NULL;
}




// DKMsgHandler ==========================================================================

///
//  DKInstallMsgHandler()
//
void DKInstallMsgHandler( DKClassRef _class, DKSEL sel, DKMsgFunction func )
{
    struct DKMsgHandler * msgHandler = DKInit( DKAllocEx( DKMsgHandlerClass(), sizeof(void *) ) );
    DKAssert( msgHandler != NULL );

    msgHandler->sel = DKRetain( sel );
    msgHandler->func = func;
    
    DKInterfaceTableInsert( _class, &_class->instanceInterfaces, msgHandler );
    
    DKRelease( msgHandler );
}


///
//  DKInstallClassMsgHandler()
//
void DKInstallClassMsgHandler( DKClassRef _class, DKSEL sel, DKMsgFunction func )
{
    struct DKMsgHandler * msgHandler = DKInit( DKAllocEx( DKMsgHandlerClass(), sizeof(void *) ) );
    DKAssert( msgHandler != NULL );

    msgHandler->sel = DKRetain( sel );
    msgHandler->func = func;
    
    DKInterfaceTableInsert( _class, &_class->classInterfaces, msgHandler );
    
    DKRelease( msgHandler );
}


///
//  DKGetMsgHandler()
//
static DKInterfaceRef DKGetMsgHandlerNotFound( DKClassRef _class, DKSEL sel )
{
    return DKMsgHandlerNotFound();
}

static DKInterfaceRef DKGetDynamicInstanceMsgHandler( DKClassRef _class, DKSEL sel )
{
    return DKInterfaceTableFind( _class, &_class->instanceInterfaces,
        DKSelector(DKDynamicMsgHandler), DKGetMsgHandlerNotFound );
}

static DKInterfaceRef DKGetDynamicClassMsgHandler( DKClassRef _class, DKSEL sel )
{
    return DKInterfaceTableFind( _class, &_class->classInterfaces,
        DKSelector(DKDynamicMsgHandler), DKGetMsgHandlerNotFound );
}

DKMsgHandlerRef DKGetMsgHandler( DKObjectRef _self, DKSEL sel )
{
    // Method calls (unlike interface calls) are often done from the DKMsgSend macro which
    // doesn't do a NULL check on _self.

    DKAssert( sel != NULL );

    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef _class = obj->isa;
        
        return (DKMsgHandlerRef)DKInterfaceTableFind( _class,
            &_class->instanceInterfaces, sel, DKGetDynamicInstanceMsgHandler );
    }

    return DKMsgHandlerNotFound();
}


///
//  DKGetClassMsgHandler()
//
DKMsgHandlerRef DKGetClassMsgHandler( DKClassRef _class, DKSEL sel )
{
    DKAssert( sel != NULL );

    if( _class )
    {
        DKAssert( (_class->_obj.isa == DKClassClass()) || (_class->_obj.isa == DKRootClass()) );

        return (DKMsgHandlerRef)DKInterfaceTableFind( _class,
            &_class->classInterfaces, sel, DKGetDynamicClassMsgHandler );
    }

    return DKMsgHandlerNotFound();
}


///
//  DKQueryMsgHandler()
//
static DKInterfaceRef DKQueryMsgHandlerNotFound( DKClassRef _class, DKSEL sel )
{
    return NULL;
}

static DKInterfaceRef DKQueryDynamicInstanceMsgHandler( DKClassRef _class, DKSEL sel )
{
    return DKInterfaceTableFind( _class, &_class->instanceInterfaces,
        DKSelector(DKDynamicMsgHandler), DKQueryMsgHandlerNotFound );
}

static DKInterfaceRef DKQueryDynamicClassMsgHandler( DKClassRef _class, DKSEL sel )
{
    return DKInterfaceTableFind( _class, &_class->classInterfaces,
        DKSelector(DKDynamicMsgHandler), DKQueryMsgHandlerNotFound );
}

bool DKQueryMsgHandler( DKObjectRef _self, DKSEL sel, DKMsgHandlerRef * _msgHandler )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef _class = obj->isa;
        
        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKInterfaceTableFind( _class,
            &_class->instanceInterfaces, sel, DKQueryDynamicInstanceMsgHandler );

        if( msgHandler )
        {
            DKAssertKindOfClass( msgHandler, DKMsgHandlerClass() );

            if( _msgHandler )
                *_msgHandler = msgHandler;
            
            return true;
        }
    }

    if( _msgHandler )
        *_msgHandler = DKMsgHandlerNotFound();

    return false;
}


///
//  DKQueryClassMsgHandler()
//
bool DKQueryClassMsgHandler( DKClassRef _class, DKSEL sel, DKMsgHandlerRef * _msgHandler )
{
    if( _class )
    {
        DKAssert( (_class->_obj.isa == DKClassClass()) || (_class->_obj.isa == DKRootClass()) );

        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKInterfaceTableFind( _class,
            &_class->classInterfaces, sel, DKQueryDynamicClassMsgHandler );

        if( msgHandler )
        {
            DKAssertKindOfClass( msgHandler, DKMsgHandlerClass() );

            if( _msgHandler )
                *_msgHandler = msgHandler;
            
            return true;
        }
    }

    if( _msgHandler )
        *_msgHandler = DKMsgHandlerNotFound();

    return false;
}








