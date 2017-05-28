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

#ifndef _DK_DIRECTED_GRAPH_H_
#define _DK_DIRECTED_GRAPH_H_

#include "DKRuntime.h"



// DKGraphEdge ===========================================================================

typedef struct DKGraphEdge * DKGraphEdgeRef;

DKClassRef  DKGraphEdgeClass( void );

#define DKNewDirectedGraphEdge( from, to )      DKGraphEdgeInit( DKAlloc( DKGraphEdgeClass() ), from, to )

DKObjectRef DKGraphEdgeInit( DKObjectRef _self, DKObjectRef from, DKObjectRef to );

DKObjectRef DKGraphEdgeGetFirstVertex( DKGraphEdgeRef _self );
DKObjectRef DKGraphEdgeGetSecondVertex( DKGraphEdgeRef _self );

DKObjectRef DKGraphEdgeGetUserInfo( DKGraphEdgeRef _self );
void DKGraphEdgeSetUserInfo( DKGraphEdgeRef _self, DKObjectRef userInfo );




// DKGraph ===============================================================================

typedef struct DKGraph * DKGraphRef;
typedef double (*DKGraphCostFunction)( DKObjectRef a, DKObjectRef b, void * context );

DKClassRef  DKGraphClass( void );

#define DKGraph()       DKAutorelease( DKNew( DKGraphClass() ) )
#define DKNewGraph()    DKNew( DKGraphClass() )

void DKGraphAddEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to, bool bidirectional );
void DKGraphRemoveEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to, bool bidirectional );
void DKGraphRemoveAllEdges( DKGraphRef _self );

DKListRef DKGraphGetEdges( DKGraphRef _self, DKObjectRef from );
DKGraphEdgeRef DKGraphGetEdge( DKGraphRef _self, DKObjectRef from, DKObjectRef to );

int DKGraphTraverse( DKGraphRef _self, DKObjectRef from, DKApplierFunction callback, void * context );

DKListRef DKGraphGetShortestPath( DKGraphRef _self, DKObjectRef from, DKObjectRef to,
    DKGraphCostFunction distance, DKGraphCostFunction heuristic, void * context );

double DKGraphUniformCostFunction( DKObjectRef a, DKObjectRef b, void * context );


#endif // _DK_DIRECTED_GRAPH_H_

