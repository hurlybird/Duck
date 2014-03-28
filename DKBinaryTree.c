//
//  DKBinaryTree.c
//  Duck
//
//  Created by Derek Nylen on 2014-02-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//
#include "DKBinaryTree.h"
#include "DKNodePool.h"
#include "DKCopying.h"


struct DKBinaryTreeNode
{
    struct DKBinaryTreeNode * left;
    struct DKBinaryTreeNode * right;
    DKIndex level;
    
    DKHashCode hash;
    DKTypeRef key;
    DKTypeRef object;
};

struct DKBinaryTree
{
    DKObjectHeader _obj;

    DKNodePool nodePool;
    struct DKBinaryTreeNode null_node;
    struct DKBinaryTreeNode * root;
    DKIndex count;
};



static DKTypeRef DKBinaryTreeInitialize( DKTypeRef ref );
static void      DKBinaryTreeFinalize( DKTypeRef ref );

static void      RemoveAll( struct DKBinaryTree * tree, struct DKBinaryTreeNode * node );

static void      DKImmutableBinaryTreeInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy );
static void      DKImmutableBinaryTreeRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key );
static void      DKImmutableBinaryTreeRemoveAllObjects( DKMutableDictionaryRef ref );


///
//  DKBinaryTreeClass()
//
DKTypeRef DKBinaryTreeClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKBinaryTree", DKObjectClass(), sizeof(struct DKBinaryTree) );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->initialize = DKBinaryTreeInitialize;
        lifeCycle->finalize = DKBinaryTreeFinalize;

        DKInstallInterface( SharedClassObject, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKRetain;
        copying->mutableCopy = DKBinaryTreeCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );
        
        // Dictionary
        struct DKDictionary * dictionary = (struct DKDictionary *)DKCreateInterface( DKSelector(Dictionary), sizeof(DKDictionary) );
        dictionary->getCount = DKBinaryTreeGetCount;
        dictionary->getObject = DKBinaryTreeGetObject;
        dictionary->applyFunction = DKBinaryTreeApplyFunction;
        dictionary->insertObject = DKImmutableBinaryTreeInsertObject;
        dictionary->removeObject = DKImmutableBinaryTreeRemoveObject;
        dictionary->removeAllObjects = DKImmutableBinaryTreeRemoveAllObjects;

        DKInstallInterface( SharedClassObject, dictionary );
        DKRelease( dictionary );
    }
    
    return SharedClassObject;
}


///
//  DKMutableBinaryTreeClass()
//
DKTypeRef DKMutableBinaryTreeClass( void )
{
    static DKTypeRef SharedClassObject = NULL;

    if( !SharedClassObject )
    {
        SharedClassObject = DKCreateClass( "DKMutableBinaryTree", DKBinaryTreeClass(), sizeof(struct DKBinaryTree) );
        
        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKBinaryTreeCreateMutableCopy;
        copying->mutableCopy = DKBinaryTreeCreateMutableCopy;
        
        DKInstallInterface( SharedClassObject, copying );
        DKRelease( copying );

        // Dictionary
        struct DKDictionary * dictionary = (struct DKDictionary *)DKCreateInterface( DKSelector(Dictionary), sizeof(DKDictionary) );
        dictionary->getCount = DKBinaryTreeGetCount;
        dictionary->getObject = DKBinaryTreeGetObject;
        dictionary->applyFunction = DKBinaryTreeApplyFunction;
        dictionary->insertObject = DKBinaryTreeInsertObject;
        dictionary->removeObject = DKBinaryTreeRemoveObject;
        dictionary->removeAllObjects = DKBinaryTreeRemoveAllObjects;

        DKInstallInterface( SharedClassObject, dictionary );
        DKRelease( dictionary );
    }
    
    return SharedClassObject;
}


///
//  DKBinaryTreeInitialize()
//
static DKTypeRef DKBinaryTreeInitialize( DKTypeRef ref )
{
    struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
    
    DKNodePoolInit( &tree->nodePool, sizeof(struct DKBinaryTreeNode), 0 );

    tree->null_node.left = &tree->null_node;
    tree->null_node.right = &tree->null_node;
    tree->null_node.level = 0;
    
    tree->null_node.hash = 0;
    tree->null_node.key = NULL;
    tree->null_node.object = NULL;
    
    tree->root = &tree->null_node;
    tree->count = 0;
    
    return ref;
}


