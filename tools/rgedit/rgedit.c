/*
 * Copyright (c) 2007-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/map/rg.h>

#include "config/datadir.h"
#include "config/version.h"
#include "config/enable_nls.h"
#include "config/localedir.h"

#include <string.h>
#include <ctype.h>

#include "rgedit.h"

static AG_Object rgedit;
static AG_Menu *appMenu = NULL;
static RG_Tileset *tsFocused = NULL;
static int exiting = 0;
static int nEditorWindows = 0;

typedef AG_Surface *(*AG_SurfaceFromFn)(const char *);

static void
CloseObject(AG_Event *event)
{
	AG_Window *win = AG_WINDOW_PTR(1);
	RG_Tileset *ts = RG_TILESET_PTR(2);
	int save = AG_INT(3);

	if (save) {
		if (AG_ObjectSave(ts) == -1) {
			AG_TextMsgFromError();	/* TODO suggest "save as" */
			return;
		}
	}
	AG_ObjectPageOut(ts);
	AG_ObjectDetach(win);

	if (--nEditorWindows == 0)
		AG_Terminate(0);
}

static void
WindowClose(AG_Event *event)
{
	AG_Window *win = AG_WINDOW_SELF();
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Event ev;
	AG_Button *bOpts[3];
	AG_Window *wDlg;

	if (!AG_ObjectChanged(ts)) {
		AG_EventArgs(&ev, "%p,%p,%i", win, ts, 0);
		CloseObject(&ev);
		return;
	}
	wDlg = AG_TextPromptOptions(bOpts, 3, _("Save changes to %s?"), AGOBJECT(ts)->name);
	AG_WindowAttach(win, wDlg);
	AG_ButtonText(bOpts[0], _("Save"));
	AG_SetEvent(bOpts[0], "button-pushed", CloseObject, "%p,%p,%i", win, ts, 1);
	AG_WidgetFocus(bOpts[0]);
	AG_ButtonText(bOpts[1], _("Discard"));
	AG_SetEvent(bOpts[1], "button-pushed", CloseObject, "%p,%p,%i", win, ts, 0);
	AG_ButtonText(bOpts[2], _("Cancel"));
	AG_SetEvent(bOpts[2], "button-pushed", AGWINDETACH(wDlg));
}

static void
WindowGainedFocus(AG_Event *event)
{
/*	AG_Window *win = AG_WINDOW_SELF(); */
	RG_Tileset *ts = RG_TILESET_PTR(1);

	tsFocused = ts;
}

static void
WindowLostFocus(AG_Event *event)
{
/*	AG_Window *win = AG_WINDOW_SELF(); */

	tsFocused = NULL;
}

static void
CreateEditionWindow(RG_Tileset *ts)
{
	extern AG_Menu *agAppMenu;
	AG_Window *win;

	if ((win = rgTilesetClass.edit(ts)) == NULL) {
		return;
	}
	AG_SetEvent(win, "window-close", WindowClose, "%p", ts);
	AG_AddEvent(win, "window-gainfocus", WindowGainedFocus, "%p", ts);
	AG_AddEvent(win, "window-lostfocus", WindowLostFocus, "%p", ts);
	AG_AddEvent(win, "window-hidden", WindowLostFocus, "%p", ts);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_ML, 300, 500);
	AG_WindowShow(win);
	nEditorWindows++;
}

static void
NewTileset(AG_Event *event)
{
	RG_Tileset *ts;

	ts = AG_ObjectNew(&rgedit, NULL, &rgTilesetClass);
	CreateEditionWindow(ts);
}

static void
OpenTilesetAGT(AG_Event *event)
{
	char *path = AG_STRING(1);
	RG_Tileset *ts;

	ts = AG_ObjectNew(&rgedit, NULL, &rgTilesetClass);
	if (AG_ObjectLoadFromFile(ts, path) == -1) {
		AG_TextMsgFromError();
		return;
	}
	AG_SetString(ts, "archive-path", path);
	CreateEditionWindow(ts);
}

