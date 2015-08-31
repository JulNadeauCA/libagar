/*
 * Copyright (c) 2011 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Polyhedron routines.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

void
M_PolyhedronInit(M_Polyhedron *P)
{
	P->nv = 0;
	P->ne = 0;
	P->nf = 0;
	P->v = NULL;
	P->e = NULL;
	P->f = NULL;
}

void
M_PolyhedronFree(M_Polyhedron *P)
{
	P->nv = 0;
	P->ne = 0;
	P->nf = 0;
	Free(P->v); P->v = NULL;
	Free(P->e); P->e = NULL;
	Free(P->f); P->f = NULL;
}

int
M_PolyhedronRead(AG_DataSource *ds, M_Polyhedron *P)
{
	Uint i, j;

	P->nv = (Uint)AG_ReadUint32(ds);
	P->ne = (Uint)AG_ReadUint32(ds);
	P->nf = (Uint)AG_ReadUint32(ds);
	
	if ((P->v = TryMalloc(P->nv*sizeof(M_Vector3))) == NULL ||
	    (P->e = TryMalloc(P->ne*sizeof(M_Halfedge))) == NULL ||
	    (P->f = TryMalloc(P->nf*sizeof(M_Facet))) == NULL)
		goto fail;

	/* Read vertices */
	for (i = 0; i < P->nv; i++)
		P->v[i] = M_ReadVector3(ds);

	/* Read edges */
	for (i = 0; i < P->ne; i+=2) {
		M_Halfedge *eHead = &P->e[i];
		M_Halfedge *eTail = &P->e[i+1];

		eHead->v = (Uint)AG_ReadUint32(ds);
		eTail->v = (Uint)AG_ReadUint32(ds);
		if (eHead->v >= P->nv || eTail->v >= P->nv) {
			AG_SetError("Edge%d: Bad vertex %d", i, eHead->v);
			goto fail;
		}
		eHead->f = (Uint)AG_ReadUint32(ds);
		eTail->f = (Uint)AG_ReadUint32(ds);
		if (eHead->f >= P->nf || eTail->f >= P->nf) {
			AG_SetError("Edge%d: Bad facet %d", i, eHead->f);
			goto fail;
		}
		eHead->oe = i+1;
		eTail->oe = i;
	}

	/* Read facets */
	for (i = 0; i < P->nf; i++) {
		M_Facet *f = &P->f[i];
		
		f->n = (Uint)AG_ReadUint8(ds);
		if ((f->e = TryMalloc(f->n*sizeof(Uint))) == NULL) {
			goto fail;
		}
		for (j = 0; j < f->n; j++) {
			f->e[j] = (Uint)AG_ReadUint32(ds);
			if (f->e[j] >= P->ne) {
				AG_SetError("Facet%d[%d]: Bad incident edge %d",
				    i, j, f->e[j]);
				goto fail;
			}
		}
	}
	return (0);
fail:
	return (-1);
}

void
M_PolyhedronWrite(AG_DataSource *ds, const M_Polyhedron *P)
{
	Uint i, j;
	
	AG_WriteUint32(ds, (Uint32)P->nv);
	AG_WriteUint32(ds, (Uint32)P->ne);
	AG_WriteUint32(ds, (Uint32)P->nf);

	/* Write vertices */
	for (i = 0; i < P->nv; i++)
		M_WriteVector3(ds, &P->v[i]);
	
	/* Write edges */
	for (i = 0; i < P->ne; i+=2) {
		M_Halfedge *eHead = &P->e[i];
		M_Halfedge *eTail = &P->e[i+1];

		AG_WriteUint32(ds, (Uint32)eHead->v);
		AG_WriteUint32(ds, (Uint32)eTail->v);
		AG_WriteUint32(ds, (Uint32)eHead->f);
		AG_WriteUint32(ds, (Uint32)eTail->f);
	}
	
	/* Write facets */
	for (i = 0; i < P->nf; i++) {
		M_Facet *f = &P->f[i];

		AG_WriteUint8(ds, (Uint8)f->n);
		for (j = 0; j < f->n; j++)
			AG_WriteUint32(ds, (Uint32)f->e[j]);
	}
}

/*
 * Create a polyhedron vertex.
 * Return vertex index on success or 0 on failure.
 */
Uint
M_PolyhedronAddVertex(M_Polyhedron *P, M_Vector3 v)
{
	M_Vector3 *vNew;

	if ((vNew = AG_TryRealloc(P->v, (P->nv+1)*sizeof(M_Vector3))) == NULL) {
		return (0);
	}
	P->v = vNew;
	P->v[P->nv] = v;
	return (P->nv++);
}

/* Remove a vertex from a polyhedron. Vertex must not be in use. */
void
M_PolyhedronDelVertex(M_Polyhedron *P, Uint i)
{
	if (i < P->nv) {
		if (i < P->nv - 1) {
			memmove(&P->v[i], &P->v[i+1],
			    (P->nv - i - 1)*sizeof(M_Vector3));
		}
		P->nv--;
	}
}

/*
 * Create a polyhedron edge between vertices v1 and v2. The edges are
 * represented by two contiguous M_Halfedge entries (by convention,
 * the first halfedge is the HEAD). Returns index of the HEAD halfedge
 * on success, or 0 on failure.
 */
Uint
M_PolyhedronAddEdge(M_Polyhedron *P, int v1, int v2)
{
	M_Halfedge *eNew;
	int e1, e2;

	if ((eNew = AG_TryRealloc(P->e, (P->ne+2)*sizeof(M_Halfedge))) == NULL) {
		return (0);
	}
	P->e = eNew;
	e1 = P->ne;
	e2 = P->ne+1;
	P->e[e1].v = v1;
	P->e[e2].v = v2;
	P->e[e1].f = 0;
	P->e[e2].f = 0;
	P->e[e1].oe = e2;
	P->e[e2].oe = e1;
	return (e1);
}

/* Remove an edge from a polyhedron. The edge must not be in use. */
void
M_PolyhedronDelEdge(M_Polyhedron *P, Uint e)
{
	Uint i = (P->e[e].oe < e) ? P->e[e].oe : e;	/* Pick head HE */
 
	if (i < P->ne) {
		if (i < P->ne - 2) {
			memmove(&P->e[i], &P->e[i+2],
			    (P->ne - i - 2)*sizeof(M_Halfedge));
		}
		P->ne--;
	}
}

/*
 * Create a polyhedron facet for the specified edges.
 * Return facet index on success or 0 on failure.
 */
Uint
M_PolyhedronAddFacet(M_Polyhedron *P, Uint n, const Uint *e)
{
	M_Facet *fNew, *f;
	Uint *eNew;

	if ((fNew = AG_TryRealloc(P->f, (P->nf+1)*sizeof(M_Facet))) == NULL) {
		return (0);
	}
	if ((eNew = TryMalloc(n*sizeof(Uint))) == NULL) {
		Free(fNew);
		return (0);
	}
	P->f = fNew;
	f = &P->f[P->nf];
	f->e = eNew;
	f->n = n;
	memcpy(f->e, e, n*sizeof(Uint));
	return (P->nf++);
}

/* Remove a facet from a polyhedron. */
void
M_PolyhedronDelFacet(M_Polyhedron *P, Uint i)
{
	if (i < P->nv) {
		Free(P->f[i].e);
		if (i < P->nf - 1) {
			memmove(&P->f[i], &P->f[i+1],
			    (P->nf - i - 1)*sizeof(M_Facet));
		}
		P->nf--;
	}
}
