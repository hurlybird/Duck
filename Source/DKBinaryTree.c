/*****************************************************************************************

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

*****************************************************************************************/

#include "DKBinaryTree.h"
#include "DKNodePool.h"
#include "DKString.h"
#include "DKComparison.h"
#include "DKCopying.h"
#include "DKDescription.h"
#include "DKEgg.h"


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
    
    DKCompareFunction keyCompare;
};


static DKObjectRef DKBinaryTreeInitialize( DKObjectRef _self );
static void        DKBinaryTreeFinalize( DKObjectRef _self );

static DKObjectRef DKBinaryTreeInitWithEgg( DKBinaryTreeRef _self, DKEggUnarchiverRef egg );
static void DKBinaryTreeAddToEgg( DKBinaryTreeRef _self, DKEggArchiverRef egg );

static DKIndex     INTERNAL_DKBinaryTreeGetCount( DKBinaryTreeRef _self );
static DKObjectRef INTERNAL_DKBinaryTreeGetObject( DKBinaryTreeRef _self, DKObjectRef key );

static void        INTERNAL_DKBinaryTreeSetObject( DKMutableBinaryTreeRef _self, DKObjectRef key, DKObjectRef object );
static void        INTERNAL_DKBinaryTreeInsertObject( DKMutableBinaryTreeRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy );
static void        INTERNAL_DKBinaryTreeRemoveObject( DKMutableBinaryTreeRef _self, DKObjectRef key );
static void        INTERNAL_DKBinaryTreeRemoveAllObjects( DKMutableBinaryTreeRef _self );

static void        INTERNAL_DKBinaryTreeAddObjectToSet( DKMutableBinaryTreeRef _self, DKObjectRef object );

///
//  DKBinaryTreeClass()
//
DKThreadSafeClassInit(  DKBinaryTreeClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKBinaryTree" ), DKObjectClass(), sizeof(struct DKBinaryTree), DKImmutableInstances, DKBinaryTreeInitialize, DKBinaryTreeFinalize );
    
    // Comparison
    struct DKComparisonInterface * comparison = DKNewInterface( DKSelector(Comparison), sizeof(struct DKComparisonInterface) );
    comparison->equal = (DKEqualityMethod)DKDictionaryEqual;
    comparison->compare = (DKCompareMethod)DKPointerCompare;
    comparison->hash = (DKHashMethod)DKPointerHash;

    DKInstallInterface( cls, comparison );
    DKRelease( comparison );

    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = DKRetain;
    copying->mutableCopy = (DKMutableCopyMethod)DKBinaryTreeMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );
    
    // Description
    struct DKDescriptionInterface * description = DKNewInterface( DKSelector(Description), sizeof(struct DKDescriptionInterface) );
    description->getDescription = (DKGetDescriptionMethod)DKKeyedCollectionGetDescription;
    description->getSizeInBytes = DKDefaultGetSizeInBytes;
    
    DKInstallInterface( cls, description );
    DKRelease( description );

    // Collection
    struct DKCollectionInterface * collection = DKNewInterface( DKSelector(Collection), sizeof(struct DKCollectionInterface) );
    collection->getCount = (DKGetCountMethod)INTERNAL_DKBinaryTreeGetCount;
    collection->containsObject = (DKContainsMethod)DKDictionaryContainsObject;
    collection->foreachObject = (DKForeachObjectMethod)DKBinaryTreeApplyFunctionToObjects;
    
    DKInstallInterface( cls, collection );
    DKRelease( collection );

    // KeyedCollection
    struct DKKeyedCollectionInterface * keyedCollection = DKNewInterface( DKSelector(KeyedCollection), sizeof(struct DKKeyedCollectionInterface) );
    keyedCollection->getCount = (DKGetCountMethod)INTERNAL_DKBinaryTreeGetCount;
    keyedCollection->containsObject = (DKContainsMethod)DKDictionaryContainsObject;
    keyedCollection->foreachObject = (DKForeachObjectMethod)DKBinaryTreeApplyFunctionToObjects;
    keyedCollection->containsKey = (DKContainsMethod)DKDictionaryContainsKey;
    keyedCollection->foreachKey = (DKForeachObjectMethod)DKBinaryTreeApplyFunctionToKeys;
    keyedCollection->foreachKeyAndObject = (DKForeachKeyAndObjectMethod)DKBinaryTreeApplyFunction;
    
    DKInstallInterface( cls, keyedCollection );
    DKRelease( keyedCollection );

    // Dictionary
    struct DKDictionaryInterface * dictionary = DKNewInterface( DKSelector(Dictionary), sizeof(struct DKDictionaryInterface) );
    dictionary->initWithVAKeysAndObjects = (DKDictionaryInitWithVAKeysAndObjectsMethod)DKBinaryTreeInitDictionaryWithVAKeysAndObjects;
    dictionary->initWithDictionary = (DKDictionaryInitWithDictionaryMethod)DKBinaryTreeInitDictionaryWithDictionary;
    
    dictionary->getCount = (DKGetCountMethod)INTERNAL_DKBinaryTreeGetCount;
    dictionary->getObject = (DKDictionaryGetObjectMethod)INTERNAL_DKBinaryTreeGetObject;
    
    dictionary->insertObject = (void *)DKImmutableObjectAccessError;
    dictionary->removeObject = (void *)DKImmutableObjectAccessError;
    dictionary->removeAllObjects = (void *)DKImmutableObjectAccessError;

    DKInstallInterface( cls, dictionary );
    DKRelease( dictionary );
    
    // Set
    struct DKSetInterface * set = DKNewInterface( DKSelector(Set), sizeof(struct DKSetInterface) );
    set->initWithVAObjects = (DKSetInitWithVAObjectsMethod)DKBinaryTreeInitSetWithVAObjects;
    set->initWithCArray = (DKSetInitWithCArrayMethod)DKBinaryTreeInitSetWithCArray;
    set->initWithCollection = (DKSetInitWithCollectionMethod)DKBinaryTreeInitSetWithCollection;

    set->getCount = (DKGetCountMethod)INTERNAL_DKBinaryTreeGetCount;
    set->getMember = (DKSetGetMemberMethod)INTERNAL_DKBinaryTreeGetObject;
    
    set->addObject = (void *)DKImmutableObjectAccessError;
    set->removeObject = (void *)DKImmutableObjectAccessError;
    set->removeAllObjects = (void *)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, set );
    DKRelease( set );
    
    // Property
    struct DKPropertyInterface * property = DKNewInterface( DKSelector(Property), sizeof(struct DKPropertyInterface) );
    property->getProperty = (DKGetPropertyMethod)INTERNAL_DKBinaryTreeGetObject;
    property->setProperty = (void *)DKImmutableObjectAccessError;
    
    DKInstallInterface( cls, property );
    DKRelease( property );

    // Egg
    struct DKEggInterface * egg = DKNewInterface( DKSelector(Egg), sizeof(struct DKEggInterface) );
    egg->initWithEgg = (DKInitWithEggMethod)DKBinaryTreeInitWithEgg;
    egg->addToEgg = (DKAddToEggMethod)DKBinaryTreeAddToEgg;
    
    DKInstallInterface( cls, egg );
    DKRelease( egg );

    return cls;
}


