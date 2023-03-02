/*
 * Copyright (c) 2003-2023 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#if defined(AG_WIDGETS) && defined(AG_TIMERS)

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/box.h>
#include <agar/gui/button.h>
#include <agar/gui/textbox.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/tlist.h>
#include <agar/gui/menu.h>
#include <agar/gui/label.h>
#include <agar/gui/separator.h>
#include <agar/gui/file_dlg.h>
#include <agar/gui/notebook.h>
#include <agar/gui/icons.h>

#include <string.h>
#include <ctype.h>

/* Active object entry */
struct objent {
	AG_Object *_Nonnull obj;	/* Object being edited */
	AG_Window *_Nonnull win;	/* Edition window */
	TAILQ_ENTRY(objent) objs;
};
static TAILQ_HEAD_(objent) dobjs;
static TAILQ_HEAD_(objent) gobjs;
static int editNowFlag = 1;

static const struct dev_tool_ent {
	char *_Nonnull name;
	AG_Window *_Nullable (*_Nonnull fn)(void);
} devTools[] = {
	{ N_("Registered classes"),	AG_DEV_ClassInfo },
	{ N_("Loaded fonts"),		AG_DEV_FontInfo },
#ifdef AG_ENABLE_STRING
	{ N_("Timer Inspector"),	AG_DEV_TimerInspector },
#endif
#ifdef AG_UNICODE
	{ N_("Unicode Browser"),	AG_DEV_UnicodeBrowser }
#endif
};

static int agDevBrowserInited = 0;

static void AG_DEV_BrowserOpenData(void *_Nonnull);

static void
SelectTool(AG_Event *_Nonnull event)
{
	const struct dev_tool_ent *ent = AG_PTR(1);
	AG_Window *win;

	if ((win = (ent->fn)()) != NULL)
		AG_WindowShow(win);
}

static void
ToolMenu(AG_MenuItem *mi)
{
	const int devToolCount = sizeof(devTools) / sizeof(devTools[0]);
	int i;

	for (i = 0; i < devToolCount; i++)
		AG_MenuAction(mi, _(devTools[i].name), NULL,
		    SelectTool, "%p", &devTools[i]);
}

static void
CreateObject(AG_Event *_Nonnull event)
{
	char name[AG_OBJECT_NAME_MAX];
	AG_Object *vfsRoot = AG_OBJECT_PTR(1);
	AG_ObjectClass *cl = AG_PTR(2);
	AG_Textbox *name_tb = AG_TEXTBOX_PTR(3);
	AG_Tlist *tlObjs = AG_TLIST_PTR(4);
	AG_Window *dlg_win = AG_WINDOW_PTR(5);
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

	AG_PostEvent(nobj, "edit-create", NULL);
	
	if (editNowFlag && cl->edit != NULL)
		AG_DEV_BrowserOpenData(nobj);
}

enum {
	OBJEDIT_EDIT_DATA,
	OBJEDIT_EDIT_GENERIC,
	OBJEDIT_LOAD,
	OBJEDIT_SAVE,
	OBJEDIT_SAVE_ALL,		/* unused */
	OBJEDIT_EXPORT,
	OBJEDIT_RESET,
	OBJEDIT_DESTROY,
	OBJEDIT_MOVE_UP,
	OBJEDIT_MOVE_DOWN,
	OBJEDIT_DUP
};

static void
CloseGenericDlg(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	struct objent *oent = AG_PTR(1);

	AG_ObjectDetach(win);
	TAILQ_REMOVE(&gobjs, oent, objs);
	free(oent);
}

static void
BrowserOpenGeneric(AG_Object *ob)
{
	struct objent *oent;
	AG_Window *win;

	TAILQ_FOREACH(oent, &gobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		if (AG_OBJECT_VALID(oent->win) && AG_WINDOW_ISA(oent->win)) {
			AG_WindowShow(oent->win);
			AG_WindowFocus(oent->win);
			return;
		} else {
			TAILQ_REMOVE(&gobjs, oent, objs);
			free(oent);
		}
	}

	if ((win = AG_DEV_ObjectEdit(ob)) == NULL)
		return;

	oent = Malloc(sizeof(struct objent));
	oent->obj = ob;
	oent->win = win;
	TAILQ_INSERT_HEAD(&gobjs, oent, objs);
	AG_SetEvent(win, "window-close", CloseGenericDlg, "%p", oent);
	AG_WindowShow(win);
}

