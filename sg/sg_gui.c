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
#include <agar/sg/sg.h>
#include <agar/sg/sg_gui.h>
#include <agar/sg/icons_data.h>

#include <string.h>

AG_Object sgVfsRoot;			/* General-purpose VFS */
static AG_WindowQ sgEditorWindows;	/* Editor windows */

/*
 * Object classes users can create and edit independently of a specific
 * scene and scene view.
 */
const char *sgEditableClasses[] = {
	"SG:*",
	"SG_Script:*",
	"SG_Texture:*",
	"SG_Palette:*",
	NULL
};

/*
 * Display "Save changes?" dialog on exit.
 */
static void
CloseObject(AG_Event *_Nonnull event)
{
	AG_Window *wMain = AG_WINDOW_PTR(1);
	AG_Object *obj = AG_OBJECT_PTR(2);
	const int save = AG_INT(3);
	AG_Event ev;

	if (save) {
		if (!AG_Defined(obj, "archive-path")) {
			AG_EventArgs(&ev, "%p,%p,%i", wMain, obj, 1);
			SG_GUI_SaveAsDlg(&ev);
			return;
		}
		if (AG_ObjectSave(obj) == -1) {
			AG_TextMsgFromError();	/* TODO suggest "save as" */
			return;
		}
	}
	TAILQ_REMOVE(&sgEditorWindows, wMain, user);
	AG_ObjectDetach(wMain);
//	AG_ObjectDelete(obj);

	if (TAILQ_EMPTY(&sgEditorWindows))
		AG_Terminate(0);
}
static void
WindowClose(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_Object *obj = AG_OBJECT_PTR(1);
	AG_Event ev;
	AG_Button *bOpts[3];
	AG_Window *wDlg, *wOther;

	TAILQ_FOREACH(wOther, &sgEditorWindows, user) {
		if (wOther != win &&
		    AG_GetPointer(wOther, "object") == obj)
			break;
	}
	if (wOther == NULL &&		/* Last editor window for this object */
	    AG_ObjectChanged(obj)) {
		wDlg = AG_TextPromptOptions(bOpts, 3,
		    _("Save changes to %s?"), OBJECT(obj)->name);
		AG_WindowAttach(win, wDlg);

		AG_ButtonText(bOpts[0], _("Save"));
		AG_SetEvent(bOpts[0], "button-pushed",
		    CloseObject, "%p,%p,%i", win, obj, 1);
		AG_WidgetFocus(bOpts[0]);

		AG_ButtonText(bOpts[1], _("Discard"));
		AG_SetEvent(bOpts[1], "button-pushed",
		    CloseObject, "%p,%p,%i", win, obj, 0);

		AG_ButtonText(bOpts[2], _("Cancel"));
		AG_SetEvent(bOpts[2], "button-pushed", AGWINDETACH(wDlg));
	} else {
		AG_EventArgs(&ev, "%p,%p,%i", win, obj, 0);
		CloseObject(&ev);
	}
}

/* Open a Agar-SG object for edition. */
AG_Window *
SG_GUI_OpenObject(void *p)
{
	AG_Object *obj = p;
	AG_Window *win = NULL;
	AG_Widget *wEdit;

	/* Invoke edit(), which may return a Window or some other Widget. */
	if ((wEdit = obj->cls->edit(obj)) == NULL) {
		AG_SetError("%s no edit()", obj->cls->name);
		return (NULL);
	}
	if (AG_OfClass(wEdit, "AG_Widget:AG_Window:*")) {
		Debug(NULL, "SG_GUI: %s->edit() returned Window %s\n", obj->name,
		    wEdit ? OBJECT(wEdit)->name : "NULL");
		win = (AG_Window *)wEdit;
	} else if (AG_OfClass(wEdit, "AG_Widget:*")) {
		Debug(NULL, "SG_GUI: %s->edit() returned Widget %s\n", obj->name,
		    wEdit ? OBJECT(wEdit)->name : "NULL");
		if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
			return (NULL);
		}
		AG_ObjectAttach(win, wEdit);
	} else {
		AG_SetError("%s: edit() illegal object", obj->cls->name);
		return (NULL);
	}
	AG_WindowSetCaptionS(win,
	    AG_Defined(obj,"archive-path") ?
	    AG_GetStringP(obj,"archive-path") : obj->name);

	AG_SetEvent(win, "window-close", WindowClose, "%p", obj);
	AG_SetPointer(win, "object", obj);
	AG_PostEvent(obj, "edit-open", NULL);

	TAILQ_INSERT_TAIL(&sgEditorWindows, win, user);
	AG_WindowShow(win);
	return (win);
}

