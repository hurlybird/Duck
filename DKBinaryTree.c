//
//  scl_avltree.c
//  scl
//
//  Created by Derek Nylen on 2014-02-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#include "scl_avltree.h"


// Internals =============================================================================

///
//  alloc_node()
//
static scl_avltree_node * alloc_node( scl_avltree * tree, scl_value * key, scl_value * value )
{
    scl_avltree_node * node = scl_pool_alloc( &tree->node_pool );

    node->left = NULL;
    node->right = NULL;
    node->level = 0;

    scl_value_init( &node->key );
    scl_value_copy( &node->key, key );

    scl_value_init( &node->value );
    
    if( value )
        scl_value_copy( &node->value, value );
    
    tree->size++;
    
    return node;
}


///
//  free_node()
//
static void free_node( scl_avltree * tree, scl_avltree_node * node )
{
    if( node )
    {
        free_node( tree, node->left );
        free_node( tree, node->right );

        node->left = NULL;
        node->right = NULL;
        node->level = 0;

        scl_value_finalize( &node->key );
        scl_value_finalize( &node->value );

        scl_pool_free( &tree->node_pool, node );
        
        assert( tree->size > 0 );
        tree->size--;
    }
}


///
//  rotate_left()
//
static scl_avltree_node * rotate_left( scl_avltree_node * k2 )
{
    scl_avltree_node * k1 = k2->left;
    k2->left = k1->right;
    k1->right = k2;
    return k1;
}


///
//  rotate_right()
//
static scl_avltree_node * rotate_right( scl_avltree_node * k1 )
{
    scl_avltree_node * k2 = k1->right;
    k1->right = k2->left;
    k2->left = k1;
    return k2;
}


///
//  skew()
//
static void skew( scl_avltree_node ** node )
{
    scl_avltree_node * left = (*node)->left;

    if( left && (left->level == (*node)->level) )
    {
        *node = rotate_left( *node );
    }
}


///
//  split()
//
static void split( scl_avltree_node ** node )
{
    scl_avltree_node * right = (*node)->right->right;

    if( right && (right->level == (*node)->level) )
    {
        *node = rotate_right( *node );
        (*node)->level++;
    }
}


///
//  insert()
//
enum
{
    InsertAlways,
    InsertIfFound,
    InsertIfNotFound
};

static void insert( scl_avltree * tree, scl_avltree_node ** node, scl_value * key, scl_value * value, int behaviour )
{
    if( *node == NULL )
    {
        if( behaviour != InsertIfFound )
            *node = alloc_node( tree, key, value );
        
        return;
    }
    
    int cmp = tree->key_cmp( key, &((*node)->key) );
    
    if( cmp < 0 )
    {
        insert( tree, &(*node)->left, key, value, behaviour );
    }
        
    else if( cmp > 0 )
    {
        insert( tree, &(*node)->right, key, value, behaviour );
    }
        
    else
    {
        if( behaviour != InsertIfNotFound )
            scl_value_copy( &(*node)->value, value );
        
        return;
    }
    
    skew( node );
    split( node );
}