static void
AG_DEV_BrowserOpenData(void *p)
{
	AG_Object *ob = p;
	struct objent *oent;
	AG_Widget *editBox;
	AG_Window *win;
	int dataFound = 0;

	TAILQ_FOREACH(oent, &dobjs, objs) {
		if (oent->obj == ob)
			break;
	}
	if (oent != NULL) {
		if (AG_OBJECT_VALID(oent->win) && AG_WINDOW_ISA(oent->win)) {
			AG_WindowShow(oent->win);
			AG_WindowFocus(oent->win);
			return;
		} else {
			TAILQ_REMOVE(&gobjs, oent, objs);
			free(oent);
		}
	}
	
	if (ob->cls->edit == NULL)
		return;

	if (AG_ObjectLoadGenericFromFile(ob, NULL) == -1 ||
	    AG_ObjectLoadDataFromFile(ob, &dataFound, NULL) == -1) {
		if (!dataFound) {
			if (AG_ObjectSaveAll(ob) == -1) {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
				return;
			}
		} else {
			AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name, AG_GetError());
			return;
		}
	}
	AG_PostEvent(ob, "edit-post-load", NULL);

	if ((editBox = ob->cls->edit(ob)) == NULL) {
		return;
	}
	if (AG_WINDOW_ISA(editBox)) {
		win = (AG_Window *)editBox;
	} else {
		if ((win = AG_WindowNew(0)) == NULL) {
			return;
		}
		AG_WindowSetCaption(win, _("%s - Editor"), ob->name);
		AG_ObjectAttach(win, editBox);
	}

	AG_PostEvent(ob, "edit-open", NULL);

	oent = Malloc(sizeof(struct objent));
	oent->obj = ob;
	oent->win = win;
	TAILQ_INSERT_HEAD(&dobjs, oent, objs);
/*	AG_SetEvent(win, "window-close", SaveChangesDlg, "%p", oent); */
	AG_WindowShow(win);
}

static void
SaveObjectToFile(AG_Event *_Nonnull event)
{
	AG_Object *ob = AG_OBJECT_PTR(1);
	const char *path = AG_STRING(2);

	Verbose("Saving <%s> %s to %s...", OBJECT_CLASS(ob)->name,
	    OBJECT(ob)->name, (path[0] != '\0') ? path : "VFS");

	if (path[0] != '\0') {                                   /* Save as */
		if (AG_ObjectSaveToFile(ob, path) == -1) {
			AG_SetError("%s: %s", path, AG_GetError());
			goto fail;
		}
	} else {                                                    /* Save */
		if (AG_ObjectSaveAll(ob) == -1) {
			AG_SetError("%s: %s", ob->name, AG_GetError());
			goto fail;
		}
	}

	Verbose("OK\n");
	return;
fail:
	AG_TextMsgFromError();
}

