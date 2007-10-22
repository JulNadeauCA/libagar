/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "config/sharedir.h"
#include "config/have_agar_dev.h"
#include "config/version.h"
#include "config/release.h"

#ifdef HAVE_AGAR_DEV
#include <agar/dev.h>
#endif
/* #define SPLASH */

#include <string.h>
#include <unistd.h>
#include <errno.h>

extern const AG_ObjectOps rgTilesetOps;
static AG_Menu *appMenu = NULL;
static RG_Tileset *tsFocused = NULL;

static void
SaveAndClose(RG_Tileset *ts, AG_Window *win)
{
	AG_ViewDetach(win);
	AG_ObjectPageOut(ts);
}

static void
SaveChangesReturn(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);
	RG_Tileset *ts = AG_PTR(2);

	SaveAndClose(ts, win);
}

static void
SaveChangesDlg(AG_Event *event)
{
	AG_Window *win = AG_SELF();
	RG_Tileset *ts = AG_PTR(1);

	if (!AG_ObjectChanged(ts)) {
		SaveAndClose(ts, win);
	} else {
		AG_Button *bOpts[3];
		AG_Window *wDlg;

		wDlg = AG_TextPromptOptions(bOpts, 3,
		    "Save changes to %s?", AGOBJECT(ts)->name);
		AG_WindowAttach(win, wDlg);
		
		AG_ButtonText(bOpts[0], "Save");
		AG_SetEvent(bOpts[0], "button-pushed", SaveChangesReturn,
		    "%p,%p,%i", win, ts, 1);
		AG_WidgetFocus(bOpts[0]);
		AG_ButtonText(bOpts[1], "Discard");
		AG_SetEvent(bOpts[1], "button-pushed", SaveChangesReturn,
		    "%p,%p,%i", win, ts, 0);
		AG_ButtonText(bOpts[2], "Cancel");
		AG_SetEvent(bOpts[2], "button-pushed", AGWINDETACH(wDlg));
	}
}

static void
WindowGainedFocus(AG_Event *event)
{
/*	AG_Window *win = AG_SELF(); */
	RG_Tileset *ts = AG_PTR(1);

	printf("gained focus: %s\n", AGOBJECT(ts)->name);
	tsFocused = ts;
}

static void
WindowLostFocus(AG_Event *event)
{
/*	AG_Window *win = AG_SELF(); */

	tsFocused = NULL;
}

static void
CreateEditionWindow(RG_Tileset *ts)
{
	AG_Window *win;

	win = rgTilesetOps.edit(ts);
	AG_SetEvent(win, "window-close", SaveChangesDlg, "%p", ts);
	AG_AddEvent(win, "window-gainfocus", WindowGainedFocus, "%p", ts);
	AG_AddEvent(win, "window-lostfocus", WindowLostFocus, "%p", ts);
	AG_AddEvent(win, "window-hidden", WindowLostFocus, "%p", ts);
	AG_WindowShow(win);
}

static void
NewTileset(AG_Event *event)
{
	RG_Tileset *ts;

	ts = AG_ObjectNew(agWorld, NULL, &rgTilesetOps);
	CreateEditionWindow(ts);
}

static void
OpenTileset(AG_Event *event)
{
	AG_Window *pWin = AG_PTR(1);
	char *path = AG_STRING(2);
	RG_Tileset *ts;

	ts = AG_ObjectNew(agWorld, NULL, &rgTilesetOps);
	if (AG_ObjectLoadFromFile(ts, path) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "Error loading %s: %s",
		    path, AG_GetError());
		AG_ObjectDetach(ts);
		AG_ObjectDestroy(ts);
		AG_Free(ts, M_OBJECT);
		return;
	}
	AG_ObjectSetArchivePath(ts, path);
	AG_ViewDetach(pWin);
	CreateEditionWindow(ts);
}

static void
OpenTilesetDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fd;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Open tileset...");
	fd = AG_FileDlgNewMRU(win, "agarpaint.mru.tilesets",
	    AG_FILEDLG_LOAD|AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, "agarpaint tileset", "*.agt", NULL, NULL);
	AG_FileDlgOkAction(fd, OpenTileset, "%p", win);
	AG_WindowShow(win);
}

