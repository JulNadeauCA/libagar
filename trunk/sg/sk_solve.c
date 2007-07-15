/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This is a generic, graph-based geometric constraint solver. It is based on
 * the idea that, since our geometric elements (points, lines and circles of
 * fixed radii) have at most two degrees of freedom, the unknown position of
 * a third element in a system of three constrained elements can be computed
 * using linear, linear-quadratic and quadratic systems of equations based
 * on the constraint between the two other elements.
 * 
 * 1) Graph analysis. This phase determines whether the problem is well
 * constrained, and if so, the sequence of construction steps to use in
 * order to place the elements. We first look at the constraints between
 * the nodes and generate a set of graphs representing rigid clusters.
 * If a set of three rigid clusters share exactly one node with each other,
 * the clusters can be brought into alignment, forming a new, larger rigid
 * cluster. The solution we are looking for is a single, rigid cluster.
 * 
 * 2) Construction phase. We solve the equations to place the components
 * in the same order we followed during graph analysis.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sk.h"

/*
 * Attempt to position the elements of a sketch such that the set
 * of geometrical constraints are satisfied.
 */
int
SK_Solve(SK *sk)
{
	SK_ConstraintGraph cgOrig;
	SK_ConstraintGraph *cgCluster;
	SK_Constraint *ct, *ctCluster, *ctPair[2];
	SK_Node *node;
	int nclusters = 0;
	Uint i, count;

	AG_MutexLock(&sk->lock);

	if (TAILQ_EMPTY(&sk->ctGraph.edges)) 		/* Nothing to do */
		goto out;

	/*
	 * First Phase: Graph analysis.
	 */

	/* Copy the source graph since we will modify its elements. */
	SK_FreeConstraintClusters(sk);
	SK_InitConstraintGraph(&cgOrig);
	SK_CopyConstraintGraph(&sk->ctGraph, &cgOrig);

	/*
	 * Classify all edges in a series of rigid clusters. We start a cluster
	 * with any edge, and we loop adding to the cluster any node which has
	 * exactly two edges to any of the nodes already in the cluster (our
	 * elements have at most two degrees of freedom).
	 */
	while (!TAILQ_EMPTY(&cgOrig.edges)) {
		printf("creating new cluster %d\n", nclusters);
		cgCluster = Malloc(sizeof(SK_ConstraintGraph), M_SG);
		SK_InitConstraintGraph(cgCluster);
		ct = TAILQ_FIRST(&cgOrig.edges);
		SK_AddConstraintCopy(cgCluster, ct);
		SK_DelConstraint(&cgOrig, ct);
populate_cluster:
		printf("cluster%d: populating cluster\n", nclusters);
		TAILQ_FOREACH(node, &sk->nodes, nodes) {
			count = 0;
			TAILQ_FOREACH(ct, &cgOrig.edges, constraints) {
				if ((ct->n1 == node &&
				     SK_NodeInGraph(ct->n2, cgCluster)) ||
				    (ct->n2 == node &&
				     SK_NodeInGraph(ct->n1, cgCluster))) {
					if (count < 2) {
						ctPair[count] = ct;
					}
					count++;
				}
			}
			if (count != 2) {
				continue;
			}
			for (i = 0; i < 2; i++) {
				SK_AddConstraintCopy(cgCluster, ctPair[i]);
				SK_DelConstraint(&cgOrig, ctPair[i]);
			}
			goto populate_cluster;
		}
		TAILQ_INSERT_TAIL(&sk->ctClusters, cgCluster, clusters);
		nclusters++;
	}
out:
	AG_MutexUnlock(&sk->lock);
	return (0);
}

#endif /* HAVE_OPENGL */