static AG_Window *
BrowserSaveToDlg(void *p, const char *name)
{
	char ext[AG_OBJECT_HIER_MAX+3];
	AG_Object *ob = p;
	AG_Window *win;
	AG_FileDlg *fd;

	ext[0] = '*';
	ext[1] = '.';
	Strlcpy(&ext[2], ob->cls->name, sizeof(ext)-2);

	if ((win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Save %s to..."), ob->name);
	fd = AG_FileDlgNewMRU(win, "object-import",
	                      AG_FILEDLG_CLOSEWIN | AG_FILEDLG_SAVE |
	                      AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, name, ext, SaveObjectToFile,"%p",ob);
	AG_FileDlgSetFilename(fd, "%s.%s", ob->name, ob->cls->name);
	AG_WindowShow(win);
	return (win);
}

static void
ObjectOp(AG_Event *_Nonnull event)
{
	AG_Object *vfsRoot = AG_OBJECT_PTR(1);
	AG_Tlist *tl = AG_TLIST_PTR(2);
	AG_Window *win, *winParent = AG_ParentWindow(tl);
	int op = AG_INT(3);
	AG_TlistItem *it;

	TAILQ_FOREACH(it, &tl->items, items) {
		AG_Object *ob = it->p1;

		if (!it->selected)
			continue;

		switch (op) {
		case OBJEDIT_EDIT_DATA:
			if (ob->cls->edit != NULL) {
				Verbose("Invoking <%s> %s -> edit\n",
				    OBJECT_CLASS(ob)->name,
				    OBJECT(ob)->name);
				AG_DEV_BrowserOpenData(ob);
			} else {
				AG_TextTmsg(AG_MSG_ERROR, 750,
				    _("Object `%s' has no edit operation."),
				    ob->name);
			}
			break;
		case OBJEDIT_EDIT_GENERIC:
			BrowserOpenGeneric(ob);
			break;
		case OBJEDIT_LOAD:
			Verbose("Invoking <%s> %s -> load\n",
			    OBJECT_CLASS(ob)->name,
			    OBJECT(ob)->name);
			if (AG_ObjectLoad(ob) == -1) {
				AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name,
				    AG_GetError());
			} else {
				AG_PostEvent(ob, "edit-post-load", NULL);
			}
			break;
		case OBJEDIT_SAVE:
			{
				AG_Event ev;

				AG_EventInit(&ev);
				AG_EventArgs(&ev, "%p,%s", ob, "");
				SaveObjectToFile(&ev);
			}
			break;
		case OBJEDIT_EXPORT:
			win = BrowserSaveToDlg(ob, _("Agar object file"));
			if (win != NULL) {
				AG_WindowAttach(winParent, win);
			}
			break;
		case OBJEDIT_DUP:
			break;
		case OBJEDIT_MOVE_UP:
			AG_ObjectMoveUp(ob);
			break;
		case OBJEDIT_MOVE_DOWN:
			AG_ObjectMoveDown(ob);
			break;
		case OBJEDIT_RESET:
			if (it->p1 == vfsRoot) {
				continue;
			}
			AG_ObjectReset(ob);
			break;
		case OBJEDIT_DESTROY:
			if (it->p1 == vfsRoot) {
				continue;
			}
			if (ob->flags & AG_OBJECT_INDESTRUCTIBLE) {
				AG_TextMsg(AG_MSG_ERROR,
				    _("The `%s' object is indestructible."),
				    ob->name);
				continue;
			}
			Verbose("Destroying <%s> %s", OBJECT_CLASS(ob)->name,
			    OBJECT(ob)->name);

			AG_ObjectDetach(ob);
			AG_ObjectUnlinkDatafiles(ob);
			AG_ObjectDestroy(ob);
			break;
		}
	}
}

static AG_TlistItem *_Nonnull
PollObjectsFind(AG_Tlist *_Nonnull tl, AG_Object *_Nonnull pob, int depth)
{
	char label[AG_TLIST_LABEL_MAX];
	AG_Object *cob;
	AG_TlistItem *it;

	Strlcpy(label, pob->name, sizeof(label));

	it = AG_TlistAddPtr(tl, NULL, label, pob);
	it->depth = depth;
	it->cat = "object";

	if (!TAILQ_EMPTY(&pob->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
		if (AG_ObjectRoot(pob) == pob)
			it->flags |= AG_TLIST_ITEM_EXPANDED;
	}
	if (AG_TlistVisibleChildren(tl, it)) {
		TAILQ_FOREACH(cob, &pob->children, cobjs)
			PollObjectsFind(tl, cob, depth+1);
	}
	return (it);
}

static void
PollObjects(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Object *pob = AG_OBJECT_PTR(1);

	AG_LockVFS(pob);
	AG_TlistBegin(tl);

	PollObjectsFind(tl, pob, 0);

	AG_TlistEnd(tl);
	AG_UnlockVFS(pob);
}

static void
LoadObject(AG_Event *_Nonnull event)
{
	AG_Object *o = AG_OBJECT_PTR(1);

	if (AG_ObjectLoad(o) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", OBJECT(o)->name,
		    AG_GetError());
	}
}

static void
SaveObject(AG_Event *_Nonnull event)
{
	AG_Object *ob = AG_OBJECT_PTR(1);

	if (AG_ObjectSaveAll(ob) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", ob->name, AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Object `%s' saved successfully."), ob->name);
	}
}