///
//  DKBinaryTreeFinalize()
//
static void DKBinaryTreeFinalize( DKTypeRef ref )
{
    struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;

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

static int CountNodes( void * context, DKTypeRef key, DKTypeRef object )
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
    
    DKAssert( tree->null_node.hash == 0 );
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
static struct DKBinaryTreeNode * AllocNode( struct DKBinaryTree * tree, DKTypeRef key, DKTypeRef object )
{
    struct DKBinaryTreeNode * node = DKNodePoolAlloc( &tree->nodePool );

    node->left = &tree->null_node;
    node->right = &tree->null_node;
    node->level = 0;

    node->hash = DKHash( key );
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
//  Compare()
//
static int Compare( struct DKBinaryTreeNode * node, DKHashCode hash, DKTypeRef key )
{
    int cmp;
    
    if( node->hash < hash )
        cmp = 1;
    
    else if( node->hash > hash )
        cmp = -1;
    
    else
        cmp = DKCompare( key, node->key );
    
    return cmp;
}


///
//  Insert()
//
static void Insert( struct DKBinaryTree * tree, struct DKBinaryTreeNode ** node,
    DKHashCode hash, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
    if( *node == &tree->null_node )
    {
        if( policy == DKDictionaryInsertIfFound )
            return;
        
        *node = AllocNode( tree, key, object );
    }
    
    else
    {
        int cmp = Compare( *node, hash, key );
        
        if( cmp < 0 )
        {
            Insert( tree, &(*node)->left, hash, key, object, policy );
        }
            
        else if( cmp > 0 )
        {
            Insert( tree, &(*node)->right, hash, key, object, policy );
        }
            
        else
        {
            if( policy != DKDictionaryInsertIfNotFound )
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


///
//  FindNode()
//
static const struct DKBinaryTreeNode * FindNode( const struct DKBinaryTree * tree, DKHashCode hash, DKTypeRef key )
{
    struct DKBinaryTreeNode * node = tree->root;

    while( node )
    {
        int cmp = Compare( node, hash, key );
        
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
    DKHashCode tmp_hash = node1->hash;
    node1->hash = node2->hash;
    node2->hash = tmp_hash;
    
    DKTypeRef tmp_key = node1->key;
    node1->key = node2->key;
    node2->key = tmp_key;

    DKTypeRef tmp_obj = node1->object;
    node1->object = node2->object;
    node2->object = tmp_obj;
}


///
//  Remove()
//
static void Remove( struct DKBinaryTree * tree, DKHashCode hash, DKTypeRef key, struct DKBinaryTreeNode ** node, struct DKBinaryTreeNode ** leaf_node, struct DKBinaryTreeNode ** erase_node )
{
    if( *node != &tree->null_node )
    {
        *leaf_node = *node;
    
        int cmp = Compare( *node, hash, key );
        
        if( cmp < 0 )
        {
            Remove( tree, hash, key, &(*node)->left, leaf_node, erase_node );
        }
            
        else
        {
            *erase_node = *node;
            Remove( tree, hash, key, &(*node)->right, leaf_node, erase_node );
        }
        
        if( *leaf_node == *node )
        {
            if( (*erase_node != &tree->null_node) && (Compare( *erase_node, hash, key ) == 0) )
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
DKDictionaryRef DKBinaryTreeCreate( DKTypeRef keys[], DKTypeRef objects[], DKIndex count )
{
    struct DKBinaryTree * tree = (struct DKBinaryTree *)DKCreate( DKBinaryTreeClass() );
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKHashCode hash = DKHash( keys[i] );
        Insert( tree, &tree->root, hash, keys[i], objects[i], DKDictionaryInsertAlways );
    }

    CheckTreeIntegrity( tree );
    
    return tree;
}


///
//  DKBinaryTreeCreateWithKeysAndObjects()
//
DKDictionaryRef DKBinaryTreeCreateWithKeysAndObjects( DKTypeRef firstKey, ... )
{
    struct DKBinaryTree * tree = (struct DKBinaryTree *)DKCreate( DKBinaryTreeClass() );

    va_list arg_ptr;
    va_start( arg_ptr, firstKey );

    for( DKTypeRef key = firstKey; key != NULL; )
    {
        DKTypeRef object = va_arg( arg_ptr, DKTypeRef );

        DKHashCode hash = DKHash( key );
        Insert( tree, &tree->root, hash, key, object, DKDictionaryInsertAlways );
        
        key = va_arg( arg_ptr, DKTypeRef );
    }

    va_end( arg_ptr );

    CheckTreeIntegrity( tree );
    
    return tree;
}


///
//  DKBinaryTreeCreateCopy()
//
DKDictionaryRef DKBinaryTreeCreateCopy( DKDictionaryRef srcDictionary )
{
    DKMutableDictionaryRef ref = DKBinaryTreeCreateMutableCopy( srcDictionary );

    // Turn the mutable tree into an immutable tree
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    DKRelease( obj->isa );
    obj->isa = DKRetain( DKBinaryTreeClass() );
    
    return ref;
}


///
//  DKBinaryTreeCreateMutable()
//
DKMutableDictionaryRef DKBinaryTreeCreateMutable( void )
{
    return (DKMutableDictionaryRef)DKCreate( DKMutableBinaryTreeClass() );
}


///
//  DKBinaryTreeCreateMutableCopy()
//
DKMutableDictionaryRef DKBinaryTreeCreateMutableCopy( DKDictionaryRef srcDictionary )
{
    DKMutableDictionaryRef ref = DKBinaryTreeCreateMutable();
    DKDictionaryAddEntriesFromDictionary( ref, srcDictionary );
    
    return ref;
}


///
//  DKBinaryTreeGetCount()
//
DKIndex DKBinaryTreeGetCount( DKDictionaryRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKBinaryTreeClass(), 0 );

        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        return tree->count;
    }
    
    return 0;
}

///
//  DKBinaryTreeGetObject()
//
DKTypeRef DKBinaryTreeGetObject( DKDictionaryRef ref, DKTypeRef key )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKBinaryTreeClass(), NULL );

        const struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        DKHashCode hash = DKHash( key );
    
        const struct DKBinaryTreeNode * node = FindNode( tree, hash, key );
    
        if( node )
            return node->object;
    }

    return NULL;
}


///
//  DKBinaryTreeApplyFunction()
//
int DKBinaryTreeApplyFunction( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context )
{
    return DKBinaryTreeTraverseInOrder( ref, callback, context );
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
                
        if( (result = callback( context, node->key, node->object )) != 0 )
            break;

        node = node->right;
    }
    
    return result;
}

int DKBinaryTreeTraverseInOrder( DKDictionaryRef ref, DKDictionaryApplierFunction callback, void * context )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKBinaryTreeClass(), 0 );

        const struct DKBinaryTree * tree = ref;
        return DKBinaryTreeTraverseInOrderInternal( tree, tree->root, callback, context );
    }
    
    return 0;
}


