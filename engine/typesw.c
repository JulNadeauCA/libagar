/*	$Csoft: typesw.c,v 1.19 2005/04/14 06:19:36 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/typesw.h>

#include <engine/object.h>
#include <engine/perso.h>

#include <engine/map/map.h>
#include <engine/map/mapedit.h>
#include <engine/vg/drawing.h>
#include <engine/rg/tileset.h>

struct object_type *typesw = NULL;
int ntypesw = 0;

/* Initialize the type switch and register the built-in types. */
void
typesw_init(void)
{
	extern const struct object_ops object_ops, map_ops,
	    perso_ops, drawing_ops, tileset_ops;

	typesw = Malloc(sizeof(struct object_type), M_TYPESW);

	typesw_register("object", sizeof(struct object), &object_ops, OBJ_ICON);
	typesw_register("map", sizeof(struct map), &map_ops, MAP_ICON);
	typesw_register("perso", sizeof(struct perso), &perso_ops, PERSO_ICON);
	typesw_register("drawing", sizeof(struct drawing), &drawing_ops,
	    DRAWING_ICON);
	typesw_register("tileset", sizeof(struct tileset), &tileset_ops,
	    TILESET_ICON);
}

void
typesw_destroy(void)
{
	Free(typesw, M_TYPESW);
}

/* Register an object type. */
void
typesw_register(const char *type, size_t size, const struct object_ops *ops,
    int icon)
{
	struct object_type *ntype;

	typesw = Realloc(typesw, (ntypesw+1)*sizeof(struct object_type));
	ntype = &typesw[ntypesw++];
	strlcpy(ntype->type, type, sizeof(ntype->type));
	ntype->size = size;
	ntype->ops = ops;
	ntype->icon = icon;
}