static void
SaveTilesetToAGT(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	char *path = AG_STRING(2);

	if (AG_ObjectSaveToFile(ts, path) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "Error saving tileset: %s",
		    AG_GetError());
	}
	AG_ObjectSetArchivePath(ts, path);
}

static void
SaveTilesetToIcons(AG_Event *event)
{
	char iconID[64];
	RG_Tileset *ts = AG_PTR(1);
	RG_Tile *t;
	char *path = AG_STRING(2);
	FILE *f;

	if ((f = fopen(path, "w")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", path, strerror(errno));
		return;
	}
	fprintf(f,
	    "/* \n"
	    " * Resource file generated by agarpaint %s (%s)\n"
	    " */\n\n",
	    VERSION,
	    RELEASE);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		fprintf(f, "/* Source: %s */\n", t->name);
		fprintf(f, "const Uint agIconWidth_%s = %u", iconID, t->su->w);
		fprintf(f, "const Uint agIconHeight_%s = %u", iconID, t->su->h);
	}
	fclose(f);
}

static void
SaveTilesetAsDlg(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, "Save tileset as...");
	fd = AG_FileDlgNewMRU(win, "agarpaint.mru.tilesets",
	    AG_FILEDLG_SAVE|AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, "agarpaint tileset", "*.agt",
	    SaveTilesetToAGT, "%p", ts);
	AG_FileDlgAddType(fd, "agar icons", "*.h",
	    SaveTilesetToIcons, "%p", ts);
	AG_WindowShow(win);
}

static void
SaveTileset(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);

	if (AGOBJECT(ts)->archivePath == NULL) {
		SaveTilesetAsDlg(event);
		return;
	}
	if (AG_ObjectSave(ts) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "Error saving tileset: %s",
		    AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 500, "Saved tileset %s successfully",
		    AGOBJECT(ts)->name);
	}
}

static void
ConfirmQuit(AG_Event *event)
{
	SDL_Event nev;

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

static void
Quit(AG_Event *event)
{
	RG_Tileset *ts;
	AG_Window *win;
	AG_Box *box;

	if (agTerminating) {
		ConfirmQuit(NULL);
	}
	agTerminating = 1;

	AGOBJECT_FOREACH_CLASS(ts, agWorld, rg_tileset, "RG_Tileset:*") {
		if (AG_ObjectChanged(ts))
			break;
	}
	if (ts == NULL) {
		ConfirmQuit(NULL);
	} else {
		if ((win = AG_WindowNewNamed(AG_WINDOW_MODAL|AG_WINDOW_NOTITLE|
		    AG_WINDOW_NORESIZE, "QuitCallback")) == NULL) {
			return;
		}
		AG_WindowSetCaption(win, "Exit application?");
		AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
		AG_WindowSetSpacing(win, 8);
		AG_LabelNewStaticString(win, 0,
		    "There is at least one tileset with unsaved changes.  "
	            "Exit application?");
		box = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|
		                                   AG_VBOX_HFILL);
		AG_BoxSetSpacing(box, 0);
		AG_BoxSetPadding(box, 0);
		AG_ButtonNewFn(box, 0, "Discard changes", ConfirmQuit, NULL);
		AG_WidgetFocus(AG_ButtonNewFn(box, 0, "Cancel", AbortQuit,
		    "%p", win));
		AG_WindowShow(win);
	}
}

static void
FileMenu(AG_Event *event)
{
	AG_MenuItem *m = AG_SENDER();

	AG_MenuActionKb(m, "New", -1, SDLK_n, KMOD_CTRL,
	    NewTileset, NULL);
	AG_MenuActionKb(m, "Open...", -1, SDLK_o, KMOD_CTRL,
	    OpenTilesetDlg, NULL);

	if (tsFocused == NULL) { AG_MenuDisable(m); }

	AG_MenuActionKb(m, "Save", -1, SDLK_s, KMOD_CTRL,
	    SaveTileset, "%p", tsFocused);
	AG_MenuAction(m, "Save as...",	-1,
	    SaveTilesetAsDlg, "%p", tsFocused);

	if (tsFocused == NULL) { AG_MenuEnable(m); }
	
	AG_MenuSeparator(m);
	AG_MenuActionKb(m, "Quit", -1, SDLK_q, KMOD_CTRL, Quit, NULL);
}

