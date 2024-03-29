/*****************************************************************************************

  DKCollection.c

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
#include "DKCollection.h"
#include "DKString.h"
#include "DKStream.h"


DKThreadSafeFastSelectorInit( Collection );
DKThreadSafeFastSelectorInit( KeyedCollection );


///
//  DKGetCount()
//
DKIndex DKGetCount( DKObjectRef _self )
{
    if( _self )
    {
        DKCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(Collection) );
        return collection->getCount( _self );
    }
    
    return 0;
}


///
//  DKGetAnyObject()
//
static int DKGetAnyObjectCallback( DKObjectRef object, void * context )
{
    DKObjectRef * result = context;
    *result = object;
    return 1;
}

DKObjectRef DKGetAnyObject( DKObjectRef _self )
{
    DKObjectRef object = NULL;
    
    DKForeachObject( _self, DKGetAnyObjectCallback, &object );
    
    return object;
}


///
//  DKGetAnyKey()
//
DKObjectRef DKGetAnyKey( DKObjectRef _self )
{
    DKObjectRef key = NULL;
    
    DKForeachKey( _self, DKGetAnyObjectCallback, &key );
    
    return key;
}


///
//  DKContainsKey()
//
bool DKContainsKey( DKObjectRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKKeyedCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(KeyedCollection) );
        return collection->containsKey( _self, key );
    }
    
    return false;
}


///
//  DKContainsObject()
//
bool DKContainsObject( DKObjectRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(Collection) );
        return collection->containsObject( _self, object );
    }
    
    return false;
}


///
//  DKForeachObject()
//
int DKForeachObject( DKObjectRef _self, DKApplierFunction callback, void * context )
{
    if( _self )
    {
        DKCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(Collection) );
        return collection->foreachObject( _self, callback, context );
    }
    
    return 0;
}


///
//  DKForeachKey()
//
int DKForeachKey( DKObjectRef _self, DKApplierFunction callback, void * context )
{
    if( _self )
    {
        DKKeyedCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(KeyedCollection) );
        return collection->foreachKey( _self, callback, context );
    }
    
    return 0;
}


///
//  DKForeachKeyAndObject()
//
int DKForeachKeyAndObject( DKObjectRef _self, DKKeyedApplierFunction callback, void * context )
{
    if( _self )
    {
        DKKeyedCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(KeyedCollection) );
        return collection->foreachKeyAndObject( _self, callback, context );
    }
    
    return 0;
}


///
//  DKCollectionCopyDescription()
//
struct PrintDescriptionContext
{
    DKObjectRef stream;
    DKIndex n;
};

static int PrintDescriptionCallback( DKObjectRef object, void * context )
{
    struct PrintDescriptionContext * ctx = context;

    const char * prefix = (ctx->n == 0) ? "\n    " : ",\n    ";

    if( DKIsKindOfClass( object, DKStringClass() ) )
        DKSPrintf( ctx->stream, "%s\"%@\"", prefix, object );
    
    else
        DKSPrintf( ctx->stream, "%s%@", prefix, object );
    
    ctx->n++;
    
    return 0;
}

DKStringRef DKCollectionGetDescription( DKObjectRef _self )
{
    DKMutableStringRef desc = DKMutableString();

    DKCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(Collection) );
    
    DKIndex count = collection->getCount( _self );
    
    DKSPrintf( desc, "%@, %d objects = (", DKGetClassName( _self ), count );
    
    struct PrintDescriptionContext context = { desc, 0 };

    collection->foreachObject( _self, PrintDescriptionCallback, &context );
    
    DKSPrintf( desc, "\n)" );
    
    return desc;
}


///
//  DKKeyedCollectionCopyDescription()
//
static int PrintKeyedDescriptionCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    struct PrintDescriptionContext * ctx = context;

    const char * prefix = (ctx->n == 0) ? "\n    " : ",\n    ";

    if( DKIsKindOfClass( key, DKStringClass() ) )
        DKSPrintf( ctx->stream, "%s\"%@\" = ", prefix, key );
    
    else
        DKSPrintf( ctx->stream, "%s%@ = ", prefix, key );

    if( DKIsKindOfClass( object, DKStringClass() ) )
        DKSPrintf( ctx->stream, "\"%@\"", object );
    
    else
        DKSPrintf( ctx->stream, "%@", object );
    
    ctx->n++;
    
    return 0;
}

DKStringRef DKKeyedCollectionGetDescription( DKObjectRef _self )
{
    DKMutableStringRef desc = DKMutableString();

    DKKeyedCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(KeyedCollection) );
    
    DKIndex count = collection->getCount( _self );
    
    DKSPrintf( desc, "%@, %d objects = (", DKGetClassName( _self ), count );
    
    struct PrintDescriptionContext context = { desc, 0 };

    collection->foreachKeyAndObject( _self, PrintKeyedDescriptionCallback, &context );
    
    DKSPrintf( desc, "\n)" );
    
    return desc;
}






