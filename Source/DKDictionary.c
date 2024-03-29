/*****************************************************************************************

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

*****************************************************************************************/

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKCollection.h"
#include "DKList.h"
#include "DKDictionary.h"
#include "DKHashTable.h"
#include "DKArray.h"
#include "DKString.h"
#include "DKStream.h"
#include "DKComparison.h"


DKThreadSafeFastSelectorInit( Dictionary );


///
//  DKDictionaryClass()
//
static DKClassRef DefaultDictionaryClass = NULL;

DKClassRef DKDictionaryClass( void )
{
    if( DefaultDictionaryClass )
        return DefaultDictionaryClass;
    
    return DKHashTableClass();
}

void DKSetDefaultDictionaryClass( DKClassRef cls )
{
    DefaultDictionaryClass = cls;
}


///
//  DKMutableDictionaryClass()
//
static DKClassRef DefaultMutableDictionaryClass = NULL;

DKClassRef DKMutableDictionaryClass( void )
{
    if( DefaultMutableDictionaryClass )
        return DefaultMutableDictionaryClass;
    
    return DKMutableHashTableClass();
}

void DKSetDefaultMutableDictionaryClass( DKClassRef cls )
{
    DefaultMutableDictionaryClass = cls;
}


///
//  DKDictionaryInitWithKeysAndObjects()
//
DKObjectRef DKDictionaryInitWithKeysAndObjects( DKDictionaryRef _self, ... )
{
    if( _self )
    {
        DKDictionaryInterfaceRef dictInterface = DKGetInterface( _self, DKSelector(Dictionary) );
    
        va_list arg_ptr;
        va_start( arg_ptr, _self );
    
        _self = dictInterface->initWithVAKeysAndObjects( _self, arg_ptr );
    
        va_end( arg_ptr );
    }
    
    return _self;
}


///
//  DKDictionaryInitWithDictionary()
//
DKObjectRef DKDictionaryInitWithDictionary( DKDictionaryRef _self, DKDictionaryRef srcDictionary )
{
    if( _self )
    {
        DKDictionaryInterfaceRef dictInterface = DKGetInterface( _self, DKSelector(Dictionary) );
        _self = dictInterface->initWithDictionary( _self, srcDictionary );
    }
    
    return _self;
}


///
//  DKDictionaryGetCount()
//
DKIndex DKDictionaryGetCount( DKDictionaryRef _self )
{
    if( _self )
    {
        DKDictionaryInterfaceRef dict = DKGetInterface( _self, DKSelector(Dictionary) );
        return dict->getCount( _self );
    }
    
    return 0;
}


///
//  DKDictionaryGetObject()
//
DKObjectRef DKDictionaryGetObject( DKDictionaryRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKDictionaryInterfaceRef dict = DKGetInterface( _self, DKSelector(Dictionary) );
        return dict->getObject( _self, key );
    }
    
    return NULL;
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
    return DKForeachKeyAndObject( _self, DKDictionaryContainsObjectCallback, object );
}


///
//  DKDictionaryGetAllKeys()
//
static int DKDictionaryGetAllKeysCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    DKListAppendObject( context, key );
    return 0;
}

DKListRef DKDictionaryGetAllKeys( DKDictionaryRef _self )
{
    DKMutableListRef list = DKNew( DKMutableArrayClass() );
    
    DKForeachKeyAndObject( _self, DKDictionaryGetAllKeysCallback, list );
    
    return DKAutorelease( list );
}


///
//  DKDictionaryGetAllObjects()
//
static int DKDictionaryGetAllObjectsCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    DKListAppendObject( context, object );
    return 0;
}

DKListRef DKDictionaryGetAllObjects( DKDictionaryRef _self )
{
    DKMutableListRef list = DKNew( DKMutableArrayClass() );
    
    DKForeachKeyAndObject( _self, DKDictionaryGetAllObjectsCallback, list );
    
    return DKAutorelease( list );
}


