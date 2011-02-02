/*
 * Copyright (c) 2007-2009 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <agar/gui/load_xcf.h>
#include <agar/rg.h>

#include "config/sharedir.h"
#include "config/have_agar_dev.h"
#include "config/version.h"
#include "config/enable_nls.h"
#include "config/localedir.h"

#ifdef HAVE_AGAR_DEV
#include <agar/dev.h>
#endif
/* #define SPLASH */

#include <string.h>

#include "agarpaint.h"

static AG_Object editor;
static AG_Menu *appMenu = NULL;
static RG_Tileset *tsFocused = NULL;

static void
SaveAndClose(RG_Tileset *ts, AG_Window *win)
{
	AG_ObjectDetach(win);
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
		    _("Save changes to %s?"), AGOBJECT(ts)->name);
		AG_WindowAttach(win, wDlg);
		
		AG_ButtonText(bOpts[0], _("Save"));
		AG_SetEvent(bOpts[0], "button-pushed", SaveChangesReturn,
		    "%p,%p,%i", win, ts, 1);
		AG_WidgetFocus(bOpts[0]);
		AG_ButtonText(bOpts[1], _("Discard"));
		AG_SetEvent(bOpts[1], "button-pushed", SaveChangesReturn,
		    "%p,%p,%i", win, ts, 0);
		AG_ButtonText(bOpts[2], _("Cancel"));
		AG_SetEvent(bOpts[2], "button-pushed", AGWINDETACH(wDlg));
	}
}

static void
WindowGainedFocus(AG_Event *event)
{
/*	AG_Window *win = AG_SELF(); */
	RG_Tileset *ts = AG_PTR(1);

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
	extern AG_Menu *agAppMenu;
	AG_Window *win;

	win = rgTilesetClass.edit(ts);
	AG_SetEvent(win, "window-close", SaveChangesDlg, "%p", ts);
	AG_AddEvent(win, "window-gainfocus", WindowGainedFocus, "%p", ts);
	AG_AddEvent(win, "window-lostfocus", WindowLostFocus, "%p", ts);
	AG_AddEvent(win, "window-hidden", WindowLostFocus, "%p", ts);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_ML, 30, 80);
	AG_WindowShow(win);
}

static void
NewTileset(AG_Event *event)
{
	RG_Tileset *ts;

	ts = AG_ObjectNew(&editor, NULL, &rgTilesetClass);
	CreateEditionWindow(ts);
}

static void
OpenTilesetAGT(AG_Event *event)
{
	char *path = AG_STRING(1);
	RG_Tileset *ts;

	ts = AG_ObjectNew(&editor, NULL, &rgTilesetClass);
	if (AG_ObjectLoadFromFile(ts, path) == -1) {
		AG_TextMsgFromError();
		AG_ObjectDetach(ts);
		AG_ObjectDestroy(ts);
		return;
	}
	AG_ObjectSetArchivePath(ts, path);
	CreateEditionWindow(ts);
}

static void
OpenTilesetDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fd;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Open tileset..."));
	fd = AG_FileDlgNewMRU(win, "agarpaint.mru.tilesets",
	    AG_FILEDLG_LOAD|AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, _("AgarPaint tileset"), "*.agt",
	    OpenTilesetAGT, NULL);
	AG_WindowShow(win);
}

static void
SaveTilesetToAGT(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	char *path = AG_STRING(2);

	if (AG_ObjectSaveToFile(ts, path) == -1) {
		AG_TextMsgFromError();
	}
	AG_ObjectSetArchivePath(ts, path);
}

static __inline__ void
TransformIconName(char *s)
{
	char *c;

	for (c = s; *c != '\0'; c++) {
		if (*c == '#') {
			*c = 'n';
		} else if (!isalnum(*c))
			*c = '_';
	}
}