/* Create a new instance of a native Agar-SG object class. */
void
SG_GUI_NewObject(AG_Event *event)
{
	AG_ObjectClass *cl = AG_PTR(1);
	AG_Object *obj;

	if (cl == &sgClass) {
		obj = (AG_Object *)SG_New(&sgVfsRoot, NULL, 0);
	} else {
		obj = AG_ObjectNew(&sgVfsRoot, NULL, cl);
	}
	if (obj == NULL ||
	    SG_GUI_OpenObject(obj) == NULL) {
		goto fail;
	}
	return;
fail:
	AG_TextError(_("Failed to create object: %s"), AG_GetError());
	if (obj != NULL) { AG_ObjectDelete(obj); }
}

/* Load a native Agar-SG object file. */
AG_Object *
SG_GUI_LoadObject(AG_ObjectClass *cls, const char *path)
{
	AG_Object *obj;

	if ((obj = AG_ObjectNew(&sgVfsRoot, NULL, cls)) == NULL) {
		return (NULL);
	}
	if (AG_ObjectLoadFromFile(obj, path) == -1) {
		AG_SetError("%s: %s", AG_ShortFilename(path), AG_GetError());
		goto fail;
	}
	AG_SetString(obj, "archive-path", path);
	AG_ObjectSetNameS(obj, AG_ShortFilename(path));
	if (SG_GUI_OpenObject(obj) == NULL) {
		goto fail;
	}
	return (obj);
fail:
	AG_ObjectDelete(obj);
	return (NULL);
}

/* Load a native Agar-SG object file. */
static void
LoadObject(AG_Event *_Nonnull event)
{
	AG_ObjectClass *cls = AG_PTR(1);
	const char *path = AG_STRING(2);

	if (SG_GUI_LoadObject(cls, path) == NULL)
		AG_TextMsgFromError();
}

/* "Open..." dialog */
void
SG_GUI_OpenDlg(AG_Event *event)
{
	AG_Window *wMain = AG_WINDOW_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;
	int j;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Open..."));

	fd = AG_FileDlgNewMRU(win, "sg-objs", AG_FILEDLG_LOAD |
	                                      AG_FILEDLG_CLOSEWIN |
	                                      AG_FILEDLG_EXPAND);

	AG_FileDlgSetOptionContainer(fd, AG_BoxNewVert(win, AG_BOX_HFILL));
	
	for (j = 0; j < agFileExtCount; j++) {
		const AG_FileExtMapping *me = &agFileExtMap[j];
		char lbl[64], ext[16];

		Snprintf(lbl, sizeof(lbl), _("Agar-SG %s file"), me->descr);
		Strlcpy(ext, "*", sizeof(ext));
		Strlcat(ext, me->ext, sizeof(ext));
		AG_FileDlgAddType(fd, lbl, ext, LoadObject, "%p", me->cls);
	}
	AG_WindowAttach(wMain, win);
	AG_WindowShow(win);
}

/* Save an object file in native Agar-SG format. */
static void
SaveObject(AG_Event *_Nonnull event)
{
	AG_Window *wMain = AG_WINDOW_PTR(1);
	AG_Object *obj = AG_OBJECT_PTR(2);
	const int saveAndClose = AG_INT(3);
	const char *path = AG_STRING(4);
	const char *pathShort;
	AG_Event ev;

	pathShort = AG_ShortFilename(path);

	if (AG_ObjectSaveToFile(obj, path) == -1) {
		AG_TextError("%s: %s", pathShort, AG_GetError());
		return;
	}
	if (saveAndClose) {
		AG_EventArgs(&ev, "%p,%p,%i", wMain, obj, 0);
		CloseObject(&ev);
	} else {
		AG_SetString(obj, "archive-path", path);
		AG_ObjectSetNameS(obj, pathShort);
		AG_WindowSetCaptionS(wMain, pathShort);
	}
	AG_TextTmsg(AG_MSG_INFO, 2000, _("Object saved to %s"), pathShort);
}

/* "Save as..." dialog. */
void
SG_GUI_SaveAsDlg(AG_Event *event)
{
	AG_Window *wMain = AG_WINDOW_PTR(1);
	AG_Object *obj = AG_OBJECT_PTR(2);
	int saveAndClose = AG_INT(3);
	char defDir[AG_PATHNAME_MAX];
	AG_Window *win;
	AG_FileDlg *fd;
	int j;

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
	AG_FileDlgSetDirectoryMRU(fd, "agar-sg.mru.files", defDir);
	
	for (j = 0; j < agFileExtCount; j++) {
		const AG_FileExtMapping *me = &agFileExtMap[j];
		AG_ObjectClass *cls = (AG_ObjectClass *)me->cls;
		char lbl[64], ext[16];

		if (AG_OfClass(obj, cls->hier)) {
			Snprintf(lbl, sizeof(lbl), _("Agar-SG %s file"),
			    me->descr);
			Strlcpy(ext, "*", sizeof(ext));
			Strlcat(ext, me->ext, sizeof(ext));

			AG_FileDlgAddType(fd, lbl, ext, 
			    SaveObject, "%p,%p,%i", wMain, obj, saveAndClose);
		}
	}
	AG_WindowAttach(wMain, win);
	AG_WindowShow(win);
}

