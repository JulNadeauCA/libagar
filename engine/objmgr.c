/*	$Csoft: objmgr.c,v 1.45 2005/09/18 05:40:48 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/config.h>
#include <engine/timeout.h>

#include <engine/map/mapedit.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>
#include <engine/widget/menu.h>
#include <engine/widget/bitmap.h>
#include <engine/widget/label.h>
#include <engine/widget/separator.h>
#include <engine/widget/file_dlg.h>
#include <engine/widget/notebook.h>

#include <engine/monitor/monitor.h>

#ifdef NETWORK
#include <engine/rcs.h>
#endif

#include <string.h>
#include <ctype.h>

#include "objmgr.h"

struct objent {
	struct object *obj;
	struct window *win;
	TAILQ_ENTRY(objent) objs;
};
static TAILQ_HEAD(,objent) dobjs;
static TAILQ_HEAD(,objent) gobjs;
static int edit_on_create = 1;
static void *current_pobj = NULL;

#ifdef NETWORK
static struct timeout repo_timeout;
#endif

int objmgr_exiting = 0;
#ifdef DEBUG
int objmgr_hexdiff = 0;
#endif

static void
create_obj(int argc, union evarg *argv)
{
	char name[OBJECT_NAME_MAX];
	struct object_type *t = argv[1].p;
	struct textbox *name_tb = argv[2].p;
	struct tlist *objs_tl = argv[3].p;
	struct window *dlg_win = argv[4].p;
	struct tlist_item *it;
	struct object *pobj;
	void *nobj;

	if ((it = tlist_selected_item(objs_tl)) != NULL) {
		pobj = it->p1;
	} else {
		pobj = world;
	}
	textbox_copy_string(name_tb, name, sizeof(name));
	view_detach(dlg_win);

	if (name[0] == '\0') {
		u_int nameno = 0;
		struct object *ch;
		char tname[OBJECT_TYPE_MAX], *s;
	
		if ((s = strrchr(t->type, '.')) != NULL && s[1] != '\0') {
			strlcpy(tname, &s[1], sizeof(tname));
		} else {
			strlcpy(tname, t->type, sizeof(tname));
		}
		tname[0] = (char)toupper(tname[0]);
tryname:
		snprintf(name, sizeof(name), "%s #%d", tname, nameno);
		TAILQ_FOREACH(ch, &pobj->children, cobjs) {
			if (strcmp(ch->name, name) == 0)
				break;
		}
		if (ch != NULL) {
			nameno++;
			goto tryname;
		}
	}

	nobj = Malloc(t->size, M_OBJECT);
	if (t->ops->init != NULL) {
		t->ops->init(nobj, name);
	} else {
		object_init(nobj, t->type, name, NULL);
	}
	object_attach(pobj, nobj);
	object_unlink_datafiles(nobj);

	event_post(NULL, nobj, "edit-create", NULL);
	
	if (edit_on_create &&
	    t->ops->edit != NULL)
		objmgr_open_data(nobj, 1);
}

enum {
	OBJEDIT_EDIT_DATA,
	OBJEDIT_EDIT_GENERIC,
	OBJEDIT_LOAD,
	OBJEDIT_SAVE,
	OBJEDIT_SAVE_ALL,
	OBJEDIT_EXPORT,
	OBJEDIT_REINIT,
	OBJEDIT_DESTROY,
	OBJEDIT_MOVE_UP,
	OBJEDIT_MOVE_DOWN,
	OBJEDIT_DUP,
	OBJEDIT_RCS_UPDATE,
	OBJEDIT_RCS_UPDATE_ALL,
	OBJEDIT_RCS_COMMIT,
	OBJEDIT_RCS_COMMIT_ALL,
	OBJEDIT_RCS_IMPORT,
	OBJEDIT_RCS_IMPORT_ALL
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

void
objmgr_open_generic(struct object *ob)
{
	struct objent *oent;

	TAILQ_FOREACH(oent, &gobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		window_show(oent->win);
		view->focus_win = oent->win;
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
close_object(struct objent *oent, struct window *win, int save)
{
	window_hide(win);
	event_post(NULL, oent->obj, "edit-close", NULL);
	view_detach(win);
	TAILQ_REMOVE(&dobjs, oent, objs);

	if (!save) {
		objmgr_exiting = 1;
	}
	object_page_out(oent->obj, OBJECT_DATA);
	object_page_out(oent->obj, OBJECT_GFX);

	objmgr_exiting = 0;
	object_del_dep(&mapedit.pseudo, oent->obj);
	Free(oent, M_MAPEDIT);
}

static void
close_object_cb(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;
	struct objent *oent = argv[2].p;
	int save = argv[3].i;

	close_object(oent, win, save);
}

static void
close_object_dlg(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct objent *oent = argv[1].p;

	if (object_changed(oent->obj)) {
		struct button *bOpts[3];
		struct window *wDlg;

		wDlg = text_prompt_options(bOpts, 3, _("Save changes to %s?"),
		    OBJECT(oent->obj)->name);
		window_attach(win, wDlg);
		{
			button_printf(bOpts[0], _("Save"));
			event_new(bOpts[0], "button-pushed",
			    close_object_cb, "%p,%p,%i", win, oent, 1);
			widget_focus(bOpts[0]);

			button_printf(bOpts[1], _("Discard"));
			event_new(bOpts[1], "button-pushed",
			    close_object_cb, "%p,%p,%i", win, oent, 0);

			button_printf(bOpts[2], _("Cancel"));
			event_new(bOpts[2], "button-pushed",
			    window_generic_detach, "%p", wDlg);
		}
	} else {
		close_object(oent, win, 1);
	}
}

void
objmgr_open_data(void *p, int new)
{
	struct object *ob = p;
	struct objent *oent;
	struct window *win;

	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		window_show(oent->win);
		view->focus_win = oent->win;
		return;
	}
	
	if (ob->ops->edit == NULL)
		return;

	if ((ob->flags & OBJECT_NON_PERSISTENT) == 0) {
		if (ob->data_used == 0 &&
		    object_load_data(ob) == -1) {
			if (new) {
				dprintf("%s: new object\n", ob->name);
				ob->flags |= OBJECT_DATA_RESIDENT;

				if (object_save(ob) == -1) {
					text_msg(MSG_ERROR, "%s: %s", ob->name,
					    error_get());
					return;
				}
			} else {
				text_msg(MSG_ERROR, "%s: %s", ob->name,
				    error_get());
				return;
			}
		}
		if (++ob->data_used > OBJECT_DEP_MAX)
			ob->data_used = OBJECT_DEP_MAX;
	}
	if (object_page_in(ob, OBJECT_GFX) == -1) {
		text_msg(MSG_ERROR, "%s (gfx): %s", ob->name, error_get());
		goto fail_data;
	}
	object_add_dep(&mapedit.pseudo, ob);

	if ((win = ob->ops->edit(ob)) == NULL) {
		goto fail_gfx;
	}
	event_post(NULL, ob, "edit-open", NULL);
	
	oent = Malloc(sizeof(struct objent), M_MAPEDIT);
	oent->obj = ob;
	oent->win = win;
	TAILQ_INSERT_HEAD(&dobjs, oent, objs);
	event_new(win, "window-close", close_object_dlg, "%p", oent);
	window_show(win);
	return;
fail_gfx:
	object_del_dep(&mapedit.pseudo, ob);
	object_page_out(ob, OBJECT_GFX);
fail_data:
	object_page_out(ob, OBJECT_DATA);
	return;
}

static void
export_object(int argc, union evarg *argv)
{
	char save_path[MAXPATHLEN];
	struct object *ob = argv[1].p;
	struct window *win = argv[2].p;
	char *path = argv[3].s;
	char *pfx_save = ob->save_pfx;
	int paged_in = 0;

	if ((ob->flags & OBJECT_DATA_RESIDENT) == 0) {
		if (object_load_data(ob) == -1) {
			/* XXX hack */
			ob->flags |= OBJECT_DATA_RESIDENT;
		}
		paged_in = 1;
	}

	prop_copy_string(config, "save-path", save_path, sizeof(save_path));
	prop_set_string(config, "save-path", "%s", path);
	ob->save_pfx = NULL;

	if (object_save_all(ob) == -1) {
		text_msg(MSG_ERROR, "%s: %s", ob->name, error_get());
	} else {
		text_tmsg(MSG_INFO, 1000,
		    _("Object `%s' was exported successfully."), ob->name);
	}

	prop_set_string(config, "save-path", "%s", save_path);
	ob->save_pfx = pfx_save;
	view_detach(win);
	
	if (paged_in)
		object_free_data(ob);
}

