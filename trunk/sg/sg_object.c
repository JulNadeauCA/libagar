/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "sg.h"
#include "sg_gui.h"

#include <gui/table.h>
#include <gui/objsel.h>

#include <string.h>

const AG_Version sgObjectVer = { 0, 0 };

SG_Object *
SG_ObjectNew(void *pnode, const char *name)
{
	SG_Object *so;

	so = Malloc(sizeof(SG_Object), M_SG);
	SG_ObjectInit(so, name);
	SG_NodeAttach(pnode, so);
	return (so);
}

void
SG_ObjectInit(void *p, const char *name)
{
	SG_Object *so = p;

	SG_NodeInit(so, name, &sgObjectOps, 0);
	so->vtx = Malloc(sizeof(SG_Vertex), M_SG);
	so->nvtx = 1;
	so->edgetbl = Malloc(sizeof(SG_EdgeEnt), M_SG);
	so->nedgetbl = 0;
	SLIST_INIT(&so->facets);
	SG_VertexInit(&so->vtx[0]);				/* Reserved */
	so->mat = NULL;
}

void
SG_VertexInit(SG_Vertex *vtx)
{
	vtx->v = SG_VECTOR(0.0, 0.0, 0.0);
	vtx->n = SG_VECTOR(0.0, 0.0, 0.0);
	vtx->c = SG_ColorRGB(0.0, 0.0, 0.0);
	vtx->s = 0.0;
	vtx->t = 0.0;
	vtx->flags = 0;
}

int
SG_VertexNewv(void *obj, const SG_Vector *vNew)
{
	SG_Object *so = obj;
	SG_Vertex *vtx;
	int i;

	/* Look for existing vertex at given point. */
	for (i = 1; i < so->nvtx; i++) {
		vtx = &so->vtx[i];
		if (vtx->v.x == vNew->x &&
		    vtx->v.y == vNew->y &&
		    vtx->v.z == vNew->z) {
			return (i);
		}
	}
	/* Allocate new vertex. */
	so->vtx = Realloc(so->vtx,(so->nvtx+1)*sizeof(SG_Vertex));
	vtx = &so->vtx[so->nvtx];
	SG_VertexInit(vtx);
	vtx->v.x = vNew->x;
	vtx->v.y = vNew->y;
	vtx->v.z = vNew->z;
	return (int)(so->nvtx++);
}

int
SG_VertexNewvn(void *obj, const SG_Vector *vNew, const SG_Vector *nNew)
{
	SG_Object *so = obj;
	SG_Vertex *vtx;
	int vn;

	vn = SG_VertexNewv(obj, vNew);
	vtx = &so->vtx[vn];
	vtx->n = *nNew;
	return (vn);
}

int
SG_VertexNew(void *obj, const SG_Vector vNew)
{
	return (SG_VertexNewv(obj, &vNew));
}

int
SG_VertexNew3(void *obj, SG_Real x, SG_Real y, SG_Real z)
{
	SG_Vector v;

	v.x = x;
	v.y = y;
	v.z = z;
	return (SG_VertexNewv(obj, &v));
}

int
SG_VertexNew2(void *obj, SG_Real x, SG_Real z)
{
	SG_Vector v;

	v.x = x;
	v.y = 0.0;
	v.z = z;
	return (SG_VertexNewv(obj, &v));
}

/*
 * Create or reuse a vertex, inheriting the position, normal, color and
 * texture coordinates of another vertex.
 */
int
SG_VertexNewCopy(void *obj, const SG_Vertex *vp)
{
	SG_Object *so = obj;
	SG_Vertex *v;
	int vind;

	vind = SG_VertexNewv(obj, &vp->v);
	v = &so->vtx[vind];
	v->c = vp->c;
	v->n = vp->n;
	v->s = vp->s;
	v->t = vp->t;
	return (vind);
}

/* Edge hash function. Opposing halfedges always share the same bucket. */
/* XXX */
static __inline__ Uint
SG_EdgeHash(SG_Object *so, int v1, int v2)
{
	return ((v1*v2) % so->nedgetbl);
}

