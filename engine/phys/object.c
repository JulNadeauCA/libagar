/*	$Csoft: phys_object.c,v 1.41 2004/09/12 05:57:24 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

#include <engine/widget/window.h>
#include <engine/widget/hbox.h>
#include <engine/widget/vbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/spinbutton.h>

#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "phys_object.h"

const struct version phys_object_ver = {
	"agar phys object",
	0, 0
};

const struct object_ops phys_object_ops = {
	phys_object_init,
	NULL,
	phys_object_destroy,
	phys_object_load,
	phys_object_save,
	phys_object_edit
};

struct phys_object *
phys_object_new(void *parent, const char *name)
{
	struct phys_object *po;

	po = Malloc(sizeof(struct phys_object), M_OBJECT);
	phys_object_init(po, name);
	object_attach(parent, po);
	return (po);
}

void
phys_object_init(void *obj, const char *name)
{
	struct phys_object *po = obj;

	object_init(po, "phys-object", name, &phys_object_ops);
	pthread_mutex_init(&po->lock, NULL);
}

void
phys_object_destroy(void *obj)
{
	/* nothing yet */
}

int
phys_object_load(void *obj, struct netbuf *buf)
{
	struct phys_object *phys_object = obj;

	if (version_read(buf, &phys_object_ver, NULL) != 0)
		return (-1);

	pthread_mutex_lock(&phys_object->lock);
	pthread_mutex_unlock(&phys_object->lock);
	return (0);
}

int
phys_object_save(void *obj, struct netbuf *buf)
{
	struct phys_object *phys_object = obj;

	version_write(buf, &phys_object_ver);

	pthread_mutex_lock(&phys_object->lock);
	pthread_mutex_unlock(&phys_object->lock);
	return (0);
}

struct window *
phys_object_edit(void *obj)
{
	struct phys_object *po = obj;
	struct window *win;
	struct vbox *vb;

	win = window_new(WINDOW_DETACH|WINDOW_NO_VRESIZE, NULL);
	window_set_caption(win, _("%s physical object"), OBJECT(po)->name);

	return (win);
}