void
objmgr_save_to(void *p)
{
	struct object *ob = p;
	char path[FILENAME_MAX];
	struct window *win;
	struct AGFileDlg *fdg;

	strlcpy(path, ob->name, sizeof(path));
	strlcat(path, ".", sizeof(path));
	strlcat(path, ob->type, sizeof(path));

	win = window_new(0, NULL);
	window_set_caption(win, _("Save %s to..."), ob->name);
	fdg = file_dlg_new(win, 0, prop_get_string(config, "save-path"), path);
	event_new(fdg, "file-validated", export_object, "%p,%p", ob, win);
	event_new(fdg, "file-cancelled", window_generic_detach, "%p", win);

	window_show(win);
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
				objmgr_open_data(ob, 0);
			} else {
				text_tmsg(MSG_ERROR, 750,
				    _("Object `%s' has no edit operation."),
				    ob->name);
			}
			break;
		case OBJEDIT_EDIT_GENERIC:
			objmgr_open_generic(ob);
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
		case OBJEDIT_SAVE_ALL:
			if (object_save_all(ob) == -1) {
				text_msg(MSG_ERROR, "%s: %s", ob->name,
				    error_get());
			} else {
				text_tmsg(MSG_INFO, 1000,
				    _("Object `%s' was saved successfully."),
				    ob->name);
			}
			break;
		case OBJEDIT_EXPORT:
			if (ob->flags & OBJECT_NON_PERSISTENT) {
				error_set(
				    _("The `%s' object is non-persistent."),
				    ob->name);
			} else {
				objmgr_save_to(ob);
			}
			break;
		case OBJEDIT_DUP:
			{
				struct object *dob;

				if (ob == world ||
				    ob->flags & OBJECT_NON_PERSISTENT) {
					text_msg(MSG_ERROR,
					    _("%s: cannot duplicate."),
					    ob->name);
					break;
				}
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
#ifdef NETWORK
		case OBJEDIT_RCS_IMPORT:
			if (object_save(ob) == -1 ||
			    rcs_import(ob) == -1) {
				text_msg(MSG_ERROR, "%s: %s", ob->name,
				    error_get());
			}
			break;
		case OBJEDIT_RCS_IMPORT_ALL:
			rcs_import_all(ob);
			break;
		case OBJEDIT_RCS_COMMIT:
			if (object_save(ob) == -1 ||
			    rcs_commit(ob) == -1) {
				text_msg(MSG_ERROR, "%s: %s", ob->name,
				    error_get());
			}
			break;
		case OBJEDIT_RCS_COMMIT_ALL:
			rcs_commit_all(ob);
			break;
		case OBJEDIT_RCS_UPDATE:
			if (object_save(ob) == -1) {
				text_msg(MSG_ERROR, _("Save failed: %s: %s"),
				    ob->name, error_get());
				break;
			}
			if (rcs_update(ob) == 0) {
				if (object_load(ob) == -1) {
					text_msg(MSG_ERROR, "%s: %s",
					    ob->name, error_get());
				}
			} else {
				text_msg(MSG_ERROR, "%s", error_get());
			}
			break;
		case OBJEDIT_RCS_UPDATE_ALL:
			rcs_update_all(ob);
			break;
#endif /* NETWORK */
		}
	}
}