/* Resize the edge table. */
void
SG_EdgeRehash(void *pso, Uint nedges_new)
{
	SG_Object *so = pso;
	Uint nedges_old = so->nedgetbl;
	Uint i;

	dprintf("Rehashing for %u->%u edges\n", nedges_old, nedges_new);

	so->edgetbl = Realloc(so->edgetbl, nedges_new*sizeof(SG_EdgeEnt));
	so->nedgetbl = nedges_new;
	for (i = nedges_old; i < nedges_new; i++) {
		SLIST_INIT(&so->edgetbl[i].edges);
	}
	for (i = 0; i < nedges_old; i++) {
		SG_EdgeEnt *oEnt = &so->edgetbl[i], *nEnt;
		SG_Edge *e;

		SLIST_FOREACH(e, &oEnt->edges, edges) {
			nEnt = &so->edgetbl[SG_EdgeHash(so, e->v, e->oe->v)];
			if (oEnt != nEnt) {
				SLIST_REMOVE(&oEnt->edges, e, sg_edge, edges);
				SLIST_INSERT_HEAD(&nEnt->edges, e, edges);
			}
		}
	}
}

/* Return the edge v1->v2, if any. */
SG_Edge *
SG_EdgeFindByVtx(void *obj, int v1, int v2)
{
	SG_Object *so = obj;
	SG_EdgeEnt *ent = &so->edgetbl[SG_EdgeHash(so, v1, v2)];
	SG_Edge *e;

	SLIST_FOREACH(e, &ent->edges, edges) {
		if (e->v == v1 && e->oe->v == v2)
			break;
	}
	return (e);
}

/*
 * Create (or reuse) two halfedges incident to the given facets/vertices.
 * Returns the HEAD halfedge. By convention, the HEAD halfedge points to
 * the face at the LEFT of the edge.
 * 
 * This function does not assign the facet pointers. The face pointers of
 * existing edges are not changed, and for new edges they are set to NULL.
 */
SG_Edge *
SG_Edge2(void *obj, int vT, int vH)
{
	SG_Object *so = obj;
	SG_Edge *eT, *eH;
	SG_EdgeEnt *ent = &so->edgetbl[SG_EdgeHash(so,vT,vH)];
	int i;

	SLIST_FOREACH(eH, &ent->edges, edges) {
		if (eH->v == vH && eH->oe->v == vT)
			return (eH);
	}
	eT = Malloc(sizeof(SG_Edge), M_SG);
	eH = Malloc(sizeof(SG_Edge), M_SG);
	eT->v = vT;
	eT->f = NULL;
	eT->oe = eH;
	eH->v = vH;
	eH->f = NULL;
	eH->oe = eT;
	SLIST_INSERT_HEAD(&ent->edges, eT, edges);
	SLIST_INSERT_HEAD(&ent->edges, eH, edges);
	return (eH);
}

SG_Facet *
SG_FacetNew(void *obj, int n)
{
	SG_Object *so = obj;
	SG_Facet *fct;

	fct = Malloc(sizeof(SG_Facet), M_SG);
	fct->obj = so;
	fct->e[0] = NULL;
	fct->e[1] = NULL;
	fct->e[2] = NULL;
	fct->e[3] = NULL;
	fct->n = (Uint16)n;
	fct->flags = 0;
	SLIST_INSERT_HEAD(&so->facets, fct, facets);
	return (fct);
}

/* 
 * Generate a triangular facet from a specified contour.
 *
 * If the contour specifies one or more existing edges, the orientation of
 * the facet may be reversed to be consistent with the existing facets sharing
 * those edges.
 */
SG_Facet *
SG_FacetFromTri3(void *obj, int v1, int v2, int v3)
{
	SG_Object *so = obj;
	SG_Facet *fct;
	int i;

	fct = SG_Facet3(so);
	fct->e[0] = SG_Edge2(so, v1, v2);
	fct->e[1] = SG_Edge2(so, v2, v3);
	fct->e[2] = SG_Edge2(so, v3, v1);
	for (i = 0; i < 3; i++) {
		SG_Edge *fE = fct->e[i];

#if 0
		if (LFACE(fE) != NULL && RFACE(fE) != NULL) {
			AG_SetError("Edges are limited to 2 faces");
			return (NULL);
		}
#endif
		if (LFACE(fE) != NULL) {
			RFACE(fE) = LFACE(fE);
		}
		LFACE(fE) = fct;
	}
	return (fct);
}