///
//  find_node()
//
static scl_avltree_node * find_node( scl_avltree * tree, scl_value * key )
{
    scl_avltree_node * node = tree->root;

    while( node )
    {
        int cmp = tree->key_cmp( key, &node->key );
        
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
//  erase()
//
static void erase( scl_avltree * tree, scl_value * key, scl_avltree_node ** node, scl_avltree_node ** leaf_node, scl_avltree_node ** erase_node )
{
    if( node )
    {
        *leaf_node = *node;
    
        int cmp = tree->key_cmp( key, &((*node)->key) );
        
        if( cmp < 0 )
        {
            erase( tree, key, &(*node)->left, leaf_node, erase_node );
        }
            
        else
        {
            if( cmp == 0 )
                *erase_node = *node;
                
            erase( tree, key, &(*node)->right, leaf_node, erase_node );
        }
            
        if( *leaf_node == *node )
        {
            if( *erase_node )
            {
                scl_value_swap( &(*erase_node)->key, &(*node)->key );
                scl_value_swap( &(*erase_node)->value, &(*node)->value );
                *erase_node = NULL;
                
                *node = (*node)->right;
                free_node( tree, *leaf_node );
                *leaf_node = NULL;
            }
        }
        
        else if( ((*node)->left->level < ((*node)->level - 1)) ||
                 ((*node)->right->level < ((*node)->level - 1)) )
        {
            if( (*node)->right->level > --((*node)->level) )
                (*node)->right->level = (*node)->level;
        
            skew( node );
            skew( &(*node)->right );
            skew( &(*node)->right->right );
            split( node );
            split( &(*node)->right );
        }
    }
}




// Interface =============================================================================

///
//  scl_avltree_init()
//
void scl_avltree_init( scl_avltree * tree, scl_value_cmp key_cmp )
{
    scl_pool_init( &tree->node_pool, sizeof(scl_avltree_node), 0 );
    
    tree->root = NULL;
    tree->size = 0;
    tree->key_cmp = key_cmp;
}
 
 
///
//  scl_avltree_finalize()
//
void scl_avltree_finalize( scl_avltree * tree )
{
    scl_avltree_remove_all( tree );
    scl_pool_finalize( &tree->node_pool );
 
    tree->root = NULL;
    tree->size = 0;
}
 
 
///
//  scl_avltree_size()
//
size_t scl_avltree_size( scl_avltree * dict )
{
    return dict->size;
}
 
 
///
//  scl_avltree_set()
//
void scl_avltree_set( scl_avltree * tree, scl_value * key, scl_value * value )
{
    insert( tree, &tree->root, key, value, InsertAlways );
}


///
//  scl_avltree_add()
//
void scl_avltree_add( scl_avltree * tree, scl_value * key, scl_value * value )
{
    insert( tree, &tree->root, key, value, InsertIfNotFound );
}


///
//  scl_avltree_replace()
//
void scl_avltree_replace( scl_avltree * tree, scl_value * key, scl_value * value )
{
    insert( tree, &tree->root, key, value, InsertIfFound );
}


///
//  scl_avltree_get()
//
scl_value * scl_avltree_get( scl_avltree * tree, scl_value * key )
{
    scl_avltree_node * node = find_node( tree, key );
    
    if( node )
        return &node->value;

    return NULL;
}


///
//  scl_avltree_contains()
//
int scl_avltree_contains( scl_avltree * tree, scl_value * key )
{
    scl_avltree_node * node = find_node( tree, key );
    return node != NULL;
}


///
//  scl_avltree_remove()
//
void scl_avltree_remove( scl_avltree * tree, scl_value * key )
{
    scl_avltree_node * last_node = NULL;
    scl_avltree_node * erase_node = NULL;

    erase( tree, key, &tree->root, &last_node, &erase_node );
}


///
//  scl_avltree_remove_all()
//
void scl_avltree_remove_all( scl_avltree * tree )
{
    free_node( tree, tree->root );
    tree->root = NULL;
    tree->size = 0;
}


///
//  scl_avltree_foreach()
//
int scl_avltree_foreach( scl_avltree * tree, scl_avltree_traversal callback, void * context )
{
    return scl_avltree_inorder( tree, callback, context );
}


///
//  scl_avltree_inorder()
//
static int inorder( scl_avltree_node * node, scl_avltree_traversal callback, void * context )
{
    int result = 0;

    while( node )
    {
        if( (result = inorder( node->left, callback, context )) != 0 )
            break;
                
        if( (result = callback( context, &node->key, &node->value )) != 0 )
            break;

        node = node->right;
    }
    
    return result;
}

int scl_avltree_inorder( scl_avltree * tree, scl_avltree_traversal callback, void * context )
{
    return inorder( tree->root, callback, context );
}








