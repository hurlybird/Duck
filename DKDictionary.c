//
//  DKDictionary.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKDictionary.h"
#include "DKBinaryTree.h"
#include "DKLinkedList.h"


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


///
//  DKSetDictionaryClass()
//
void DKSetDictionaryClass( DKClassRef _self )
{
    DefaultDictionaryClass = _self;
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
        dict->insertObject( _self, key, object, DKDictionaryInsertAlways );
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
        dict->insertObject( _self, key, object, DKDictionaryInsertIfNotFound );
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
        dict->insertObject( _self, key, object, DKDictionaryInsertIfFound );
    }
}


///
//  DKDictionaryAddEntriesFromDictionary()
//
static int DKDictionaryAddEntriesCallback( void * context, DKObjectRef key, DKObjectRef object )
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
static int DKDictionaryContainsObjectCallback( void * context, DKObjectRef key, DKObjectRef object )
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
static int DKDictionaryCopyKeysCallback( void * context, DKObjectRef key, DKObjectRef object )
{
    DKListAppendObject( context, key );
    return 0;
}

DKListRef DKDictionaryCopyKeys( DKDictionaryRef _self )
{
    DKMutableListRef list = (DKMutableListRef)DKCreate( DKMutableLinkedListClass() );
    
    DKDictionaryApplyFunction( _self, DKDictionaryCopyKeysCallback, (void *)list );
    
    return list;
}


///
//  DKDictionaryCopyObjects()
//
static int DKDictionaryCopyObjectsCallback( void * context, DKObjectRef key, DKObjectRef object )
{
    DKListAppendObject( context, object );
    return 0;
}

DKListRef DKDictionaryCopyObjects( DKDictionaryRef _self )
{
    DKMutableListRef list = (DKMutableListRef)DKCreate( DKMutableLinkedListClass() );
    
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









