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
    int level;
    
    DKHashIndex hash;
    DKTypeRef key;
    DKTypeRef object;
};

struct DKBinaryTree
{
    DKObjectHeader _obj;

    DKNodePool nodePool;
    struct DKBinaryTreeNode * root;
    DKIndex count;
};



static DKTypeRef    DKBinaryTreeAllocate( void );
static DKTypeRef    DKMutableBinaryTreeAllocate( void );
static DKTypeRef    DKBinaryTreeInitialize( DKTypeRef ref );
static void         DKBinaryTreeFinalize( DKTypeRef ref );
static DKTypeRef    DKBinaryTreeCopy( DKTypeRef ref );
static DKTypeRef    DKMutableBinaryTreeCopy( DKTypeRef ref );
static DKTypeRef    DKBinaryTreeMutableCopy( DKTypeRef ref );


///
//  DKBinaryTreeClass()
//
DKTypeRef DKBinaryTreeClass( void )
{
    static DKTypeRef binaryTreeClass = NULL;

    if( !binaryTreeClass )
    {
        binaryTreeClass = DKCreateClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKBinaryTreeAllocate;
        lifeCycle->initialize = DKBinaryTreeInitialize;
        lifeCycle->finalize = DKBinaryTreeFinalize;

        DKInstallInterface( binaryTreeClass, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKBinaryTreeCopy;
        copying->mutableCopy = DKBinaryTreeMutableCopy;
        
        DKInstallInterface( binaryTreeClass, copying );
        DKRelease( copying );
        
        // Dictionary
        struct DKDictionary * dictionary = (struct DKDictionary *)DKCreateInterface( DKSelector(Dictionary), sizeof(DKDictionary) );
        dictionary->getCount = DKBinaryTreeGetCount;
        dictionary->getObject = DKBinaryTreeGetObject;
        dictionary->insertObject = DKBinaryTreeInsertObject;
        dictionary->removeObject = DKBinaryTreeRemoveObject;
        dictionary->removeAllObjects = DKBinaryTreeRemoveAllObjects;
        dictionary->applyFunction = DKBinaryTreeApplyFunction;

        DKInstallInterface( binaryTreeClass, dictionary );
        DKRelease( dictionary );
    }
    
    return binaryTreeClass;
}


///
//  DKMutableBinaryTreeClass()
//
DKTypeRef DKMutableBinaryTreeClass( void )
{
    static DKTypeRef mutableBinaryTreeClass = NULL;

    if( !mutableBinaryTreeClass )
    {
        mutableBinaryTreeClass = DKCreateClass( DKObjectClass() );
        
        // LifeCycle
        struct DKLifeCycle * lifeCycle = (struct DKLifeCycle *)DKCreateInterface( DKSelector(LifeCycle), sizeof(DKLifeCycle) );
        lifeCycle->allocate = DKMutableBinaryTreeAllocate;
        lifeCycle->initialize = DKBinaryTreeInitialize;
        lifeCycle->finalize = DKBinaryTreeFinalize;

        DKInstallInterface( mutableBinaryTreeClass, lifeCycle );
        DKRelease( lifeCycle );

        // Copying
        struct DKCopying * copying = (struct DKCopying *)DKCreateInterface( DKSelector(Copying), sizeof(DKCopying) );
        copying->copy = DKMutableBinaryTreeCopy;
        copying->mutableCopy = DKBinaryTreeMutableCopy;
        
        DKInstallInterface( mutableBinaryTreeClass, copying );
        DKRelease( copying );

        // Dictionary
        struct DKDictionary * dictionary = (struct DKDictionary *)DKCreateInterface( DKSelector(Dictionary), sizeof(DKDictionary) );
        dictionary->getCount = DKBinaryTreeGetCount;
        dictionary->getObject = DKBinaryTreeGetObject;
        dictionary->insertObject = DKBinaryTreeInsertObject;
        dictionary->removeObject = DKBinaryTreeRemoveObject;
        dictionary->removeAllObjects = DKBinaryTreeRemoveAllObjects;
        dictionary->applyFunction = DKBinaryTreeApplyFunction;

        DKInstallInterface( mutableBinaryTreeClass, dictionary );
        DKRelease( dictionary );
    }
    
    return mutableBinaryTreeClass;
}


///
//  DKBinaryTreeAllocate()
//
static DKTypeRef DKBinaryTreeAllocate( void )
{
    return DKAllocObject( DKBinaryTreeClass(), sizeof(struct DKBinaryTree), 0 );
}


///
//  DKMutableBinaryTreeAllocate()
//
static DKTypeRef DKMutableBinaryTreeAllocate( void )
{
    return DKAllocObject( DKMutableBinaryTreeClass(), sizeof(struct DKBinaryTree), DKObjectIsMutable );
}


///
//  DKBinaryTreeInitialize()
//
static DKTypeRef DKBinaryTreeInitialize( DKTypeRef ref )
{
    ref = DKObjectInitialize( ref );
    
    if( ref )
    {
        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        
        DKNodePoolInit( &tree->nodePool, sizeof(struct DKBinaryTreeNode), 0 );

        tree->root = NULL;
        tree->count = 0;
    }
    
    return ref;
}


///
//  DKBinaryTreeFinalize()
//
static void DKBinaryTreeFinalize( DKTypeRef ref )
{
    if( ref )
    {
        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;

        DKBinaryTreeRemoveAllObjects( tree );
        
        DKNodePoolClear( &tree->nodePool );
    }
}


///
//  DKBinaryTreeCopy()
//
static DKTypeRef DKBinaryTreeCopy( DKTypeRef ref )
{
    return DKRetain( ref );
}


///
//  DKMutableBinaryTreeCopy()
//
static DKTypeRef DKMutableBinaryTreeCopy( DKTypeRef ref )
{
    return DKBinaryTreeCreateCopy( ref );
}


///
//  DKBinaryTreeMutableCopy()
//
static DKTypeRef DKBinaryTreeMutableCopy( DKTypeRef ref )
{
    return DKBinaryTreeCreateMutableCopy( ref );
}






// Internals =============================================================================

///
//  AllocNode()
//
static struct DKBinaryTreeNode * AllocNode( struct DKBinaryTree * tree, DKTypeRef key, DKTypeRef object )
{
    struct DKBinaryTreeNode * node = DKNodePoolAlloc( &tree->nodePool );

    node->left = NULL;
    node->right = NULL;
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
    while( node )
    {
        struct DKBinaryTreeNode * next = node->right;
        
        FreeNode( tree, node->left );

        node->left = NULL;
        node->right = NULL;
        node->level = 0;

        DKRelease( node->key );
        DKRelease( node->object );
        
        DKAssert( tree->count > 0 );
        tree->count--;
        
        node = next;
    }
}


///
//  RotateLeft()
//
static struct DKBinaryTreeNode * RotateLeft( struct DKBinaryTreeNode * k2 )
{
    struct DKBinaryTreeNode * k1 = k2->left;
    k2->left = k1->right;
    k1->right = k2;
    return k1;
}


///
//  RotateRight()
//
static struct DKBinaryTreeNode * RotateRight( struct DKBinaryTreeNode * k1 )
{
    struct DKBinaryTreeNode * k2 = k1->right;
    k1->right = k2->left;
    k2->left = k1;
    return k2;
}


///
//  Skew()
//
static void Skew( struct DKBinaryTreeNode ** node )
{
    struct DKBinaryTreeNode * left = (*node)->left;

    if( left && (left->level == (*node)->level) )
    {
        *node = RotateLeft( *node );
    }
}


///
//  Split()
//
static void Split( struct DKBinaryTreeNode ** node )
{
    struct DKBinaryTreeNode * right = (*node)->right->right;

    if( right && (right->level == (*node)->level) )
    {
        *node = RotateRight( *node );
        (*node)->level++;
    }
}


///
//  Compare()
//
static int Compare( struct DKBinaryTreeNode * node, DKHashIndex hash, DKTypeRef key )
{
    int cmp;
    
    if( hash < node->hash )
        cmp = 1;
    
    else if( hash > node->hash )
        cmp = -1;
    
    else
        cmp = DKCompare( key, node->key );
    
    return cmp;
}


///
//  Insert()
//
static void Insert( struct DKBinaryTree * tree, struct DKBinaryTreeNode ** node,
    DKHashIndex hash, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
    if( *node == NULL )
    {
        if( policy != DKDictionaryInsertIfFound )
            *node = AllocNode( tree, key, object );
        
        return;
    }
    
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
    
    Skew( node );
    Split( node );
}


///
//  FindNode()
//
static struct DKBinaryTreeNode * FindNode( struct DKBinaryTree * tree, DKHashIndex hash, DKTypeRef key )
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
    DKHashIndex tmp_hash;
    DKTypeRef tmp_ref;
    
    tmp_hash = node1->hash;
    node1->hash = node2->hash;
    node2->hash = tmp_hash;
    
    tmp_ref = node1->key;
    node1->key = node2->key;
    node2->key = tmp_hash;

    tmp_ref = node1->object;
    node1->object = node2->object;
    node2->object = tmp_hash;
}


