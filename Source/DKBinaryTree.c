/*******************************************************************************

  DKBinaryTree.c

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

*******************************************************************************/

#include "DKBinaryTree.h"
#include "DKNodePool.h"
#include "DKCopying.h"
#include "DKString.h"


struct DKBinaryTreeNode
{
    struct DKBinaryTreeNode * left;
    struct DKBinaryTreeNode * right;
    DKIndex level;
    
    DKObjectRef key;
    DKObjectRef object;
};

struct DKBinaryTree
{
    DKObject _obj;

    DKNodePool nodePool;
    struct DKBinaryTreeNode null_node;
    struct DKBinaryTreeNode * root;
    DKIndex count;
    
    DKCompareFunction compareKeys;
};



static DKObjectRef DKBinaryTreeInitialize( DKObjectRef _self );
static void      DKBinaryTreeFinalize( DKObjectRef _self );

static void      RemoveAll( struct DKBinaryTree * tree, struct DKBinaryTreeNode * node );

static void      DKImmutableBinaryTreeInsertObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
static void      DKImmutableBinaryTreeRemoveObject( DKMutableDictionaryRef _self, DKObjectRef key );
static void      DKImmutableBinaryTreeRemoveAllObjects( DKMutableDictionaryRef _self );


///
//  DKBinaryTreeClass()
//
DKThreadSafeClassInit(  DKBinaryTreeClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKBinaryTree" ), DKObjectClass(), sizeof(struct DKBinaryTree), 0 );
    
    // Allocation
    struct DKAllocation * allocation = DKAllocInterface( DKSelector(Allocation), sizeof(DKAllocation) );
    allocation->initialize = DKBinaryTreeInitialize;
    allocation->finalize = DKBinaryTreeFinalize;

    DKInstallInterface( cls, allocation );
    DKRelease( allocation );

    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKBinaryTreeCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Description
    struct DKDescription * description = DKAllocInterface( DKSelector(Description), sizeof(DKDescription) );
    description->copyDescription = (DKCopyDescriptionMethod)DKDictionaryCopyDescription;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Dictionary
    struct DKDictionary * dictionary = DKAllocInterface( DKSelector(Dictionary), sizeof(DKDictionary) );
    dictionary->getCount = (DKDictionaryGetCountMethod)DKBinaryTreeGetCount;
    dictionary->getObject = (DKDictionaryGetObjectMethod)DKBinaryTreeGetObject;
    dictionary->applyFunction = (DKDictionaryApplyFunctionMethod)DKBinaryTreeApplyFunction;
    dictionary->insertObject = DKImmutableBinaryTreeInsertObject;
    dictionary->removeObject = DKImmutableBinaryTreeRemoveObject;
    dictionary->removeAllObjects = DKImmutableBinaryTreeRemoveAllObjects;

    DKInstallInterface( cls, dictionary );
    DKRelease( dictionary );
    
    return cls;
}


///
//  DKMutableBinaryTreeClass()
//
DKThreadSafeClassInit( DKMutableBinaryTreeClass )
{
    DKClassRef cls = DKAllocClass( DKSTR( "DKMutableBinaryTree" ), DKBinaryTreeClass(), sizeof(struct DKBinaryTree), 0 );
    
    // Copying
    struct DKCopying * copying = DKAllocInterface( DKSelector(Copying), sizeof(DKCopying) );
    copying->copy = (DKCopyMethod)DKBinaryTreeCreateMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKBinaryTreeCreateMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Dictionary
    struct DKDictionary * dictionary = DKAllocInterface( DKSelector(Dictionary), sizeof(DKDictionary) );
    dictionary->getCount = (DKDictionaryGetCountMethod)DKBinaryTreeGetCount;
    dictionary->getObject = (DKDictionaryGetObjectMethod)DKBinaryTreeGetObject;
    dictionary->applyFunction = (DKDictionaryApplyFunctionMethod)DKBinaryTreeApplyFunction;
    dictionary->insertObject = (DKDictionaryInsertObjectMethod)DKBinaryTreeInsertObject;
    dictionary->removeObject = (DKDictionaryRemoveObjectMethod)DKBinaryTreeRemoveObject;
    dictionary->removeAllObjects = (DKDictionaryRemoveAllObjectsMethod)DKBinaryTreeRemoveAllObjects;

    DKInstallInterface( cls, dictionary );
    DKRelease( dictionary );
    
    return cls;
}


///
//  DKBinaryTreeInitialize()
//
static DKObjectRef DKBinaryTreeInitialize( DKObjectRef _self )
{
    struct DKBinaryTree * tree = (struct DKBinaryTree *)_self;
    
    DKNodePoolInit( &tree->nodePool, sizeof(struct DKBinaryTreeNode), 0 );

    tree->null_node.left = &tree->null_node;
    tree->null_node.right = &tree->null_node;
    tree->null_node.level = 0;
    
    tree->null_node.key = NULL;
    tree->null_node.object = NULL;
    
    tree->root = &tree->null_node;
    tree->count = 0;
    
    tree->compareKeys = DKCompare;
    
    return _self;
}


