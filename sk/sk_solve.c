/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * This is a generic (graph-based) geometric constraint solver.
 * 
 * Phase 1 - Recursive graph analysis:
 *   This phase determines whether the problem is well constrained, and if so,
 *   the sequence of construction steps to follow.
 *   o We first look at the constraints between the nodes and generate a set
 *     of graphs representing rigid clusters.
 *   o If a ring of three clusters share exactly one node with each other,
 *     the clusters can be brought into alignment and merged into a larger
 *     cluster. If any other cluster is sharing two elements with the newly
 *     generated cluster, they are also merged.
 *   o We continue searching for rings of 3 constrained clusters and merging
 *     them. If we end up with a single, rigid cluster, the problem is
 *     well-constrained.
 * 
 * Phase 2 - Construction phase:
 *   We simply follow the order of cluster formation to place elements
 *   in the same order. The position of an unknown element with respect
 *   to two other constrained elements can be computed using systems of
 *   equations (linear, linear-quadratic and quadratic). Where multiple
 *   solutions are possible, we optimize for minimum displacement from
 *   the original point.
 */

#include <agar/core/core.h>

#include "sk.h"

/*
 * Look for nodes in clOrig that share exactly two edges with cluster cl
 * and merge them into it (deleting them from clOrig). Since our elements
 * have two degrees of freedom, any element connected to a rigid cluster
 * by two constraints can be merged in that cluster.
 */
static void
MergeConstrainedRings(SK *_Nonnull sk, SK_Cluster *_Nonnull clOrig,
    SK_Cluster *_Nonnull cl)
{
	SK_Constraint *ct, *ctPair[2];
	SK_Node *nUnknown, *nKnown[2];
	Uint i, count;
	
	Debug(sk, "Solver: MergeConstrainedRings(Cluster%u)\n", (Uint)cl->name);
restart:
	TAILQ_FOREACH(nUnknown, &sk->nodes, nodes) {
		if (nUnknown->flags & (SK_NODE_SUPCONSTRAINTS|SK_NODE_FIXED)) {
			continue;
		}
		count = 0;
		TAILQ_FOREACH(ct, &clOrig->edges, constraints) {
			if (ct->n1 == nUnknown &&
			    SK_NodeInCluster(ct->n2, cl)) {
				if (count < 2) {
					ctPair[count] = ct;
					nKnown[count] = ct->n2;
				}
				count++;
			} else if (ct->n2 == nUnknown &&
			     SK_NodeInCluster(ct->n1, cl)) {
				if (count < 2) {
					ctPair[count] = ct;
					nKnown[count] = ct->n1;
				}
				count++;
			}
		}
		if (count != 2) {
			continue;
		}
		SK_AddInsn(sk, SK_COMPOSE_RING,
		    nUnknown, nKnown[0], nKnown[1],
		    SK_DupConstraint(ctPair[0]),
		    SK_DupConstraint(ctPair[1]));
		for (i = 0; i < 2; i++) {
			SK_AddConstraintCopy(cl, ctPair[i]);
			SK_DelConstraint(clOrig, ctPair[i]);
		}

		/* XXX is this needed? */
		goto restart;
	}
}

/*
 * Look for any sketch cluster that shares two elements with the given
 * cluster, and merge them into a single cluster.
 *
 * cl must not be already attached to sk.
 */
static void
MergeConstrainedClusters(SK *_Nonnull sk, SK_Cluster *_Nonnull clMerged)
{
	SK_Cluster *cl;
	SK_Constraint *ct;
	Uint count;
	
	Debug(sk, "Solver: MergeConstrainedClusters(Cluster%u)\n",
	    (Uint)clMerged->name);
restart:
	TAILQ_FOREACH(cl, &sk->clusters, clusters) {
		count = 0;
		TAILQ_FOREACH(ct, &cl->edges, constraints) {
			if (SK_NodeInCluster(ct->n1, clMerged) ||
			    SK_NodeInCluster(ct->n2, clMerged))
				count++;
		}
		if (count == 2) {
			Debug(sk,
			    "Solver: Merging cluster%d into cluster%d (pair)\n",
			    cl->name, clMerged->name);
			SK_CopyCluster(cl, clMerged);
			TAILQ_REMOVE(&sk->clusters, cl, clusters);
			SK_FreeCluster(cl);
			Free(cl);
			goto restart;		/* Cluster chain changed */
		}
	}
}

/*
 * Check if the sketch is well-constrained. If DOF analysis ended with a
 * single cluster, and all nodes within the cluster have two constraint
 * edges, the sketch is well-constrained.
 */
