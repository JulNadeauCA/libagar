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

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sk.h"

/*
 * Position the elements of a sketch such that the set of geometrical
 * constraints are satisfied.
 */
int
SK_Solve(SK *sk)
{
	SK_ConstraintGraph cg;
	SK_ConstraintGraph *cluster;
	SK_Constraint *ct, *ctCluster;
	int nclusters = 0;

	if (TAILQ_EMPTY(&sk->ctGraph.edges))		/* Nothing to do */
		return (0);

	/*
	 * First Phase: Analysis of the constraint graph.
	 *
	 * We determine whether the sketch is well-constrainted, and if that's
	 * the case, we also determine the sequence of steps in placing the
	 * geometric elements correctly.
	 */

	/* Copy the user-specified constraint graph since we will modify it. */
	SK_FreeConstraintClusters(sk);
	SK_InitConstraintGraph(&cg);
	SK_CopyConstraintGraph(&sk->ctGraph, &cg);

	/*
	 * Create an initial cluster from any two nodes which are connected
	 * by a constraint edge and add any other node which is connected to
	 * any node in the new cluster by exactly two constraint edges
	 * (our geometric elements have at most two degrees of freedom).
	 */
create_cluster:
	if (TAILQ_EMPTY(&cg.edges)) {
		goto out;
	}
	printf("creating new cluster %d\n", nclusters);
	cluster = Malloc(sizeof(SK_ConstraintGraph), M_SG);
	SK_InitConstraintGraph(cluster);
	SK_AddConstraintCopy(cluster, TAILQ_FIRST(&cg.edges));
populate_cluster:
	printf("cluster%d: populating cluster\n", nclusters);
	TAILQ_FOREACH(ct, &cg.edges, constraints) {
		TAILQ_FOREACH(ctCluster, &cluster->edges, constraints) {
			if (ctCluster->n1 == ct->n1 ||
			    ctCluster->n2 == ct->n1 ||
			    ctCluster->n1 == ct->n2 ||
			    ctCluster->n2 == ct->n2) {
				break;
			}
		}
		if (ctCluster == NULL) {
			continue;
		}
		printf("cluster%d: adding edge: %s(%s-%s)\n", nclusters,
		    skConstraintNames[ctCluster->type],
		    SK_NodeName(ctCluster->n1),
		    SK_NodeName(ctCluster->n2));
		SDL_Delay(1000);
		SK_AddConstraintCopy(cluster, ctCluster);
		TAILQ_REMOVE(&cg.edges, ct, constraints);
		Free(ct, M_SG);
		goto populate_cluster;
	}
	/* Continue creating clusters until all edges have been added. */
	TAILQ_INSERT_TAIL(&sk->ctClusters, cluster, clusters);
	nclusters++;
	goto create_cluster;
out:
	printf("done\n");
	return (0);
}

#endif /* HAVE_OPENGL */
