//
//  DKBinaryTree.h
//  Duck
//
//  Created by Derek Nylen on 2014-02-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _DK_BINARY_TREE_H_
#define _DK_BINARY_TREE_H_

#include "DKDictionary.h"


typedef const struct DKBinaryTree * DKBinaryTreeRef;
typedef struct DKBinaryTree * DKMutableBinaryTreeRef;

DKClassRef DKBinaryTreeClass( void );
DKClassRef DKMutableBinaryTreeClass( void );

DKBinaryTreeRef DKBinaryTreeCreate( void );
DKBinaryTreeRef DKBinaryTreeCreateWithKeysAndObjects( DKCompareFunction compareKeys, DKObjectRef firstKey, ... );
DKBinaryTreeRef DKBinaryTreeCreateCopy( DKDictionaryRef srcDictionary );

DKMutableBinaryTreeRef DKBinaryTreeCreateMutable( DKCompareFunction compareKeys );
DKMutableBinaryTreeRef DKBinaryTreeCreateMutableCopy( DKDictionaryRef srcDictionary );

DKIndex     DKBinaryTreeGetCount( DKBinaryTreeRef _self );
DKObjectRef DKBinaryTreeGetObject( DKBinaryTreeRef _self, DKObjectRef key );

int         DKBinaryTreeApplyFunction( DKBinaryTreeRef _self, DKDictionaryApplierFunction callback, void * context );
int         DKBinaryTreeTraverseInOrder( DKBinaryTreeRef _self, DKDictionaryApplierFunction callback, void * context );

void        DKBinaryTreeInsertObject( DKMutableBinaryTreeRef _self, DKObjectRef key, DKObjectRef object, DKDictionaryInsertPolicy policy );
void        DKBinaryTreeRemoveObject( DKMutableBinaryTreeRef _self, DKObjectRef key );
void        DKBinaryTreeRemoveAllObjects( DKMutableBinaryTreeRef _self );




#endif // _DK_BINARY_TREE_H_



