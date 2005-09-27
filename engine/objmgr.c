/*	$Csoft: objmgr.c,v 1.48 2005/09/20 13:46:29 vedge Exp $	*/

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
	AG_Object *obj;
	AG_Window *win;
	TAILQ_ENTRY(objent) objs;
};
static TAILQ_HEAD(,objent) dobjs;
static TAILQ_HEAD(,objent) gobjs;
static int edit_on_create = 1;
static void *current_pobj = NULL;

#ifdef NETWORK
static AG_Timeout repo_timeout;
#endif

int agObjMgrExiting = 0;
#ifdef DEBUG
int agObjMgrHexDiff = 0;
#endif

static void
create_obj(int argc, union evarg *argv)
{
	char name[AG_OBJECT_NAME_MAX];
	AG_ObjectType *t = argv[1].p;
	AG_Textbox *name_tb = argv[2].p;
	AG_Tlist *objs_tl = argv[3].p;
	AG_Window *dlg_win = argv[4].p;
	AG_TlistItem *it;
	AG_Object *pobj;
	void *nobj;

	if ((it = AG_TlistSelectedItem(objs_tl)) != NULL) {
		pobj = it->p1;
	} else {
		pobj = agWorld;
	}
	AG_TextboxCopyString(name_tb, name, sizeof(name));
	AG_ViewDetach(dlg_win);

	if (name[0] == '\0') {
		u_int nameno = 0;
		AG_Object *ch;
		char tname[AG_OBJECT_TYPE_MAX], *s;
	
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
		AG_ObjectInit(nobj, t->type, name, NULL);
	}
	AG_ObjectAttach(pobj, nobj);
	AG_ObjectUnlinkDatafiles(nobj);

	AG_PostEvent(NULL, nobj, "edit-create", NULL);
	
	if (edit_on_create &&
	    t->ops->edit != NULL)
		AG_ObjMgrOpenData(nobj, 1);
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
	AG_Window *win = argv[0].p;
	struct objent *oent = argv[1].p;

	AG_ViewDetach(win);
	TAILQ_REMOVE(&gobjs, oent, objs);
	AG_ObjectDelDep(&agMapEditor.pseudo, oent->obj);
	Free(oent, M_MAPEDIT);
}

void
AG_ObjMgrOpenGeneric(AG_Object *ob)
{
	struct objent *oent;

	TAILQ_FOREACH(oent, &gobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		AG_WindowShow(oent->win);
		agView->focus_win = oent->win;
		return;
	}
	
	AG_ObjectAddDep(&agMapEditor.pseudo, ob);
	
	oent = Malloc(sizeof(struct objent), M_MAPEDIT);
	oent->obj = ob;
	oent->win = AG_ObjectEdit(ob);
	TAILQ_INSERT_HEAD(&gobjs, oent, objs);
	AG_WindowShow(oent->win);

	AG_SetEvent(oent->win, "window-close", close_obj_generic, "%p", oent);
}

static void
close_object(struct objent *oent, AG_Window *win, int save)
{
	AG_WindowHide(win);
	AG_PostEvent(NULL, oent->obj, "edit-close", NULL);
	AG_ViewDetach(win);
	TAILQ_REMOVE(&dobjs, oent, objs);

	if (!save) {
		agObjMgrExiting = 1;
	}
	AG_ObjectPageOut(oent->obj, AG_OBJECT_DATA);
	AG_ObjectPageOut(oent->obj, AG_OBJECT_GFX);

	agObjMgrExiting = 0;
	AG_ObjectDelDep(&agMapEditor.pseudo, oent->obj);
	Free(oent, M_MAPEDIT);
}

static void
close_object_cb(int argc, union evarg *argv)
{
	AG_Window *win = argv[1].p;
	struct objent *oent = argv[2].p;
	int save = argv[3].i;

	close_object(oent, win, save);
}

