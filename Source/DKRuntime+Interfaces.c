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


// Get a dynamic cache index for a selector
#define GetDynamicCacheline( sel )  (int)((DKObjectUniqueHash(sel) & (DKDynamicCacheSize-1)) + DKStaticCacheSize)

// Inline selector equality
#define DKFastSelectorEqual( a, b ) (a == b)




// DKSelector ============================================================================

///
//  DKAllocSelector()
//
DKSEL DKAllocSelector( DKStringRef name )
{
    struct _DKSEL * sel = DKInit( DKAlloc( DKSelectorClass(), 0 ) );

    DKAssert( sel != NULL );

    sel->name = DKCopy( name );
    sel->cacheline = DKDynamicCache;

    DKNameDatabaseInsertSelector( sel );

    return sel;
}


///
//  DKSelectorFinalize()
//
void DKSelectorFinalize( DKObjectRef _self )
{
    DKSEL sel = _self;

    DKNameDatabaseRemoveSelector( sel );

    DKRelease( sel->name );
}




// DKInterface ===========================================================================

///
//  DKAllocInterface()
//
void * DKAllocInterface( DKSEL sel, size_t structSize )
{
    if( sel )
    {
        size_t extraBytes = structSize - sizeof(DKInterface);
        
        // DKGetInterface returns a generic interface filled with DK_MAX_INTERFACE_SIZE
        // pointers to DKInterfaceNotFound. The size needs to be large enough to deal
        // with the largest interface or it'll cause an access error at run time.
        DKAssert( (extraBytes / sizeof(void *)) <= DK_MAX_INTERFACE_SIZE );
        
        DKInterface * interface = DKInit( DKAlloc( DKInterfaceClass(), extraBytes ) );
        DKAssert( interface != NULL );

        interface->sel = DKRetain( sel );
    
        // Init all the function pointers to NULL
        memset( interface + 1, 0, extraBytes );
    
        return interface;
    }
    
    return NULL;
}


///
//  DKInterfaceFinalize()
//
void DKInterfaceFinalize( DKObjectRef _self )
{
    DKInterface * interface = (DKInterface *)_self;
    DKRelease( interface->sel );
}


///
//  DKInstallInterfaceInGroup()
//
static void DKInstallInterfaceInGroup( DKClassRef _class, DKInterfaceRef _interface, struct DKInterfaceGroup * interfaceGroup )
{
    DKAssertMemberOfClass( _class, DKClassClass() );
    DKAssertKindOfClass( _interface, DKInterfaceClass() );

    DKInterface * interface = (DKInterface *)_interface;

    // Retain the new interface
    DKRetain( interface );

    // Resolve the cache line from the selector
    DKIndex cacheline = interface->sel->cacheline;
    DKAssert( (cacheline >= 0) && (cacheline < DKStaticCacheSize) );

    if( cacheline == DKDynamicCache )
        cacheline = GetDynamicCacheline( interface->sel );
    
    DKAssert( (cacheline > 0) && (cacheline < (DKStaticCacheSize + DKDynamicCacheSize)) );
    
    // Lock while we make changes
    DKSpinLockLock( &interfaceGroup->lock );
    
    // Invalidate the cache
    
    // *** WARNING ***
    // This doesn't invalidate the caches of any subclasses, so it's possible that
    // subclasses will still reference the old interface, or even crash if the old
    // interface is released (the cache doesn't maintain a reference).
    // *** WARNING ***
    
    interfaceGroup->cache[cacheline] = NULL;

    // Replace the interface in the interface table
    DKIndex count = interfaceGroup->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * oldInterface = DKGenericArrayGetElementAtIndex( &interfaceGroup->interfaces, i, DKInterface * );
        
        if( DKEqual( oldInterface->sel, interface->sel ) )
        {
            DKGenericArrayGetElementAtIndex( &interfaceGroup->interfaces, i, DKInterface * ) = interface;

            DKSpinLockUnlock( &interfaceGroup->lock );

            // Release the old interface after unlocking
            DKRelease( oldInterface );
            return;
        }
    }
    
    // Add the interface to the interface table
    DKGenericArrayAppendElements( &interfaceGroup->interfaces, &interface, 1 );

    DKSpinLockUnlock( &interfaceGroup->lock );
}


///
//  DKInstallInterface()
//
void DKInstallInterface( DKClassRef _class, DKInterfaceRef _interface )
{
    DKInstallInterfaceInGroup( _class, _interface, (struct DKInterfaceGroup *)&_class->instanceInterfaces );
}


///
//  DKInstallClassInterface()
//
void DKInstallClassInterface( DKClassRef _class, DKInterfaceRef _interface )
{
    DKInstallInterfaceInGroup( _class, _interface, (struct DKInterfaceGroup *)&_class->classInterfaces );
}