/* Generate a quad facet from a specified contour. */
SG_Facet *
SG_FacetFromQuad4(void *obj, int v1, int v2, int v3, int v4)
{
	SG_Object *so = obj;
	SG_Facet *fct;
	int i;

	fct = SG_Facet4(so);
	fct->e[0] = SG_Edge2(so, v1, v2);
	fct->e[1] = SG_Edge2(so, v2, v3);
	fct->e[2] = SG_Edge2(so, v3, v4);
	fct->e[3] = SG_Edge2(so, v4, v1);
	for (i = 0; i < 4; i++) {
		SG_Edge *fE = fct->e[i];
#if 0
		if (LFACE(fE) != NULL && RFACE(fE) != NULL) {
			AG_SetError("Edges are limited to 2 faces");
			return (NULL);
		}
#endif
		if (LFACE(fE) != NULL) {
			RFACE(fE) = LFACE(fE);
		}
		LFACE(fE) = fct;
	}
	return (fct);
}

/* Create an extrusion along a given direction from a facet. */
int
SG_FacetExtrude(void *obj, SG_Facet *f, SG_Vector d, SG_ExtrudeMode mode)
{
	SG_Object *so = obj;
	SG_Facet *fE[4];
	int i;

	for (i = 0; i < f->n; i++) {
		if (LFACE(f->e[i]) != f) {
			AG_SetError("Inconsistent facet edge %d", i);
			return (-1);
		}
		if (RFACE(f->e[i]) != NULL) {
			/* TODO replace face */
			AG_SetError("Cannot extrude solid facet edge %d", i);
			return (-1);
		}
	}
	for (i = 0; i < 1; i++) {
		fE[i] = SG_FacetFromQuad4(so,
		    HVTX(f->e[i]),
		    SG_VertexNew(so, SG_VectorAdd(OBJ_V(so,HVTX(f->e[i])), d)),
		    SG_VertexNew(so, SG_VectorAdd(OBJ_V(so,TVTX(f->e[i])), d)),
		    TVTX(f->e[i]));
		if (fE[i] == NULL)
			return (-1);
	}
	return (0);
}

/* Compute the normal for a given polygon facet. */
SG_Vector
SG_FacetNormal(SG_Object *so, SG_Facet *fct)
{
	SG_Vector v1 = so->vtx[fct->e[2]->v].v;		/* Right-hand rule */
	SG_Vector v0 = so->vtx[fct->e[1]->v].v;
	SG_Vector v2 = so->vtx[fct->e[0]->v].v;
	SG_Vector n;

	n = SG_VectorCross(SG_VectorSub(v0, v1),
	                   SG_VectorSub(v0, v2));
	return (SG_VectorNorm(n));
}

/* Return the area covered by a facet. */
SG_Real
SG_FacetArea(SG_Object *so, SG_Facet *fct)
{
	SG_Real area = 0.0;
	int i, j;

	for (i = 0; i < fct->n; i++) {
		j = (i + 1) % fct->n;
		area += FACET_V(so,fct,i).x * FACET_V(so,fct,j).y;
		area -= FACET_V(so,fct,i).y * FACET_V(so,fct,j).x;
	}
	area /= 2.0;
	return (SG_Fabs(area));
}

/* Return the area covered by a facet (signed). */
SG_Real
SG_FacetAreaSigned(SG_Object *so, SG_Facet *fct)
{
	SG_Real area = 0.0;
	int i, j;

	for (i = 0; i < fct->n; i++) {
		j = (i + 1) % fct->n;
		area += FACET_V(so,fct,i).x * FACET_V(so,fct,j).y;
		area -= FACET_V(so,fct,i).y * FACET_V(so,fct,j).x;
	}
	return (area/2.0);
}

/*
 * Compute the center of mass (centroid) of the given polygonal facet
 * in world coordinates.
 */
SG_Vector
SG_FacetCentroid(SG_Object *so, SG_Facet *fct)
{
	int i, j;
	SG_Real dot, aTmp = 0.0;
	SG_Vector vTmp = SG_0;
	SG_Vector cent;

	for (i = (fct->n - 1), j = 0;
	     j < fct->n;
	     i = j, j++) {
		SG_Vector *vi = &FACET_V(so,fct,i);
		SG_Vector *vj = &FACET_V(so,fct,j);

		dot = SG_VectorDotp(vi, vj);
		aTmp += dot;
		vTmp.x += (vj->x + vi->x) * dot;
		vTmp.y += (vj->y + vi->y) * dot;
		vTmp.z += (vj->z + vi->z) * dot;
	}
	if (aTmp != 0.0) {
		return (SG_VectorScalep(&vTmp, 1.0/(2.0*aTmp)));
	}
	return (SG_0);					/* Undefined */
}

