//
//  DKDictionary.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-23.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_DICTIONARY_H_
#define _DK_DICTIONARY_H_

#include "DKRuntime.h"
#include "DKList.h"


DKDeclareInterfaceSelector( Dictionary );


typedef const void * DKDictionaryRef;
typedef void * DKMutableDictionaryRef;


typedef int (*DKDictionaryApplierFunction)( void * context, DKObjectRef key, DKObjectRef value );


typedef enum
{
    DKDictionaryInsertAlways,
    DKDictionaryInsertIfFound,
    DKDictionaryInsertIfNotFound
    
} DKDictionaryInsertPolicy;

typedef DKIndex     (*DKDictionaryGetCountMethod)( DKDictionaryRef _self );
typedef DKObjectRef (*DKDictionaryGetObjectMethod)( DKDictionaryRef _self, DKObjectRef key );
typedef int         (*DKDictionaryApplyFunctionMethod)( DKDictionaryRef _self, DKDictionaryApplierFunction, void * context );
typedef void        (*DKDictionaryInsertObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKDictionaryInsertPolicy policy );
typedef void        (*DKDictionaryRemoveObjectMethod)( DKMutableDictionaryRef _self, DKObjectRef key );
typedef void        (*DKDictionaryRemoveAllObjectsMethod)( DKMutableDictionaryRef _self );

struct DKDictionary
{
    DKInterface _interface;

    DKDictionaryGetCountMethod          getCount;
    DKDictionaryGetObjectMethod         getObject;
    DKDictionaryApplyFunctionMethod     applyFunction;
    DKDictionaryInsertObjectMethod      insertObject;
    DKDictionaryRemoveObjectMethod      removeObject;
    DKDictionaryRemoveAllObjectsMethod  removeAllObjects;
};

typedef const struct DKDictionary DKDictionary;


DKClassRef  DKDictionaryClass( void );
void        DKSetDictionaryClass( DKClassRef _self );

DKIndex DKDictionaryGetCount( DKDictionaryRef _self );

void DKDictionarySetObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
void DKDictionaryAddObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
void DKDictionaryReplaceObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object );
void DKDictionaryAddEntriesFromDictionary( DKMutableDictionaryRef _self, DKDictionaryRef src );

int DKDictionaryContainsKey( DKDictionaryRef _self, DKObjectRef key );
int DKDictionaryContainsObject( DKDictionaryRef _self, DKObjectRef object );

DKListRef DKDictionaryCopyKeys( DKDictionaryRef _self );
DKListRef DKDictionaryCopyObjects( DKDictionaryRef _self );

DKObjectRef DKDictionaryGetObject( DKDictionaryRef _self, DKObjectRef key );

void DKDictionaryRemoveObject( DKMutableDictionaryRef _self, DKObjectRef key );
void DKDictionaryRemoveAllObjects( DKMutableDictionaryRef _self );

int DKDictionaryApplyFunction( DKDictionaryRef _self, DKDictionaryApplierFunction callback, void * context );


#endif // _DK_DICTIONARY_H_




