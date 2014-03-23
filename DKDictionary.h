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


DKDeclareInterface( Dictionary );


typedef const void * DKDictionaryRef;
typedef void * DKMutableDictionaryRef;


typedef int (*DKDictionaryApplierFunction)( void * context, DKTypeRef key, DKTypeRef value );


typedef enum
{
    DKDictionaryInsertAlways,
    DKDictionaryInsertIfFound,
    DKDictionaryInsertIfNotFound
    
} DKDictionaryInsertPolicy;


struct DKDictionary
{
    DKInterface _interface;

    DKIndex     (*getCount)( DKDictionaryRef ref );
    DKTypeRef   (*getObject)( DKDictionaryRef ref, DKTypeRef key );
    void        (*insertObject)( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy );
    void        (*removeObject)( DKMutableDictionaryRef ref, DKTypeRef key );
    void        (*removeAllObjects)( DKMutableDictionaryRef ref );
    int         (*applyFunction)( DKDictionaryRef ref, DKDictionaryApplierFunction, void * context );
};

typedef const struct DKDictionary DKDictionary;


DKIndex DKDictionaryGetCount( DKDictionaryRef ref );

void DKDictionarySetObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object );
void DKDictionaryAddObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object );
void DKDictionaryReplaceObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object );
void DKDictionaryAddEntriesFromDictionary( DKMutableDictionaryRef ref, DKDictionaryRef src );

int DKDictionaryContainsKey( DKDictionaryRef ref, DKTypeRef key );
int DKDictionaryContainsObject( DKDictionaryRef ref, DKTypeRef key );

DKListRef DKDictionaryCopyKeys( DKDictionaryRef ref );
DKListRef DKDictionaryCopyObjects( DKDictionaryRef ref );

DKTypeRef DKDictionaryGetObject( DKDictionaryRef ref, DKTypeRef key );

void DKDictionaryRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key );
void DKDictionaryRemoveAllObjects( DKMutableDictionaryRef ref );

int DKDictionaryApplyFunction( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context );


#endif // _DK_DICTIONARY_H_




