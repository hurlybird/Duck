// =======================================================================================
//
// DKCopying.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKCopying.h"


// The copying selector is initialized by DKRuntimeInit() so that constant strings can
// be used during initialization.
//DKThreadSafeFastSelectorInit( Copying );


///
//  DKCopy()
//
DKObjectRef DKCopy( DKObjectRef _self )
{
    if( _self )
    {
        DKCopyingInterfaceRef copying = DKGetInterface( _self, DKSelector(Copying) );
        return copying->copy( _self );
    }

    return _self;
}


///
//  DKMutableCopy()
//
DKObjectRef DKMutableCopy( DKObjectRef _self )
{
    if( _self )
    {
        DKCopyingInterfaceRef copying = DKGetInterface( _self, DKSelector(Copying) );
        return copying->mutableCopy( _self );
    }

    return NULL;
}


///
//  DKDeepCopy()
//
DKObjectRef DKDeepCopy( DKObjectRef _self, int options )
{
    if( _self )
    {
        DKCopyingInterfaceRef copying = DKGetInterface( _self, DKSelector(Copying) );
        return copying->deepCopy( _self, options );
    }

    return NULL;
}


///
//  DKDefaultDeepCopy()
//
DKObjectRef DKDefaultDeepCopy( DKObjectRef object, int options )
{
    if( options & DKDeepCopyMutableObjects )
        return DKMutableCopy( object );
        
    else
        return DKCopy( object );
}



