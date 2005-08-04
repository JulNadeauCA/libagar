/*	$Csoft: objsel.c,v 1.2 2005/08/04 07:22:18 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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
#include <engine/view.h>

#include "objsel.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

struct objsel *
objsel_new(void *parent, int flags, void *pobj, void *root,
    const char *fmt, ...)
{
	char label[LABEL_MAX];
	struct objsel *os;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	os = Malloc(sizeof(struct objsel), M_OBJECT);
	objsel_init(os, label, flags, pobj, root);
	object_attach(parent, os);
	return (os);
}

/* Select an object in the list. */
struct tlist_item *
objsel_select(struct objsel *os, void *p)
{
	struct tlist_item *it;

	return (combo_select_pointer(&os->com, p));
}

static void
find_objs(struct objsel *os, struct tlist *tl, struct object *pob, int depth)
{
	struct object *cob;
	struct tlist_item *it;
	SDL_Surface *icon;

	it = tlist_insert(tl, object_icon(pob), "%s%s", pob->name,
	    (pob->flags & OBJECT_DATA_RESIDENT) ? " (resident)" : "");
	it->depth = depth;
	it->class = "object";
	it->p1 = pob;

	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= TLIST_HAS_CHILDREN;
		if (object_root(pob) == pob)
			it->flags |= TLIST_VISIBLE_CHILDREN;
	}
	if ((it->flags & TLIST_HAS_CHILDREN)) {
		TAILQ_FOREACH(cob, &pob->children, cobjs)
			find_objs(os, tl, cob, depth+1);
	}
}

static void
poll_objs(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct objsel *os = argv[1].p;
	struct tlist_item *it;

	tlist_clear_items(tl);
	lock_linkage();
	find_objs(os, tl, os->root, 0);
	unlock_linkage();
	tlist_restore_selections(tl);
}

static void
selected_obj(int argc, union evarg *argv)
{
	struct combo *com = argv[0].p;
	struct objsel *os = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct widget_binding *objectb;
	void **object;
	
	objectb = widget_get_binding(os, "object", &object);

	if (*object != NULL) {
		if (os->flags & OBJSEL_PAGE_DATA)
			object_page_out(*object, OBJECT_DATA);
		if (os->flags & OBJSEL_PAGE_GFX)
			object_page_out(*object, OBJECT_GFX);
		if (os->flags & OBJSEL_PAGE_AUDIO)
			object_page_out(*object, OBJECT_AUDIO);
		
		object_del_dep(os->pobj, *object);
	}

	*object = it->p1;

	object_add_dep(os->pobj, *object);
	if (os->flags & OBJSEL_PAGE_DATA)
		object_page_in(*object, OBJECT_DATA);
	if (os->flags & OBJSEL_PAGE_GFX)
		object_page_in(*object, OBJECT_GFX);
	if (os->flags & OBJSEL_PAGE_AUDIO)
		object_page_in(*object, OBJECT_AUDIO);

	widget_binding_unlock(objectb);
}

static void
bound(int argc, union evarg *argv)
{
	struct objsel *os = argv[0].p;
	struct widget_binding *b = argv[1].p;

	if (strcmp(b->name, "object") == 0) {
		void **object = b->p1;
	
		if (*object != NULL) {
			dprintf("sel %s\n", OBJECT(*object)->name);
			combo_select_pointer(&os->com, *object);
		}
	}
}

void
objsel_init(struct objsel *os, const char *label, int flags, void *pobj,
    void *root)
{
	combo_init(&os->com, label, COMBO_POLL|COMBO_TREE);
	object_set_type(os, "combo.objsel");

	os->flags = flags;
	os->pobj = pobj;
	os->root = root;
	os->type_mask[0] = '\0';

	widget_bind(os, "object", WIDGET_POINTER, &os->object);

	event_new(os->com.list, "tlist-poll", poll_objs, "%p", os);
	event_new(&os->com, "combo-selected", selected_obj, "%p", os);
	event_new(os, "widget-bound", bound, NULL);
}

void
objsel_mask_type(struct objsel *os, const char *type)
{
	strlcpy(os->type_mask, type, sizeof(os->type_mask));
}
