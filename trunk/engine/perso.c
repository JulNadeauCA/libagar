/*	$Csoft: perso.c,v 1.24 2003/05/09 01:59:47 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/compat/vasprintf.h>
#include <engine/engine.h>

#include <libfobj/fobj.h>

#include <engine/map.h>
#include <engine/rootmap.h>
#include <engine/perso.h>
#include <engine/version.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

enum {
	DEFAULT_HP	= 10,
	DEFAULT_MP	= 0,
	DEFAULT_NZUARS	= 0
};

const struct version perso_ver = {
	"agar personage",
	2, 0
};

const struct object_ops perso_ops = {
	perso_init,
	perso_destroy,
	perso_load,
	perso_save
};

#ifdef DEBUG
#define	DEBUG_STATE	0x01
#define DEBUG_POSITION	0x02

int	perso_debug = DEBUG_STATE|DEBUG_POSITION;
#define engine_debug	perso_debug
#endif

struct perso *
perso_new(void *parent, char *name)
{
	struct perso *pers;

	pers = Malloc(sizeof(struct perso));
	perso_init(pers, name);
	object_attach(parent, pers);
	return (pers);
}

void
perso_init(void *obj, char *name)
{
	struct perso *pers = obj;

	object_init(pers, "perso", name, &perso_ops);
	object_load_art(pers, name, 0);
	object_load_submap(pers, "n-idle");
	object_load_submap(pers, "s-idle");
	object_load_submap(pers, "w-idle");
	object_load_submap(pers, "e-idle");
	object_load_submap(pers, "n-move");
	object_load_submap(pers, "s-move");
	object_load_submap(pers, "w-move");
	object_load_submap(pers, "e-move");

	pthread_mutex_init(&pers->lock, NULL);
	pers->name = Strdup(name);
	pers->flags = 0;
	pers->level = 0;
	pers->exp = 0;
	pers->age = 0;
	pers->seed = 0;			/* TODO arc4random */
	pers->hp = pers->maxhp = DEFAULT_HP;
	pers->mp = pers->maxmp = DEFAULT_MP;
	pers->nzuars = DEFAULT_NZUARS;
}

void
perso_destroy(void *obj)
{
	struct perso *perso = obj;

	free(perso->name);
}

int
perso_load(void *obj, struct netbuf *buf)
{
	struct perso *perso = obj;

	if (version_read(buf, &perso_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&perso->lock);
	perso->name = read_string(buf, perso->name);
	perso->flags = read_uint32(buf);
	perso->level = read_uint32(buf);
	perso->exp = (int)read_uint32(buf);
	perso->age = (int)read_uint32(buf);
	perso->seed = read_uint32(buf);
	perso->maxhp = (int)read_uint32(buf);
	perso->hp = (int)read_uint32(buf);
	perso->maxmp = (int)read_uint32(buf);
	perso->mp = (int)read_uint32(buf);
	perso->nzuars = read_uint32(buf);
	pthread_mutex_unlock(&perso->lock);
	return (0);
}

int
perso_save(void *obj, struct netbuf *buf)
{
	struct perso *perso = obj;

	version_write(buf, &perso_ver);

	pthread_mutex_lock(&perso->lock);
	write_string(buf, perso->name);
	write_uint32(buf, perso->flags);
	write_uint32(buf, perso->level);
	write_uint32(buf, (Uint32)perso->exp);
	write_uint32(buf, perso->age);
	write_uint32(buf, perso->seed);
	write_uint32(buf, (Uint32)perso->maxhp);
	write_uint32(buf, (Uint32)perso->hp);
	write_uint32(buf, (Uint32)perso->maxmp);
	write_uint32(buf, (Uint32)perso->mp);
	write_uint32(buf, perso->nzuars);
	pthread_mutex_unlock(&perso->lock);
	return (0);
}

