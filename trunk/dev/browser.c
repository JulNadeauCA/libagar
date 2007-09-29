/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Generic object browser.
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

#include "dev.h"

#ifdef NETWORK
#include <core/rcs.h>
#endif

#include <string.h>
#include <ctype.h>

#include "dev.h"

struct objent {
	AG_Object *obj;
	AG_Window *win;
	TAILQ_ENTRY(objent) objs;
};
static TAILQ_HEAD(,objent) dobjs;
static TAILQ_HEAD(,objent) gobjs;
static int editNowFlag = 1;
static void *lastSelectedParent = NULL;

static void
CreateObject(AG_Event *event)
{
	char name[AG_OBJECT_NAME_MAX];
	AG_ObjectType *t = AG_PTR(1);
	AG_Textbox *name_tb = AG_PTR(2);
	AG_Tlist *tlObjs = AG_PTR(3);
	AG_Window *dlg_win = AG_PTR(4);
	AG_TlistItem *it;
	AG_Object *pobj;
	void *nobj;

	if ((it = AG_TlistSelectedItem(tlObjs)) != NULL) {
		pobj = it->p1;
	} else {
		pobj = agWorld;
	}
	AG_TextboxCopyString(name_tb, name, sizeof(name));
	AG_ViewDetach(dlg_win);

	if (name[0] == '\0')
		AG_ObjectGenName(agWorld, t->ops, name, sizeof(name));

	nobj = Malloc(t->ops->size, M_OBJECT);
	if (t->ops->init != NULL) {
		t->ops->init(nobj, name);
	} else {
		AG_ObjectInit(nobj, name, t->ops);
	}
	AG_ObjectAttach(pobj, nobj);
	AG_ObjectUnlinkDatafiles(nobj);

	AG_PostEvent(NULL, nobj, "edit-create", NULL);
	
	if (editNowFlag && t->ops->edit != NULL)
		DEV_BrowserOpenData(nobj);
}

enum {
	OBJEDIT_EDIT_DATA,
	OBJEDIT_EDIT_GENERIC,
	OBJEDIT_LOAD,
	OBJEDIT_SAVE,
	OBJEDIT_SAVE_ALL,
	OBJEDIT_EXPORT,
	OBJEDIT_FREE_DATASET,
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
CloseGenericDlg(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	struct objent *oent = AG_PTR(1);

	AG_ViewDetach(win);
	TAILQ_REMOVE(&gobjs, oent, objs);
	Free(oent, M_OBJECT);
}

void
DEV_BrowserOpenGeneric(AG_Object *ob)
{
	struct objent *oent;

	TAILQ_FOREACH(oent, &gobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		AG_WindowShow(oent->win);
		agView->winToFocus = oent->win;
		return;
	}
	
	oent = Malloc(sizeof(struct objent), M_OBJECT);
	oent->obj = ob;
	oent->win = AG_ObjectEdit(ob);
	TAILQ_INSERT_HEAD(&gobjs, oent, objs);
	AG_WindowShow(oent->win);

	AG_SetEvent(oent->win, "window-close", CloseGenericDlg, "%p", oent);
}

static void
SaveAndCloseObject(struct objent *oent, AG_Window *win, int save)
{
	AG_WindowHide(win);
	AG_PostEvent(NULL, oent->obj, "edit-close", NULL);
	AG_ViewDetach(win);
	TAILQ_REMOVE(&dobjs, oent, objs);

	if (!save) {
		agTerminating = 1;
	}
	AG_ObjectPageOut(oent->obj);

	agTerminating = 0;
	Free(oent, M_OBJECT);
}

static void
SaveChangesReturn(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);
	struct objent *oent = AG_PTR(2);
	int save = AG_INT(3);

	SaveAndCloseObject(oent, win, save);
}

