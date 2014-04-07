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


typedef const struct DKBinaryTree * DKBinaryTreeRef;
typedef struct DKBinaryTree * DKMutableBinaryTreeRef;

DKClassRef  DKBinaryTreeClass( void );
DKClassRef  DKMutableBinaryTreeClass( void );

#define     DKBinaryTreeCreateEmpty()    DKCreate( DKBinaryTreeClass() )
#define     DKBinaryTreeCreateMutable()  DKCreate( DKMutableBinaryTreeClass() )

DKObjectRef DKBinaryTreeCreateDictionaryWithDictionary( DKClassRef _class, DKDictionaryRef dictionary );

DKObjectRef DKBinaryTreeCreateSetWithCArray( DKClassRef _class, DKObjectRef objects[], DKIndex count );
DKObjectRef DKBinaryTreeCreateSetWithCollection( DKClassRef _class, DKObjectRef collection );

DKBinaryTreeRef DKBinaryTreeCopy( DKBinaryTreeRef _self );
DKMutableBinaryTreeRef DKBinaryTreeMutableCopy( DKBinaryTreeRef _self );

DKIndex     DKBinaryTreeGetCount( DKBinaryTreeRef _self );
DKObjectRef DKBinaryTreeGetObject( DKBinaryTreeRef _self, DKObjectRef key );

int         DKBinaryTreeApplyFunction( DKBinaryTreeRef _self, DKKeyedApplierFunction callback, void * context );
int         DKBinaryTreeApplyFunctionToKeys( DKBinaryTreeRef _self, DKApplierFunction callback, void * context );
int         DKBinaryTreeApplyFunctionToObjects( DKBinaryTreeRef _self, DKApplierFunction callback, void * context );

int         DKBinaryTreeTraverseInOrder( DKBinaryTreeRef _self, DKKeyedApplierFunction callback, void * context );

void        DKBinaryTreeInsertObject( DKMutableBinaryTreeRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
void        DKBinaryTreeRemoveObject( DKMutableBinaryTreeRef _self, DKObjectRef key );
void        DKBinaryTreeRemoveAllObjects( DKMutableBinaryTreeRef _self );




#endif // _DK_BINARY_TREE_H_



