/*	$Csoft: perso.c,v 1.55 2005/09/19 01:25:16 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/widget/hbox.h>
#include <engine/widget/vbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/objsel.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/notebook.h>
#include <engine/widget/separator.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "perso.h"

const struct version perso_ver = {
	"agar personage",
	3, 0
};

static int perso_keydown(void *, int, int);
static int perso_keyup(void *, int, int);

const struct actor_ops perso_ops = {
	{
		perso_init,
		perso_reinit,
		perso_destroy,
		perso_load,
		perso_save,
		perso_edit
	},
	perso_map,
	NULL,		/* unmap */
	perso_update,
	perso_keydown,
	perso_keyup,
	NULL,		/* mousemotion */
	NULL,		/* mousebuttondown */
	NULL,		/* mousebuttonup */
	NULL,		/* joyaxis */
	NULL,		/* joyball */
	NULL,		/* joyhat */
	NULL		/* joybutton */
};

static void *perso_tileset = NULL;

struct perso *
perso_new(void *parent, const char *name)
{
	struct perso *ps;

	ps = Malloc(sizeof(struct perso), M_OBJECT);
	perso_init(ps, name);
	object_attach(parent, ps);
	return (ps);
}

static int
can_walk_to(void *p, int xo, int yo)
{
	struct actor *go = p;
	struct map *m = go->parent;
	int dx0 = go->g_map.x0 + xo;
	int dy0 = go->g_map.y0 + yo;
	int dx1 = go->g_map.x1 + xo;
	int dy1 = go->g_map.y1 + yo;
	struct node *n;
	int x, y;

	if (dx0 < 0 || dy0 < 0 || dx0 >= m->mapw || dy0 >= m->maph ||
	    dx1 < 0 || dy1 < 0 || dx1 >= m->mapw || dy1 >= m->maph)
		return (0);

	for (y = dy0; y < dy1; y++) {
		for (x = dx0; x < dx1; x++) {
			struct node *n = &m->map[y][x];
			struct noderef *r;

			TAILQ_FOREACH(r, &n->nrefs, nrefs) {
				if (r->p != go &&
				    r->layer >= go->g_map.l0 &&
				    r->layer <= go->g_map.l1 &&
				    r->flags & NODEREF_BLOCK) {
					dprintf("%s: blocked\n",
					    OBJECT(go)->name);
					return (0);
				}
			}
		}
	}
	return (1);
}

static Uint32
perso_move(void *obj, Uint32 ival, void *arg)
{
	struct actor *go = obj;
	struct perso *ps = obj;
	int xo = 0, yo = 0;

	switch (go->g_map.da) {
	case 0:	  xo = -go->g_map.dv; break;
	case 90:  yo = -go->g_map.dv; break;
	case 180: xo = +go->g_map.dv; break;
	case 270: yo = +go->g_map.dv; break;
	}

	if ((xo != 0 || yo != 0) &&
	    can_walk_to(ps, xo, yo)) {
		actor_move_sprite(ps, xo, yo);
	}
	return (ival);
}

void
perso_init(void *obj, const char *name)
{
	struct perso *ps = obj;

	actor_init(ps, "perso", name, &perso_ops);
	ps->tileset = NULL;
	ps->name[0] = '\0';
	ps->flags = 0;
	ps->level = 0;
	ps->exp = 0;
	ps->age = 0;
	ps->seed = 0;			/* TODO arc4random */
	ps->hp = ps->maxhp = 1;
	ps->mp = ps->maxmp = 0;
	ps->nzuars = 0;
	pthread_mutex_init(&ps->lock, NULL);
	timeout_set(&ps->move_to, perso_move, NULL, 0);
}

void
perso_reinit(void *obj)
{
	struct perso *ps = obj;
	
	timeout_del(ps, &ps->move_to);

	actor_reinit(obj);
	
	if (ps->tileset != NULL) {
		object_page_out(ps->tileset, OBJECT_GFX);
		object_del_dep(ps, ps->tileset);
		ps->tileset = NULL;
	}
}

