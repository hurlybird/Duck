/*******************************************************************************

  DKDictionary.c

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

*******************************************************************************/

#include "DKDictionary.h"
#include "DKBinaryTree.h"
#include "DKArray.h"
#include "DKString.h"
#include "DKStream.h"


DKThreadSafeFastSelectorInit( Dictionary );


///
//  DKDictionaryClass()
//
static DKClassRef DefaultDictionaryClass = NULL;

DKClassRef DKDictionaryClass( void )
{
    if( DefaultDictionaryClass )
        return DefaultDictionaryClass;
    
    return DKBinaryTreeClass();
}

void DKSetDictionaryClass( DKClassRef _self )
{
    DefaultDictionaryClass = _self;
}


///
//  DKMutableDictionaryClass()
//
static DKClassRef DefaultMutableDictionaryClass = NULL;

DKClassRef DKMutableDictionaryClass( void )
{
    if( DefaultMutableDictionaryClass )
        return DefaultMutableDictionaryClass;
    
    return DKBinaryTreeClass();
}

void DKSetMutableDictionaryClass( DKClassRef _self )
{
    DefaultMutableDictionaryClass = _self;
}


///
//  DKDictionaryGetCount()
//
DKIndex DKDictionaryGetCount( DKDictionaryRef _self )
{
    if( _self )
    {
        DKDictionary * dict = DKGetInterface( _self, DKSelector(Dictionary) );
        return dict->getCount( _self );
    }
    
    return 0;
}


///
//  DKDictionarySetObject()
//
void DKDictionarySetObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object )
{
    if( _self )
    {
        DKDictionary * dict = DKGetInterface( _self, DKSelector(Dictionary) );
        dict->insertObject( _self, key, object, DKInsertAlways );
    }
}


///
//  DKDictionaryAddObject()
//
void DKDictionaryAddObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object )
{
    if( _self )
    {
        DKDictionary * dict = DKGetInterface( _self, DKSelector(Dictionary) );
        dict->insertObject( _self, key, object, DKInsertIfNotFound );
    }
}


///
//  DKDictionaryReplaceObject()
//
void DKDictionaryReplaceObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object )
{
    if( _self )
    {
        DKDictionary * dict = DKGetInterface( _self, DKSelector(Dictionary) );
        dict->insertObject( _self, key, object, DKInsertIfFound );
    }
}


///
//  DKDictionaryAddEntriesFromDictionary()
//
static int DKDictionaryAddEntriesCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    DKDictionaryAddObject( context, key, object );
    return 0;
}

void DKDictionaryAddEntriesFromDictionary( DKMutableDictionaryRef _self, DKDictionaryRef src )
{
    if( _self )
    {
        DKDictionaryApplyFunction( src, DKDictionaryAddEntriesCallback, (void *)_self );
    }
}


///
//  DKDictionaryContainsKey()
//
int DKDictionaryContainsKey( DKDictionaryRef _self, DKObjectRef key )
{
    return DKDictionaryGetObject( _self, key ) != NULL;
}


///
//  DKDictionaryContainsObject()
//
static int DKDictionaryContainsObjectCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    return object == context;
}

int DKDictionaryContainsObject( DKDictionaryRef _self, DKObjectRef object )
{
    return DKDictionaryApplyFunction( _self, DKDictionaryContainsObjectCallback, (void *)object );
}


///
//  DKDictionaryCopyKeys()
//
static int DKDictionaryCopyKeysCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    DKListAppendObject( context, key );
    return 0;
}

DKListRef DKDictionaryCopyKeys( DKDictionaryRef _self )
{
    DKMutableListRef list = (DKMutableListRef)DKCreate( DKMutableArrayClass() );
    
    DKDictionaryApplyFunction( _self, DKDictionaryCopyKeysCallback, (void *)list );
    
    return list;
}


///
//  DKDictionaryCopyObjects()
//
static int DKDictionaryCopyObjectsCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    DKListAppendObject( context, object );
    return 0;
}

DKListRef DKDictionaryCopyObjects( DKDictionaryRef _self )
{
    DKMutableListRef list = (DKMutableListRef)DKCreate( DKMutableArrayClass() );
    
    DKDictionaryApplyFunction( _self, DKDictionaryCopyObjectsCallback, (void *)list );
    
    return list;
}


///
//  DKDictionaryGetObject()
//
DKObjectRef DKDictionaryGetObject( DKDictionaryRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKDictionary * dict = DKGetInterface( _self, DKSelector(Dictionary) );
        return dict->getObject( _self, key );
    }
    
    return NULL;
}


///
//  DKDictionaryRemoveObject()
//
void DKDictionaryRemoveObject( DKMutableDictionaryRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKDictionary * dict = DKGetInterface( _self, DKSelector(Dictionary) );
        return dict->removeObject( _self, key );
    }
}


///
//  DKDictionaryRemoveAllObjects()
//
void DKDictionaryRemoveAllObjects( DKMutableDictionaryRef _self )
{
    if( _self )
    {
        DKDictionary * dict = DKGetInterface( _self, DKSelector(Dictionary) );
        return dict->removeAllObjects( _self );
    }
}


///
//  DKDictionaryApplyFunction()
//
int DKDictionaryApplyFunction( DKDictionaryRef _self, DKDictionaryApplierFunction callback, void * context )
{
    if( _self )
    {
        DKDictionary * dict = DKGetInterface( _self, DKSelector(Dictionary) );
        return dict->applyFunction( _self, callback, context );
    }
    
    return 0;
}


///
//  DKCopyDictionaryDescription()
//
struct PrintDescriptionContext
{
    DKMutableObjectRef stream;
    DKIndex n;
};

static int PrintDescriptionCallback( DKObjectRef key, DKObjectRef object, void * context )
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

DKStringRef DKDictionaryCopyDescription( DKListRef _self )
{
    DKMutableStringRef desc = DKStringCreateMutable();
    
    DKIndex count = DKListGetCount( _self );
    
    DKSPrintf( desc, "%@, %d key-object pairs = {", DKGetClassName( _self ), count );
    
    struct PrintDescriptionContext context = { desc, 0 };
    DKDictionaryApplyFunction( _self, PrintDescriptionCallback, &context );
    
    DKSPrintf( desc, "\n}\n" );
    
    return desc;
}







