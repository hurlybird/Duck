/*****************************************************************************************

  DKBinaryTree.h

  Copyright (c) 2014 Derek W. Nylen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

*****************************************************************************************/

#ifndef _DK_BINARY_TREE_H_
#define _DK_BINARY_TREE_H_

#include "DKDictionary.h"
#include "DKSet.h"


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



#endif // _DK_BINARY_TREE_H_