/* Calculate vertex normals for the given object. */
int
SG_ObjectNormalize(void *obj)
{
	SG_Object *so = obj;
	SG_Facet *fct;

	SLIST_FOREACH(fct, &so->facets, facets) {
		SG_Vector n;
		int i;

		n = SG_FacetNormal(so, fct);
		for (i = 0; i < fct->n; i++) {
			FACET_N(so,fct,i) = n;
		}
	}
	return (0);
}

/* Check for errors in the edge/facet/vertex connectivity information. */
int
SG_ObjectCheckConnectivity(void *obj)
{
	SG_Object *so = obj;
	SG_Facet *f;
	SG_Edge *e, *oe;
	int i;

	SLIST_FOREACH(f, &so->facets, facets) {
		/* All facets must be referenced by at least one edge. */
		for (i = 0; i < so->nedgetbl; i++) {
			SG_EdgeEnt *ent = &so->edgetbl[i];

			SLIST_FOREACH(e, &ent->edges, edges) {
				if (e->f == f)
					break;
			}
			if (e != NULL)
				break;
		}
		if (i == so->nedgetbl) {
			AG_SetError("Facet %p not referenced by any edge", f);
			return (-1);
		}
		for (i = 0; i < f->n; i++) {
#if 0
			SLIST_FOREACH(e, &so->edges, edges) {
				if (f->e[i] == e)
					break;
			}
			if (e == NULL) {
				AG_SetError("Facet %p unknown edge %d", f, i);
				return (-1);
			}
#endif
			/*
			 * We are using the CCW convention. The LEFT of any
			 * edge shared by any facet f must point back to f when
			 * traversing the facet counter-clockwise.
			 */
			if (LFACE(f->e[i]) != f) {
				AG_SetError("Facet %p inconsistent e%d", f, i);
				return (-1);
			}
#if 1
			/*
			 * We don't allow edges to have the same facet both as
			 * LEFT and RIGHT yet, although this may be allowed
			 * eventually (ie. for representing two-sided surfaces).
			 */
			if (LFACE(f->e[i]) == RFACE(f->e[i])) {
				AG_SetError("Facet %p two-sided edge %d", f, i);
				return (-1);
			}
#endif
		}
		if (f->n != 3 && f->n != 4) {
			AG_SetError("Facet %p bad edge count %d", f, f->n);
			return (-1);
		}
		if (f->obj != so) {
			AG_SetError("Facet %p invalid backref %p", f, f->obj);
			return (-1);
		}
	}

	/* Check edge/facet and edge/vertex connectivity. */
	for (i = 0; i < so->nedgetbl; i++) {
		SG_EdgeEnt *ent = &so->edgetbl[i];

		/* Edge must point to a valid vertex. */
		SLIST_FOREACH(e, &ent->edges, edges) {
			if (e->v < 0 || e->v >= so->nvtx) {
				AG_SetError("Edge %p invalid vtx %d (obj=%u)",
				    e, e->v, so->nvtx);
				return (-1);
			}
			/* Edge must be referenced by at least one facet. */
			SLIST_FOREACH(f, &so->facets, facets) {
				for (i = 0; i < f->n; i++) {
					if (f->e[i] == e)
						break;
				}
				if (i < f->n)
					break;
			}
			if (f == NULL) {
				AG_SetError("Unreferenced edge %p", e);
				return (-1);
			}
		}
	}
	return (0);
}

/* Generate vertex/edge adjacency matrix. */
Uint8 *
SG_ObjectEdgeMatrix(SG_Object *so, Uint *pn)
{
#if 0
	Uint8 *M;
	SG_Edge *e;
	int i, j;
	Uint n = so->nvtx;
	
	M = Malloc(sizeof(Uint8)*n*n, M_SG);
	for (i = 0; i < n*n; i++) {
		M[i] = 0;
	}
	SLIST_FOREACH(e, &so->edges, edges) {
		M[e->oe->v*n + e->v] = 1;
	}
	*pn = so->nvtx;
	return (M);
#else
	return (NULL);
#endif
}