static void
generic_save(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;

	if (object_save(ob) == -1) {
		text_msg(MSG_ERROR, _("Save failed: %s: %s"), ob->name,
		    error_get());
	} else {
		text_tmsg(MSG_INFO, 1000, _("Object `%s' saved successfully."),
		    ob->name);
	}
}

static void
generic_save_to(int argc, union evarg *argv)
{
	objmgr_save_to(argv[1].p);
}

void
objmgr_generic_menu(void *menup, void *obj)
{
	struct AGMenuItem *pitem = menup;

	menu_action(pitem, _("Save"), OBJSAVE_ICON,
	    generic_save, "%p", obj);
	menu_action(pitem, _("Save to..."), OBJSAVE_ICON,
	    generic_save_to, "%p", obj);
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
	it->class = "object";

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
	struct object *dob = argv[2].p;
	struct tlist_item *it;

	lock_linkage();
	tlist_clear_items(tl);
	find_objs(tl, pob, 0);
	tlist_restore_selections(tl);
	unlock_linkage();

	if (tlist_selected_item(tl) == NULL) {
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->p1 == dob)
				break;
		}
		if (it != NULL)
			tlist_select(tl, it);
	}
}

static void
load_object(int argc, union evarg *argv)
{
	struct object *o = argv[1].p;

	if (object_load(o) == -1) {
		text_msg(MSG_ERROR, "%s: %s", OBJECT(o)->name, error_get());
	}
}