///
//  DKMutableBinaryTreeClass()
//
DKThreadSafeClassInit( DKMutableBinaryTreeClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKMutableBinaryTree" ), DKBinaryTreeClass(), sizeof(struct DKBinaryTree), 0, NULL, NULL );
    
    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying), sizeof(struct DKCopyingInterface) );
    copying->copy = (DKCopyMethod)DKBinaryTreeMutableCopy;
    copying->mutableCopy = (DKMutableCopyMethod)DKBinaryTreeMutableCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    // Dictionary
    struct DKDictionaryInterface * dictionary = DKNewInterface( DKSelector(Dictionary), sizeof(struct DKDictionaryInterface) );
    dictionary->initWithVAKeysAndObjects = (DKDictionaryInitWithVAKeysAndObjectsMethod)DKBinaryTreeInitDictionaryWithVAKeysAndObjects;
    dictionary->initWithDictionary = (DKDictionaryInitWithDictionaryMethod)DKBinaryTreeInitDictionaryWithDictionary;
    
    dictionary->getCount = (DKGetCountMethod)INTERNAL_DKBinaryTreeGetCount;
    dictionary->getObject = (DKDictionaryGetObjectMethod)INTERNAL_DKBinaryTreeGetObject;

    dictionary->insertObject = (DKDictionaryInsertObjectMethod)INTERNAL_DKBinaryTreeInsertObject;
    dictionary->removeObject = (DKDictionaryRemoveObjectMethod)INTERNAL_DKBinaryTreeRemoveObject;
    dictionary->removeAllObjects = (DKDictionaryRemoveAllObjectsMethod)INTERNAL_DKBinaryTreeRemoveAllObjects;

    DKInstallInterface( cls, dictionary );
    DKRelease( dictionary );
    
    // Set
    struct DKSetInterface * set = DKNewInterface( DKSelector(Set), sizeof(struct DKSetInterface) );
    set->initWithVAObjects = (DKSetInitWithVAObjectsMethod)DKBinaryTreeInitSetWithVAObjects;
    set->initWithCArray = (DKSetInitWithCArrayMethod)DKBinaryTreeInitSetWithCArray;
    set->initWithCollection = (DKSetInitWithCollectionMethod)DKBinaryTreeInitSetWithCollection;

    set->getCount = (DKGetCountMethod)INTERNAL_DKBinaryTreeGetCount;
    set->getMember = (DKSetGetMemberMethod)INTERNAL_DKBinaryTreeGetObject;
    
    set->addObject = (DKSetAddObjectMethod)INTERNAL_DKBinaryTreeAddObjectToSet;
    set->removeObject = (DKSetRemoveObjectMethod)INTERNAL_DKBinaryTreeRemoveObject;
    set->removeAllObjects = (DKSetRemoveAllObjectsMethod)INTERNAL_DKBinaryTreeRemoveAllObjects;
    
    DKInstallInterface( cls, set );
    DKRelease( set );
    
    // Property
    struct DKPropertyInterface * property = DKNewInterface( DKSelector(Property), sizeof(struct DKPropertyInterface) );
    property->getProperty = (DKGetPropertyMethod)INTERNAL_DKBinaryTreeGetObject;
    property->setProperty = (DKSetPropertyMethod)INTERNAL_DKBinaryTreeSetObject;
    
    DKInstallInterface( cls, property );
    DKRelease( property );
    
    return cls;
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
        int cmp = tree->keyCompare( (*node)->key, key );
        
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

    while( node != &tree->null_node )
    {
        int cmp = tree->keyCompare( node->key, key );
        
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
    
        int cmp = tree->keyCompare( (*node)->key, key );
        
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
            if( (*erase_node != &tree->null_node) && (tree->keyCompare( (*erase_node)->key, key ) == 0) )
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
//  InsertKeyAndObject()
//
static int InsertKeyAndObject( DKObjectRef key, DKObjectRef object, void * context )
{
    struct DKBinaryTree * tree = context;
    
    DKObjectRef keyCopy = DKCopy( key );
    
    Insert( tree, keyCopy, object, DKInsertAlways );
    
    DKRelease( keyCopy );
    
    return 0;
}


///
//  InsertObject()
//
static int InsertObject( DKObjectRef object, void * context )
{
    struct DKBinaryTree * tree = context;
    
    Insert( tree, object, object, DKInsertAlways );
    
    return 0;
}




// Interface =============================================================================

///
//  DKBinaryTreeInitialize()
//
static DKObjectRef DKBinaryTreeInitialize( DKObjectRef _self )
{
    _self = DKSuperInit( _self, DKObjectClass() );

    if( _self )
    {
        struct DKBinaryTree * tree = _self;
        
        DKNodePoolInit( &tree->nodePool, sizeof(struct DKBinaryTreeNode), 0 );

        tree->null_node.left = &tree->null_node;
        tree->null_node.right = &tree->null_node;
        tree->null_node.level = 0;
        
        tree->null_node.key = NULL;
        tree->null_node.object = NULL;
        
        tree->root = &tree->null_node;
        tree->count = 0;
        
        tree->keyCompare = DKCompare;
    }
    
    return _self;
}


///
//  DKBinaryTreeFinalize()
//
static void DKBinaryTreeFinalize( DKObjectRef _self )
{
    struct DKBinaryTree * tree = _self;

    INTERNAL_DKBinaryTreeRemoveAllObjects( tree );
    
    DKNodePoolFinalize( &tree->nodePool );
}


///
//  DKNewBinaryTreeWithCompareFunction()
//
DKMutableBinaryTreeRef DKNewBinaryTreeWithCompareFunction( DKCompareFunction keyCompare )
{
    struct DKBinaryTree * tree = DKNew( DKMutableBinaryTreeClass() );

    if( tree && keyCompare )
    {
        tree->keyCompare = keyCompare;
    }
    
    return tree;
}


///
//  DKBinaryTreeInitDictionaryWithVAKeysAndObjects()
//
DKObjectRef DKBinaryTreeInitDictionaryWithVAKeysAndObjects( DKBinaryTreeRef _self, va_list keysAndObjects )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );

        DKObjectRef key, object;
    
        while( (key = va_arg( keysAndObjects, DKObjectRef ) ) != NULL )
        {
            object = va_arg( keysAndObjects, DKObjectRef );
    
            DKObjectRef keyCopy = DKCopy( key );
    
            Insert( _self, keyCopy, object, DKInsertAlways );
            
            DKRelease( keyCopy );
        }

        CheckTreeIntegrity( _self );
    }
    
    return _self;
}


