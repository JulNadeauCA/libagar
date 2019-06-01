/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Perform various geometric operations on elements of the sketch. Most of
 * these routines use functions provided by the math library.
 */

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

static void
ConnectPointsWithLines(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes)
{
	SK_Line *Ln;
	int i;

	if (n < 2) {
		AG_TextMsg(AG_MSG_ERROR, "Select at least two Points");
		return;
	}
	for (i = 1; i < n; i++) {
		Ln = SK_LineNew(sk->root);
		Ln->p1 = (SK_Point *)nodes[i-1];
		Ln->p2 = (SK_Point *)nodes[i];
	}
}

static void
TranslatePointsPerVector(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes)
{
	M_Vector3 v;
	int i;
	
	for (i = 0; i < n; i++) {
		if (SK_NodeOfClass(nodes[i], "Line:*")) {
			SK_Line *L = SKLINE(nodes[i]);

			v = M_VecSub3(SK_Pos(L->p1), SK_Pos(L->p2));
			break;
		}
	}
	if (i == n) {
		AG_TextMsg(AG_MSG_ERROR, "Select at least one Line");
		return;
	}
	for (i = 0; i < n; i++) {
		if (!SK_NodeOfClass(nodes[i], "Point:*")) {
			continue;
		}
		SK_Translatev(nodes[i], &v);
	}
}

static void
SortPoints2(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes,
    enum m_point_set_sort_mode2 mode)
{
	M_PointSet2 S = M_POINT_SET_EMPTY;
	SK_Node *node;
	SK_Point *pt;
	int i;

	for (i = 0; i < n; i++) {
		if (M_PointSetAdd2(&S, SK_Pos2(nodes[i])) == -1)
			goto out;
	}
	M_PointSetSort2(&S, mode);
	node = SK_NodeNew(sk->root);
	SK_NodeSetName(node, _("Sorted Points"));
	for (i = 0; i < n; i++) {
		pt = SK_PointNew(node);
		SK_Translatev(pt, &S.p[i]);
	}
out:
	M_PointSetFree2(&S);
}

static void
SortPointsByX(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes)
{
	SortPoints2(sk, n, nodes, M_POINT_SET_SORT_XY);
}

static void
SortPointsByY(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes)
{
	SortPoints2(sk, n, nodes, M_POINT_SET_SORT_YX);
}

static void
CreateParallelLines(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes)
{
	int i;
	
	for (i = 0; i < n; i++) {
		M_Line2 L = SK_LineValue(SKLINE(nodes[i]));
		SK_Line *Ln;
		M_Vector2 v1, v2;

		M_LineToPts2(M_LineParallel2(L, +1.0), &v1, &v2);
		Ln = SK_LineNew(sk->root);
		Ln->p1 = SK_PointNew(sk->root);
		Ln->p2 = SK_PointNew(sk->root);
		SK_TranslateVec(Ln->p1, v1);
		SK_TranslateVec(Ln->p2, v2);
		
		M_LineToPts2(M_LineParallel2(L, -1.0), &v1, &v2);
		Ln = SK_LineNew(sk->root);
		Ln->p1 = SK_PointNew(sk->root);
		Ln->p2 = SK_PointNew(sk->root);
		SK_TranslateVec(Ln->p1, v1);
		SK_TranslateVec(Ln->p2, v2);
	}
}

static void
DividePointsByLine(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes)
{
	SK_Node *nLeft = NULL, *nRight = NULL, *nOn = NULL; /* compiler happy */
	M_Line2 L;
	M_Real rv;
	int i;
	
	for (i = 0; i < n; i++) {
		if (SK_NodeOfClass(nodes[i], "Line:*")) {
			SK_Line *Ln = SKLINE(nodes[i]);

			L = SK_LineValue(SKLINE(nodes[i]));
			nLeft = SK_NodeNew(sk->root);
			nRight = SK_NodeNew(sk->root);
			nOn = SK_NodeNew(sk->root);
			SK_NodeSetName(nLeft, "Left of %s", SKNODE(Ln)->name);
			SK_NodeSetName(nRight, "Right of %s", SKNODE(Ln)->name);
			SK_NodeSetName(nOn, "On %s", SKNODE(Ln)->name);
			break;
		}
	}
	if (i == n) {
		AG_TextMsg(AG_MSG_ERROR, "Select at least one Line");
		return;
	}
	for (i = 0; i < n; i++) {
		if (!SK_NodeOfClass(nodes[i], "Point:*")) {
			continue;
		}
		rv = M_LinePointSide2(L, SK_Pos2(nodes[i]));
		if (M_MACHZERO(rv)) {
			SK_NodeMoveToParent(nodes[i], nOn);
		} else if (rv < -M_MACHEP) {
			SK_NodeMoveToParent(nodes[i], nLeft);
		} else if (rv > +M_MACHEP) {
			SK_NodeMoveToParent(nodes[i], nRight);
		}
	}
}