/* Generate vertex/facet adjacency matrix. */
Uint8 *
SG_ObjectFacetMatrix(SG_Object *so, Uint *pn)
{
#if 0
	Uint8 *M;
	SG_Edge *e;
	int i, j;
	Uint n = so->nvtx;
	
	M = Malloc(sizeof(Uint8)*n*n, M_SG);
	for (i = 0; i < n*n; i++) {
		M[i] = 0;
	}
	SLIST_FOREACH(e, &so->edges, edges) {
		M[e->oe->v*n + e->v] = 1;
	}
	*pn = so->nvtx;
	return (M);
#else
	return (NULL);
#endif
}

void
SG_ObjectFreeGeometry(void *p)
{
	SG_Object *so = p;
	SG_Facet *f1, *f2;
	Uint i;
	
	for (i = 0; i < so->nedgetbl; i++) {
		SG_EdgeEnt *ent = &so->edgetbl[i];
		SG_Edge *e1, *e2;

		for (e1 = SLIST_FIRST(&ent->edges);
		     e1 != SLIST_END(&ent->edges);
		     e1 = e2) {
			e2 = SLIST_NEXT(e1, edges);
			Free(e1, M_SG);
		}
		SLIST_INIT(&ent->edges);
	}
	for (f1 = SLIST_FIRST(&so->facets);
	     f1 != SLIST_END(&so->facets);
	     f1 = f2) {
		f2 = SLIST_NEXT(f1, facets);
		Free(f1, M_SG);
	}
	SLIST_INIT(&so->facets);
	
	so->vtx = Realloc(so->vtx, sizeof(SG_Vertex));
	so->nvtx = 1;
	so->edgetbl = Realloc(so->edgetbl, sizeof(SG_EdgeEnt));
	so->nedgetbl = 0;
}

void
SG_ObjectDestroy(void *p)
{
	SG_Object *so = p;

	SG_ObjectFreeGeometry(so);
}

int
SG_ObjectLoad(void *p, AG_Netbuf *buf)
{
	SG_Object *so = p;
	Uint i;
	
	if (AG_ReadVersion(buf, "SG:SG_Object", &sgObjectVer, NULL) != 0) {
		return (-1);
	}
	so->flags = AG_ReadUint32(buf);
	
	return (0);
	/* TODO */

	so->vtx = NULL;
	so->nvtx = (Uint)AG_ReadUint32(buf);
	so->vtx = Realloc(so->vtx, so->nvtx*sizeof(SG_Vertex));
	dprintf("%s: reading %u vertices\n", SGNODE(so)->name, (Uint)so->nvtx);
	for (i = 0; i < so->nvtx; i++) {
		SG_Vertex *vtx = &so->vtx[i];
	
		vtx->s = SG_ReadReal(buf);
		vtx->t = SG_ReadReal(buf);
		vtx->c = SG_ReadColor(buf);
		SG_ReadVectorv(buf, &vtx->n);
		SG_ReadVectorv(buf, &vtx->v);
		vtx->flags = (Uint)AG_ReadUint8(buf);
	}
	
	return (0);
}

int
SG_ObjectSave(void *p, AG_Netbuf *buf)
{
	SG_Object *so = p;
	Uint i;
	off_t offs;
	SG_Facet *f;
	SG_Edge *e;
	
	AG_WriteVersion(buf, "SG:SG_Object", &sgObjectVer);
	AG_WriteUint32(buf, (Uint32)so->flags);
	
	return (0);
	/* TODO */
	
	/* Save the vertices. */
	AG_WriteUint32(buf, so->nvtx);
	dprintf("%s: saving %u vertices\n", SGNODE(so)->name, so->nvtx);
	for (i = 1; i < so->nvtx; i++) {
		SG_Vertex *vtx = &so->vtx[i];

		SG_WriteReal(buf, vtx->s);
		SG_WriteReal(buf, vtx->t);
		SG_WriteColor(buf, &vtx->c);
		SG_WriteVector(buf, &vtx->n);
		SG_WriteVector(buf, &vtx->v);
		AG_WriteUint8(buf, (Uint8)vtx->flags);
	}
	
	dprintf("%s: saving %u edges\n", SGNODE(so)->name, (Uint)so->nedgetbl);

	/* Save the edge table. */
	AG_WriteUint32(buf, (Uint32)so->nedgetbl);
	for (i = 0; i < so->nedgetbl; i++) {
		SG_EdgeEnt *ee = &so->edgetbl[i];
		SG_Edge *e;
		Uint32 count = 0;

		offs = AG_NetbufTell(buf);
		AG_WriteUint32(buf, 0);
		SLIST_FOREACH(e, &ee->edges, edges) {
			AG_WriteUint32(buf, e->v);
			/* XXX */
			count++;
		}
		AG_PwriteUint32(buf, count, offs);
	}
	return (0);
}