static void
close_object_dlg(int argc, union evarg *argv)
{
	AG_Window *win = argv[0].p;
	struct objent *oent = argv[1].p;

	if (AG_ObjectChanged(oent->obj)) {
		AG_Button *bOpts[3];
		AG_Window *wDlg;

		wDlg = AG_TextPromptOptions(bOpts, 3, _("Save changes to %s?"),
		    AGOBJECT(oent->obj)->name);
		AG_WindowAttach(win, wDlg);
		{
			AG_ButtonPrintf(bOpts[0], _("Save"));
			AG_SetEvent(bOpts[0], "button-pushed",
			    close_object_cb, "%p,%p,%i", win, oent, 1);
			AG_WidgetFocus(bOpts[0]);

			AG_ButtonPrintf(bOpts[1], _("Discard"));
			AG_SetEvent(bOpts[1], "button-pushed",
			    close_object_cb, "%p,%p,%i", win, oent, 0);

			AG_ButtonPrintf(bOpts[2], _("Cancel"));
			AG_SetEvent(bOpts[2], "button-pushed",
			    AGWINDETACH(wDlg));
		}
	} else {
		close_object(oent, win, 1);
	}
}

void
AG_ObjMgrOpenData(void *p, int new)
{
	AG_Object *ob = p;
	struct objent *oent;
	AG_Window *win;

	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		AG_WindowShow(oent->win);
		agView->focus_win = oent->win;
		return;
	}
	
	if (ob->ops->edit == NULL)
		return;

	if ((ob->flags & AG_OBJECT_NON_PERSISTENT) == 0) {
		if (ob->data_used == 0 &&
		   (AG_ObjectLoad(ob) == -1 || AG_ObjectLoadData(ob) == -1)) {
			if (new) {
				dprintf("%s: new object\n", ob->name);
				ob->flags |= AG_OBJECT_DATA_RESIDENT;

				if (AG_ObjectSave(ob) == -1) {
					AG_TextMsg(AG_MSG_ERROR, "%s: %s",
					    ob->name, AG_GetError());
					return;
				}
			} else {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
				return;
			}
		}
		if (++ob->data_used > AG_OBJECT_DEP_MAX)
			ob->data_used = AG_OBJECT_DEP_MAX;
	}
	if (AG_ObjectPageIn(ob, AG_OBJECT_GFX) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s (gfx): %s", ob->name, AG_GetError());
		goto fail_data;
	}
	AG_ObjectAddDep(&agMapEditor.pseudo, ob);

	if ((win = ob->ops->edit(ob)) == NULL) {
		goto fail_gfx;
	}
	AG_PostEvent(NULL, ob, "edit-open", NULL);
	
	oent = Malloc(sizeof(struct objent), M_MAPEDIT);
	oent->obj = ob;
	oent->win = win;
	TAILQ_INSERT_HEAD(&dobjs, oent, objs);
	AG_SetEvent(win, "window-close", close_object_dlg, "%p", oent);
	AG_WindowShow(win);
	return;
fail_gfx:
	AG_ObjectDelDep(&agMapEditor.pseudo, ob);
	AG_ObjectPageOut(ob, AG_OBJECT_GFX);
fail_data:
	AG_ObjectPageOut(ob, AG_OBJECT_DATA);
	return;
}