static void
SaveTilesetToIconsHdr(AG_Event *event)
{
	char pathData[AG_PATHNAME_MAX];
	char iconID[64];
	RG_Tileset *ts = AG_PTR(1);
	char *path = AG_STRING(2);
	AG_FileType *ft = AG_PTR(3);
	char *pkgName = AG_FileOptionString(ft,"pkg-name");
	int cppDecls =  AG_FileOptionBool(ft,"cpp-decls");
	int beginCode =  AG_FileOptionBool(ft,"begin-code");
	int w, h, count = 0;
	RG_Tile *t;
	FILE *f;
	Uint8 *src;
	char *c;

	if ((f = fopen(path, "w")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("Unable to open %s"), path);
		return;
	}
	fprintf(f, "/* Agar icon reference file */\n");
	fprintf(f, "/* Generated by agarpaint %s */\n\n", VERSION);
	if (beginCode) { fprintf(f, "#include \"begin.h\"\n"); }
	if (cppDecls) { fprintf(f, "__BEGIN_DECLS\n"); }
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		Strlcpy(iconID, t->name, sizeof(iconID));
		TransformIconName(iconID);
		fprintf(f, "extern AG_StaticIcon %s%s;\n", pkgName, iconID);
	}
	if (cppDecls) { fprintf(f, "__END_DECLS\n"); }
	if (beginCode) { fprintf(f, "#include \"close.h\"\n"); }
	fclose(f);

	Strlcpy(pathData, path, sizeof(pathData));
	if ((c = strrchr(pathData, '.')) != NULL) { *c = '\0'; }
	Strlcat(pathData, "_data.h", sizeof(pathData));
	if ((f = fopen(pathData, "w")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("Unable to open %s"), pathData);
		return;
	}
	fprintf(f, "/* Agar icon data file */\n");
	fprintf(f, "/* Generated by agarpaint %s */\n\n", VERSION);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		Strlcpy(iconID, t->name, sizeof(iconID));
		TransformIconName(iconID);
		fprintf(f, "static const Uint32 %s%s_Data[%u] = {",
		    pkgName, iconID, t->su->w*t->su->h);
		src = t->su->pixels;
		for (h = 0; h < t->su->h; h++) {
			for (w = 0; w < t->su->w; w++) {
				unsigned long px = AG_GetPixel(t->su, src);

				if (px == 0) {
					fprintf(f, "0,");
				} else {
					fprintf(f, "0x%lx,", px);
				}
				src += t->su->format->BytesPerPixel;
			}
		}
		fprintf(f, "};\n");
		fprintf(f, "AG_StaticIcon %s%s = {", pkgName, iconID);
		fprintf(f, "%lu,%lu,0x%.08lx,0x%.08lx,0x%.08lx,0x%.08lx,",
		    (unsigned long)t->su->w,
		    (unsigned long)t->su->h,
		    (unsigned long)t->su->format->Rmask,
		    (unsigned long)t->su->format->Gmask,
		    (unsigned long)t->su->format->Bmask,
		    (unsigned long)t->su->format->Amask);
		fprintf(f, "%s%s_Data, NULL", pkgName, iconID);
		fprintf(f, "};\n");
	}
	fprintf(f, "\nstatic __inline__ void\n%s_Init(void)\n{\n", pkgName);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		Strlcpy(iconID, t->name, sizeof(iconID));
		TransformIconName(iconID);
		fprintf(f, "\tAG_InitStaticIcon(&%s%s);\n", pkgName, iconID);
		count++;
	}
	fprintf(f, "}\n");
	fclose(f);

	AG_TextInfo("exported-tileset",
	    _("Successfully exported %s to header (%s)"),
	    AGOBJECT(ts)->name, pkgName);
}