static void
save_object(int argc, union evarg *argv)
{
	struct object *ob = argv[1].p;

	if (object_save(ob) == -1) {
		text_msg(MSG_ERROR, "%s: %s", ob->name, error_get());
	} else {
		text_tmsg(MSG_INFO, 1000, _("Object `%s' saved successfully."),
		    ob->name);
	}
}

static void
exit_program(int argc, union evarg *argv)
{
	SDL_Event ev;

	ev.type = SDL_QUIT;
	SDL_PushEvent(&ev);
}

static void
show_preferences(int argc, union evarg *argv)
{
	if (!config->settings->visible) {
		window_show(config->settings);
	} else {
		window_focus(config->settings);
	}
}

static void
create_obj_dlg(int argc, union evarg *argv)
{
	struct window *win;
	struct object_type *t = argv[1].p;
	struct window *pwin = argv[2].p;
	struct tlist *pobj_tl;
	struct box *bo;
	struct textbox *tb;
	struct checkbox *cb;

	win = window_new(WINDOW_NO_CLOSE|WINDOW_NO_MINIMIZE, NULL);
	window_set_caption(win, _("New %s object"), t->type);
	window_set_position(win, WINDOW_CENTER, 1);

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		label_new(bo, LABEL_STATIC, _("Type: %s"), t->type);
		tb = textbox_new(bo, _("Name: "));
		widget_focus(tb);
	}

	separator_new(win, SEPARATOR_HORIZ);

	bo = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	box_set_padding(bo, 0);
	box_set_spacing(bo, 0);
	{
		label_new(bo, LABEL_STATIC, _("Parent object:"));

		pobj_tl = tlist_new(bo, TLIST_POLL|TLIST_TREE);
		tlist_prescale(pobj_tl, "XXXXXXXXXXXXXXXXXXX", 5);
		widget_bind(pobj_tl, "selected", WIDGET_POINTER, &current_pobj);
		event_new(pobj_tl, "tlist-poll", poll_objs, "%p,%p", world,
		    current_pobj);
	}

	bo = box_new(win, BOX_VERT, BOX_WFILL);
	{
		cb = checkbox_new(win, _("Edit now"));
		widget_bind(cb, "state", WIDGET_INT, &edit_on_create);
	}

	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
	{
		struct button *btn;
	
		btn = button_new(bo, _("OK"));
		event_new(tb, "textbox-return", create_obj, "%p,%p,%p,%p", t,
		    tb, pobj_tl, win);
		event_new(btn, "button-pushed", create_obj, "%p,%p,%p,%p", t,
		    tb, pobj_tl, win);
		
		btn = button_new(bo, _("Cancel"));
		event_new(btn, "button-pushed", window_generic_detach, "%p",
		    win);
	}

	window_attach(pwin, win);
	window_show(win);
}

#ifdef NETWORK

static void
update_repo_listing(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;

	if (!rcs) {
		text_msg(MSG_ERROR, _("RCS is currently disabled."));
		return;
	}
	if (rcs_connect() == -1 ||
	    rcs_list(tl) == -1) {
		text_msg(MSG_ERROR, "%s", error_get());
	}
	rcs_disconnect();
}

static void
update_from_repo(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct tlist_item *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected)
			continue;

		if (rcs_checkout(it->text) == -1)
			text_msg(MSG_ERROR, "%s: %s", it->text, error_get());
	}
}

#endif /* NETWORK */