static void
export_object(int argc, union evarg *argv)
{
	char save_path[MAXPATHLEN];
	AG_Object *ob = argv[1].p;
	AG_Window *win = argv[2].p;
	char *path = argv[3].s;
	char *pfx_save = ob->save_pfx;
	int paged_in = 0;

	if ((ob->flags & AG_OBJECT_DATA_RESIDENT) == 0) {
		if (AG_ObjectLoadData(ob) == -1) {
			/* XXX hack */
			ob->flags |= AG_OBJECT_DATA_RESIDENT;
		}
		paged_in = 1;
	}

	AG_StringCopy(agConfig, "save-path", save_path, sizeof(save_path));
	AG_SetString(agConfig, "save-path", "%s", path);
	ob->save_pfx = NULL;

	if (AG_ObjectSaveAll(ob) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name, AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Object `%s' was exported successfully."), ob->name);
	}

	AG_SetString(agConfig, "save-path", "%s", save_path);
	ob->save_pfx = pfx_save;
	AG_ViewDetach(win);
	
	if (paged_in)
		AG_ObjectFreeData(ob);
}

void
AG_ObjMgrSaveTo(void *p)
{
	AG_Object *ob = p;
	char path[FILENAME_MAX];
	AG_Window *win;
	AG_FileDlg *fdg;

	strlcpy(path, ob->name, sizeof(path));
	strlcat(path, ".", sizeof(path));
	strlcat(path, ob->type, sizeof(path));

	win = AG_WindowNew(0, NULL);
	AG_WindowSetCaption(win, _("Save %s to..."), ob->name);
	fdg = AG_FileDlgNew(win, 0, AG_String(agConfig, "save-path"),
	    path);
	AG_SetEvent(fdg, "file-validated", export_object, "%p,%p", ob, win);
	AG_SetEvent(fdg, "file-cancelled", AGWINDETACH(win));

	AG_WindowShow(win);
}

static void
obj_op(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;
	AG_TlistItem *it;
	int op = argv[2].i;

	TAILQ_FOREACH(it, &tl->items, items) {
		AG_Object *ob = it->p1;

		if (!it->selected)
			continue;

		switch (op) {
		case OBJEDIT_EDIT_DATA:
			if (ob->ops->edit != NULL) {
				AG_ObjMgrOpenData(ob, 0);
			} else {
				AG_TextTmsg(AG_MSG_ERROR, 750,
				    _("Object `%s' has no edit operation."),
				    ob->name);
			}
			break;
		case OBJEDIT_EDIT_GENERIC:
			AG_ObjMgrOpenGeneric(ob);
			break;
		case OBJEDIT_LOAD:
			if (AG_ObjectLoad(ob) == -1) {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
			}
			break;
		case OBJEDIT_SAVE:
			if (AG_ObjectSave(ob) == -1) {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
			} else {
				AG_TextTmsg(AG_MSG_INFO, 1000,
				    _("Object `%s' was saved successfully."),
				    ob->name);
			}
			break;
		case OBJEDIT_SAVE_ALL:
			if (AG_ObjectSaveAll(ob) == -1) {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
			} else {
				AG_TextTmsg(AG_MSG_INFO, 1000,
				    _("Object `%s' was saved successfully."),
				    ob->name);
			}
			break;
		case OBJEDIT_EXPORT:
			if (ob->flags & AG_OBJECT_NON_PERSISTENT) {
				AG_SetError(
				    _("The `%s' object is non-persistent."),
				    ob->name);
			} else {
				AG_ObjMgrSaveTo(ob);
			}
			break;
		case OBJEDIT_DUP:
			{
				AG_Object *dob;

				if (ob == agWorld ||
				    ob->flags & AG_OBJECT_NON_PERSISTENT) {
					AG_TextMsg(AG_MSG_ERROR,
					    _("%s: cannot duplicate."),
					    ob->name);
					break;
				}
				if ((dob = AG_ObjectDuplicate(ob)) == NULL) {
					AG_TextMsg(AG_MSG_ERROR, "%s: %s",
					    ob->name, AG_GetError());
				}
			}
			break;
		case OBJEDIT_MOVE_UP:
			AG_ObjectMoveUp(ob);
			break;
		case OBJEDIT_MOVE_DOWN:
			AG_ObjectMoveDown(ob);
			break;
		case OBJEDIT_REINIT:
			if (it->p1 == agWorld) {
				continue;
			}
			if (ob->ops->reinit != NULL) {
				ob->ops->reinit(ob);
			}
			break;
		case OBJEDIT_DESTROY:
			if (it->p1 == agWorld) {
				continue;
			}
			if (AG_ObjectInUse(ob)) {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
				continue;
			}
			if (ob->flags & AG_OBJECT_INDESTRUCTIBLE) {
				AG_TextMsg(AG_MSG_ERROR,
				    _("The `%s' object is indestructible."),
				    ob->name);
				continue;
			}
			AG_ObjectDetach(ob);
			AG_ObjectUnlinkDatafiles(ob);
			AG_ObjectDestroy(ob);
			if ((ob->flags & AG_OBJECT_STATIC) == 0) {
				Free(ob, M_OBJECT);
			}
			break;
