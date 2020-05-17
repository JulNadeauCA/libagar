/*
 * Copyright (c) 2006-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Polyhedral object (with winged-edge structure as boundary representation).
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>
#include <agar/sg/sg_load_ply.h>

#include <string.h>

SG_Object *
SG_ObjectNew(void *parent, const char *name)
{
	SG_Object *so;

	so = Malloc(sizeof(SG_Object));
	AG_ObjectInit(so, &sgObjectClass);
	if (name) {
		AG_ObjectSetNameS(so, name);
	} else {
		OBJECT(so)->flags |= AG_OBJECT_NAME_ONATTACH;
	}
	AG_ObjectAttach(parent, so);
	return (so);
}

static void
EditListPoll(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_SELF();
	AG_Tlist *tl = AG_TLIST_PTR(1);
	const int depth = AG_INT(2);
	AG_TlistItem *itVertices, *itFacets, *itEdges, *it;
	int i;
	
	/* Vertices */
	itVertices = AG_TlistAddS(tl, sgIconVertices.s, _("Vertices"));
	itVertices->depth = depth+1;
	itVertices->p1 = so->vtx;
	if (so->nVtx > 0) {
		itVertices->flags |= AG_TLIST_HAS_CHILDREN;
		if (AG_TlistVisibleChildren(tl, itVertices)) {
			for (i = 0; i < so->nVtx; i++) {
				SG_Vertex *v = &so->vtx[i];

				it = AG_TlistAdd(tl, sgIconOrigin.s, "v%d", i);
				it->cat = "vertex";
				it->depth = depth+2;
				it->p1 = v;
				it->selected = (v->flags & SG_VERTEX_SELECTED);
			}
		}
	}

	/* Edges */
	itEdges = AG_TlistAddS(tl, sgIconLine.s, _("Edges"));
	itEdges->depth = depth+1;
	itEdges->p1 = so->edgeTbl;
	if (so->nEdgeTbl > 0) {
		itEdges->flags |= AG_TLIST_HAS_CHILDREN;
		if (AG_TlistVisibleChildren(tl, itEdges)) {
			char en[SG_EDGE_NAME_MAX];
			SG_Edge *e;

			SG_FOREACH_EDGE(e, i, so) {
				SG_EdgeGetName(e, en, sizeof(en));
				it = AG_TlistAddS(tl, sgIconLine.s, en);
				it->cat = "edge";
				it->depth = depth+2;
				it->p1 = e;
				it->selected = (e->flags & SG_EDGE_SELECTED);
			}
		}
	}

	/* Facets */
	itFacets = AG_TlistAddS(tl, sgIconFacet.s, _("Facets"));
	itFacets->depth = depth+1;
	itFacets->p1 = so->facetTbl;
	if (so->nFacetTbl > 0) {
		itFacets->flags |= AG_TLIST_HAS_CHILDREN;
		if (AG_TlistVisibleChildren(tl, itFacets)) {
			for (i = 0; i < so->nFacetTbl; i++) {
				char fn[SG_FACET_NAME_MAX];
				SG_FacetEnt *fe = &so->facetTbl[i];
				SG_Facet *f;

				SLIST_FOREACH(f, &fe->facets, facets) {
					SG_FacetGetName(f, fn, sizeof(fn));
					it = AG_TlistAddS(tl, sgIconFacet.s, fn);
					it->cat = "facet";
					it->depth = depth+2;
					it->p1 = f;
					it->selected = (f->flags & SG_FACET_SELECTED);
				}
			}
		}
	}
}

static void
Init(void *_Nonnull obj)
{
	SG_Object *so = obj;

	so->flags = 0;
	so->nVtx = 1;
	so->vtx = Malloc(sizeof(SG_Vertex));
	so->edgeTbl = Malloc(sizeof(SG_EdgeEnt));
	so->nEdgeTbl = 1;
	SLIST_INIT(&so->edgeTbl[0].edges);
	so->nFacetTbl = 1;
	so->facetTbl = Malloc(sizeof(SG_FacetEnt));
	SLIST_INIT(&so->facetTbl[0].facets);

	SG_VertexInit(&so->vtx[0]);				/* Reserved */
	so->tex = NULL;
	so->bsp = NULL;

	AG_SetEvent(so, "edit-list-poll", EditListPoll, NULL);
}

static void
Reset(void *_Nonnull obj)
{
	SG_Object *so = obj;

	SG_ObjectFreeGeometry(so);
}

/* Set an object rendering texture. */
void
SG_ObjectSetTexture(void *obj, SG_Texture *tex)
{
	SG_Object *so = obj;

	AG_ObjectLock(so);
	so->tex = tex;
	AG_ObjectUnlock(so);
}

/* Get a pointer to an object texture. */
SG_Texture *
SG_ObjectGetTexture(void *obj)
{	
	SG_Object *so = obj;
	SG_Texture *tex;
	
	AG_ObjectLock(so);
	tex = so->tex;
	AG_ObjectUnlock(so);
	return (tex);
}

/* Initialize an existing vertex structure. */
void
SG_VertexInit(SG_Vertex *vtx)
{
	vtx->v = M_VECTOR3(0.0, 0.0, 0.0);
	vtx->n = M_VECTOR3(0.0, 0.0, 0.0);
	vtx->c = M_ColorBlack();
	vtx->st = M_VECTOR2(0.0, 0.0);
	vtx->flags = 0;
}

/* Allocate a vertex from a specified position vector. */
int
SG_VertexNewv(void *obj, const M_Vector3 *vNew)
{
	SG_Object *so = obj;
	SG_Vertex *vtx;
	Uint i;
	int rv;

	AG_ObjectLock(so);

	/* Check for duplicate vertices. */
	if (so->flags & SG_OBJECT_NODUPVERTEX) {
		for (i = 1; i < so->nVtx; i++) {
			vtx = &so->vtx[i];
			if (vtx->v.x == vNew->x &&
			    vtx->v.y == vNew->y &&
			    vtx->v.z == vNew->z) {
				rv = i;
				goto out;
			}
		}
	}

	/* Allocate new vertex. */
	so->vtx = Realloc(so->vtx,(so->nVtx+1)*sizeof(SG_Vertex));
	vtx = &so->vtx[so->nVtx];
	SG_VertexInit(vtx);
	vtx->v.x = vNew->x;
	vtx->v.y = vNew->y;
	vtx->v.z = vNew->z;
	rv = (int)(so->nVtx++);
out:
	AG_ObjectUnlock(so);
	return (rv);
}

