/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include <map/map.h>

#include <gui/window.h>
#include <gui/label.h>

#include "actor.h"

static void
Init(void *obj)
{
	MAP_Actor *a = obj;

	AG_ObjectRemain(a, AG_OBJECT_REMAIN_DATA);
	AG_MutexInitRecursive(&a->lock);
	a->flags = 0;
	a->parent = NULL;
	a->type = AG_ACTOR_MAP;
	a->g_map.x = 0;
	a->g_map.y = 0;
	a->g_map.l0 = 0;
	a->g_map.l1 = 0;
	a->g_map.x0 = 0;
	a->g_map.y0 = 0;
	a->g_map.x1 = 0;
	a->g_map.y1 = 0;
	a->g_map.xmot = 0;
	a->g_map.ymot = 0;
}

static void
Destroy(void *obj)
{
	MAP_Actor *a = obj;

	AG_MutexDestroy(&a->lock);
}

static int
Load(void *obj, AG_DataSource *buf, const AG_Version *ver)
{
	MAP_Actor *a = obj;
#if 0
	MAP *m;
#endif
	AG_MutexLock(&a->lock);
#if 0
	if (a->parent != NULL) {
		dprintf("reattaching %s to %s\n", OBJECT(a)->name,
		    OBJECT(a->parent)->name);
		m = a->parent;
		AG_MutexLock(&m->lock);
		MAP_DetachActor(m, a);
	} else {
		m = NULL;
	}
#endif

	a->type = (enum map_actor_type)AG_ReadUint32(buf);
	a->flags = (int)AG_ReadUint32(buf) & AG_ACTOR_SAVED_FLAGS;
	switch (a->type) {
	case AG_ACTOR_MAP:
		a->g_map.x = (int)AG_ReadUint32(buf);
		a->g_map.y = (int)AG_ReadUint32(buf);
		a->g_map.l0 = (int)AG_ReadUint8(buf);
		a->g_map.l1 = (int)AG_ReadUint8(buf);
		a->g_map.x0 = a->g_map.x;
		a->g_map.y0 = a->g_map.y;
		a->g_map.x1 = a->g_map.x;
		a->g_map.y1 = a->g_map.y;
		break;
	case AG_ACTOR_SCENE:
		a->g_scene.x = AG_ReadDouble(buf);
		a->g_scene.y = AG_ReadDouble(buf);
		a->g_scene.z = AG_ReadDouble(buf);
		a->g_scene.dx = AG_ReadDouble(buf);
		a->g_scene.dy = AG_ReadDouble(buf);
		a->g_scene.dz = AG_ReadDouble(buf);
		break;
	default:
		break;
	}
#if 0
	if (m != NULL) {
		MAP_DetachActor(m, a);
		dprintf("reattached %s to %s\n", OBJECT(a)->name,
		    OBJECT(m)->name);
		AG_MutexUnlock(&m->lock);
	}
#endif
	AG_MutexUnlock(&a->lock);
	return (0);
}

static int
Save(void *obj, AG_DataSource *buf)
{
	MAP_Actor *a = obj;

	AG_MutexLock(&a->lock);
	AG_WriteUint32(buf, (Uint32)a->type);
	AG_WriteUint32(buf, (Uint32)a->flags & AG_ACTOR_SAVED_FLAGS);

	switch (a->type) {
	case AG_ACTOR_MAP:
		AG_WriteUint32(buf, (Uint32)a->g_map.x);
		AG_WriteUint32(buf, (Uint32)a->g_map.y);
		AG_WriteUint8(buf, (Uint8)a->g_map.l0);
		AG_WriteUint8(buf, (Uint8)a->g_map.l1);
		break;
	case AG_ACTOR_SCENE:
		AG_WriteDouble(buf, a->g_scene.x);
		AG_WriteDouble(buf, a->g_scene.y);
		AG_WriteDouble(buf, a->g_scene.z);
		AG_WriteDouble(buf, a->g_scene.dx);
		AG_WriteDouble(buf, a->g_scene.dy);
		AG_WriteDouble(buf, a->g_scene.dz);
		break;
	default:
		break;
	}
	AG_MutexUnlock(&a->lock);
	return (0);
}

void
MAP_ActorUpdate(void *obj)
{
}