#ifdef NETWORK
		case OBJEDIT_RCS_IMPORT:
			if (AG_ObjectSave(ob) == -1 ||
			    AG_RcsImport(ob) == -1) {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
			}
			break;
		case OBJEDIT_RCS_IMPORT_ALL:
			AG_RcsImportAll(ob);
			break;
		case OBJEDIT_RCS_COMMIT:
			if (AG_ObjectSave(ob) == -1 ||
			    AG_RcsCommit(ob) == -1) {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
			}
			break;
		case OBJEDIT_RCS_COMMIT_ALL:
			AG_RcsCommitAll(ob);
			break;
		case OBJEDIT_RCS_UPDATE:
			if (AG_ObjectSave(ob) == -1) {
				AG_TextMsg(AG_MSG_ERROR,
				    _("Save failed: %s: %s"), ob->name,
				    AG_GetError());
				break;
			}
			if (AG_RcsUpdate(ob) == 0) {
				if (AG_ObjectLoad(ob) == -1) {
					AG_TextMsg(AG_MSG_ERROR, "%s: %s",
					    ob->name, AG_GetError());
				}
			} else {
				AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
			}
			break;
		case OBJEDIT_RCS_UPDATE_ALL:
			AG_RcsUpdateAll(ob);
			break;
#endif /* NETWORK */
		}
	}
}