static void
SaveChangesDlg(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	struct objent *oent = AG_PTR(1);

	if (AG_ObjectChanged(oent->obj)) {
		AG_Button *bOpts[3];
		AG_Window *wDlg;

		wDlg = AG_TextPromptOptions(bOpts, 3, _("Save changes to %s?"),
		    OBJECT(oent->obj)->name);
		AG_WindowAttach(win, wDlg);
		{
			AG_ButtonText(bOpts[0], _("Save"));
			AG_SetEvent(bOpts[0], "button-pushed",
			    SaveChangesReturn, "%p,%p,%i", win, oent, 1);
			AG_WidgetFocus(bOpts[0]);

			AG_ButtonText(bOpts[1], _("Discard"));
			AG_SetEvent(bOpts[1], "button-pushed",
			    SaveChangesReturn, "%p,%p,%i", win, oent, 0);

			AG_ButtonText(bOpts[2], _("Cancel"));
			AG_SetEvent(bOpts[2], "button-pushed",
			    AGWINDETACH(wDlg));
		}
	} else {
		SaveAndCloseObject(oent, win, 1);
	}
}

void
DEV_BrowserOpenData(void *p)
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
		agView->winToFocus = oent->win;
		return;
	}
	
	if (ob->ops->edit == NULL)
		return;

	if (OBJECT_PERSISTENT(ob) &&
	    !OBJECT_RESIDENT(ob)) {
		if (AG_ObjectLoad(ob) == -1 ||
		    AG_ObjectLoadData(ob, &dataFound) == -1) {
			if (!dataFound) {
				/*
				 * Data not found in storage, so assume
				 * object is new. Mark it resident and save it.
				 */
				dprintf("%s: new object\n", ob->name);
				ob->flags |= AG_OBJECT_RESIDENT;
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
		AG_PostEvent(NULL, ob, "edit-post-load", NULL);
	}
	if ((win = ob->ops->edit(ob)) == NULL) {
		goto fail;
	}
	AG_PostEvent(NULL, ob, "edit-open", NULL);
	
	oent = Malloc(sizeof(struct objent), M_OBJECT);
	oent->obj = ob;
	oent->win = win;
	TAILQ_INSERT_HEAD(&dobjs, oent, objs);
	AG_SetEvent(win, "window-close", SaveChangesDlg, "%p", oent);
	AG_WindowShow(win);
	return;
fail:
	AG_ObjectPageOut(ob);
	return;
}

static void
ExportObject(AG_Event *event)
{
	AG_Object *ob = AG_PTR(1);
	AG_Window *win = AG_PTR(2);
	char *path = AG_STRING(3);
	char *pfx_save = ob->save_pfx;
	int loadedTmp = 0;
	int dataFound;

	/* Load the object temporarily if it is non-resident. */
	if (!OBJECT_RESIDENT(ob)) {
		if (AG_ObjectLoadData(ob, &dataFound) == -1) {
			if (!dataFound) {
				/* Object was never saved before. */
				ob->flags |= AG_OBJECT_RESIDENT;
			} else {
				AG_TextMsg(AG_MSG_ERROR,
				    _("%s: Loading failed (non-resident): %s"),
				    ob->name, AG_GetError());
				return;
			}
		}
		AG_PostEvent(NULL, ob, "edit-post-load", NULL);
		loadedTmp = 1;
	}
	if (AG_ObjectSaveToFile(ob, path) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name, AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Object `%s' was exported successfully."), ob->name);
	}
	if (loadedTmp)
		AG_ObjectFreeDataset(ob);
}

static void
ImportObject(AG_Event *event)
{
	AG_Object *ob = AG_PTR(1);
	AG_Window *win = AG_PTR(2);
	char *path = AG_STRING(3);
	int loadedTmp = 0;
	int dataFound;

	/* Load the object temporarily if it is non-resident. */
	if (!OBJECT_RESIDENT(ob)) {
		if (AG_ObjectLoadData(ob, &dataFound) == -1) {
			if (!dataFound) {
				/* Object was never saved before. */
				ob->flags |= AG_OBJECT_RESIDENT;
			} else {
				AG_TextMsg(AG_MSG_ERROR,
				    _("%s: Loading failed (non-resident): %s"),
				    ob->name, AG_GetError());
				return;
			}
		}
		loadedTmp = 1;
		AG_PostEvent(NULL, ob, "edit-post-load", NULL);
	}
	if (AG_ObjectLoadFromFile(ob, path) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name, AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Object `%s' was imported successfully."), ob->name);
	}
	if (loadedTmp)
		AG_ObjectFreeDataset(ob);
}

void
DEV_BrowserSaveTo(void *p, const char *name)
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
	AG_FileDlgAddType(fd, name, ext, ExportObject, "%p,%p", ob, win);
	AG_FileDlgSetDirectory(fd, AG_String(agConfig, "save-path"));
	AG_FileDlgSetFilename(fd, "%s.%s", ob->name, ob->ops->type);
	AG_WindowShow(win);
}