static void
MoveNodes(MAP_Actor *a, int xo, int yo)
{
	MAP *m = a->parent;
	int x, y;

	for (y = a->g_map.y0; y <= a->g_map.y1; y++) {
		for (x = a->g_map.x0; x <= a->g_map.x1; x++) {
			MAP_Node *n1 = &m->map[y][x];
			MAP_Node *n2 = &m->map[y+yo][x+xo];
			MAP_Item *r, *nr;

			for (r = TAILQ_FIRST(&n1->nrefs);
			     r != TAILQ_END(&n1->nrefs);
			     r = nr) {
				nr = TAILQ_NEXT(r, nrefs);
				if (r->p == a &&
				    r->layer >= a->g_map.l0 &&
				    r->layer <= a->g_map.l1) {
					TAILQ_REMOVE(&n1->nrefs, r, nrefs);
					TAILQ_INSERT_TAIL(&n2->nrefs, r, nrefs);
					r->r_gfx.xmotion = a->g_map.xmot;
					r->r_gfx.ymotion = a->g_map.ymot;
					break;
				}
			}
		}
	}
	a->g_map.x += xo;
	a->g_map.y += yo;
	a->g_map.x0 += xo;
	a->g_map.y0 += yo;
	a->g_map.x1 += xo;
	a->g_map.y1 += yo;
}

void
MAP_ActorMoveTiles(void *obj, int xo, int yo)
{
	MAP_Actor *a = obj;
	MAP *m = a->parent;
	int x, y;

	AG_MutexLock(&a->lock);

	a->g_map.xmot += xo;
	a->g_map.ymot += yo;

	for (y = a->g_map.y0; y <= a->g_map.y1; y++) {
		for (x = a->g_map.x0; x <= a->g_map.x1; x++) {
			MAP_Node *node = &m->map[y][x];
			MAP_Item *r;
		
			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if (r->p != a ||
				    r->layer < a->g_map.l0 ||
				    r->layer > a->g_map.l1) {
					continue;
				}

				r->r_gfx.xmotion = a->g_map.xmot;
				r->r_gfx.ymotion = a->g_map.ymot;
					
				switch (a->g_map.da) {
				case 0:
					if (a->g_map.xmot < -MAPTILESZ/2) {
						a->g_map.xmot = +MAPTILESZ/2;
						MoveNodes(a, -1, 0);
						goto out;
					}
					break;
				case 90:
					if (a->g_map.ymot < -MAPTILESZ/2) {
						a->g_map.ymot = +MAPTILESZ/2;
						MoveNodes(a, 0, -1);
						goto out;
					}
					break;
				case 180:
					if (a->g_map.xmot > +MAPTILESZ/2) {
						a->g_map.xmot = -MAPTILESZ/2;
						MoveNodes(a, +1, 0);
						goto out;
					}
					break;
				case 270:
					if (a->g_map.ymot > +MAPTILESZ/2) {
						a->g_map.ymot = -MAPTILESZ/2;
						MoveNodes(a, 0, +1);
						goto out;
					}
					break;
				}
			}
		}
	}
out:
	AG_MutexUnlock(&a->lock);
}

int
MAP_ActorSetTile(void *obj, int x, int y, int l0, RG_Tileset *ts,
    const char *name)
{
	MAP_Actor *a = obj;

	MAP_ActorUnmapTiles(a);
	return (MAP_ActorMapTiles(a, x, y, l0, ts, name));
}

