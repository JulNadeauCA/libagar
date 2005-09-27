/*	$Csoft: actor.c,v 1.1 2005/09/20 13:46:29 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <engine/map/map.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "actor.h"

const AG_Version ag_actor_ver = {
	"agar actor",
	0, 0
};

void
AG_ActorInit(void *obj, const char *type, const char *name,
    const AG_ActorOps *ops)
{
	char tname[AG_OBJECT_TYPE_MAX];
	AG_Actor *go = obj;

	strlcpy(tname, "actor.", sizeof(tname));
	strlcat(tname, type, sizeof(tname));

	AG_ObjectInit(go, tname, name, ops);
	AG_ObjectRemain(go, AG_OBJECT_REMAIN_DATA);
	pthread_mutex_init(&go->lock, &agRecursiveMutexAttr);
	go->flags = 0;
	go->parent = NULL;
	go->type = AG_ACTOR_MAP;
	go->g_map.x = 0;
	go->g_map.y = 0;
	go->g_map.l0 = 0;
	go->g_map.l1 = 0;
	go->g_map.x0 = 0;
	go->g_map.y0 = 0;
	go->g_map.x1 = 0;
	go->g_map.y1 = 0;
	go->g_map.xmot = 0;
	go->g_map.ymot = 0;
}

void
AG_ActorReinit(void *obj)
{
}

void
AG_ActorDestroy(void *obj)
{
	AG_Actor *go = obj;

	pthread_mutex_destroy(&go->lock);
}

int
AG_ActorLoad(void *obj, AG_Netbuf *buf)
{
	AG_Actor *go = obj;
	AG_Space *space;

	if (AG_ReadVersion(buf, &ag_actor_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&go->lock);

#if 0
	if (go->parent != NULL) {
		dprintf("reattaching %s to %s\n", AGOBJECT(go)->name,
		    AGOBJECT(go->parent)->name);
		space = go->parent;
		pthread_mutex_lock(&space->lock);
		AG_SpaceDetach(space, go);
	} else {
		space = NULL;
	}
#endif

	go->type = (enum ag_actor_type)AG_ReadUint32(buf);
	go->flags = (int)AG_ReadUint32(buf) & AG_ACTOR_SAVED_FLAGS;
	switch (go->type) {
	case AG_ACTOR_MAP:
		go->g_map.x = (int)AG_ReadUint32(buf);
		go->g_map.y = (int)AG_ReadUint32(buf);
		go->g_map.l0 = (int)AG_ReadUint8(buf);
		go->g_map.l1 = (int)AG_ReadUint8(buf);
		go->g_map.x0 = go->g_map.x;
		go->g_map.y0 = go->g_map.y;
		go->g_map.x1 = go->g_map.x;
		go->g_map.y1 = go->g_map.y;
		break;
	case AG_ACTOR_SCENE:
		go->g_scene.x = AG_ReadDouble(buf);
		go->g_scene.y = AG_ReadDouble(buf);
		go->g_scene.z = AG_ReadDouble(buf);
		go->g_scene.dx = AG_ReadDouble(buf);
		go->g_scene.dy = AG_ReadDouble(buf);
		go->g_scene.dz = AG_ReadDouble(buf);
		break;
	default:
		break;
	}
#if 0
	if (space != NULL) {
		AG_SpaceAttach(space, go);
		dprintf("reattached %s to %s\n", AGOBJECT(go)->name,
		    AGOBJECT(go->parent)->name);
		pthread_mutex_unlock(&space->lock);
	}
#endif
	pthread_mutex_unlock(&go->lock);
	return (0);
}

int
AG_ActorSave(void *obj, AG_Netbuf *buf)
{
	AG_Actor *go = obj;

	AG_WriteVersion(buf, &ag_actor_ver);

	pthread_mutex_lock(&go->lock);
	AG_WriteUint32(buf, (Uint32)go->type);
	AG_WriteUint32(buf, (Uint32)go->flags & AG_ACTOR_SAVED_FLAGS);

	switch (go->type) {
	case AG_ACTOR_MAP:
		AG_WriteUint32(buf, (Uint32)go->g_map.x);
		AG_WriteUint32(buf, (Uint32)go->g_map.y);
		AG_WriteUint8(buf, (Uint8)go->g_map.l0);
		AG_WriteUint8(buf, (Uint8)go->g_map.l1);
		break;
	case AG_ACTOR_SCENE:
		AG_WriteDouble(buf, go->g_scene.x);
		AG_WriteDouble(buf, go->g_scene.y);
		AG_WriteDouble(buf, go->g_scene.z);
		AG_WriteDouble(buf, go->g_scene.dx);
		AG_WriteDouble(buf, go->g_scene.dy);
		AG_WriteDouble(buf, go->g_scene.dz);
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&go->lock);
	return (0);
}

void
AG_ActorUpdate(void *obj)
{
}

static void
move_nodes(AG_Actor *go, int xo, int yo)
{
	AG_Map *m = go->parent;
	int x, y;

	for (y = go->g_map.y0; y <= go->g_map.y1; y++) {
		for (x = go->g_map.x0; x <= go->g_map.x1; x++) {
			AG_Node *n1 = &m->map[y][x];
			AG_Node *n2 = &m->map[y+yo][x+xo];
			AG_Nitem *r, *nr;

			for (r = TAILQ_FIRST(&n1->nrefs);
			     r != TAILQ_END(&n1->nrefs);
			     r = nr) {
				nr = TAILQ_NEXT(r, nrefs);
				if (r->p == go &&
				    r->layer >= go->g_map.l0 &&
				    r->layer <= go->g_map.l1) {
					TAILQ_REMOVE(&n1->nrefs, r, nrefs);
					TAILQ_INSERT_TAIL(&n2->nrefs, r, nrefs);
					r->r_gfx.xmotion = go->g_map.xmot;
					r->r_gfx.ymotion = go->g_map.ymot;
					break;
				}
			}
		}
	}
	go->g_map.x += xo;
	go->g_map.y += yo;
	go->g_map.x0 += xo;
	go->g_map.y0 += yo;
	go->g_map.x1 += xo;
	go->g_map.y1 += yo;
}

void
AG_ActorMoveSprite(void *obj, int xo, int yo)
{
	AG_Actor *go = obj;
	AG_Map *m = go->parent;
	int x, y;

	pthread_mutex_lock(&go->lock);

	go->g_map.xmot += xo;
	go->g_map.ymot += yo;

	for (y = go->g_map.y0; y <= go->g_map.y1; y++) {
		for (x = go->g_map.x0; x <= go->g_map.x1; x++) {
			AG_Node *node = &m->map[y][x];
			AG_Nitem *r, *nr;
		
			TAILQ_FOREACH(r, &node->nrefs, nrefs) {
				if (r->p != go ||
				    r->layer < go->g_map.l0 ||
				    r->layer > go->g_map.l1) {
					continue;
				}

				r->r_gfx.xmotion = go->g_map.xmot;
				r->r_gfx.ymotion = go->g_map.ymot;
					
				switch (go->g_map.da) {
				case 0:
					if (go->g_map.xmot < -AGTILESZ/2) {
						go->g_map.xmot = +AGTILESZ/2;
						move_nodes(go, -1, 0);
						goto out;
					}
					break;
				case 90:
					if (go->g_map.ymot < -AGTILESZ/2) {
						go->g_map.ymot = +AGTILESZ/2;
						move_nodes(go, 0, -1);
						goto out;
					}
					break;
				case 180:
					if (go->g_map.xmot > +AGTILESZ/2) {
						go->g_map.xmot = -AGTILESZ/2;
						move_nodes(go, +1, 0);
						goto out;
					}
					break;
				case 270:
					if (go->g_map.ymot > +AGTILESZ/2) {
						go->g_map.ymot = -AGTILESZ/2;
						move_nodes(go, 0, +1);
						goto out;
					}
					break;
				}
			}
		}
	}
out:
	pthread_mutex_unlock(&go->lock);
}

int
AG_ActorSetSprite(void *obj, int x, int y, int l0, void *gfx_obj,
    const char *name)
{
	AG_Actor *go = obj;

	AG_ActorUnmapSprite(go);
	return (AG_ActorMapSprite(go, x, y, l0, gfx_obj, name));
}

int
AG_ActorMapSprite(void *obj, int X0, int Y0, int L0, void *gfx_obj,
    const char *name)
{
	AG_Actor *go = obj;
	AG_Map *m = go->parent;
	AG_Gfx *gfx;
	Uint32 offs;
	AG_Sprite *spr;
	int x = go->g_map.x + X0;
	int y = go->g_map.y + Y0;
	int l0 = go->g_map.l0 + L0, l;
	int sx, sy, dx, dy;
	int dx0, dy0, xorig, yorig;
	SDL_Surface *su;
	int n = 0;

	if (gfx_obj == NULL || (gfx = AGOBJECT(gfx_obj)->gfx) == NULL) {
		AG_SetError("NULL gfx");
		return (-1);
	}
	if (!AG_SpriteFind(gfx, name, &offs)) {
		return (-1);
	}
	spr = &gfx->sprites[offs];
	su = spr->su;
	dx0 = x - spr->xOrig/AGTILESZ;
	dy0 = y - spr->yOrig/AGTILESZ;
	xorig = spr->xOrig%AGTILESZ;
	yorig = spr->yOrig%AGTILESZ;

	for (sy = 0, dy = dy0;
	     sy < su->h;
	     sy += AGTILESZ, dy++) {
		for (sx = 0, dx = dx0;
		     sx < su->w;
		     sx += AGTILESZ, dx++) {
			AG_Node *dn;
			AG_Nitem *r;

			if (dx < 0 || dx >= m->mapw ||
			    dy < 0 || dy >= m->maph) {
				goto out;
			}
			dn = &m->map[dy][dx];

			r = AG_NodeAddSprite(m, dn, gfx_obj, offs);
			r->p = obj;
			r->r_gfx.rs.x = sx;
			r->r_gfx.rs.y = sy;
			r->r_gfx.rs.w = AGTILESZ;
			r->r_gfx.rs.h = AGTILESZ;
			r->r_gfx.xorigin = xorig;
			r->r_gfx.yorigin = yorig;
			r->r_gfx.xcenter = AGTILESZ/2;
			r->r_gfx.ycenter = AGTILESZ/2;
			r->r_gfx.xmotion = go->g_map.xmot;
			r->r_gfx.ymotion = go->g_map.ymot;
			r->flags |= spr->attrs[n];
			r->flags |= AG_NITEM_NOSAVE;

			l = l0 + spr->layers[n];
			if (l < 0) {
				l = 0;
			} else {
				while (m->nlayers <= l) {
					AG_MapPushLayer(m, "");
				}
			}
			r->layer = l;
	
			if (dx < go->g_map.x0) { go->g_map.x0 = dx; }
			else if (dx > go->g_map.x1) { go->g_map.x1 = dx; }
			if (dy < go->g_map.y0) { go->g_map.y0 = dy; }
			else if (dy > go->g_map.y1) { go->g_map.y1 = dy; }
			if (l < go->g_map.l0) { go->g_map.l0 = l; }
			else if (l > go->g_map.l1) { go->g_map.l1 = l; }
out:
			n++;
		}
	}
	return (0);
}

void
AG_ActorUnmapSprite(void *obj)
{
	AG_Actor *go = obj;
	AG_Map *m = go->parent;
	int x, y;

	if (m == NULL)
		return;

	for (y = go->g_map.y0; y <= go->g_map.y1; y++) {
		for (x = go->g_map.x0; x <= go->g_map.x1; x++) {
			AG_Node *node = &m->map[y][x];
			AG_Nitem *r, *nr;
		
			for (r = TAILQ_FIRST(&node->nrefs);
			     r != TAILQ_END(&node->nrefs);
			     r = nr) {
				nr = TAILQ_NEXT(r, nrefs);
				if (r->p == go &&
				    r->layer >= go->g_map.l0 &&
				    r->layer <= go->g_map.l1)
					AG_NodeDelItem(m, node, r);
			}
		}
	}
}

#ifdef EDITION
void
AG_ActorEdit(AG_Actor *go, void *cont)
{
	AG_LabelNew(cont, AG_LABEL_POLLED, _("Type: %d"), &go->type);
	AG_LabelNew(cont, AG_LABEL_POLLED, _("Flags: 0x%x"), &go->flags);
	AG_LabelNew(cont, AG_LABEL_POLLED, _("Map layers: %d-%d"),
	    &go->g_map.l0, &go->g_map.l1);
	AG_LabelNew(cont, AG_LABEL_POLLED, _("Map position: [%d,%d]"),
	    &go->g_map.x, &go->g_map.y);
	AG_LabelNew(cont, AG_LABEL_POLLED, _("Map extent: [%d,%d]-[%d,%d]"),
	    &go->g_map.x0, &go->g_map.y0, &go->g_map.x1, &go->g_map.y1);
	AG_LabelNew(cont, AG_LABEL_POLLED, _("Map offset: [%d,%d]"),
	    &go->g_map.xmot, &go->g_map.ymot);
	AG_LabelNew(cont, AG_LABEL_POLLED, _("Direction: %d(%d)"),
	    &go->g_map.da, &go->g_map.dv);
}
#endif /* EDITION */
