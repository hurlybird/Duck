// =======================================================================================
//
// DKDescription.c
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#define DK_RUNTIME_PRIVATE 1

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKGenericArray.h"
#include "DKGenericHashTable.h"
#include "DKRuntime.h"
#include "DKDescription.h"


// The description selector is initialized by DKRuntimeInit() so that constant strings can
// be used during initialization.
//DKThreadSafeFastSelectorInit( Description );


///
//  DKDefaultGetDescription()
//
DKStringRef DKDefaultGetDescription( DKObjectRef _self )
{
    return DKGetClassName( _self );
}


///
//  DKDefaultGetSizeInBytes()
//
size_t DKDefaultGetSizeInBytes( DKObjectRef _self )
{
    if( _self )
    {
        const DKObject * obj = _self;
        DKClassRef cls = obj->isa;
        
        if( (cls == DKClassClass()) || (cls == DKRootClass()) )
            cls = _self;
        
        return cls->structSize;
    }
    
    return 0;
}


///
//  DKGetDescription()
//
DKStringRef DKGetDescription( DKObjectRef _self )
{
    if( _self )
    {
        DKDescriptionInterfaceRef description = DKGetInterface( _self, DKSelector(Description) );
        return description->getDescription( _self );
    }
    
    return DKSTR( "null" );
}


///
//  DKGetSizeInBytes()
//
size_t DKGetSizeInBytes( DKObjectRef _self )
{
    if( _self )
    {
        DKDescriptionInterfaceRef description = DKGetInterface( _self, DKSelector(Description) );
        return description->getSizeInBytes( _self );
    }
    
    return 0;
}