void
DEV_BrowserLoadFrom(void *p, const char *name)
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
	AG_FileDlgAddType(fd, name, ext, ImportObject, "%p,%p", ob, win);
	AG_FileDlgSetDirectory(fd, AG_String(agConfig, "save-path"));
	AG_FileDlgSetFilename(fd, "%s.%s", ob->name, ob->ops->type);
	AG_WindowShow(win);
}

static void
ObjectOp(AG_Event *event)
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
				DEV_BrowserOpenData(ob);
			} else {
				AG_TextTmsg(AG_MSG_ERROR, 750,
				    _("Object `%s' has no edit operation."),
				    ob->name);
			}
			break;
		case OBJEDIT_EDIT_GENERIC:
			DEV_BrowserOpenGeneric(ob);
			break;
		case OBJEDIT_LOAD:
			if (AG_ObjectLoad(ob) == -1) {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
			} else {
				AG_PostEvent(NULL, ob, "edit-post-load", NULL);
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
			if (!OBJECT_PERSISTENT(ob)) {
				AG_SetError(
				    _("The `%s' object is non-persistent."),
				    ob->name);
			} else {
				DEV_BrowserSaveTo(ob, _("Agar object file"));
			}
			break;
		case OBJEDIT_DUP:
			{
				char dupName[AG_OBJECT_NAME_MAX];
				AG_Object *dob;

				if (ob == agWorld || !OBJECT_PERSISTENT(ob)) {
					AG_TextMsg(AG_MSG_ERROR,
					    _("%s: cannot duplicate."),
					    ob->name);
					break;
				}
				AG_ObjectGenName(ob->parent, ob->ops, dupName,
				    sizeof(dupName));
				if ((dob = AG_ObjectDuplicate(ob, dupName))
				    == NULL) {
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
		case OBJEDIT_FREE_DATASET:
			if (it->p1 == agWorld) {
				continue;
			}
			AG_ObjectFreeDataset(ob);
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
DEV_BrowserGenericSave(AG_Event *event)
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
DEV_BrowserGenericLoad(AG_Event *event)
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
DEV_BrowserGenericSaveTo(AG_Event *event)
{
	DEV_BrowserSaveTo(AG_PTR(1), _("Agar object file"));
}

static void
DEV_BrowserGenericLoadFrom(AG_Event *event)
{
	DEV_BrowserLoadFrom(AG_PTR(1), _("Agar object file"));
}

void
DEV_BrowserGenericMenu(void *menup, void *obj)
{
	AG_MenuItem *pitem = menup;

	AG_MenuAction(pitem, _("Save"), OBJSAVE_ICON,
	    DEV_BrowserGenericSave, "%p", obj);
	AG_MenuAction(pitem, _("Load"), OBJLOAD_ICON,
	    DEV_BrowserGenericLoad, "%p", obj);
	AG_MenuAction(pitem, _("Export to..."), OBJSAVE_ICON,
	    DEV_BrowserGenericSaveTo, "%p", obj);
	AG_MenuAction(pitem, _("Import from..."), OBJLOAD_ICON,
	    DEV_BrowserGenericLoadFrom, "%p", obj);

	AG_MenuSeparator(pitem);

	AG_MenuUintFlags(pitem, _("Persistence"), OBJLOAD_ICON,
	    &OBJECT(obj)->flags, AG_OBJECT_NON_PERSISTENT, 1);
	AG_MenuUintFlags(pitem, _("Destructible"), TRASH_ICON,
	    &OBJECT(obj)->flags, AG_OBJECT_INDESTRUCTIBLE, 1);
	AG_MenuUintFlags(pitem, _("Editable"), OBJ_ICON,
	    &OBJECT(obj)->flags, AG_OBJECT_READONLY, 1);
}

static AG_TlistItem *
PollObjectsFind(AG_Tlist *tl, AG_Object *pob, int depth)
{
	char label[AG_TLIST_LABEL_MAX];
	AG_Object *cob;
	AG_TlistItem *it;
	SDL_Surface *icon;

	strlcpy(label, pob->name, sizeof(label));
	if (OBJECT_RESIDENT(pob)) {
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
			PollObjectsFind(tl, cob, depth+1);
	}
	return (it);
}

static void
PollObjects(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Object *pob = AG_PTR(1);
	AG_Object *dob = AG_PTR(2);
	AG_TlistItem *it;

	AG_LockLinkage();
	AG_TlistClear(tl);
	PollObjectsFind(tl, pob, 0);
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
LoadObject(AG_Event *event)
{
	AG_Object *o = AG_PTR(1);

	if (AG_ObjectLoad(o) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", OBJECT(o)->name,
		    AG_GetError());
	}
}

static void
SaveObject(AG_Event *event)
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
ExitProgram(AG_Event *event)
{
	SDL_Event ev;

	ev.type = SDL_QUIT;
	SDL_PushEvent(&ev);
}

static void
ShowPreferences(AG_Event *event)
{
	if (!agConfig->window->visible) {
		AG_WindowShow(agConfig->window);
	} else {
		AG_WindowFocus(agConfig->window);
	}
}

static void
CreateObjectDlg(AG_Event *event)
{
	AG_Window *win;
	AG_ObjectType *t = AG_PTR(1);
	AG_Window *pwin = AG_PTR(2);
	AG_Tlist *tlParents;
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

		tlParents = AG_TlistNewPolled(bo,
		    AG_TLIST_POLL|AG_TLIST_TREE|AG_TLIST_EXPAND,
		    PollObjects, "%p,%p", agWorld, lastSelectedParent);
		AG_TlistSizeHint(tlParents, "XXXXXXXXXXXXXXXXXXX", 5);
		AG_WidgetBind(tlParents, "selected", AG_WIDGET_POINTER,
		    &lastSelectedParent);
	}

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		cb = AG_CheckboxNew(win, 0, _("Edit now"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT, &editNowFlag);
	}

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		AG_ButtonNewFn(bo, 0, _("OK"), CreateObject, "%p,%p,%p,%p",
		    t, tb, tlParents, win);
		AG_SetEvent(tb, "textbox-return", CreateObject, "%p,%p,%p,%p",
		    t, tb, tlParents, win);
		
		AG_ButtonNewFn(bo, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

#ifdef NETWORK

static void
PollRevisions(AG_Event *event)
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
UpdateFromRepo(AG_Event *event)
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
DeleteFromRepo(AG_Event *event)
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
RepoRenameObject(AG_Event *event)
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
RepoRenameDlg(AG_Event *event)
{
	char prompt[AG_LABEL_MAX];
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	
	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;
	
	snprintf(prompt, sizeof(prompt), _("Rename %s to:"), it->text);
	AG_TextPromptString(prompt, RepoRenameObject, "%p,%s", tl, it->text);
}

#endif /* NETWORK */

/* Create the object browser window. */
AG_Window *
DEV_Browser(void)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_Textbox *name_tb;
	AG_Tlist *tlObjs;
	AG_Menu *me;
	AG_MenuItem *mi, *mi_objs;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;

	win = AG_WindowNewNamed(0, "DEV_Browser");
	AG_WindowSetCaption(win, _("Object Browser"));
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_LEFT, 0);
	
	tlObjs = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(tlObjs, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_TREE|
	                      AG_TLIST_EXPAND);
	AG_TlistSizeHint(tlObjs, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 10);
	AG_SetEvent(tlObjs, "tlist-poll", PollObjects, "%p,%p", agWorld, NULL);
	AG_SetEvent(tlObjs, "tlist-dblclick", ObjectOp, "%p, %i", tlObjs,
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
			    CreateObjectDlg, "%p,%p", t, win);
		}

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Load full state"), OBJLOAD_ICON,
		    LoadObject, "%p", agWorld);
		AG_MenuAction(mi, _("Save full state"), OBJSAVE_ICON,
		    SaveObject, "%p", agWorld);
		
		AG_MenuSeparator(mi);
		
		AG_MenuAction(mi, _("Exit"), -1, ExitProgram, NULL);
	}
	
	mi = AG_MenuAddItem(me, _("Edit"));
	{
		AG_MenuAction(mi, _("Edit object data..."), OBJEDIT_ICON,
		    ObjectOp, "%p, %i", tlObjs, OBJEDIT_EDIT_DATA);
		AG_MenuAction(mi, _("Edit generic information..."),
		    OBJGENEDIT_ICON,
		    ObjectOp, "%p, %i", tlObjs, OBJEDIT_EDIT_GENERIC);
		
		AG_MenuSeparator(mi);
			
		AG_MenuAction(mi, _("Duplicate"), OBJDUP_ICON,
		    ObjectOp, "%p, %i", tlObjs, OBJEDIT_DUP);
		AG_MenuActionKb(mi, _("Move up"), OBJMOVEUP_ICON,
		    SDLK_u, KMOD_SHIFT, ObjectOp, "%p, %i", tlObjs,
		    OBJEDIT_MOVE_UP);
		AG_MenuActionKb(mi, _("Move down"), OBJMOVEDOWN_ICON,
		    SDLK_d, KMOD_SHIFT, ObjectOp, "%p, %i", tlObjs,
		    OBJEDIT_MOVE_DOWN);

		if (agConfig->window != NULL) {
			AG_MenuSeparator(mi);
			AG_MenuAction(mi, _("Preferences..."), -1,
			    ShowPreferences, NULL);
		}
	}

