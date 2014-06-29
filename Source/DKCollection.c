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

#include "DKCollection.h"
#include "DKString.h"
#include "DKStream.h"


DKThreadSafeFastSelectorInit( Collection );


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
//  DKGetAnyKey()
//
static int DKGetAnyKeyCallback( DKObjectRef key, void * context )
{
    DKObjectRef * result = context;
    *result = key;
    return 1;
}

DKObjectRef DKGetAnyKey( DKObjectRef _self )
{
    DKObjectRef key = NULL;
    
    DKForeachKey( _self, DKGetAnyKeyCallback, &key );
    
    return key;
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
    
    DKForeachObject( _self, DKGetAnyKeyCallback, &object );
    
    return object;
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
        DKCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(Collection) );
        
        if( collection->foreachKey )
        {
            return collection->foreachKey( _self, callback, context );
        }
        
        else
        {
            DKFatalError( "DKForeachKey: '%s' is not a keyed collection.\n",
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );
        }
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
        DKCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(Collection) );
        
        if( collection->foreachKeyAndObject )
        {
            return collection->foreachKeyAndObject( _self, callback, context );
        }
        
        else
        {
            DKFatalError( "DKForeachKeyAndObject: '%s' is not a keyed collection.\n",
                DKStringGetCStringPtr( DKGetClassName( _self ) ) );
        }
    }
    
    return 0;
}


///
//  DKCollectionCopyDescription()
//
struct PrintDescriptionContext
{
    DKMutableObjectRef stream;
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

DKStringRef DKCollectionCopyDescription( DKObjectRef _self )
{
    DKMutableStringRef desc = DKStringCreateMutable();

    DKCollectionInterfaceRef collection = DKGetInterface( _self, DKSelector(Collection) );
    
    DKIndex count = collection->getCount( _self );
    
    DKSPrintf( desc, "%@, %d objects = (", DKGetClassName( _self ), count );
    
    struct PrintDescriptionContext context = { desc, 0 };

    if( collection->foreachKeyAndObject )
        collection->foreachKeyAndObject( _self, PrintKeyedDescriptionCallback, &context );
    
    else
        collection->foreachObject( _self, PrintDescriptionCallback, &context );
    
    DKSPrintf( desc, "\n)\n" );
    
    return desc;
}






