// =======================================================================================
//
// DKBinaryTree.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_BINARY_TREE_H_
#define _DK_BINARY_TREE_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct DKBinaryTree * DKBinaryTreeRef;
typedef struct DKBinaryTree * DKMutableBinaryTreeRef;

DK_API DKClassRef  DKBinaryTreeClass( void );
DK_API DKClassRef  DKMutableBinaryTreeClass( void );

#define     DKEmptyBinaryTree()         DKAutorelease( DKNew( DKBinaryTreeClass() ) )
#define     DKMutableBinaryTree()       DKAutorelease( DKNew( DKMutableBinaryTreeClass() ) )

#define     DKNewMutableBinaryTree()    DKNew( DKMutableBinaryTreeClass() )

#define DKBinaryTreeWithCompareFunction( keyCompare )   DKAutorelease( DKNewBinaryTreeWithCompareFunction( keyCompare ) )
DK_API DKMutableBinaryTreeRef DKNewBinaryTreeWithCompareFunction( DKCompareFunction keyCompare );

DK_API DKObjectRef DKBinaryTreeInitDictionaryWithVAKeysAndObjects( DKBinaryTreeRef _self, va_list keysAndObjects );
DK_API DKObjectRef DKBinaryTreeInitDictionaryWithDictionary( DKBinaryTreeRef _self, DKDictionaryRef dictionary );

DK_API DKObjectRef DKBinaryTreeInitSetWithVAObjects( DKBinaryTreeRef _self, va_list objects );
DK_API DKObjectRef DKBinaryTreeInitSetWithCArray( DKBinaryTreeRef _self, DKObjectRef objects[], DKIndex count );
DK_API DKObjectRef DKBinaryTreeInitSetWithCollection( DKBinaryTreeRef _self, DKObjectRef collection );

DK_API DKBinaryTreeRef DKBinaryTreeCopy( DKBinaryTreeRef _self );
DK_API DKMutableBinaryTreeRef DKBinaryTreeMutableCopy( DKBinaryTreeRef _self );

DK_API DKIndex     DKBinaryTreeGetCount( DKBinaryTreeRef _self );
DK_API DKObjectRef DKBinaryTreeGetObject( DKBinaryTreeRef _self, DKObjectRef key );

DK_API int         DKBinaryTreeApplyFunction( DKBinaryTreeRef _self, DKKeyedApplierFunction callback, void * context );
DK_API int         DKBinaryTreeApplyFunctionToKeys( DKBinaryTreeRef _self, DKApplierFunction callback, void * context );
DK_API int         DKBinaryTreeApplyFunctionToObjects( DKBinaryTreeRef _self, DKApplierFunction callback, void * context );

DK_API int         DKBinaryTreeTraverseInOrder( DKBinaryTreeRef _self, DKKeyedApplierFunction callback, void * context );
DK_API DKObjectRef DKBinaryTreeGetFirstObject( DKBinaryTreeRef _self );

DK_API void        DKBinaryTreeInsertObject( DKMutableBinaryTreeRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
DK_API void        DKBinaryTreeRemoveObject( DKMutableBinaryTreeRef _self, DKObjectRef key );
DK_API void        DKBinaryTreeRemoveAllObjects( DKMutableBinaryTreeRef _self );

DK_API void        DKBinaryTreeAddObjectToSet( DKMutableBinaryTreeRef _self, DKObjectRef object );


#ifdef __cplusplus
}
#endif

#endif // _DK_BINARY_TREE_H_



