/*****************************************************************************************

  DKGraph.h

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

DK_API void DKGraphAddEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to, bool bidirectional );
DK_API void DKGraphRemoveEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to, bool bidirectional );
DK_API void DKGraphRemoveAllEdges( DKGraphRef _self );

DK_API DKListRef DKGraphGetEdges( DKGraphRef _self, DKObjectRef from );
DK_API DKGraphEdgeRef DKGraphGetEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to );

DK_API int DKGraphTraverse( DKGraphRef _self, DKObjectRef from, DKApplierFunction callback, void * context );

DK_API DKListRef DKGraphGetShortestPath( DKGraphRef _self, DKObjectRef from, DKObjectRef to,
    DKGraphCostFunction distance, DKGraphCostFunction heuristic, void * context );

DK_API double DKGraphUniformCostFunction( DKObjectRef a, DKObjectRef b, void * context );


#ifdef __cplusplus
}
#endif

#endif // _DK_GRAPH_H_

