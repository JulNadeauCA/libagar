/*	$Csoft: perso.c,v 1.37 2003/10/13 23:48:58 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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
#include <engine/map.h>
#include <engine/rootmap.h>
#include <engine/perso.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/hbox.h>
#include <engine/widget/vbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/spinbutton.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

const struct version perso_ver = {
	"agar personage",
	2, 0
};

const struct object_ops perso_ops = {
	perso_init,
	NULL,
	perso_destroy,
	perso_load,
	perso_save,
	perso_edit
};

#ifdef DEBUG
#define	DEBUG_STATE	0x01
#define DEBUG_POSITION	0x02

int	perso_debug = DEBUG_STATE|DEBUG_POSITION;
#define engine_debug	perso_debug
#endif

struct perso *
perso_new(void *parent, const char *name)
{
	struct perso *pers;

	pers = Malloc(sizeof(struct perso));
	perso_init(pers, name);
	object_attach(parent, pers);
	return (pers);
}

void
perso_init(void *obj, const char *name)
{
	struct perso *pers = obj;

	object_init(pers, "perso", name, &perso_ops);
	pthread_mutex_init(&pers->lock, NULL);
	pers->name[0] = '\0';
	pers->flags = 0;
	pers->level = 1;
	pers->exp = 0;
	pers->age = 5;
	pers->seed = 0;			/* TODO arc4random */
	pers->hp = pers->maxhp = 10;
	pers->mp = pers->maxmp = 0;
	pers->nzuars = 73;
}

void
perso_destroy(void *obj)
{
	/* nothing yet */
}

int
perso_load(void *obj, struct netbuf *buf)
{
	struct perso *perso = obj;

	if (version_read(buf, &perso_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&perso->lock);
	copy_string(perso->name, buf, sizeof(perso->name));
	perso->flags = read_uint32(buf);
	perso->level = read_sint32(buf);
	perso->exp = read_uint32(buf);
	perso->age = (int)read_uint32(buf);
	perso->seed = read_uint32(buf);
	perso->maxhp = (int)read_uint32(buf);
	perso->hp = (int)read_uint32(buf);
	perso->maxmp = (int)read_uint32(buf);
	perso->mp = (int)read_uint32(buf);
	perso->nzuars = (unsigned int)read_uint32(buf);
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
	write_sint32(buf, perso->level);
	write_uint32(buf, perso->exp);
	write_uint32(buf, perso->age);
	write_uint32(buf, perso->seed);
	write_uint32(buf, (Uint32)perso->maxhp);
	write_uint32(buf, (Uint32)perso->hp);
	write_uint32(buf, (Uint32)perso->maxmp);
	write_uint32(buf, (Uint32)perso->mp);
	write_uint32(buf, (Uint32)perso->nzuars);
	pthread_mutex_unlock(&perso->lock);
	return (0);
}

struct window *
perso_edit(void *obj)
{
	struct perso *pers = obj;
	struct window *win;
	struct vbox *vb;

	win = window_new(NULL);
	window_set_caption(win, _("%s character"), OBJECT(pers)->name);
	window_set_closure(win, WINDOW_DETACH);

	vb = vbox_new(win, VBOX_WFILL|VBOX_HFILL);
	{
		struct textbox *tb;
		struct spinbutton *sbu;
		struct hbox *hb;

		tb = textbox_new(vb, _("Name: "));
		widget_bind(tb, "string", WIDGET_STRING, pers->name,
		    sizeof(pers->name));

		sbu = spinbutton_new(vb, _("Level: "));
		widget_bind(sbu, "value", WIDGET_SINT32, &pers->level);

		sbu = spinbutton_new(vb, _("Experience: "));
		widget_bind(sbu, "value", WIDGET_UINT32, &pers->exp);

		sbu = spinbutton_new(vb, _("Age: "));
		widget_bind(sbu, "value", WIDGET_INT, &pers->age);

		sbu = spinbutton_new(vb, _("Zuars: "));
		widget_bind(sbu, "value", WIDGET_UINT32, &pers->nzuars);

		hb = hbox_new(vb, HBOX_HOMOGENOUS|HBOX_WFILL);
		{
			sbu = spinbutton_new(hb, _("HP: "));
			widget_bind(sbu, "value", WIDGET_INT, &pers->hp);
			
			sbu = spinbutton_new(hb, " / ");
			widget_bind(sbu, "value", WIDGET_INT, &pers->maxhp);
		}
		
		hb = hbox_new(vb, HBOX_HOMOGENOUS|HBOX_WFILL);
		{
			sbu = spinbutton_new(hb, _("MP: "));
			widget_bind(sbu, "value", WIDGET_INT, &pers->mp);
			
			sbu = spinbutton_new(hb, " / ");
			widget_bind(sbu, "value", WIDGET_INT, &pers->maxmp);
		}
	}
	return (win);
}
