/*	$Csoft: world.c,v 1.45 2002/09/02 08:12:14 vedge Exp $	*/

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

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <libfobj/fobj.h>

#include "engine.h"

#include "mapedit/mapedit.h"

static const struct object_ops world_ops = {
	NULL,
	world_load,
	world_save
};

void
world_init(struct world *wo, char *name)
{
	object_init(&wo->obj, "world", name, NULL, 0, &world_ops);
	wo->nobjs = 0;
	SLIST_INIT(&wo->wobjsh);
	pthread_mutex_init(&wo->lock, NULL);
}

/* World must be locked. */
int
world_load(void *p, int fd)
{
	struct world *wo = p;
	struct object *ob;

	/* XXX load the state map */
	SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
		dprintf("loading %s\n", ob->name);

		if (curmapedit != NULL && !strcmp(ob->saveext, "m")) {
			/* XXX map editor hack */
			continue;
		}
		object_load(ob);
	}
	printf("%s: loaded %d objects\n", OBJECT(wo)->name, wo->nobjs);
	return (0);
}

/* World must be locked. */
int
world_save(void *p, int fd)
{
	struct world *wo = p;
	struct object *ob;

	SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
		dprintf("saving %s\n", ob->name);

		if (curmapedit != NULL && !strcmp(ob->saveext, "m")) {
			/* XXX map editor hack */
			continue;
		}
		write_string(fd, ob->name);
		write_string(fd, (ob->desc != NULL) ? ob->desc : "");
		object_save(ob);
	}

	dprintf("saved %d objects\n", wo->nobjs);
	return (0);
}

/* Destroy the world! */
void
world_destroy(void *p)
{
	struct world *wo = p;
	struct object *ob, *nextob;
	
	pthread_mutex_lock(&wo->lock);

	printf("freed:");
	fflush(stdout);
	for (ob = SLIST_FIRST(&wo->wobjsh);
	     ob != SLIST_END(&wo->wobjsh);
	     ob = nextob) {
		nextob = SLIST_NEXT(ob, wobjs);
		printf(" %s", ob->name);
		fflush(stdout);
		object_destroy(ob);
	}
	printf(".\n");

	pthread_mutex_unlock(&wo->lock);
	pthread_mutex_destroy(&wo->lock);
}

/* World must be locked. */
void
world_attach(void *parent, void *child)
{
	struct world *wo = parent;
	struct object *ob = child;

	SLIST_INSERT_HEAD(&wo->wobjsh, ob, wobjs);
	wo->nobjs++;
	
	event_post(ob, "attached", "%p", wo);
}

/* World must be locked. */
void
world_detach(void *parent, void *child)
{
	struct world *wo = parent;
	struct object *ob = child;
	
	event_post(ob, "detached", "%p", wo);
	
	SLIST_REMOVE(&wo->wobjsh, ob, object, wobjs);
	wo->nobjs--;

	object_destroy(ob);
}