/* Create the object editor window. */
struct window *
objmgr_window(void)
{
	struct window *win;
	struct vbox *vb;
	struct textbox *name_tb;
	struct tlist *objs_tl;
	struct AGMenu *me;
	struct AGMenuItem *mi, *mi_objs;
	struct notebook *nb;
	struct notebook_tab *ntab;

	win = window_new(0, "objmgr");
	window_set_caption(win, _("Object manager"));
	window_set_position(win, WINDOW_UPPER_LEFT, 0);
	
	objs_tl = Malloc(sizeof(struct tlist), M_OBJECT);
	tlist_init(objs_tl, TLIST_POLL|TLIST_MULTI|TLIST_TREE);
	tlist_prescale(objs_tl, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 10);
	event_new(objs_tl, "tlist-poll", poll_objs, "%p,%p", world, NULL);
	event_new(objs_tl, "tlist-dblclick", obj_op, "%p, %i", objs_tl,
	    OBJEDIT_EDIT_DATA);

	me = menu_new(win);
	mi = menu_add_item(me, _("File"));
	{
		int i;

		mi_objs = menu_action(mi, _("New object"), OBJCREATE_ICON,
		    NULL, NULL);
		for (i = ntypesw-1; i >= 0; i--) {
			char label[32];
			struct object_type *t = &typesw[i];

			strlcpy(label, t->type, sizeof(label));
			label[0] = (char)toupper((int)label[0]);
			menu_action(mi_objs, label, t->icon,
			    create_obj_dlg, "%p,%p", t, win);
		}

		menu_separator(mi);

		menu_action(mi, _("Load full state"), OBJLOAD_ICON,
		    load_object, "%p", world);
		menu_action(mi, _("Save full state"), OBJSAVE_ICON,
		    save_object, "%p", world);
		
		menu_separator(mi);
		
		menu_action(mi, _("Exit"), -1, exit_program, NULL);
	}
	
	mi = menu_add_item(me, _("Edit"));
	{
		menu_action(mi, _("Edit object..."), OBJEDIT_ICON,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_DATA);
		menu_action(mi, _("Edit object information..."),
		    OBJGENEDIT_ICON,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_GENERIC);
		
		menu_separator(mi);
			
		menu_action(mi, _("Duplicate"), OBJDUP_ICON,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_DUP);
		menu_action_kb(mi, _("Move up"), OBJMOVEUP_ICON,
		    SDLK_u, KMOD_SHIFT, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_MOVE_UP);
		menu_action_kb(mi, _("Move down"), OBJMOVEDOWN_ICON,
		    SDLK_d, KMOD_SHIFT, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_MOVE_DOWN);
	
		menu_separator(mi);

		menu_action(mi, _("Preferences..."), -1, show_preferences,
		    NULL);
	}

#ifdef NETWORK
	mi = menu_add_item(me, _("Repository"));
	{
		menu_action(mi, _("Commit"),
		    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_COMMIT);

		menu_action(mi, _("Update"),
		    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_UPDATE);

		menu_action(mi, _("Import"),
		    OBJSAVE_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_IMPORT);

		menu_separator(mi);

		menu_action(mi, _("Commit all"),
		    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_COMMIT_ALL);

		menu_action(mi, _("Update all"),
		    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_UPDATE_ALL);
			
		
		menu_action(mi, _("Import all"),
		    OBJSAVE_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_IMPORT_ALL);
	}
#endif /* NETWORK */

#ifdef DEBUG
	mi = menu_add_item(me, _("Debug"));
	monitor_menu(mi);
#endif /* DEBUG */

	nb = notebook_new(win, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
	ntab = notebook_add_tab(nb, _("Working copy"), BOX_VERT);
	{
		struct AGMenuItem *mi, *mi2;

		object_attach(ntab, objs_tl);

		mi = tlist_set_popup(objs_tl, "object");
		{
			menu_action(mi, _("Edit object..."), OBJEDIT_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_DATA);
			menu_action(mi, _("Edit object information..."),
			    OBJGENEDIT_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_GENERIC);

			menu_separator(mi);
			
			menu_action(mi, _("Load"), OBJLOAD_ICON, obj_op,
			    "%p, %i", objs_tl, OBJEDIT_LOAD);
			menu_action(mi, _("Save"), OBJSAVE_ICON, obj_op,
			    "%p, %i", objs_tl, OBJEDIT_SAVE);
			menu_action(mi, _("Save all"), OBJSAVE_ICON, obj_op,
			    "%p, %i", objs_tl, OBJEDIT_SAVE_ALL);
			menu_action(mi, _("Save to..."), OBJSAVE_ICON, obj_op,
			    "%p, %i", objs_tl, OBJEDIT_EXPORT);

#ifdef NETWORK
			if (rcs) {
				menu_separator(mi);
			
				mi2 = menu_action(mi, _("Repository"),
				    OBJLOAD_ICON, NULL, NULL);

				menu_action(mi2, _("Commit"),
				    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_COMMIT);
				
				menu_action(mi2, _("Update"),
				    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_UPDATE);
				
				menu_action(mi2, _("Import"),
				    OBJSAVE_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_IMPORT);
		
				menu_separator(mi2);
				
				menu_action(mi2, _("Commit all"),
				    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_COMMIT_ALL);

				menu_action(mi2, _("Update all"),
				    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_UPDATE_ALL);
			
				menu_action(mi2, _("Import all"),
				    OBJSAVE_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_IMPORT_ALL);
			}
#endif /* NETWORK */
			menu_separator(mi);
			
			menu_action(mi, _("Duplicate"), OBJDUP_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_DUP);
			menu_action_kb(mi, _("Move up"), OBJMOVEUP_ICON,
			    SDLK_u, KMOD_SHIFT, obj_op, "%p, %i", objs_tl,
			    OBJEDIT_MOVE_UP);
			menu_action_kb(mi, _("Move down"), OBJMOVEDOWN_ICON,
			    SDLK_d, KMOD_SHIFT, obj_op, "%p, %i", objs_tl,
			    OBJEDIT_MOVE_DOWN);
			
			menu_separator(mi);
			
			menu_action(mi, _("Reinitialize"), OBJREINIT_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_REINIT);
			menu_action(mi, _("Destroy"), TRASH_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_DESTROY);
		}
	}

#ifdef NETWORK
	ntab = notebook_add_tab(nb, _("Repository"), BOX_VERT);
	{
		struct tlist *tl;
		struct button *btn;
		struct AGMenuItem *pop;

		tl = tlist_new(ntab, TLIST_MULTI|TLIST_TREE);
		tlist_set_compare_fn(tl, tlist_compare_strings);
		pop = tlist_set_popup(tl, "object");
		{
			menu_action(pop, _("Update from repository"),
			    OBJLOAD_ICON, update_from_repo, "%p", tl);
		}

		btn = button_new(ntab, _("Refresh listing"));
		WIDGET(btn)->flags |= WIDGET_WFILL;
		event_new(btn, "button-pushed", update_repo_listing, "%p", tl);

		if (rcs)
			event_post(NULL, btn, "button-pushed", NULL);
	}
#endif /* NETWORK */
	
	window_show(win);
	return (win);
}

void
objmgr_init(void)
{
	TAILQ_INIT(&dobjs);
	TAILQ_INIT(&gobjs);
}

static void
objmgr_quit(int argc, union evarg *argv)
{
	SDL_Event nev;

	objmgr_exiting = 0;
	nev.type = SDL_USEREVENT;
	SDL_PushEvent(&nev);
}

static void
objmgr_quit_cancel(int argc, union evarg *argv)
{
	struct window *win = argv[1].p;

	objmgr_exiting = 0;
	view_detach(win);
}

/*
 * Present the user with a dialog asking whether to save modifications to
 * an object and optionally exit afterwards.
 */
void
objmgr_quit_dlg(void *obj)
{
	struct window *win;
	struct box *bo;
	struct button *b;

	if ((win = window_new(WINDOW_MODAL|WINDOW_NO_TITLEBAR|WINDOW_NO_RESIZE,
	    "objmgr-changed-dlg")) == NULL) {
		return;
	}
	window_set_caption(win, _("Exit application?"));
	window_set_position(win, WINDOW_CENTER, 0);
	window_set_spacing(win, 8);

	label_new(win, LABEL_STATIC,
	    _("Some objects have been modified. Exit application?"));

	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|VBOX_WFILL);
	box_set_spacing(bo, 0);
	box_set_padding(bo, 0);
	{
		b = button_new(bo, _("Quit"));
		event_new(b, "button-pushed", objmgr_quit, NULL);

		b = button_new(bo, _("Cancel"));
		event_new(b, "button-pushed", objmgr_quit_cancel, "%p", win);
		widget_focus(b);
	}
	window_show(win);
}

void
objmgr_destroy(void)
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

/*
 * Close and reopen all edition windows associated with an object, if there
 * are any. This is called automatically from object_load() for objects with
 * the flag OBJECT_REOPEN_ONLOAD.
 */
void
objmgr_reopen(struct object *obj)
{
	struct objent *oent;

	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj == obj) {
			dprintf("reopening %s\n", obj->name);

			window_hide(oent->win);
			event_post(NULL, oent->obj, "edit-close", NULL);
			view_detach(oent->win);

			event_post(NULL, oent->obj, "edit-open", NULL);
			oent->win = obj->ops->edit(obj);
			window_show(oent->win);
			event_new(oent->win, "window-close", close_object_dlg,
			    "%p", oent);
		}
	}
}

void
objmgr_close_data(void *p)
{
	struct object *obj = p;
	struct objent *oent;

	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj != obj) {
			continue;
		}
		event_post(NULL, oent->win, "window-close", NULL);
	}
}
