/*
 * Copyright (c) 2006-2007 Hypertriton, Inc
 * <http://www.hypertriton.com/>
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

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <agar/core/core.h>
#include <agar/sg/sg.h>
#include "ply.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define PLY_MAX_HEADER		256
#define PLY_MAX_ELEMENT_NAME	32
#define PLY_MAX_PROP_NAME	32
#define PLY_MAX_LIST_ITEMS	128
enum ply_prop_type {
	SG_PLY_INT8,
	SG_PLY_INT16,
	SG_PLY_INT32,
	SG_PLY_UINT8,
	SG_PLY_UINT16,
	SG_PLY_UINT32,
	SG_PLY_FLOAT32,
	SG_PLY_FLOAT64,
	SG_PLY_LAST_TYPE
} type;

struct ply_prop {
	char name[PLY_MAX_PROP_NAME];
	enum ply_prop_type type;		/* Type of item */
	int list;				/* Is a variable-sized list */
	enum ply_prop_type list_type;		/* Type of list count number */
	TAILQ_ENTRY(ply_prop) props;
};

struct ply_element {
	char name[PLY_MAX_ELEMENT_NAME];	/* Element identifier */
	Ulong count;				/* Number of instances */
	TAILQ_HEAD(,ply_prop) props;		/* Properties per instance */
	TAILQ_ENTRY(ply_element) elements;
};

struct ply_info {
	enum ply_format {
		PLY_ASCII,
		PLY_BIN_BE,
		PLY_BIN_LE
	} format;
	TAILQ_HEAD(,ply_element) elements;
};

static const char *ply_prop_names[] = {
	"int8", "int16", "int32", "uint8", "uint16", "uint32",
	"float32", "float64"
};
static const char *ply_prop_names_legacy[] = {
	"char", "short", "int", "uchar", "ushort", "uint",
	"float", "double"
};

static void
PLY_Init(struct ply_info *ply)
{
	ply->format = PLY_ASCII;
	TAILQ_INIT(&ply->elements);
}

static void
PLY_Free(struct ply_info *ply)
{
	struct ply_element *el, *el2;
	struct ply_prop *prop, *nprop;

	for (el = TAILQ_FIRST(&ply->elements);
	     el != TAILQ_END(&ply->elements);
	     el = el2) {
		el2 = TAILQ_NEXT(el, elements);

		for (prop = TAILQ_FIRST(&el->props);
		     prop != TAILQ_END(&el->props);
		     prop = nprop) {
			nprop = TAILQ_NEXT(prop, props);
			Free(prop, M_SG);
		}
		Free(el, M_SG);
	}
}

static int
PLY_NameOK(const char *s)
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

static int
PLY_GetType(const char *name)
{
	int i;

	for (i = 0; i < SG_PLY_LAST_TYPE; i++) {
		if (strcmp(name, ply_prop_names[i]) == 0 ||
		    strcmp(name, ply_prop_names_legacy[i]) == 0) {
			return (i);
		}
	}
	AG_SetError("Bad prop type `%s'", name);
	return (-1);
}

static __inline__ int
PLY_GetUintASCII(const char *s, Uint *rv)
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
PLY_GetRealASCII(const char *s, SG_Real *rv)
{
	SG_Real v;
	char *c;

	v = (SG_Real)strtod(s, &c);
	if (s[0] == '\0' || *c != '\0') {
		AG_SetError("Malformed real `%s'", s);
		return (-1);
	}
	*rv = v;
	return (0);
}

static void
PLY_SetVertexProps(SG_Vertex *v, struct ply_prop *prop, SG_Real fv)
{
	struct {
		const char *name;
		SG_Real *p;
		int conv8;
	} vprops[] = {
		{ "x", &v->v.x, 0 },
		{ "y", &v->v.y, 0 },
		{ "z", &v->v.z, 0 },
		{ "nx", &v->n.x, 0 },
		{ "ny", &v->n.y, 0 },
		{ "nz", &v->n.z, 0 },
		{ "s", &v->s, 0 },
		{ "t", &v->t, 0 },
		{ "red", &v->c.r, 1 },
		{ "green", &v->c.g, 1 },
		{ "blue", &v->c.b, 1 },
	};
	const int nprops = sizeof(vprops)/sizeof(vprops[0]);
	int i;
	
	for (i = 0; i < nprops; i++) {
		if (strcmp(vprops[i].name, prop->name) == 0) {
			if (vprops[i].conv8) {
				*(vprops[i].p) = ((SG_Real)fv)/255.0;
				continue;
			}
			*(vprops[i].p) = fv;
		}
	}
}

