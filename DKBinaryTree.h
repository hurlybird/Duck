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
DKBinaryTreeRef DKBinaryTreeCreateWithKeysAndObjects( DKObjectRef firstKey, ... );
DKBinaryTreeRef DKBinaryTreeCreateCopy( DKDictionaryRef srcDictionary );

DKMutableBinaryTreeRef DKBinaryTreeCreateMutable( void );
DKMutableBinaryTreeRef DKBinaryTreeCreateMutableCopy( DKDictionaryRef srcDictionary );

DKIndex     DKBinaryTreeGetCount( DKBinaryTreeRef ref );
DKObjectRef DKBinaryTreeGetObject( DKBinaryTreeRef ref, DKObjectRef key );

int         DKBinaryTreeApplyFunction( DKBinaryTreeRef ref, DKDictionaryApplierFunction callback, void * context );
int         DKBinaryTreeTraverseInOrder( DKBinaryTreeRef ref, DKDictionaryApplierFunction callback, void * context );

void        DKBinaryTreeInsertObject( DKMutableBinaryTreeRef ref, DKObjectRef key, DKObjectRef object, DKDictionaryInsertPolicy policy );
void        DKBinaryTreeRemoveObject( DKMutableBinaryTreeRef ref, DKObjectRef key );
void        DKBinaryTreeRemoveAllObjects( DKMutableBinaryTreeRef ref );




#endif // _DK_BINARY_TREE_H_



