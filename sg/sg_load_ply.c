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
 * Loader for Stanford PLY file format.
 */

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include <agar/sg/sg_load_ply.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>

#define PLY_MAX_HEADER		256
#define PLY_MAX_ELEMENT_NAME	32
#define PLY_MAX_PROP_NAME	32
#define PLY_MAX_LIST_ITEMS	128

enum ply_prop_type {
	PLY_INT8,
	PLY_INT16,
	PLY_INT32,
	PLY_UINT8,
	PLY_UINT16,
	PLY_UINT32,
	PLY_FLOAT32,
	PLY_FLOAT64,
	PLY_LAST_TYPE
};
static const char *plyTypeNames[] = {
	"int8", "int16", "int32", "uint8", "uint16", "uint32",
	"float32", "float64"
};
static const char *plyTypeNamesLegacy[] = {
	"char", "short", "int", "uchar", "ushort", "uint",
	"float", "double"
};

enum ply_std_prop {
	PLY_VERTEX_INDICES,
	PLY_X, PLY_Y, PLY_Z,
	PLY_NX, PLY_NY, PLY_NZ,
	PLY_U, PLY_V,
	PLY_RED, PLY_GREEN, PLY_BLUE,
	PLY_INTENSITY, PLY_OPACITY,
	PLY_AMBIENT_COEFF, PLY_AMBIENT_RED, PLY_AMBIENT_GREEN, PLY_AMBIENT_BLUE,
	PLY_DIFFUSE_COEFF, PLY_DIFFUSE_RED, PLY_DIFFUSE_GREEN, PLY_DIFFUSE_BLUE,
	PLY_SPECULAR_COEFF, PLY_SPECULAR_RED, PLY_SPECULAR_GREEN,
	PLY_SPECULAR_BLUE, PLY_SPECULAR_POWER,
	PLY_STD_PROP_LAST
};
static const char *plyStdPropNames[] = {
	"vertex_indices",
	"x", "y", "z",
	"nx", "ny", "nz",
	"u", "v",
	"red", "green", "blue",
	"intensity",
	"opacity",
	"ambient_coeff", "ambient_red", "ambient_green", "ambient_blue",
	"diffuse_coeff", "diffuse_red", "diffuse_green", "diffuse_blue",
	"specular_coeff", "specular_red", "specular_green", "specular_blue",
	"specular_power"
};

enum ply_std_element {
	PLY_VERTEX,
	PLY_FACE,
	PLY_MATERIAL,
	PLY_STD_ELEMENT_LAST
};
static const char *plyStdElementNames[] = {
	"vertex",
	"face",
	"material"
};

typedef struct ply_prop {
	char name[PLY_MAX_PROP_NAME];
	enum ply_std_prop std;			/* Standard property */
	enum ply_prop_type type;		/* Type of item */
	int list;				/* Is a variable-sized list */
	enum ply_prop_type list_type;		/* Type of list count number */
	TAILQ_ENTRY(ply_prop) props;
} PLY_Prop;

typedef struct ply_element {
	char name[PLY_MAX_ELEMENT_NAME];
	enum ply_std_element std;		/* Standard property */
	Uint count;				/* Number of instances */
	TAILQ_HEAD_(ply_prop) props;		/* Properties per instance */
	TAILQ_ENTRY(ply_element) elements;
} PLY_Element;

typedef struct ply_info {
	enum ply_format {
		PLY_ASCII,
		PLY_BIN_BE,
		PLY_BIN_LE
	} format;
	Uint32 _pad;
	TAILQ_HEAD_(ply_element) elements;
} PLY_Info;

static void
InitPLY(PLY_Info *_Nonnull ply)
{
	ply->format = PLY_ASCII;
	TAILQ_INIT(&ply->elements);
}

static void
FreePLY(PLY_Info *_Nonnull ply)
{
	PLY_Element *el, *el2;
	PLY_Prop *prop, *nprop;

	for (el = TAILQ_FIRST(&ply->elements);
	     el != TAILQ_END(&ply->elements);
	     el = el2) {
		el2 = TAILQ_NEXT(el, elements);

		for (prop = TAILQ_FIRST(&el->props);
		     prop != TAILQ_END(&el->props);
		     prop = nprop) {
			nprop = TAILQ_NEXT(prop, props);
			Free(prop);
		}
		Free(el);
	}
}