static void
ExitProgram(AG_Event *_Nonnull event)
{
	AG_Quit();
}

static void
ShowPreferences(AG_Event *_Nonnull event)
{
	AG_DEV_ConfigShow();
}

static void
CreateObjectDlg(AG_Event *_Nonnull event)
{
	AG_Window *win;
	AG_Object *vfsRoot = AG_OBJECT_PTR(1);
	AG_ObjectClass *cl = AG_PTR(2);
	AG_Window *winParent = AG_WINDOW_PTR(3);
	AG_Tlist *tlParents;
	AG_Box *bo;
	AG_Textbox *tb;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("New %s object"), cl->name);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		AG_LabelNew(bo, 0, _("Class: %s"), cl->hier);
		tb = AG_TextboxNew(bo, AG_TEXTBOX_HFILL, _("Name: "));
		AG_WidgetFocus(tb);
	}

	AG_SeparatorNew(win, AG_SEPARATOR_HORIZ);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL | AG_BOX_VFILL);
	AG_SetPadding(bo, "0");
	AG_SetSpacing(bo, "0");
	{
		AG_LabelNewS(bo, 0, _("Parent object:"));

		tlParents = AG_TlistNewPolled(bo, AG_TLIST_EXPAND,
		    PollObjects, "%p", vfsRoot);
		AG_TlistSizeHint(tlParents, "XXXXXXXXXXXXXXXXXXX", 5);
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

	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
GenNewObjectMenu(AG_MenuItem *_Nonnull mParent, AG_ObjectClass *_Nonnull cls,
    AG_Object *_Nonnull vfsRoot, AG_Window *_Nonnull winParent)
{
	AG_ObjectClass *Csub;
	AG_MenuItem *mNode;

	mNode = AG_MenuNode(mParent, cls->name, NULL);

	AG_MenuAction(mNode, _("Create instance..."), NULL,
	    CreateObjectDlg, "%p,%p,%p", vfsRoot, cls, winParent);

	if (!TAILQ_EMPTY(&cls->sub)) {
		AG_MenuSeparator(mNode);
		TAILQ_FOREACH(Csub, &cls->sub, subclasses)
			GenNewObjectMenu(mNode, Csub, vfsRoot, winParent);
	}
}

static void
AG_DEV_ConfirmQuit(AG_Event *_Nonnull event)
{
	AG_QuitGUI();
}

static void
AG_DEV_AbortQuit(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_PTR(1);

	AG_ObjectDetach(win);
}

/*
 * Callback invoked when the user has requested termination of the
 * application. We check whether objects have been modified and not
 * saved and prompt the user if that's the case.
 */
static void
AG_DEV_QuitCallback(AG_Event *_Nonnull event)
{
	AG_Object *vfsRoot = AG_OBJECT_PTR(1);
	AG_Window *win;
	AG_Box *box;
	
	if (!AG_ObjectChangedAll(vfsRoot)) {
		AG_QuitGUI();
		return;
	}
	
	if ((win = AG_WindowNewNamedS(AG_WINDOW_MODAL | AG_WINDOW_NOTITLE |
	                              AG_WINDOW_NORESIZE, "DEV_quitCallback")) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Exit application?"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_SetSpacing(win, "8");

	AG_LabelNewS(win, 0, _("Unsaved objects have been modified. "
	                       "Exit application?"));

	box = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	{
		AG_SetPadding(box, "0");
		AG_SetSpacing(box, "0");

		AG_ButtonNewFn(box, 0, _("Quit"), AG_DEV_ConfirmQuit, NULL);

		AG_WidgetFocus(
		    AG_ButtonNewFn(box, 0, _("Cancel"),
		        AG_DEV_AbortQuit,"%p",win)
		);
	}

	AG_WindowShow(win);
}


/* Create the object browser window. */
AG_Window *
AG_DEV_Browser(void *vfsRoot)
{
	AG_Window *win;
	AG_Tlist *tlObjs;
	AG_Menu *menu;
	AG_MenuItem *mi;

	if (!agDevBrowserInited) {
		TAILQ_INIT(&dobjs);
		TAILQ_INIT(&gobjs);
		agDevBrowserInited = 1;
	}

	AG_AddEvent(vfsRoot, "quit", AG_DEV_QuitCallback, "%p", vfsRoot);

	if ((win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, OBJECT(vfsRoot)->name);
	AG_WindowSetPosition(win, AG_WINDOW_ML, 0);
	AG_SetFontSize(win, "120%");

	menu = AG_MenuNew(win, AG_MENU_HFILL);

	AG_LabelNew(win, 0, _("%s VFS"), OBJECT(vfsRoot)->name);

	tlObjs = AG_TlistNew(NULL, AG_TLIST_POLL | AG_TLIST_MULTI |
	                           AG_TLIST_EXPAND);
	AG_TlistSetRefresh(tlObjs, 120);
	AG_TlistSizeHint(tlObjs, "XXXXXXXXXXXXXXXXXXXXXXXX", 10);
	AG_SetEvent(tlObjs, "tlist-poll", PollObjects, "%p", vfsRoot);
	AG_SetEvent(tlObjs, "tlist-dblclick", ObjectOp, "%p,%p,%i", vfsRoot,
	    tlObjs, OBJEDIT_EDIT_DATA);

	mi = AG_MenuNode(menu->root, _("File"), NULL);
	{
		AG_MenuItem *miObjs;

		miObjs = AG_MenuNode(mi, _("New object"), NULL);
		GenNewObjectMenu(miObjs, &agObjectClass, vfsRoot, win);

		AG_MenuSeparator(mi);

		AG_MenuAction(mi, _("Load VFS"), agIconLoad.s,
		    LoadObject, "%p", vfsRoot);
		AG_MenuAction(mi, _("Save VFS"), agIconSave.s,
		    SaveObject, "%p", vfsRoot);
		
		AG_MenuSeparator(mi);
		
		AG_MenuAction(mi, _("Exit"), NULL, ExitProgram, NULL);
	}
	
	mi = AG_MenuNode(menu->root, _("Edit"), NULL);
	{
		AG_MenuAction(mi, _("Edit object data..."), NULL,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
		    OBJEDIT_EDIT_DATA);
		AG_MenuAction(mi, _("Edit generic information..."), NULL,
		    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
		    OBJEDIT_EDIT_GENERIC);
		
		AG_MenuSeparator(mi);
			
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

	mi = AG_MenuNode(menu->root, _("Tools"), NULL);
	ToolMenu(mi);

	{
		AG_MenuItem *mi;

		AG_ObjectAttach(win, tlObjs);

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

			AG_MenuSeparator(mi);
			
			AG_MenuActionKb(mi, _("Move up"), agIconUp.s,
			    AG_KEY_U, AG_KEYMOD_SHIFT,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_MOVE_UP);
			AG_MenuActionKb(mi, _("Move down"), agIconDown.s,
			    AG_KEY_D, AG_KEYMOD_SHIFT,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_MOVE_DOWN);
			
			AG_MenuSeparator(mi);
			
			AG_MenuAction(mi, _("Reset"), NULL,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_RESET);
			AG_MenuAction(mi, _("Destroy"), agIconTrash.s,
			    ObjectOp, "%p,%p,%i", vfsRoot, tlObjs,
			    OBJEDIT_DESTROY);
		}
	}

	AG_WindowShow(win);
	return (win);
}

#endif /* AG_WIDGETS and AG_TIMERS */