static void
generic_save(int argc, union evarg *argv)
{
	AG_Object *ob = argv[1].p;

	if (AG_ObjectSave(ob) == -1) {
		AG_TextMsg(AG_MSG_ERROR, _("Save failed: %s: %s"), ob->name,
		    AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Object `%s' saved successfully."), ob->name);
	}
}

static void
generic_save_to(int argc, union evarg *argv)
{
	AG_ObjMgrSaveTo(argv[1].p);
}

void
AG_ObjMgrGenericMenu(void *menup, void *obj)
{
	AG_MenuItem *pitem = menup;

	AG_MenuAction(pitem, _("Save"), OBJSAVE_ICON,
	    generic_save, "%p", obj);
	AG_MenuAction(pitem, _("Save to..."), OBJSAVE_ICON,
	    generic_save_to, "%p", obj);
}

/* Display the object tree. */
static AG_TlistItem *
find_objs(AG_Tlist *tl, AG_Object *pob, int depth)
{
	char label[AG_TLIST_LABEL_MAX];
	AG_Object *cob;
	AG_TlistItem *it;
	SDL_Surface *icon;

	strlcpy(label, pob->name, sizeof(label));
	if (pob->flags & AG_OBJECT_DATA_RESIDENT) {
		strlcat(label, _(" (resident)"), sizeof(label));
	}
	it = AG_TlistAddPtr(tl, AG_ObjectIcon(pob), label, pob);
	it->depth = depth;
	it->class = "object";

	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
		if (AG_ObjectRoot(pob) == pob)
			it->flags |= AG_TLIST_VISIBLE_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		TAILQ_FOREACH(cob, &pob->children, cobjs)
			find_objs(tl, cob, depth+1);
	}
	return (it);
}

/* Update the object tree display. */
static void
poll_objs(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_Object *pob = argv[1].p;
	AG_Object *dob = argv[2].p;
	AG_TlistItem *it;

	AG_LockLinkage();
	AG_TlistClear(tl);
	find_objs(tl, pob, 0);
	AG_TlistRestore(tl);
	AG_UnlockLinkage();

	if (AG_TlistSelectedItem(tl) == NULL) {
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->p1 == dob)
				break;
		}
		if (it != NULL)
			AG_TlistSelect(tl, it);
	}
}

static void
load_object(int argc, union evarg *argv)
{
	AG_Object *o = argv[1].p;

	if (AG_ObjectLoad(o) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", AGOBJECT(o)->name,
		    AG_GetError());
	}
}

static void
save_object(int argc, union evarg *argv)
{
	AG_Object *ob = argv[1].p;

	if (AG_ObjectSave(ob) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name, AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Object `%s' saved successfully."), ob->name);
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
show_config_win(int argc, union evarg *argv)
{
	if (!agConfig->window->visible) {
		AG_WindowShow(agConfig->window);
	} else {
		AG_WindowFocus(agConfig->window);
	}
}

static void
create_obj_dlg(int argc, union evarg *argv)
{
	AG_Window *win;
	AG_ObjectType *t = argv[1].p;
	AG_Window *pwin = argv[2].p;
	AG_Tlist *pobj_tl;
	AG_Box *bo;
	AG_Textbox *tb;
	AG_Checkbox *cb;

	win = AG_WindowNew(AG_WINDOW_NO_CLOSE|AG_WINDOW_NO_MINIMIZE, NULL);
	AG_WindowSetCaption(win, _("New %s object"), t->type);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL);
	{
		AG_LabelNew(bo, AG_LABEL_STATIC, _("Type: %s"), t->type);
		tb = AG_TextboxNew(bo, _("Name: "));
		AG_WidgetFocus(tb);
	}

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL|AG_BOX_HFILL);
	AG_BoxSetPadding(bo, 0);
	AG_BoxSetSpacing(bo, 0);
	{
		AG_LabelNew(bo, AG_LABEL_STATIC, _("Parent object:"));

		pobj_tl = AG_TlistNew(bo, AG_TLIST_POLL|AG_TLIST_TREE);
		AG_TlistPrescale(pobj_tl, "XXXXXXXXXXXXXXXXXXX", 5);
		AG_WidgetBind(pobj_tl, "selected", AG_WIDGET_POINTER,
		    &current_pobj);
		AG_SetEvent(pobj_tl, "tlist-poll", poll_objs, "%p,%p", agWorld,
		    current_pobj);
	}

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL);
	{
		cb = AG_CheckboxNew(win, _("Edit now"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT, &edit_on_create);
	}

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_WFILL);
	{
		AG_Button *btn;
	
		btn = AG_ButtonNew(bo, _("OK"));
		AG_SetEvent(tb, "textbox-return", create_obj, "%p,%p,%p,%p", t,
		    tb, pobj_tl, win);
		AG_SetEvent(btn, "button-pushed", create_obj, "%p,%p,%p,%p", t,
		    tb, pobj_tl, win);
		
		btn = AG_ButtonNew(bo, _("Cancel"));
		AG_SetEvent(btn, "button-pushed", AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

#ifdef NETWORK

static void
update_repo_listing(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;

	if (!agRcsMode) {
		AG_TextMsg(AG_MSG_ERROR, _("RCS is currently disabled."));
		return;
	}
	if (AG_RcsConnect() == -1 ||
	    AG_RcsList(tl) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
	}
	AG_RcsDisconnect();
}

static void
update_from_repo(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected)
			continue;

		if (AG_RcsCheckout(it->text) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "%s: %s", it->text,
			    AG_GetError());
		}
	}
}

static void
delete_from_repo(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected)
			continue;

		if (AG_RcsDelete(it->text) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "%s: %s", it->text,
			    AG_GetError());
		} else {
			AG_TextTmsg(AG_MSG_INFO, 500,
			    _("Object %s removed from repository."), it->text);
		}
	}
	if (AG_RcsConnect() == 0) {
		AG_RcsList(tl);
		AG_RcsDisconnect();
	}
}

