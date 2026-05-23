// =======================================================================================
//
// DKGraph.h
// Duck Object Library -- See LICENSE for legal information.
//
// Copyright (c) 2014-2026 Derek W. Nylen
//
// =======================================================================================
#ifndef _DK_GRAPH_H_
#define _DK_GRAPH_H_

#ifdef __cplusplus
extern "C"
{
#endif


// DKGraphEdge ===========================================================================

typedef struct DKGraphEdge * DKGraphEdgeRef;

DK_API DKClassRef  DKGraphEdgeClass( void );

#define DKNewDirectedGraphEdge( from, to )      DKGraphEdgeInit( DKAlloc( DKGraphEdgeClass() ), from, to )

DK_API DKObjectRef DKGraphEdgeInit( DKObjectRef _self, DKObjectRef from, DKObjectRef to );

DK_API DKObjectRef DKGraphEdgeGetFirstVertex( DKGraphEdgeRef _self );
DK_API DKObjectRef DKGraphEdgeGetSecondVertex( DKGraphEdgeRef _self );

DK_API DKObjectRef DKGraphEdgeGetUserInfo( DKGraphEdgeRef _self );
DK_API void DKGraphEdgeSetUserInfo( DKGraphEdgeRef _self, DKObjectRef userInfo );




// DKGraph ===============================================================================

typedef struct DKGraph * DKGraphRef;
typedef double (*DKGraphCostFunction)( DKObjectRef a, DKObjectRef b, void * context );

DK_API DKClassRef  DKGraphClass( void );

#define DKGraph()       DKAutorelease( DKNew( DKGraphClass() ) )
#define DKNewGraph()    DKNew( DKGraphClass() )

DK_API DKIndex DKGraphGetVertexCount( DKGraphRef _self );
DK_API DKIndex DKGraphGetEdgeCount( DKGraphRef _self );

DK_API void DKGraphAddVertex( DKGraphRef _self, DKObjectRef vertex );
DK_API void DKGraphAddEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to, bool bidirectional, DKGraphEdgeRef addedEdges[] );
DK_API void DKGraphRemoveEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to, bool bidirectional );
DK_API void DKGraphRemoveAllEdges( DKGraphRef _self );

DK_API DKListRef DKGraphGetVertices( DKGraphRef _self );
DK_API DKListRef DKGraphGetEdges( DKGraphRef _self, DKObjectRef from );
DK_API DKGraphEdgeRef DKGraphGetEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to );

DK_API bool DKGraphContainsVertex( DKGraphRef _self, DKObjectRef vertex );
DK_API bool DKGraphContainsEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to );

DK_API int DKGraphForeachVertex( DKGraphRef _self, DKApplierFunction callback, void * context );
DK_API int DKGraphForeachEdge( DKGraphRef _self, DKApplierFunction callback, void * context );

DK_API int DKGraphTraverse( DKGraphRef _self, DKObjectRef from, DKApplierFunction callback, void * context );

DK_API DKListRef DKGraphGetShortestPath( DKGraphRef _self, DKObjectRef from, DKObjectRef to,
    DKGraphCostFunction distance, DKGraphCostFunction heuristic, void * context );

DK_API double DKGraphUniformCostFunction( DKObjectRef a, DKObjectRef b, void * context );


#ifdef __cplusplus
}
#endif

#endif // _DK_GRAPH_H_

