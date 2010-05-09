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
#include <core/config.h>
#ifdef AG_NETWORK
#include <core/rcs.h>
#endif

#include <gui/gui.h>
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
#include <gui/icons.h>

#include <string.h>
#include <ctype.h>

#include "dev.h"

struct objent {
	AG_Object *obj;
	AG_Window *win;
	TAILQ_ENTRY(objent) objs;
};
static TAILQ_HEAD_(objent) dobjs;
static TAILQ_HEAD_(objent) gobjs;
static int editNowFlag = 1;
static void *lastSelectedParent = NULL;

static void
CreateObject(AG_Event *event)
{
	char name[AG_OBJECT_NAME_MAX];
	AG_Object *vfsRoot = AG_PTR(1);
	AG_ObjectClass *cl = AG_PTR(2);
	AG_Textbox *name_tb = AG_PTR(3);
	AG_Tlist *tlObjs = AG_PTR(4);
	AG_Window *dlg_win = AG_PTR(5);
	AG_TlistItem *it;
	AG_Object *pobj;
	void *nobj;

	if ((it = AG_TlistSelectedItem(tlObjs)) != NULL) {
		pobj = it->p1;
	} else {
		pobj = vfsRoot;
	}
	AG_TextboxCopyString(name_tb, name, sizeof(name));
	AG_ObjectDetach(dlg_win);

	if (name[0] == '\0')
		AG_ObjectGenName(vfsRoot, cl, name, sizeof(name));

	nobj = Malloc(cl->size);
	AG_ObjectInit(nobj, cl);
	AG_ObjectSetNameS(nobj, name);
	AG_ObjectAttach(pobj, nobj);
	AG_ObjectUnlinkDatafiles(nobj);

	AG_PostEvent(NULL, nobj, "edit-create", NULL);
	
	if (editNowFlag && cl->edit != NULL)
		DEV_BrowserOpenData(nobj);
}

enum {
	OBJEDIT_EDIT_DATA,
	OBJEDIT_EDIT_GENERIC,
	OBJEDIT_LOAD,
	OBJEDIT_SAVE,
	OBJEDIT_SAVE_ALL,		/* unused */
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

	AG_ObjectDetach(win);
	TAILQ_REMOVE(&gobjs, oent, objs);
	Free(oent);
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
		AG_WindowFocus(oent->win);
		return;
	}
	
	oent = Malloc(sizeof(struct objent));
	oent->obj = ob;
	oent->win = DEV_ObjectEdit(ob);
	TAILQ_INSERT_HEAD(&gobjs, oent, objs);
	AG_WindowShow(oent->win);

	AG_SetEvent(oent->win, "window-close", CloseGenericDlg, "%p", oent);
}