///
//  DKBinaryTreeFinalize()
//
static void DKBinaryTreeFinalize( DKObjectRef _self )
{
    struct DKBinaryTree * tree = (struct DKBinaryTree *)_self;

    RemoveAll( tree, tree->root );
    
    DKNodePoolFinalize( &tree->nodePool );
}




// Internals =============================================================================

///
//  CheckTreeIntegrity()
//
#if DK_RUNTIME_INTEGRITY_CHECKS
static DKIndex CheckTreeIntegrityRecursive( const struct DKBinaryTree * tree, const struct DKBinaryTreeNode * node )
{
    DKIndex count = 0;

    if( node != &tree->null_node )
    {
        count = 1;

        if( node->left != &tree->null_node )
        {
            DKAssert( node->left->level == (node->level - 1) );
        }
        
        if( node->right != &tree->null_node )
        {
            DKAssert( (node->right->level == node->level) || (node->right->level == (node->level - 1)) );
        }
        
        if( node->level >= 2 )
        {
            DKAssert( node->left != &tree->null_node );
            DKAssert( node->right != &tree->null_node );
        }
        
        if( node->right->level != node->level )
        {
            DKAssert( node->right->level == node->left->level );
        }
        
        count += CheckTreeIntegrityRecursive( tree, node->left );
        count += CheckTreeIntegrityRecursive( tree, node->right );
    }
    
    return count;
}

static int CountNodes( void * context, DKObjectRef key, DKObjectRef object )
{
    DKIndex * count = context;
    (*count)++;
    
    return 0;
}

static void CheckTreeIntegrity( const struct DKBinaryTree * tree )
{
    DKAssert( tree->null_node.left == &tree->null_node );
    DKAssert( tree->null_node.right == &tree->null_node );
    DKAssert( tree->null_node.level == 0 );
    
    DKAssert( tree->null_node.key == NULL );
    DKAssert( tree->null_node.object == NULL );

    DKIndex count = CheckTreeIntegrityRecursive( tree, tree->root );
    DKAssert( count == tree->count );
}
#else
#define CheckTreeIntegrity( tree )
#endif


///
//  AllocNode()
//
static struct DKBinaryTreeNode * AllocNode( struct DKBinaryTree * tree, DKObjectRef key, DKObjectRef object )
{
    struct DKBinaryTreeNode * node = DKNodePoolAlloc( &tree->nodePool );

    node->left = &tree->null_node;
    node->right = &tree->null_node;
    node->level = 0;

    node->key = DKRetain( key );
    node->object = DKRetain( object );
    
    tree->count++;
    
    return node;
}


///
//  FreeNode()
//
static void FreeNode( struct DKBinaryTree * tree, struct DKBinaryTreeNode * node )
{
    node->left = NULL;
    node->right = NULL;
    node->level = 0;

    DKRelease( node->key );
    DKRelease( node->object );
    
    DKAssert( tree->count > 0 );
    tree->count--;
}


///
//  RotateLeft()
//
static struct DKBinaryTreeNode * RotateWithLeftChild( struct DKBinaryTreeNode * k2 )
{
    struct DKBinaryTreeNode * k1 = k2->left;
    k2->left = k1->right;
    k1->right = k2;
    return k1;
}


///
//  RotateRight()
//
static struct DKBinaryTreeNode * RotateWithRightChild( struct DKBinaryTreeNode * k1 )
{
    struct DKBinaryTreeNode * k2 = k1->right;
    k1->right = k2->left;
    k2->left = k1;
    return k2;
}


///
//  Skew()
//
static void Skew( struct DKBinaryTree * tree, struct DKBinaryTreeNode ** node )
{
    struct DKBinaryTreeNode * left = (*node)->left;

    if( left->level == (*node)->level )
    {
        *node = RotateWithLeftChild( *node );
    }
}


///
//  Split()
//
static void Split( struct DKBinaryTree * tree, struct DKBinaryTreeNode ** node )
{
    struct DKBinaryTreeNode * right = (*node)->right;

    if( right->right->level == (*node)->level )
    {
        *node = RotateWithRightChild( *node );
        (*node)->level++;
    }
}


