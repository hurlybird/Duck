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
void DKSetDictionaryClass( DKClassRef ref )
{
    DefaultDictionaryClass = ref;
}


///
//  DKDictionaryGetCount()
//
DKIndex DKDictionaryGetCount( DKDictionaryRef ref )
{
    if( ref )
    {
        DKDictionary * dict = DKGetInterface( ref, DKSelector(Dictionary) );
        return dict->getCount( ref );
    }
    
    return 0;
}


///
//  DKDictionarySetObject()
//
void DKDictionarySetObject( DKMutableDictionaryRef ref, DKObjectRef key, DKObjectRef object )
{
    if( ref )
    {
        DKDictionary * dict = DKGetInterface( ref, DKSelector(Dictionary) );
        dict->insertObject( ref, key, object, DKDictionaryInsertAlways );
    }
}


///
//  DKDictionaryAddObject()
//
void DKDictionaryAddObject( DKMutableDictionaryRef ref, DKObjectRef key, DKObjectRef object )
{
    if( ref )
    {
        DKDictionary * dict = DKGetInterface( ref, DKSelector(Dictionary) );
        dict->insertObject( ref, key, object, DKDictionaryInsertIfNotFound );
    }
}


///
//  DKDictionaryReplaceObject()
//
void DKDictionaryReplaceObject( DKMutableDictionaryRef ref, DKObjectRef key, DKObjectRef object )
{
    if( ref )
    {
        DKDictionary * dict = DKGetInterface( ref, DKSelector(Dictionary) );
        dict->insertObject( ref, key, object, DKDictionaryInsertIfFound );
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

void DKDictionaryAddEntriesFromDictionary( DKMutableDictionaryRef ref, DKDictionaryRef src )
{
    if( ref )
    {
        DKDictionaryApplyFunction( src, DKDictionaryAddEntriesCallback, (void *)ref );
    }
}


///
//  DKDictionaryContainsKey()
//
int DKDictionaryContainsKey( DKDictionaryRef ref, DKObjectRef key )
{
    return DKDictionaryGetObject( ref, key ) != NULL;
}


///
//  DKDictionaryContainsObject()
//
static int DKDictionaryContainsObjectCallback( void * context, DKObjectRef key, DKObjectRef object )
{
    return object == context;
}

int DKDictionaryContainsObject( DKDictionaryRef ref, DKObjectRef object )
{
    return DKDictionaryApplyFunction( ref, DKDictionaryContainsObjectCallback, (void *)object );
}


///
//  DKDictionaryCopyKeys()
//
static int DKDictionaryCopyKeysCallback( void * context, DKObjectRef key, DKObjectRef object )
{
    DKListAppendObject( context, key );
    return 0;
}

DKListRef DKDictionaryCopyKeys( DKDictionaryRef ref )
{
    DKMutableListRef list = (DKMutableListRef)DKCreate( DKMutableLinkedListClass() );
    
    DKDictionaryApplyFunction( ref, DKDictionaryCopyKeysCallback, (void *)list );
    
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

DKListRef DKDictionaryCopyObjects( DKDictionaryRef ref )
{
    DKMutableListRef list = (DKMutableListRef)DKCreate( DKMutableLinkedListClass() );
    
    DKDictionaryApplyFunction( ref, DKDictionaryCopyObjectsCallback, (void *)list );
    
    return list;
}


///
//  DKDictionaryGetObject()
//
DKObjectRef DKDictionaryGetObject( DKDictionaryRef ref, DKObjectRef key )
{
    if( ref )
    {
        DKDictionary * dict = DKGetInterface( ref, DKSelector(Dictionary) );
        return dict->getObject( ref, key );
    }
    
    return NULL;
}


///
//  DKDictionaryRemoveObject()
//
void DKDictionaryRemoveObject( DKMutableDictionaryRef ref, DKObjectRef key )
{
    if( ref )
    {
        DKDictionary * dict = DKGetInterface( ref, DKSelector(Dictionary) );
        return dict->removeObject( ref, key );
    }
}


///
//  DKDictionaryRemoveAllObjects()
//
void DKDictionaryRemoveAllObjects( DKMutableDictionaryRef ref )
{
    if( ref )
    {
        DKDictionary * dict = DKGetInterface( ref, DKSelector(Dictionary) );
        return dict->removeAllObjects( ref );
    }
}


///
//  DKDictionaryApplyFunction()
//
int DKDictionaryApplyFunction( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context )
{
    if( ref )
    {
        DKDictionary * dict = DKGetInterface( ref, DKSelector(Dictionary) );
        return dict->applyFunction( ref, callback, context );
    }
    
    return 0;
}









