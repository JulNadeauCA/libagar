/*	$Csoft: world.c,v 1.7 2002/02/15 02:31:32 vedge Exp $	*/

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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>

#include <engine/engine.h>

static struct obvec world_vec = {
	world_destroy,
	NULL,
	world_load,
	world_save,
	NULL,
	NULL
};

struct world *
world_create(char *name)
{
	struct passwd *pwd;
	struct stat sta;
	
	pwd = getpwuid(getuid());

	world = (struct world *)emalloc(sizeof(struct world));
	object_init(&world->obj, name, 0, &world_vec);
	world->udatadir = (char *)emalloc(strlen(pwd->pw_dir) + strlen(name)+2);
	world->sysdatadir = (char *)emalloc(strlen(DATADIR) + strlen(name)+1);
	sprintf(world->udatadir, "%s/.%s", pwd->pw_dir, name);
	sprintf(world->sysdatadir, "%s/%s", DATADIR, name);
	
	if (stat(world->sysdatadir, &sta) != 0) {
		warning("%s: %s\n", world->sysdatadir, strerror(errno));
	}
	if (stat(world->udatadir, &sta) != 0 &&
	    mkdir(world->udatadir, 00700) != 0) {
		fatal("%s: %s\n", world->udatadir, strerror(errno));
	}

	world->curmap = NULL;

	SLIST_INIT(&world->wobjsh);
	SLIST_INIT(&world->wcharsh);
	pthread_mutex_init(&world->lock, NULL);
	
	return (world);
}

int
world_load(void *p, int fd)
{
	struct world *wo = (struct world *)p;
	struct object *ob;

	SLIST_FOREACH(ob, &wo->wobjsh, wobjs) {
		object_load(ob);
	}
	return (0);
}

int
world_save(void *p, int fd)
{
	struct world *wo = (struct world *)p;
	struct object *ob;

	SLIST_FOREACH(ob, &wo->wobjsh, wobjs) {
		object_save(ob);
	}
	return (0);
}

int
world_destroy(void *p)
{
	struct world *wo = (struct world *)p;
	struct object *nob;

	if (world->curmap != NULL) {
		map_unfocus(world->curmap);
	}

	SLIST_FOREACH(nob, &wo->wobjsh, wobjs) {
		object_unlink(nob);
		object_destroy(nob);
	}
	
	object_lategc();
	return (0);
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