///
//  Insert()
//
static void InsertRecursive( struct DKBinaryTree * tree, struct DKBinaryTreeNode ** node,
    DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    if( *node == &tree->null_node )
    {
        if( policy == DKInsertIfFound )
            return;
        
        *node = AllocNode( tree, key, object );
    }
    
    else
    {
        int cmp = tree->compareKeys( (*node)->key, key );
        
        if( cmp < 0 )
        {
            InsertRecursive( tree, &(*node)->left, key, object, policy );
        }
            
        else if( cmp > 0 )
        {
            InsertRecursive( tree, &(*node)->right, key, object, policy );
        }
            
        else
        {
            if( policy != DKInsertIfNotFound )
            {
                DKRetain( object );
                DKRelease( (*node)->object );
                (*node)->object = object;
            }
            
            return;
        }
    }
    
    Skew( tree, node );
    Split( tree, node );
}

static void Insert( struct DKBinaryTree * tree, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    if( key == NULL )
    {
        DKError( "DKBinaryTreeInsert: Trying to insert a NULL key.\n" );
        return;
    }

    InsertRecursive( tree, &tree->root, key, object, policy );
}


///
//  FindNode()
//
static const struct DKBinaryTreeNode * FindNode( const struct DKBinaryTree * tree, DKObjectRef key )
{
    struct DKBinaryTreeNode * node = tree->root;

    while( node )
    {
        int cmp = tree->compareKeys( node->key, key );
        
        if( cmp < 0 )
            node = node->left;
            
        else if( cmp > 0 )
            node = node->right;
            
        else
            return node;
    }

    return NULL;
}


///
//  Swap()
//
static void Swap( struct DKBinaryTreeNode * node1, struct DKBinaryTreeNode * node2 )
{
    DKObjectRef tmp_key = node1->key;
    node1->key = node2->key;
    node2->key = tmp_key;

    DKObjectRef tmp_obj = node1->object;
    node1->object = node2->object;
    node2->object = tmp_obj;
}


///
//  Remove()
//
static void Remove( struct DKBinaryTree * tree, DKObjectRef key, struct DKBinaryTreeNode ** node, struct DKBinaryTreeNode ** leaf_node, struct DKBinaryTreeNode ** erase_node )
{
    if( *node != &tree->null_node )
    {
        *leaf_node = *node;
    
        int cmp = tree->compareKeys( (*node)->key, key );
        
        if( cmp < 0 )
        {
            Remove( tree, key, &(*node)->left, leaf_node, erase_node );
        }
            
        else
        {
            *erase_node = *node;
            Remove( tree, key, &(*node)->right, leaf_node, erase_node );
        }
        
        if( *leaf_node == *node )
        {
            if( (*erase_node != &tree->null_node) && (tree->compareKeys( (*erase_node)->key, key ) == 0) )
            {
                DKAssert( (*node)->left == &tree->null_node );
                
                Swap( *erase_node, *node );
                
                *erase_node = &tree->null_node;

                *node = (*node)->right;
                
                FreeNode( tree, *leaf_node );
                *leaf_node = NULL;
            }
        }
        
        else if( ((*node)->left->level < ((*node)->level - 1)) ||
                 ((*node)->right->level < ((*node)->level - 1)) )
        {
            if( (*node)->right->level > --((*node)->level) )
                (*node)->right->level = (*node)->level;
        
            Skew( tree, node );
            Skew( tree, &(*node)->right );
            Skew( tree, &(*node)->right->right );
            Split( tree, node );
            Split( tree, &(*node)->right );
        }
    }
}


///
//  RemoveAll()
//
static void RemoveAll( struct DKBinaryTree * tree, struct DKBinaryTreeNode * node )
{
    while( node != &tree->null_node )
    {
        struct DKBinaryTreeNode * next = node->right;
        
        RemoveAll( tree, node->left );
        FreeNode( tree, node );
        
        node = next;
    }

    tree->root = &tree->null_node;
    tree->count = 0;

    CheckTreeIntegrity( tree );
}




// Interface =============================================================================

///
//  DKBinaryTreeCreate()
//
DKBinaryTreeRef DKBinaryTreeCreate( void )
{
    return DKAllocObject( DKBinaryTreeClass(), 0 );
}


///
//  DKBinaryTreeCreateWithKeysAndObjects()
//
DKBinaryTreeRef DKBinaryTreeCreateWithKeysAndObjects( DKCompareFunction compareKeys, DKObjectRef firstKey, ... )
{
    struct DKBinaryTree * tree = DKAllocObject( DKBinaryTreeClass(), 0 );
    
    if( tree )
    {
        tree->compareKeys = compareKeys;

        va_list arg_ptr;
        va_start( arg_ptr, firstKey );

        for( DKObjectRef key = firstKey; key != NULL; )
        {
            DKObjectRef object = va_arg( arg_ptr, DKObjectRef );

            Insert( tree, key, object, DKInsertAlways );
            
            key = va_arg( arg_ptr, DKObjectRef );
        }

        va_end( arg_ptr );

        CheckTreeIntegrity( tree );
    }
    
    return tree;
}


