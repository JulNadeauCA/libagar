/*	$Csoft: objmgr.c,v 1.51 2005/10/04 17:34:50 vedge Exp $	*/

/*
 * Copyright (c) 2003-2006 CubeSoft Communications, Inc.
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

#include <core/core.h>
#include <core/view.h>
#include <core/config.h>
#include <core/timeout.h>
#include <core/typesw.h>

#include <gui/window.h>
#include <gui/box.h>
#include <gui/vbox.h>
#include <gui/button.h>
#include <gui/textbox.h>
#include <gui/checkbox.h>
#include <gui/tlist.h>
#include <gui/menu.h>
#include <gui/label.h>
#include <gui/separator.h>
#include <gui/file_dlg.h>
#include <gui/notebook.h>

#include <core/monitor/monitor.h>
#ifdef NETWORK
#include <core/rcs.h>
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
create_obj(AG_Event *event)
{
	char name[AG_OBJECT_NAME_MAX];
	AG_ObjectType *t = AG_PTR(1);
	AG_Textbox *name_tb = AG_PTR(2);
	AG_Tlist *objs_tl = AG_PTR(3);
	AG_Window *dlg_win = AG_PTR(4);
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
		Uint nameno = 0;
		AG_Object *ch;
		char tname[AG_OBJECT_TYPE_MAX], *s;
	
		if ((s = strrchr(t->ops->type, '.')) != NULL && s[1] != '\0') {
			strlcpy(tname, &s[1], sizeof(tname));
		} else {
			strlcpy(tname, t->ops->type, sizeof(tname));
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

	nobj = Malloc(t->ops->size, M_OBJECT);
	if (t->ops->init != NULL) {
		t->ops->init(nobj, name);
	} else {
		AG_ObjectInit(nobj, name, t->ops);
	}
	AG_ObjectAttach(pobj, nobj);
	AG_ObjectUnlinkDatafiles(nobj);

	AG_PostEvent(NULL, nobj, "edit-create", NULL);
	
	if (edit_on_create &&
	    t->ops->edit != NULL)
		AG_ObjMgrOpenData(nobj);
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
close_obj_generic(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	struct objent *oent = AG_PTR(1);

	AG_ViewDetach(win);
	TAILQ_REMOVE(&gobjs, oent, objs);
	Free(oent, M_OBJECT);
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
	
	oent = Malloc(sizeof(struct objent), M_OBJECT);
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
	AG_ObjectPageOut(oent->obj);

	agObjMgrExiting = 0;
	Free(oent, M_OBJECT);
}

static void
close_object_cb(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);
	struct objent *oent = AG_PTR(2);
	int save = AG_INT(3);

	close_object(oent, win, save);
}

static void
close_object_dlg(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	struct objent *oent = AG_PTR(1);

	if (AG_ObjectChanged(oent->obj)) {
		AG_Button *bOpts[3];
		AG_Window *wDlg;

		wDlg = AG_TextPromptOptions(bOpts, 3, _("Save changes to %s?"),
		    AGOBJECT(oent->obj)->name);
		AG_WindowAttach(win, wDlg);
		{
			AG_ButtonText(bOpts[0], _("Save"));
			AG_SetEvent(bOpts[0], "button-pushed",
			    close_object_cb, "%p,%p,%i", win, oent, 1);
			AG_WidgetFocus(bOpts[0]);

			AG_ButtonText(bOpts[1], _("Discard"));
			AG_SetEvent(bOpts[1], "button-pushed",
			    close_object_cb, "%p,%p,%i", win, oent, 0);

			AG_ButtonText(bOpts[2], _("Cancel"));
			AG_SetEvent(bOpts[2], "button-pushed",
			    AGWINDETACH(wDlg));
		}
	} else {
		close_object(oent, win, 1);
	}
}

void
AG_ObjMgrOpenData(void *p)
{
	AG_Object *ob = p;
	struct objent *oent;
	AG_Window *win;
	int dataFound;

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

	if (AGOBJECT_PERSISTENT(ob)) {
		if (!AGOBJECT_RESIDENT(ob) &&
		   (AG_ObjectLoad(ob) == -1 ||
		    AG_ObjectLoadData(ob, &dataFound) == -1)) {
			if (!dataFound) {
				/*
				 * Data not found in storage, so assume
				 * object is new. Mark it resident and save it.
				 */
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
	}
	if ((win = ob->ops->edit(ob)) == NULL) {
		goto fail;
	}
	AG_PostEvent(NULL, ob, "edit-open", NULL);
	
	oent = Malloc(sizeof(struct objent), M_OBJECT);
	oent->obj = ob;
	oent->win = win;
	TAILQ_INSERT_HEAD(&dobjs, oent, objs);
	AG_SetEvent(win, "window-close", close_object_dlg, "%p", oent);
	AG_WindowShow(win);
	return;