static void
rename_repo(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;
	char *from = argv[2].s;
	char *to = argv[3].s;

	if (AG_RcsRename(from, to) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", from, AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000, _("Object %s renamed to %s."),
		    from, to);
	}
	if (AG_RcsConnect() == 0) {
		AG_RcsList(tl);
		AG_RcsDisconnect();
	}
}

static void
rename_repo_dlg(int argc, union evarg *argv)
{
	char prompt[AG_LABEL_MAX];
	AG_Tlist *tl = argv[1].p;
	AG_TlistItem *it;
	
	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;
	
	snprintf(prompt, sizeof(prompt), _("Rename %s to:"), it->text);
	AG_TextPromptString(prompt, rename_repo, "%p,%s", tl, it->text);
}

#endif /* NETWORK */

/* Create the object editor window. */
AG_Window *
AG_ObjMgrWindow(void)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_Textbox *name_tb;
	AG_Tlist *objs_tl;
	AG_Menu *me;
	AG_MenuItem *mi, *mi_objs;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;

	win = AG_WindowNew(0, "objmgr");
	AG_WindowSetCaption(win, _("Object manager"));
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_LEFT, 0);
	
	objs_tl = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(objs_tl, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_TREE);
	AG_TlistPrescale(objs_tl, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 10);
	AG_SetEvent(objs_tl, "tlist-poll", poll_objs, "%p,%p", agWorld, NULL);
	AG_SetEvent(objs_tl, "tlist-dblclick", obj_op, "%p, %i", objs_tl,
	    OBJEDIT_EDIT_DATA);

	me = AG_MenuNew(win);
	mi = AG_MenuAddItem(me, _("File"));
	{
		int i;

		mi_objs = AG_MenuAction(mi, _("New object"), OBJCREATE_ICON,
		    NULL, NULL);
		for (i = agnTypes-1; i >= 0; i--) {
			char label[32];
			AG_ObjectType *t = &agTypes[i];

			strlcpy(label, t->type, sizeof(label));
			label[0] = (char)toupper((int)label[0]);
			AG_MenuAction(mi_objs, label, t->icon,
			    create_obj_dlg, "%p,%p", t, win);
		}

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Load full state"), OBJLOAD_ICON,
		    load_object, "%p", agWorld);
		AG_MenuAction(mi, _("Save full state"), OBJSAVE_ICON,
		    save_object, "%p", agWorld);
		
		AG_MenuSeparator(mi);
		
		AG_MenuAction(mi, _("Exit"), -1, exit_program, NULL);
	}
	
	mi = AG_MenuAddItem(me, _("Edit"));
	{
		AG_MenuAction(mi, _("Edit object data..."), OBJEDIT_ICON,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_DATA);
		AG_MenuAction(mi, _("Edit generic information..."),
		    OBJGENEDIT_ICON,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_GENERIC);
		
		AG_MenuSeparator(mi);
			
		AG_MenuAction(mi, _("Duplicate"), OBJDUP_ICON,
		    obj_op, "%p, %i", objs_tl, OBJEDIT_DUP);
		AG_MenuActionKb(mi, _("Move up"), OBJMOVEUP_ICON,
		    SDLK_u, KMOD_SHIFT, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_MOVE_UP);
		AG_MenuActionKb(mi, _("Move down"), OBJMOVEDOWN_ICON,
		    SDLK_d, KMOD_SHIFT, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_MOVE_DOWN);

		if (agConfig->window != NULL) {
			AG_MenuSeparator(mi);
			AG_MenuAction(mi, _("Preferences..."), -1,
			    show_config_win, NULL);
		}
	}