static void
DrawFacetNormals(SG_Object *so, SG_Facet *fct)
{
	SG_Vector vc = SG_FacetCentroid(so, fct);
	SG_Vector n = SG_FacetNormal(so, fct);
	int lit;
	
	if ((lit = glIsEnabled(GL_LIGHTING))) { glDisable(GL_LIGHTING); }

	SG_Begin(SG_LINES);
	SG_Color3ub(255, 255, 0);
	SG_Vertex3v(&vc);
	SG_VectorScalev(&n, 0.1);
	SG_VectorAddv(&n, &vc);
	SG_Vertex3v(&n);
	SG_End();
	
	if (lit) { glEnable(GL_LIGHTING); }
}

static void
DrawVertexNormals(SG_Object *so)
{
	int i;
	int lit;
	
	if ((lit = glIsEnabled(GL_LIGHTING))) { glDisable(GL_LIGHTING); }

	for (i = 0; i < so->nvtx; i++) {
		SG_Vector *v = &so->vtx[i].v;
		SG_Vector n = so->vtx[i].n;

		SG_Begin(SG_LINES);
		SG_Color3ub(0, 255, 0);
		SG_Vertex3v(v);
		SG_VectorScalev(&n, 0.1);
		SG_VectorAddv(&n, v);
		SG_Vertex3v(&n);
		SG_End();
	}
	
	if (lit) { glEnable(GL_LIGHTING); }
}

static void
DrawObjectSilouhette(SG_Object *so, SG_View *view)
{
	SG_Vector vCam = SG_CameraVector(view->cam);
	int lit;
	int i;
		
	if ((lit = glIsEnabled(GL_LIGHTING))) { glDisable(GL_LIGHTING); }
	glLineWidth(4.0);
	SG_Begin(SG_LINES);
	SG_Color3ub(0, 0, 255);
	
	for (i = 0; i < so->nedgetbl; i++) {
		SG_EdgeEnt *ent = &so->edgetbl[i];
		SG_Edge *e;

		SLIST_FOREACH(e, &ent->edges, edges) {
			/*
			 * Edge is a silouhette edge if it is shared
			 * by both a front-facing and back-facing
			 * polygon.
			 */
			if (e->f != NULL && e->oe->f != NULL) {
				SG_Real dot1 = SG_VectorDot(vCam,
				    SG_FacetNormal(so, e->f));
				SG_Real dot2 = SG_VectorDot(vCam,
				    SG_FacetNormal(so, e->oe->f));

				if ((dot1 > 0.0 && dot2 < 0.0) ||
				    (dot1 < 0.0 && dot2 > 0.0)) {
					SG_Vertex3v(&so->vtx[e->v].v);
					SG_Vertex3v(&so->vtx[e->oe->v].v);
				}
			}
		}
	}
	SG_End();

	if (lit) { glEnable(GL_LIGHTING); }
	glLineWidth(1.0);
}

static void
DrawObjectWireframe(SG_Object *so)
{
	Uint i;

	SG_Begin(SG_LINES);
	SG_Color3ub(128, 128, 128);
	for (i = 0; i < so->nedgetbl; i++) {
		SG_EdgeEnt *ent = &so->edgetbl[i];
		SG_Edge *e;

		SLIST_FOREACH(e, &ent->edges, edges) {
			SG_Vertex3v(&so->vtx[e->v].v);
			SG_Vertex3v(&so->vtx[e->oe->v].v);
		}
	}
	SG_End();
}

