/*	$Csoft: objedit.c,v 1.24 2004/02/20 04:18:09 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004 CubeSoft Communications, Inc.
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

#include <string.h>

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
		text_msg(MSG_ERROR, _("No object type was specified."));
		return;
	}
	for (i = 0; i < ntypesw; i++) {
		if (strcmp(typesw[i].type, type) == 0)
			break;
	}
	if (i == ntypesw) {
		text_msg(MSG_ERROR, _("No such object type."));
		return;
	}

	textbox_copy_string(name_tb, name, sizeof(name));
	if (name[0] == '\0') {
		text_msg(MSG_ERROR, _("No object name was specified."));
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

enum {
	OBJEDIT_EDIT,
	OBJEDIT_EDIT_OBJ,
	OBJEDIT_LOAD,
	OBJEDIT_SAVE,
	OBJEDIT_DESTROY,
	OBJEDIT_MOVE_UP,
	OBJEDIT_MOVE_DOWN,
	OBJEDIT_DUP
};

/* Execute a generic operation on the selected objects. */
static void
invoke_op(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;
	int op = argv[2].i;

	TAILQ_FOREACH(it, &tl->items, items) {
		struct object *ob = it->p1;

		if (!it->selected)
			continue;

		switch (op) {
		case OBJEDIT_EDIT:
			if (ob->ops->edit != NULL) {
				mapedit_edit_objdata(ob);
			}
			break;
		case OBJEDIT_EDIT_OBJ:
			mapedit_edit_objgen(ob);
			break;
		case OBJEDIT_LOAD:
			if (object_load(ob) == -1) {
				text_msg(MSG_ERROR, "%s: %s", ob->name,
				    error_get());
			}
			break;
		case OBJEDIT_SAVE:
			if (object_save(ob) == -1) {
				text_msg(MSG_ERROR, "%s: %s", ob->name,
				    error_get());
			}
			break;
		case OBJEDIT_DUP:
			{
				struct object *dob;

				if ((dob = object_duplicate(ob)) == NULL) {
					text_msg(MSG_ERROR, "%s: %s", ob->name,
					    error_get());
				}
			}
			break;
		case OBJEDIT_MOVE_UP:
			object_move_up(ob);
			break;
		case OBJEDIT_MOVE_DOWN:
			object_move_down(ob);
			break;
		case OBJEDIT_DESTROY:
			if (it->p1 == world) {
				continue;
			}
			if (object_used(ob)) {
				text_msg(MSG_ERROR, "%s: %s", ob->name,
				    error_get());
				continue;
			}
			if (ob->flags & OBJECT_INDESTRUCTIBLE) {
				text_msg(MSG_ERROR,
				    _("The `%s' object is indestructible."),
				    ob->name);
				continue;
			}
			object_detach(ob);
			if (object_destroy(ob) == -1) {
				text_msg(MSG_ERROR, "%s: %s", ob->name,
				    error_get());
				continue;
			}
			if ((ob->flags & OBJECT_STATIC) == 0) {
				free(ob);
			}
			break;
		}
	}
}

/* Display the object tree. */
static struct tlist_item *
find_objs(struct tlist *tl, struct object *pob, int depth)
{
	char label[TLIST_LABEL_MAX];
	struct object *cob;
	struct tlist_item *it;

	strlcpy(label, pob->name, sizeof(label));
	if (pob->flags & OBJECT_DATA_RESIDENT) {
		strlcat(label, _(" (resident)"), sizeof(label));
	}
	it = tlist_insert_item(tl, OBJECT_ICON(pob), label, pob);
	it->depth = depth;

	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= TLIST_HAS_CHILDREN;
	}
	if ((it->flags & TLIST_HAS_CHILDREN) &&
	    tlist_visible_children(tl, it)) {
		TAILQ_FOREACH(cob, &pob->children, cobjs)
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
	struct button *create_bu, *edit_bu, *oedit_bu, *load_bu, *save_bu,
	    *destroy_bu, *dup_bu, *mvup_bu, *mvdown_bu;
	struct combo *types_com;
	struct tlist *objs_tl;

	win = window_new("mapedit-objedit");
	window_set_caption(win, _("Object editor"));
	window_set_position(win, WINDOW_LOWER_RIGHT, 0);
	window_set_closure(win, WINDOW_HIDE);

	vb = vbox_new(win, VBOX_WFILL|VBOX_HFILL);
	{
		struct hbox *hb;
		int i;

		name_tb = textbox_new(vb, _("Name: "));
		types_com = combo_new(vb, 0, _("Type: "));
		textbox_printf(types_com->tbox, "object");
		for (i = 0; i < ntypesw; i++) {
			char label[TLIST_LABEL_MAX];

			strlcpy(label, typesw[i].type, sizeof(label));
			tlist_insert_item(types_com->list, NULL, label,
			    &typesw[i]);
		}

		hb = hbox_new(vb, BOX_HOMOGENOUS|HBOX_WFILL);
		hbox_set_padding(hb, 1);
		{
			create_bu = button_new(hb, _("Create"));
			edit_bu = button_new(hb, _("Edit"));
			oedit_bu = button_new(hb, _("Edit obj"));
			load_bu = button_new(hb, _("Load"));
			save_bu = button_new(hb, _("Save"));
		}

		hb = hbox_new(vb, BOX_HOMOGENOUS|HBOX_WFILL);
		hbox_set_padding(hb, 1);
		{
			dup_bu = button_new(hb, _("Duplicate"));
			mvup_bu = button_new(hb, _("Move up"));
			mvdown_bu = button_new(hb, _("Move down"));
			destroy_bu = button_new(hb, _("Destroy"));
		}

		objs_tl = tlist_new(vb, TLIST_POLL|TLIST_MULTI|TLIST_TREE);
		tlist_prescale(objs_tl, "XXXXXXXXXXXXXXXX", 12);
		event_new(objs_tl, "tlist-poll", poll_objs, "%p", world);
	}

	event_new(name_tb, "textbox-return", create_obj, "%p, %p, %p", objs_tl,
	    name_tb, types_com->tbox);
	event_new(types_com->tbox, "textbox-return", create_obj, "%p, %p, %p",
	    objs_tl, name_tb, types_com->tbox);

	event_new(create_bu, "button-pushed", create_obj, "%p, %p, %p", objs_tl,
	    name_tb, types_com->tbox);

	event_new(edit_bu, "button-pushed", invoke_op, "%p, %i", objs_tl,
	    OBJEDIT_EDIT);
	event_new(oedit_bu, "button-pushed", invoke_op, "%p, %i", objs_tl,
	    OBJEDIT_EDIT_OBJ);
	event_new(load_bu, "button-pushed", invoke_op, "%p, %i", objs_tl,
	    OBJEDIT_LOAD);
	event_new(save_bu, "button-pushed", invoke_op, "%p, %i", objs_tl,
	    OBJEDIT_SAVE);

	event_new(dup_bu, "button-pushed", invoke_op, "%p, %i", objs_tl,
	    OBJEDIT_DUP);
	event_new(mvup_bu, "button-pushed", invoke_op, "%p, %i", objs_tl,
	    OBJEDIT_MOVE_UP);
	event_new(mvdown_bu, "button-pushed", invoke_op, "%p, %i", objs_tl,
	    OBJEDIT_MOVE_DOWN);
	event_new(destroy_bu, "button-pushed", invoke_op, "%p, %i", objs_tl,
	    OBJEDIT_DESTROY);

	window_show(win);
	return (win);
}

