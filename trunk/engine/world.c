/*	$Csoft: world.c,v 1.6 2002/02/14 06:30:51 vedge Exp $	*/

/*
 * Copyright (c) 2001 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <engine/engine.h>

static void	world_destroy(void *);

struct world *
world_create(char *name)
{
	world = (struct world *)emalloc(sizeof(struct world));
	object_create(&world->obj, name, NULL, DESTROY_HOOK);
	world->obj.destroy_hook = world_destroy;

	SLIST_INIT(&world->wobjsh);
	SLIST_INIT(&world->wcharsh);

	world->agef = 0.01;
	pthread_mutex_init(&world->lock, NULL);

	return (world);
}

void
world_destroy(void *p)
{
	struct world *wo = (struct world *)p;
	struct object *nob;

	if (curmap != NULL && (curmap->flags & MAP_FOCUSED)) {
		map_unfocus(curmap);
	}

	SLIST_FOREACH(nob, &wo->wobjsh, wobjs) {
		object_destroy(nob);
		object_unlink(nob);
	}
	
	object_lategc();
}

#ifdef DEBUG

void
world_dump(struct world *wo)
{
	struct object *ob;
	struct character *ch;

	printf("objs\n");
	SLIST_FOREACH(ob, &wo->wobjsh, wobjs)
		object_dump(ob);
	printf("characters\n");
	SLIST_FOREACH(ch, &wo->wcharsh, wchars)
		char_dump(ch);
}

#endif /* DEBUG */