#ifdef NETWORK
	mi = AG_MenuAddItem(me, _("Repository"));
	{
		AG_MenuAction(mi, _("Commit"),
		    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_COMMIT);

		AG_MenuAction(mi, _("Update"),
		    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_UPDATE);

		AG_MenuAction(mi, _("Import"),
		    OBJSAVE_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_IMPORT);

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Commit all"),
		    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_COMMIT_ALL);

		AG_MenuAction(mi, _("Update all"),
		    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_UPDATE_ALL);
			
		
		AG_MenuAction(mi, _("Import all"),
		    OBJSAVE_ICON, obj_op, "%p, %i", objs_tl,
		    OBJEDIT_RCS_IMPORT_ALL);
	}
#endif /* NETWORK */

#ifdef DEBUG
	mi = AG_MenuAddItem(me, _("Debug"));
	AG_MonitorMenu(mi);
#endif /* DEBUG */

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	ntab = AG_NotebookAddTab(nb, _("Working copy"), AG_BOX_VERT);
	{
		AG_MenuItem *mi, *mi2;

		AG_ObjectAttach(ntab, objs_tl);

		mi = AG_TlistSetPopup(objs_tl, "object");
		{
			AG_MenuAction(mi, _("Edit object..."), OBJEDIT_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_DATA);
			AG_MenuAction(mi, _("Edit object information..."),
			    OBJGENEDIT_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_EDIT_GENERIC);

			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Load"), OBJLOAD_ICON, obj_op,
			    "%p, %i", objs_tl, OBJEDIT_LOAD);
			AG_MenuAction(mi, _("Save"), OBJSAVE_ICON, obj_op,
			    "%p, %i", objs_tl, OBJEDIT_SAVE);
			AG_MenuAction(mi, _("Save all"), OBJSAVE_ICON, obj_op,
			    "%p, %i", objs_tl, OBJEDIT_SAVE_ALL);
			AG_MenuAction(mi, _("Save to..."), OBJSAVE_ICON, obj_op,
			    "%p, %i", objs_tl, OBJEDIT_EXPORT);

#ifdef NETWORK
			if (agRcsMode) {
				AG_MenuSeparator(mi);
			
				mi2 = AG_MenuAction(mi, _("Repository"),
				    OBJLOAD_ICON, NULL, NULL);

				AG_MenuAction(mi2, _("Commit"),
				    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_COMMIT);
				
				AG_MenuAction(mi2, _("Update"),
				    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_UPDATE);
				
				AG_MenuAction(mi2, _("Import"),
				    OBJSAVE_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_IMPORT);
		
				AG_MenuSeparator(mi2);
				
				AG_MenuAction(mi2, _("Commit all"),
				    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_COMMIT_ALL);

				AG_MenuAction(mi2, _("Update all"),
				    OBJLOAD_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_UPDATE_ALL);
			
				AG_MenuAction(mi2, _("Import all"),
				    OBJSAVE_ICON, obj_op, "%p, %i", objs_tl,
				    OBJEDIT_RCS_IMPORT_ALL);
			}
#endif /* NETWORK */
			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Duplicate"), OBJDUP_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_DUP);
			AG_MenuActionKb(mi, _("Move up"), OBJMOVEUP_ICON,
			    SDLK_u, KMOD_SHIFT, obj_op, "%p, %i", objs_tl,
			    OBJEDIT_MOVE_UP);
			AG_MenuActionKb(mi, _("Move down"), OBJMOVEDOWN_ICON,
			    SDLK_d, KMOD_SHIFT, obj_op, "%p, %i", objs_tl,
			    OBJEDIT_MOVE_DOWN);
			
			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Reinitialize"), OBJREINIT_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_REINIT);
			AG_MenuAction(mi, _("Destroy"), TRASH_ICON,
			    obj_op, "%p, %i", objs_tl, OBJEDIT_DESTROY);
		}
	}