/* "Save" action */
void
SG_GUI_Save(AG_Event *event)
{
	AG_Window *wMain = AG_WINDOW_PTR(1);
	AG_Object *obj = AG_OBJECT_PTR(2);
	const int saveAndClose = AG_INT(3);
	char *archivePath;
	AG_Event ev;
	
	if (obj == NULL) {
		AG_TextError(_("No document is selected for saving."));
		return;
	}
	if (!AG_Defined(obj,"archive-path") ||
	    (archivePath = AG_GetStringP(obj,"archive-path")) == NULL ||
	    archivePath[0] == '\0') {
		AG_EventArgs(&ev, "%p,%p,%i", wMain, obj, saveAndClose);
		SG_GUI_SaveAsDlg(&ev);
		return;
	}
	if (AG_ObjectSave(obj) == -1) {
		AG_TextError(_("Error saving object: %s"), AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1250, _("Saved %s to %s"),
		    obj->name, archivePath);
	}
}

/* Undo last action. */
void
SG_GUI_Undo(AG_Event *event)
{
	/* TODO */
}

/* Redo last undone action. */
void
SG_GUI_Redo(AG_Event *event)
{
	/* TODO */
}

/* Standard Edit / Preferences dialog. */
void
SG_GUI_EditPreferences(AG_Event *event)
{
}

static void
SelectedFont(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_PTR(1);

	AG_SetString(agConfig, "font.face",  OBJECT(agDefaultFont)->name);
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
SG_GUI_SelectFontDlg(AG_Event *event)
{
	AG_Window *wMain = AG_WINDOW_PTR(1);
	AG_Window *win;
	AG_FontSelector *fs;
	AG_Box *hBox;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Font selection"));

	fs = AG_FontSelectorNew(win, AG_FONTSELECTOR_EXPAND);
	AG_BindPointer(fs, "font", (void *)&agDefaultFont);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
	AG_ButtonNewFn(hBox, 0, _("OK"), SelectedFont, "%p", win);
	AG_ButtonNewFn(hBox, 0, _("Cancel"), AGWINCLOSE(win));
	
	AG_SeparatorNewHoriz(win);

#ifdef __APPLE__
	AG_LabelNewS(win, 0,
	    _("Note: Use [Command] and [-] or [=] to zoom current window."));
#else
	AG_LabelNewS(win, 0,
	    _("Note: Use [Ctrl] and [-] or [=] to zoom current window."));
#endif

	AG_WindowAttach(wMain, win);
	AG_WindowShow(win);
}

/* "Create new" dialog (used by sgedit(1) when called without arguments) */
void
SG_GUI_CreateNewDlg(AG_Event *event)
{
	AG_Window *win;
	const AG_FileExtMapping *me;
	AG_Label *lbl;
	int i;

	if ((win = AG_WindowNew(AG_WINDOW_MAIN)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, "sgedit");
	AG_SetStyle(win, "padding", "20");

	lbl = AG_LabelNewS(win, AG_LABEL_HFILL, "sgedit");
	AG_SetStyle(lbl, "font-size", "250%");
	AG_SetStyle(lbl, "text-color", "#aaeeff");
	AG_SetStyle(lbl, "font-weight", "bold");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);

	AG_LabelNewS(win, 0, _("Create New:"));

	for (i = 0; i < agFileExtCount; i++) {
		me = &agFileExtMap[i];
		if (!me->editDirect) {
			continue;
		}
		AG_ButtonNewFn(win, AG_BUTTON_HFILL, me->descr,
		    SG_GUI_NewObject, "%p", me->cls);
	}
	
	AG_SeparatorNewHoriz(win);

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Quit"), AGWINCLOSE(win));

	AG_WindowSetPosition(win, AG_WINDOW_MC, 0);
	AG_WindowShow(win);
}

/* Create a read-only view for a scene. */
void
SG_GUI_CreateNewView(AG_Event *event)
{
	char name[AG_OBJECT_NAME_MAX];
	AG_Window *wMain = AG_WINDOW_PTR(1);
	SG_View *svOrig = SG_VIEW_PTR(2), *sv;
	const int shareCam = AG_INT(3);
	SG *sg = svOrig->sg;
	AG_Window *win;
	int num = 0;
	
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, OBJECT(sg)->name);
	sv = SG_ViewNew(win, sg, SG_VIEW_EXPAND);
tryname:
	AG_Snprintf(name, sizeof(name), "cView%d", num++);
	if (SG_FindNode(sg, name) != NULL) {
		goto tryname;
	}
	sv->cam = shareCam ? svOrig->cam :
	          SG_CameraNewDuplicate(sg->root, name, svOrig->cam);
	AG_RedrawOnTick(sv, 16);

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_TR, 40, 30);
	AG_WindowAttach(wMain, win);
	AG_WindowShow(win);
}

