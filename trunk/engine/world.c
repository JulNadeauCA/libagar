/*	$Csoft: world.c,v 1.51 2002/11/22 08:56:49 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include "engine.h"

#include <libfobj/fobj.h>

#include "world.h"

#include "mapedit/mapedit.h"

static const struct object_ops world_ops = {
	NULL,		/* destroy */
	world_load,
	world_save
};

#ifdef DEBUG
#define DEBUG_STATE	0x02
#define DEBUG_GC	0x04

int	world_debug = DEBUG_STATE|DEBUG_GC;
#define engine_debug world_debug
#endif

void
world_init(struct world *wo, char *name)
{
	object_init(&wo->obj, "world", name, NULL, 0, &world_ops);
	wo->nobjs = 0;
	SLIST_INIT(&wo->wobjs);
	pthread_mutexattr_init(&wo->lockattr);
	pthread_mutexattr_settype(&wo->lockattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&wo->lock, &wo->lockattr);
}

int
world_load(void *p, int fd)
{
	struct world *wo = p;
	struct object *ob;

	/* XXX load the state map */

	pthread_mutex_lock(&world->lock);

	SLIST_FOREACH(ob, &world->wobjs, wobjs) {
		debug(DEBUG_STATE, "loading %s\n", ob->name);
		if (curmapedit != NULL && OBJECT_ISTYPE(ob, "map")) {
			/* XXX map editor hack */
			continue;
		}
		object_load(ob);
	}
	debug(DEBUG_STATE, "%s: loaded %d objects\n", OBJECT(wo)->name,
	    wo->nobjs);
	
	pthread_mutex_unlock(&world->lock);
	return (0);
}

int
world_save(void *p, int fd)
{
	struct world *wo = p;
	struct object *ob;

	pthread_mutex_lock(&world->lock);

	SLIST_FOREACH(ob, &world->wobjs, wobjs) {
		debug(DEBUG_STATE, "saving %s\n", ob->name);

		if (curmapedit != NULL && OBJECT_ISTYPE(ob, "map")) {
			/* XXX map editor hack */
			continue;
		}
		write_string(fd, ob->name);
		write_string(fd, "");
		object_save(ob);
	}
	debug(DEBUG_STATE, "saved %d objects\n", wo->nobjs);

	pthread_mutex_unlock(&world->lock);
	return (0);
}

/* Destroy the world! */
void
world_destroy(void *p)
{
	struct world *wo = p;
	struct object *ob, *nextob;
	
	pthread_mutex_lock(&wo->lock);
	debug_n(DEBUG_GC, "freed:");
	for (ob = SLIST_FIRST(&wo->wobjs);
	     ob != SLIST_END(&wo->wobjs);
	     ob = nextob) {
		nextob = SLIST_NEXT(ob, wobjs);
		debug_n(DEBUG_GC, " %s", ob->name);
		object_destroy(ob);
	}
	debug_n(DEBUG_GC, ".\n");
	pthread_mutex_unlock(&wo->lock);
	pthread_mutex_destroy(&wo->lock);
	pthread_mutexattr_destroy(&wo->lockattr);
}

void
world_attach(void *parent, void *child)
{
	struct world *wo = parent;
	struct object *ob = child;

	pthread_mutex_lock(&wo->lock);

	SLIST_INSERT_HEAD(&wo->wobjs, ob, wobjs);
	wo->nobjs++;
	ob->state = OBJECT_CONSISTENT;

	event_post(ob, "attached", "%p", wo);

	pthread_mutex_unlock(&wo->lock);
}

void
world_detach(void *parent, void *child)
{
	struct world *wo = parent;
	struct object *ob = child;
	
	debug(DEBUG_GC, "freeing %s\n", ob->name);

	pthread_mutex_lock(&wo->lock);
	if (ob->state != OBJECT_CONSISTENT) {
		fatal("inconsistent: %s\n", ob->name);
	}
	
	event_post(ob, "detached", "%p", wo);
	SLIST_REMOVE(&wo->wobjs, ob, object, wobjs);
	wo->nobjs--;
	ob->state = OBJECT_ZOMBIE;

	pthread_mutex_unlock(&wo->lock);

	object_destroy(ob);
}

int
world_attached(void *p)
{
	struct object *ob = p;

	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(ob, &world->wobjs, wobjs) {
		if (ob == (struct object *)p) {
			pthread_mutex_unlock(&world->lock);
			return (1);
		}
	}
	pthread_mutex_unlock(&world->lock);
	return (0);
}

void *
world_find(char *s)
{
	struct object *ob;

	pthread_mutex_lock(&world->lock);
	SLIST_FOREACH(ob, &world->wobjs, wobjs) {
		if (strcmp(ob->name, s) == 0) {
			pthread_mutex_unlock(&world->lock);
			return (ob);
		}
	}
	pthread_mutex_unlock(&world->lock);
	return (NULL);
}

