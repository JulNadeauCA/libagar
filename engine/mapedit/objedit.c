/*	$Csoft: objedit.c,v 1.9 2003/06/10 19:13:03 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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
#include <engine/typesw.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>

#include "mapedit.h"

/* Create a new object. */
static void
create_obj(int argc, union evarg *argv)
{
	char name[OBJECT_NAME_MAX];
	char type[OBJECT_TYPE_MAX];
	struct tlist *objs_tl = argv[1].p;
	struct textbox *name_tb = argv[2].p;
	struct textbox *type_tb = argv[3].p;
	struct tlist_item *parent_it;
	void *nobj;
	int i;

	if ((parent_it = tlist_item_selected(objs_tl)) == NULL)
		parent_it = tlist_item_first(objs_tl);

	textbox_copy_string(type_tb, type, sizeof(type));
	if (type[0] == '\0') {
		text_msg("Error", "No object type specified");
		return;
	}
	for (i = 0; i < ntypesw; i++) {
		if (strcmp(typesw[i].type, type) == 0)
			break;
	}
	if (i == ntypesw) {
		text_msg("Error", "No such object type");
		return;
	}

	textbox_copy_string(name_tb, name, sizeof(name));
	if (name[0] == '\0') {
		text_msg("Error", "No object name specified");
		return;
	}

	nobj = Malloc(typesw[i].size);
	if (typesw[i].ops->init != NULL) {
		typesw[i].ops->init(nobj, name);
	} else {
		object_init(nobj, typesw[i].type, name, NULL);
	}
	object_attach(parent_it->p1, nobj);
}

/* Edit the selected objects. */
static void
edit_objs(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		struct object *ob = it->p1;

		if (!it->selected)
			continue;

		if (ob->ops->edit != NULL) {
			ob->ops->edit(ob);
		} else {
			text_msg("Error", "%s objects have no edit op",
			    ob->type);
		}
	}
}

/* Detach and the selected objects. */
static void
destroy_objs(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		struct object *ob = it->p1;

		if (!it->selected || it->p1 == world)
			continue;

		object_detach(ob->parent, ob);
		object_destroy(ob);
	}
}

/* Recursive function to display the object tree. */
static struct tlist_item *
find_objs(struct tlist *tl, struct object *pob, int depth)
{
	struct object *cob;
	struct tlist_item *it;

	it = tlist_insert_item(tl, OBJECT_ICON(pob), pob->name, pob);
	it->depth = depth;
	it->haschilds = !TAILQ_EMPTY(&pob->childs);
	if (it->haschilds && tlist_visible_childs(tl, it)) {
		TAILQ_FOREACH(cob, &pob->childs, cobjs)
			find_objs(tl, cob, depth+1);
	}
	return (it);
}

/* Update the object tree display. */
static void
poll_objs(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *pob = argv[1].p;

	lock_linkage();
	tlist_clear_items(tl);
	find_objs(tl, pob, 0);
	tlist_restore_selections(tl);
	unlock_linkage();
}

/* Create the object editor window. */
struct window *
objedit_window(void)
{
	struct window *win;
	struct vbox *vb;
	struct textbox *name_tb;
	struct button *create_bu, *edit_bu, *destroy_bu;
	struct combo *types_com;
	struct tlist *objs_tl;

	win = window_new("mapedit-objedit");
	window_set_caption(win, "Object editor");
	window_set_position(win, WINDOW_LOWER_RIGHT, 0);
	window_set_closure(win, WINDOW_HIDE);

	vb = vbox_new(win, VBOX_WFILL|VBOX_HFILL);
	{
		struct hbox *hb;
		int i;

		name_tb = textbox_new(vb, "New: ");
		types_com = combo_new(vb, "Type: ");
		for (i = 0; i < ntypesw; i++) {
			char label[TLIST_LABEL_MAX];

			strlcpy(label, typesw[i].type, sizeof(label));
			tlist_insert_item(types_com->list, NULL, label,
			    &typesw[i]);
		}
		
		hb = hbox_new(vb, HBOX_HOMOGENOUS|HBOX_WFILL);
		{
			create_bu = button_new(hb, "Create");
			edit_bu = button_new(hb, "Edit");
			destroy_bu = button_new(hb, "Destroy");
		}
		objs_tl = tlist_new(vb, TLIST_POLL|TLIST_MULTI|TLIST_TREE);
		event_new(objs_tl, "tlist-poll", poll_objs, "%p", world);
	}
	
	event_new(name_tb, "textbox-return", create_obj, "%p, %p, %p", objs_tl,
	    name_tb, types_com->tbox);
	event_new(types_com->tbox, "textbox-return", create_obj, "%p, %p, %p",
	    objs_tl, name_tb, types_com->tbox);
	event_new(create_bu, "button-pushed", create_obj, "%p, %p, %p", objs_tl,
	    name_tb, types_com->tbox);
	event_new(edit_bu, "button-pushed", edit_objs, "%p", objs_tl);
	event_new(destroy_bu, "button-pushed", destroy_objs, "%p", objs_tl);

	window_show(win);
	return (win);
}

