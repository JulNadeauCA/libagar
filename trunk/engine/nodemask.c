/*	$Csoft: nodemask.c,v 1.17 2004/01/03 04:25:04 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

#include <engine/map.h>
#include <engine/view.h>

#include <string.h>

struct nodemask *
nodemask_new(enum nodemask_type type)
{
	struct nodemask *mask;

	mask = Malloc(sizeof(struct nodemask));
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
	case NODEMASK_TRIANGLE:
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
	case NODEMASK_TRIANGLE:
	case NODEMASK_RECTANGLE:
	case NODEMASK_POLYGON:
		Free(mask->nm_poly.vertices);
		break;
	}
	free(mask);
}

int
nodemask_load(struct map *m, struct netbuf *buf, struct nodemask *mask)
{
	struct object *obj;
	Uint32 i, objref;

	mask->type = read_uint8(buf);
	mask->scale = (int)read_sint16(buf);

	switch (mask->type) {
	case NODEMASK_BITMAP:
		objref = read_uint32(buf);
		if ((mask->nm_bitmap.obj = object_find_dep(m, objref))
		    == NULL) {
			error_set(_("Cannot resolve object: %u."), objref);
			return (-1);
		}
		mask->nm_bitmap.offs = read_uint32(buf);
		object_add_dep(m, mask->nm_bitmap.obj);
		if (object_page_in(mask->nm_bitmap.obj, OBJECT_GFX) == -1) {
			return (-1);
		}
		break;
	case NODEMASK_POLYGON:
	case NODEMASK_TRIANGLE:
	case NODEMASK_RECTANGLE:
		Free(mask->nm_poly.vertices);
		mask->nm_poly.nvertices = read_uint32(buf);
		mask->nm_poly.vertices = Malloc(mask->nm_poly.nvertices *
		    sizeof(int));
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
		write_uint32(buf, object_dep_index(m, mask->nm_bitmap.obj));
		write_uint32(buf, mask->nm_bitmap.offs);
		break;
	case NODEMASK_POLYGON:
	case NODEMASK_TRIANGLE:
	case NODEMASK_RECTANGLE:
		write_uint32(buf, mask->nm_poly.nvertices);
		for (i = 0; i < mask->nm_poly.nvertices; i++) {
			write_uint32(buf, mask->nm_poly.vertices[i]);
		}
		break;
	}
}

void
nodemask_copy(const struct nodemask *smask, struct map *dm,
    struct nodemask *dmask)
{
	dmask->type = smask->type;
	dmask->scale = smask->scale;

	switch (smask->type) {
	case NODEMASK_BITMAP:
		dmask->nm_bitmap.obj = smask->nm_bitmap.obj;
		dmask->nm_bitmap.offs = smask->nm_bitmap.offs;
		object_add_dep(dm, dmask->nm_bitmap.obj);
		object_page_in(dmask->nm_bitmap.obj, OBJECT_GFX);
		break;
	case NODEMASK_POLYGON:
	case NODEMASK_TRIANGLE:
	case NODEMASK_RECTANGLE:
		Free(dmask->nm_poly.vertices);
		dmask->nm_poly.nvertices = smask->nm_poly.nvertices;
		dmask->nm_poly.vertices = Malloc(smask->nm_poly.nvertices *
		    sizeof(Uint32));
		memcpy(dmask->nm_poly.vertices, smask->nm_poly.vertices,
		    smask->nm_poly.nvertices * sizeof(Uint32));
		break;
	}
}

int
nodemask_intersect(const struct nodemask *m1, const struct nodemask *m2)
{
	/* TODO */
	return (0);
}