#ifdef NETWORK
	mi = AG_MenuAddItem(me, _("Repository"));
	{
		AG_MenuAction(mi, _("Commit"),
		    OBJLOAD_ICON, ObjectOp, "%p, %i", tlObjs,
		    OBJEDIT_RCS_COMMIT);

		AG_MenuAction(mi, _("Update"),
		    OBJLOAD_ICON, ObjectOp, "%p, %i", tlObjs,
		    OBJEDIT_RCS_UPDATE);

		AG_MenuAction(mi, _("Import"),
		    OBJSAVE_ICON, ObjectOp, "%p, %i", tlObjs,
		    OBJEDIT_RCS_IMPORT);

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Commit all"),
		    OBJLOAD_ICON, ObjectOp, "%p, %i", tlObjs,
		    OBJEDIT_RCS_COMMIT_ALL);

		AG_MenuAction(mi, _("Update all"),
		    OBJLOAD_ICON, ObjectOp, "%p, %i", tlObjs,
		    OBJEDIT_RCS_UPDATE_ALL);
			
		
		AG_MenuAction(mi, _("Import all"),
		    OBJSAVE_ICON, ObjectOp, "%p, %i", tlObjs,
		    OBJEDIT_RCS_IMPORT_ALL);
	}
#endif /* NETWORK */
	