/* Allocate a vertex from specified position and normal vectors. */
int
SG_VertexNewvn(void *obj, const M_Vector3 *vNew, const M_Vector3 *nNew)
{
	SG_Object *so = obj;
	SG_Vertex *vtx;
	int vn;

	AG_ObjectLock(so);
	vn = SG_VertexNewv(obj, vNew);
	vtx = &so->vtx[vn];
	vtx->n = *nNew;
	AG_ObjectUnlock(so);
	return (vn);
}

/* Allocate a vertex from specified position and normal vectors. */
int
SG_VertexNew(void *obj, const M_Vector3 vNew)
{
	return (SG_VertexNewv(obj, &vNew));
}

/* Allocate a vertex from specified position. */
int
SG_VertexNew3(void *obj, M_Real x, M_Real y, M_Real z)
{
	M_Vector3 v;

	v.x = x;
	v.y = y;
	v.z = z;
	return (SG_VertexNewv(obj, &v));
}

/* Allocate a vertex from specified position. */
int
SG_VertexNew2(void *obj, M_Real x, M_Real z)
{
	M_Vector3 v;

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

	AG_ObjectLock(so);
	vind = SG_VertexNewv(obj, &vp->v);
	v = &so->vtx[vind];
	v->c = vp->c;
	v->n = vp->n;
	v->st = vp->st;
	AG_ObjectUnlock(so);
	return (vind);
}

/* Resize the object's edge table. */
int
SG_EdgeRehash(void *pso, Uint nEdgesNew)
{
	SG_Object *so = pso;
	SG_EdgeEnt *edgeTblNew;
	Uint nEdgesOld, i;

	AG_ObjectLock(so);

	nEdgesOld = so->nEdgeTbl;
	Verbose("%s: rehashing for %u->%u edges\n", OBJECT(so)->name,
	    nEdgesOld, nEdgesNew);

	if ((edgeTblNew = TryMalloc(nEdgesNew*sizeof(SG_EdgeEnt))) == NULL) {
		AG_SetError("Out of memory");
		AG_ObjectUnlock(so);
		return (-1);
	}
	so->nEdgeTbl = nEdgesNew;

	for (i = 0; i < nEdgesNew; i++) {
		SLIST_INIT(&edgeTblNew[i].edges);
	}
	for (i = 0; i < nEdgesOld; i++) {
		SG_EdgeEnt *eeNew, *eeOld;
		SG_Edge *e, *eNext;
		
		eeOld = &so->edgeTbl[i];
		for (e = SLIST_FIRST(&eeOld->edges);
		     e != SLIST_END(&eeOld->edges);
		     e = eNext) {
			eNext = SLIST_NEXT(e, edges);
			eeNew = &edgeTblNew[SG_HashEdge(so, e->v, e->oe->v)];
			SLIST_INSERT_HEAD(&eeNew->edges, e, edges);
		}
	}
	Free(so->edgeTbl);
	so->edgeTbl = edgeTblNew;

	AG_ObjectUnlock(so);
	return (0);
}

/* Resize the object's facet table. */
int
SG_FacetRehash(void *pso, Uint nFacetsNew)
{
	SG_Object *so = pso;
	SG_FacetEnt *facetTblNew;
	Uint nFacetsOld, i;
	
	AG_ObjectLock(so);

	nFacetsOld = so->nFacetTbl;
	Verbose("%s: rehashing for %u->%u facets\n", OBJECT(so)->name,
	    nFacetsOld, nFacetsNew);

	if ((facetTblNew = TryMalloc(nFacetsNew*sizeof(SG_FacetEnt))) == NULL) {
		AG_SetError("Out of memory");
		AG_ObjectUnlock(so);
		return (-1);
	}
	so->nFacetTbl = nFacetsNew;

	for (i = 0; i < nFacetsNew; i++) {
		SLIST_INIT(&facetTblNew[i].facets);
	}
	for (i = 0; i < nFacetsOld; i++) {
		SG_FacetEnt *feNew, *feOld;
		SG_Facet *f, *fNext;

		feOld = &so->facetTbl[i];
		for (f = SLIST_FIRST(&feOld->facets);
		     f != SLIST_END(&feOld->facets);
		     f = fNext) {
			fNext = SLIST_NEXT(f, facets);
			feNew = &facetTblNew[SG_HashFacet(so,f)];
			SLIST_INSERT_HEAD(&feNew->facets, f, facets);
		}
	}
	Free(so->facetTbl);
	so->facetTbl = facetTblNew;

	AG_ObjectUnlock(so);
	return (0);
}

/*
 * Create (or fetch) the edge between vertices vT and vH. Returns the
 * HEAD halfedge (vH). By convention, the HEAD halfedge points to the
 * face at the LEFT of the edge.
 * 
 * This function does not assign the facet pointers. The face pointers of
 * existing edges are not changed, and for new edges they are set to NULL.
 */
SG_Edge *
SG_Edge2(void *obj, int vT, int vH)
{
	SG_Object *so = obj;
	SG_EdgeEnt *ee;
	SG_Edge *e;
	
	AG_ObjectLock(so);
	ee = &so->edgeTbl[SG_HashEdge(so, vT,vH)];

	SLIST_FOREACH(e, &ee->edges, edges) {
		if (e->v == vH && e->oe->v == vT) {
			AG_ObjectUnlock(so);
			return (e);
		}
		if (e->v == vT && e->oe->v == vH) {
			AG_ObjectUnlock(so);
			return (e->oe);
		}
	}
	e = Malloc(sizeof(SG_Edge));
	e->flags = 0;
	e->v = vH;
	e->f = NULL;
	SLIST_INSERT_HEAD(&ee->edges, e, edges);

	e->oe = Malloc(sizeof(SG_Edge));
	e->oe->flags = 0;
	e->oe->v = vT;
	e->oe->f = NULL;
	e->oe->oe = e;
	SLIST_INSERT_HEAD(&ee->edges, e->oe, edges);
	
	AG_ObjectUnlock(so);
	return (e);
}

/*
 * Generate a unique name string for an (half)edge. The parent object
 * must be locked.
 */
void
SG_EdgeGetName(SG_Edge *e, char *dst, AG_Size dst_len)
{
	Snprintf(dst, dst_len, "%u->%u", e->v, e->oe->v);
}

/*
 * Destroy a facet and all references to it. This leaves a "hole"
 * in the place of the facet.
 */
