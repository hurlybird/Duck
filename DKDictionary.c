//
//  DKDictionary.c
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "DKDictionary.h"
#include "DKLinkedList.h"


DKDefineFastLookupInterface( Dictionary );


///
//  DKDictionaryGetCount()
//
DKIndex DKDictionaryGetCount( DKDictionaryRef ref )
{
    if( ref )
    {
        DKDictionary * dict = DKLookupInterface( ref, DKSelector(Dictionary) );
        return dict->getCount( ref );
    }
    
    return 0;
}


///
//  DKDictionarySetObject()
//
void DKDictionarySetObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object )
{
    if( ref )
    {
        DKDictionary * dict = DKLookupInterface( ref, DKSelector(Dictionary) );
        dict->insertObject( ref, key, object, DKDictionaryInsertAlways );
    }
}


///
//  DKDictionaryAddObject()
//
void DKDictionaryAddObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object )
{
    if( ref )
    {
        DKDictionary * dict = DKLookupInterface( ref, DKSelector(Dictionary) );
        dict->insertObject( ref, key, object, DKDictionaryInsertIfNotFound );
    }
}


///
//  DKDictionaryReplaceObject()
//
void DKDictionaryReplaceObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object )
{
    if( ref )
    {
        DKDictionary * dict = DKLookupInterface( ref, DKSelector(Dictionary) );
        dict->insertObject( ref, key, object, DKDictionaryInsertIfFound );
    }
}


///
//  DKDictionaryAddEntriesFromDictionary()
//
static int DKDictionaryAddEntriesCallback( void * context, DKTypeRef key, DKTypeRef object )
{
    DKDictionaryAddObject( context, key, object );
    return 0;
}

void DKDictionaryAddEntriesFromDictionary( DKMutableDictionaryRef ref, DKDictionaryRef src )
{
    if( ref )
    {
        DKDictionaryApplyFunction( src, DKDictionaryAddEntriesCallback, ref );
    }
}


///
//  DKDictionaryContainsKey()
//
int DKDictionaryContainsKey( DKDictionaryRef ref, DKTypeRef key )
{
    return DKDictionaryGetObject( ref, key ) != NULL;
}


///
//  DKDictionaryContainsObject()
//
static int DKDictionaryContainsObjectCallback( void * context, DKTypeRef key, DKTypeRef object )
{
    return object == context;
}

int DKDictionaryContainsObject( DKDictionaryRef ref, DKTypeRef object )
{
    return DKDictionaryApplyFunction( ref, DKDictionaryContainsObjectCallback, (void *)object );
}


///
//  DKDictionaryCopyKeys()
//
static int DKDictionaryCopyKeysCallback( void * context, DKTypeRef key, DKTypeRef object )
{
    DKListAppendObject( context, key );
    return 0;
}

DKListRef DKDictionaryCopyKeys( DKDictionaryRef ref )
{
    DKMutableListRef list = (DKMutableListRef)DKCreate( DKMutableLinkedListClass() );
    
    DKDictionaryApplyFunction( ref, DKDictionaryCopyKeysCallback, list );
    
    return list;
}


///
//  DKDictionaryCopyObjects()
//
static int DKDictionaryCopyObjectsCallback( void * context, DKTypeRef key, DKTypeRef object )
{
    DKListAppendObject( context, object );
    return 0;
}

DKListRef DKDictionaryCopyObjects( DKDictionaryRef ref )
{
    DKMutableListRef list = (DKMutableListRef)DKCreate( DKMutableLinkedListClass() );
    
    DKDictionaryApplyFunction( ref, DKDictionaryCopyObjectsCallback, list );
    
    return list;
}


///
//  DKDictionaryGetObject()
//
DKTypeRef DKDictionaryGetObject( DKDictionaryRef ref, DKTypeRef key )
{
    if( ref )
    {
        DKDictionary * dict = DKLookupInterface( ref, DKSelector(Dictionary) );
        return dict->getObject( ref, key );
    }
    
    return NULL;
}


///
//  DKDictionaryRemoveObject()
//
void DKDictionaryRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key )
{
    if( ref )
    {
        DKDictionary * dict = DKLookupInterface( ref, DKSelector(Dictionary) );
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
        DKDictionary * dict = DKLookupInterface( ref, DKSelector(Dictionary) );
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
        DKDictionary * dict = DKLookupInterface( ref, DKSelector(Dictionary) );
        return dict->applyFunction( ref, callback, context );
    }
    
    return 0;
}









