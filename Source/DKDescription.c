/*****************************************************************************************

  DKDescription.c

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