void
SG_FacetDelete(SG_Facet *f)
{
	SG_Object *so = f->obj;
	SG_FacetEnt *fe;
	SG_Edge *e;
	Uint i;

	AG_ObjectLock(so);

	SG_FOREACH_EDGE(e, i, so) {
		if (e->f == f)
			e->f = NULL;
	}

	fe = &so->facetTbl[SG_HashFacet(so,f)];
	SLIST_REMOVE(&fe->facets, f, sg_facet, facets);
	Free(f);
	
	AG_ObjectUnlock(so);
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
	SG_FacetEnt *fe;
	SG_Facet *f;
	int i;
	
	f = Malloc(sizeof(SG_Facet));
	f->obj = so;
	f->n = 3;
	f->flags = 0;
	f->of = Malloc(sizeof(SG_Facet));
	f->of->obj = so;
	f->of->n = 3;
	f->of->flags = 0;

	AG_ObjectLock(so);

	f->e[0] = SG_Edge2(so, v1,v2);	f->of->e[0] = f->e[0]->oe;
	f->e[1] = SG_Edge2(so, v2,v3);	f->of->e[1] = f->e[1]->oe;
	f->e[2] = SG_Edge2(so, v3,v1);	f->of->e[2] = f->e[2]->oe;
	f->e[3] = NULL;			f->of->e[3] = NULL;

	for (i = 0; i < 3; i++) {
		f->e[i]->f = f;
		f->e[i]->oe->f = f->of;
	}

	fe = &so->facetTbl[SG_HashTriangle(so, v1,v2,v3)];
	SLIST_INSERT_HEAD(&fe->facets, f, facets);

	AG_ObjectUnlock(so);
	return (f);
}

/* Generate a quad facet from a specified contour. */
SG_Facet *
SG_FacetFromQuad4(void *obj, int v1, int v2, int v3, int v4)
{
	SG_Object *so = obj;
	SG_FacetEnt *fe;
	SG_Facet *f;
	int i;

	f = Malloc(sizeof(SG_Facet));
	f->obj = so;
	f->n = 4;
	f->flags = 0;
	f->of = Malloc(sizeof(SG_Facet));
	f->of->obj = so;
	f->of->n = 4;
	f->of->flags = 0;

	AG_ObjectLock(so);

	f->e[0] = SG_Edge2(so, v1,v2);	f->of->e[0] = f->e[0]->oe;
	f->e[1] = SG_Edge2(so, v2,v3);	f->of->e[1] = f->e[1]->oe;
	f->e[2] = SG_Edge2(so, v3,v4);	f->of->e[2] = f->e[2]->oe;
	f->e[3] = SG_Edge2(so, v4,v1);	f->of->e[3] = f->e[3]->oe;
	for (i = 0; i < 4; i++) {
		f->e[i]->f = f;
		f->e[i]->oe->f = f->of;
	}
	
	fe = &so->facetTbl[SG_HashQuad(so, v1,v2,v3,v4)];
	SLIST_INSERT_HEAD(&fe->facets, f, facets);
	
	AG_ObjectUnlock(so);
	return (f);
}

/*
 * Generate a unique name string for a facet.
 * Parent object must be locked.
 */
void
SG_FacetGetName(SG_Facet *f, char *dst, AG_Size dst_len)
{
	switch (f->n) {
	case 3:
		Snprintf(dst, dst_len, "%u,%u,%u",
		    f->e[0]->v,
		    f->e[1]->v,
		    f->e[2]->v);
		break;
	case 4:
		Snprintf(dst, dst_len, "%u,%u,%u,%u",
		    f->e[0]->v,
		    f->e[1]->v,
		    f->e[2]->v,
		    f->e[3]->v);
		break;
	default:
		break;
	}
}

/* Create an extrusion along a given direction from a facet. */
int
SG_FacetExtrude(void *obj, SG_Facet *f, M_Vector3 d, SG_ExtrudeMode mode)
{
	SG_Object *so = obj;
	SG_Facet *fE[4];
	int i;

	AG_ObjectLock(so);

	for (i = 0; i < f->n; i++) {
		if (LFACE(f->e[i]) != f) {
			AG_SetError("Inconsistent facet edge %d", i);
			goto fail;
		}
		if (RFACE(f->e[i]) != NULL) {
			/* TODO replace face */
			AG_SetError("Cannot extrude solid facet edge %d", i);
			goto fail;
		}
	}
	for (i = 0; i < 1; i++) {
		fE[i] = SG_FacetFromQuad4(so,
		    HVTX(f->e[i]),
		    SG_VertexNew(so, M_VecAdd3(OBJ_V(so,HVTX(f->e[i])), d)),
		    SG_VertexNew(so, M_VecAdd3(OBJ_V(so,TVTX(f->e[i])), d)),
		    TVTX(f->e[i]));
		if (fE[i] == NULL)
			goto fail;
	}
	AG_ObjectUnlock(so);
	return (0);
fail:
	AG_ObjectUnlock(so);
	return (-1);
}

/* Return the area covered by a facet. */
M_Real
SG_FacetArea(SG_Object *so, SG_Facet *f)
{
	M_Real area = 0.0;
	int i, j;

	AG_ObjectLock(so);
	for (i = 0; i < f->n; i++) {
		j = (i + 1) % f->n;
		area += FACET_V(so,f,i).x * FACET_V(so,f,j).y;
		area -= FACET_V(so,f,i).y * FACET_V(so,f,j).x;
	}
	AG_ObjectUnlock(so);

	area /= 2.0;
	return (Fabs(area));
}

/* Return the area covered by a facet (signed). */
M_Real
SG_FacetAreaSigned(SG_Object *so, SG_Facet *f)
{
	M_Real area = 0.0;
	int i, j;

	AG_ObjectLock(so);
	for (i = 0; i < f->n; i++) {
		j = (i + 1) % f->n;
		area += FACET_V(so,f,i).x * FACET_V(so,f,j).y;
		area -= FACET_V(so,f,i).y * FACET_V(so,f,j).x;
	}
	AG_ObjectUnlock(so);

	return (area/2.0);
}

/*
 * Compute the center of mass (centroid) of the given polygonal facet
 * in world coordinates.
 */
M_Vector3
SG_FacetCentroid(SG_Object *so, SG_Facet *f)
{
	int i, j;
	M_Real dot, aTmp = 0.0;
	M_Vector3 vTmp = M_VecZero3();
	
	AG_ObjectLock(so);

	for (i = (f->n - 1), j = 0;
	     j < f->n;
	     i = j, j++) {
		M_Vector3 *vi = &FACET_V(so,f,i);
		M_Vector3 *vj = &FACET_V(so,f,j);

		dot = M_VecDot3p(vi, vj);
		aTmp += dot;
		vTmp.x += (vj->x + vi->x) * dot;
		vTmp.y += (vj->y + vi->y) * dot;
		vTmp.z += (vj->z + vi->z) * dot;
	}

	AG_ObjectUnlock(so);

	if (aTmp != 0.0) {
		return (M_VecScale3p(&vTmp, 1.0/(2.0*aTmp)));
	}
	return M_VecZero3();				/* Undefined */
}