///
//  DKLookupInterfaceInGroup()
//
static DKInterface * DKLookupInterfaceInGroup( DKClassRef _class, DKSEL sel, struct DKInterfaceGroup * interfaceGroup )
{
    DKAssert( (_class->_obj.isa == DKClassClass()) || (_class->_obj.isa == DKRootClass()) );
    DKAssert( sel->_obj.isa == DKSelectorClass() );

    // Get the static cache line from the selector
    DKIndex cacheline = sel->cacheline;
    DKAssert( (cacheline >= 0) && (cacheline < DKStaticCacheSize) );

    // We shoudn't need to acquire the spin lock while reading and writing to the cache
    // since the worst that can happen is doing an extra lookup after reading a stale
    // cache line.
    #define SPIN_LOCKED_CACHE_ACCESS 0

    // Lock while we lookup the interface
    #if SPIN_LOCKED_CACHE_ACCESS
    DKSpinLockLock( &interfaceGroup->lock );
    #endif
    
    // First check the static cache (line 0 will always be NULL)
    DKInterface * interface = interfaceGroup->cache[cacheline];
    
    if( interface )
    {
        DKAssert( DKFastSelectorEqual( interface->sel, sel ) );
    
        #if SPIN_LOCKED_CACHE_ACCESS
        DKSpinLockUnlock( &interfaceGroup->lock );
        #endif
        
        return interface;
    }
    
    // Next check the dynamic cache
    if( cacheline == DKDynamicCache )
    {
        cacheline = GetDynamicCacheline( sel );
        DKAssert( (cacheline > 0) && (cacheline < (DKStaticCacheSize + DKDynamicCacheSize)) );
        
        interface = interfaceGroup->cache[cacheline];
        
        if( interface && DKFastSelectorEqual( interface->sel, sel ) )
        {
            #if SPIN_LOCKED_CACHE_ACCESS
            DKSpinLockUnlock( &interfaceGroup->lock );
            #endif
            
            return interface;
        }
    }

    // Search our interface table
    #if !SPIN_LOCKED_CACHE_ACCESS
    DKSpinLockLock( &interfaceGroup->lock );
    #endif
    
    DKIndex count = interfaceGroup->interfaces.length;
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKInterface * interface = DKGenericArrayGetElementAtIndex( &interfaceGroup->interfaces, i, DKInterface * );
        DKAssert( interface != NULL );
        
        if( DKFastSelectorEqual( interface->sel, sel ) )
        {
            // Update the cache
            interfaceGroup->cache[cacheline] = interface;

            DKSpinLockUnlock( &interfaceGroup->lock );
            return interface;
        }
    }

    // Lookup the interface in our superclasses
    DKSpinLockUnlock( &interfaceGroup->lock );
    
    if( _class->superclass )
    {
        // This pointer math lets this function work for both class and instance interfaces
        intptr_t offset = (void *)interfaceGroup - (void *)_class;
        struct DKInterfaceGroup * superclassInterfaceGroup = (void *)_class->superclass + offset;
    
        interface = DKLookupInterfaceInGroup( _class->superclass, sel, superclassInterfaceGroup );

        // Update the cache
        if( interface )
        {
            #if SPIN_LOCKED_CACHE_ACCESS
            DKSpinLockLock( &interfaceGroup->lock );
            #endif
            
            interfaceGroup->cache[cacheline] = interface;
            
            #if SPIN_LOCKED_CACHE_ACCESS
            DKSpinLockUnlock( &interfaceGroup->lock );
            #endif
            
            return interface;
        }
    }

    return NULL;
}


///
//  DKGetInterface()
//
DKInterfaceRef DKGetInterface( DKObjectRef _self, DKSEL sel )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        DKInterfaceRef interface = DKLookupInterfaceInGroup( cls, sel, (struct DKInterfaceGroup *)&cls->instanceInterfaces );
        
        if( interface )
            return interface;

        DKFatalError( "DKRuntime: Interface '%s' not found on object '%s'\n", sel->name, cls->name );
    }

    return DKInterfaceNotFound();
}


///
//  DKGetClassInterface()
//
DKInterfaceRef DKGetClassInterface( DKClassRef _class, DKSEL sel )
{
    if( _class )
    {
        DKAssert( (_class->_obj.isa == DKClassClass()) || (_class->_obj.isa == DKRootClass()) );

        DKInterfaceRef interface = DKLookupInterfaceInGroup( _class, sel, (struct DKInterfaceGroup *)&_class->classInterfaces );
        
        if( interface )
            return interface;

        DKFatalError( "DKRuntime: Class interface '%s' not found in class '%s'\n", sel->name, _class->name );
    }

    return DKInterfaceNotFound();
}