///
//  DKBinaryTreeInitDictionaryWithDictionary()
//
DKObjectRef DKBinaryTreeInitDictionaryWithDictionary( DKBinaryTreeRef _self, DKDictionaryRef dictionary )
{
    _self = DKInit( _self );
        
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );
        DKForeachKeyAndObject( dictionary, InsertKeyAndObject, _self );
    }
    
    return _self;
}


///
//  DKBinaryTreeInitSetWithVAObjects()
//
DKObjectRef DKBinaryTreeInitSetWithVAObjects( DKBinaryTreeRef _self, va_list objects )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );

        DKObjectRef object;

        while( (object = va_arg( objects, DKObjectRef ) ) != NULL )
        {
            Insert( _self, object, object, DKInsertAlways );
        }
    }
    
    return _self;
}


///
//  DKBinaryTreeInitSetWithCArray()
//
DKObjectRef DKBinaryTreeInitSetWithCArray( DKBinaryTreeRef _self, DKObjectRef objects[], DKIndex count )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );

        for( DKIndex i = 0; i < count; ++i )
        {
            DKObjectRef object = objects[i];
            Insert( _self, object, object, DKInsertAlways );
        }
    }
    
    return _self;
}


///
//  DKBinaryTreeInitSetWithCollection()
//
DKObjectRef DKBinaryTreeInitSetWithCollection( DKBinaryTreeRef _self, DKObjectRef collection )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );
        DKForeachObject( collection, InsertObject, _self );
    }
    
    return _self;
}