void
perso_destroy(void *obj)
{
	actor_destroy(obj);
}

int
perso_load(void *obj, struct netbuf *buf)
{
	struct perso *ps = obj;
	void *tileset;
	Uint32 name;

	if (version_read(buf, &perso_ver, NULL) != 0)
		return (-1);
	
	if (actor_load(ps, buf) == -1)
		return (-1);

	pthread_mutex_lock(&ps->lock);
	copy_string(ps->name, buf, sizeof(ps->name));
	ps->flags = read_uint32(buf);
	name = read_uint32(buf);

	if (name != 0) {
		if (object_find_dep(ps, name, &ps->tileset) == -1) {
			goto fail;
		}
		if (ps->tileset != NULL) {
			object_add_dep(ps, ps->tileset);
			object_page_in(ps->tileset, OBJECT_GFX);
		}
	}

	ps->level = read_sint32(buf);
	ps->exp = read_uint32(buf);
	ps->age = (int)read_uint32(buf);
	ps->seed = read_uint32(buf);
	ps->maxhp = (int)read_uint32(buf);
	ps->hp = (int)read_uint32(buf);
	ps->maxmp = (int)read_uint32(buf);
	ps->mp = (int)read_uint32(buf);
	ps->nzuars = (u_int)read_uint32(buf);

	pthread_mutex_unlock(&ps->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ps->lock);
	return (-1);
}

int
perso_save(void *obj, struct netbuf *buf)
{
	struct perso *ps = obj;

	version_write(buf, &perso_ver);
	
	if (actor_save(ps, buf) == -1)
		return (-1);

	pthread_mutex_lock(&ps->lock);
	write_string(buf, ps->name);
	write_uint32(buf, ps->flags);
	write_uint32(buf, object_encode_name(ps, ps->tileset));

	write_sint32(buf, ps->level);
	write_uint32(buf, ps->exp);
	write_uint32(buf, ps->age);
	write_uint32(buf, ps->seed);
	write_uint32(buf, (Uint32)ps->maxhp);
	write_uint32(buf, (Uint32)ps->hp);
	write_uint32(buf, (Uint32)ps->maxmp);
	write_uint32(buf, (Uint32)ps->mp);
	write_uint32(buf, (Uint32)ps->nzuars);
	pthread_mutex_unlock(&ps->lock);
	return (0);
}