///
//  DKQueryInterface()
//
bool DKQueryInterface( DKObjectRef _self, DKSEL sel, DKInterfaceRef * _interface )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        DKInterfaceRef interface = DKLookupInterfaceInGroup( cls, sel, (struct DKInterfaceGroup *)&cls->instanceInterfaces );

        if( interface )
        {
            if( _interface )
                *_interface = interface;
            
            return true;
        }
    }
    
    return false;
}


///
//  DKQueryClassInterface()
//
bool DKQueryClassInterface( DKClassRef _class, DKSEL sel, DKInterfaceRef * _interface )
{
    if( _class )
    {
        DKAssert( (_class->_obj.isa == DKClassClass()) || (_class->_obj.isa == DKRootClass()) );
        
        DKInterfaceRef interface = DKLookupInterfaceInGroup( _class, sel, (struct DKInterfaceGroup *)&_class->classInterfaces );

        if( interface )
        {
            if( _interface )
                *_interface = interface;
            
            return true;
        }
    }
    
    return false;
}




// DKMsgHandler ==========================================================================

///
//  DKInstallMsgHandler()
//
void DKInstallMsgHandler( DKClassRef _class, DKSEL sel, DKMsgFunction func )
{
    struct DKMsgHandler * msgHandler = DKInit( DKAlloc( DKMsgHandlerClass(), sizeof(void *) ) );
    DKAssert( msgHandler != NULL );

    msgHandler->sel = DKRetain( sel );
    msgHandler->func = func;
    
    DKInstallInterfaceInGroup( _class, msgHandler, (struct DKInterfaceGroup *)&_class->instanceInterfaces );
    
    DKRelease( msgHandler );
}


///
//  DKInstallClassMsgHandler()
//
void DKInstallClassMsgHandler( DKClassRef _class, DKSEL sel, DKMsgFunction func )
{
    struct DKMsgHandler * msgHandler = DKInit( DKAlloc( DKMsgHandlerClass(), sizeof(void *) ) );
    DKAssert( msgHandler != NULL );

    msgHandler->sel = DKRetain( sel );
    msgHandler->func = func;
    
    DKInstallInterfaceInGroup( _class, msgHandler, (struct DKInterfaceGroup *)&_class->classInterfaces );
    
    DKRelease( msgHandler );
}


///
//  DKGetMsgHandler()
//
DKMsgHandlerRef DKGetMsgHandler( DKObjectRef _self, DKSEL sel )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKLookupInterfaceInGroup( cls, sel, (struct DKInterfaceGroup *)&cls->instanceInterfaces );

        if( msgHandler )
        {
            DKCheckKindOfClass( msgHandler, DKMsgHandlerClass(), DKMsgHandlerNotFound() );
            return msgHandler;
        }

        DKWarning( "DKRuntime: Message handler for '%s' not found on object '%s'\n", sel->name, obj->isa->name );
    }

    return DKMsgHandlerNotFound();
}


///
//  DKGetClassMsgHandler()
//
DKMsgHandlerRef DKGetClassMsgHandler( DKClassRef _class, DKSEL sel )
{
    if( _class )
    {
        DKAssert( (_class->_obj.isa == DKClassClass()) || (_class->_obj.isa == DKRootClass()) );

        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKLookupInterfaceInGroup( _class, sel, (struct DKInterfaceGroup *)&_class->classInterfaces );

        if( msgHandler )
        {
            DKCheckKindOfClass( msgHandler, DKMsgHandlerClass(), DKMsgHandlerNotFound() );
            return msgHandler;
        }

        DKWarning( "DKRuntime: Class message handler for '%s' not found in class '%s'\n", sel->name, _class->name );
    }

    return DKMsgHandlerNotFound();
}


///
//  DKQueryMsgHandler()
//
bool DKQueryMsgHandler( DKObjectRef _self, DKSEL sel, DKMsgHandlerRef * _msgHandler )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKLookupInterfaceInGroup( cls, sel, (struct DKInterfaceGroup *)&cls->instanceInterfaces );

        if( msgHandler )
        {
            DKCheckKindOfClass( msgHandler, DKMsgHandlerClass(), 0 );

            if( _msgHandler )
                *_msgHandler = msgHandler;
            
            return true;
        }
    }

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

        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKLookupInterfaceInGroup( _class, sel, (struct DKInterfaceGroup *)&_class->classInterfaces );

        if( msgHandler )
        {
            DKCheckKindOfClass( msgHandler, DKMsgHandlerClass(), 0 );

            if( _msgHandler )
                *_msgHandler = msgHandler;
            
            return true;
        }
    }

    return false;
}