/* Calculate vertex normals for the given object. */
int
SG_ObjectNormalize(void *obj)
{
	SG_Object *so = obj;
	SG_Facet *f;
	Uint fi;

	AG_ObjectLock(so);
	SG_FOREACH_FACET(f, fi, so) {
		M_Vector3 n;
		int i;

		n = SG_FacetNormal(so, f);
		for (i = 0; i < f->n; i++) {
			FACET_N(so,f,i) = n;
		}
	}
	AG_ObjectUnlock(so);
	return (0);
}

/* Tesselate quads to triangles for the given object. */
Uint
SG_ObjectConvQuadsToTriangles(void *obj)
{
	SG_Object *so = obj;
	SG_Facet *f; //, *t1, *t2;
	Uint count = 0, fi;

	AG_ObjectLock(so);
	SG_FOREACH_FACET(f, fi, so) {
		if (f->n != 4) {
			continue;
		}
		(void)SG_FacetFromTri3(so, FV1(f), FV2(f), FV3(f));
		(void)SG_FacetFromTri3(so, FV3(f), FV4(f), FV1(f));
		/* TODO XXX edge faces */
		SG_FacetDelete(f);
		count++;
	}
	AG_ObjectUnlock(so);
	return (count);
}

/* Check for errors in the edge/facet/vertex connectivity information. */
int
SG_ObjectCheckConnectivity(void *obj, AG_Console *C)
{
	char fn[SG_FACET_NAME_MAX], en[SG_EDGE_NAME_MAX];
	SG_Object *so = obj;
	SG_Facet *f;
	SG_Edge *e;
	Uint i, j, fi;
	Uint nErrors = 0, nRefs;
	Uint nHoles = 0;
	
	AG_ConsoleMsg(C, "*** Pass 1: Checking facets");

	AG_ObjectLock(so);

	SG_FOREACH_FACET(f, fi, so) {
		SG_FacetGetName(f, fn, sizeof(fn));
	
		/* Check back pointer */
		if (f->obj != so) {
			AG_ConsoleMsg(C, "Facet %s invalid backref %p",
			    fn, f->obj);
			nErrors++;
			continue;
		}
		
		/* Check edge count */
		if (f->n != 3 && f->n != 4) {
			AG_ConsoleMsg(C, "Facet %s bad edge count %d",
			    fn, f->n);
			nErrors++;
			continue;
		}

		/*
		 * Each facet must be referenced by exactly n halfedges,
		 * where n is the facet's side count.
		 */
		for (j = 0, nRefs = 0;
		     j < so->nEdgeTbl;
		     j++) {
			SG_EdgeEnt *ee = &so->edgeTbl[j];

			SLIST_FOREACH(e, &ee->edges, edges) {
				if (e->f == f)
					break;
			}
			if (e != NULL)
				nRefs++;
		}
		if (nRefs != f->n) {
			AG_ConsoleMsg(C,
			    "Facet %s is referenced by %d edges instead of %d",
			    fn, nRefs, f->n);
			nErrors++;
		}

		for (j = 0; j < f->n; j++) {
			e = f->e[j];

			/*
			 * Check that the facet's referenced halfedges have
			 * the correct facet pointer.
			 */
			if (e->f != f) {
				AG_ConsoleMsg(C, "Facet %s inconsistent e%d",
				    fn, j);
				nErrors++;
			}

			/*
			 * The opposing halfedge must point to the opposing
			 * facet (dual-sided facet), or NULL
			 * (single-sided facet).
			 */
			if (e->f == e->oe->f) {
				AG_ConsoleMsg(C,
				    "Facet %s e%d has inconsistent facet ref",
				    fn, j);
				nErrors++;
			} else if (e->oe->f == NULL) {
				nHoles++;
			}
		}
	}
	AG_ConsoleMsg(C, "There are %u hole edges", nHoles);

	/* Check edge/facet and edge/vertex connectivity. */
	AG_ConsoleMsg(C, "*** Pass 2: Checking edge->{facet,vertex} connectivity");
	for (i = 0; i < so->nEdgeTbl; i++) {
		SG_EdgeEnt *ee = &so->edgeTbl[i];

		/* Edge must point to a valid vertex. */
		SLIST_FOREACH(e, &ee->edges, edges) {
			SG_EdgeGetName(e, en, sizeof(en));
			if (e->v < 0 || (Uint)e->v >= so->nVtx) {
				AG_ConsoleMsg(C, "Edge %s bad vertex %d",
				    en, e->v);
				continue;
			}
			/* Edge must be referenced by at least one facet. */
			for (fi = 0; fi < so->nFacetTbl; fi++) {
				SLIST_FOREACH(f, &so->facetTbl[fi].facets, facets) {
					for (j = 0; j < f->n; j++) {
						if (f->e[j] == e ||
						    f->e[j] == e->oe)
							break;
					}
					if (j < f->n)
						break;
				}
				if (f != NULL)
					break;
			}
			if (fi == so->nFacetTbl) {
				AG_ConsoleMsg(C, "Unreferenced edge %s", en);
				nErrors++;
			}
		}
	}
	AG_ObjectUnlock(so);

	AG_ConsoleMsg(C, "Found %u errors.\n\n", nErrors);

	if (nErrors > 0) {
		AG_SetError("Found %u errors", nErrors);
		return (-1);
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
	Uint n;

	AG_ObjectLock(so);
	n = so->nVtx;
	M = Malloc(sizeof(Uint8)*n*n);
	for (i = 0; i < n*n; i++) {
		M[i] = 0;
	}
	SLIST_FOREACH(e, &so->edges, edges) {
		M[e->oe->v*n + e->v] = 1;
	}
	*pn = so->nVtx;
	AG_ObjectUnlock(so);
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
	Uint n;

	AG_ObjectLock(so);
	n = so->nVtx;
	M = Malloc(sizeof(Uint8)*n*n);
	for (i = 0; i < n*n; i++) {
		M[i] = 0;
	}
	SLIST_FOREACH(e, &so->edges, edges) {
		M[e->oe->v*n + e->v] = 1;
	}
	*pn = so->nVtx;
	AG_ObjectUnlock(so);
	return (M);
#else
	return (NULL);
#endif
}

/* Destroy all vertices, edges and facets. */
void
SG_ObjectFreeGeometry(void *p)
{
	SG_Object *so = p;
	Uint i;
	
	AG_ObjectLock(so);

	/* Free the edges */
	for (i = 0; i < so->nEdgeTbl; i++) {
		SG_EdgeEnt *ee = &so->edgeTbl[i];
		SG_Edge *e, *eNext;

		for (e = SLIST_FIRST(&ee->edges);
		     e != SLIST_END(&ee->edges);
		     e = eNext) {
			eNext = SLIST_NEXT(e, edges);
			Free(e);
		}
	}
	so->edgeTbl = Realloc(so->edgeTbl, sizeof(SG_EdgeEnt));
	so->nEdgeTbl = 1;
	SLIST_INIT(&so->edgeTbl[0].edges);

	/* Free the facets */
	for (i = 0; i < so->nFacetTbl; i++) {
		SG_FacetEnt *fe = &so->facetTbl[i];
		SG_Facet *f1, *f2;

		for (f1 = SLIST_FIRST(&fe->facets);
		     f1 != SLIST_END(&fe->facets);
		     f1 = f2) {
			f2 = SLIST_NEXT(f1, facets);
			Free(f1);
		}
	}
	so->facetTbl = Realloc(so->facetTbl, sizeof(SG_FacetEnt));
	so->nFacetTbl = 1;
	SLIST_INIT(&so->facetTbl[0].facets);
	
	/* Free the vertices */
	so->vtx = Realloc(so->vtx, sizeof(SG_Vertex));
	so->nVtx = 1;
	
	AG_ObjectUnlock(so);
}

static void
Destroy(void *_Nonnull p)
{
	SG_Object *so = p;

	SG_ObjectFreeGeometry(so);
}

static int
Load(void *_Nonnull obj, AG_DataSource *_Nonnull ds, const AG_Version *_Nonnull ver)
{
	SG_Object *so = obj;
	Uint i, j, count;
	SG_Vertex *vtx;
	SG_Edge *e;
	SG_EdgeEnt *edgeTblNew;
	SG_Facet *f;
	SG_FacetEnt *facetTblNew;
	Uint v[4];

	so->flags = AG_ReadUint32(ds);

	/* Load the vertex data. */
	so->vtx = NULL;
	so->nVtx = (Uint)AG_ReadUint32(ds);
	if ((vtx = TryRealloc(so->vtx, so->nVtx*sizeof(SG_Vertex))) == NULL) {
		return (-1);
	}
	so->vtx = vtx;
	for (i = 1; i < so->nVtx; i++) {
		vtx = &so->vtx[i];
		vtx->st = M_ReadVector2(ds);
		vtx->c = M_ReadColor(ds);
		M_ReadVector3v(ds, &vtx->n);
		M_ReadVector3v(ds, &vtx->v);
		vtx->flags = (Uint)AG_ReadUint8(ds);
	}

	/* Load the halfedge table. */
	count = (Uint)AG_ReadUint32(ds);
	if ((edgeTblNew = TryRealloc(so->edgeTbl, count*sizeof(SG_EdgeEnt)))
	    == NULL) {
		goto fail;
	}
	so->edgeTbl = edgeTblNew;
	so->nEdgeTbl = count;
	Verbose("%s: %u vertices, %u edges\n", AGOBJECT(so)->name,
	    (Uint)so->nVtx, (Uint)so->nEdgeTbl/2);
	for (i = 0; i < so->nEdgeTbl; i++) {
		SG_EdgeEnt *ee = &so->edgeTbl[i];

		SLIST_INIT(&ee->edges);

		count = (Uint)AG_ReadUint32(ds);
		for (j = 0; j < count; j++) {
			if ((e = TryMalloc(sizeof(SG_Edge))) == NULL) {
				goto fail;
			}
			if ((e->oe = TryMalloc(sizeof(SG_Edge))) == NULL) {
				Free(e);
				goto fail;
			}

			/* Load first halfedge */
			e->f = NULL;
			e->v = (int)AG_ReadUint32(ds);
			e->flags = AG_ReadUint8(ds);
			SLIST_INSERT_HEAD(&ee->edges, e, edges);

			/* Load second halfedge */
			e->oe->oe = e;
			e->oe->f = NULL;
			e->oe->v = (int)AG_ReadUint32(ds);
			e->oe->flags = AG_ReadUint8(ds);
			SLIST_INSERT_HEAD(&ee->edges, e->oe, edges);
			
			/* Check that the edge is in the correct bucket. */
			if (SG_HashEdge(so, e->v, e->oe->v) != i) {
				AG_SetError("Edge is in wrong bucket");
				goto fail;
			}
		}
	}

	/* Load the facet information. */
	count = (Uint)AG_ReadUint32(ds);
	if ((facetTblNew = TryRealloc(so->facetTbl, count*sizeof(SG_FacetEnt))) == NULL) {
		goto fail;
	}
	so->facetTbl = facetTblNew;
	so->nFacetTbl = count;
	for (i = 0; i < count; i++) {
		SG_FacetEnt *fe = &so->facetTbl[i];
		SLIST_INIT(&fe->facets);
	}

	Verbose("%s: reading %u facets\n", AGOBJECT(so)->name, count);
	for (i = 0; i < count; i++) {
		SG_FacetEnt *fe;

		if ((f = TryMalloc(sizeof(SG_Facet))) == NULL) {
			goto fail;
		}
		f->obj = so;
		f->n = (Uint)AG_ReadUint8(ds);
		if (f->n < 3 || f->n > 4) {
			AG_SetError("Facet %u invalid edge count", i);
			Free(f);
			goto fail;
		}
		f->flags = (Uint)AG_ReadUint8(ds);
		for (j = 0; j < f->n; j++) {
			v[j] = (Uint)AG_ReadUint32(ds);
		}
		for (j = 0; j < f->n; j++) {
			f->e[j] = SG_FindEdge(so, v[j], v[(j+1)%f->n]);
			if (f->e[j] == NULL) {
				AG_SetError("Facet %u bad edge ref", i);
				Free(f);
				goto fail;
			}
			f->e[j]->f = f;
		}

		fe = &so->facetTbl[SG_HashFacet(so,f)];
		SLIST_INSERT_HEAD(&fe->facets, f, facets);
	}

	/* XXX TODO: load texture */

	return (0);
fail:
	SG_ObjectFreeGeometry(so);
	return (-1);
}

static int
Save(void *_Nonnull p, AG_DataSource *_Nonnull ds)
{
	SG_Object *so = p;
	Uint i, j, count;
	off_t offs;
	SG_Edge *e;
	SG_Vertex *vtx;
	SG_Facet *f;
	
	AG_WriteUint32(ds, (Uint32)so->flags);

	/* Save the vertices. */
	AG_WriteUint32(ds, so->nVtx);
	for (i = 1; i < so->nVtx; i++) {
		vtx = &so->vtx[i];
		M_WriteVector2(ds, &vtx->st);
		M_WriteColor(ds, &vtx->c);
		M_WriteVector3(ds, &vtx->n);
		M_WriteVector3(ds, &vtx->v);
		AG_WriteUint8(ds, (Uint8)vtx->flags);
	}

	/* Save the edge table. Halfedges are saved in pair. */
	AG_WriteUint32(ds, (Uint32)so->nEdgeTbl);
	for (i = 0; i < so->nEdgeTbl; i++) {
		SLIST_FOREACH(e, &so->edgeTbl[i].edges, edges)
			e->flags &= ~(SG_EDGE_SAVED);
	}
	for (i = 0; i < so->nEdgeTbl; i++) {
		count = 0;
		offs = AG_Tell(ds);
		AG_WriteUint32(ds, 0);
		SLIST_FOREACH(e, &so->edgeTbl[i].edges, edges) {
			if (e->flags & SG_EDGE_SAVED)
				continue;

			/* Save first halfedge */
			AG_WriteUint32(ds, (Uint32)e->v);
			AG_WriteUint8(ds, e->flags);
			e->flags |= SG_EDGE_SAVED;

			/* Save second halfedge */
			AG_WriteUint32(ds, (Uint32)e->oe->v);
			AG_WriteUint8(ds, e->oe->flags);
			e->oe->flags |= SG_EDGE_SAVED;

			count++;
		}
		AG_WriteUint32At(ds, (Uint32)count, offs);
	}
	for (i = 0; i < so->nEdgeTbl; i++)
		SLIST_FOREACH(e, &so->edgeTbl[i].edges, edges)
			e->flags &= ~(SG_EDGE_SAVED);

	/* Save the list of facets. */
	count = 0;
	offs = AG_Tell(ds);
	AG_WriteUint32(ds, 0);
	for (i = 0; i < so->nFacetTbl; i++) {
		SG_FacetEnt *fe = &so->facetTbl[i];

		SLIST_FOREACH(f, &fe->facets, facets) {
			AG_WriteUint8(ds, (Uint8)f->n);
			AG_WriteUint8(ds, (Uint8)f->flags);
			for (j = 0; j < f->n; j++) {
				AG_WriteUint32(ds, f->e[j]->v);
			}
			count++;
		}
	}
	AG_WriteUint32At(ds, (Uint32)count, offs);
	return (0);
}

static void
DrawFacetNormals(SG_Object *_Nonnull so, SG_Facet *_Nonnull f)
{
	M_Vector3 vc = SG_FacetCentroid(so, f);
	M_Vector3 n = SG_FacetNormal(so, f);
	int lighting;

	GL_DisableSave(GL_LIGHTING, &lighting);
	GL_Begin(GL_LINES);
	{
		GL_Color3ub(255, 255, 0);
		GL_Vertex3v(&vc);
		M_VecScale3v(&n, 0.1);
		M_VecAdd3v(&n, &vc);
		GL_Vertex3v(&n);
	}
	GL_End();
	GL_EnableSaved(GL_LIGHTING, lighting);
}

static void
DrawVertexNormals(SG_Object *_Nonnull so)
{
	int lighting;
	Uint i;

	GL_DisableSave(GL_LIGHTING, &lighting);
	GL_Begin(GL_LINES);
	GL_Color3ub(0, 255, 0);
	for (i = 0; i < so->nVtx; i++) {
		M_Vector3 *v = &so->vtx[i].v;
		M_Vector3 n = so->vtx[i].n;

		GL_Vertex3v(v);
		M_VecScale3v(&n, 0.1);
		M_VecAdd3v(&n, v);
		GL_Vertex3v(&n);
	}
	GL_End();
	GL_EnableSaved(GL_LIGHTING, lighting);
}

static void
DrawObjectSilouhette(SG_Object *_Nonnull so, SG_View *_Nonnull view)
{
#if 0
	M_Vector3 vCam = SG_NodeDir(view->cam);
	int depthTest;
#endif
	Uint i;
	
	GL_PushAttrib(GL_LIGHTING_BIT|GL_LINE_BIT|GL_COLOR_BUFFER_BIT);
	GL_Disable(GL_LIGHTING);
#if 0
	GL_DisableSave(GL_DEPTH_TEST, &depthTest);
#endif
	GL_LineWidth(4.0);

	GL_Enable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GL_Begin(GL_LINES);
	GL_Color4ub(0, 255, 0, 50);
	for (i = 0; i < so->nEdgeTbl; i++) {
		SG_EdgeEnt *ent = &so->edgeTbl[i];
		SG_Edge *e;

		SLIST_FOREACH(e, &ent->edges, edges) {
#if 0
			/*
			 * Edge is a silouhette edge if it is shared
			 * by both a front-facing and back-facing
			 * polygon.
			 */
			if (e->f != NULL && e->oe->f != NULL) {
				M_Real dot1 = M_VecDot3(vCam,
				    SG_FacetNormal(so, e->f));
				M_Real dot2 = M_VecDot3(vCam,
				    SG_FacetNormal(so, e->oe->f));

				if ((dot1 >= 0 && dot2 <= 0) ||
				    (dot1 <= 0 && dot2 >= 0)) {
					GL_Vertex3v(&so->vtx[e->v].v);
					GL_Vertex3v(&so->vtx[e->oe->v].v);
				}
			}
#else
			GL_Vertex3v(&so->vtx[e->v].v);
			GL_Vertex3v(&so->vtx[e->oe->v].v);
			
#endif
		}
	}
	GL_End();
#if 0
	GL_EnableSaved(GL_DEPTH_TEST, depthTest);
#endif
	GL_PopAttrib();
}

static void
DrawObjectWireframe(SG_Object *_Nonnull so)
{
	Uint i;

	GL_Begin(GL_LINES);
	GL_Color3ub(128, 128, 128);
	for (i = 0; i < so->nEdgeTbl; i++) {
		SG_EdgeEnt *ent = &so->edgeTbl[i];
		SG_Edge *e;

		SLIST_FOREACH(e, &ent->edges, edges) {
			GL_Vertex3v(&so->vtx[e->v].v);
			GL_Vertex3v(&so->vtx[e->oe->v].v);
		}
	}
	GL_End();
}

static void
DrawObjectVertices(SG_Object *_Nonnull so)
{
	float pointSize;
	int lighting;
	Uint i;

	GL_DisableSave(GL_LIGHTING, &lighting);
	GL_GetFloatv(GL_POINT_SIZE, &pointSize);
	GL_PointSize(3.0);
	GL_Begin(GL_POINTS);
	for (i = 1; i < so->nVtx; i++) {
		GL_VertexTN(&so->vtx[i]);
	}
	GL_End();
	GL_EnableSaved(GL_LIGHTING, lighting);
	GL_PointSize(pointSize);
}

static void
Draw(void *_Nonnull p, SG_View *_Nonnull view)
{
	SG_Object *so = p;
	SG *sg = view->sg;
	SG_Facet *f;
	Uint fi, j;

	if (so->tex != NULL) {
		SG_TextureBind(so->tex, view);
	}
	SG_FOREACH_FACET(f, fi, so) {
		switch (f->n) {
		case 3:
			GL_Begin(GL_TRIANGLES);
			GL_VertexTN(&so->vtx[f->e[0]->v]);
			GL_VertexTN(&so->vtx[f->e[1]->v]);
			GL_VertexTN(&so->vtx[f->e[2]->v]);
			GL_End();
			break;
		case 4:
			GL_Begin(GL_QUADS);
			GL_VertexTN(&so->vtx[f->e[0]->v]);
			GL_VertexTN(&so->vtx[f->e[1]->v]);
			GL_VertexTN(&so->vtx[f->e[2]->v]);
			GL_VertexTN(&so->vtx[f->e[3]->v]);
			GL_End();
			break;
		}
		if (sg->flags & SG_OVERLAY_FNORMALS)
			DrawFacetNormals(so, f);

		if (f->flags & SG_FACET_SELECTED) {
			glDisable(GL_LIGHTING);
			glLineWidth(2.0);
			GL_Begin(GL_LINE_LOOP);
			for (j = 0; j < f->n; j++) {
				GL_Color3ub(0, 255, 0);
				GL_Vertex3v(&so->vtx[f->e[j]->v].v);
			}
			GL_End();
			glLineWidth(1.0);
			glEnable(GL_LIGHTING);
		}
	}
	if (so->tex != NULL) {
		SG_TextureUnbind(so->tex, view);
	}
	if (SGNODE_SELECTED(so))
		DrawObjectSilouhette(so, view);
	if (sg->flags & SG_OVERLAY_WIREFRAME)
		DrawObjectWireframe(so);
	if (sg->flags & SG_OVERLAY_VERTICES)
		DrawObjectVertices(so);
	if (sg->flags & SG_OVERLAY_VNORMALS)
		DrawVertexNormals(so);
#if 0
	glInterleavedArrays(GL_T2F_N3F_V3F, SG_VERTEX_STRIDE,
	    (const GLvoid *)so->vtx);
	SLIST_FOREACH(prim, &so->prims, prims) {
		glDrawArrays(prim->type, 0, prim->n);
	}
#endif
}

static int
Intersect(void *_Nonnull obj, M_Geom3 g, M_GeomSet3 *_Nullable S)
{
	return (0);
}

static void
CheckConnectivity(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_PTR(1);
	AG_Window *win;
	AG_Console *cons;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "<%s>: Connectivity Check", OBJECT(so)->name);
	AG_WindowSetPosition(win, AG_WINDOW_MR, 1);
	cons = AG_ConsoleNew(win, AG_CONSOLE_EXPAND);

	(void)SG_ObjectCheckConnectivity(so, cons);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_BR, 80, 20);
	AG_WindowShow(win);
}