static void
DrawObjectVertices(SG_Object *so)
{
	float ptsize_save;
	int lit;
	Uint i;
		
	if ((lit = glIsEnabled(GL_LIGHTING))) {
		glDisable(GL_LIGHTING);
	}
	glGetFloatv(GL_POINT_SIZE, &ptsize_save);

	glPointSize(3.0);
	glBegin(GL_POINTS);
	for (i = 1; i < so->nvtx; i++) {
		SG_VertexTN(&so->vtx[i]);
	}
	glEnd();
	glPointSize(ptsize_save);
	
	if (lit) {
		glEnable(GL_LIGHTING);
	}
}

void
SG_ObjectDraw(void *p, SG_View *view)
{
	SG_Object *so = p;
	SG *sg = SGNODE(so)->sg;
	SG_Facet *fct;

	if (so->mat != NULL) {
		SG_MaterialBind(so->mat, view);
	}
	SLIST_FOREACH(fct, &so->facets, facets) {
		switch (fct->n) {
		case 3:
			SG_Begin(SG_TRIANGLES);
			SG_VertexTN(&so->vtx[fct->e[0]->v]);
			SG_VertexTN(&so->vtx[fct->e[1]->v]);
			SG_VertexTN(&so->vtx[fct->e[2]->v]);
			SG_End();
			break;
		case 4:
			SG_Begin(SG_QUADS);
			SG_VertexTN(&so->vtx[fct->e[0]->v]);
			SG_VertexTN(&so->vtx[fct->e[1]->v]);
			SG_VertexTN(&so->vtx[fct->e[2]->v]);
			SG_VertexTN(&so->vtx[fct->e[3]->v]);
			SG_End();
			break;
		}
		if (sg->flags & SG_OVERLAY_FNORMALS)
			DrawFacetNormals(so, fct);
	}
	if (so->mat != NULL) {
		SG_MaterialUnbind(so->mat, view);
	}
	if (SGNODE_SELECTED(so)) {
		DrawObjectSilouhette(so, view);
	}
	if (sg->flags & SG_OVERLAY_WIREFRAME) {
		DrawObjectWireframe(so);
	}
	if (sg->flags & SG_OVERLAY_VERTICES) {
		DrawObjectVertices(so);
	}
	if (sg->flags & SG_OVERLAY_VNORMALS) {
		DrawVertexNormals(so);
	}
#if 0
	glInterleavedArrays(GL_T2F_N3F_V3F, SG_VERTEX_STRIDE,
	    (const GLvoid *)so->vtx);
	SLIST_FOREACH(prim, &so->prims, prims) {
		glDrawArrays(prim->type, 0, prim->n);
	}
#endif
}

static void
CheckConnectivity(AG_Event *event)
{
	SG_Object *so = AG_PTR(1);

	if (SG_ObjectCheckConnectivity(so) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", SGNODE(so)->name,
		    AG_GetError());
	} else {
		AG_TextMsg(AG_MSG_INFO, _("%s: Connectivity OK"),
		    SGNODE(so)->name);
	}
}

static void
CalculateNormals(AG_Event *event)
{
	SG_Object *so = AG_PTR(1);

	if (SG_ObjectNormalize(so) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", SGNODE(so)->name,
		    AG_GetError());
	}
}

static void
EdgeTableDlg(AG_Event *event)
{
	SG_Object *so = AG_PTR(1);
	AG_Window *win;
	AG_Table *tbl;
	AG_Label *lbl;
	Uint totEdges = 0, bBig = 0, bUsed = 0;
	Uint i;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("%s: Edge table"), SGNODE(so)->name);
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_RIGHT, 0);

	lbl = AG_LabelNewStatic(win, AG_LABEL_HFILL, NULL);
	tbl = AG_TableNew(win, AG_TABLE_EXPAND);
	AG_TableAddCol(tbl, "v1", "<8888>", NULL);
	AG_TableAddCol(tbl, "v2", "<8888>", NULL);
	AG_TableAddCol(tbl, "bucket", "<8888>", NULL);
	AG_TableAddCol(tbl, "face", NULL, NULL);
	AG_TableBegin(tbl);
	for (i = 0; i < so->nedgetbl; i++) {
		SG_EdgeEnt *ent = &so->edgetbl[i];
		SG_Edge *e;
		Uint j = 0;

		SLIST_FOREACH(e, &ent->edges, edges) {
			AG_TableAddRow(tbl, "%d:%d:%d:%p", e->v, e->oe->v, i,
			    e->f);
			j++;
		}
		if (j > 0) { bUsed++; }
		if (j > bBig) { bBig = j; }
		totEdges += j;
	}
	AG_TableEnd(tbl);
	AG_LabelPrintf(lbl,
	    _("%u edges in %u buckets (%u%% usage, biggest=%u)"),
	    totEdges/2, so->nedgetbl, bUsed*100/so->nedgetbl, bBig);

	AG_WindowShow(win);
}

