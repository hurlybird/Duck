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

typedef DKIndex     (*DKDictionaryGetCountMethod)( DKDictionaryRef ref );
typedef DKObjectRef (*DKDictionaryGetObjectMethod)( DKDictionaryRef ref, DKObjectRef key );
typedef int         (*DKDictionaryApplyFunctionMethod)( DKDictionaryRef ref, DKDictionaryApplierFunction, void * context );
typedef void        (*DKDictionaryInsertObjectMethod)( DKMutableDictionaryRef ref, DKObjectRef key, DKObjectRef object, DKDictionaryInsertPolicy policy );
typedef void        (*DKDictionaryRemoveObjectMethod)( DKMutableDictionaryRef ref, DKObjectRef key );
typedef void        (*DKDictionaryRemoveAllObjectsMethod)( DKMutableDictionaryRef ref );

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
void        DKSetDictionaryClass( DKClassRef ref );

DKIndex DKDictionaryGetCount( DKDictionaryRef ref );

void DKDictionarySetObject( DKMutableDictionaryRef ref, DKObjectRef key, DKObjectRef object );
void DKDictionaryAddObject( DKMutableDictionaryRef ref, DKObjectRef key, DKObjectRef object );
void DKDictionaryReplaceObject( DKMutableDictionaryRef ref, DKObjectRef key, DKObjectRef object );
void DKDictionaryAddEntriesFromDictionary( DKMutableDictionaryRef ref, DKDictionaryRef src );

int DKDictionaryContainsKey( DKDictionaryRef ref, DKObjectRef key );
int DKDictionaryContainsObject( DKDictionaryRef ref, DKObjectRef object );

DKListRef DKDictionaryCopyKeys( DKDictionaryRef ref );
DKListRef DKDictionaryCopyObjects( DKDictionaryRef ref );

DKObjectRef DKDictionaryGetObject( DKDictionaryRef ref, DKObjectRef key );

void DKDictionaryRemoveObject( DKMutableDictionaryRef ref, DKObjectRef key );
void DKDictionaryRemoveAllObjects( DKMutableDictionaryRef ref );

int DKDictionaryApplyFunction( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context );


#endif // _DK_DICTIONARY_H_




