/*	$Csoft: space.c,v 1.2 2005/08/10 06:59:26 vedge Exp $	*/

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
#include <engine/gobject.h>

#include <engine/map/map.h>

#include <engine/widget/window.h>
#include <engine/widget/menu.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "space.h"

const struct version space_ver = {
	"agar space",
	1, 0
};

void
space_init(void *obj, const char *type, const char *name, const void *ops)
{
	struct space *sp = obj;

	object_init(sp, type, name, ops);
	pthread_mutex_init(&sp->lock, &recursive_mutexattr);
	TAILQ_INIT(&sp->gobjs);
}

void
space_reinit(void *obj)
{
	struct space *sp = obj;
	struct gobject *go;
	
	pthread_mutex_lock(&sp->lock);
	TAILQ_FOREACH(go, &sp->gobjs, gobjs) {
		space_detach(sp, go);
	}
	TAILQ_INIT(&sp->gobjs);
	pthread_mutex_unlock(&sp->lock);
}

void
space_destroy(void *obj)
{
	/* nothing yet */
}

int
space_load(void *obj, struct netbuf *buf)
{
	struct space *sp = obj;
	Uint32 i, ngobjs, name;
	void *gobj;

	if (version_read(buf, &space_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&sp->lock);
	ngobjs = read_uint32(buf);
	for (i = 0; i < ngobjs; i++) {
		name = read_uint32(buf);
		if (object_find_dep(sp, name, &gobj) == -1) {
			goto fail;
		}
		TAILQ_INSERT_TAIL(&sp->gobjs, GOBJECT(gobj), gobjs);
	}
	pthread_mutex_unlock(&sp->lock);
	return (0);
fail:
	pthread_mutex_unlock(&sp->lock);
	return (-1);
}

int
space_save(void *obj, struct netbuf *buf)
{
	struct space *sp = obj;
	struct gobject *gobj;
	off_t ngobjs_offs;
	Uint32 ngobjs = 0;

	version_write(buf, &space_ver);

	pthread_mutex_lock(&sp->lock);
	ngobjs_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(gobj, &sp->gobjs, gobjs) {
		write_uint32(buf, object_encode_name(sp, gobj));
		ngobjs++;
	}
	pwrite_uint32(buf, ngobjs, ngobjs_offs);
	pthread_mutex_unlock(&sp->lock);
	return (0);
}

/* Map a geometric object into a space. */
int
space_attach(void *sp_obj, void *obj)
{
	struct space *space = sp_obj;
	struct gobject *go = obj;

	pthread_mutex_lock(&go->lock);
	
	if (object_page_in(go, OBJECT_DATA) == -1) {
		pthread_mutex_unlock(&go->lock);
		return (-1);
	}
	
	if (OBJECT_TYPE(space, "map")) {
		struct map *m = (struct map *)space;
		
		if (go->g_map.x < 0 || go->g_map.x >= m->mapw ||
		    go->g_map.y < 0 || go->g_map.y >= m->maph)  {
			error_set(_("Illegal coordinates: %s:%d,%d"),
			    OBJECT(m)->name, go->g_map.x, go->g_map.y);
			goto fail;
		}

		if (object_page_in(go, OBJECT_GFX) == -1) {
			goto fail;
		}
		object_add_dep(m, go);

		go->type = GOBJECT_MAP;
		go->g_map.x0 = go->g_map.x;
		go->g_map.y0 = go->g_map.y;
		go->g_map.x1 = go->g_map.x;
		go->g_map.y1 = go->g_map.y;
	
		if (GOBJECT_OPS(go)->map != NULL)
			GOBJECT_OPS(go)->map(go, m);
	}

	go->parent = space;
	pthread_mutex_unlock(&go->lock);
	return (0);
fail:
	object_page_out(go, OBJECT_DATA);
	pthread_mutex_unlock(&go->lock);
	return (-1);
}

void
space_detach(void *sp_obj, void *obj)
{
	struct gobject *go = obj;
	struct space *space = sp_obj;

	pthread_mutex_lock(&go->lock);

	if (OBJECT_TYPE(space, "map")) {
		struct map *m = (struct map *)space;
		int x, y;

		for (y = go->g_map.y0; y <= go->g_map.y1; y++) {
			for (x = go->g_map.x0; x <= go->g_map.x1; x++) {
				struct node *node;
				struct noderef *r, *nr;
		
				if (x < 0 || x >= m->mapw ||
				    y < 0 || y >= m->maph) {
					continue;
				}
				node = &m->map[y][x];

				for (r = TAILQ_FIRST(&node->nrefs);
				     r != TAILQ_END(&node->nrefs);
				     r = nr) {
					nr = TAILQ_NEXT(r, nrefs);
					if (r->p == go &&
					    r->layer >= go->g_map.l0 &&
					    r->layer <= go->g_map.l1)
						node_remove_ref(m, node, r);
				}
			}
		}
		object_del_dep(space, go);
		object_page_out(go, OBJECT_DATA);
		object_page_out(go, OBJECT_GFX);
	}

	go->type = GOBJECT_NONE;
	go->parent = NULL;
out:
	pthread_mutex_unlock(&go->lock);
}

#ifdef EDITION
static void
detach_all_gobjs(int argc, union evarg *argv)
{
	struct space *sp = argv[1].p;
	struct gobject *go;

	TAILQ_FOREACH(go, &sp->gobjs, gobjs) {
		space_detach(sp, go);
	}
	TAILQ_INIT(&sp->gobjs);
}

void
space_generic_menu(void *menup, void *space)
{
	struct AGMenuItem *pitem = menup;
	struct AGMenuItem *it;

	it = menu_action(pitem, _("Space"), -1, NULL, NULL);
	{
		menu_action(it, _("Detach all objects"), -1,
		    detach_all_gobjs, "%p", space);
	}
}
#endif /* EDITION */
