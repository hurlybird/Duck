/*****************************************************************************************

  DKGraph.c

  Copyright (c) 2017 Derek W. Nylen

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

#include "DKConfig.h"
#include "DKPlatform.h"
#include "DKEncoding.h"
#include "DKRuntime.h"
#include "DKGraph.h"
#include "DKCollection.h"
#include "DKCopying.h"
#include "DKList.h"
#include "DKDictionary.h"
#include "DKSet.h"
#include "DKBinaryTree.h"
#include "DKLinkedList.h"
#include "DKList.h"


// DKGraphEdge ===========================================================================

struct DKGraphEdge
{
    DKObject _obj;
    
    DKObjectRef firstVertex;
    DKObjectRef secondVertex;
    DKObjectRef userInfo;
};


static void DKGraphEdgeFinalize( DKObjectRef _untyped_self );

static DKObjectRef DKGraphEdgeCopy( DKObjectRef _self );
static DKObjectRef DKGraphEdgeDeepCopy( DKObjectRef _self, int options );


DKThreadSafeClassInit( DKGraphEdgeClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKGraphEdge" ), DKObjectClass(), sizeof(struct DKGraphEdge), 0, NULL, DKGraphEdgeFinalize );
    
    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying) );
    copying->copy = DKGraphEdgeCopy;
    copying->mutableCopy = DKGraphEdgeCopy;
    copying->deepCopy = DKGraphEdgeDeepCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    return cls;
}


///
//  DKGraphEdgeInit()
//
DKObjectRef DKGraphEdgeInit( DKObjectRef _untyped_self, DKObjectRef from, DKObjectRef to )
{
    DKGraphEdgeRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        _self->firstVertex = DKRetain( from );
        _self->secondVertex = DKRetain( to );
    }
    
    return _self;
}


///
//  DKGraphEdgeFinalize()
//
static void DKGraphEdgeFinalize( DKObjectRef _untyped_self )
{
    DKGraphEdgeRef _self = _untyped_self;
    
    DKRelease( _self->firstVertex );
    DKRelease( _self->secondVertex );
    DKRelease( _self->userInfo );
}


///
//  DKGraphEdgeCopy()
//
static DKObjectRef DKGraphEdgeCopy( DKObjectRef _untyped_self )
{
    DKGraphEdgeRef _self = _untyped_self;
    
    return DKGraphEdgeInit( DKAlloc( DKGraphEdgeClass() ), _self->firstVertex, _self->secondVertex );
}


///
//  DKGraphEdgeDeepCopy()
//
static DKObjectRef DKGraphEdgeDeepCopy( DKObjectRef _untyped_self, int options )
{
    DKGraphEdgeRef _self = _untyped_self;
    
    DKObjectRef firstVertexCopy = DKDeepCopy( _self->firstVertex, options );
    DKObjectRef secondVertexCopy = DKDeepCopy( _self->secondVertex, options );
    
    DKGraphEdgeRef copy = DKGraphEdgeInit( DKAlloc( DKGraphEdgeClass() ), firstVertexCopy, secondVertexCopy );
    
    DKRelease( firstVertexCopy );
    DKRelease( secondVertexCopy );
    
    return copy;
}


///
//  DKGraphEdgeGetFirstVertex()
//
DKObjectRef DKGraphEdgeGetFirstVertex( DKGraphEdgeRef _self )
{
    return _self ? _self->firstVertex : NULL;
}


///
//  DKGraphEdgeGetSecondVertex()
//
DKObjectRef DKGraphEdgeGetSecondVertex( DKGraphEdgeRef _self )
{
    return _self ? _self->secondVertex : NULL;
}


///
//  DKGraphEdgeGetUserInfo()
//
DKObjectRef DKGraphEdgeGetUserInfo( DKGraphEdgeRef _self )
{
    return _self ? _self->userInfo : NULL;
}


///
//  DKGraphEdgeSetUserInfo()
//
void DKGraphEdgeSetUserInfo( DKGraphEdgeRef _self, DKObjectRef userInfo )
{
    if( _self )
    {
        DKRetain( userInfo );
        DKRelease( _self->userInfo );
        _self->userInfo = userInfo;
    }
}




// DKGraphNode ===========================================================================

// Internal object for pathfinding

struct DKGraphNode
{
    DKObject _obj;
    
    DKObjectRef object; // Not retained
    double g;
    double h;
};

typedef struct DKGraphNode * DKGraphNodeRef;

DKClassRef DKGraphNodeClass( void );

DKThreadSafeClassInit( DKGraphNodeClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKGraphNode" ), DKObjectClass(), sizeof(struct DKGraphNode), 0, NULL, NULL );
    
    return cls;
}


///
//  DKGraphNodeRef()
//
static DKGraphNodeRef DKNewDirectedGraphNode( DKObjectRef object, double g, double h )
{
    DKGraphNodeRef node = DKNew( DKGraphNodeClass() );
    
    node->object = object;
    node->g = g;
    node->h = h;
    
    return node;
}


///
//  DKGraphNodeCompare()
//
static int DKGraphNodeCompare( DKGraphNodeRef a, DKGraphNodeRef b )
{
    if( a == b )
        return 0;
    
    double fa = a->g + a->h;
    double fb = b->g + b->h;
    
    if( fa < fb )
        return 1;
    
    if( fa > fb )
        return -1;
    
    if( a < b )
        return 1;
    
    return -1;
}




// DKGraph ===============================================================================

struct DKGraph
{
    DKObject _obj;
    
    DKMutableDictionaryRef graph;
};


static DKObjectRef DKGraphInit( DKObjectRef _untyped_self );
static void DKGraphFinalize( DKObjectRef _untyped_self );

static DKObjectRef DKGraphCopy( DKObjectRef _self );
static DKObjectRef DKGraphDeepCopy( DKObjectRef _self, int options );


DKThreadSafeClassInit( DKGraphClass )
{
    DKClassRef cls = DKNewClass( DKSTR( "DKGraph" ), DKObjectClass(), sizeof(struct DKGraph), 0, DKGraphInit, DKGraphFinalize );
    
    // Collection
    struct DKCollectionInterface * collection = DKNewInterface( DKSelector(Collection) );
    collection->getCount = (DKGetCountMethod)DKGraphGetVertexCount;
    collection->containsObject = (DKContainsMethod)DKGraphContainsVertex;
    collection->foreachObject = (DKForeachObjectMethod)DKGraphForeachVertex;
    
    DKInstallInterface( cls, collection );
    DKRelease( collection );

    // Copying
    struct DKCopyingInterface * copying = DKNewInterface( DKSelector(Copying) );
    copying->copy = DKGraphCopy;
    copying->mutableCopy = DKGraphCopy;
    copying->deepCopy = DKGraphDeepCopy;
    
    DKInstallInterface( cls, copying );
    DKRelease( copying );

    return cls;
}


///
//  DKGraphInit()
//
DKObjectRef DKGraphInit( DKObjectRef _untyped_self )
{
    DKGraphRef _self = DKSuperInit( _untyped_self, DKObjectClass() );
    
    if( _self )
    {
        _self->graph = DKNewMutableDictionary();    }
    
    return _self;
}


///
//  DKGraphFinalize()
//
static void DKGraphFinalize( DKObjectRef _untyped_self )
{
    DKGraphRef _self = _untyped_self;
    
    DKRelease( _self->graph );
}


///
//  DKGraphCopy()
//
static int CopyEdges( DKObjectRef vertex, DKObjectRef edges, void * context )
{
    DKGraphRef copy = context;
    
    DKMutableListRef edgesCopy = DKMutableCopy( edges );
    DKDictionarySetObject( copy->graph, vertex, edgesCopy );
    DKRelease( edgesCopy );
    
    return 0;
}

static DKObjectRef DKGraphCopy( DKObjectRef _untyped_self )
{
    DKGraphRef _self = _untyped_self;
    
    DKGraphRef copy = DKNewGraph();
    DKForeachKeyAndObject( _self->graph, CopyEdges, copy );
    
    return copy;
}


///
//  DKGraphDeepCopy()
//
struct DeepCopyContext
{
    DKGraphRef copy;
    int options;
};

static int DeepCopyEdges( DKObjectRef vertex, DKObjectRef edges, void * _context )
{
    struct DeepCopyContext * context = _context;
    
    DKObjectRef vertexCopy = DKDeepCopy( vertex, context->options );
    
    DKMutableListRef edgesCopy = DKNewMutableList();
    DKIndex count = DKListGetCount( edges );
    
    for( DKIndex i = 0; i < count; i++ )
    {
        DKGraphEdgeRef edge = DKListGetObjectAtIndex( edges, i );
        DKGraphEdgeRef edgeCopy = DKGraphEdgeDeepCopy( edge, context->options );
        DKListAppendObject( edgesCopy, edgeCopy );
        DKRelease( edgeCopy );
    }
    
    DKDictionarySetObject( context->copy->graph, vertexCopy, edgesCopy );
    DKRelease( vertexCopy );
    DKRelease( edgesCopy );
    
    return 0;
}

static DKObjectRef DKGraphDeepCopy( DKObjectRef _untyped_self, int options )
{
    DKGraphRef _self = _untyped_self;

    struct DeepCopyContext context;
    context.copy = DKNewGraph();
    context.options = options;

    DKForeachKeyAndObject( _self->graph, DeepCopyEdges, &context );
    
    return context.copy;
}


///
//  DKGraphGetVertexCount()
//
DKIndex DKGraphGetVertexCount( DKGraphRef _self )
{
    return _self ? DKDictionaryGetCount( _self->graph ) : 0;
}


///
//  DKGraphGetEdgeCount()
//
static int CountEdges( DKObjectRef key, DKObjectRef object, void * context )
{
    DKIndex * count = context;
    *count += DKListGetCount( object );
    
    return 0;
}

DKIndex DKGraphGetEdgeCount( DKGraphRef _self )
{
    DKIndex count = 0;
    
    if( _self )
        DKForeachKeyAndObject( _self->graph, CountEdges, &count );
    
    return count;
}


///
//  FindEdge()
//
static DKGraphEdgeRef FindEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to, bool insertIfNotFound )
{
    DKMutableListRef edges = DKDictionaryGetObject( _self->graph, from );
    
    if( edges )
    {
        DKIndex edgeCount = DKListGetCount( edges );
        
        for( DKIndex i = 0; i < edgeCount; i++ )
        {
            DKGraphEdgeRef edge = DKListGetObjectAtIndex( edges, i );
            
            if( DKGraphEdgeGetSecondVertex( edge ) == to )
                return edge;
        }
    }
    
    if( insertIfNotFound )
    {
        if( !edges )
        {
            edges = DKNewMutableList();
            DKDictionarySetObject( _self->graph, from, edges );
            DKRelease( edges );
        }
    
        DKGraphEdgeRef edge = DKNewDirectedGraphEdge( from, to );
        DKListAppendObject( edges, edge );
        DKRelease( edge );
        
        return edge;
    }
    
    return NULL;
}


///
//  RemoveEdge()
//
static void RemoveEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to )
{
    DKMutableListRef edges = DKDictionaryGetObject( _self->graph, from );
    DKIndex edgeCount = DKListGetCount( edges );
    
    for( DKIndex i = 0; i < edgeCount; i++ )
    {
        DKGraphEdgeRef edge = DKListGetObjectAtIndex( edges, i );
        
        if( DKGraphEdgeGetSecondVertex( edge ) == to )
        {
            if( edgeCount == 1 )
                DKDictionaryRemoveObject( _self->graph, from );
            
            else
                DKListRemoveObjectAtIndex( edges, i );
            
            return;
        }
    }
}


///
//  DKGraphAddVertex()
//
void DKGraphAddVertex( DKGraphRef _self, DKObjectRef vertex )
{
    if( _self )
    {
        if( DKDictionaryGetObject( _self->graph, vertex ) == NULL )
        {
            DKMutableListRef edges = DKNewMutableList();
            DKDictionarySetObject( _self->graph, vertex, edges );
            DKRelease( edges );
        }
    }
}


///
//  DKGraphAddEdge()
//
void DKGraphAddEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to, bool bidirectional, DKGraphEdgeRef addedEdges[] )
{
    DKGraphEdgeRef edge;
    
    if( _self )
    {
        edge = FindEdge( _self, from, to, true );
        
        if( addedEdges )
            addedEdges[0] = edge;
        
        if( bidirectional )
        {
            edge = FindEdge( _self, to, from, true );

            if( addedEdges )
                addedEdges[1] = edge;
        }
        
        else
        {
            DKGraphAddVertex( _self, to );
        }
    }
}


///
//  DKGraphRemoveEdge()
//
void DKGraphRemoveEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to, bool bidirectional )
{
    if( _self )
    {
        RemoveEdge( _self, from, to );
        
        if( bidirectional )
            RemoveEdge( _self, to, from );
    }
}


///
//  DKGraphRemoveAllEdges()
//
void DKGraphRemoveAllEdges( DKGraphRef _self )
{
    if( _self )
    {
        DKDictionaryRemoveAllObjects( _self->graph );
    }
}


///
//  DKGraphGetVertices()
//
DKListRef DKGraphGetVertices( DKGraphRef _self )
{
    if( _self )
        return DKDictionaryGetAllKeys( _self->graph );
        
    return NULL;
}


///
//  DKGraphGetEdges()
//
DKListRef DKGraphGetEdges( DKGraphRef _self, DKObjectRef from )
{
    if( _self )
        return DKDictionaryGetObject( _self->graph, from );
    
    return NULL;
}


///
//  DKGraphGetEdge()
//
DKGraphEdgeRef DKGraphGetEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to )
{
    if( _self )
        return FindEdge( _self, from, to, false );
    
    return NULL;
}


///
//  DKGraphContainsVertex()
//
bool DKGraphContainsVertex( DKGraphRef _self, DKObjectRef vertex )
{
    return _self ? DKDictionaryContainsKey( _self->graph, vertex ) : false;
}


///
//  DKGraphContainsEdge()
//
bool DKGraphContainsEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to )
{
    return DKGraphGetEdge( _self, from, to ) != NULL;
}


///
//  DKGraphForeachVertex()
//
int DKGraphForeachVertex( DKGraphRef _self, DKApplierFunction callback, void * context )
{
    if( _self )
        return DKForeachKey( _self->graph, callback, context );
        
    return 0;
}


///
//  DKGraphForeachEdge()
//
struct ForeachEdgeAtVertexContext
{
    DKApplierFunction edgeCallback;
    void * edgeContext;
};

static int ForeachEdgeAtVertex( DKObjectRef vertex, DKObjectRef edgeList, void * _context )
{
    struct ForeachEdgeAtVertexContext * context = _context;
    return DKForeachObject( edgeList, context->edgeCallback, context->edgeContext );
}

int DKGraphForeachEdge( DKGraphRef _self, DKApplierFunction callback, void * context )
{
    if( _self )
    {
        struct ForeachEdgeAtVertexContext vertexContext;
        vertexContext.edgeCallback = callback;
        vertexContext.edgeContext = context;
    
        return DKForeachKeyAndObject( _self->graph, ForeachEdgeAtVertex, &vertexContext );
    }
    
    return 0;
}


///
//  DKGraphTraverse()
//
int DKGraphTraverse( DKGraphRef _self, DKObjectRef from, DKApplierFunction callback, void * context )
{
    DKMutableSetRef visitedSet = DKMutableSet();
    DKMutableListRef queue = DKMutableLinkedList();
    DKListAppendObject( queue, from );
    
    DKObjectRef object;
    
    while( (object = DKListGetFirstObject( queue )) != NULL )
    {
        int result = callback( object, context );
        
        if( result )
            return result;
        
        DKSetAddObject( visitedSet, object );
        DKListRemoveFirstObject( queue );

        DKListRef edges = DKDictionaryGetObject( _self->graph, object );
        DKIndex edgeCount = DKListGetCount( edges );
        
        for( DKIndex i = 0; i < edgeCount; i++ )
        {
            DKGraphEdgeRef edge = DKListGetObjectAtIndex( edges, i );
            DKObjectRef neighbour = DKGraphEdgeGetSecondVertex( edge );
            
            if( !DKSetContainsObject( visitedSet, neighbour ) )
                DKListAppendObject( queue, neighbour );
        }
    }
    
    return 0;
}


///
//  DKGraphGetShortestPath()
//
static DKListRef ReconstructPath( DKDictionaryRef cameFrom, DKObjectRef current )
{
    DKMutableListRef path = DKMutableList();
    DKListAppendObject( path, current );
    
    while( (current = DKDictionaryGetObject( cameFrom, current )) != NULL )
        DKListAppendObject( path, current );
    
    DKListReverse( path );
    
    return path;
}

DKListRef DKGraphGetShortestPath( DKGraphRef _self, DKObjectRef from, DKObjectRef to,
    DKGraphCostFunction distance, DKGraphCostFunction heuristic, void * context )
{
    // Cache the node class
    DKClassRef nodeClass = DKGraphNodeClass();

    // Visited set of nodes
    DKMutableSetRef visitedSet = DKMutableSet();

    // The set of discovered node that haven't been evaluated
    DKMutableBinaryTreeRef discoveredSet = DKBinaryTreeWithCompareFunction( (DKCompareFunction)DKGraphNodeCompare );

    // Add the start node t- the open set
    DKGraphNodeRef start = DKNewDirectedGraphNode( from, 0, heuristic( from, to, context ) );
    DKBinaryTreeAddObjectToSet( discoveredSet, start );
    DKRelease( start );

    // The most efficient route to each visited node
    DKMutableDictionaryRef cameFrom = DKMutableDictionary();

    while( DKBinaryTreeGetCount( discoveredSet ) )
    {
        // Get the first open object
        DKGraphNodeRef current = DKBinaryTreeGetFirstObject( discoveredSet );
        
        if( current->object == to )
            return ReconstructPath( cameFrom, current->object );
        
        DKSetAddObject( visitedSet, current->object );
        DKBinaryTreeRemoveObject( discoveredSet, current );

        DKListRef edges = DKDictionaryGetObject( _self->graph, current->object );
        DKIndex edgeCount = DKListGetCount( edges );
        
        for( DKIndex i = 0; i < edgeCount; i++ )
        {
            DKGraphEdgeRef edge = DKListGetObjectAtIndex( edges, i );
            DKObjectRef neighbour = DKGraphEdgeGetSecondVertex( edge );
            
            // Skip previously visited vertexes
            if( DKSetContainsObject( visitedSet, neighbour ) )
                continue;
            
            // Get the cost from the start node to the neighbour node
            double g = current->g + distance( current->object, neighbour, context );
            
            struct DKGraphNode search = { DKInitStaticObjectHeader( nodeClass ), neighbour, 0, 0 };
            DKGraphNodeRef node = DKBinaryTreeGetObject( discoveredSet, &search );
            
            // If the node isn't in the open set, add it
            if( !node )
            {
                node = DKNewDirectedGraphNode( neighbour, g, heuristic( from, to, context ) );
                
                DKBinaryTreeInsertObject( discoveredSet, node, node, DKInsertAlways );
                DKDictionarySetObject( cameFrom, node->object, current->object );
                
                DKRelease( node );
            }
            
            // If g improves upon the current cost to the node, update it
            else if( g < node->g )
            {
                DKRetain( node );
                DKBinaryTreeRemoveObject( discoveredSet, node );

                node->g = g;

                DKBinaryTreeInsertObject( discoveredSet, node, node, DKInsertAlways );
                DKDictionarySetObject( cameFrom, node->object, current->object );
                
                DKRelease( node );
            }
        }
    }
    
    return NULL;
}


///
//  DKGraphUniformCostFunction()
//
double DKGraphUniformCostFunction( DKObjectRef a, DKObjectRef b, void * context )
{
    return 1.0;
}




