/*	$Csoft: perso.c,v 1.21 2003/04/18 04:03:56 vedge Exp $	*/

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
#include <engine/input.h>
#include <engine/version.h>
#include <engine/view.h>
#include <engine/world.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

enum {
	DEFAULT_HP	= 10,
	DEFAULT_MP	= 10,
	DEFAULT_SPEED	= 16,
	DEFAULT_ZUARS	= 0
};

static const struct version perso_ver = {
	"agar personage",
	2, 0
};

static const struct object_ops perso_ops = {
	perso_destroy,
	perso_load,
	perso_save
};

static Uint32	perso_time(Uint32, void *);

#ifdef DEBUG
#define	DEBUG_STATE	0x01
#define DEBUG_POSITION	0x02

int	perso_debug = DEBUG_STATE|DEBUG_POSITION;
#define engine_debug	perso_debug
#endif

struct perso *
perso_new(char *name, int hp, int mp)
{
	struct perso *pers;

	pers = Malloc(sizeof(struct perso));
	perso_init(pers, name, hp, mp);

	world_attach(pers);
	return (pers);
}

void
perso_init(struct perso *pers, char *name, int hp, int mp)
{
	object_init(&pers->obj, "perso", name, 0, &perso_ops);
	object_load_art(pers, name, 0);

	object_load_submap(pers, "n-idle");
	object_load_submap(pers, "s-idle");
	object_load_submap(pers, "w-idle");
	object_load_submap(pers, "e-idle");
	object_load_submap(pers, "n-move");
	object_load_submap(pers, "s-move");
	object_load_submap(pers, "w-move");
	object_load_submap(pers, "e-move");

	pers->name = Strdup(name);
	pers->flags = 0;
	pers->level = 0;
	pers->exp = 0;
	pers->age = 0;
	pers->seed = (Uint32)lrand48();

	pers->hp = pers->maxhp = hp;
	pers->mp = pers->maxmp = mp;
	pers->nzuars = DEFAULT_ZUARS;

	pthread_mutex_init(&pers->lock, NULL);

	event_new(pers, "attached", perso_attached, NULL);
	event_new(pers, "detached", perso_detached, NULL);
}

void
perso_destroy(void *p)
{
	struct perso *perso = p;

	free(perso->name);
}

int
perso_load(void *p, struct netbuf *buf)
{
	struct perso *perso = p;

	if (version_read(buf, &perso_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&perso->lock);

	/* Read the character status. */
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

	debug(DEBUG_STATE,
	    "%s (0x%x) lvl=%d exp=%d age=%d hp=%d/%d mp=%d/%d nzuars=%d\n",
	    OBJECT(perso)->name, perso->flags, perso->level, perso->exp,
	    perso->age, perso->hp, perso->maxhp, perso->mp, perso->maxhp,
	    perso->nzuars);

	if (read_uint32(buf) == 0) {
		debug(DEBUG_STATE, "%s is nowhere.\n", OBJECT(perso)->name);
	} else {
		if (object_load_position(perso, buf) == -1)
			goto fail;
	}

	pthread_mutex_unlock(&perso->lock);
	return (0);
fail:
	pthread_mutex_unlock(&perso->lock);
	return (-1);
}

int
perso_save(void *p, struct netbuf *buf)
{
	struct perso *perso = p;

	version_write(buf, &perso_ver);

	pthread_mutex_lock(&perso->lock);
	write_string(buf, perso->name);
	write_uint32(buf, perso->flags & ~(PERSO_EPHEMERAL));
	write_uint32(buf, perso->level);
	write_uint32(buf, (Uint32)perso->exp);
	write_uint32(buf, perso->age);
	write_uint32(buf, perso->seed);
	write_uint32(buf, (Uint32)perso->maxhp);
	write_uint32(buf, (Uint32)perso->hp);
	write_uint32(buf, (Uint32)perso->maxmp);
	write_uint32(buf, (Uint32)perso->mp);
	write_uint32(buf, perso->nzuars);
	write_uint32(buf, 1);			/* One position */
	object_save_position(perso, buf);
	pthread_mutex_unlock(&perso->lock);
	return (0);
}

void
perso_attached(int argc, union evarg *argv)
{
	struct perso *pers = argv[0].p;

	pthread_mutex_lock(&pers->lock);
	pers->timer = SDL_AddTimer(30, perso_time, pers);
	pthread_mutex_unlock(&pers->lock);
}

void
perso_detached(int argc, union evarg *argv)
{
	struct perso *pers = argv[0].p;

	pthread_mutex_lock(&pers->lock);
	SDL_RemoveTimer(pers->timer);
	pthread_mutex_unlock(&pers->lock);
}

static Uint32
perso_time(Uint32 ival, void *p)
{
	struct object *ob = p;

	pthread_mutex_lock(&ob->lock);
	if (ob->pos != NULL &&
	    mapdir_move(&ob->pos->dir) == -1) {
		debug(DEBUG_POSITION, "%s: %s\n", ob->name, error_get());
	} else {
		debug(DEBUG_POSITION, "%s is nowhere!\n", ob->name);
	}
	pthread_mutex_unlock(&ob->lock);
	return (ival);
}

void
perso_say(struct perso *pers, const char *fmt, ...)
{
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct button *button;
	va_list args;
	char *msg;

	win = window_generic_new(253, 140, NULL);
	if (win == NULL) {
		return;
	}
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
	
	va_start(args, fmt);
	Vasprintf(&msg, fmt, args);
	va_end(args);

	lab = label_new(reg, 100, 60, msg);
	button = button_new(reg, "Ok", NULL, 0, 99, 40);
	WIDGET_FOCUS(button);

	event_new(button, "button-pushed", window_generic_detach, "%p", win);
	window_show(win);

	free(msg);
}