#ifdef DEBUG
	mi = AG_MenuAddItem(me, _("Debug"));
	DEV_ToolMenu(mi);
#endif /* DEBUG */

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAddTab(nb, _("Working copy"), AG_BOX_VERT);
	{
		AG_MenuItem *mi, *mi2;

		AG_ObjectAttach(ntab, tlObjs);

		mi = AG_TlistSetPopup(tlObjs, "object");
		{
			AG_MenuAction(mi, _("Edit object..."), OBJEDIT_ICON,
			    ObjectOp, "%p, %i", tlObjs, OBJEDIT_EDIT_DATA);
			AG_MenuAction(mi, _("Edit object information..."),
			    OBJGENEDIT_ICON,
			    ObjectOp, "%p, %i", tlObjs, OBJEDIT_EDIT_GENERIC);

			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Load"), OBJLOAD_ICON, ObjectOp,
			    "%p, %i", tlObjs, OBJEDIT_LOAD);
			AG_MenuAction(mi, _("Save"), OBJSAVE_ICON, ObjectOp,
			    "%p, %i", tlObjs, OBJEDIT_SAVE);
			AG_MenuAction(mi, _("Save all"), OBJSAVE_ICON, ObjectOp,
			    "%p, %i", tlObjs, OBJEDIT_SAVE_ALL);
			AG_MenuAction(mi, _("Save to..."), OBJSAVE_ICON, ObjectOp,
			    "%p, %i", tlObjs, OBJEDIT_EXPORT);

#ifdef NETWORK
			if (agRcsMode) {
				AG_MenuSeparator(mi);
			
				mi2 = AG_MenuAction(mi, _("Repository"),
				    OBJLOAD_ICON, NULL, NULL);

				AG_MenuAction(mi2, _("Commit"),
				    OBJLOAD_ICON, ObjectOp, "%p, %i", tlObjs,
				    OBJEDIT_RCS_COMMIT);
				
				AG_MenuAction(mi2, _("Update"),
				    OBJLOAD_ICON, ObjectOp, "%p, %i", tlObjs,
				    OBJEDIT_RCS_UPDATE);
				
				AG_MenuAction(mi2, _("Import"),
				    OBJSAVE_ICON, ObjectOp, "%p, %i", tlObjs,
				    OBJEDIT_RCS_IMPORT);
		
				AG_MenuSeparator(mi2);
				
				AG_MenuAction(mi2, _("Commit all"),
				    OBJLOAD_ICON, ObjectOp, "%p, %i", tlObjs,
				    OBJEDIT_RCS_COMMIT_ALL);

				AG_MenuAction(mi2, _("Update all"),
				    OBJLOAD_ICON, ObjectOp, "%p, %i", tlObjs,
				    OBJEDIT_RCS_UPDATE_ALL);
			
				AG_MenuAction(mi2, _("Import all"),
				    OBJSAVE_ICON, ObjectOp, "%p, %i", tlObjs,
				    OBJEDIT_RCS_IMPORT_ALL);
			}
#endif /* NETWORK */
			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Duplicate"), OBJDUP_ICON,
			    ObjectOp, "%p, %i", tlObjs, OBJEDIT_DUP);
			AG_MenuActionKb(mi, _("Move up"), OBJMOVEUP_ICON,
			    SDLK_u, KMOD_SHIFT, ObjectOp, "%p, %i", tlObjs,
			    OBJEDIT_MOVE_UP);
			AG_MenuActionKb(mi, _("Move down"), OBJMOVEDOWN_ICON,
			    SDLK_d, KMOD_SHIFT, ObjectOp, "%p, %i", tlObjs,
			    OBJEDIT_MOVE_DOWN);
			
			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Free dataset"), OBJREINIT_ICON,
			    ObjectOp, "%p, %i", tlObjs, OBJEDIT_FREE_DATASET);
			AG_MenuAction(mi, _("Destroy"), TRASH_ICON,
			    ObjectOp, "%p, %i", tlObjs, OBJEDIT_DESTROY);
		}
	}