///
//  DKBinaryTreeInitWithEgg()
//
static DKObjectRef DKBinaryTreeInitWithEgg( DKBinaryTreeRef _self, DKEggUnarchiverRef egg )
{
    _self = DKInit( _self );
    
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );
        DKEggGetKeyedCollection( egg, DKSTR( "pairs" ), InsertKeyAndObject, _self );
    }
    
    return _self;
}


///
//  DKBinaryTreeAddToEgg()
//
static void DKBinaryTreeAddToEgg( DKBinaryTreeRef _self, DKEggArchiverRef egg )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );
        DKEggAddKeyedCollection( egg, DKSTR( "pairs" ), _self );
    }
}


///
//  DKBinaryTreeCopy()
//
DKBinaryTreeRef DKBinaryTreeCopy( DKBinaryTreeRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );

        struct DKBinaryTree * copy = DKNew( DKGetClass( _self ) );

        copy->keyCompare = _self->keyCompare;

        DKBinaryTreeApplyFunction( _self, InsertKeyAndObject, copy );
        
        return copy;
    }
    
    return NULL;
}


///
//  DKBinaryTreeMutableCopy()
//
DKMutableBinaryTreeRef DKBinaryTreeMutableCopy( DKBinaryTreeRef _self )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );

        struct DKBinaryTree * copy = DKNew( DKMutableBinaryTreeClass() );

        copy->keyCompare = _self->keyCompare;

        DKBinaryTreeApplyFunction( _self, InsertKeyAndObject, copy );
        
        return copy;
    }
    
    return NULL;
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

static DKIndex INTERNAL_DKBinaryTreeGetCount( DKBinaryTreeRef _self )
{
    return _self->count;
}


///
//  DKBinaryTreeGetObject()
//
DKObjectRef DKBinaryTreeGetObject( DKBinaryTreeRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKAssertKindOfClass( _self, DKBinaryTreeClass() );
        return INTERNAL_DKBinaryTreeGetObject( _self, key );
    }

    return NULL;
}

static DKObjectRef INTERNAL_DKBinaryTreeGetObject( DKBinaryTreeRef _self, DKObjectRef key )
{
    const struct DKBinaryTreeNode * node = FindNode( _self, key );

    if( node )
        return node->object;

    return NULL;
}


///
//  DKBinaryTreeApplyFunction()
//
int DKBinaryTreeApplyFunction( DKBinaryTreeRef _self, DKKeyedApplierFunction callback, void * context )
{
    return DKBinaryTreeTraverseInOrder( _self, callback, context );
}


///
//  DKBinaryTreeApplyFunctionToObjects()
//
struct ApplyFunctionContext
{
    DKApplierFunction callback;
    void * context;
};

static int ApplyFunctionToObjectsCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    struct ApplyFunctionContext * ctx = context;
    return ctx->callback( object, ctx->context );
}