///
//  DKDictionarySetObject()
//
void DKDictionarySetObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object )
{
    if( _self )
    {
        DKDictionaryInterfaceRef dict = DKGetInterface( _self, DKSelector(Dictionary) );
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
        DKDictionaryInterfaceRef dict = DKGetInterface( _self, DKSelector(Dictionary) );
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
        DKDictionaryInterfaceRef dict = DKGetInterface( _self, DKSelector(Dictionary) );
        dict->insertObject( _self, key, object, DKInsertIfFound );
    }
}


///
//  DKDictionaryInsertObject()
//
void DKDictionaryInsertObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    if( _self )
    {
        DKDictionaryInterfaceRef dict = DKGetInterface( _self, DKSelector(Dictionary) );
        dict->insertObject( _self, key, object, policy );
    }
}


///
//  DKDictionaryInsertEntriesFromDictionary()
//
struct DKDictionaryInsertEntriesContext
{
    DKMutableDictionaryRef _self;
    DKDictionaryInterfaceRef dict;
    DKInsertPolicy policy;
};

static int DKDictionaryInsertEntriesCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    struct DKDictionaryInsertEntriesContext * ctx = context;
    ctx->dict->insertObject( ctx->_self, key, object, ctx->policy );
    return 0;
}

void DKDictionaryInsertEntriesFromDictionary( DKMutableDictionaryRef _self, DKDictionaryRef src, DKInsertPolicy policy )
{
    if( _self )
    {
        struct DKDictionaryInsertEntriesContext ctx =
        {
            _self,
            DKGetInterface( _self, DKSelector(Dictionary) ),
            policy
        };
        
        DKForeachKeyAndObject( src, DKDictionaryInsertEntriesCallback, &ctx );
    }
}


///
//  DKDictionaryRemoveObject()
//
void DKDictionaryRemoveObject( DKMutableDictionaryRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKDictionaryInterfaceRef dict = DKGetInterface( _self, DKSelector(Dictionary) );
        dict->removeObject( _self, key );
    }
}


///
//  DKDictionaryRemoveAllObjects()
//
void DKDictionaryRemoveAllObjects( DKMutableDictionaryRef _self )
{
    if( _self )
    {
        DKDictionaryInterfaceRef dict = DKGetInterface( _self, DKSelector(Dictionary) );
        dict->removeAllObjects( _self );
    }
}


///
//  DKDictionaryEqual()
//
static int DKDictionaryEqualCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    DKObjectRef object2 = DKDictionaryGetObject( context, key );
    
    if( DKEqual( object, object2 ) )
        return 0;
    
    return 1;
}

bool DKDictionaryEqual( DKDictionaryRef _self, DKDictionaryRef other )
{
    if( _self )
    {
        DKDictionaryInterfaceRef dict1 = DKGetInterface( _self, DKSelector(Dictionary) );
        
        DKDictionaryInterfaceRef dict2;
        
        if( DKQueryInterface( other, DKSelector(Dictionary), (DKInterfaceRef *)&dict2 ) )
        {
            if( dict1->getCount( _self ) == dict2->getCount( other ) )
            {
                int result = DKForeachKeyAndObject( _self, DKDictionaryEqualCallback, other );
                return result == 0;
            }
        }
    }
    
    return false;
}


///
//  DKDictionaryIsSubsetOfDictionary()
//
bool DKDictionaryIsSubsetOfDictionary( DKDictionaryRef _self, DKDictionaryRef other )
{
    if( _self )
    {
        // This check may be pendantic, but we don't do a dictionary interface lookup on
        // 'other' and a keyed collection isn't technically required to be a dictionary
        // (i.e. it could be a multimap or something).
        if( DKQueryInterface( _self, DKSelector(Dictionary), NULL ) )
        {
            int result = DKForeachKeyAndObject( _self, DKDictionaryEqualCallback, other );
            return result == 0;
        }
    }
    
    return false;
}