static void
CloseEditor(AG_Event *_Nonnull event)
{
	AG_Menu *m = AG_MENU_SELF();
	AG_Window *win = AG_ParentWindow(m);

	if (win)
		AG_PostEvent(win, "window-close", NULL);
}

/* Build a generic "File" menu. */
void
SG_FileMenu(AG_MenuItem *m, void *obj, AG_Window *wMain)
{
	const AG_FileExtMapping *me = NULL;
	int j;

	for (j = 0; j < agFileExtCount; j++) {
		me = &agFileExtMap[j];
		if (!me->editDirect) {
			continue;
		}
		AG_MenuAction(m,
		    AG_Printf("New %s...", me->descr),
		    agIconDoc.s,
		    SG_GUI_NewObject, "%p", me->cls);
	}
	
	AG_MenuSeparator(m);

	AG_MenuActionKb(m, _("Open..."), agIconLoad.s,
	    AG_KEY_O, AG_KEYMOD_CTRL,
	    SG_GUI_OpenDlg, "%p", wMain);
	AG_MenuActionKb(m, _("Save"), agIconSave.s,
	    AG_KEY_S, AG_KEYMOD_CTRL,
	    SG_GUI_Save, "%p,%p,%i", wMain, obj, 0);
	AG_MenuAction(m, _("Save as..."), agIconSave.s,
	    SG_GUI_SaveAsDlg, "%p,%p,%i", wMain, obj, 0);
	
	AG_MenuSeparator(m);

	AG_MenuAction(m, _("Close"), agIconClose.s,
	    CloseEditor, "%p", obj);
}

/* Build a generic "Edit" menu. */
void
SG_EditMenu(AG_MenuItem *m, void *obj, AG_Window *wMain)
{
	AG_MenuActionKb(m, _("Undo"), agIconUp.s, AG_KEY_Z, AG_KEYMOD_CTRL,
	    SG_GUI_Undo, "%p", obj);
	AG_MenuActionKb(m, _("Redo"), agIconDown.s, AG_KEY_R, AG_KEYMOD_CTRL,
	    SG_GUI_Redo, "%p", obj);
	
	AG_MenuSeparator(m);

	AG_MenuAction(m, _("Font preferences..."), agIconMagnifier.s,
	    SG_GUI_SelectFontDlg, "%p", wMain);
}

/* Build a generic "View" menu. */
void
SG_ViewMenu(AG_MenuItem *m, void *obj, AG_Window *wMain, SG_View *sv)
{
	AG_MenuAction(m, _("New view..."), NULL,
	    SG_GUI_CreateNewView, "%p,%p,%i", wMain, sv, 0);
	AG_MenuAction(m, _("New view (share camera)..."), NULL,
	    SG_GUI_CreateNewView, "%p,%p,%i", wMain, sv, 1);
	
//	AG_MenuSeparator(m);
//	AG_MenuAction(m, _("Switch camera..."), NULL,
//	    SG_GUI_SwitchCameraDlg, "%p", sv);
}

/* Initialize Agar-SG GUI globals. */
void
SG_InitGUI(void)
{
	if (agGUI) {
		AG_RegisterClass(&sgViewClass);
		AG_RegisterClass(&sgPaletteViewClass);
		sgIcon_Init();
	}
	AG_ObjectInit(&sgVfsRoot, NULL);
	sgVfsRoot.flags |= AG_OBJECT_STATIC;
	AG_ObjectSetName(&sgVfsRoot, "Agar-SG VFS");
	TAILQ_INIT(&sgEditorWindows);
}

/* Release Agar-SG GUI globals. */
void
SG_DestroyGUI(void)
{
	if (agGUI) {
		AG_UnregisterClass(&sgViewClass);
		AG_UnregisterClass(&sgPaletteViewClass);
	}
	AG_ObjectDestroy(&sgVfsRoot);
}