static void
RecomputeNormals(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_PTR(1);

	if (SG_ObjectNormalize(so) == -1)
		AG_TextError("%s: %s", AGOBJECT(so)->name, AG_GetError());
}

static void
ConvQuadsToTriangles(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_PTR(1);
	Uint count;

	count = SG_ObjectConvQuadsToTriangles(so);
	AG_TextInfo("%s: converted %u quads", AGOBJECT(so)->name, count);
}

static void
EdgeTableUpdate(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_PTR(1);
	AG_Table *tbl = AG_TABLE_PTR(2);
	AG_Label *lbl = AG_LABEL_PTR(3);
	Uint i;
	Uint totEdges = 0, nWorst = 0, nUsed = 0, nCollisions = 0;
	
	AG_ObjectLock(so);

	AG_TableBegin(tbl);
	for (i = 0; i < so->nEdgeTbl; i++) {
		SG_EdgeEnt *ee = &so->edgeTbl[i];
		SG_Edge *e;
		Uint j = 0;

		SLIST_FOREACH(e, &ee->edges, edges) {
			AG_TableAddRow(tbl, "%d:%d:%d:%p", e->v, e->oe->v, i,
			    e->f);
			j++;
		}
		if (j > 0) { nUsed++; }
		if (j > 2) { nCollisions++; }
		if (j > nWorst) { nWorst = j; }
		totEdges += j;
	}
	AG_TableEnd(tbl);

	AG_LabelText(lbl,
	    _("%u edges in %u buckets (%u vertices)\n"
	      "Distribution: %d%% (worst=%u, collisions=%u)"),
	    totEdges/2, so->nEdgeTbl, so->nVtx,
	    nUsed*100/so->nEdgeTbl, nWorst, nCollisions);
	
	AG_ObjectUnlock(so);
}