#ifdef NETWORK
	ntab = AG_NotebookAddTab(nb, _("Repository"), AG_BOX_VERT);
	{
		AG_Tlist *tl;
		AG_Button *btn;
		AG_MenuItem *pop;

		tl = AG_TlistNew(ntab, AG_TLIST_MULTI|AG_TLIST_TREE);
		AG_TlistSetCompareFn(tl, AG_TlistCompareStrings);
		pop = AG_TlistSetPopup(tl, "object");
		{
			AG_MenuAction(pop, _("Update from repository"),
			    OBJLOAD_ICON, update_from_repo, "%p", tl);
			
			AG_MenuAction(pop, _("Delete from repository"),
			    TRASH_ICON, delete_from_repo, "%p", tl);
			
			AG_MenuAction(pop, _("Rename"),
			    -1, rename_repo_dlg, "%p", tl);
		}

		btn = AG_ButtonNew(ntab, _("Refresh listing"));
		AGWIDGET(btn)->flags |= AG_WIDGET_WFILL;
		AG_SetEvent(btn, "button-pushed", update_repo_listing, "%p",
		    tl);

		if (agRcsMode)
			AG_PostEvent(NULL, btn, "button-pushed", NULL);
	}
#endif /* NETWORK */
	
	AG_WindowShow(win);
	return (win);
}

void
AG_ObjMgrInit(void)
{
	TAILQ_INIT(&dobjs);
	TAILQ_INIT(&gobjs);
}

static void
objmgr_quit(int argc, union evarg *argv)
{
	SDL_Event nev;

	agObjMgrExiting = 0;
	nev.type = SDL_USEREVENT;
	SDL_PushEvent(&nev);
}

static void
objmgr_quit_cancel(int argc, union evarg *argv)
{
	AG_Window *win = argv[1].p;

	agObjMgrExiting = 0;
	AG_ViewDetach(win);
}

/*
 * Present the user with a dialog asking whether to save modifications to
 * an object and optionally exit afterwards.
 */
void
AG_ObjMgrQuitDlg(void *obj)
{
	AG_Window *win;
	AG_Box *bo;
	AG_Button *b;

	if ((win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NO_TITLEBAR|AG_WINDOW_NO_RESIZE,
	    "objmgr-changed-dlg")) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Exit application?"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	AG_LabelNew(win, AG_LABEL_STATIC,
	    _("Some objects have been modified. Exit application?"));

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_VBOX_WFILL);
	AG_BoxSetSpacing(bo, 0);
	AG_BoxSetPadding(bo, 0);
	{
		b = AG_ButtonNew(bo, _("Quit"));
		AG_SetEvent(b, "button-pushed", objmgr_quit, NULL);

		b = AG_ButtonNew(bo, _("Cancel"));
		AG_SetEvent(b, "button-pushed", objmgr_quit_cancel, "%p", win);
		AG_WidgetFocus(b);
	}
	AG_WindowShow(win);
}

void
AG_ObjMgrDestroy(void)
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
 * are any. This is called automatically from AG_ObjectLoad() for objects with
 * the flag AG_OBJECT_REOPEN_ONLOAD.
 */
void
AG_ObjMgrReopen(AG_Object *obj)
{
	struct objent *oent;

	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj == obj) {
			dprintf("reopening %s\n", obj->name);

			AG_WindowHide(oent->win);
			AG_PostEvent(NULL, oent->obj, "edit-close", NULL);
			AG_ViewDetach(oent->win);

			AG_PostEvent(NULL, oent->obj, "edit-open", NULL);
			oent->win = obj->ops->edit(obj);
			AG_WindowShow(oent->win);
			AG_SetEvent(oent->win, "window-close", close_object_dlg,
			    "%p", oent);
		}
	}
}

void
AG_ObjMgrCloseData(void *p)
{
	AG_Object *obj = p;
	struct objent *oent;

	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj != obj) {
			continue;
		}
		AG_PostEvent(NULL, oent->win, "window-close", NULL);
	}
}