static void
SaveAndCloseObject(struct objent *oent, AG_Window *win, int save)
{
	AG_WindowHide(win);
	AG_PostEvent(NULL, oent->obj, "edit-close", NULL);
	AG_ObjectDetach(win);
	TAILQ_REMOVE(&dobjs, oent, objs);

	if (!save) {
		agTerminating = 1;
	}
	AG_ObjectPageOut(oent->obj);

	agTerminating = 0;
	Free(oent);
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
	int dataFound = 0;

	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		AG_WindowShow(oent->win);
		AG_WindowFocus(oent->win);
		return;
	}
	
	if (ob->cls->edit == NULL)
		return;

	if (OBJECT_PERSISTENT(ob) &&
	    !OBJECT_RESIDENT(ob)) {
		if (AG_ObjectLoadGenericFromFile(ob, NULL) == -1 ||
		    AG_ObjectResolveDeps(ob) == -1 ||
		    AG_ObjectLoadDataFromFile(ob, &dataFound, NULL) == -1) {
			if (!dataFound) {
				/*
				 * Data not found in storage, so assume
				 * object is new. Mark it resident and save it.
				 */
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
	if ((win = ob->cls->edit(ob)) == NULL) {
		goto fail;
	}
	AG_PostEvent(NULL, ob, "edit-open", NULL);
	
	oent = Malloc(sizeof(struct objent));
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
SaveObjectToFile(AG_Event *event)
{
	AG_Object *ob = AG_PTR(1);
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
	char ext[AG_OBJECT_HIER_MAX+3];
	AG_Object *ob = p;
	AG_Window *win;
	AG_FileDlg *fd;

	ext[0] = '*';
	ext[1] = '.';
	Strlcpy(&ext[2], ob->cls->name, sizeof(ext)-2);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Save %s to..."), ob->name);
	fd = AG_FileDlgNewMRU(win, "dev.mru.object-import",
	    AG_FILEDLG_CLOSEWIN|AG_FILEDLG_SAVE| AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, name, ext, SaveObjectToFile, "%p,%p", ob, win);
	AG_FileDlgSetFilename(fd, "%s.%s", ob->name, ob->cls->name);
	AG_WindowShow(win);
}

void
DEV_BrowserLoadFrom(void *p, const char *name)
{
	char ext[AG_OBJECT_HIER_MAX+3];
	AG_Object *ob = p;
	AG_Window *win;
	AG_FileDlg *fd;

	ext[0] = '*';
	ext[1] = '.';
	Strlcpy(&ext[2], ob->cls->name, sizeof(ext)-2);

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Load %s from..."), ob->name);
	fd = AG_FileDlgNewMRU(win, "dev.mru.object-import",
	    AG_FILEDLG_CLOSEWIN|AG_FILEDLG_LOAD|AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, name, ext, ImportObject, "%p,%p", ob, win);
	AG_FileDlgSetFilename(fd, "%s.%s", ob->name, ob->cls->name);
	AG_WindowShow(win);
}

static void
ObjectOp(AG_Event *event)
{
	void *vfsRoot = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
	int op = AG_INT(3);
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		AG_Object *ob = it->p1;

		if (!it->selected)
			continue;

		switch (op) {
		case OBJEDIT_EDIT_DATA:
			if (ob->cls->edit != NULL) {
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
			{
				AG_Event ev;

				AG_EventInit(&ev);
				AG_EventPushPointer(&ev, "", ob);
				AG_EventPushString(&ev, "", ob->archivePath);
				SaveObjectToFile(&ev);
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

				if (ob == vfsRoot || !OBJECT_PERSISTENT(ob)) {
					AG_TextMsg(AG_MSG_ERROR,
					    _("%s: cannot duplicate."),
					    ob->name);
					break;
				}
				AG_ObjectGenName(ob->parent, ob->cls, dupName,
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
			if (it->p1 == vfsRoot) {
				continue;
			}
			AG_ObjectFreeDataset(ob);
			break;
		case OBJEDIT_DESTROY:
			if (it->p1 == vfsRoot) {
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
			break;
#ifdef AG_NETWORK
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
				AG_TextMsgFromError();
			}
			break;
		case OBJEDIT_RCS_UPDATE_ALL:
			AG_RcsUpdateAll(ob);
			break;
#endif /* AG_NETWORK */
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

	AG_MenuAction(pitem, _("Save"), agIconSave.s,
	    DEV_BrowserGenericSave, "%p", obj);
	AG_MenuAction(pitem, _("Load"), agIconLoad.s,
	    DEV_BrowserGenericLoad, "%p", obj);
	AG_MenuAction(pitem, _("Export to..."), agIconSave.s,
	    DEV_BrowserGenericSaveTo, "%p", obj);
	AG_MenuAction(pitem, _("Import from..."), agIconLoad.s,
	    DEV_BrowserGenericLoadFrom, "%p", obj);

	AG_MenuSeparator(pitem);

	AG_MenuUintFlags(pitem, _("Persistence"), agIconLoad.s,
	    &OBJECT(obj)->flags, AG_OBJECT_NON_PERSISTENT, 1);
	AG_MenuUintFlags(pitem, _("Destructible"), agIconTrash.s,
	    &OBJECT(obj)->flags, AG_OBJECT_INDESTRUCTIBLE, 1);
	AG_MenuUintFlags(pitem, _("Editable"), NULL,
	    &OBJECT(obj)->flags, AG_OBJECT_READONLY, 1);
}

static AG_TlistItem *
PollObjectsFind(AG_Tlist *tl, AG_Object *pob, int depth)
{
	char label[AG_TLIST_LABEL_MAX];
	AG_Object *cob;
	AG_TlistItem *it;

	Strlcpy(label, pob->name, sizeof(label));
	if (OBJECT_RESIDENT(pob)) {
		Strlcat(label, _(" (resident)"), sizeof(label));
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

	AG_LockVFS(pob);
	AG_TlistClear(tl);
	PollObjectsFind(tl, pob, 0);
	AG_TlistRestore(tl);
	AG_UnlockVFS(pob);

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
	AG_QuitGUI();
}

static void
ShowPreferences(AG_Event *event)
{
	DEV_ConfigShow();
}

static void
CreateObjectDlg(AG_Event *event)
{
	AG_Window *win;
	AG_Object *vfsRoot = AG_PTR(1);
	AG_ObjectClass *cl = AG_PTR(2);
	AG_Window *pwin = AG_PTR(3);
	AG_Tlist *tlParents;
	AG_Box *bo;
	AG_Textbox *tb;

	win = AG_WindowNew(AG_WINDOW_NOCLOSE|AG_WINDOW_NOMINIMIZE);
	AG_WindowSetCaption(win, _("New %s object"), cl->name);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		AG_LabelNew(bo, 0, _("Class: %s"), cl->hier);
		tb = AG_TextboxNew(bo, AG_TEXTBOX_HFILL, _("Name: "));
		AG_WidgetFocus(tb);
	}

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL|AG_BOX_VFILL);
	AG_BoxSetPadding(bo, 0);
	AG_BoxSetSpacing(bo, 0);
	{
		AG_LabelNewS(bo, 0, _("Parent object:"));

		tlParents = AG_TlistNewPolled(bo,
		    AG_TLIST_POLL|AG_TLIST_TREE|AG_TLIST_EXPAND,
		    PollObjects, "%p,%p", vfsRoot, lastSelectedParent);
		AG_TlistSizeHint(tlParents, "XXXXXXXXXXXXXXXXXXX", 5);
		AG_BindPointer(tlParents, "selected",
		    (void **)&lastSelectedParent);
	}

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		AG_CheckboxNewInt(win, 0, _("Edit now"), &editNowFlag);
	}

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		AG_ButtonNewFn(bo, 0, _("OK"),
		    CreateObject, "%p,%p,%p,%p,%p", vfsRoot, cl, tb, tlParents,
		    win);
		AG_SetEvent(tb, "textbox-return",
		    CreateObject, "%p,%p,%p,%p,%p", vfsRoot, cl, tb, tlParents,
		    win);
		AG_ButtonNewFn(bo, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowAttach(pwin, win);
	AG_WindowShow(win);
}

#ifdef AG_NETWORK

static void
PollRevisions(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	AG_RCSList list;
	int i;

	if (!agRcsMode) {
		AG_TextMsg(AG_MSG_ERROR, _("RCS is currently disabled."));
		return;
	}
	if (AG_RcsConnect() == -1 ||
	    AG_RcsGetList(&list) == -1) {
		AG_TextMsgFromError();
		goto out;
	}

	AG_TlistClear(tl);
	it = AG_TlistAdd(tl, NULL, "/");
	it->flags |= AG_TLIST_HAS_CHILDREN;
	it->cat = "object";
	it->depth = 0;

	for (i = 0; i < list.nEnts; i++) {
		AG_RCSListEntry *lent = &list.ents[i];
		int depth = 0;
		char *c;

		for (c = &lent->name[0]; *c != '\0'; c++) {
			if (*c == '/')
				depth++;
		}
		it = AG_TlistAddS(tl, NULL, &lent->name[1]);
		it->cat = "object";
		it->depth = depth;
	}
	AG_TlistRestore(tl);
	AG_RcsFreeList(&list);
out:
	AG_RcsDisconnect();
}

static void
UpdateFromRepo(AG_Event *event)
{
	void *vfsRoot = AG_PTR(1);
	AG_Tlist *tl = AG_PTR(2);
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (!it->selected)
			continue;

		if (AG_RcsCheckout(vfsRoot, it->text) == -1) {
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
#if 0
	if (AG_RcsConnect() == 0) {
		AG_RcsList(tl);
		AG_RcsDisconnect();
	}
#endif
}

static void
RepoRenameObject(AG_Event *event)
{
	char *from = AG_STRING(2);
	char *to = AG_STRING(3);

	if (AG_RcsRename(from, to) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", from, AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000, _("Object %s renamed to %s."),
		    from, to);
	}
#if 0
	if (AG_RcsConnect() == 0) {
		AG_RcsList(tl);
		AG_RcsDisconnect();
	}
#endif
}

static void
RepoRenameDlg(AG_Event *event)
{
	char prompt[AG_LABEL_MAX];
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	
	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;
	
	Snprintf(prompt, sizeof(prompt), _("Rename %s to:"), it->text);
	AG_TextPromptString(prompt, RepoRenameObject, "%p,%s", tl, it->text);
}

#endif /* AG_NETWORK */

static void
GenNewObjectMenu(AG_MenuItem *mParent, AG_ObjectClass *cls, AG_Object *vfsRoot,
    AG_Window *win)
{
	AG_ObjectClass *subcls;
	AG_MenuItem *mNode;

	mNode = AG_MenuNode(mParent, cls->name, NULL);

	AG_MenuAction(mNode, _("Create instance..."), NULL,
	    CreateObjectDlg, "%p,%p,%p", vfsRoot, cls, win);

	if (!TAILQ_EMPTY(&cls->sub)) {
		AG_MenuSeparator(mNode);
		TAILQ_FOREACH(subcls, &cls->sub, subclasses)
			GenNewObjectMenu(mNode, subcls, vfsRoot, win);
	}
}

/* Create the object browser window. */
AG_Window *
DEV_Browser(void *vfsRoot)
{
	AG_Window *win;
	AG_Tlist *tlObjs;
	AG_Menu *me;
	AG_MenuItem *mi, *mi_objs;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;

	win = AG_WindowNewNamedS(0, "DEV_Browser");
	AG_WindowSetCaptionS(win, OBJECT(vfsRoot)->name);
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_LEFT, 0);
	
	tlObjs = AG_TlistNew(NULL, AG_TLIST_POLL|AG_TLIST_MULTI|AG_TLIST_TREE|
	                           AG_TLIST_EXPAND);
	AG_TlistSizeHint(tlObjs, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", 10);
	AG_SetEvent(tlObjs, "tlist-poll",
	    PollObjects, "%p,%p", vfsRoot, NULL);
	AG_SetEvent(tlObjs, "tlist-dblclick",
	    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs, OBJEDIT_EDIT_DATA);

	me = AG_MenuNew(win, AG_MENU_HFILL);
	mi = AG_MenuNode(me->root, _("File"), NULL);
	{
		mi_objs = AG_MenuNode(mi, _("New object"), NULL);
		GenNewObjectMenu(mi_objs, agClassTree, vfsRoot, win);

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Load VFS"), agIconLoad.s,
		    LoadObject, "%p", vfsRoot);
		AG_MenuAction(mi, _("Save VFS"), agIconSave.s,
		    SaveObject, "%p", vfsRoot);
		
		AG_MenuSeparator(mi);
		
		AG_MenuAction(mi, _("Exit"), NULL, ExitProgram, NULL);
	}
	
	mi = AG_MenuNode(me->root, _("Edit"), NULL);
	{
		AG_MenuAction(mi, _("Edit object data..."), NULL,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
		    OBJEDIT_EDIT_DATA);
		AG_MenuAction(mi, _("Edit generic information..."), NULL,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
		    OBJEDIT_EDIT_GENERIC);
		
		AG_MenuSeparator(mi);
			
		AG_MenuAction(mi, _("Duplicate"), NULL,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs, OBJEDIT_DUP);
		AG_MenuActionKb(mi, _("Move up"), agIconUp.s,
		    AG_KEY_U, AG_KEYMOD_SHIFT,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs, OBJEDIT_MOVE_UP);
		AG_MenuActionKb(mi, _("Move down"), agIconDown.s,
		    AG_KEY_D, AG_KEYMOD_SHIFT,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs, OBJEDIT_MOVE_DOWN);

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Agar settings..."), NULL,
		    ShowPreferences, NULL);
	}

#ifdef AG_NETWORK
	mi = AG_MenuNode(me->root, _("Repository"), NULL);
	{
		AG_MenuAction(mi, _("Commit"), agIconLoad.s,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs, OBJEDIT_RCS_COMMIT);
		AG_MenuAction(mi, _("Update"), agIconLoad.s,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs, OBJEDIT_RCS_UPDATE);
		AG_MenuAction(mi, _("Import"), agIconSave.s,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs, OBJEDIT_RCS_IMPORT);
		AG_MenuSeparator(mi);
		AG_MenuAction(mi, _("Commit all"), agIconSave.s,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
		    OBJEDIT_RCS_COMMIT_ALL);
		AG_MenuAction(mi, _("Update all"), agIconLoad.s,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
		    OBJEDIT_RCS_UPDATE_ALL);
		AG_MenuAction(mi, _("Import all"), agIconSave.s,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
		    OBJEDIT_RCS_IMPORT_ALL);
	}
#endif /* AG_NETWORK */
	
#ifdef AG_DEBUG
	mi = AG_MenuNode(me->root, _("Debug"), NULL);
	DEV_ToolMenu(mi);
#endif /* AG_DEBUG */

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAddTab(nb, _("Working copy"), AG_BOX_VERT);
	{
		AG_MenuItem *mi;
#ifdef AG_NETWORK
		AG_MenuItem *mi2;
#endif

		AG_ObjectAttach(ntab, tlObjs);

		mi = AG_TlistSetPopup(tlObjs, "object");
		{
			AG_MenuAction(mi, _("Edit object..."), NULL,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_EDIT_DATA);
			AG_MenuAction(mi, _("Edit object information..."), NULL,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_EDIT_GENERIC);

			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Load"), agIconLoad.s,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_LOAD);
			AG_MenuAction(mi, _("Save"), agIconSave.s,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_SAVE);
			AG_MenuAction(mi, _("Save to..."), agIconSave.s,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_EXPORT);

#ifdef AG_NETWORK
			if (agRcsMode) {
				AG_MenuSeparator(mi);
			
				mi2 = AG_MenuAction(mi, _("Repository"),
				    agIconLoad.s, NULL, NULL);
				AG_MenuAction(mi2, _("Commit"), agIconLoad.s,
				    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
				    OBJEDIT_RCS_COMMIT);
				AG_MenuAction(mi2, _("Update"), agIconLoad.s,
				    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
				    OBJEDIT_RCS_UPDATE);
				AG_MenuAction(mi2, _("Import"), agIconLoad.s,
				    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
				    OBJEDIT_RCS_IMPORT);
		
				AG_MenuSeparator(mi2);
				
				AG_MenuAction(mi2, _("Commit all"),
				    agIconLoad.s,
				    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
				    OBJEDIT_RCS_COMMIT_ALL);
				AG_MenuAction(mi2, _("Update all"),
				    agIconLoad.s,
				    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
				    OBJEDIT_RCS_UPDATE_ALL);
				AG_MenuAction(mi2, _("Import all"),
				    agIconSave.s,
				    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
				    OBJEDIT_RCS_IMPORT_ALL);
			}
#endif /* AG_NETWORK */
			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Duplicate"), NULL,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs, OBJEDIT_DUP);
			AG_MenuActionKb(mi, _("Move up"), agIconUp.s,
			    AG_KEY_U, AG_KEYMOD_SHIFT,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_MOVE_UP);
			AG_MenuActionKb(mi, _("Move down"), agIconDown.s,
			    AG_KEY_D, AG_KEYMOD_SHIFT,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_MOVE_DOWN);
			
			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Free dataset"), NULL,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_FREE_DATASET);
			AG_MenuAction(mi, _("Destroy"), agIconTrash.s,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_DESTROY);
		}
	}

#ifdef AG_NETWORK
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
			    agIconLoad.s, UpdateFromRepo, "%p,%p", vfsRoot, tl);
			AG_MenuAction(mi, _("Delete from repository"),
			    agIconTrash.s, DeleteFromRepo, "%p", tl);
			AG_MenuAction(mi, _("Rename"), NULL,
			    RepoRenameDlg, "%p", tl);
		}

		btn = AG_ButtonNewFn(ntab, AG_BUTTON_HFILL,
		    _("Refresh revisions"),
		    PollRevisions, "%p", tl);
	
		if (agRcsMode)
			AG_PostEvent(NULL, btn, "button-pushed", NULL);
	}
#endif /* AG_NETWORK */
	
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
			AG_ObjectDetach(oent->win);

			AG_PostEvent(NULL, oent->obj, "edit-open", NULL);
			oent->win = obj->cls->edit(obj);
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
	agTerminating = 0;
	AG_QuitGUI();
}

static void
AbortQuit(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	agTerminating = 0;
	AG_ObjectDetach(win);
}


/*
 * Callback invoked when the user has requested termination of the
 * application. We check whether objects have been modified and not
 * saved and prompt the user if that's the case.
 */
static void
DEV_QuitCallback(AG_Event *event)
{
	AG_Object *vfsRoot = AG_PTR(1);
	AG_Window *win;
	AG_Box *bo;
	
	agTerminating = 1;

	if (!AG_ObjectChangedAll(vfsRoot)) {
		AG_QuitGUI();
		return;
	}
	
	if ((win = AG_WindowNewNamedS(AG_WINDOW_MODAL|AG_WINDOW_NOTITLE|
	    AG_WINDOW_NORESIZE, "DEV_QuitCallback")) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Exit application?"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);
	AG_LabelNewS(win, 0, _("Unsaved objects have been modified. "
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
DEV_BrowserInit(void *vfsRoot)
{
	TAILQ_INIT(&dobjs);
	TAILQ_INIT(&gobjs);

	AG_AddEvent(vfsRoot, "object-post-load-data", DEV_PostLoadDataCallback,
	    NULL);
	AG_AddEvent(vfsRoot, "object-page-out", DEV_PageOutCallback, NULL);
	AG_AddEvent(vfsRoot, "quit", DEV_QuitCallback, "%p", vfsRoot);
}

void
DEV_BrowserDestroy(void)
{
	struct objent *oent, *noent;

	for (oent = TAILQ_FIRST(&dobjs);
	     oent != TAILQ_END(&dobjs);
	     oent = noent) {
		noent = TAILQ_NEXT(oent, objs);
		Free(oent);
	}
	for (oent = TAILQ_FIRST(&gobjs);
	     oent != TAILQ_END(&gobjs);
	     oent = noent) {
		noent = TAILQ_NEXT(oent, objs);
		Free(oent);
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