static void
EdgeTableDlg(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_PTR(1);
	AG_Window *win;
	AG_Table *tbl;
	AG_Label *lbl;
	AG_Event ev;

	if ((win = AG_WindowNewNamed(0, "%p-edgetbl", so)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("%s: Edge table"), AGOBJECT(so)->name);
	AG_WindowSetPosition(win, AG_WINDOW_MR, 1);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	lbl = AG_LabelNewS(win, AG_LABEL_HFILL, NULL);
	tbl = AG_TableNew(win, AG_TABLE_EXPAND);
	AG_TableAddCol(tbl, "v1", "<8888>", NULL);
	AG_TableAddCol(tbl, "v2", "<8888>", NULL);
	AG_TableAddCol(tbl, "b#", "<8888>", NULL);
	AG_TableAddCol(tbl, "face", NULL, NULL);
	
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Refresh"),
	    EdgeTableUpdate, "%p,%p,%p", so, tbl, lbl);
	
	AG_EventArgs(&ev, "%p,%p,%p", so, tbl, lbl);
	EdgeTableUpdate(&ev);
	
	AG_WindowShow(win);
}

static void
FacetTableUpdate(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_PTR(1);
	AG_Table *tbl = AG_TABLE_PTR(2);
	AG_Label *lbl = AG_LABEL_PTR(3);
	Uint totTris = 0, totQuads = 0;
	Uint nBig = 0, nUsed = 0, nCollisions = 0;
	Uint i;

	AG_ObjectLock(so);

	AG_TableBegin(tbl);
	for (i = 0; i < so->nFacetTbl; i++) {
		SG_FacetEnt *fe = &so->facetTbl[i];
		SG_Facet *f;
		Uint j = 0;

		SLIST_FOREACH(f, &fe->facets, facets) {
			if (f->n == 3) {
				AG_TableAddRow(tbl, "%s:%d:%d:%d:%s:%d:%s",
				    "Tri",
				    f->e[0]->v,
				    f->e[1]->v,
				    f->e[2]->v,
				    "",
				    i,
				    f->flags & SG_FACET_SELECTED ? "SELECTED" : "");
				totTris++;
			} else {
				AG_TableAddRow(tbl, "%s:%d:%d:%d:%d:%d:%s",
				    "Quad",
				    f->e[0]->v,
				    f->e[1]->v,
				    f->e[2]->v,
				    f->e[3]->v,
				    i,
				    f->flags & SG_FACET_SELECTED ? "SELECTED" : "");
				totQuads++;
			}
			j++;
		}
		if (j > 0) { nUsed++; }
		if (j > 1) { nCollisions++; }
		if (j > nBig) { nBig = j; }
	}
	AG_TableEnd(tbl);
	
	AG_LabelText(lbl, 
	    _("%u triangles and %u quads in %u buckets (%u vertices)\n"
	      "Distribution: %d%% (worst=%u, collisions=%u)"),
	    totTris, totQuads, so->nFacetTbl, so->nVtx,
	    nUsed*100/so->nFacetTbl, nBig, nCollisions);
	
	AG_ObjectUnlock(so);
}

