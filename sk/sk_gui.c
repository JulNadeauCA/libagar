/*
 * Copyright (c) 2010-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Graphical user interface.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>

#include "sk.h"
#include "sk_gui.h"

#include <string.h>

AG_Object skVfsRoot;			/* General-purpose VFS */
AG_Mutex  skObjLock;

static int nEditorWindows = 0;

/* Object classes users can create directly. */
const char *skEditableClasses[] = {
	"SK:*",
	NULL
};

/*
 * Display "Save changes?" dialog on exit.
 */
static void
CloseObject(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_PTR(1);
	AG_Object *obj = AG_PTR(2);
	int save = AG_INT(3);

	if (save) {
		if (AG_ObjectSave(obj) == -1) {
			AG_TextMsgFromError();	/* TODO suggest "save as" */
			return;
		}
	}
	AG_ObjectDetach(win);
	AG_ObjectDelete(obj);

	if (--nEditorWindows == 0)
		AG_Terminate(0);
}
static void
WindowClose(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_SELF();
	AG_Object *obj = AG_PTR(1);
	AG_Event ev;
	AG_Button *bOpts[3];
	AG_Window *wDlg;

	if (!AG_ObjectChanged(obj)) {
		AG_EventArgs(&ev, "%p,%p,%i", win, obj, 0);
		CloseObject(&ev);
		return;
	}
	wDlg = AG_TextPromptOptions(bOpts, 3, _("Save changes to %s?"), OBJECT(obj)->name);
	AG_WindowAttach(win, wDlg);
	AG_ButtonText(bOpts[0], _("Save"));
	AG_SetEvent(bOpts[0], "button-pushed", CloseObject, "%p,%p,%i", win, obj, 1);
	AG_WidgetFocus(bOpts[0]);
	AG_ButtonText(bOpts[1], _("Discard"));
	AG_SetEvent(bOpts[1], "button-pushed", CloseObject, "%p,%p,%i", win, obj, 0);
	AG_ButtonText(bOpts[2], _("Cancel"));
	AG_SetEvent(bOpts[2], "button-pushed", AGWINDETACH(wDlg));
}

/* Open a sketch for edition. */
AG_Window *
SK_GUI_OpenObject(void *p)
{
	AG_Object *obj = p;
	AG_Window *win = NULL;
	AG_Widget *wEdit;

	/* Invoke edit(), which may return a Window or some other Widget. */
	Verbose("Opening %s (%s)\n", obj->name, obj->cls->name);
	if ((wEdit = obj->cls->edit(obj)) == NULL) {
		AG_SetError("%s no edit()", obj->cls->name);
		return (NULL);
	}
	if (AG_OfClass(wEdit, "AG_Widget:AG_Window:*")) {
		win = (AG_Window *)wEdit;
	} else if (AG_OfClass(wEdit, "AG_Widget:*")) {
		if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
			return (NULL);
		}
		AG_ObjectAttach(win, wEdit);
	} else {
		AG_SetError("%s: edit() illegal object", obj->cls->name);
		return (NULL);
	}

	win->flags |= AG_WINDOW_MAIN;

	AG_WindowSetCaptionS(win, AG_Defined(obj,"archive-path") ?
	    AG_ShortFilename(AG_GetStringP(obj,"archive-path")) :
	    obj->name);

	AG_SetEvent(win, "window-close", WindowClose, "%p", obj);
	AG_SetPointer(win, "object", obj);
	AG_PostEvent(obj, "edit-open", NULL);

	nEditorWindows++;
	AG_WindowShow(win);
	return (win);
}

/* Create a new sketch instance. */
void
SK_GUI_NewObject(AG_Event *event)
{
	AG_ObjectClass *cl = AG_PTR(1);
	AG_Object *obj;

	if (cl == &skClass) {
		obj = (AG_Object *)SK_New(&skVfsRoot, NULL);
	} else {
		obj = AG_ObjectNew(&skVfsRoot, NULL, cl);
	}
	if (obj == NULL ||
	    SK_GUI_OpenObject(obj) == NULL) {
		goto fail;
	}
	return;
fail:
	AG_TextError(_("Failed to create object: %s"), AG_GetError());
	if (obj != NULL) { AG_ObjectDelete(obj); }
}

/* Load an native SK sketch file. */
void
SK_GUI_LoadObject(AG_Event *event)
{
	AG_ObjectClass *cl = AG_PTR(1);
	char *path = AG_STRING(2);
	AG_Object *obj;

	if ((obj = AG_ObjectNew(&skVfsRoot, NULL, cl)) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	if (AG_ObjectLoadFromFile(obj, path) == -1) {
		goto fail;
	}
	AG_SetString(obj, "archive-path", path);
	AG_ObjectSetNameS(obj, AG_ShortFilename(path));

	if (SK_GUI_OpenObject(obj) == NULL) {
		goto fail;
	}
	return;
fail:
	AG_TextMsgFromError();
	AG_ObjectDestroy(obj);
}

/* "Open..." dialog */
void
SK_GUI_OpenDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fd;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Open..."));

	fd = AG_FileDlgNewMRU(win, "sk-objs",
	    AG_FILEDLG_LOAD | AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND);

	AG_FileDlgSetOptionContainer(fd, AG_BoxNewVert(win, AG_BOX_HFILL));

	AG_FileDlgAddType(fd, _("Agar sketch file"), "*.sk",
	    SK_GUI_LoadObject, "%p", &skClass);

	AG_WindowShow(win);
}