int
MAP_ActorMapTiles(void *obj, int X0, int Y0, int L0, RG_Tileset *ts,
    const char *name)
{
	MAP_Actor *a = obj;
	MAP *m = a->parent;
	RG_Tile *tile;
	int x = a->g_map.x + X0;
	int y = a->g_map.y + Y0;
	int l0 = a->g_map.l0 + L0, l;
	int sx, sy, dx, dy;
	int dx0, dy0, xorig, yorig;
	int n = 0;

	if ((tile = RG_TilesetFindTile(ts, name)) == NULL) {
		return (-1);
	}
	dx0 = x - tile->xOrig/MAPTILESZ;
	dy0 = y - tile->yOrig/MAPTILESZ;
	xorig = tile->xOrig%MAPTILESZ;
	yorig = tile->yOrig%MAPTILESZ;

	for (sy = 0, dy = dy0;
	     sy < tile->su->h;
	     sy += MAPTILESZ, dy++) {
		for (sx = 0, dx = dx0;
		     sx < tile->su->w;
		     sx += MAPTILESZ, dx++) {
			MAP_Node *dn;
			MAP_Item *r;

			if (dx < 0 || dx >= m->mapw ||
			    dy < 0 || dy >= m->maph) {
				goto out;
			}
			dn = &m->map[dy][dx];

			r = MAP_NodeAddTile(m, dn, ts, tile->main_id);
			r->p = obj;
			r->r_gfx.rs.x = sx;
			r->r_gfx.rs.y = sy;
			r->r_gfx.rs.w = MAPTILESZ;
			r->r_gfx.rs.h = MAPTILESZ;
			r->r_gfx.xorigin = xorig;
			r->r_gfx.yorigin = yorig;
			r->r_gfx.xcenter = MAPTILESZ/2;
			r->r_gfx.ycenter = MAPTILESZ/2;
			r->r_gfx.xmotion = a->g_map.xmot;
			r->r_gfx.ymotion = a->g_map.ymot;
			r->flags |= tile->attrs[n];
			r->flags |= MAP_ITEM_NOSAVE;

			l = l0 + tile->layers[n];
			if (l < 0) {
				l = 0;
			} else {
				while (m->nlayers <= l)
					MAP_PushLayer(m, "");
			}
			r->layer = l;
	
			if (dx < a->g_map.x0) { a->g_map.x0 = dx; }
			else if (dx > a->g_map.x1) { a->g_map.x1 = dx; }
			if (dy < a->g_map.y0) { a->g_map.y0 = dy; }
			else if (dy > a->g_map.y1) { a->g_map.y1 = dy; }
			if (l < a->g_map.l0) { a->g_map.l0 = l; }
			else if (l > a->g_map.l1) { a->g_map.l1 = l; }
out:
			n++;
		}
	}
	return (0);
}

void
MAP_ActorUnmapTiles(void *obj)
{
	MAP_Actor *a = obj;
	MAP *m = a->parent;
	int x, y;

	if (m == NULL)
		return;

	for (y = a->g_map.y0; y <= a->g_map.y1; y++) {
		for (x = a->g_map.x0; x <= a->g_map.x1; x++) {
			MAP_Node *node = &m->map[y][x];
			MAP_Item *r, *nr;
		
			for (r = TAILQ_FIRST(&node->nrefs);
			     r != TAILQ_END(&node->nrefs);
			     r = nr) {
				nr = TAILQ_NEXT(r, nrefs);
				if (r->p == a &&
				    r->layer >= a->g_map.l0 &&
				    r->layer <= a->g_map.l1)
					MAP_NodeDelItem(m, node, r);
			}
		}
	}
}

#ifdef EDITION
static void *
Edit(void *p)
{
	MAP_Actor *a = p;
	AG_Window *win;
	
	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Actor: %s"), OBJECT(a)->name);
	AG_WindowSetPosition(win, AG_WINDOW_LOWER_LEFT, 1);

	AG_LabelNewPolled(win, 0, _("Type: %d"), &a->type);
	AG_LabelNewPolled(win, 0, _("Flags: 0x%x"), &a->flags);
	AG_LabelNewPolled(win, 0, _("Map layers: %d-%d"),
	    &a->g_map.l0, &a->g_map.l1);
	AG_LabelNewPolled(win, 0, _("Map position: [%d,%d]"),
	    &a->g_map.x, &a->g_map.y);
	AG_LabelNewPolled(win, 0, _("Map extent: [%d,%d]-[%d,%d]"),
	    &a->g_map.x0, &a->g_map.y0, &a->g_map.x1, &a->g_map.y1);
	AG_LabelNewPolled(win, 0, _("Map offset: [%d,%d]"),
	    &a->g_map.xmot, &a->g_map.ymot);
	AG_LabelNewPolled(win, 0, _("Direction: %d(%d)"),
	    &a->g_map.da, &a->g_map.dv);

	return (win);
}
#endif /* EDITION */

const AG_ObjectClass mapActorClass = {
	"MAP_Actor",
	sizeof(MAP_Actor),
	{ 0, 0 },
	Init,
	NULL,		/* reinit */
	Destroy,
	Load,
	Save,
#ifdef EDITION
	Edit
#else
	NULL
#endif
};