fail:
	AG_ObjectPageOut(ob);
	return;
}

static void
ExportObject(AG_Event *event)
{
	char save_path[MAXPATHLEN];
	AG_Object *ob = AG_PTR(1);
	AG_Window *win = AG_PTR(2);
	char *path = AG_STRING(3);
	char *pfx_save = ob->save_pfx;
	int loadedTmp = 0;
	int dataFound;

	/* Load the object temporarily if it is non-resident. */
	if (!AGOBJECT_RESIDENT(ob)) {
		if (AG_ObjectLoadData(ob, &dataFound) == -1) {
			if (!dataFound) {
				/* Object was never saved before. */
				ob->flags |= AG_OBJECT_DATA_RESIDENT;
			} else {
				AG_TextMsg(AG_MSG_ERROR,
				    _("%s: Loading failed (non-resident): %s"),
				    ob->name, AG_GetError());
				return;
			}
		}
		loadedTmp = 1;
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
	
	if (loadedTmp)
		AG_ObjectFreeData(ob);
}

static void
ImportObject(AG_Event *event)
{
	char load_path[MAXPATHLEN];
	AG_Object *ob = AG_PTR(1);
	AG_Window *win = AG_PTR(2);
	char *path = AG_STRING(3);
	char *pfx_save = ob->save_pfx;
	int loadedTmp = 0;
	int dataFound;

	/* Load the object temporarily if it is non-resident. */
	if (!AGOBJECT_RESIDENT(ob)) {
		if (AG_ObjectLoadData(ob, &dataFound) == -1) {
			if (!dataFound) {
				/* Object was never saved before. */
				ob->flags |= AG_OBJECT_DATA_RESIDENT;
			} else {
				AG_TextMsg(AG_MSG_ERROR,
				    _("%s: Loading failed (non-resident): %s"),
				    ob->name, AG_GetError());
				return;
			}
		}
		loadedTmp = 1;
	}

	AG_StringCopy(agConfig, "load-path", load_path, sizeof(load_path));
	AG_SetString(agConfig, "load-path", "%s", path);
	ob->save_pfx = NULL;

	if (AG_ObjectLoad(ob) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name, AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Object `%s' was imported successfully."), ob->name);
	}

	AG_SetString(agConfig, "load-path", "%s", load_path);
	ob->save_pfx = pfx_save;
	
	if (loadedTmp)
		AG_ObjectFreeData(ob);
}

void
AG_ObjMgrSaveTo(void *p, const char *name)
{
	char ext[AG_OBJECT_TYPE_MAX+3];
	AG_Object *ob = p;
	AG_Window *win;
	AG_FileDlg *fd;

	ext[0] = '*';
	ext[1] = '.';
	strlcpy(&ext[2], ob->ops->type, sizeof(ext)-2);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Save %s to..."), ob->name);
	fd = AG_FileDlgNew(win, AG_FILEDLG_CLOSEWIN|AG_FILEDLG_SAVE|
	                        AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, name, ext, NULL, NULL);
	AG_FileDlgSetDirectory(fd, AG_String(agConfig, "save-path"));
	AG_FileDlgSetFilename(fd, "%s.%s", ob->name, ob->ops->type);
	AG_SetEvent(fd, "file-chosen", ExportObject, "%p,%p", ob, win);
	AG_SetEvent(fd, "file-cancelled", AGWINDETACH(win));

	AG_WindowShow(win);
}

void
AG_ObjMgrLoadFrom(void *p, const char *name)
{
	char ext[AG_OBJECT_TYPE_MAX+3];
	AG_Object *ob = p;
	AG_Window *win;
	AG_FileDlg *fd;

	ext[0] = '*';
	ext[1] = '.';
	strlcpy(&ext[2], ob->ops->type, sizeof(ext)-2);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Load %s from..."), ob->name);
	fd = AG_FileDlgNew(win, AG_FILEDLG_CLOSEWIN|AG_FILEDLG_LOAD|
	                        AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, name, ext, NULL, NULL);
	AG_FileDlgSetDirectory(fd, AG_String(agConfig, "save-path"));
	AG_FileDlgSetFilename(fd, "%s.%s", ob->name, ob->ops->type);
	AG_SetEvent(fd, "file-chosen", ImportObject, "%p,%p", ob, win);
	AG_SetEvent(fd, "file-cancelled", AGWINDETACH(win));

	AG_WindowShow(win);
}

static void
obj_op(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	int op = AG_INT(2);

	TAILQ_FOREACH(it, &tl->items, items) {
		AG_Object *ob = it->p1;

		if (!it->selected)
			continue;

		switch (op) {
		case OBJEDIT_EDIT_DATA:
			if (ob->ops->edit != NULL) {
				AG_ObjMgrOpenData(ob);
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
			if (!AGOBJECT_PERSISTENT(ob)) {
				AG_SetError(
				    _("The `%s' object is non-persistent."),
				    ob->name);
			} else {
				AG_ObjMgrSaveTo(ob, _("Agar object file"));
			}
			break;
		case OBJEDIT_DUP:
			{
				AG_Object *dob;

				if (ob == agWorld || !AGOBJECT_PERSISTENT(ob)) {
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
AG_ObjMgrGenericSave(AG_Event *event)
{
	AG_Object *ob = AG_PTR(1);

	if (AG_ObjectSave(ob) == -1) {
		AG_TextMsg(AG_MSG_ERROR, _("Save failed: %s: %s"), ob->name,
		    AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000, _("`%s' saved successfully."),
		    ob->name);
	}
}

static void
AG_ObjMgrGenericLoad(AG_Event *event)
{
	AG_Object *ob = AG_PTR(1);

	if (AG_ObjectLoad(ob) == -1) {
		AG_TextMsg(AG_MSG_ERROR, _("Load failed: %s: %s"), ob->name,
		    AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000, _("`%s' loaded successfully."),
		    ob->name);
	}
}

static void
AG_ObjMgrGenericSaveTo(AG_Event *event)
{
	AG_ObjMgrSaveTo(AG_PTR(1), _("Agar object file"));
}

static void
AG_ObjMgrGenericLoadFrom(AG_Event *event)
{
	AG_ObjMgrLoadFrom(AG_PTR(1), _("Agar object file"));
}

void
AG_ObjMgrGenericMenu(void *menup, void *obj)
{
	AG_MenuItem *pitem = menup;

	AG_MenuAction(pitem, _("Save"), OBJSAVE_ICON,
	    AG_ObjMgrGenericSave, "%p", obj);
	AG_MenuAction(pitem, _("Load"), OBJLOAD_ICON,
	    AG_ObjMgrGenericLoad, "%p", obj);
	AG_MenuAction(pitem, _("Export to..."), OBJSAVE_ICON,
	    AG_ObjMgrGenericSaveTo, "%p", obj);
	AG_MenuAction(pitem, _("Import from..."), OBJLOAD_ICON,
	    AG_ObjMgrGenericLoadFrom, "%p", obj);

	AG_MenuSeparator(pitem);

	AG_MenuIntFlags(pitem, _("Persistence"), OBJLOAD_ICON,
	    &AGOBJECT(obj)->flags, AG_OBJECT_NON_PERSISTENT, 1);
	AG_MenuIntFlags(pitem, _("Destructible"), TRASH_ICON,
	    &AGOBJECT(obj)->flags, AG_OBJECT_INDESTRUCTIBLE, 1);
	AG_MenuIntFlags(pitem, _("Editable"), OBJ_ICON,
	    &AGOBJECT(obj)->flags, AG_OBJECT_READONLY, 1);
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
	if (AGOBJECT_RESIDENT(pob)) {
		strlcat(label, _(" (resident)"), sizeof(label));
	}
	it = AG_TlistAddPtr(tl, AG_ObjectIcon(pob), label, pob);
	it->depth = depth;
	it->cat = "object";

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
poll_objs(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Object *pob = AG_PTR(1);
	AG_Object *dob = AG_PTR(2);
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
load_object(AG_Event *event)
{
	AG_Object *o = AG_PTR(1);

	if (AG_ObjectLoad(o) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", AGOBJECT(o)->name,
		    AG_GetError());
	}
}

static void
save_object(AG_Event *event)
{
	AG_Object *ob = AG_PTR(1);

	if (AG_ObjectSave(ob) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name, AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Object `%s' saved successfully."), ob->name);
	}
}

static void
exit_program(AG_Event *event)
{
	SDL_Event ev;

	ev.type = SDL_QUIT;
	SDL_PushEvent(&ev);
}

static void
show_config_win(AG_Event *event)
{
	if (!agConfig->window->visible) {
		AG_WindowShow(agConfig->window);
	} else {
		AG_WindowFocus(agConfig->window);
	}
}

static void
create_obj_dlg(AG_Event *event)
{
	AG_Window *win;
	AG_ObjectType *t = AG_PTR(1);
	AG_Window *pwin = AG_PTR(2);
	AG_Tlist *pobj_tl;
	AG_Box *bo;
	AG_Textbox *tb;
	AG_Checkbox *cb;

	win = AG_WindowNew(AG_WINDOW_NOCLOSE|AG_WINDOW_NOMINIMIZE);
	AG_WindowSetCaption(win, _("New %s object"), t->ops->type);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		AG_LabelNewStatic(bo, 0, _("Type: %s"), t->ops->type);
		tb = AG_TextboxNew(bo, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS,
		    _("Name: "));
	}

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL|AG_BOX_VFILL);
	AG_BoxSetPadding(bo, 0);
	AG_BoxSetSpacing(bo, 0);
	{
		AG_LabelNewStaticString(bo, 0, _("Parent object:"));

		pobj_tl = AG_TlistNew(bo, AG_TLIST_POLL|AG_TLIST_TREE|
		                          AG_TLIST_EXPAND);
		AG_TlistPrescale(pobj_tl, "XXXXXXXXXXXXXXXXXXX", 5);
		AG_WidgetBind(pobj_tl, "selected", AG_WIDGET_POINTER,
		    &current_pobj);
		AG_SetEvent(pobj_tl, "tlist-poll", poll_objs, "%p,%p", agWorld,
		    current_pobj);
	}

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		cb = AG_CheckboxNew(win, 0, _("Edit now"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT, &edit_on_create);
	}

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		AG_ButtonAct(bo, 0, _("OK"), create_obj, "%p,%p,%p,%p",
		    t, tb, pobj_tl, win);
		AG_SetEvent(tb, "textbox-return", create_obj, "%p,%p,%p,%p",
		    t, tb, pobj_tl, win);
		
		AG_ButtonAct(bo, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

#ifdef NETWORK

static void
update_repo_listing(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);

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
update_from_repo(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
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
delete_from_repo(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
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
rename_repo(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	char *from = AG_STRING(2);
	char *to = AG_STRING(3);

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
rename_repo_dlg(AG_Event *event)
{
	char prompt[AG_LABEL_MAX];
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	
	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;
	
	snprintf(prompt, sizeof(prompt), _("Rename %s to:"), it->text);
	AG_TextPromptString(prompt, rename_repo, "%p,%s", tl, it->text);
}

#endif /* NETWORK */

/* Create the object editor window. */
struct ag_window *
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

	win = AG_WindowNewNamed(0, "objmgr");
	AG_WindowSetCaption(win, _("Object manager"));
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_LEFT, 0);
	
	objs_tl = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(objs_tl, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_TREE|
	                      AG_TLIST_EXPAND);
	AG_TlistPrescale(objs_tl, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 10);
	AG_SetEvent(objs_tl, "tlist-poll", poll_objs, "%p,%p", agWorld, NULL);
	AG_SetEvent(objs_tl, "tlist-dblclick", obj_op, "%p, %i", objs_tl,
	    OBJEDIT_EDIT_DATA);

	me = AG_MenuNew(win, AG_MENU_HFILL);
	mi = AG_MenuAddItem(me, _("File"));
	{
		int i;

		mi_objs = AG_MenuAction(mi, _("New object"), OBJCREATE_ICON,
		    NULL, NULL);
		for (i = agnTypes-1; i >= 0; i--) {
			char label[32];
			AG_ObjectType *t = &agTypes[i];

			strlcpy(label, t->ops->type, sizeof(label));
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

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
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

		tl = AG_TlistNew(ntab, AG_TLIST_MULTI|AG_TLIST_TREE|
		                       AG_TLIST_EXPAND);
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

		btn = AG_ButtonAct(ntab, AG_BUTTON_HFILL, _("Refresh listing"),
		    update_repo_listing, "%p", tl);
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
objmgr_quit(AG_Event *event)
{
	SDL_Event nev;

	agObjMgrExiting = 0;
	nev.type = SDL_USEREVENT;
	SDL_PushEvent(&nev);
}

static void
quit_cancel(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

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

	if ((win = AG_WindowNewNamed(AG_WINDOW_MODAL|AG_WINDOW_NOTITLE|
	    AG_WINDOW_NORESIZE, "objmgr-changed-dlg")) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Exit application?"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	AG_LabelNewStaticString(win, 0,
	    _("Some objects have been modified. Exit application?"));

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_VBOX_HFILL);
	AG_BoxSetSpacing(bo, 0);
	AG_BoxSetPadding(bo, 0);
	{
		AG_ButtonAct(bo, 0, _("Quit"), objmgr_quit, NULL);
		AG_ButtonAct(bo, AG_BUTTON_FOCUS, _("Cancel"),
		    quit_cancel, "%p", win);
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
		Free(oent, M_OBJECT);
	}
	for (oent = TAILQ_FIRST(&gobjs);
	     oent != TAILQ_END(&gobjs);
	     oent = noent) {
		noent = TAILQ_NEXT(oent, objs);
		Free(oent, M_OBJECT);
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