#ifdef NETWORK
	ntab = AG_NotebookAddTab(nb, _("Repository"), AG_BOX_VERT);
	{
		AG_Tlist *tl;
		AG_Button *btn;
		AG_MenuItem *mi;

		tl = AG_TlistNew(ntab, AG_TLIST_MULTI|AG_TLIST_TREE|
		                       AG_TLIST_EXPAND);
		AG_TlistSetCompareFn(tl, AG_TlistCompareStrings);
		mi = AG_TlistSetPopup(tl, "object");
		{
			AG_MenuAction(mi, _("Update from repository"),
			    OBJLOAD_ICON, UpdateFromRepo, "%p", tl);
			AG_MenuAction(mi, _("Delete from repository"),
			    TRASH_ICON, DeleteFromRepo, "%p", tl);
			AG_MenuAction(mi, _("Rename"),
			    -1, RepoRenameDlg, "%p", tl);
		}

		btn = AG_ButtonNewFn(ntab, AG_BUTTON_HFILL,
		    _("Refresh revisions"),
		    PollRevisions, "%p", tl);
	
		if (agRcsMode)
			AG_PostEvent(NULL, btn, "button-pushed", NULL);
	}
#endif /* NETWORK */
	
	AG_WindowShow(win);
	return (win);
}

/*
 * Post AG_ObjectLoad() callback for all objects. If the REOPEN_ONLOAD
 * flag is set, we automatically close and re-open all edition windows
 * associated with the object after the load.
 */