static void
SaveTilesetAsDlg(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;
	AG_FileType *ft;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Save tileset as..."));
	fd = AG_FileDlgNewMRU(win, "agarpaint.mru.tilesets",
	    AG_FILEDLG_SAVE|AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgSetOptionContainer(fd, AG_BoxNewVert(win, AG_BOX_HFILL));

	AG_FileDlgAddType(fd, _("AgarPaint tileset"), "*.agt",
	    SaveTilesetToAGT, "%p", ts);
	ft = AG_FileDlgAddType(fd, _("Agar icon header"), "*.h",
	    SaveTilesetToIconsHdr, "%p", ts);
	AG_FileOptionNewString(ft, _("Package name: "), "pkg-name", "fooIcon");
	AG_FileOptionNewBool(ft, _("Use begin and close.h"), "begin-code", 1);
	AG_FileOptionNewBool(ft, _("Use __BEGIN_DECLS"), "cpp-decls", 1);

	AG_WindowShow(win);
}

static void
ImportImageFromBMP(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	char *path = AG_STRING(2);
	AG_Surface *bmp;
	RG_Tile *t;
	RG_Pixmap *px;

	if ((bmp = AG_SurfaceFromBMP(path)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", path, AG_GetError());
		return;
	}
	px = RG_PixmapNew(ts, "Bitmap", 0);
	px->su = AG_SurfaceConvert(bmp, ts->fmt);
	AG_SurfaceFree(bmp);
	
	t = RG_TileNew(ts, "Bitmap", px->su->w, px->su->h, RG_TILE_SRCALPHA);
	RG_TileAddPixmap(t, NULL, px, 0, 0);
	RG_TileGenerate(t);
}

static void
LoadTileFromXCF(AG_Surface *xcf, const char *lbl, void *p)
{
	RG_Tileset *ts = p;
	RG_Pixmap *px;
	RG_Tile *t;

	px = RG_PixmapNew(ts, lbl, 0);
	px->su = AG_SurfaceConvert(xcf, ts->fmt);

	t = RG_TileNew(ts, lbl, xcf->w, xcf->h, RG_TILE_SRCALPHA);
	RG_TileAddPixmap(t, NULL, px, 0, 0);
	RG_TileGenerate(t);
}

static void
ImportImagesFromXCF(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	char *path = AG_STRING(2);
	AG_DataSource *buf;

	if ((buf = AG_OpenFile(path, "rb")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", path, AG_GetError());
		return;
	}
	if (AG_XCFLoad(buf, 0, LoadTileFromXCF, ts) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", path, AG_GetError());
	} else {
		AG_TextInfo("imported-image",
		    _("Successfully imported XCF image into %s"),
		    AGOBJECT(ts)->name);
	}
	AG_CloseFile(buf);
}

static void
ImportImagesDlg(AG_Event *event)
{
	RG_Tileset *ts = AG_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Import images..."));
	fd = AG_FileDlgNewMRU(win, "agarpaint.mru.tilesets",
	    AG_FILEDLG_LOAD|AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, _("PC bitmap"), "*.bmp",
	    ImportImageFromBMP, "%p", ts);
	AG_FileDlgAddType(fd, _("Gimp XCF layers"), "*.xcf",
	    ImportImagesFromXCF, "%p", ts);
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
		AG_TextMsg(AG_MSG_ERROR, _("Error saving tileset: %s"),
		    AG_GetError());
	} else {
		AG_TextInfo("saved-tileset",
		    _("Saved tileset %s successfully"),
		    AGOBJECT(ts)->name);
	}
}

static void
ConfirmQuit(AG_Event *event)
{
	AG_QuitGUI();
}

static void
AbortQuit(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	agTerminating = 0;
	AG_ObjectDetach(win);
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

	AGOBJECT_FOREACH_CLASS(ts, &editor, rg_tileset, "RG_Tileset:*") {
		if (AG_ObjectChanged(ts))
			break;
	}
	if (ts == NULL) {
		ConfirmQuit(NULL);
	} else {
		if ((win = AG_WindowNewNamedS(
		    AG_WINDOW_MODAL|AG_WINDOW_NOTITLE| AG_WINDOW_NORESIZE,
		    "QuitCallback")) == NULL) {
			return;
		}
		AG_WindowSetCaption(win, _("Exit application?"));
		AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
		AG_WindowSetSpacing(win, 8);
		AG_LabelNewS(win, 0,
		    _("There is at least one tileset with unsaved changes.  "
	              "Exit application?"));
		box = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|
		                                   AG_VBOX_HFILL);
		AG_BoxSetSpacing(box, 0);
		AG_BoxSetPadding(box, 0);
		AG_ButtonNewFn(box, 0, _("Discard changes"), ConfirmQuit, NULL);
		AG_WidgetFocus(AG_ButtonNewFn(box, 0, _("Cancel"), AbortQuit,
		    "%p", win));
		AG_WindowShow(win);
	}
}

static void
FileMenu(AG_Event *event)
{
	AG_MenuItem *m = AG_SENDER();

	AG_MenuActionKb(m, _("New"), agIconDoc.s,
	    AG_KEY_N, AG_KEYMOD_CTRL,
	    NewTileset, NULL);
	AG_MenuActionKb(m, _("Open..."), agIconLoad.s,
	    AG_KEY_O, AG_KEYMOD_CTRL,
	    OpenTilesetDlg, NULL);

	if (tsFocused == NULL) { AG_MenuDisable(m); }

	AG_MenuActionKb(m, _("Save"), agIconSave.s,
	    AG_KEY_S, AG_KEYMOD_CTRL,
	    SaveTileset, "%p", tsFocused);
	AG_MenuAction(m, _("Save as..."), agIconSave.s,
	    SaveTilesetAsDlg, "%p", tsFocused);
	
	AG_MenuSeparator(m);

	AG_MenuActionKb(m, _("Import images..."), agIconDocImport.s,
	    AG_KEY_O, AG_KEYMOD_CTRL,
	    ImportImagesDlg, "%p", tsFocused);

	if (tsFocused == NULL) { AG_MenuEnable(m); }
	
	AG_MenuSeparator(m);
	AG_MenuActionKb(m, "Quit", NULL,
	    AG_KEY_Q, AG_KEYMOD_CTRL,
	    Quit, NULL);
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

	AG_MenuActionKb(m, "Undo", NULL,
	    AG_KEY_Z, AG_KEYMOD_CTRL,
	    Undo, "%p", tsFocused);

	if (tsFocused == NULL) { AG_MenuEnable(m); }
}

#ifdef SPLASH

static Uint32
SplashExpire(void *obj, Uint32 ival, void *arg)
{
	AG_Window *win = obj;

	AG_ObjectDetach(win);
	return (0);
}

static void
Splash(void)
{
	char path[AG_FILENAME_MAX];
	AG_Window *win;
	AG_Pixmap *px;
	AG_Timeout to;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	if (AG_ConfigFile("load-path", "agarpaint.bmp", NULL, path,
	    sizeof(path)) == 0) {
		px = AG_PixmapFromBMP(win, 0, path);
		AG_SetTimeout(&to, SplashExpire, NULL, 0);
		AG_ScheduleTimeout(win, &to, 2000);
	} else {
		fprintf(stderr, "%s\n", AG_GetError());
	}
	AG_WindowShow(win);
}

#endif /* SPLASH */

int
main(int argc, char *argv[])
{
	int i, debug = 0;
	const char *fontSpec = NULL;
	const char *driverSpec = NULL;
	char *optArg = NULL;
	int optInd;
	RG_Tileset *ts;
	int c, openedFiles = 0;

#ifdef ENABLE_NLS
	bindtextdomain("agarpaint", LOCALEDIR);
	bind_textdomain_codeset("agarpaint", "UTF-8");
	textdomain("agarpaint");
#endif

	if (AG_InitCore("agarpaint", AG_CREATE_DATADIR) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((c = AG_Getopt(argc, argv, "?vDd:t:", &optArg, &optInd)) != -1) {
		switch (c) {
		case 'v':
			printf("agarpaint %s\n", VERSION);
			exit(0);
		case 'D':
			debug = 1;
			break;
		case 'd':
			driverSpec = optArg;
			break;
		case 't':
			fontSpec = optArg;
			break;
		case '?':
		default:
			printf("%s [-vD] [-d agar-driver-spec] "
			       "[-t font,size,flags]\n", agProgName);
			exit(0);
		}
	}

	if (fontSpec != NULL) {
		AG_TextParseFontSpec(fontSpec);
	}
	if (AG_InitGraphics(driverSpec) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_PrtString(agConfig, "load-path", ".:%s", SHAREDIR);

	AG_AtExitFuncEv(Quit);
	AG_BindGlobalKeyEv(AG_KEY_ESCAPE, AG_KEYMOD_ANY, Quit);
	AG_BindGlobalKey(AG_KEY_F8, AG_KEYMOD_ANY, AG_ViewCapture);

	/* Initialize the subsystems. */
	RG_InitSubsystem();

	/* Initialize an "editor" object we will use as VFS root. */
	AG_ObjectInitStatic(&editor, NULL);

	/* Create the application menu. */ 
	if (agDriverSw != NULL) {
		/*
		 * MDI-style
		 */
		appMenu = AG_MenuNewGlobal(0);
		AG_MenuDynamicItem(appMenu->root, _("File"), NULL, FileMenu, NULL);
		AG_MenuDynamicItem(appMenu->root, _("Edit"), NULL, EditMenu, NULL);
#ifdef HAVE_AGAR_DEV
		if (debug) {
			DEV_InitSubsystem(0);
			DEV_ToolMenu(AG_MenuNode(appMenu->root, _("Debug"), NULL));
		}
#endif
#ifdef SPLASH
		Splash();
#endif
	}

	for (i = optInd; i < argc; i++) {
		ts = AG_ObjectNew(&editor, NULL, &rgTilesetClass);
		if (AG_ObjectLoadFromFile(ts, argv[i]) == 0) {
			AG_ObjectSetArchivePath(ts, argv[i]);
			CreateEditionWindow(ts);
			openedFiles++;
		} else {
			AG_TextMsgFromError();
			AG_ObjectDetach(ts);
			AG_ObjectDestroy(ts);
		}
	}
	if (!agDriverSw && !openedFiles) {
		ts = AG_ObjectNew(&editor, NULL, &rgTilesetClass);
		CreateEditionWindow(ts);
	}

	AG_EventLoop();
	AG_Destroy();
	return (0);
}