static __inline__ int
PLY_InsertFace(SG_Object *so, Uint *vind, Uint nind, Uint *map)
{
	switch (nind) {
	case 4:
		SG_FacetFromQuad4(so, map[vind[0]], map[vind[1]], map[vind[2]],
		                      map[vind[3]]);
		break;
	case 3:
		SG_FacetFromTri3(so, map[vind[0]], map[vind[1]], map[vind[2]]);
		break;
	default:
		AG_SetError("Bad facet %d", nind);
		return (-1);
	}
	return (0);
}

static int
PLY_LoadASCII(SG_Object *so, struct ply_info *ply, FILE *f)
{
	char line[4096], *s, *c;
	struct ply_element *el;
	struct ply_prop *prop;
	Uint *map = NULL;
	Ulong i;

	/*
	 * Load the data into the object. Use an array to remap vertex names
	 * since the file might contain duplicate vertices.
	 */
	TAILQ_FOREACH(el, &ply->elements, elements) {
		if (strcmp(el->name, "vertex") == 0) {
			if (map != NULL) {
				AG_SetError("Multiple `vertex' elements");
				Free(map, M_SG);
				goto fail;
			}
			map = Malloc(el->count*sizeof(long), M_SG);
		} else if (strcmp(el->name, "face") == 0) {
			/*
			 * Optimize based on assumption that object has
			 * an Euler characteristic of 2.
			 */
			SG_EdgeRehash(so, (Uint)el->count + so->nvtx - 2);
		}
		for (i = 0; i < el->count; i++) {
			SG_Vertex vTmp;

			if ((s = fgets(line, sizeof(line), f)) == NULL ||
			    (c = strchr(line, '\n')) == NULL) {
				AG_SetError("Premature end of PLY file");
				goto fail;
			}
			*c = '\0';
			TAILQ_FOREACH(prop, &el->props, props) {
				char *num = strsep(&s, " ");
				SG_Real fv;
				Uint vind[4], nind = 0, j;

				if (prop->list) {
					if (PLY_GetUintASCII(num, &nind) == -1)
						goto fail;
					if (nind > 4) {
						AG_SetError("Polygon facet");
						goto fail;
					}
					for (j = 0; j < nind; j++) {
						if (PLY_GetUintASCII(
						    strsep(&s, " "),
						    &vind[j]) == -1)
							goto fail;
					}
				} else {
					if (PLY_GetRealASCII(num, &fv) == -1) {
						goto fail;
					}
					if (strcmp(el->name, "vertex") == 0) {
						PLY_SetVertexProps(&vTmp,
						    prop, fv);
					}
				}
				if (strcmp(el->name, "face") == 0 &&
				    PLY_InsertFace(so, vind, nind, map) == -1)
					goto fail;
			}
			if (strcmp(el->name, "vertex") == 0) {
				Uint j;
#if 1
				/* 
				 * Find and remap duplicate vertices.
				 * XXX extremely expensive; we should allow for
				 * skipping if we know the input file has no
				 * duplicate vertices.
				 */
				for (j = 1; j < so->nvtx; j++) {
					SG_Vertex *ve = &so->vtx[j];

					if (ve->v.x == vTmp.v.x &&
					    ve->v.y == vTmp.v.y &&
					    ve->v.z == vTmp.v.z) {
						ve->n = vTmp.n;
						ve->c = vTmp.c;
						ve->s = vTmp.s;
						ve->t = vTmp.t;
						map[i] = j;   /* Map existing */
						break;
					}
				}
#else
				j = so->nvtx;
#endif
				if (j == so->nvtx) {
					SG_Vertex *vNew;

					so->vtx = Realloc(so->vtx,
					    (so->nvtx+1)*sizeof(SG_Vertex));
					vNew = &so->vtx[so->nvtx];
					vNew->v = vTmp.v;
					vNew->n = vTmp.n;
					vNew->c = vTmp.c;
					vNew->s = vTmp.s;
					vNew->t = vTmp.t;
					vNew->flags = 0;
					map[i] = so->nvtx++;	   /* Map new */
				}
			}
		}
	}
	Free(map, M_SG);
	return (0);
fail:
	Free(map, M_SG);
	return (-1);
}

