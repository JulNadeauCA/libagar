/*	$Csoft: space.c,v 1.8 2005/08/27 04:40:00 vedge Exp $	*/

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
#include <engine/actor.h>

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
	TAILQ_INIT(&sp->actors);
}

void
space_reinit(void *obj)
{
	struct space *sp = obj;
	struct actor *ac;
	
	pthread_mutex_lock(&sp->lock);
	TAILQ_FOREACH(ac, &sp->actors, actors) {
		space_detach(sp, ac);
	}
	TAILQ_INIT(&sp->actors);
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
	Uint32 i, nactors, name;
	void *actor;

	if (version_read(buf, &space_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&sp->lock);
	nactors = read_uint32(buf);
	for (i = 0; i < nactors; i++) {
		name = read_uint32(buf);
		if (object_find_dep(sp, name, &actor) == -1) {
			goto fail;
		}
		TAILQ_INSERT_TAIL(&sp->actors, ACTOR(actor), actors);
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
	struct actor *actor;
	off_t nactors_offs;
	Uint32 nactors = 0;

	version_write(buf, &space_ver);

	pthread_mutex_lock(&sp->lock);
	nactors_offs = netbuf_tell(buf);
	write_uint32(buf, 0);
	TAILQ_FOREACH(actor, &sp->actors, actors) {
		dprintf("actor: %s (%s)\n", OBJECT(actor)->name,
		    OBJECT(actor)->type);
		write_uint32(buf, object_encode_name(sp, actor));
		nactors++;
	}
	pwrite_uint32(buf, nactors, nactors_offs);
	pthread_mutex_unlock(&sp->lock);
	return (0);
}

/* Map an actor into a space. */
int
space_attach(void *sp_obj, void *obj)
{
	struct space *space = sp_obj;
	struct actor *ac = obj;

	pthread_mutex_lock(&ac->lock);
		
	object_add_dep(space, ac);
	
	if (OBJECT_TYPE(space, "map")) {
		struct map *m = (struct map *)space;
		
		if (ac->g_map.x < 0 || ac->g_map.x >= m->mapw ||
		    ac->g_map.y < 0 || ac->g_map.y >= m->maph)  {
			error_set(_("Illegal coordinates: %s:%d,%d"),
			    OBJECT(m)->name, ac->g_map.x, ac->g_map.y);
			goto fail;
		}

		ac->type = ACTOR_MAP;
		ac->parent = space;
		ac->g_map.x0 = ac->g_map.x;
		ac->g_map.y0 = ac->g_map.y;
		ac->g_map.x1 = ac->g_map.x;
		ac->g_map.y1 = ac->g_map.y;
	
		if (ACTOR_OPS(ac)->map != NULL)
			ACTOR_OPS(ac)->map(ac, m);
	} else {
		ac->type = ACTOR_NONE;
		ac->parent = NULL;
	}
	pthread_mutex_unlock(&ac->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ac->lock);
	return (-1);
}

void
space_detach(void *sp_obj, void *obj)
{
	struct actor *ac = obj;
	struct space *space = sp_obj;

	pthread_mutex_lock(&ac->lock);

	object_cancel_timeouts(ac, 0);		/* XXX hook? */

	if (OBJECT_TYPE(space, "map")) {
		actor_unmap_sprite(ac);
	}
	object_del_dep(space, ac);
	ac->parent = NULL;
out:
	pthread_mutex_unlock(&ac->lock);
}

