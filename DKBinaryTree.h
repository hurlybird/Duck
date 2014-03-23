//
//  scl_avltree.h
//  scl
//
//  Created by Derek Nylen on 2014-02-28.
//  Copyright (c) 2014 Derek W. Nylen. All rights reserved.
//

#ifndef _SCL_AVLTREE_H_
#define _SCL_AVLTREE_H_

#include "scl_value.h"
#include "scl_env.h"
#include "scl_pool.h"


typedef struct scl_avltree_node
{
    struct scl_avltree_node * left;
    struct scl_avltree_node * right;
    int level;
    
    scl_value key;
    scl_value value;

} scl_avltree_node;

typedef struct
{
    scl_pool node_pool;
    scl_avltree_node * root;
    size_t size;
    scl_value_cmp key_cmp;

} scl_avltree;

typedef int (*scl_avltree_traversal)( void * context, scl_value * key, scl_value * value );

void scl_avltree_init( scl_avltree * tree, scl_value_cmp key_cmp );
void scl_avltree_finalize( scl_avltree * tree );
 
size_t scl_avltree_size( scl_avltree * tree );

void scl_avltree_set( scl_avltree * tree, scl_value * key, scl_value * value );
void scl_avltree_add( scl_avltree * tree, scl_value * key, scl_value * value );
void scl_avltree_replace( scl_avltree * tree, scl_value * key, scl_value * value );

scl_value * scl_avltree_get( scl_avltree * tree, scl_value * key );
int scl_avltree_contains( scl_avltree * tree, scl_value * key );

void scl_avltree_remove( scl_avltree * tree, scl_value * key );
void scl_avltree_remove_all( scl_avltree * tree );

int scl_avltree_foreach( scl_avltree * tree, scl_avltree_traversal callback, void * context );
int scl_avltree_inorder( scl_avltree * tree, scl_avltree_traversal callback, void * context );



#endif // _SCL_TREE_H_



