/*	$Csoft: world.c,v 1.8 2002/02/17 08:11:21 vedge Exp $	*/

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
#include <engine/config.h>

static struct obvec world_vec = {
	world_destroy,
	NULL,
	world_load,
	world_save,
	NULL,
	NULL
};

char *
savepath(char *obname, const char *suffix)
{
	static char path[FILENAME_MAX];
	struct stat sta;

	sprintf(path, "%s/%s.%s", world->udatadir, obname, suffix);
	if (stat(path, &sta) == 0) {
		return (path);
	}
	sprintf(path, "%s/%s.%s", world->sysdatadir, obname, suffix);
	if (stat(path, &sta) == 0) {
		return (path);
	}
	fatal("cannot find %s.%s in %s:%s\n", obname, suffix,
	    world->udatadir, world->sysdatadir);
	return (NULL);
}

struct world *
world_create(char *name)
{
	struct passwd *pwd;
	struct stat sta;
	
	pwd = getpwuid(getuid());

	world = (struct world *)emalloc(sizeof(struct world));
	object_init(&world->obj, name, 0, &world_vec);
	world->udatadir = (char *)emalloc(strlen(pwd->pw_dir) + strlen(name)+2);
	world->sysdatadir = (char *)emalloc(strlen(SHAREDIR) + strlen(name)+1);
	sprintf(world->udatadir, "%s/.%s", pwd->pw_dir, name);
	sprintf(world->sysdatadir, SHAREDIR);
	
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
	off_t soffs;
	int nobjs = 0;

	dprintf("saving state\n");
	soffs = lseek(fd, 0, SEEK_SET);
	fobj_write_uint32(fd, 0);
	SLIST_FOREACH(ob, &wo->wobjsh, wobjs) {
		fobj_write_uint32(fd, ob->id);
		fobj_write_string(fd, ob->name);
		fobj_write_string(fd, (ob->desc != NULL) ? ob->desc : "");
		object_save(ob);
		nobjs++;
	}
	fobj_pwrite_uint32(fd, nobjs, soffs);
	dprintf("%s: %d objects\n", wo->obj.name, nobjs);
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

	object_dump((struct object *)wo);
	SLIST_FOREACH(ob, &wo->wobjsh, wobjs)
		object_dump(ob);
#if 0
	struct character *ch;
	printf("characters\n");
	SLIST_FOREACH(ch, &wo->wcharsh, wchars)
		char_dump(ch);
#endif
}

#endif /* DEBUG */

