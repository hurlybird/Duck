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




// Error Handling Interfaces =============================================================

// This handles sending messages to NULL objects.

DKDeclareMessageSelector( MsgHandlerNotFound );
DKThreadSafeSelectorInit( MsgHandlerNotFound );

static intptr_t DKMsgHandlerNotFoundMethod( DKObjectRef _self, DKSEL sel )
{
    return 0;
}

DKThreadSafeSharedObjectInit( DKMsgHandlerNotFound, DKMsgHandlerRef )
{
    struct DKMsgHandler * msgHandler = DKAllocInterface( DKSelector(MsgHandlerNotFound), sizeof(struct DKMsgHandler) );

    msgHandler->func = DKMsgHandlerNotFoundMethod;

    return msgHandler;
}




// DKSelector ============================================================================

static DKSpinLock NextCacheLineSpinLock = DKSpinLockInit;
static unsigned int NextCacheLine = 0;


///
//  DKAllocSelector()
//
DKSEL DKAllocSelector( DKStringRef name )
{
    struct _DKSEL * sel = DKInit( DKAlloc( DKSelectorClass(), 0 ) );

    DKAssert( sel != NULL );

    sel->name = DKCopy( name );
    
    DKSpinLockLock( &NextCacheLineSpinLock );
    sel->cacheline = DKStaticCacheSize + (NextCacheLine & (DKDynamicCacheSize - 1));
    NextCacheLine++;
    DKSpinLockUnlock( &NextCacheLineSpinLock );

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
DKInterfaceRef DKAllocInterface( DKSEL sel, size_t structSize )
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
    DKInterface * interface = _self;
    DKRelease( interface->sel );
}


///
//  DKInstallInterface()
//
void DKInstallInterface( DKClassRef _class, DKInterfaceRef _interface )
{
    DKInterfaceTableInsert( _class, &_class->instanceInterfaces, _interface );
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
    DKFatalError( "DKRuntime: Interface '%s' not found on object '%s'\n", sel->name, _class->name );
    return NULL;
}

DKInterfaceRef DKGetInterface( DKObjectRef _self, DKSEL sel )
{
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
    DKFatalError( "DKRuntime: Class interface '%s' not found on object '%s'\n", sel->name, _class->name );
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

    if( interface )
    {
        if( _interface )
            *_interface = interface;
        
        return true;
    }
    
    return false;
}


///
//  DKQueryClassInterface()
//
bool DKQueryClassInterface( DKClassRef _class, DKSEL sel, DKInterfaceRef * _interface )
{
    DKAssert( (_class != NULL) && (sel != NULL) );
    DKAssert( (_class->_obj.isa == DKClassClass()) || (_class->_obj.isa == DKRootClass()) );
    
    DKInterfaceRef interface = DKInterfaceTableFind( _class, &_class->classInterfaces, sel, DKQueryInterfaceNotFound );

    if( interface )
    {
        if( _interface )
            *_interface = interface;
        
        return true;
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
    
    DKInterfaceTableInsert( _class, &_class->instanceInterfaces, msgHandler );
    
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

DKMsgHandlerRef DKGetMsgHandler( DKObjectRef _self, DKSEL sel )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        return (DKMsgHandlerRef)DKInterfaceTableFind( cls, &cls->instanceInterfaces, sel, DKGetMsgHandlerNotFound );
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

        return (DKMsgHandlerRef)DKInterfaceTableFind( _class, &_class->classInterfaces, sel, DKGetMsgHandlerNotFound );
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
        
        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKInterfaceTableFind( cls, &cls->instanceInterfaces, sel, DKQueryInterfaceNotFound );

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

        DKMsgHandlerRef msgHandler = (DKMsgHandlerRef)DKInterfaceTableFind( _class, &_class->classInterfaces, sel, DKQueryInterfaceNotFound );

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