static void
ComputeLinePointDistance(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes)
{
	SK_Line *Ln = NULL;
	SK_Point *Pn = NULL;
	M_Real dist;
	int i;
	
	for (i = 0; i < n; i++) {
		if (SK_NodeOfClass(nodes[i], "Point:*")) {
			Pn = SKPOINT(nodes[i]);
		} else if (SK_NodeOfClass(nodes[i], "Line:*")) {
			Ln = SKLINE(nodes[i]);
		}
	}
	if (Pn == NULL || Ln == NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("Select a Line and a Point"));
		return;
	}
	dist = M_LinePointDistance2(SK_LineValue(Ln), SK_Pos2(Pn));
	AG_TextMsg(AG_MSG_INFO, "%s-%s distance = %.04f",
	    SKNODE(Ln)->name, SKNODE(Pn)->name, dist);
	SK_SetStatus(sk, sk->status, "||%s - %s|| = %.04f",
	    SKNODE(Ln)->name, SKNODE(Pn)->name, dist);
}

static void
ComputeLineLineAngle(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes)
{
	SK_Line *Ln[2];
	int i, nLines = 0;
	M_Real theta;
	
	for (i = 0; i < n; i++) {
		if (SK_NodeOfClass(nodes[i], "Line:*") &&
		    nLines < 2) {
			Ln[nLines++] = SKLINE(nodes[i]);
		}
	}
	if (nLines != 2) {
		AG_TextMsg(AG_MSG_ERROR, _("Select exactly two Lines"));
		return;
	}
	theta = Degrees(M_LineLineAngle2(SK_LineValue(Ln[0]),
	                                 SK_LineValue(Ln[1])));

	AG_TextMsg(AG_MSG_INFO, "%s-%s angle = %.04f\xc2\xb0",
	    SKNODE(Ln[0])->name, SKNODE(Ln[1])->name, theta);
	SK_SetStatus(sk, sk->status, "Acos(%s \xc2\xb7 %s) = %.04f\xc2\xb0",
	    SKNODE(Ln[0])->name, SKNODE(Ln[1])->name, theta);
}

static void
TestPolygonConvexity(SK *_Nonnull sk, int n, SK_Node *_Nonnull *_Nonnull nodes)
{
	int i;

	for (i = 0; i < n; i++) {
		M_Polygon P = SK_PolygonValue(SKPOLYGON(nodes[i]));
		int rv;

		rv = M_PolygonIsConvex(&P);
		if (rv == 1) {
			AG_TextMsg(AG_MSG_INFO, _("%s is convex"),
			    nodes[i]->name);
		} else if (rv == 0) {
			AG_TextMsg(AG_MSG_INFO, _("%s is concave"),
			    nodes[i]->name);
		} else {
			AG_TextMsg(AG_MSG_INFO, _("%s is invalid"),
			    nodes[i]->name);
		}
	}
}

static void
GeometryOp(AG_Event *_Nonnull event)
{
	SK_View *skv = AG_PTR(1);
	const SK_GeometryFn *fn = AG_PTR(2);
	SK_Node **nodes, *node;
	int nNodes = 0;
	char *mask, *s;

	mask = Strdup(fn->mask);
	nodes = Malloc(sizeof(SK_Node **));
	while ((s = AG_Strsep(&mask, ",")) != NULL) {
		SK_FOREACH_NODE(node, skv->sk, sk_node) {
			if (SKNODE_SELECTED(node) && SK_NodeOfClass(node, s)) {
				nodes = Realloc(nodes, (nNodes+1) *
				                       sizeof(SK_Node **));
				nodes[nNodes++] = node;
			}
		}
	}
	if (nNodes > 0) {
		fn->fn(skv->sk, nNodes, nodes);
	} else {
		AG_TextMsg(AG_MSG_ERROR, "Must select: %s", fn->mask);
	}
	Free(mask);
	Free(s);
}

void
SK_GeometryMenu(SK_View *skv, void *pMenu)
{
	AG_MenuItem *m = pMenu;
	Uint i;

	for (i = 0; i < skGeometryFnCount; i++) {
		const SK_GeometryFn *fn = &skGeometryFns[i];
		AG_MenuAction(m, fn->name, NULL, GeometryOp, "%p,%p", skv, fn);
	}
}

const SK_GeometryFn skGeometryFns[] = {
	{
		"Connect Points with Lines",
		"Point:*",
		ConnectPointsWithLines
	},
	{
		"Translate Points per vector",
		"Point:*,Line:*",
		TranslatePointsPerVector
	},
	{
		"Sort Points by X-coordinate",
		"Point:*",
		SortPointsByX
	},
	{
		"Sort Points by Y-coordinate",
		"Point:*",
		SortPointsByY
	},
	{
		"Divide Points with Line",
		"Point:*,Line:*",
		DividePointsByLine
	},
	{
		"Create parallel Lines",
		"Line:*",
		CreateParallelLines
	},
	{
		"Compute Line-Point distance",
		"Point:*,Line:*",
		ComputeLinePointDistance
	},
	{
		"Compute Line-Line angle",
		"Line:*,Line:*",
		ComputeLineLineAngle
	},
	{
		"Test Polygon convexity",
		"Polygon:*",
		TestPolygonConvexity
	},
};
const Uint skGeometryFnCount = sizeof(skGeometryFns)/sizeof(skGeometryFns[0]);
