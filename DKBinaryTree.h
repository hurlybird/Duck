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


DKTypeRef DKBinaryTreeClass( void );
DKTypeRef DKMutableBinaryTreeClass( void );

DKDictionaryRef DKBinaryTreeCreate( DKTypeRef keys[], DKTypeRef objects[], DKIndex count );
DKDictionaryRef DKBinaryTreeCreateWithKeysAndObjects( DKTypeRef firstKey, ... );
DKDictionaryRef DKBinaryTreeCreateCopy( DKDictionaryRef srcDictionary );

DKMutableDictionaryRef DKBinaryTreeCreateMutable( void );
DKMutableDictionaryRef DKBinaryTreeCreateMutableCopy( DKDictionaryRef srcDictionary );

DKIndex DKBinaryTreeGetCount( DKDictionaryRef ref );

void DKBinaryTreeInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy );

DKTypeRef DKBinaryTreeGetObject( DKDictionaryRef ref, DKTypeRef key );

void DKBinaryTreeRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key );
void DKBinaryTreeRemoveAllObjects( DKMutableDictionaryRef ref );

int DKBinaryTreeApplyFunction( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context );
int DKBinaryTreeTraverseInOrder( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context );



#endif // _DK_BINARY_TREE_H_