static int
IsValidElementName(const char *_Nonnull s)
{
	const char *c;

	for (c = &s[0]; *c != '\0'; c++) {
		if (!isprint(*c)) {
			AG_SetError("Illegal PLY element name `%s'", s);
			return (0);
		}
	}
	return (1);
}

static __inline__ enum ply_prop_type
GetType(const char *_Nonnull name)
{
	int i;

	for (i = 0; i < PLY_LAST_TYPE; i++) {
		if (AG_Strcasecmp(name, plyTypeNames[i]) == 0 ||
		    AG_Strcasecmp(name, plyTypeNamesLegacy[i]) == 0) {
			return (i);
		}
	}
	AG_SetError("Bad prop type `%s'", name);
	return (PLY_LAST_TYPE);
}

static __inline__ int
GetUintASCII(const char *_Nonnull s, Uint *_Nonnull rv)
{
	Uint v;
	char *c;

	v = (Uint)strtoul(s, &c, 10);
	if (s[0] == '\0' || *c != '\0') {
		AG_SetError("Malformed int `%s'", s);
		return (-1);
	}
	*rv = v;
	return (0);
}
		
static __inline__ int
GetRealASCII(const char *_Nonnull s, M_Real *_Nonnull rv)
{
	double v;
	char *c;

	v = (M_Real)strtod(s, &c);
	if (s[0] == '\0' || *c != '\0') {
		AG_SetError("Malformed number `%s'", s);
		return (-1);
	}
	*rv = v;
	return (0);
}

