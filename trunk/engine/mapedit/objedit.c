/*	$Csoft: objedit.c,v 1.50 2004/09/18 06:37:43 vedge Exp $	*/

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
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>

#include <string.h>

#include "mapedit.h"
#include "objedit.h"

struct objent {
	struct object *obj;
	struct window *win;
	TAILQ_ENTRY(objent) objs;
};
static TAILQ_HEAD(,objent) dobjs;
static TAILQ_HEAD(,objent) gobjs;

static void
create_obj(int argc, union evarg *argv)
{
	char name[OBJECT_NAME_MAX];
	char type[OBJECT_TYPE_MAX];
	struct tlist *objs_tl = argv[1].p;
	struct textbox *name_tb = argv[2].p;
	struct textbox *type_tb = argv[3].p;
	struct tlist_item *parent_it;
	struct object *pobj;
	void *nobj;
	int i;

	if ((parent_it = tlist_item_selected(objs_tl)) == NULL) {
		parent_it = tlist_item_first(objs_tl);
	}
	pobj = parent_it->p1;

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
		unsigned int nameno = 0;
		struct object *ch;
tryname:
		snprintf(name, sizeof(name), _("New %s #%d"), type, nameno);
		TAILQ_FOREACH(ch, &pobj->children, cobjs) {
			if (strcmp(ch->name, name) == 0)
				break;
		}
		if (ch != NULL) {
			nameno++;
			goto tryname;
		}
	}

	nobj = Malloc(typesw[i].size, M_OBJECT);
	if (typesw[i].ops->init != NULL) {
		typesw[i].ops->init(nobj, name);
	} else {
		object_init(nobj, typesw[i].type, name, NULL);
	}
	object_unlink_datafiles(nobj);
	object_attach(pobj, nobj);
}

enum {
	OBJEDIT_EDIT_DATA,
	OBJEDIT_EDIT_GENERIC,
	OBJEDIT_LOAD,
	OBJEDIT_SAVE,
	OBJEDIT_REINIT,
	OBJEDIT_DESTROY,
	OBJEDIT_MOVE_UP,
	OBJEDIT_MOVE_DOWN,
	OBJEDIT_DUP
};

static void
close_obj_generic(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct objent *oent = argv[1].p;

	view_detach(win);
	TAILQ_REMOVE(&gobjs, oent, objs);
	object_del_dep(&mapedit.pseudo, oent->obj);
	Free(oent, M_MAPEDIT);
}

static void
open_obj_generic(struct object *ob)
{
	struct objent *oent;
	
	TAILQ_FOREACH(oent, &gobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		window_show(oent->win);
		return;
	}
	
	object_add_dep(&mapedit.pseudo, ob);
	
	oent = Malloc(sizeof(struct objent), M_MAPEDIT);
	oent->obj = ob;
	oent->win = object_edit(ob);
	TAILQ_INSERT_HEAD(&gobjs, oent, objs);
	window_show(oent->win);

	event_new(oent->win, "window-close", close_obj_generic, "%p", oent);
}

static void
close_obj_data(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct objent *oent = argv[1].p;

	window_hide(win);
	event_post(NULL, oent->obj, "edit-close", NULL);
	view_detach(win);
	TAILQ_REMOVE(&dobjs, oent, objs);
	object_page_out(oent->obj, OBJECT_DATA);
	object_del_dep(&mapedit.pseudo, oent->obj);
	Free(oent, M_MAPEDIT);
}

void
objedit_open_data(struct object *ob)
{
	struct objent *oent;
	
	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		window_show(oent->win);
		return;
	}

	if (object_page_in(ob, OBJECT_DATA) == -1) {
		printf("%s: %s\n", ob->name, error_get());
		return;
	}
	object_add_dep(&mapedit.pseudo, ob);

	event_post(NULL, ob, "edit-open", NULL);
	
	oent = Malloc(sizeof(struct objent), M_MAPEDIT);
	oent->obj = ob;
	oent->win = ob->ops->edit(ob);
	TAILQ_INSERT_HEAD(&dobjs, oent, objs);
	window_show(oent->win);

	event_new(oent->win, "window-close", close_obj_data, "%p", oent);
}