///
//  DKBinaryTreeInsertObject()
//
static void DKImmutableBinaryTreeInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
    DKError( "DKBinaryTreeInsertObject: Trying to modify an immutable object." );
}

void DKBinaryTreeInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableBinaryTreeClass() );

        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        DKHashCode hash = DKHash( key );

        Insert( tree, &tree->root, hash, key, object, policy );

        CheckTreeIntegrity( tree );
    }
}


///
//  DKBinaryTreeRemoveObject()
//
static void DKImmutableBinaryTreeRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key )
{
    DKError( "DKBinaryTreeRemoveObject: Trying to modify an immutable object." );
}

void DKBinaryTreeRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableBinaryTreeClass() );

        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        DKHashCode hash = DKHash( key );

        struct DKBinaryTreeNode * leaf_node = NULL;
        struct DKBinaryTreeNode * erase_node = &tree->null_node;

        Remove( tree, hash, key, &tree->root, &leaf_node, &erase_node );

        CheckTreeIntegrity( tree );
    }
}


///
//  DKBinaryTreeRemoveAllObjects()
//
static void DKImmutableBinaryTreeRemoveAllObjects( DKMutableDictionaryRef ref )
{
    DKError( "DKBinaryTreeRemoveAllObjects: Trying to modify an immutable object." );
}

void DKBinaryTreeRemoveAllObjects( DKMutableDictionaryRef ref )
{
    if( ref )
    {
        DKVerifyKindOfClass( ref, DKMutableBinaryTreeClass() );
        
        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;

        RemoveAll( tree, tree->root );
    }
}