static void
FacetTableDlg(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_PTR(1);
	AG_Window *win;
	AG_Table *tbl;
	AG_Label *lbl;
	AG_Event ev;

	if ((win = AG_WindowNewNamed(0, "%p-facettbl", so)) == NULL) {
		return;
	}
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("%s: Facet table"), AGOBJECT(so)->name);
	AG_WindowSetPosition(win, AG_WINDOW_MR, 1);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	lbl = AG_LabelNewS(win, AG_LABEL_HFILL, NULL);
	tbl = AG_TableNew(win, AG_TABLE_EXPAND);
	AG_TableAddCol(tbl, "n", "<Quad>", NULL);
	AG_TableAddCol(tbl, "v0", "<8888>", NULL);
	AG_TableAddCol(tbl, "v1", "<8888>", NULL);
	AG_TableAddCol(tbl, "v2", "<8888>", NULL);
	AG_TableAddCol(tbl, "v3", "<8888>", NULL);
	AG_TableAddCol(tbl, "b#", "<8888>", NULL);
	AG_TableAddCol(tbl, "flags", NULL, NULL);
	
	AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Refresh"),
	    FacetTableUpdate, "%p,%p,%p", so, tbl, lbl);

	AG_EventArgs(&ev, "%p,%p,%p", so, tbl, lbl);
	FacetTableUpdate(&ev);

	AG_WindowShow(win);
}

