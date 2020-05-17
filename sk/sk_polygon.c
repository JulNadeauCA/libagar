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
 * Base class for any plane figure bounded by a closed path composed of line
 * segments.
 */

#include <agar/core/core.h>

#include "sk.h"
#include "sk_gui.h"

SK_Polygon *
SK_PolygonNew(void *pnode)
{
	SK_Polygon *Pn;

	Pn = Malloc(sizeof(SK_Polygon));
	SK_PolygonInit(Pn, SK_GenNodeName(SKNODE(pnode)->sk, "Polygon"));
	SK_NodeAttach(pnode, Pn);
	return (Pn);
}

SK_Polygon *
SK_PolygonFromValue(void *pnode, M_Polygon P)
{
	SK_Polygon *Pn;
	Uint i;

	Pn = SK_PolygonNew(pnode);
	if (P.n >= 3) {
		for (i = 1; i < P.n; i++) {
			SK_PolygonAddSide(Pn,
			    SK_LineFromValue(pnode,
			    M_LineFromPts2(P.v[i-1], P.v[i])));
		}
		SK_PolygonAddSide(Pn,
		    SK_LineFromValue(pnode, M_LineFromPts2(P.v[P.n-1], P.v[0])));
	}
	return (Pn);
}

void
SK_PolygonInit(void *p, Uint name)
{
	SK_Polygon *poly = p;

	SK_NodeInit(poly, &skPolygonOps, name, 0);
	poly->color = M_ColorRGB(0.0, 0.0, 1.0);
	poly->s = Malloc(sizeof(SK_Line **));
	poly->flags = 0;
	poly->n = 0;
}

int
SK_PolygonLoad(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Polygon *poly = p;
	Uint i;

	poly->color = M_ReadColor(buf);
	poly->n = (Uint)AG_ReadUint32(buf);
	if (poly->n < 1) {
		AG_SetError("Polygon has 0 sides");
		return (-1);
	}
	poly->s = Malloc(poly->n * sizeof(SK_Line **));
	for (i = 0; i < poly->n; i++) {
		poly->s[i] = SK_ReadRef(buf, sk, "Line");
		SK_NodeAddReference(poly, poly->s[i]);
	}
	if (poly->s[poly->n-1]->p1 != poly->s[0]->p1 &&
	    poly->s[poly->n-1]->p1 != poly->s[0]->p2 &&
	    poly->s[poly->n-1]->p2 != poly->s[0]->p1 &&
	    poly->s[poly->n-1]->p2 != poly->s[0]->p2) {
		AG_SetError("Polygon circuit is not closed");
		return (-1);
	}
	return (0);
}

int
SK_PolygonSave(SK *sk, void *p, AG_DataSource *buf)
{
	SK_Polygon *poly = p;
	Uint i;

	M_WriteColor(buf, &poly->color);
	AG_WriteUint32(buf, (Uint32)poly->n);
	for (i = 0; i < poly->n; i++) {
		SK_WriteRef(buf, poly->s[i]);
	}
	return (0);
}

void
SK_PolygonDraw(void *p, SK_View *skv)
{
	SK_Polygon *poly = p;
	M_Color color = SK_NodeColor(poly, &poly->color);
	int cullFace;
	M_Vector3 v;
	Uint i;

	if (poly->n < 3)
		return;

	/* Winding is unknown and expensive to compute so we cheat. */
	GL_DisableSave(GL_CULL_FACE, &cullFace);
	GL_Begin(GL_POLYGON);
	GL_Color3v(&color);
	for (i = 0; i <= poly->n; i++) {
		v = SK_Pos(poly->s[i % poly->n]->p1);
		GL_Vertex3v(&v);
	}
	GL_End();
	GL_EnableSaved(GL_CULL_FACE, cullFace);
}

M_Real
SK_PolygonProximity(void *p, const M_Vector3 *v, M_Vector3 *vC)
{
	SK_Polygon *poly = p;
	M_Polygon P;
	int rv;

	P = SK_PolygonValue(poly);
	rv = M_PointInPolygon(&P, M_VECTOR2(v->x, v->y));
	M_PolygonFree(&P);

	if (rv) {
		*vC = *v;
		return (0.0);
	} else {
		
	}
	return (M_INFINITY);
}