///
//  Erase()
//
static void Erase( struct DKBinaryTree * tree, DKHashIndex hash, DKTypeRef key, struct DKBinaryTreeNode ** node, struct DKBinaryTreeNode ** leaf_node, struct DKBinaryTreeNode ** erase_node )
{
    if( node )
    {
        *leaf_node = *node;
    
        int cmp = Compare( *node, hash, key );
        
        if( cmp < 0 )
        {
            Erase( tree, hash, key, &(*node)->left, leaf_node, erase_node );
        }
            
        else
        {
            if( cmp == 0 )
                *erase_node = *node;
                
            Erase( tree, hash, key, &(*node)->right, leaf_node, erase_node );
        }
            
        if( *leaf_node == *node )
        {
            if( *erase_node )
            {
                Swap( *erase_node, *node );
                *erase_node = NULL;
                
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
        
            Skew( node );
            Skew( &(*node)->right );
            Skew( &(*node)->right->right );
            Split( node );
            Split( &(*node)->right );
        }
    }
}




// Interface =============================================================================

///
//  DKBinaryTreeCreate()
//
DKDictionaryRef DKBinaryTreeCreate( DKTypeRef keys[], DKTypeRef objects[], DKIndex count )
{
    DKMutableDictionaryRef ref = DKBinaryTreeCreateMutable();
    
    for( DKIndex i = 0; i < count; ++i )
    {
        DKBinaryTreeInsertObject( ref, keys[i], objects[i], DKDictionaryInsertAlways );
    }
    
    // Turn the mutable tree into an immutable tree
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    obj->isa = DKBinaryTreeClass();
    obj->attributes &= ~DKObjectIsMutable;
    
    return ref;
}


///
//  DKBinaryTreeCreateWithKeysAndObjects()
//
DKDictionaryRef DKBinaryTreeCreateWithKeysAndObjects( DKTypeRef firstKey, ... )
{
    DKMutableDictionaryRef ref = DKBinaryTreeCreateMutable();

    va_list arg_ptr;
    va_start( arg_ptr, firstKey );

    for( DKTypeRef key = firstKey; key != NULL; )
    {
        DKTypeRef object = va_arg( arg_ptr, DKTypeRef );

        DKBinaryTreeInsertObject( ref, key, object, DKDictionaryInsertAlways );
        
        key = va_arg( arg_ptr, DKTypeRef );
    }

    va_end( arg_ptr );
    
    // Turn the mutable tree into an immutable tree
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    obj->isa = DKBinaryTreeClass();
    obj->attributes &= ~DKObjectIsMutable;
    
    return ref;
}


///
//  DKBinaryTreeCreateCopy()
//
DKDictionaryRef DKBinaryTreeCreateCopy( DKDictionaryRef srcDictionary )
{
    DKMutableDictionaryRef ref = DKBinaryTreeCreateMutableCopy( srcDictionary );

    // Turn the mutable tree into an immutable tree
    struct DKObjectHeader * obj = (struct DKObjectHeader *)ref;
    obj->isa = DKBinaryTreeClass();
    obj->attributes &= ~DKObjectIsMutable;
    
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
        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        return tree->count;
    }
    
    return 0;
}