/* Save an object file in native .sk format. */
static void
SaveNativeObject(AG_Event *_Nonnull event)
{
	AG_Object *obj = AG_PTR(1);
	char *path = AG_STRING(2);
	AG_Window *wEdit;

	if (AG_ObjectSaveToFile(obj, path) == -1) {
		AG_TextMsgFromError();
		return;
	}
	AG_SetString(obj, "archive-path", path);
	AG_ObjectSetNameS(obj, AG_ShortFilename(path));

	if ((wEdit = AG_WindowFindFocused()) != NULL)
		AG_WindowSetCaptionS(wEdit, AG_ShortFilename(path));
}

/* "Save as..." dialog. */
void
SK_GUI_SaveAsDlg(AG_Event *event)
{
	char defDir[AG_PATHNAME_MAX];
	AG_Object *obj = AG_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;

	if (obj == NULL) {
		AG_TextError(_("No document is selected for saving."));
		return;
	}
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Save %s as..."), obj->name);

	fd = AG_FileDlgNew(win, AG_FILEDLG_SAVE | AG_FILEDLG_CLOSEWIN |
	                        AG_FILEDLG_EXPAND);
	AG_FileDlgSetOptionContainer(fd, AG_BoxNewVert(win, AG_BOX_HFILL));

	AG_ConfigGetPath(AG_CONFIG_PATH_DATA, 0, defDir, sizeof(defDir));
	AG_FileDlgSetDirectoryMRU(fd, "agar-sk.mru.files", defDir);

	if (AG_OfClass(obj, "SK:*")) {
		AG_FileDlgAddType(fd, _("Agar sketch file"), "*.sk",
		    SaveNativeObject, "%p", obj);
	}
	AG_WindowShow(win);
}

/* "Save" action */
void
SK_GUI_Save(AG_Event *event)
{
	AG_Object *obj = AG_PTR(1);
	
	if (obj == NULL) {
		AG_TextError(_("No document is selected for saving."));
		return;
	}
	if (!AG_Defined(obj, "archive-path")) {
		SK_GUI_SaveAsDlg(event);
		return;
	}
	if (AG_ObjectSave(obj) == -1) {
		AG_TextError(_("Error saving object: %s"), AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1250, _("Saved %s successfully"),
		    AG_GetStringP(obj, "archive-path"));
	}
}

/* Undo last action. */
void
SK_GUI_Undo(AG_Event *event)
{
	/* TODO */
}

/* Redo last undone action. */
void
SK_GUI_Redo(AG_Event *event)
{
	/* TODO */
}

/* Standard Edit / Preferences dialog. */
void
SK_GUI_EditPreferences(AG_Event *event)
{
}

static void
SelectedFont(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_PTR(1);

	AG_SetString(agConfig, "font.face", OBJECT(agDefaultFont)->name);
	AG_SetInt(agConfig, "font.size", agDefaultFont->spec.size);
	AG_SetUint(agConfig, "font.flags", agDefaultFont->flags);
	(void)AG_ConfigSave();

	AG_TextWarning("default-font-changed",
	    _("The default font has been changed.\n"
	      "Please restart application for this change to take effect."));
	AG_ObjectDetach(win);
}

/* "Select font" dialog */
void
SK_GUI_SelectFontDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FontSelector *fs;
	AG_Box *hBox;

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, _("Font selection"));

	fs = AG_FontSelectorNew(win, AG_FONTSELECTOR_EXPAND);
	AG_BindPointer(fs, "font", (void *)&agDefaultFont);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	AG_ButtonNewFn(hBox, 0, _("OK"), SelectedFont, "%p", win);
	AG_ButtonNewFn(hBox, 0, _("Cancel"), AGWINCLOSE(win));
	AG_WindowShow(win);
}

/* Build a generic "File" menu. */
void
SK_FileMenu(AG_MenuItem *m, void *obj)
{
	AG_MenuActionKb(m, _("New sketch..."), agIconDoc.s,
	    AG_KEY_N, AG_KEYMOD_CTRL,
	    SK_GUI_NewObject, "%p", &skClass);
	
	AG_MenuSeparator(m);

	AG_MenuActionKb(m, _("Open..."), agIconLoad.s,
	    AG_KEY_O, AG_KEYMOD_CTRL,
	    SK_GUI_OpenDlg, NULL);
	AG_MenuActionKb(m, _("Save"), agIconSave.s,
	    AG_KEY_S, AG_KEYMOD_CTRL,
	    SK_GUI_Save, "%p", obj);
	AG_MenuAction(m, _("Save as..."), agIconSave.s,
	    SK_GUI_SaveAsDlg, "%p", obj);
}

/* Build a generic "Edit" menu. */
void
SK_EditMenu(AG_MenuItem *m, void *obj)
{
	AG_MenuActionKb(m, _("Undo"), agIconUp.s, AG_KEY_Z, AG_KEYMOD_CTRL,
	    SK_GUI_Undo, "%p", obj);
	AG_MenuActionKb(m, _("Redo"), agIconDown.s, AG_KEY_R, AG_KEYMOD_CTRL,
	    SK_GUI_Redo, "%p", obj);
	
	AG_MenuSeparator(m);

	AG_MenuAction(m, _("Select font..."), agIconMagnifier.s,
	    SK_GUI_SelectFontDlg, NULL);
}

/* Initialize Agar-SK GUI globals. */
void
SK_InitGUI(void)
{
	if (agGUI) {
		AG_RegisterClass(&skViewClass);
	}
	AG_ObjectInit(&skVfsRoot, NULL);
	skVfsRoot.flags |= AG_OBJECT_STATIC;
	AG_ObjectSetName(&skVfsRoot, "Agar-SK VFS");
	AG_MutexInitRecursive(&skObjLock);
}

/* Release Agar-SK GUI globals. */
void
SK_DestroyGUI(void)
{
	if (agGUI) {
		AG_UnregisterClass(&skViewClass);
	}
	AG_MutexDestroy(&skObjLock);
	AG_ObjectDestroy(&skVfsRoot);
}
