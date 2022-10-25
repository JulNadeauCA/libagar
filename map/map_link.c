/*
 * Copyright (c) 2021 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Link to another location on this or a different map.
 */

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/gui/primitive.h>
#include <agar/gui/icons.h>

#include <agar/map/map.h>

#include <string.h>
#include <stdlib.h>

/* Create a new link to a map location. */
MAP_Link *
MAP_LinkNew(MAP *m, MAP_Node *node, const char *map, int x, int y, Uint dir)
{
	MAP_Link *ml;

	ml = Malloc(sizeof(MAP_Link));
	MAP_ItemInit(ml, MAP_ITEM_LINK);
	ml->map = Strdup(map);
	ml->x = x;
	ml->y = y;
	ml->dir = dir;

	TAILQ_INSERT_TAIL(&node->items, MAPITEM(ml), items);
	return (ml);
}

static void
Init(void *mi)
{
	MAP_Link *ml = MAPLINK(mi);

	ml->map = NULL;
	ml->x = 0;
	ml->y = 0;
	ml->dir = 0;
}

static void
Destroy(void *mi)
{
	MAP_Link *ml = MAPLINK(mi);

	Free(ml->map);
}

static void *
Duplicate(MAP *map, MAP_Node *node, const void *mi)
{
	const MAP_Link *ml = MAPLINK(mi);

	return (void *)MAP_LinkNew(map, node, ml->map, ml->x, ml->y, ml->dir);
}

static int
Load(MAP *map, void *mi, AG_DataSource *ds)
{
	MAP_Link *ml = MAPLINK(mi);

	ml->map = AG_ReadStringLen(ds, MAP_LINK_ID_MAX);
	ml->x = (Uint)AG_ReadUint32(ds);
	ml->y = (Uint)AG_ReadUint32(ds);
	if (ml->x > MAP_WIDTH_MAX || ml->y > MAP_HEIGHT_MAX) {
		AG_SetErrorS("Bad link coordinates");
		return (-1);
	}
	ml->dir = (Uint)AG_ReadUint8(ds);
	return (0);
}

static void
Save(MAP *map, void *mi, AG_DataSource *ds)
{
	MAP_Link *ml = MAPLINK(mi);

	AG_WriteString(ds, ml->map);
	AG_WriteUint32(ds, (Uint32)ml->x);
	AG_WriteUint32(ds, (Uint32)ml->y);
	AG_WriteUint8(ds, ml->dir);
}

MAP_ItemClass mapLinkClass = {
	N_("Link"),
	"https://libagar.org/",
	N_("Reference to a map location"),
	MAP_ITEM_LINK,
	0,
	sizeof(MAP_Link),
	Init,
	Destroy,
	Duplicate,
	Load,
	Save,
	NULL,		/* draw */
	NULL,		/* extent */
	NULL,		/* collide */
	NULL		/* edit */
};
