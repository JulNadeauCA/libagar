/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Compute the intersections between different sketch entities. These
 * functions generate 0 or more entities representing the intersections
 * in the sketch.
 */

#include <agar/core/core.h>

#include "sk.h"

/* Create a set of SK entities from a M_GeomSet2. */
static void
InstantiateGeomSet(SK_Group *_Nonnull g, M_GeomSet2 *_Nonnull S)
{
	Uint i;

	for (i = 0; i < S->n; i++) {
		M_Geom2 *G = &S->g[i];

		switch (G->type) {
		case M_POINT:
			SK_Translatev(SK_PointNew(g), &G->g.point);
			break;
		case M_LINE:
			SK_LineFromValue(g, G->g.line);
			break;
		case M_CIRCLE:
			SK_CircleFromValue(g, G->g.circle);
			break;
		case M_POLYGON:
			SK_PolygonFromValue(g, G->g.polygon);
			break;
		default:
			break;
		}
	}

}

static void
IntersectPointPoint(SK_Group *_Nonnull g, void *_Nonnull node1,
    void *_Nonnull node2)
{
	M_Vector3 p1 = SK_Pos(node1);
	M_Vector3 p2 = SK_Pos(node2);

	if (Fabs(p1.x - p2.x) < M_MACHEP &&
	    Fabs(p1.y - p2.y) < M_MACHEP) {
		SK_Translatev(SK_PointNew(g), &p1);
	}
}

static void
IntersectLineLine(SK_Group *_Nonnull g, void *_Nonnull node1,
    void *_Nonnull node2)
{
	M_GeomSet2 Sint = M_GEOM_SET_EMPTY;

	Sint = M_IntersectLineLine2(SK_LineValue(SKLINE(node1)),
	                            SK_LineValue(SKLINE(node2)));
	InstantiateGeomSet(g, &Sint);
	M_GeomSetFree2(&Sint);
}

static void
IntersectCircleLine(SK_Group *_Nonnull g, void *_Nonnull node1,
    void *_Nonnull node2)
{
	M_GeomSet2 Sint;

	Sint = M_IntersectCircleLine2(SK_CircleValue(SKCIRCLE(node1)),
	                              SK_LineValue(SKLINE(node2)));
	InstantiateGeomSet(g, &Sint);
	M_GeomSetFree2(&Sint);
}

static void
IntersectCircleCircle(SK_Group *_Nonnull g, void *_Nonnull node1,
    void *_Nonnull node2)
{
	M_GeomSet2 Sint;

	Sint = M_IntersectCircleCircle2(SK_CircleValue(SKCIRCLE(node1)),
	                                SK_CircleValue(SKCIRCLE(node2)));
	InstantiateGeomSet(g, &Sint);
	M_GeomSetFree2(&Sint);
}

/*
 * Compute the intersections between two nodes and create entities
 * for them in the given group.
 */
void
SK_ComputeIntersections(SK_Group *g, SK_Node *n1, SK_Node *n2)
{
	Uint i;

	for (i = 0; i < skIntersectFnCount; i++) {
		const SK_IntersectFn *fn = &skIntersectFns[i];

		if (SK_NodeOfClass(n1, fn->type1) &&
		    SK_NodeOfClass(n2, fn->type2)) {
			fn->fn(g, n1, n2);
			return;
		}
		if (SK_NodeOfClass(n1, fn->type2) &&
		    SK_NodeOfClass(n2, fn->type1)) {
			fn->fn(g, n2, n1);
			return;
		}
	}
}

const SK_IntersectFn skIntersectFns[] = {
	{ "Point:*",	"Point:*",	IntersectPointPoint },
/*	{ "Point:*",	"Line:*",	IntersectPointLine }, */
	{ "Line:*",	"Line:*",	IntersectLineLine },
	{ "Circle:*",	"Line:*",	IntersectCircleLine },
	{ "Circle:*",	"Circle:*",	IntersectCircleCircle },
};
const Uint skIntersectFnCount = sizeof(skIntersectFns) /
                                sizeof(skIntersectFns[0]);
