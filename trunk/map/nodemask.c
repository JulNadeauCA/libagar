/*	$Csoft: nodemask.c,v 1.5 2005/08/27 04:34:06 vedge Exp $	*/

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

#include <agar/core/core.h>

#include "map.h"

#include <string.h>

MAP_NodeMask *
MAP_NodeMaskNew(enum map_nodemask_type type)
{
	MAP_NodeMask *mask;

	mask = Malloc(sizeof(MAP_NodeMask), M_NODEMASK);
	MAP_NodeMaskInit(mask, type);
	return (mask);
}

void
MAP_NodeMaskInit(MAP_NodeMask *mask, enum map_nodemask_type type)
{
	mask->type = type;
	mask->scale = 1;

	switch (type) {
	case AG_NODEMASK_BITMAP:
		mask->nm_bitmap.obj = NULL;
		mask->nm_bitmap.offs = 0;
		break;
	case AG_NODEMASK_POLYGON:
	case AG_NODEMASK_RECTANGLE:
		mask->nm_poly.vertices = NULL;
		mask->nm_poly.nvertices = 0;
		break;
	}
}

void
MAP_NodeMaskDestroy(MAP *m, MAP_NodeMask *mask)
{
	switch (mask->type) {
	case AG_NODEMASK_BITMAP:
		if (mask->nm_bitmap.obj != NULL) {
			AG_ObjectDelDep(m, mask->nm_bitmap.obj);
			AG_ObjectPageOut(mask->nm_bitmap.obj, AG_OBJECT_GFX);
		}
		break;
	case AG_NODEMASK_RECTANGLE:
	case AG_NODEMASK_POLYGON:
		Free(mask->nm_poly.vertices, M_NODEMASK);
		break;
	}
	Free(mask, M_NODEMASK);
}

int
MAP_NodeMaskLoad(MAP *m, AG_Netbuf *buf, MAP_NodeMask *mask)
{
	AG_Object *obj;
	Uint32 i, objref, offs;
	void *pobj;

	mask->type = AG_ReadUint8(buf);
	mask->scale = (int)AG_ReadSint16(buf);

	switch (mask->type) {
	case AG_NODEMASK_BITMAP:
		objref = AG_ReadUint32(buf);
		offs = AG_ReadUint32(buf);
		if (AG_ObjectFindDep(m, objref, &pobj) == -1) {
			return (-1);
		}
		MAP_NodeMaskBitmap(m, mask, pobj, offs);
		break;
	case AG_NODEMASK_POLYGON:
	case AG_NODEMASK_RECTANGLE:
		Free(mask->nm_poly.vertices, M_NODEMASK);
		mask->nm_poly.nvertices = AG_ReadUint32(buf);
		mask->nm_poly.vertices = Malloc(mask->nm_poly.nvertices *
		    sizeof(int), M_NODEMASK);
		for (i = 0; i < mask->nm_poly.nvertices; i++) {
			mask->nm_poly.vertices[i] = AG_ReadUint32(buf);
		}
		break;
	}
	return (0);
}

void
MAP_NodeMaskSave(MAP *m, AG_Netbuf *buf, const MAP_NodeMask *mask)
{
	Uint32 i;

	AG_WriteUint8(buf, mask->type);
	AG_WriteSint16(buf, (Sint16)mask->scale);

	switch (mask->type) {
	case AG_NODEMASK_BITMAP:
		AG_WriteUint32(buf, AG_ObjectEncodeName(m,
		    mask->nm_bitmap.obj));
		AG_WriteUint32(buf, mask->nm_bitmap.offs);
		break;
	case AG_NODEMASK_POLYGON:
	case AG_NODEMASK_RECTANGLE:
		AG_WriteUint32(buf, mask->nm_poly.nvertices);
		for (i = 0; i < mask->nm_poly.nvertices; i++) {
			AG_WriteUint32(buf, mask->nm_poly.vertices[i]);
		}
		break;
	}
}

void
MAP_NodeMaskCopy(const MAP_NodeMask *smask, MAP *m,
    MAP_NodeMask *dmask)
{
	dmask->type = smask->type;
	dmask->scale = smask->scale;

	switch (smask->type) {
	case AG_NODEMASK_BITMAP:
		MAP_NodeMaskBitmap(m, dmask, smask->nm_bitmap.obj,
		    smask->nm_bitmap.offs);
		break;
	case AG_NODEMASK_POLYGON:
	case AG_NODEMASK_RECTANGLE:
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
MAP_NodeMaskBitmap(MAP *m, MAP_NodeMask *mask, void *pobj, Uint32 offs)
{
	if (m != NULL && pobj != NULL) {
		AG_ObjectAddDep(m, pobj);
	}
	if (pobj != NULL) {
		AG_ObjectPageIn(pobj, AG_OBJECT_GFX);
	}
	mask->nm_bitmap.obj = pobj;
	mask->nm_bitmap.offs = offs;
}

void
MAP_NodeMaskVertex(MAP_NodeMask *mask, Uint32 x, Uint32 y)
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
MAP_NodeMaskIntersect(const MAP_NodeMask *m1, const MAP_NodeMask *m2)
{
	/* TODO */
	return (0);
}