static void
obj_op(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;
	int op = argv[2].i;

	TAILQ_FOREACH(it, &tl->items, items) {
		struct object *ob = it->p1;

		if (!it->selected)
			continue;

		switch (op) {
		case OBJEDIT_EDIT_DATA:
			if (ob->ops->edit != NULL) {
				objedit_open_data(ob);
			}
			break;
		case OBJEDIT_EDIT_GENERIC:
			open_obj_generic(ob);
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
			} else {
				text_tmsg(MSG_INFO, 1000,
				    _("Object `%s' was saved successfully."),
				    ob->name);
			}
			break;
		case OBJEDIT_DUP:
			{
				struct object *dob;

				if ((dob = object_duplicate(ob)) == NULL)
					text_msg(MSG_ERROR, "%s: %s", ob->name,
					    error_get());
			}
			break;
		case OBJEDIT_MOVE_UP:
			object_move_up(ob);
			break;
		case OBJEDIT_MOVE_DOWN:
			object_move_down(ob);
			break;
		case OBJEDIT_REINIT:
			if (it->p1 == world) {
				continue;
			}
			if (ob->ops->reinit != NULL) {
				ob->ops->reinit(ob);
			}
			break;
		case OBJEDIT_DESTROY:
			if (it->p1 == world) {
				continue;
			}
			if (object_in_use(ob)) {
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
			object_unlink_datafiles(ob);
			object_destroy(ob);
			if ((ob->flags & OBJECT_STATIC) == 0) {
				Free(ob, M_OBJECT);
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
	SDL_Surface *icon;

	strlcpy(label, pob->name, sizeof(label));
	if (pob->flags & OBJECT_DATA_RESIDENT) {
		strlcat(label, _(" (resident)"), sizeof(label));
	}
	it = tlist_insert_item(tl, object_icon(pob), label, pob);
	it->depth = depth;

	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= TLIST_HAS_CHILDREN;
		if (object_root(pob) == pob)
			it->flags |= TLIST_VISIBLE_CHILDREN;
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
	struct combo *types_com;
	struct tlist *objs_tl;
	struct toolbar *tbar;

	win = window_new(0, "mapedit-objedit");
	window_set_caption(win, _("Object editor"));
	window_set_position(win, WINDOW_LOWER_RIGHT, 0);

	vb = vbox_new(win, VBOX_WFILL|VBOX_HFILL);
	{
		int i;

		name_tb = textbox_new(vb, _("Name: "));
		types_com = combo_new(vb, 0, _("Type: "));
		textbox_printf(types_com->tbox, "object");
		for (i = ntypesw-1; i >= 0; i--) {
			char label[TLIST_LABEL_MAX];

			strlcpy(label, typesw[i].type, sizeof(label));
			tlist_insert_item(types_com->list,
			    typesw[i].icon >= 0 ? ICON(typesw[i].icon) : NULL,
			    label, &typesw[i]);
		}
	
		tbar = toolbar_new(vb, TOOLBAR_HORIZ, 1);

		objs_tl = tlist_new(vb, TLIST_POLL|TLIST_MULTI|TLIST_TREE);
		tlist_prescale(objs_tl, "XXXXXXXXXXXXXXXX", 12);
		event_new(objs_tl, "tlist-poll", poll_objs, "%p", world);
		event_new(objs_tl, "tlist-dblclick", obj_op, "%p, %i", objs_tl,
		    OBJEDIT_EDIT_DATA);

		toolbar_add_button(tbar, 0, ICON(OBJCREATE_ICON), 0, 0,
		    create_obj, "%p, %p, %p", objs_tl, name_tb,
		    types_com->tbox);

		toolbar_add_button(tbar, 0, ICON(OBJEDIT_ICON), 0, 0,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_DATA);
		toolbar_add_button(tbar, 0, ICON(OBJGENEDIT_ICON), 0, 0,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_GENERIC);

		toolbar_add_button(tbar, 0, ICON(OBJLOAD_ICON), 0, 0,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_LOAD);
		toolbar_add_button(tbar, 0, ICON(OBJSAVE_ICON), 0, 0,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_SAVE);

		toolbar_add_button(tbar, 0, ICON(OBJDUP_ICON), 0, 0,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_DUP);
		toolbar_add_button(tbar, 0, ICON(OBJMOVEUP_ICON), 0, 0,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_MOVE_UP);
		toolbar_add_button(tbar, 0, ICON(OBJMOVEDOWN_ICON), 0, 0,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_MOVE_DOWN);
		toolbar_add_button(tbar, 0, ICON(OBJREINIT_ICON), 0, 0,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_REINIT);
		toolbar_add_button(tbar, 0, ICON(TRASH_ICON), 0, 0,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_DESTROY);
	}

	event_new(name_tb, "textbox-return", create_obj, "%p, %p, %p", objs_tl,
	    name_tb, types_com->tbox);
	event_new(types_com->tbox, "textbox-return", create_obj, "%p, %p, %p",
	    objs_tl, name_tb, types_com->tbox);

	window_show(win);
	return (win);
}

void
objedit_init(void)
{
	TAILQ_INIT(&dobjs);
	TAILQ_INIT(&gobjs);
}

void
objedit_destroy(void)
{
	struct objent *oent, *noent;

	for (oent = TAILQ_FIRST(&dobjs);
	     oent != TAILQ_END(&dobjs);
	     oent = noent) {
		noent = TAILQ_NEXT(oent, objs);
		Free(oent, M_MAPEDIT);
	}
	for (oent = TAILQ_FIRST(&gobjs);
	     oent != TAILQ_END(&gobjs);
	     oent = noent) {
		noent = TAILQ_NEXT(oent, objs);
		Free(oent, M_MAPEDIT);
	}
}