static void
OpenTilesetDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fd;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Open tileset..."));
	fd = AG_FileDlgNewMRU(win, "rg-tilesets",
	    AG_FILEDLG_LOAD | AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, _("AgarPaint tileset"), "*.agt",
	    OpenTilesetAGT, NULL);
	AG_WindowShow(win);
}

static void
SaveTilesetToAGT(AG_Event *event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	char *path = AG_STRING(2);

	if (AG_ObjectSaveToFile(ts, path) == -1) {
		AG_TextMsgFromError();
		return;
	}
	AG_SetString(ts, "archive-path", path);
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
ExportTilesetToC(AG_Event *event)
{
	char pathData[AG_PATHNAME_MAX];
	char iconID[64];
	RG_Tileset *ts = RG_TILESET_PTR(1);
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
		AG_SetError(_("Unable to open %s"), path);
		AG_TextMsgFromError();
		return;
	}
	fprintf(f, "/* Agar icon reference file */\n");
	fprintf(f, "/* Generated by Agar rgedit %s */\n\n", VERSION);
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
		AG_SetError(_("Unable to open %s"), pathData);
		AG_TextMsgFromError();
		return;
	}
	fprintf(f, "/* Agar icon data file */\n");
	fprintf(f, "/* Generated by Agar rgedit %s */\n\n", VERSION);
	TAILQ_FOREACH(t, &ts->tiles, tiles) {
		Strlcpy(iconID, t->name, sizeof(iconID));
		TransformIconName(iconID);
		fprintf(f, "static const Uint32 %s%s_Data[%u] = {",
		    pkgName, iconID, t->su->w*t->su->h);
		src = t->su->pixels;
		for (h = 0; h < t->su->h; h++) {
			for (w = 0; w < t->su->w; w++) {
				Uint32 px;

				px = AG_SurfaceGet32_At(t->su, src);

				if (px == 0) {
					fprintf(f, "0,");
				} else {
					fprintf(f, "0x%lx,", (Ulong)px);
				}
				src += t->su->format.BytesPerPixel;
			}
		}
		fprintf(f, "};\n");
		fprintf(f, "AG_StaticIcon %s%s = {", pkgName, iconID);
		fprintf(f, "%lu,%lu,0x%.08lx,0x%.08lx,0x%.08lx,0x%.08lx,",
		    (unsigned long)t->su->w,
		    (unsigned long)t->su->h,
		    (unsigned long)t->su->format.Rmask,
		    (unsigned long)t->su->format.Gmask,
		    (unsigned long)t->su->format.Bmask,
		    (unsigned long)t->su->format.Amask);
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
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;
	AG_FileType *ft;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Save tileset as..."));
	fd = AG_FileDlgNewMRU(win, "rg-tilesets",
	    AG_FILEDLG_SAVE | AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND);
	AG_FileDlgSetOptionContainer(fd, AG_BoxNewVert(win, AG_BOX_HFILL));

	AG_FileDlgAddType(fd, _("AgarPaint tileset"), "*.agt",
	    SaveTilesetToAGT, "%p", ts);
	ft = AG_FileDlgAddType(fd, _("Agar icon header"), "*.h",
	    ExportTilesetToC, "%p", ts);
	AG_FileOptionNewString(ft, _("Package name: "), "pkg-name", "fooIcon");
	AG_FileOptionNewBool(ft, _("Use begin and close.h"), "begin-code", 1);
	AG_FileOptionNewBool(ft, _("Use __BEGIN_DECLS"), "cpp-decls", 1);

	AG_WindowShow(win);
}