///
//  DKBinaryTreeCreateCopy()
//
DKBinaryTreeRef DKBinaryTreeCreateCopy( DKDictionaryRef srcDictionary )
{
    DKMutableDictionaryRef _self = DKBinaryTreeCreateMutableCopy( srcDictionary );

    // Turn the mutable tree into an immutable tree
    struct DKObject * obj = (struct DKObject *)_self;
    DKRelease( obj->isa );
    obj->isa = DKRetain( DKBinaryTreeClass() );
    
    return _self;
}


///
//  DKBinaryTreeCreateMutable()
//
DKMutableBinaryTreeRef DKBinaryTreeCreateMutable( DKCompareFunction compareKeys )
{
    struct DKBinaryTree * tree = DKAllocObject( DKMutableBinaryTreeClass(), 0 );
    
    if( tree )
    {
        tree->compareKeys = compareKeys;
    }
    
    return tree;
}


///
//  DKBinaryTreeCreateMutableCopy()
//
DKMutableBinaryTreeRef DKBinaryTreeCreateMutableCopy( DKDictionaryRef srcDictionary )
{
    DKCompareFunction compareKeys = DKCompare;

    if( DKIsKindOfClass( srcDictionary, DKBinaryTreeClass() ) )
    {
        DKBinaryTreeRef srcTree = srcDictionary;
        compareKeys = srcTree->compareKeys;
    }

    DKMutableDictionaryRef _self = DKBinaryTreeCreateMutable( compareKeys );
    DKDictionaryAddEntriesFromDictionary( _self, srcDictionary );
    
    return _self;
}


///
//  DKBinaryTreeGetCount()
//
DKIndex DKBinaryTreeGetCount( DKBinaryTreeRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );
        return _self->count;
    }
    
    return 0;
}

///
//  DKBinaryTreeGetObject()
//
DKObjectRef DKBinaryTreeGetObject( DKBinaryTreeRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );

        const struct DKBinaryTreeNode * node = FindNode( _self, key );
    
        if( node )
            return node->object;
    }

    return NULL;
}


///
//  DKBinaryTreeApplyFunction()
//
int DKBinaryTreeApplyFunction( DKBinaryTreeRef _self, DKDictionaryApplierFunction callback, void * context )
{
    return DKBinaryTreeTraverseInOrder( _self, callback, context );
}


///
//  DKBinaryTreeTraverseInOrderInternal()
//
static int DKBinaryTreeTraverseInOrderInternal( const struct DKBinaryTree * tree, struct DKBinaryTreeNode * node, DKDictionaryApplierFunction callback, void * context )
{
    int result = 0;

    while( node != &tree->null_node )
    {
        if( (result = DKBinaryTreeTraverseInOrderInternal( tree, node->left, callback, context )) != 0 )
            break;
                
        if( (result = callback( node->key, node->object, context )) != 0 )
            break;

        node = node->right;
    }
    
    return result;
}

int DKBinaryTreeTraverseInOrder( DKBinaryTreeRef _self, DKDictionaryApplierFunction callback, void * context )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );

        const struct DKBinaryTree * tree = _self;
        return DKBinaryTreeTraverseInOrderInternal( tree, tree->root, callback, context );
    }
    
    return 0;
}


///
//  DKBinaryTreeInsertObject()
//
static void DKImmutableBinaryTreeInsertObject( DKMutableDictionaryRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    DKError( "DKBinaryTreeInsertObject: Trying to modify an immutable object." );
}

void DKBinaryTreeInsertObject( DKMutableBinaryTreeRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableBinaryTreeClass() );

        Insert( _self, key, object, policy );

        CheckTreeIntegrity( _self );
    }
}


///
//  DKBinaryTreeRemoveObject()
//
static void DKImmutableBinaryTreeRemoveObject( DKMutableDictionaryRef _self, DKObjectRef key )
{
    DKError( "DKBinaryTreeRemoveObject: Trying to modify an immutable object." );
}

void DKBinaryTreeRemoveObject( DKMutableBinaryTreeRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableBinaryTreeClass() );

        struct DKBinaryTreeNode * leaf_node = NULL;
        struct DKBinaryTreeNode * erase_node = &_self->null_node;

        Remove( _self, key, &_self->root, &leaf_node, &erase_node );

        CheckTreeIntegrity( _self );
    }
}


///
//  DKBinaryTreeRemoveAllObjects()
//
static void DKImmutableBinaryTreeRemoveAllObjects( DKMutableDictionaryRef _self )
{
    DKError( "DKBinaryTreeRemoveAllObjects: Trying to modify an immutable object." );
}

void DKBinaryTreeRemoveAllObjects( DKMutableBinaryTreeRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKMutableBinaryTreeClass() );
        
        RemoveAll( _self, _self->root );
    }
}