static void
UpdateConstraintStatus(SK *_Nonnull sk)
{
	Uint count = 0;
	SK_Cluster *cl;
	SK_Constraint *ct;
	SK_Node *node;

	/* Count the constraint edges per node in the original graph. */
	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		node->nEdges = 0;
		node->flags &= ~(SK_NODE_CHECKED);
	}
	TAILQ_FOREACH(cl, &sk->clusters, clusters) {
		count++;
	}
	TAILQ_FOREACH(ct, &sk->ctGraph.edges, constraints) {
		ct->n1->nEdges++;
		ct->n2->nEdges++;
	}

	/* Check the constrainedness status of all nodes. */
	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (node->flags & (SK_NODE_SUPCONSTRAINTS|SK_NODE_FIXED|
		                   SK_NODE_CHECKED)) {
			continue;
		}
		if (node->ops->constrained != NULL) {
			switch (node->ops->constrained(node)) {
			case SK_UNDER_CONSTRAINED:
				goto under;
#if 0
			case SK_OVER_CONSTRAINED:
				goto over;
#endif
			default:
				break;
			}
		}
		SKNODE(node)->flags |= SK_NODE_CHECKED;
	}
	
	/*
	 * The sketch is under-constrained if the cluster merging
	 * phase fails to produce a single cluster.
	 */
	if (count > 1) {
		SK_SetStatus(sk, SK_UNDER_CONSTRAINED,
		    _("Underconstrained (%u clusters)"), count);
		return;
	}
	
	SK_SetStatus(sk, SK_WELL_CONSTRAINED, _("Well-constrained"));
	return;
#if 0
over:
	SK_SetStatus(sk, SK_OVER_CONSTRAINED,
	    _("Overconstrained (%s)"), node->name);
	return;
#endif
under:
	SK_SetStatus(sk, SK_UNDER_CONSTRAINED,
	    _("Underconstrained (%s)"), node->name);
}

/*
 * Analyze the constraint graph, determine its constrainedness and
 * generate a sketch placement program.
 */
int
SK_Solve(SK *sk)
{
	SK_Cluster clOrig;
	SK_Cluster *cl, *clRing[3], *clPair[2];
	SK_Constraint *ct;
	SK_Node *node;
	Uint i, j, count, nRing;

	AG_MutexLock(&sk->lock);

	if (TAILQ_EMPTY(&sk->ctGraph.edges)) 		/* Nothing to do */
		goto out;

	/*
	 * First Phase: Degree of freedom analysis.
	 */

	/* Copy the source graph since we will modify its elements. */
	SK_FreeClusters(sk);
	SK_FreeInsns(sk);
	SK_InitCluster(&clOrig, 1);
	SK_CopyCluster(&sk->ctGraph, &clOrig);

	while (!TAILQ_EMPTY(&clOrig.edges)) {
		cl = Malloc(sizeof(SK_Cluster));
		SK_InitCluster(cl, SK_GenClusterName(sk));

		/* Start with any edge and find n2 from n1. */
		ct = TAILQ_FIRST(&clOrig.edges);
		Debug(sk, "Solver: Starting DOF analysis with %s-%s\n",
		    ct->n1->name, ct->n2->name);
		SK_AddConstraintCopy(cl, ct);
		SK_AddInsn(sk, SK_COMPOSE_PAIR, ct->n1, ct->n2,
		    SK_DupConstraint(ct));
		SK_DelConstraint(&clOrig, ct);
	
		/* Keep merging constrained rings into this cluster. */
		MergeConstrainedRings(sk, &clOrig, cl);
		TAILQ_INSERT_TAIL(&sk->clusters, cl, clusters);
	}

	/*
	 * Search for constrained rings of 3 clusters and merge them into
	 * larger clusters.
	 */
merge_rings:
	nRing = 0;
	TAILQ_FOREACH(node, &sk->nodes, nodes) {
		if (node->flags & SK_NODE_SUPCONSTRAINTS) {
			continue;
		}
		count = 0;
		TAILQ_FOREACH(cl, &sk->clusters, clusters) {
			if (SK_NodeInCluster(node, cl)) {
				if (count < 2) {
					clPair[count] = cl;
				}
				count++;
			}
		}
		if (count == 2) {
			Debug(sk,
			    "Solver: %s is shared by Cluster%u and Cluster%u\n",
			    node->name, (Uint)clPair[0]->name, (Uint)clPair[1]->name);
			for (j = 0; j < 2; j++) {
				for (i = 0; i < nRing; i++) {
					if (clRing[i] == clPair[j])
						break;
				}
				if (i == nRing && nRing < 3) {
					clRing[nRing++] = clPair[j];
				}
			}
			if (nRing == 3)
				break;
		}
	}
	if (nRing == 3) {
		SK_Cluster *clMerged;

		clMerged = Malloc(sizeof(SK_Cluster));
		SK_InitCluster(clMerged, SK_GenClusterName(sk));
		Debug(sk,
		    "Solver: Merging ring: Cluster%u-Cluster%u-Cluster%u -> "
		    "Cluster%u\n",
		    clRing[0]->name, clRing[1]->name, clRing[2]->name,
		    clMerged->name);
		for (i = 0; i < 3; i++) {
			SK_CopyCluster(clRing[i], clMerged);
			TAILQ_REMOVE(&sk->clusters, clRing[i], clusters);
			SK_FreeCluster(clRing[i]);
			Free(clRing[i]);
		}

		/*
		 * Merge any other cluster sharing two elements with
		 * the new cluster.
		 */
		MergeConstrainedClusters(sk, clMerged);

		TAILQ_INSERT_TAIL(&sk->clusters, clMerged, clusters);
		goto merge_rings;
	}
	UpdateConstraintStatus(sk);
out:
	AG_MutexUnlock(&sk->lock);
	return (0);
}