int
SK_PolygonDelete(void *p)
{
	SK_Polygon *poly = p;
	SK *sk = SKNODE(poly)->sk;
	SK_Point **pts = NULL;
	Uint i, j, nPts = 0;
	int rv;

	for (i = 0; i < poly->n; i++) {
		SK_Line *s = poly->s[i];

		SK_NodeDelReference(poly, s);
		if (SKNODE(s)->nRefs == 0) {
			pts = Realloc(pts, (nPts+2)*sizeof(SK_Point **));
			pts[nPts++] = s->p1;
			pts[nPts++] = s->p2;
			SK_NodeDelReference(s, s->p1);
			SK_NodeDelReference(s, s->p2);
			SK_NodeDel(s);
		}
	}
	for (i = 0; i < nPts; i++) {
		if (pts[i] == NULL ||
		    SKNODE(pts[i])->nRefs > 0) {
			continue;
		}
		if (i+1 < nPts) {
			for (j = i+1; j < nPts; j++)
				if (pts[j] == pts[i])
					pts[j] = NULL;
		}
		SK_NodeDel(pts[i]);
	}
	rv = SK_NodeDel(poly);
	SK_Update(sk);
	Free(pts);
	return (rv);
}

void
SK_PolygonColor(SK_Polygon *poly, M_Color c)
{
	poly->color = c;
}

/* Return the math library representation of this polygon. */
M_Polygon
SK_PolygonValue(SK_Polygon *poly)
{
	M_Polygon P;
	Uint i;

	P.n = poly->n;
	P.v = Malloc(P.n*sizeof(M_Vector2));
	for (i = 0; i < P.n; i++) {
		M_Line2 ln = SK_LineValue(poly->s[i]);
		P.v[i] = ln.p;
	}
	P._pad = 0;
	return (P);
}

Uint
SK_PolygonAddSide(SK_Polygon *poly, SK_Line *L)
{
	poly->s = Realloc(poly->s, (poly->n+1)*sizeof(SK_Line **));
	poly->s[poly->n] = L;
	return (poly->n++);
}

SK_NodeOps skPolygonOps = {
	"Polygon",
	sizeof(SK_Polygon),
	0,
	SK_PolygonInit,
	NULL,			/* destroy */
	SK_PolygonLoad,
	SK_PolygonSave,
	SK_PolygonDraw,
	NULL,			/* redraw */
	NULL,			/* edit */
	SK_PolygonProximity,
	SK_PolygonDelete,
	NULL,			/* move */
	NULL,			/* constrained */
};

struct sk_polygon_tool {
	SK_Tool tool;
	SK_Polygon *curPoly;
	SK_Point *lastPt;
};

static void
ToolInit(void *_Nonnull p)
{
	struct sk_polygon_tool *t = p;

	t->curPoly = NULL;
	t->lastPt = NULL;
}

static int
ToolMouseButtonDown(void *_Nonnull p, M_Vector3 pos, int btn)
{
	struct sk_polygon_tool *t = p;
	SK_Polygon *poly;
	SK_View *skv = SKTOOL(t)->skv;
	SK *sk = skv->sk;
	SK_Point *newPt;
	SK_Line *Ln;
	M_Vector3 vC;
	Uint i;

	switch (btn) {
	case AG_MOUSE_LEFT:
		if (t->curPoly == NULL) {
			t->curPoly = SK_PolygonNew(sk->root);
		}
		poly = t->curPoly;

		if ((newPt = SK_ViewOverPoint(skv, &pos, &vC, t->lastPt))
		    == NULL) {
			newPt = SK_PointNew(sk->root);
			SK_Translatev(newPt, &pos);
			SK_SuppressConstraints(newPt);
		}
		if (t->lastPt != NULL) {
			Ln = SK_LineNew(sk->root);
			Ln->p1 = t->lastPt;
			Ln->p2 = newPt;
			SK_PolygonAddSide(poly, Ln);
			SK_NodeAddReference(poly, Ln);
		}
		t->lastPt = newPt;
		break;
	default:
		if (t->curPoly != NULL && t->curPoly->n >= 3) {
			poly = t->curPoly;
			if (poly->n < 3) {
				SK_NodeDel(poly);
				break;
			}
			Ln = SK_LineNew(sk->root);
			Ln->p1 = poly->s[poly->n-1]->p2;
			Ln->p2 = poly->s[0]->p1;
			SK_PolygonAddSide(poly, Ln);
			SK_NodeAddReference(poly, Ln);
			for (i = 0; i < poly->n; i++) {
				SK_UnsuppressConstraints(poly->s[i]->p1);
				SK_UnsuppressConstraints(poly->s[i]->p2);
			}
			t->curPoly = NULL;
		}
		t->lastPt = NULL;
		break;
	}
	SK_Update(sk);
	return (1);
}

SK_ToolOps skPolygonToolOps = {
	N_("Polygon"),
	N_("Insert a polygon into the sketch"),
	NULL,
	sizeof(struct sk_polygon_tool),
	0,
	ToolInit,
	NULL,			/* destroy */
	NULL,			/* edit */
	NULL,			/* mousemotion */
	ToolMouseButtonDown,
	NULL,			/* buttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
