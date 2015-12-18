//
//  DKConversion.c
//  Duck
//
//  Created by Derek Nylen on 2015-12-18.
//  Copyright © 2015 Derek W. Nylen. All rights reserved.
//

#include "DKConversion.h"


// The description selector is initialized by DKRuntimeInit() so that constant strings can
// be used during initialization.
//DKThreadSafeFastSelectorInit( Conversion );


///
//  DKGetString()
//
DKStringRef DKGetString( DKObjectRef _self )
{
    if( _self )
    {
        DKConversionInterfaceRef conversion = DKGetInterface( _self, DKSelector(Conversion) );
        return conversion->getString( _self );
    }
    
    return NULL;
}


///
//  DKGetBool()
//
bool DKGetBool( DKObjectRef _self )
{
    if( _self )
    {
        DKConversionInterfaceRef conversion = DKGetInterface( _self, DKSelector(Conversion) );
        return conversion->getBool( _self );
    }
    
    return false;
}


///
//  DKGetInt32()
//
int32_t DKGetInt32( DKObjectRef _self )
{
    if( _self )
    {
        DKConversionInterfaceRef conversion = DKGetInterface( _self, DKSelector(Conversion) );
        return conversion->getInt32( _self );
    }
    
    return 0;
}


///
//  DKGetInt64()
//
int64_t DKGetInt64( DKObjectRef _self )
{
    if( _self )
    {
        DKConversionInterfaceRef conversion = DKGetInterface( _self, DKSelector(Conversion) );
        return conversion->getInt64( _self );
    }
    
    return 0;
}


///
//  DKGetFloat()
//
float DKGetFloat( DKObjectRef _self )
{
    if( _self )
    {
        DKConversionInterfaceRef conversion = DKGetInterface( _self, DKSelector(Conversion) );
        return conversion->getFloat( _self );
    }
    
    return 0.0f;
}


///
//  DKGetDouble()
//
double DKGetDouble( DKObjectRef _self )
{
    if( _self )
    {
        DKConversionInterfaceRef conversion = DKGetInterface( _self, DKSelector(Conversion) );
        return conversion->getDouble( _self );
    }
    
    return 0.0;
}