static void
Undo(AG_Event *event)
{
	printf("undo!\n");
}

static void
EditMenu(AG_Event *event)
{
	AG_MenuItem *m = AG_SENDER();
	
	if (tsFocused == NULL) { AG_MenuDisable(m); }

	AG_MenuActionKb(m, "Undo", -1, SDLK_z, KMOD_CTRL,
	    Undo, "%p", tsFocused);

	if (tsFocused == NULL) { AG_MenuEnable(m); }
}

#ifdef SPLASH

static Uint32
SplashExpire(void *obj, Uint32 ival, void *arg)
{
	AG_Window *win = obj;

	AG_ViewDetach(win);
	return (0);
}

static void
Splash(void)
{
	char path[FILENAME_MAX];
	AG_Window *win;
	AG_Pixmap *px;
	AG_Timeout to;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	if (AG_ConfigFile("load-path", "agarpaint.bmp", NULL, path,
	    sizeof(path)) == 0) {
		px = AG_PixmapFromBMP(win, 0, path);
		AG_SetTimeout(&to, SplashExpire, NULL, 0);
		AG_AddTimeout(win, &to, 2000);
	} else {
		fprintf(stderr, "%s\n", AG_GetError());
	}
	AG_WindowShow(win);
}

#endif /* SPLASH */

int
main(int argc, char *argv[])
{
	int c, fps = -1;
	const char *fontSpec = NULL;
#ifdef HAVE_AGAR_DEV
	int debug = 0;
#endif
	if (AG_InitCore("agarpaint", AG_CORE_VERBOSE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((c = getopt(argc, argv, "?dvfFgGr:t:")) != -1) {
		extern char *optarg;

		switch (c) {
		case 'v':
			exit(0);
		case 'f':
			AG_SetBool(agConfig, "view.full-screen", 1);
			break;
		case 'F':
			AG_SetBool(agConfig, "view.full-screen", 0);
			break;
#ifdef HAVE_OPENGL
		case 'g':
			AG_SetBool(agConfig, "view.opengl", 1);
			break;
		case 'G':
			AG_SetBool(agConfig, "view.opengl", 0);
			break;
#endif
		case 'r':
			fps = atoi(optarg);
			break;
		case 't':
			fontSpec = optarg;
			break;
#ifdef HAVE_AGAR_DEV
		case 'd':
			debug = 1;
			break;
#endif
		case '?':
		default:
			printf("%s [-vfFgGd] [-t font,size,flags] [-r fps]\n",
			    agProgName);
			exit(0);
		}
	}
	if (fontSpec != NULL) {
		AG_TextParseFontSpec(fontSpec);
	}
	if (AG_InitVideo(800, 600, 32, 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_InitInput(0);
	AG_SetRefreshRate(fps);
	AG_SetString(agConfig, "load-path", ".:%s", SHAREDIR);

	AG_AtExitFuncEv(Quit);
	AG_BindGlobalKeyEv(SDLK_ESCAPE, KMOD_NONE, Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);

	/* Initialize the subsystems. */
	RG_InitSubsystem();
	MAP_InitSubsystem();

	/* Initialize default application settings. */
	if (AG_ObjectLoad(agWorld) == -1) {
		AG_ObjectSave(agConfig);
		AG_ObjectSave(agWorld);
	}

	/* Create the application menu. */ 
	appMenu = AG_MenuNewGlobal(0);
	AG_MenuDynamicItem(appMenu->root, "File", -1, FileMenu, NULL);
	AG_MenuDynamicItem(appMenu->root, "Edit", -1, EditMenu, NULL);
#ifdef HAVE_AGAR_DEV
	if (debug) {
		DEV_InitSubsystem(0);
		DEV_ToolMenu(AG_MenuNode(appMenu->root, "Debug", -1));
	}
#endif
#ifdef SPLASH
	Splash();
#endif
	AG_EventLoop();
	AG_Destroy();
	return (0);
}

