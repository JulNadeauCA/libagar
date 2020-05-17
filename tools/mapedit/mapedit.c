/*
 * Copyright (c) 2007-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/map.h>

#include "config/datadir.h"
#include "config/version.h"
#include "config/enable_nls.h"
#include "config/localedir.h"

#include <string.h>
#include <ctype.h>

#include "mapedit.h"

static AG_Object mapedit;
static AG_Menu *appMenu = NULL;
static MAP *mapFocused = NULL;
static int exiting = 0;
static int nEditorWindows = 0;

typedef AG_Surface *(*AG_SurfaceFromFn)(const char *);

static void
CloseObject(AG_Event *event)
{
	AG_Window *win = AG_WINDOW_PTR(1);
	MAP *m = MAP_PTR(2);
	int save = AG_INT(3);

	if (save) {
		if (AG_ObjectSave(m) == -1) {
			AG_TextMsgFromError();	/* TODO suggest "save as" */
			return;
		}
	}
	AG_ObjectPageOut(m);
	AG_ObjectDetach(win);

	if (--nEditorWindows == 0)
		AG_Terminate(0);
}

static void
WindowClose(AG_Event *event)
{
	AG_Window *win = AG_WINDOW_SELF();
	MAP *m = MAP_PTR(1);
	AG_Event ev;
	AG_Button *bOpts[3];
	AG_Window *wDlg;

	if (!AG_ObjectChanged(m)) {
		AG_EventArgs(&ev, "%p,%p,%i", win, m, 0);
		CloseObject(&ev);
		return;
	}
	wDlg = AG_TextPromptOptions(bOpts, 3, _("Save changes to %s?"), AGOBJECT(m)->name);
	AG_WindowAttach(win, wDlg);
	AG_ButtonText(bOpts[0], _("Save"));
	AG_SetEvent(bOpts[0], "button-pushed", CloseObject, "%p,%p,%i", win, m, 1);
	AG_WidgetFocus(bOpts[0]);
	AG_ButtonText(bOpts[1], _("Discard"));
	AG_SetEvent(bOpts[1], "button-pushed", CloseObject, "%p,%p,%i", win, m, 0);
	AG_ButtonText(bOpts[2], _("Cancel"));
	AG_SetEvent(bOpts[2], "button-pushed", AGWINDETACH(wDlg));
}

static void
WindowGainedFocus(AG_Event *event)
{
/*	AG_Window *win = AG_WINDOW_SELF(); */
	MAP *m = MAP_PTR(1);

	mapFocused = m;
}

static void
WindowLostFocus(AG_Event *event)
{
/*	AG_Window *win = AG_WINDOW_SELF(); */

	mapFocused = NULL;
}

static void
CreateMapEditor(MAP *m)
{
	extern AG_Menu *agAppMenu;
	AG_Window *win;

	if ((win = mapClass.edit(m)) == NULL) {
		return;
	}
	AG_SetEvent(win, "window-close", WindowClose, "%p", m);
	AG_AddEvent(win, "window-gainfocus", WindowGainedFocus, "%p", m);
	AG_AddEvent(win, "window-lostfocus", WindowLostFocus, "%p", m);
	AG_AddEvent(win, "window-hidden", WindowLostFocus, "%p", m);

	AG_WindowSetGeometryAligned(win, AG_WINDOW_ML, 300, 500);
	AG_WindowShow(win);

	nEditorWindows++;
}

static void
NewMap(AG_Event *event)
{
	MAP *m;

	m = MAP_New(&mapedit, NULL);
	CreateMapEditor(m);
}

/* Load a level from a native .agm file. */
static void
OpenMapAGM(AG_Event *event)
{
	const char *path = AG_STRING(1);
	MAP *m;

	m = MAP_New(&mapedit, NULL);
	if (AG_ObjectLoadFromFile(m, path) == -1) {
		AG_TextMsgFromError();
		return;
	}
	AG_SetString(m, "archive-path", path);
	CreateMapEditor(m);
}

static void
OpenMapDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fd;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Load map from..."));

	fd = AG_FileDlgNewMRU(win, "map-maps", AG_FILEDLG_LOAD |
	                                       AG_FILEDLG_CLOSEWIN |
	                                       AG_FILEDLG_EXPAND);

	AG_FileDlgAddType(fd, _("Agar-Map Level"), "*.agm",
	    OpenMapAGM, NULL);

	AG_WindowShow(win);
}

static void
SaveMapToAGM(AG_Event *event)
{
	MAP *m = MAP_PTR(1);
	const char *path = AG_STRING(2);

	if (AG_ObjectSaveToFile(m, path) == -1) {
		AG_TextMsgFromError();
		return;
	}
	AG_SetString(m, "archive-path", path);
}