static void
DEV_PostLoadDataCallback(AG_Event *event)
{
	AG_Object *obj = AG_SENDER();	
	struct objent *oent;

	if ((obj->flags & AG_OBJECT_REOPEN_ONLOAD) == 0)
		return;

	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj == obj) {
			AG_WindowHide(oent->win);
			AG_PostEvent(NULL, oent->obj, "edit-close", NULL);
			AG_ViewDetach(oent->win);

			AG_PostEvent(NULL, oent->obj, "edit-open", NULL);
			oent->win = obj->ops->edit(obj);
			AG_WindowShow(oent->win);
			AG_SetEvent(oent->win, "window-close",
			    SaveChangesDlg, "%p", oent);
		}
	}
}

/*
 * Callback invoked whenever an object is paged out. We save the object
 * unless the application is terminating.
 */
static void
DEV_PageOutCallback(AG_Event *event)
{
	AG_Object *obj = AG_SENDER();
	
	if (!agTerminating)
		if (AG_ObjectSave(obj) == -1)
			AG_TextMsgFromError();
}

static void
ConfirmQuit(AG_Event *event)
{
	SDL_Event nev;

	agTerminating = 0;
	nev.type = SDL_USEREVENT;
	SDL_PushEvent(&nev);
}

static void
AbortQuit(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	agTerminating = 0;
	AG_ViewDetach(win);
}


/*
 * Callback invoked when the user has requested termination of the
 * application. We check whether objects have been modified and not
 * saved and prompt the user if that's the case.
 */
static void
DEV_QuitCallback(AG_Event *event)
{
	AG_Window *win;
	AG_Box *bo;
	
	agTerminating = 1;

	if (!AG_ObjectChangedAll(agWorld)) {
		SDL_Event nev;

		nev.type = SDL_USEREVENT;
		SDL_PushEvent(&nev);
		return;
	}
	
	if ((win = AG_WindowNewNamed(AG_WINDOW_MODAL|AG_WINDOW_NOTITLE|
	    AG_WINDOW_NORESIZE, "DEV_QuitCallback")) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Exit application?"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);
	AG_LabelNewStaticString(win, 0, _("Unsaved objects have been modified. "
	                                  "Exit application?"));
	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_VBOX_HFILL);
	AG_BoxSetSpacing(bo, 0);
	AG_BoxSetPadding(bo, 0);
	AG_ButtonNewFn(bo, 0, _("Quit"), ConfirmQuit, NULL);
	AG_WidgetFocus(AG_ButtonNewFn(bo, 0, _("Cancel"), AbortQuit,
	    "%p", win));
	AG_WindowShow(win);
}

void
DEV_BrowserInit(void)
{
	TAILQ_INIT(&dobjs);
	TAILQ_INIT(&gobjs);

	AG_AddEvent(agWorld, "object-post-load-data", DEV_PostLoadDataCallback,
	    NULL);
	AG_AddEvent(agWorld, "object-page-out", DEV_PageOutCallback, NULL);
	AG_AddEvent(agWorld, "quit", DEV_QuitCallback, NULL);
}

void
DEV_BrowserDestroy(void)
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

void
DEV_BrowserCloseData(void *p)
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