static void
ImportImageFrom(AG_Event *event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_SurfaceFromFn *fn = AG_PTR(2);
	char *path = AG_STRING(3);
	AG_Surface *bmp;
	RG_Tile *t;
	RG_Pixmap *px;

	if ((bmp = (*fn)(path)) == NULL) {
		AG_TextMsgFromError();
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

/* Load image layers from Gimp 1.x XCF file. */
static void
ImportLayersFromXCF(AG_Event *event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	char *path = AG_STRING(2);
	AG_DataSource *ds;
	int rv;

	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		AG_TextMsgFromError();
		return;
	}
	rv = AG_XCFLoad(ds, 0, LoadTileFromXCF, ts);
	AG_CloseFile(ds);
}

static void
ImportImagesDlg(AG_Event *event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("Import images..."));
	fd = AG_FileDlgNewMRU(win, "rg-images",
	    AG_FILEDLG_LOAD | AG_FILEDLG_CLOSEWIN | AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, _("Windows bitmap"), "*.bmp",
	    ImportImageFrom, "%p,%p", ts, AG_SurfaceFromBMP);
	AG_FileDlgAddType(fd, _("JPEG image"), "*.jpg,*.jpeg",
	    ImportImageFrom, "%p,%p", ts, AG_SurfaceFromJPEG);
	AG_FileDlgAddType(fd, _("PNG image"), "*.png",
	    ImportImageFrom, "%p,%p", ts, AG_SurfaceFromJPEG);
	AG_FileDlgAddType(fd, _("Gimp 1 XCF image"), "*.xcf",
	    ImportLayersFromXCF, "%p", ts);
	AG_WindowShow(win);
}

static void
SaveTileset(AG_Event *event)
{
	RG_Tileset *ts = RG_TILESET_PTR(1);

	if (!AG_Defined(ts,"archive-path")) {
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
FileMenu(AG_Event *event)
{
	AG_MenuItem *m = AG_MENU_ITEM_PTR(1);

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
}

static void
Undo(AG_Event *event)
{
	printf("undo!\n");
}

static void
EditMenu(AG_Event *event)
{
	AG_MenuItem *m = AG_MENU_ITEM_PTR(1);
	
	if (tsFocused == NULL) { AG_MenuDisable(m); }

	AG_MenuActionKb(m, "Undo", NULL,
	    AG_KEY_Z, AG_KEYMOD_CTRL,
	    Undo, "%p", tsFocused);

	if (tsFocused == NULL) { AG_MenuEnable(m); }
}

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
	bindtextdomain("rgedit", LOCALEDIR);
	bind_textdomain_codeset("rgedit", "UTF-8");
	textdomain("rgedit");
#endif

	if (AG_InitCore("rgedit", AG_CREATE_DATADIR) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	while ((c = AG_Getopt(argc, argv, "?vDd:t:p:", &optArg, &optInd)) != -1) {
		switch (c) {
		case 'v':
			printf("rgedit %s\n", VERSION);
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
			printf("%s [-vD] [-d agar-driver-spec] "
			       "[-t font,size,flags]\n", agProgName);
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

	RG_InitSubsystem();

	AG_ObjectInit(&rgedit, NULL);
	rgedit.flags |= AG_OBJECT_STATIC;

	if (agDriverSw != NULL) {			/* Go MDI-style */
		appMenu = AG_MenuNewGlobal(0);
		AG_MenuDynamicItem(appMenu->root, _("File"), NULL, FileMenu, NULL);
		AG_MenuDynamicItem(appMenu->root, _("Edit"), NULL, EditMenu, NULL);
	}

	for (i = optInd; i < argc; i++) {
		ts = AG_ObjectNew(&rgedit, NULL, &rgTilesetClass);
		if (AG_ObjectLoadFromFile(ts, argv[i]) == 0) {
			AG_SetString(ts, "archive-path", argv[i]);
			CreateEditionWindow(ts);
			openedFiles++;
		} else {
			AG_TextMsgFromError();
			AG_ObjectDetach(ts);
			AG_ObjectDestroy(ts);
		}
	}
	if (!agDriverSw && !openedFiles) {
		ts = AG_ObjectNew(&rgedit, NULL, &rgTilesetClass);
		CreateEditionWindow(ts);
	}
	AG_EventLoop();
	AG_ConfigSave();
	AG_DestroyGraphics();
	AG_Destroy();
	return (0);
}
