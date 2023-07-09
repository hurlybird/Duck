/*****************************************************************************************

  DKConversion.c

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

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKConversion.h"
#include "DKBoolean.h"


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
        if( _self == DKTrue() )
            return true;
        
        if( _self == DKFalse() )
            return false;
    
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





