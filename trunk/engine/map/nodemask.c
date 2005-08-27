/*	$Csoft: nodemask.c,v 1.4 2005/08/10 06:59:24 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>

#ifdef MAP

#include <engine/view.h>

#include "map.h"

#include <string.h>

struct nodemask *
nodemask_new(enum nodemask_type type)
{
	struct nodemask *mask;

	mask = Malloc(sizeof(struct nodemask), M_NODEMASK);
	nodemask_init(mask, type);
	return (mask);
}

void
nodemask_init(struct nodemask *mask, enum nodemask_type type)
{
	mask->type = type;
	mask->scale = 1;

	switch (type) {
	case NODEMASK_BITMAP:
		mask->nm_bitmap.obj = NULL;
		mask->nm_bitmap.offs = 0;
		break;
	case NODEMASK_POLYGON:
	case NODEMASK_RECTANGLE:
		mask->nm_poly.vertices = NULL;
		mask->nm_poly.nvertices = 0;
		break;
	}
}

void
nodemask_destroy(struct map *m, struct nodemask *mask)
{
	switch (mask->type) {
	case NODEMASK_BITMAP:
		if (mask->nm_bitmap.obj != NULL) {
			object_del_dep(m, mask->nm_bitmap.obj);
			object_page_out(mask->nm_bitmap.obj, OBJECT_GFX);
		}
		break;
	case NODEMASK_RECTANGLE:
	case NODEMASK_POLYGON:
		Free(mask->nm_poly.vertices, M_NODEMASK);
		break;
	}
	Free(mask, M_NODEMASK);
}

int
nodemask_load(struct map *m, struct netbuf *buf, struct nodemask *mask)
{
	struct object *obj;
	Uint32 i, objref, offs;
	void *pobj;

	mask->type = read_uint8(buf);
	mask->scale = (int)read_sint16(buf);

	switch (mask->type) {
	case NODEMASK_BITMAP:
		objref = read_uint32(buf);
		offs = read_uint32(buf);
		if (object_find_dep(m, objref, &pobj) == -1) {
			return (-1);
		}
		nodemask_bitmap(m, mask, pobj, offs);
		break;
	case NODEMASK_POLYGON:
	case NODEMASK_RECTANGLE:
		Free(mask->nm_poly.vertices, M_NODEMASK);
		mask->nm_poly.nvertices = read_uint32(buf);
		mask->nm_poly.vertices = Malloc(mask->nm_poly.nvertices *
		    sizeof(int), M_NODEMASK);
		for (i = 0; i < mask->nm_poly.nvertices; i++) {
			mask->nm_poly.vertices[i] = read_uint32(buf);
		}
		break;
	}
	return (0);
}

void
nodemask_save(struct map *m, struct netbuf *buf, const struct nodemask *mask)
{
	Uint32 i;

	write_uint8(buf, mask->type);
	write_sint16(buf, (Sint16)mask->scale);

	switch (mask->type) {
	case NODEMASK_BITMAP:
		write_uint32(buf, object_encode_name(m, mask->nm_bitmap.obj));
		write_uint32(buf, mask->nm_bitmap.offs);
		break;
	case NODEMASK_POLYGON:
	case NODEMASK_RECTANGLE:
		write_uint32(buf, mask->nm_poly.nvertices);
		for (i = 0; i < mask->nm_poly.nvertices; i++) {
			write_uint32(buf, mask->nm_poly.vertices[i]);
		}
		break;
	}
}

void
nodemask_copy(const struct nodemask *smask, struct map *m,
    struct nodemask *dmask)
{
	dmask->type = smask->type;
	dmask->scale = smask->scale;

	switch (smask->type) {
	case NODEMASK_BITMAP:
		nodemask_bitmap(m, dmask, smask->nm_bitmap.obj,
		    smask->nm_bitmap.offs);
		break;
	case NODEMASK_POLYGON:
	case NODEMASK_RECTANGLE:
		Free(dmask->nm_poly.vertices, M_NODEMASK);
		if (smask->nm_poly.vertices != NULL) {
			dmask->nm_poly.nvertices = smask->nm_poly.nvertices;
			dmask->nm_poly.vertices = Malloc(
			    smask->nm_poly.nvertices * sizeof(Uint32),
			    M_NODEMASK);
			memcpy(dmask->nm_poly.vertices, smask->nm_poly.vertices,
			    smask->nm_poly.nvertices * sizeof(Uint32));
		} else {
			dmask->nm_poly.nvertices = 0;
			dmask->nm_poly.vertices = NULL;
		}
		break;
	}
}

void
nodemask_bitmap(struct map *m, struct nodemask *mask, void *pobj, Uint32 offs)
{
	if (m != NULL && pobj != NULL) {
		object_add_dep(m, pobj);
	}
	if (pobj != NULL) {
		object_page_in(pobj, OBJECT_GFX);
	}
	mask->nm_bitmap.obj = pobj;
	mask->nm_bitmap.offs = offs;
}

void
nodemask_vertex(struct nodemask *mask, Uint32 x, Uint32 y)
{
	dprintf("%u,%u\n", x, y);

	if (mask->nm_poly.vertices == NULL) {
		mask->nm_poly.vertices = Malloc(2*sizeof(Uint32), M_NODEMASK);
		mask->nm_poly.nvertices = 0;
	} else {
		mask->nm_poly.vertices = Realloc(mask->nm_poly.vertices,
		    (mask->nm_poly.nvertices+2)*sizeof(Uint32));
	}

	mask->nm_poly.vertices[mask->nm_poly.nvertices++] = x;
	mask->nm_poly.vertices[mask->nm_poly.nvertices++] = y;
}

int
nodemask_intersect(const struct nodemask *m1, const struct nodemask *m2)
{
	/* TODO */
	return (0);
}

#endif /* MAP */