int DKBinaryTreeApplyFunctionToObjects( DKBinaryTreeRef _self, DKApplierFunction callback, void * context )
{
    struct ApplyFunctionContext ctx = { callback, context };
    return DKBinaryTreeTraverseInOrder( _self, ApplyFunctionToObjectsCallback, &ctx );
}


///
//  DKBinaryTreeApplyFunctionToKeys()
//
static int ApplyFunctionToKeysCallback( DKObjectRef key, DKObjectRef object, void * context )
{
    struct ApplyFunctionContext * ctx = context;
    return ctx->callback( key, ctx->context );
}

int DKBinaryTreeApplyFunctionToKeys( DKBinaryTreeRef _self, DKApplierFunction callback, void * context )
{
    struct ApplyFunctionContext ctx = { callback, context };
    return DKBinaryTreeTraverseInOrder( _self, ApplyFunctionToKeysCallback, &ctx );
}


///
//  DKBinaryTreeTraverseInOrderInternal()
//
static int DKBinaryTreeTraverseInOrderInternal( const struct DKBinaryTree * tree, struct DKBinaryTreeNode * node, DKKeyedApplierFunction callback, void * context )
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

int DKBinaryTreeTraverseInOrder( DKBinaryTreeRef _self, DKKeyedApplierFunction callback, void * context )
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
//  DKBinaryTreeSetObject()
//
static void INTERNAL_DKBinaryTreeSetObject( DKMutableBinaryTreeRef _self, DKObjectRef key, DKObjectRef object )
{
    return INTERNAL_DKBinaryTreeInsertObject( _self, key, object, DKInsertAlways );
}


///
//  DKBinaryTreeInsertObject()
//
void DKBinaryTreeInsertObject( DKMutableBinaryTreeRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableBinaryTreeClass() );
        INTERNAL_DKBinaryTreeInsertObject( _self, key, object, policy );
    }
}

static void INTERNAL_DKBinaryTreeInsertObject( DKMutableBinaryTreeRef _self, DKObjectRef key, DKObjectRef object, DKInsertPolicy policy )
{
    DKObjectRef keyCopy = DKCopy ( key );

    Insert( _self, keyCopy, object, policy );
    
    DKRelease( keyCopy );

    CheckTreeIntegrity( _self );
}


///
//  DKBinaryTreeRemoveObject()
//
void DKBinaryTreeRemoveObject( DKMutableBinaryTreeRef _self, DKObjectRef key )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableBinaryTreeClass() );
        INTERNAL_DKBinaryTreeRemoveObject( _self, key );
    }
}

static void INTERNAL_DKBinaryTreeRemoveObject( DKMutableBinaryTreeRef _self, DKObjectRef key )
{
    struct DKBinaryTreeNode * leaf_node = NULL;
    struct DKBinaryTreeNode * erase_node = &_self->null_node;

    Remove( _self, key, &_self->root, &leaf_node, &erase_node );

    CheckTreeIntegrity( _self );
}


///
//  DKBinaryTreeRemoveAllObjects()
//
void DKBinaryTreeRemoveAllObjects( DKMutableBinaryTreeRef _self )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableBinaryTreeClass() );
        INTERNAL_DKBinaryTreeRemoveAllObjects( _self );
    }
}

static void RemoveAllRecursive( struct DKBinaryTree * tree, struct DKBinaryTreeNode * node )
{
    while( node != &tree->null_node )
    {
        struct DKBinaryTreeNode * next = node->right;
        
        RemoveAllRecursive( tree, node->left );
        FreeNode( tree, node );
        
        node = next;
    }
}

static void INTERNAL_DKBinaryTreeRemoveAllObjects( DKBinaryTreeRef tree )
{
    RemoveAllRecursive( tree, tree->root );
    DKAssert( tree->count == 0 );

    tree->root = &tree->null_node;

    CheckTreeIntegrity( tree );
}




///
//  DKBinaryTreeAddObjectToSet()
//
void DKBinaryTreeAddObjectToSet( DKMutableBinaryTreeRef _self, DKObjectRef object )
{
    if( _self )
    {
        DKCheckKindOfClass( _self, DKMutableBinaryTreeClass() );
        INTERNAL_DKBinaryTreeAddObjectToSet( _self, object );
    }
}

static void INTERNAL_DKBinaryTreeAddObjectToSet( DKMutableBinaryTreeRef _self, DKObjectRef object )
{
    Insert( _self, object, object, DKInsertIfNotFound );

    CheckTreeIntegrity( _self );
}