static void
SaveMapAsDlg(AG_Event *event)
{
	MAP *m = MAP_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;
	AG_FileType *ft;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Save map as..."));

	fd = AG_FileDlgNewMRU(win, "map-maps", AG_FILEDLG_SAVE |
	                                       AG_FILEDLG_CLOSEWIN |
	                                       AG_FILEDLG_EXPAND);

	AG_FileDlgSetOptionContainer(fd, AG_BoxNewVert(win, AG_BOX_HFILL));

	AG_FileDlgAddType(fd, _("Agar-Map Level"), "*.agm", SaveMapToAGM, "%p", m);
	/* TODO AG_FileDlgAddType(fd, _("Image File"), "*.png,*.jpg,*.bmp", ExportMapToImage, "%p", m); */

	AG_WindowShow(win);
}

static void
SaveMap(AG_Event *event)
{
	MAP *m = MAP_PTR(1);

	if (!AG_Defined(m,"archive-path")) {
		SaveMapAsDlg(event);
		return;
	}
	if (AG_ObjectSave(m) == -1) {
		AG_TextMsg(AG_MSG_ERROR, _("Error saving map: %s"),
		    AG_GetError());
	} else {
		AG_TextInfo("saved-map", _("Saved map %s successfully"),
		    AGOBJECT(m)->name);
	}
}

static void
FileMenu(AG_Event *event)
{
	AG_MenuItem *m = AG_MENU_ITEM_PTR(1);

	AG_MenuActionKb(m, _("New"), agIconDoc.s,
	    AG_KEY_N, AG_KEYMOD_CTRL,
	    NewMap, NULL);
	AG_MenuActionKb(m, _("Open..."), agIconLoad.s,
	    AG_KEY_O, AG_KEYMOD_CTRL,
	    OpenMapDlg, NULL);

	if (mapFocused == NULL) { AG_MenuDisable(m); }

	AG_MenuActionKb(m, _("Save"), agIconSave.s,
	    AG_KEY_S, AG_KEYMOD_CTRL,
	    SaveMap, "%p", mapFocused);
	AG_MenuAction(m, _("Save as..."), agIconSave.s,
	    SaveMapAsDlg, "%p", mapFocused);
	
	if (mapFocused == NULL) { AG_MenuEnable(m); }
}

static void
Undo(AG_Event *event)
{
	/* TODO */
	printf("undo!\n");
}

static void
EditMenu(AG_Event *event)
{
	AG_MenuItem *m = AG_MENU_ITEM_PTR(1);
	
	if (mapFocused == NULL) { AG_MenuDisable(m); }

	AG_MenuActionKb(m, "Undo", NULL,
	    AG_KEY_Z, AG_KEYMOD_CTRL,
	    Undo, "%p", mapFocused);

	if (mapFocused == NULL) { AG_MenuEnable(m); }
}

int
main(int argc, char *argv[])
{
	const char *fontSpec = NULL;
	const char *driverSpec = NULL;
	char *optArg = NULL;
	MAP *m;
	int optInd;
	int i, debug=0, c, openedFiles=0;

#ifdef ENABLE_NLS
	bindtextdomain("mapedit", LOCALEDIR);
	bind_textdomain_codeset("mapedit", "UTF-8");
	textdomain("mapedit");
#endif
	if (AG_InitCore("mapedit", AG_CREATE_DATADIR) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((c = AG_Getopt(argc, argv, "?vDd:t:p:", &optArg, &optInd)) != -1) {
		switch (c) {
		case 'v':
			printf("mapedit %s\n", VERSION);
			return (0);
		case 'D':
			debug = 1;
			break;
		case 'd':
			driverSpec = optArg;
			break;
		case 't':
			fontSpec = optArg;
			break;
		case 'p':
			break;
		case '?':
		default:
			printf("%s [-vD] [-d agar-driver-spec] [-t font] [file ...]\n", agProgName);
			return (1);
		}
	}

	if (fontSpec != NULL) {
		AG_TextParseFontSpec(fontSpec);
	}
	if (AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	AG_SetStringF(agConfig, "load-path", ".:%s", DATADIR);
	AG_BindStdGlobalKeys();

	MAP_InitSubsystem();

	AG_ObjectInit(&mapedit, NULL);
	mapedit.flags |= AG_OBJECT_STATIC;

	if (agDriverSw != NULL) {			/* Go MDI-style */
		appMenu = AG_MenuNewGlobal(0);
		AG_MenuDynamicItem(appMenu->root, _("File"), NULL, FileMenu, NULL);
		AG_MenuDynamicItem(appMenu->root, _("Edit"), NULL, EditMenu, NULL);
	}

	for (i = optInd; i < argc; i++) {
		m = AG_ObjectNew(&mapedit, NULL, &mapClass);
		if (AG_ObjectLoadFromFile(m, argv[i]) == 0) {
			AG_SetString(m, "archive-path", argv[i]);
			CreateMapEditor(m);
			openedFiles++;
		} else {
			AG_TextMsgFromError();
			AG_ObjectDetach(m);
			AG_ObjectDestroy(m);
		}
	}
	if (!agDriverSw && !openedFiles) {
		m = AG_ObjectNew(&mapedit, NULL, &mapClass);
		CreateMapEditor(m);
	}

	AG_EventLoop();

	AG_ConfigSave();

	MAP_DestroySubsystem();
	AG_DestroyGraphics();
	AG_Destroy();
	return (0);
}