static void
FacetTableDlg(AG_Event *event)
{
	SG_Object *so = AG_PTR(1);
	AG_Window *win;
	AG_Table *tbl;
	AG_Label *lbl;
	Uint totTris = 0, totQuads = 0;
	SG_Facet *f;

	if ((win = AG_WindowNewNamed(0, "%p-facettbl", so)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("%s: Facet table"), SGNODE(so)->name);
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_RIGHT, 0);

	lbl = AG_LabelNewStaticString(win, AG_LABEL_HFILL, NULL);
	tbl = AG_TableNew(win, AG_TABLE_EXPAND);
	AG_TableAddCol(tbl, "n", "<Quad>", NULL);
	AG_TableAddCol(tbl, "v0", "<8888>", NULL);
	AG_TableAddCol(tbl, "v1", "<8888>", NULL);
	AG_TableAddCol(tbl, "v2", "<8888>", NULL);
	AG_TableAddCol(tbl, "v3", "<8888>", NULL);
	AG_TableAddCol(tbl, "flags", NULL, NULL);
	AG_TableBegin(tbl);
	SLIST_FOREACH(f, &so->facets, facets) {
		if (f->n == 3) {
			AG_TableAddRow(tbl, "%s:%d:%d:%d:%s:%s", "Tri",
			    f->e[0]->v, f->e[1]->v, f->e[2]->v, "",
			    f->flags & SG_FACET_SELECTED ? "SELECTED" : "");
			totTris++;
		} else {
			AG_TableAddRow(tbl, "%s:%d:%d:%d:%d:%s", "Quad",
			    f->e[0]->v, f->e[1]->v, f->e[2]->v, f->e[3]->v,
			    f->flags & SG_FACET_SELECTED ? "SELECTED" : "");
			totQuads++;
		}
	}
	AG_TableEnd(tbl);
	AG_LabelPrintf(lbl, _("Total: %u triangles, %u quads"),
	    totTris, totQuads);
	AG_WindowShow(win);
}

void
SG_ObjectMenuInstance(void *pNode, AG_MenuItem *m, SG_View *sgv)
{
	SG_Node *node = pNode;

	AG_MenuAction(m, _("    Edge table..."), -1,
	    EdgeTableDlg, "%p", node);
	AG_MenuAction(m, _("    Facet table..."), -1,
	    FacetTableDlg, "%p", node);
	AG_MenuAction(m, _("    Check connectivity"), -1,
	    CheckConnectivity, "%p", node);
	AG_MenuAction(m, _("    Recalculate normals"), -1,
	    CalculateNormals, "%p", node);
}

void
SG_ObjectMenuClass(SG *sg, AG_MenuItem *m, SG_View *sgv)
{
}

void
SG_ObjectEdit(void *p, AG_Widget *box, SG_View *sgv)
{
	SG_Object *so = p;
	AG_ObjectSelector *os;
	
	os = AG_ObjectSelectorNew(box, AG_OBJSEL_PAGE_DATA, SGNODE(so)->sg,
	    agWorld, _("Material: "));
	AG_ObjectSelectorMaskType(os, "SG_Material:*");
	AG_WidgetBindPointer(os, "object", &so->mat);
}

SG_NodeOps sgObjectOps = {
	"Object",
	sizeof(SG_Object),
	0,
	SG_ObjectInit,
	SG_ObjectDestroy,
	SG_ObjectLoad,
	SG_ObjectSave,
	SG_ObjectEdit,
	SG_ObjectMenuInstance,
	NULL,			/* menuClass */
	SG_ObjectDraw
};

#endif /* HAVE_OPENGL */
