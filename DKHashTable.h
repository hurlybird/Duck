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


typedef const struct DKHashTable * DKHashTableRef;
typedef struct DKHashTable * DKMutableHashTableRef;


DKClassRef DKHashTableClass( void );
DKClassRef DKMutableHashTableClass( void );

DKHashTableRef DKHashTableCreate( void );
DKHashTableRef DKHashTableCreateWithKeysAndObjects( DKObjectRef firstKey, ... );
DKHashTableRef DKHashTableCreateCopy( DKDictionaryRef srcDictionary );

DKMutableHashTableRef DKHashTableCreateMutable( void );
DKMutableHashTableRef DKHashTableCreateMutableCopy( DKDictionaryRef srcDictionary );

DKIndex     DKHashTableGetCount( DKHashTableRef ref );
DKObjectRef DKHashTableGetObject( DKHashTableRef ref, DKObjectRef key );

int         DKHashTableApplyFunction( DKHashTableRef ref, DKDictionaryApplierFunction callback, void * context );

void        DKHashTableInsertObject( DKMutableHashTableRef ref, DKObjectRef key, DKObjectRef object, DKDictionaryInsertPolicy policy );
void        DKHashTableRemoveObject( DKMutableHashTableRef ref, DKObjectRef key );
void        DKHashTableRemoveAllObjects( DKMutableHashTableRef ref );




#endif // _DK_HASHTABLE_H_