/* Return a generic menu for an Object. */
void
SG_ObjectMenuInstance(void *pNode, AG_MenuItem *m, SG_View *sgv)
{
	SG_Node *node = pNode;

	AG_MenuAction(m, _("    Edge table..."), NULL,
	    EdgeTableDlg, "%p", node);
	AG_MenuAction(m, _("    Facet table..."), NULL,
	    FacetTableDlg, "%p", node);
	AG_MenuAction(m, _("    Check connectivity"), NULL,
	    CheckConnectivity, "%p", node);
	AG_MenuAction(m, _("    Recompute normals"), NULL,
	    RecomputeNormals, "%p", node);
	AG_MenuAction(m, _("    Convert quads to triangles"), NULL,
	    ConvQuadsToTriangles, "%p", node);
}

/* Import mesh from Stanford PLY file as a SG_Object. */
static void
ImportMeshFromPLY(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_PTR(1);
	SG_View *sgv = SG_VIEW_PTR(2);
	const char *path = AG_STRING(3);
	AG_FileType *ft = AG_PTR(4);
	Uint flags = 0;

	if (AG_FileOptionInt(ft, "ply.vtxnormals")) { flags |= SG_PLY_LOAD_VTX_NORMALS; }
	if (AG_FileOptionInt(ft, "ply.vtxcolors")) { flags |= SG_PLY_LOAD_VTX_COLORS; }
	if (AG_FileOptionInt(ft, "ply.texcoords")) { flags |= SG_PLY_LOAD_TEXCOORDS; }
	if (AG_FileOptionInt(ft, "ply.dups")) { flags |= SG_PLY_DUP_VERTICES; }

	if (SG_ObjectLoadPLY(so, path, flags) == -1) {
		return;
	}
	AG_Redraw(sgv);
}

static void
ImportMeshDlg(AG_Event *_Nonnull event)
{
	SG_Object *so = SG_OBJECT_PTR(1);
	SG_View *sgv = SG_VIEW_PTR(2);
	AG_Window *win;
	AG_FileDlg *dlg;
	AG_FileType *ft;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Import Data into %s..."), OBJECT(so)->name);
	dlg = AG_FileDlgNewMRU(win, "sg-objs",
	    AG_FILEDLG_LOAD | AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND |
	    AG_FILEDLG_MASK_EXT | AG_FILEDLG_MASK_HIDDEN);

	AG_FileDlgSetOptionContainer(dlg, AG_BoxNewVert(win,AG_BOX_HFILL));

	ft = AG_FileDlgAddType(dlg, _("Stanford .PLY Format"), "*.ply",
	    ImportMeshFromPLY, "%p,%p", so, sgv);
	AG_FileOptionNewBool(ft, _("Load vertex normals"), "ply.vtxnormals", 1);
	AG_FileOptionNewBool(ft, _("Load vertex colors"), "ply.vtxcolors", 1);
	AG_FileOptionNewBool(ft, _("Load texture coordinates"), "ply.texcoords", 1);
	AG_FileOptionNewBool(ft, _("Scan for duplicate vertices"), "ply.dups", 1);

	AG_WindowShow(win);
}

static void *_Nullable
Edit(void *_Nonnull obj, SG_View *_Nullable sgv)
{
	SG_Object *so = SGOBJECT(obj);
	AG_Mutex *lock = &OBJECT(so)->lock;
	const SG *sg = SGNODE(so)->sg;
	AG_ObjectSelector *os;
	AG_Box *box;

	if (sg == NULL)
		return (NULL);

	box = AG_BoxNewVert(NULL, AG_BOX_HFILL);
	
	AG_ButtonNewFn(box, 0, _("Import Mesh..."),
	    ImportMeshDlg, "%p,%p", so, sgv);
	
	AG_SeparatorNewHoriz(box);

	os = AG_ObjectSelectorNew(box, AG_OBJSEL_PAGE_DATA, SGNODE(so)->sg,
	    OBJECT(sg)->root, _("Texture: "));
	AG_ObjectSelectorMaskType(os, "SG_Texture:*");
	AG_BindPointerMp(os, "object", (void *)&so->tex, lock);

	return (box);
}

SG_NodeClass sgObjectClass = {
	{
		"SG_Node:SG_Object",
		sizeof(SG_Object),
		{ 0,0 },
		Init,
		Reset,
		Destroy,
		Load,
		Save,
		SG_NodeEdit
	},
	SG_ObjectMenuInstance,
	NULL,			/* menuClass */
	Draw,
	Intersect,
	Edit
};
