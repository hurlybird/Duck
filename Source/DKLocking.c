// =======================================================================================
//
// DKLocking.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKLocking.h"


// The copying selector is initialized by DKRuntimeInit() so that constant strings can
// be used during initialization.
//DKThreadSafeFastSelectorInit( Locking );


///
//  DKLock()
//
void DKLock( DKObjectRef _self )
{
    if( _self )
    {
        DKLockingInterfaceRef lockingInterface = DKGetInterface( _self, DKSelector(Locking) );
        lockingInterface->lock( _self );
    }
}


///
//  DKUnlock()
//
void DKUnlock( DKObjectRef _self )
{
    if( _self )
    {
        DKLockingInterfaceRef lockingInterface = DKGetInterface( _self, DKSelector(Locking) );
        lockingInterface->unlock( _self );
    }
}