///
//  DKBinaryTreeInsertObject()
//
void DKBinaryTreeInsertObject( DKMutableDictionaryRef ref, DKTypeRef key, DKTypeRef object, DKDictionaryInsertPolicy policy )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKBinaryTreeInsertObject: Trying to modify an immutable object." );
            return;
        }
        
        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        DKHashIndex hash = DKHash( key );

        Insert( tree, &tree->root, hash, key, object, policy );
    }
}


///
//  DKBinaryTreeGetObject()
//
DKTypeRef DKBinaryTreeGetObject( DKDictionaryRef ref, DKTypeRef key )
{
    if( ref )
    {
        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        DKHashIndex hash = DKHash( key );
    
        struct DKBinaryTreeNode * node = FindNode( tree, hash, key );
    
        if( node )
            return node->object;
    }

    return NULL;
}


///
//  DKBinaryTreeRemoveObject()
//
void DKBinaryTreeRemoveObject( DKMutableDictionaryRef ref, DKTypeRef key )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKBinaryTreeRemoveObject: Trying to modify an immutable object." );
            return;
        }
        
        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        DKHashIndex hash = DKHash( key );

        struct DKBinaryTreeNode * last_node = NULL;
        struct DKBinaryTreeNode * erase_node = NULL;

        Erase( tree, hash, key, &tree->root, &last_node, &erase_node );
    }
}


///
//  DKBinaryTreeRemoveAllObjects()
//
void DKBinaryTreeRemoveAllObjects( DKMutableDictionaryRef ref )
{
    if( ref )
    {
        if( !DKTestObjectAttribute( ref, DKObjectIsMutable ) )
        {
            DKError( "DKBinaryTreeRemoveAllObjects: Trying to modify an immutable object." );
            return;
        }
        
        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;

        FreeNode( tree, tree->root );
        
        tree->root = NULL;
        tree->count = 0;
    }
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
static int DKBinaryTreeTraverseInOrderInternal( struct DKBinaryTreeNode * node, DKDictionaryApplierFunction callback, void * context )
{
    int result = 0;

    while( node )
    {
        if( (result = DKBinaryTreeTraverseInOrderInternal( node->left, callback, context )) != 0 )
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
        struct DKBinaryTree * tree = (struct DKBinaryTree *)ref;
        return DKBinaryTreeTraverseInOrderInternal( tree->root, callback, context );
    }
    
    return 0;
}