static __inline__ int
GetUintBinary(enum ply_format fmt, enum ply_prop_type type, FILE *_Nonnull f,
    Uint *_Nullable rv)
{
	union {
		Uint8 u8;
		Uint32 u16;
		Uint32 u32;
	} data;

	switch (type) {
	case PLY_INT8:
	case PLY_UINT8:
		if (fread(&data.u8, sizeof(Uint8), 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		if (rv != NULL) {
			*rv = (Uint)data.u8;
		}
		break;
	case PLY_INT16:
	case PLY_UINT16:
		if (fread(&data.u16, sizeof(Uint16), 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		if (rv != NULL) {
			*rv = (fmt == PLY_BIN_BE) ?
			    (Uint)AG_SwapBE16(data.u16) :
			    (Uint)AG_SwapLE16(data.u16);
		}
		break;
	case PLY_INT32:
	case PLY_UINT32:
		if (fread(&data.u32, sizeof(Uint32), 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		if (rv != NULL) {
			*rv = (fmt == PLY_BIN_BE) ?
			    (Uint)AG_SwapBE32(data.u32) :
		            (Uint)AG_SwapLE32(data.u32);
		}
		break;
	default:
		AG_SetError("Invalid int type: %d", type);
		return (-1);
	}
	return (0);
}

static __inline__ int
GetRealBinary(enum ply_format fmt, enum ply_prop_type type, FILE *_Nonnull f,
    M_Real *_Nonnull rv)
{
	union {
		Uint8 u8;
		Sint8 s8;
		Uint16 u16;
		Sint16 s16;
		Uint32 u32;
		Sint32 s32;
		float flt;
		double dbl;
	} data;

	switch (type) {
	case PLY_UINT8:
		if (fread(&data.u8, sizeof(Uint8), 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		*rv = (M_Real)data.u8;
		break;
	case PLY_INT8:
		if (fread(&data.s8, sizeof(Sint8), 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		*rv = (M_Real)data.s8;
		break;
	case PLY_UINT16:
		if (fread(&data.u16, sizeof(Uint16), 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		*rv = (fmt == PLY_BIN_BE) ?
		    (M_Real)AG_SwapBE16(data.u16) :
		    (M_Real)AG_SwapLE16(data.u16);
		break;
	case PLY_INT16:
		if (fread(&data.s16, sizeof(Sint16), 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		*rv = (fmt == PLY_BIN_BE) ?
		    (M_Real)AG_SwapBE16(data.s16) :
		    (M_Real)AG_SwapLE16(data.s16);
		break;
	case PLY_UINT32:
		if (fread(&data.u32, sizeof(Uint32), 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		*rv = (fmt == PLY_BIN_BE) ?
		    (M_Real)AG_SwapBE32(data.u32) :
		    (M_Real)AG_SwapLE32(data.u32);
		break;
	case PLY_INT32:
		if (fread(&data.s32, sizeof(Sint32), 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		*rv = (fmt == PLY_BIN_BE) ?
		    (M_Real)AG_SwapBE32(data.s32) :
		    (M_Real)AG_SwapLE32(data.s32);
		break;
	case PLY_FLOAT32:
		if (fread(&data.flt, 4, 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		*rv = (M_Real)data.flt;
		break;
	case PLY_FLOAT64:
		if (fread(&data.dbl, 8, 1, f) < 1) {
			AG_SetError("Read error");
			return (-1);
		}
		*rv = (M_Real)data.dbl;
		break;
	default:
		AG_SetError("Invalid real type: %d", type);
		return (-1);
	}
	return (0);
}

static __inline__ void
SetVertexProps(SG_Vertex *_Nonnull v, PLY_Prop *_Nonnull prop, M_Real fv,
    Uint flags)
{
	struct ply_vertex_prop {
		enum ply_std_prop std;		/* PLY property */
		int inVec;			/* Is a vector? */
		void *p;			/* Address */
		int conv8;			/* Convert to 8-bit */
		Uint flags;
	} vProps[] = {
		{ PLY_X,     1,  &v->v.x,   0, 0 },
		{ PLY_Y,     1,  &v->v.y,   0, 0 },
		{ PLY_Z,     1,  &v->v.z,   0, 0 },
		{ PLY_NX,    1,  &v->n.x,   0, SG_PLY_LOAD_VTX_NORMALS },
		{ PLY_NY,    1,  &v->n.y,   0, SG_PLY_LOAD_VTX_NORMALS },
		{ PLY_NZ,    1,  &v->n.z,   0, SG_PLY_LOAD_VTX_NORMALS },
		{ PLY_U,     0,  &v->st.x,  0, SG_PLY_LOAD_TEXCOORDS },
		{ PLY_V,     0,  &v->st.y,  0, SG_PLY_LOAD_TEXCOORDS },
		{ PLY_RED,   1,  &v->c.r,   1, SG_PLY_LOAD_VTX_COLORS },
		{ PLY_GREEN, 1,  &v->c.g,   1, SG_PLY_LOAD_VTX_COLORS },
		{ PLY_BLUE,  1,  &v->c.b,   1, SG_PLY_LOAD_VTX_COLORS },
	};
	const int nprops = sizeof(vProps)/sizeof(vProps[0]);
	int i;

	for (i = 0; i < nprops; i++) {
		struct ply_vertex_prop *vp = &vProps[i];

		if ((vp->flags != 0 && (vp->flags & flags) == 0) ||
		    vp->std != prop->std) {
			continue;
		}
		if (vp->inVec) {
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
			*(float *)vp->p = vp->conv8 ? ((float)fv)/255.0f : fv;
#elif defined(DOUBLE_PRECISION)
			*(double *)vp->p = vp->conv8 ? ((double)fv)/255.0 : fv;
#elif defined(QUAD_PRECISION)
			*(long double *)vp->p = vp->conv8 ? ((long double)fv)/255.0L : fv;
#endif
		} else {
			*(M_Real *)vp->p = vp->conv8 ? fv/255.0 : fv;
		}
	}
}

/*
 * Insert a new facet. We don't check facets for validity or duplicates,
 * and we only support triangles and quads.
 */
static __inline__ int
InsertFacet(SG_Object *_Nonnull so, Uint *_Nonnull v, Uint nind,
    PLY_Element *_Nonnull elem, Uint *_Nonnull map, Uint mapSize, Uint flags)
{
	switch (nind) {
	case 3:
		if (v[0] >= mapSize || v[1] >= mapSize || v[2] >= mapSize) {
			AG_SetError("Bad refs in triangle [%u,%u,%u]",
			    v[0], v[1], v[2]);
			return (-1);
		}
		SG_FacetFromTri3(so, map[v[0]], map[v[1]], map[v[2]]);
		break;
	case 4:
		if (v[0] >= mapSize || v[1] >= mapSize ||
		    v[2] >= mapSize || v[3] >= mapSize) {
			AG_SetError("Bad refs in quad [%u,%u,%u,%u]",
			    v[0], v[1], v[2], v[3]);
			return (-1);
		}
		SG_FacetFromQuad4(so, map[v[0]], map[v[1]], map[v[2]],
		                      map[v[3]]);
		break;
	default:
		AG_SetError("%d-sided face!", nind);
		return (-1);
	}
	return (0);
}

/*
 * Insert a new vertex. If requested, we check for duplicates and remap
 * them, but that is extremely slow since no space partitioning is done.
 */
static __inline__ int
InsertVertex(SG_Object *_Nonnull so, const SG_Vertex *_Nonnull v, Uint i,
    Uint *_Nonnull vtxMap, Uint flags)
{
	Uint j;

	if (flags & SG_PLY_DUP_VERTICES) {
		for (j = 1; j < so->nVtx; j++) {
			SG_Vertex *ve = &so->vtx[j];

			if (ve->v.x != v->v.x ||
			    ve->v.y != v->v.y ||
			    ve->v.z != v->v.z) {
				continue;
			}
			ve->n = v->n;
			ve->c = v->c;
			ve->st = v->st;
			vtxMap[i] = j;		/* Map existing */
			break;
		}
	} else {
		j = so->nVtx;
	}
	if (j == so->nVtx) {
		SG_Vertex *vNew;

		if ((vNew = realloc(so->vtx, (so->nVtx+1)*sizeof(SG_Vertex))) 
		    == NULL) {
			AG_SetError("Out of memory for vertices");
			return (-1);
		}
		so->vtx = vNew;

		vNew = &so->vtx[so->nVtx];
		vNew->v = v->v;
		vNew->n = v->n;
		vNew->c = v->c;
		vNew->st = v->st;
		vNew->flags = 0;
		vtxMap[i] = so->nVtx++;	 		  /* Map new */
	}
	return (0);
}

static int
LoadASCII(SG_Object *_Nonnull so, PLY_Info *_Nonnull ply, FILE *_Nonnull f,
    Uint flags)
{
	char line[4096], *s, *c;
	Uint *vtxMap = NULL, mapSize = 0;
	PLY_Element *el;
	PLY_Prop *prop;
	Uint i;

	TAILQ_FOREACH(el, &ply->elements, elements) {
		switch (el->std) {
		case PLY_VERTEX:
			if (vtxMap != NULL) {
				AG_SetError("Multiple vertex elements");
				Free(vtxMap);
				goto fail;
			}
			mapSize = el->count;
			if ((vtxMap = malloc(mapSize*sizeof(Uint))) == NULL) {
				AG_SetError("Out of memory for vtxmap");
				goto fail;
			}
			break;
		case PLY_FACE:
			/*
			 * Optimize based on assumption that object has
			 * an Euler characteristic of 2.
			 */
			(void)SG_EdgeRehash(so, (Uint)el->count + so->nVtx - 2);
			(void)SG_FacetRehash(so, so->nEdgeTbl);
			break;
		default:
			break;
		}
		for (i = 0; i < el->count; i++) {
			SG_Vertex vTmp;
	
			vTmp.n = M_VecZero3();
			vTmp.st = M_VecZero2();
			vTmp.c.r = 0.5;
			vTmp.c.g = 0.5;
			vTmp.c.b = 0.5;

			if ((s = fgets(line, sizeof(line), f)) == NULL ||
			    (c = strchr(line, '\n')) == NULL) {
				AG_SetError("Premature end of PLY file");
				goto fail;
			}
			*c = '\0';
			TAILQ_FOREACH(prop, &el->props, props) {
				char *num = AG_Strsep(&s, " ");
				M_Real fv;
				Uint face[4], count = 0, j;

				if (prop->list) {
					if (GetUintASCII(num, &count) == -1) {
						goto fail;
					}
					if (el->std == PLY_FACE &&
					    prop->std == PLY_VERTEX_INDICES) {
						if (count > 4) {
							AG_SetError(
							  "%d-sided faces not"
							  " supported", count);
							goto fail;
						}
						for (j = 0; j < count; j++)
							if (GetUintASCII(
							    AG_Strsep(&s, " "),
							    &face[j]) == -1)
								goto fail;
					} else {
						/* Skip */
						for (j = 0; j < count; j++)
							AG_Strsep(&s, " ");
					}
				} else {
					if (GetRealASCII(num, &fv) == -1) {
						goto fail;
					}
					if (el->std == PLY_VERTEX)
						SetVertexProps(&vTmp, prop, fv,
						    flags);
				}
				if (el->std == PLY_FACE) {
					if (vtxMap == NULL) {
						AG_SetError("Face element "
						            "without vertices");
						goto fail;
					}
					if (InsertFacet(so, face, count, el,
					    vtxMap, mapSize, flags) == -1)
						goto fail;
				}
			}
			if (el->std == PLY_VERTEX)
				InsertVertex(so, &vTmp, i, vtxMap, flags);
		}
	}
	Free(vtxMap);
	return (0);
fail:
	Free(vtxMap);
	return (-1);
}

static int
LoadBinary(SG_Object *_Nonnull so, PLY_Info *_Nonnull ply, FILE *_Nonnull f,
    Uint flags)
{
	Uint *vtxMap = NULL, mapSize = 0;
	PLY_Element *el;
	PLY_Prop *prop;
	Uint i, j;

	TAILQ_FOREACH(el, &ply->elements, elements) {
		switch (el->std) {
		case PLY_VERTEX:
			if (vtxMap != NULL) {
				AG_SetError("Multiple vertex elements");
				Free(vtxMap);
				goto fail;
			}
			mapSize = el->count;
			if ((vtxMap = malloc(mapSize*sizeof(Uint))) == NULL) {
				AG_SetError("Out of memory for dupmap");
				goto fail;
			}
			break;
		case PLY_FACE:
			/*
			 * Optimize based on assumption that object has
			 * an Euler characteristic of 2.
			 */
			SG_EdgeRehash(so, (Uint)el->count + so->nVtx - 2);
			break;
		default:
			break;
		}
		for (i = 0; i < el->count; i++) {
			SG_Vertex vTmp;
	
			vTmp.n = M_VecZero3();
			vTmp.st = M_VecZero2();
			vTmp.c.r = 0.5;
			vTmp.c.g = 0.5;
			vTmp.c.b = 0.5;

			TAILQ_FOREACH(prop, &el->props, props) {
				Uint face[4], count = 0;
				M_Real fv;
		
				if (prop->list) {
					if (GetUintBinary(ply->format,
					    prop->list_type, f, &count) == -1) {
						goto fail;
					}
					if (el->std == PLY_FACE &&
					    prop->std == PLY_VERTEX_INDICES) {
						if (count > 4) {
							AG_SetError(
							  "%d-sided faces not"
							  " supported", count);
							goto fail;
						}
						for (j = 0; j < count; j++)
							if (GetUintBinary(
							    ply->format,
							    prop->type, f,
							    &face[j]) == -1)
								goto fail;
					} else {
						/* Skip */
						for (j = 0; j < count; j++)
							(void)GetUintBinary(
							    ply->format,
							    prop->type, f,
							    NULL);
					}
				} else {
					if (GetRealBinary(ply->format,
					    prop->type, f, &fv) == -1) {
						goto fail;
					}
					if (el->std == PLY_VERTEX)
						SetVertexProps(&vTmp, prop, fv,
						    flags);
				}
				if (el->std == PLY_FACE &&
				    prop->std == PLY_VERTEX_INDICES) {
					if (vtxMap == NULL) {
						AG_SetError("Face element "
						            "without vertices");
						goto fail;
					}
					if (InsertFacet(so, face, count, el,
					    vtxMap, mapSize, flags) == -1)
						goto fail;
				}
			}
			if (el->std == PLY_VERTEX)
				InsertVertex(so, &vTmp, i, vtxMap, flags);
		}
	}
	Free(vtxMap);
	return (0);
fail:
	Free(vtxMap);
	return (-1);
}

int
SG_ObjectLoadPLY(void *obj, const char *path, Uint flags)
{
	SG_Object *so = obj;
	PLY_Info ply;
	PLY_Element *cur_el = NULL;
	char line[4096], *s;
	char sig[4];
	FILE *f;
	int i, nline = 0;
	char *c;

	if ((f = fopen(path, "r")) == NULL) {
		AG_SetError("%s: %s", path, strerror(errno));
		return (-1);
	}
	if (fread(sig, sizeof(sig), 1, f) < 1 ||
	    strncmp(sig, "ply\n", 4) != 0) {
		AG_SetError("Not a Stanford PLY file");
		fclose(f);
		return (-1);
	}

	InitPLY(&ply);

	/* Load the PLY header information */
	for (nline = 0;
	     (nline < PLY_MAX_HEADER) &&
	     (s = fgets(line, sizeof(line), f)) != NULL;
	     nline++) {
		char *key, *v1, *v2, *v3, *v4;

		if ((c = strchr(s, '\n')) != NULL) {
			*c = '\0';
		} else {
			AG_SetError("Line too long");
			goto fail;
		}
		if (strcmp(line, "end_header") == 0) {
			break;
		}
		key = AG_Strsep(&s, " ");
		if (strcmp(key, "format") == 0) {
			if ((v1 = AG_Strsep(&s, " ")) == NULL) {
				AG_SetError("Bad format line");
				goto fail;
			}
			if (strcmp(v1, "ascii") == 0) {
				ply.format = PLY_ASCII;
			} else if (strcmp(v1, "binary_little_endian") == 0) {
				ply.format = PLY_BIN_LE;
			} else if (strcmp(v1, "binary_big_endian") == 0) {
				ply.format = PLY_BIN_BE;
			} else {
				AG_SetError("Unrecognized format `%s'", v1);
				goto fail;
			}
		} else if (strcmp(key, "element") == 0) {
			PLY_Element *el;
			Uint count;
			
			if ((v1 = AG_Strsep(&s, " ")) == NULL ||
			    (v2 = AG_Strsep(&s, " ")) == NULL) {
				AG_SetError("Bad element line");
				goto fail;
			}
			if (!IsValidElementName(v1)) {
				goto fail;
			}
			count = strtoul(v2, &c, 10);
			if (v2[0] == '\0' || *c != '\0') {
				AG_SetError("Bad element count `%s'", v2);
				goto fail;
			}

			el = Malloc(sizeof(PLY_Element));
			Strlcpy(el->name, v1, sizeof(el->name));
			el->count = count;
			TAILQ_INIT(&el->props);
			TAILQ_INSERT_TAIL(&ply.elements, el, elements);
			cur_el = el;

			for (i = 0; i < PLY_STD_ELEMENT_LAST; i++) {
				if (strcmp(el->name, plyStdElementNames[i])
				    == 0)
					break;
			}
			el->std = (enum ply_std_element)i;
		} else if (strcmp(key, "property") == 0) {
			PLY_Prop *prop;
			
			if (cur_el == NULL) {
				AG_SetError("Properties must follow elements");
				goto fail;
			}
			if ((v1 = AG_Strsep(&s, " ")) == NULL ||
			    (v2 = AG_Strsep(&s, " ")) == NULL) {
				AG_SetError("Bad property line");
				goto fail;
			}

			prop = Malloc(sizeof(PLY_Prop));
			if (strcmp(v1, "list") == 0) {
				if ((v3 = AG_Strsep(&s, " ")) == NULL ||
				    (v4 = AG_Strsep(&s, " ")) == NULL) {
					AG_SetError("Bad property list line");
					Free(prop);
					goto fail;
				}
				if ((prop->list_type = GetType(v2)) == PLY_LAST_TYPE ||
				    (prop->type = GetType(v3)) == PLY_LAST_TYPE ||
				    !IsValidElementName(v4)) {
					Free(prop);
					goto fail;
				}
				Strlcpy(prop->name, v4, sizeof(prop->name));
				prop->list = 1;
			} else {
				if ((prop->type = GetType(v1)) == PLY_LAST_TYPE ||
				    !IsValidElementName(v2)) {
					Free(prop);
					goto fail;
				}
				Strlcpy(prop->name, v2, sizeof(prop->name));
				prop->list = 0;
			}
			for (i = 0; i < PLY_STD_PROP_LAST; i++) {
				if (strcmp(prop->name, plyStdPropNames[i]) == 0)
					break;
			}
			prop->std = (enum ply_std_prop)i;
			TAILQ_INSERT_TAIL(&cur_el->props, prop, props);
		} 
	}

	AG_ObjectLock(so);
	switch (ply.format) {
	case PLY_ASCII:
		if (LoadASCII(so, &ply, f, flags) == -1) {
			AG_ObjectUnlock(so);
			goto fail;
		}
		break;
	case PLY_BIN_BE:
	case PLY_BIN_LE:
		if (LoadBinary(so, &ply, f, flags) == -1) {
			AG_ObjectUnlock(so);
			goto fail;
		}
		break;
	}
	AG_ObjectUnlock(so);

	fclose(f);
	FreePLY(&ply);
	return (0);
fail:
	fclose(f);
	FreePLY(&ply);
	return (-1);
}