static int
PLY_LoadBinary(SG_Object *so, struct ply_info *ply, FILE *f)
{
	AG_SetError("Unimplemented format %d", ply->format);
	return (-1);
}

int
SG_ObjectLoadPLY(void *obj, const char *path)
{
	SG_Object *so = obj;
	struct ply_info ply;
	struct ply_element *cur_el = NULL;
	char line[4096], *s;
	char sig[4];
	FILE *f;
	int nline = 0;
	char *c;

	if ((f = fopen(path, "r")) == NULL) {
		AG_SetError("%s: %s", path, strerror(errno));
		return (-1);
	}
	if (fread(sig, sizeof(sig), 1, f) < 1 ||
	    strncmp(sig, "ply\n", 4) != 0) {
		AG_SetError("Bad PLY signature");
		fclose(f);
		return (-1);
	}

	PLY_Init(&ply);

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
			struct ply_element *el;
			Ulong count;
			
			if ((v1 = AG_Strsep(&s, " ")) == NULL ||
			    (v2 = AG_Strsep(&s, " ")) == NULL) {
				AG_SetError("Bad element line");
				goto fail;
			}
			if (!PLY_NameOK(v1)) {
				goto fail;
			}
			count = strtoul(v2, &c, 10);
			if (v2[0] == '\0' || *c != '\0' || count < 0) {
				AG_SetError("Bad element count `%s'", v2);
				goto fail;
			}

			el = Malloc(sizeof(struct ply_element), M_SG);
			strlcpy(el->name, v1, sizeof(el->name));
			el->count = count;
			TAILQ_INIT(&el->props);
			TAILQ_INSERT_TAIL(&ply.elements, el, elements);
			cur_el = el;
		} else if (strcmp(key, "property") == 0) {
			struct ply_prop *prop;
			int i;
			
			if (cur_el == NULL) {
				AG_SetError("Properties must follow elements");
				goto fail;
			}
			if ((v1 = AG_Strsep(&s, " ")) == NULL ||
			    (v2 = AG_Strsep(&s, " ")) == NULL) {
				AG_SetError("Bad property line");
				goto fail;
			}

			prop = Malloc(sizeof(struct ply_prop), M_SG);
			if (strcmp(v1, "list") == 0) {
				if ((v3 = AG_Strsep(&s, " ")) == NULL ||
				    (v4 = AG_Strsep(&s, " ")) == NULL) {
					AG_SetError("Bad property list line");
					Free(prop, M_SG);
					goto fail;
				}
				if ((prop->list_type = PLY_GetType(v2)) == -1 ||
				    (prop->type = PLY_GetType(v3)) == -1 ||
				    !PLY_NameOK(v4)) {
					Free(prop, M_SG);
					goto fail;
				}
				strlcpy(prop->name, v4, sizeof(prop->name));
				prop->list = 1;
			} else {
				if ((prop->type = PLY_GetType(v1)) == -1 ||
				    !PLY_NameOK(v2)) {
					Free(prop, M_SG);
					goto fail;
				}
				strlcpy(prop->name, v2, sizeof(prop->name));
				prop->list = 0;
			}
			TAILQ_INSERT_TAIL(&cur_el->props, prop, props);
		} 
	}

	switch (ply.format) {
	case PLY_ASCII:
		if (PLY_LoadASCII(so, &ply, f) == -1) { goto fail; }
		break;
	case PLY_BIN_BE:
	case PLY_BIN_LE:
		if (PLY_LoadBinary(so, &ply, f) == -1) { goto fail; }
		break;
	}

	fclose(f);
	PLY_Free(&ply);
	return (0);
fail:
	fclose(f);
	PLY_Free(&ply);
	return (-1);
}

#endif /* HAVE_OPENGL */