void *
perso_edit(void *obj)
{
	struct perso *ps = obj;
	struct window *win;
	struct notebook *nb;
	struct notebook_tab *ntab;
	struct vbox *vb;

	win = window_new(WINDOW_DETACH|WINDOW_NO_VRESIZE, NULL);
	window_set_caption(win, _("Character \"%s\""), OBJECT(ps)->name);

	nb = notebook_new(win, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
	ntab = notebook_add_tab(nb, _("Informations"), BOX_VERT);
	{
		struct textbox *tb;
		struct spinbutton *sbu;
		struct hbox *hb;
		struct objsel *os;

		tb = textbox_new(ntab, _("Name: "));
		widget_bind(tb, "string", WIDGET_STRING, ps->name,
		    sizeof(ps->name));

		os = objsel_new(ntab, OBJSEL_PAGE_GFX, ps, world,
		    _("Tileset: "));
		objsel_mask_type(os, "tileset");
		widget_bind(os, "object", WIDGET_POINTER, &ps->tileset);
		objsel_select(os, ps->tileset);

		separator_new(ntab, SEPARATOR_HORIZ);

		sbu = spinbutton_new(ntab, _("Level: "));
		widget_bind(sbu, "value", WIDGET_SINT32, &ps->level);

		sbu = spinbutton_new(ntab, _("Experience: "));
		widget_bind(sbu, "value", WIDGET_UINT32, &ps->exp);

		sbu = spinbutton_new(ntab, _("Zuars: "));
		widget_bind(sbu, "value", WIDGET_UINT32, &ps->nzuars);

		hb = hbox_new(ntab, HBOX_HOMOGENOUS|HBOX_WFILL);
		{
			sbu = spinbutton_new(hb, _("HP: "));
			widget_bind(sbu, "value", WIDGET_INT, &ps->hp);
			
			sbu = spinbutton_new(hb, " / ");
			widget_bind(sbu, "value", WIDGET_INT, &ps->maxhp);
		}
		
		hb = hbox_new(ntab, HBOX_HOMOGENOUS|HBOX_WFILL);
		{
			sbu = spinbutton_new(hb, _("MP: "));
			widget_bind(sbu, "value", WIDGET_INT, &ps->mp);
			
			sbu = spinbutton_new(hb, " / ");
			widget_bind(sbu, "value", WIDGET_INT, &ps->maxmp);
		}
	}
	ntab = notebook_add_tab(nb, _("Position"), BOX_VERT);
	actor_edit(ACTOR(ps), ntab);
	return (win);
}

void
perso_map(void *obj, void *space)
{
	struct perso *ps = obj;

	dprintf("%s: mapping into %s\n", OBJECT(ps)->name, OBJECT(space)->name);

	if (OBJECT_TYPE(space, "map")) {
		struct map *m = space;

		if (actor_map_sprite(ps, 0, 0, 0, ps->tileset, "Idle-S")
		    == -1) {
			text_msg(MSG_ERROR, "%s->%s: %s", OBJECT(obj)->name,
			    OBJECT(space)->name, error_get());
		}
	}
#ifdef HAVE_OPENGL
	else if (OBJECT_TYPE(space, "scene")) {
		glBegin(GL_LINE_LOOP);
		glVertex3f(-3, +3, 0);
		glVertex3f(-3, -3, 0);
		glVertex3f(+3, -3, 0);
		glVertex3f(+3, +3, 0);
		glEnd();
	}
#endif
}

void
perso_update(void *space, void *obj)
{
	struct perso *ps = obj;
	
	dprintf("%s: update (space=%s)\n", OBJECT(ps)->name,
	    OBJECT(space)->name);
}

int
perso_keydown(void *p, int ks, int km)
{
	struct actor *go = p;
	struct perso *ps = p;

	switch (ks) {
	case SDLK_LEFT:
		go->g_map.da = 0;
		go->g_map.dv = 1;
		actor_set_sprite(go, 0, 0, 0, ps->tileset, "Idle-W");
		break;
	case SDLK_UP:
		go->g_map.da = 90;
		go->g_map.dv = 1;
		actor_set_sprite(go, 0, 0, 0, ps->tileset, "Idle-N");
		break;
	case SDLK_RIGHT:
		go->g_map.da = 180;
		go->g_map.dv = 1;
		actor_set_sprite(go, 0, 0, 0, ps->tileset, "Idle-E");
		break;
	case SDLK_DOWN:
		go->g_map.da = 270;
		go->g_map.dv = 1;
		actor_set_sprite(go, 0, 0, 0, ps->tileset, "Idle-S");
		break;
	}
	if (go->g_map.dv > 0) {
		timeout_add(p, &ps->move_to, 10);
	}
	return (0);
}

int
perso_keyup(void *p, int ks, int km)
{
	struct perso *ps = p;
	struct actor *go = p;

	switch (ks) {
	case SDLK_LEFT:
		if (go->g_map.da == 0) { goto stop; }
		break;
	case SDLK_UP:
		if (go->g_map.da == 90) { goto stop; }
		break;
	case SDLK_RIGHT:
		if (go->g_map.da == 180) { goto stop; }
		break;
	case SDLK_DOWN:
		if (go->g_map.da == 270) { goto stop; }
		break;
	}
	return (0);
stop:
	timeout_del(ps, &ps->move_to);
	go->g_map.dv = 0;
	return (0);
}
