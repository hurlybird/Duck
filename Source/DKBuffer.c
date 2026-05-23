// =======================================================================================
//
// DKBuffer.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKBuffer.h"


// The stream selector is initialized by DKRuntimeInit() so that constant strings can be
// used during initialization.
//DKThreadSafeSelectorInit( Buffer );


///
//  DKBufferGetLength()
//
DKIndex DKBufferGetLength( DKObjectRef _self )
{
    DKBufferInterfaceRef interface = DKGetInterface( _self, DKSelector(Buffer) );
    return interface->getLength( _self );
}


///
//  DKBufferGetBytePtr()
//
const void * DKBufferGetBytePtr( DKObjectRef _self, DKIndex index )
{
    DKBufferInterfaceRef interface = DKGetInterface( _self, DKSelector(Buffer) );
    return interface->getBytePtr( _self, index );
}


///
//  DKBufferSetLength()
//
void DKBufferSetLength( DKObjectRef _self, DKIndex length )
{
    DKBufferInterfaceRef interface = DKGetInterface( _self, DKSelector(Buffer) );
    interface->setLength( _self, length );
}


///
//  DKBufferGetMutableBytePtr()
//
void * DKBufferGetMutableBytePtr( DKObjectRef _self, DKIndex index )
{
    DKBufferInterfaceRef interface = DKGetInterface( _self, DKSelector(Buffer) );
    return interface->getMutableBytePtr( _self, index );
}


