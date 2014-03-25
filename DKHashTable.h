//
//  DKHashTable.h
//  Duck
//
//  Created by Derek Nylen on 2014-03-24.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_HASHTABLE_H_
#define _DK_HASHTABLE_H_

#include "DKDictionary.h"


DKTypeRef DKHashTableClass( void );
DKTypeRef DKMutableHashTableClass( void );

DKDictionaryRef DKHashTableCreate( DKTypeRef keys[], DKTypeRef objects[], DKIndex count );
DKDictionaryRef DKHashTableCreateWithKeysAndObjects( DKTypeRef firstKey, ... );
DKDictionaryRef DKHashTableCreateCopy( DKDictionaryRef srcDictionary );

DKMutableDictionaryRef DKHashTableCreateMutable( void );
DKMutableDictionaryRef DKHashTableCreateMutableCopy( DKDictionaryRef srcDictionary );

DKIndex     DKHashTableGetCount( DKDictionaryRef ref );
DKTypeRef   DKHashTableGetObject( DKDictionaryRef ref, DKTypeRef key );

int         DKHashTableApplyFunction( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context );

void        DKHashTableInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy );
void        DKHashTableRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key );
void        DKHashTableRemoveAllObjects( DKMutableDictionaryRef ref );




#endif // _DK_HASHTABLE_H_
