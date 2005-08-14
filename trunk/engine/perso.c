/*	$Csoft: perso.c,v 1.50 2005/08/10 06:53:59 vedge Exp $	*/

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

const struct gobject_ops perso_ops = {
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
	perso_update
};

struct perso *
perso_new(void *parent, const char *name)
{
	struct perso *ps;

	ps = Malloc(sizeof(struct perso), M_OBJECT);
	perso_init(ps, name);
	object_attach(parent, ps);
	return (ps);
}

void
perso_init(void *obj, const char *name)
{
	struct perso *ps= obj;

	gobject_init(ps, "perso", name, &perso_ops);
	pthread_mutex_init(&ps->lock, NULL);
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
}

void
perso_reinit(void *obj)
{
	struct perso *ps = obj;

	gobject_reinit(obj);

	if (ps->tileset != NULL) {
		object_page_out(ps->tileset, OBJECT_GFX);
		object_del_dep(ps, ps->tileset);
		ps->tileset = NULL;
	}
}

void
perso_destroy(void *obj)
{
	gobject_destroy(obj);
}

int
perso_load(void *obj, struct netbuf *buf)
{
	struct perso *ps = obj;
	void *tileset;
	Uint32 name;

	if (version_read(buf, &perso_ver, NULL) != 0)
		return (-1);
	
	if (gobject_load(ps, buf) == -1)
		return (-1);

	pthread_mutex_lock(&ps->lock);
	copy_string(ps->name, buf, sizeof(ps->name));
	ps->flags = read_uint32(buf);
	name = read_uint32(buf);

	if (name != 0) {
		dprintf("loading tileset (%d)\n", name);
		if (object_find_dep(ps, name, &ps->tileset) == -1) {
			dprintf("error loading tileset %d: %s\n", name,
			    error_get());
			goto fail;
		}
		object_add_dep(ps, ps->tileset);
		object_page_in(ps->tileset, OBJECT_GFX);
	} else {
		dprintf("loading tileset (none)\n");
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
	
	if (gobject_save(ps, buf) == -1)
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

struct window *
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
	gobject_edit(GOBJECT(ps), ntab);
	return (win);
}

void
perso_map(void *obj, void *space)
{
	struct perso *ps = obj;

	dprintf("%s: mapping into %s\n", OBJECT(ps)->name, OBJECT(space)->name);

	if (OBJECT_TYPE(space, "map")) {
		struct map *m = space;

		if (go_map_sprite(ps, m, 0, 0, 0, ps->tileset, "Idle-S")
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
