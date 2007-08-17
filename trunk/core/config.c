/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Global configuration object. Used for Agar-related settings, but
 * applications are free to insert additional properties into the
 * object.
 */

#include <config/sharedir.h>
#include <config/ttfdir.h>
#include <config/have_getpwuid.h>
#include <config/have_getuid.h>
#include <config/have_freetype.h>

#include <core/core.h>
#include <core/config.h>
#include <core/view.h>
#include <core/rcs.h>

#include <compat/dir.h>
#include <compat/file.h>

#include <gui/window.h>
#include <gui/vbox.h>
#include <gui/hbox.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/checkbox.h>
#include <gui/textbox.h>
#include <gui/keycodes.h>
#include <gui/tlist.h>
#include <gui/mspinbutton.h>
#include <gui/spinbutton.h>
#include <gui/notebook.h>
#include <gui/hsvpal.h>
#include <gui/separator.h>
#include <gui/file_dlg.h>
#include <gui/pane.h>

#include <stdlib.h>
#include <string.h>
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
#include <pwd.h>
#endif
#include <unistd.h>

const AG_ObjectOps agConfigOps = {
	"AG_Config",
	sizeof(AG_Config),
	{ 9, 3 },
	NULL,
	NULL,
	NULL,
	AG_ConfigLoad,
	AG_ConfigSave,
	NULL
};

int agKbdUnicode = 1;				/* Unicode translation */
int agKbdDelay = 250;				/* Key repeat delay */
int agKbdRepeat = 35;				/* Key repeat interval */
int agMouseDblclickDelay = 250;			/* Mouse double-click delay */
int agMouseSpinDelay = 250;			/* Spinbutton repeat delay */
int agMouseSpinIval = 50;			/* Spinbutton repeat interval */

extern int agTextAntialiasing;
extern int agTextComposition;
extern int agTextBidi;
extern int agTextTabWidth;
extern int agIdleThresh;
extern int agScreenshotQuality;
extern int agWindowAnySize;

static void
Set_SearchPath(AG_Event *event)
{
	char path[MAXPATHLEN];
	AG_Textbox *tbox = AG_SELF();
	char *varname = AG_STRING(1);

	AG_TextboxCopyString(tbox, path, sizeof(path));
	AG_SetString(agConfig, varname, "%s", path);
	WIDGET(tbox)->flags &= ~(AG_WIDGET_FOCUSED);
}

static void
Set_Fullscreen(AG_Event *event)
{
	int enable = AG_INT(1);
	SDL_Event vexp;

	if (agView == NULL)
		return;

	if ((enable && (agView->v->flags & SDL_FULLSCREEN) == 0) ||
	   (!enable && (agView->v->flags & SDL_FULLSCREEN))) {
		SDL_WM_ToggleFullScreen(agView->v);
		vexp.type = SDL_VIDEOEXPOSE;
		SDL_PushEvent(&vexp);
	}
}

static void
Set_UnicodeKbd(AG_Event *event)
{
	int enable = AG_INT(1);

	if (SDL_EnableUNICODE(enable)) {
		dprintf("disabled unicode translation\n");
	} else {
		dprintf("enabled unicode translation\n");
	}
}

static void
WarnRestart(AG_Event *event)
{
	char *key = AG_STRING(1);

	AG_TextWarning(key,
	    _("Note: Save the configuration and restart %s "
	      "for this change to take effect"), agProgName);
}

static void
SaveConfig(AG_Event *event)
{
	if (AG_ObjectSave(agConfig) == -1) {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
	} else {
		AG_TextTmsg(AG_MSG_INFO, 750,
		    _("Configuration settings saved successfully."));
	}
}

void
AG_ConfigInit(AG_Config *cfg)
{
	char udatadir[MAXPATHLEN];
	char tmpdir[MAXPATHLEN];
	struct passwd *pwd;

	AG_ObjectInit(cfg, "config", &agConfigOps);
	OBJECT(cfg)->flags |= AG_OBJECT_RELOAD_PROPS|AG_OBJECT_RESIDENT;
	OBJECT(cfg)->save_pfx = NULL;
	cfg->window = NULL;

	AG_SetBool(cfg, "initial-run", 1);
	AG_SetBool(cfg, "view.full-screen", 0);
	AG_SetBool(cfg, "view.async-blits", 0);
	AG_SetBool(cfg, "view.opengl", 0);
	AG_SetUint16(cfg, "view.w", 800);
	AG_SetUint16(cfg, "view.h", 600);
	AG_SetUint16(cfg, "view.min-w", 320);
	AG_SetUint16(cfg, "view.min-h", 240);
	AG_SetUint8(cfg, "view.depth", 32);
	AG_SetUint(cfg, "view.nominal-fps", 40);
	AG_SetBool(cfg, "input.joysticks", 1);

	/* Set the save directory path and create it as needed. */
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	pwd = getpwuid(getuid());
	strlcpy(udatadir, pwd->pw_dir, sizeof(udatadir));
	strlcat(udatadir, AG_PATHSEP, sizeof(udatadir));
	strlcat(udatadir, ".", sizeof(udatadir));
	strlcat(udatadir, agProgName, sizeof(udatadir));
#else
	udatadir[0] = '.';
	strlcpy(&udatadir[1], agProgName, sizeof(udatadir)-1);
#endif
	strlcpy(tmpdir, udatadir, sizeof(tmpdir));
	strlcat(tmpdir, "/tmp", sizeof(tmpdir));
	
	if (AG_FileExists(udatadir) == 0 && AG_MkDir(udatadir) != 0)
		fatal("%s: %s", udatadir, AG_GetError());
	if (AG_FileExists(tmpdir) == 0 && AG_MkDir(tmpdir) != 0)
		fatal("%s: %s", tmpdir, AG_GetError());
	
	AG_SetString(cfg, "save-path", "%s", udatadir);
	AG_SetString(cfg, "tmp-path", "%s", tmpdir);

#if defined(WIN32)
	AG_SetString(cfg, "den-path", ".");
	AG_SetString(cfg, "load-path", "%s:.", udatadir);
#else
	AG_SetString(cfg, "den-path", "%s", SHAREDIR);
	AG_SetString(cfg, "load-path", "%s:%s", udatadir, SHAREDIR);
#endif
	
#if defined(__APPLE__)
	AG_SetString(cfg, "font-path", "%s/fonts:%s:%s/Library/Fonts:"
	                                  "/Library/Fonts:"
					  "/System/Library/Fonts",
					  udatadir, TTFDIR, pwd->pw_dir);
#elif defined(WIN32)
	AG_SetString(cfg, "font-path", "fonts:.");
#else
	AG_SetString(cfg, "font-path", "%s/fonts:%s", udatadir, TTFDIR);
#endif

#ifdef HAVE_FREETYPE
	AG_SetBool(cfg, "font.freetype", 1);
#else
	AG_SetBool(cfg, "font.freetype", 0);
#endif
	AG_SetString(cfg, "font.face", "?");
	AG_SetInt(cfg, "font.size", -1);
	AG_SetUint(cfg, "font.flags", 0);
}

int
AG_ConfigLoad(void *p, AG_Netbuf *buf)
{
	AG_Version ver;

	if (AG_ReadVersion(buf, agConfigOps.type, &agConfigOps.ver, &ver) != 0)
		return (-1);

#ifdef DEBUG
	agDebugLvl = AG_ReadUint8(buf);
#else
	AG_ReadUint8(buf);
#endif
	if (ver.minor < 2) { AG_ReadUint8(buf); } /* agServerMode */
	agIdleThresh = (int)AG_ReadUint8(buf);
	if (ver.minor >= 3) { agWindowAnySize = (int)AG_ReadUint8(buf); }
	agTextComposition = AG_ReadUint8(buf);
	agTextBidi = AG_ReadUint8(buf);
	agKbdUnicode = AG_ReadUint8(buf);
	agKbdDelay = (int)AG_ReadUint32(buf);
	agKbdRepeat = (int)AG_ReadUint32(buf);
	agMouseDblclickDelay = (int)AG_ReadUint32(buf);
	agMouseSpinDelay = (int)AG_ReadUint16(buf);
	agMouseSpinIval = (int)AG_ReadUint16(buf);
	agScreenshotQuality = (int)AG_ReadUint8(buf);
	agTextTabWidth = (int)AG_ReadUint16(buf);
	if (ver.minor >= 1) {
		agTextAntialiasing = AG_ReadUint8(buf);
	}

	agRcsMode = (int)AG_ReadUint8(buf);
	AG_CopyString(agRcsHostname, buf, sizeof(agRcsHostname));
	agRcsPort = (Uint)AG_ReadUint16(buf);
	AG_CopyString(agRcsUsername, buf, sizeof(agRcsUsername));
	AG_CopyString(agRcsPassword, buf, sizeof(agRcsPassword));
	return (0);
}

int
AG_ConfigSave(void *p, AG_Netbuf *buf)
{
	AG_WriteVersion(buf, agConfigOps.type, &agConfigOps.ver);

#ifdef DEBUG
	AG_WriteUint8(buf, (Uint8)agDebugLvl);
#else
	AG_WriteUint8(buf, 0);
#endif
	AG_WriteUint8(buf, (Uint8)agIdleThresh);
	AG_WriteUint8(buf, (Uint8)agWindowAnySize);
	AG_WriteUint8(buf, (Uint8)agTextComposition);
	AG_WriteUint8(buf, (Uint8)agTextBidi);
	AG_WriteUint8(buf, (Uint8)agKbdUnicode);
	AG_WriteUint32(buf, (Uint32)agKbdDelay);
	AG_WriteUint32(buf, (Uint32)agKbdRepeat);
	AG_WriteUint32(buf, (Uint32)agMouseDblclickDelay);
	AG_WriteUint16(buf, (Uint16)agMouseSpinDelay);
	AG_WriteUint16(buf, (Uint16)agMouseSpinIval);
	AG_WriteUint8(buf, (Uint8)agScreenshotQuality);
	AG_WriteUint16(buf, (Uint16)agTextTabWidth);
	AG_WriteUint8(buf, (Uint8)agTextAntialiasing);

	AG_WriteUint8(buf, (Uint8)agRcsMode);
	AG_WriteString(buf, agRcsHostname);
	AG_WriteUint16(buf, (Uint16)agRcsPort);
	AG_WriteString(buf, agRcsUsername);
	AG_WriteString(buf, agRcsPassword);

	if (AG_String(agConfig, "save-path") != NULL) {
		char path[MAXPATHLEN];

		strlcpy(path, AG_String(agConfig, "save-path"), sizeof(path));
		strlcat(path, AG_PATHSEP, sizeof(path));
		strlcat(path, "gui-colors.acs", sizeof(path));
		if (AG_ColorsSave(path) == -1)
			fprintf(stderr, "%s\n", AG_GetError());
	}
	return (0);
}

static void
BindSelectedColor(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_HSVPal *hsv = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);
	Uint32 *c = it->p1;

	AG_WidgetBind(hsv, "pixel", AG_WIDGET_UINT32, c);
}

static void
Set_Color(AG_Event *event)
{
	AG_HSVPal *hsv = AG_SELF();
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	Uint8 r, g, b;

	if ((it = AG_TlistSelectedItem(tl)) != NULL &&
	    it->p1 == &agColors[BG_COLOR]) {
#ifdef HAVE_OPENGL
		if (agView->opengl) {
			SDL_GetRGB(AG_COLOR(BG_COLOR), agVideoFmt, &r, &g, &b);
			AG_LockGL();
			glClearColor(r/255.0, g/255.0, b/255.0, 1.0);
			AG_UnlockGL();
		} else
#endif
		{
			SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
			AG_WidgetDraw(AG_WidgetParentWindow(hsv));
			SDL_UpdateRect(agView->v, 0, 0, agView->w, agView->h);
		}
	}
}

void
AG_ShowSettings(void)
{
	if (agView->nModal > 0)		/* Avoid clobbering modal windows */
		return;

	if (!agConfig->window->visible) {
		AG_WindowShow(agConfig->window);
	} else {
		AG_WindowFocus(agConfig->window);
	}
}

static void
AG_LoadColorSchemeFromACS(AG_Event *event)
{
	char *file = AG_STRING(1);

	if (AG_ColorsLoad(file) == 0) {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Color scheme loaded from %s."), file);
	} else {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
	}
}

static void
AG_SaveColorSchemeToACS(AG_Event *event)
{
	char *file = AG_STRING(1);

	if (AG_ColorsSave(file) == 0) {
		AG_TextTmsg(AG_MSG_INFO, 1000, _("Color scheme saved to %s."),
		    file);
	} else {
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());
	}
}

static void
AG_LoadColorSchemeDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fd;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Load color scheme..."));
	fd = AG_FileDlgNew(win, AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgSetDirectory(fd, AG_String(agConfig, "save-path"));
	AG_FileDlgSetFilename(fd, "colors.acs");
	AG_FileDlgAddType(fd, _("Agar Color Scheme"), "*.acs",
	    AG_LoadColorSchemeFromACS, NULL);
	AG_WindowShow(win);
}

static void
AG_SaveColorSchemeDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fd;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Load color scheme..."));
	fd = AG_FileDlgNew(win, AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgSetDirectory(fd, AG_String(agConfig, "save-path"));
	AG_FileDlgAddType(fd, _("Agar Color Scheme"), "*.acs",
	    AG_SaveColorSchemeToACS, NULL);
	AG_WindowShow(win);
}

void
AG_ConfigWindow(AG_Config *cfg, Uint flags)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_HBox *hb;
	AG_Textbox *tbox;
	AG_Checkbox *cbox;
	AG_Notebook *nb;
	AG_NotebookTab *tab;
	AG_MSpinbutton *msb;
	AG_Spinbutton *sbu;

	win = AG_WindowNewNamed(0, "config-engine-settings");
	AG_WindowSetCaption(win, _("Agar settings"));

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	tab = AG_NotebookAddTab(nb, _("Video"), AG_BOX_VERT);
	{
		if (flags & AG_CONFIG_FULLSCREEN) {
			cbox = AG_CheckboxNew(tab, 0, _("Full screen"));
			AG_WidgetBind(cbox, "state", AG_WIDGET_PROP, agConfig,
			    "view.full-screen");
			AG_SetEvent(cbox, "checkbox-changed", Set_Fullscreen,
			    NULL);
		}

		cbox = AG_CheckboxNew(tab, 0, _("Asynchronous blits"));
		AG_WidgetBind(cbox, "state", AG_WIDGET_PROP, agConfig,
		    "view.async-blits");
		AG_SetEvent(cbox, "checkbox-changed", WarnRestart, "%s",
		    "config.view.async-blits");

		if (flags & AG_CONFIG_GL) {
			cbox = AG_CheckboxNew(tab, 0, _("OpenGL mode"));
			AG_WidgetBind(cbox, "state", AG_WIDGET_PROP, agConfig,
			    "view.opengl");
			AG_SetEvent(cbox, "checkbox-changed", WarnRestart, "%s",
			    "config.view.opengl");
		}
#if 0
		if (flags & AG_CONFIG_RESOLUTION) {
			msb = AG_MSpinbuttonNew(tab, 0, "x", _("Resolution: "));
			AG_WidgetBind(msb, "xvalue", AG_WIDGET_UINT16,
			    &agView->w);
			AG_WidgetBind(msb, "yvalue", AG_WIDGET_UINT16,
			    &agView->h);
			AG_MSpinbuttonSetRange(msb, 320, 4096);
		}
#endif
		AG_SeparatorNewHorizInv(tab);

		sbu = AG_SpinbuttonNew(tab, 0, _("Screenshot quality (%): "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_INT,
		    &agScreenshotQuality);
		AG_SpinbuttonSetMin(sbu, 1);
		AG_SpinbuttonSetMax(sbu, 100);
	
		sbu = AG_SpinbuttonNew(tab, 0, _("Idling threshold (ms): "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &agIdleThresh);
		AG_SpinbuttonSetMin(sbu, 0);
		AG_SpinbuttonSetMax(sbu, 255);
	}

	tab = AG_NotebookAddTab(nb, _("GUI"), AG_BOX_VERT);
	{
		cbox = AG_CheckboxNew(tab, 0, _("Antialiased text rendering"));
		AG_WidgetBindInt(cbox, "state", &agTextAntialiasing);
		AG_SetEvent(cbox, "checkbox-changed", WarnRestart, "%s",
		    "config.text.antialiasing");
		
		cbox = AG_CheckboxNew(tab, 0, _("Allow any window size"));
		AG_WidgetBindInt(cbox, "state", &agWindowAnySize);
		
		AG_SeparatorNewHorizInv(tab);

		cbox = AG_CheckboxNew(tab, 0, _("Unicode keyboard input"));
		AG_WidgetBindInt(cbox, "state", &agKbdUnicode);
		AG_SetEvent(cbox, "checkbox-changed", Set_UnicodeKbd, NULL);

		cbox = AG_CheckboxNew(tab, 0, _("Built-in key composition"));
		AG_WidgetBindInt(cbox, "state", &agTextComposition);

		cbox = AG_CheckboxNew(tab, 0, _("Edit text left to right"));
		AG_WidgetBindInt(cbox, "state", &agTextBidi);
		
		AG_SeparatorNewHorizInv(tab);
		
		sbu = AG_SpinbuttonNew(tab, 0, _("Double click delay (ms): "));
		AG_WidgetBindInt(sbu, "value", &agMouseDblclickDelay);
		AG_SpinbuttonSetMin(sbu, 1);
		
		sbu = AG_SpinbuttonNew(tab, 0, _("Mouse spin delay (ms): "));
		AG_WidgetBindInt(sbu, "value", &agMouseSpinDelay);
		AG_SpinbuttonSetMin(sbu, 1);

		sbu = AG_SpinbuttonNew(tab, 0, _("Mouse spin interval (ms): "));
		AG_WidgetBindInt(sbu, "value", &agMouseSpinIval);
		AG_SpinbuttonSetMin(sbu, 1);

		sbu = AG_SpinbuttonNew(tab, 0,
		    _("Keyboard repeat delay (ms): "));
		AG_WidgetBindInt(sbu, "value", &agKbdDelay);
		AG_SpinbuttonSetMin(sbu, 1);
		
		sbu = AG_SpinbuttonNew(tab, 0,
		    _("Keyboard repeat interval (ms): "));
		AG_WidgetBindInt(sbu, "value", &agKbdRepeat);
		AG_SpinbuttonSetMin(sbu, 1);
	}

	if (flags & AG_CONFIG_DIRECTORIES) {
		char path[MAXPATHLEN];

		tab = AG_NotebookAddTab(nb, _("Directories"), AG_BOX_VERT);

		tbox = AG_TextboxNew(tab, AG_TEXTBOX_HFILL,
		    _("Data save dir: "));
		AG_StringCopy(agConfig, "save-path", path, sizeof(path));
		AG_TextboxPrintf(tbox, "%s", path);
		AG_SetEvent(tbox, "textbox-return", Set_SearchPath, "%s",
		    "save-path");
	
		tbox = AG_TextboxNew(tab, AG_TEXTBOX_HFILL,
		    _("Data load path: "));
		AG_StringCopy(agConfig, "load-path", path, sizeof(path));
		AG_TextboxPrintf(tbox, "%s", path);
		AG_SetEvent(tbox, "textbox-return", Set_SearchPath, "%s",
		    "load-path");
	
		tbox = AG_TextboxNew(tab, AG_TEXTBOX_HFILL, _("Font path: "));
		AG_StringCopy(agConfig, "font-path", path, sizeof(path));
		AG_TextboxPrintf(tbox, "%s", path);
		AG_SetEvent(tbox, "textbox-return", Set_SearchPath, "%s",
		    "font-path");
		
		tbox = AG_TextboxNew(tab, AG_TEXTBOX_HFILL, _("Den path: "));
		AG_StringCopy(agConfig, "den-path", path, sizeof(path));
		AG_TextboxPrintf(tbox, "%s", path);
		AG_SetEvent(tbox, "textbox-return", Set_SearchPath, "%s",
		    "den-path");
	}
	
	tab = AG_NotebookAddTab(nb, _("Colors"), AG_BOX_VERT);
	{
		AG_Pane *hPane;
		AG_HSVPal *hsv;
		AG_Tlist *tl;
		AG_TlistItem *it;
		AG_Label *lbl;
		int i;
	
		hPane = AG_PaneNew(tab, AG_PANE_HORIZ, AG_PANE_EXPAND);
		{
			tl = AG_TlistNew(hPane->div[0], AG_TLIST_EXPAND);
			AG_TlistPrescale(tl, "Tileview text background", 10);
			for (i = 0; i < LAST_COLOR; i++) {
				it = AG_TlistAdd(tl, NULL, _(agColorNames[i]));
				it->p1 = &agColors[i];
			}
			for (i = 0; i < agColorsBorderSize; i++) {
				it = AG_TlistAdd(tl, NULL,
				    _("Window border #%i"), i);
				it->p1 = &agColorsBorder[i];
			}

			hsv = AG_HSVPalNew(hPane->div[1], AG_HSVPAL_EXPAND);
			AG_WidgetBind(hsv, "pixel-format", AG_WIDGET_POINTER,
			    &agVideoFmt);
			AG_SetEvent(hsv, "h-changed", Set_Color, "%p", tl);
			AG_SetEvent(hsv, "sv-changed", Set_Color, "%p", tl);
			AG_SetEvent(tl, "tlist-selected", BindSelectedColor,
			    "%p", hsv);
		}
		
		lbl = AG_LabelNewStatic(tab, 0,
		    _("Warning: Some color changes will not "
		      "take effect until %s is restarted."), agProgName);
		AG_LabelSetPaddingLeft(lbl, 10);
		AG_LabelSetPaddingRight(lbl, 10);
		
		hb = AG_HBoxNew(tab, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL);
		{
			AG_ButtonNewFn(hb, 0, _("Load scheme"),
			    AG_LoadColorSchemeDlg, NULL);
			AG_ButtonNewFn(hb, 0, _("Save scheme"),
			    AG_SaveColorSchemeDlg, NULL);
		}
	}

#ifdef NETWORK
	tab = AG_NotebookAddTab(nb, _("RCS"), AG_BOX_VERT);
	{
		AG_Textbox *tb;
		AG_Spinbutton *sb;
		AG_Box *box;
		AG_Checkbox *cb;

		cb = AG_CheckboxNew(tab, 0, _("Enable RCS"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT, &agRcsMode);

		AG_SeparatorNewHorizInv(tab);

		tb = AG_TextboxNew(tab, AG_TEXTBOX_HFILL,
		    _("Server hostname: "));
		AG_WidgetBind(tb, "string", AG_WIDGET_STRING, agRcsHostname,
		    sizeof(agRcsHostname));
	
		sb = AG_SpinbuttonNew(tab, 0, _("Server port: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT, &agRcsPort);

		AG_SeparatorNewHoriz(tab);

		box = AG_BoxNew(tab, AG_BOX_HORIZ, AG_BOX_HFILL|
				                   AG_BOX_HOMOGENOUS);
		{
			tb = AG_TextboxNew(box, AG_TEXTBOX_HFILL,
			    _("Username: "));
			AG_WidgetBind(tb, "string", AG_WIDGET_STRING,
			    agRcsUsername, sizeof(agRcsUsername));

			tb = AG_TextboxNew(box, AG_TEXTBOX_HFILL,
			    _("Password: "));
			AG_TextboxSetPassword(tb, 1);
			AG_WidgetBind(tb, "string", AG_WIDGET_STRING,
			    agRcsPassword, sizeof(agRcsPassword));
		}
	}
#endif /* NETWORK */

#ifdef DEBUG
	tab = AG_NotebookAddTab(nb, _("Debug"), AG_BOX_VERT);
	{
		cbox = AG_CheckboxNew(tab, 0, _("Enable debugging"));
		AG_WidgetBind(cbox, "state", AG_WIDGET_INT, &agDebugLvl);
	}
#endif

	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL);
	{
		AG_ButtonNewFn(hb, 0, _("Close"), AGWINHIDE(win));
		AG_ButtonNewFn(hb, 0, _("Save"), SaveConfig, NULL);
	}
	agConfig->window = win;
}

/* Copy the full pathname of a data file to a sized buffer. */
int
AG_ConfigFile(const char *path_key, const char *name, const char *ext,
    char *path, size_t path_len)
{
	char file[MAXPATHLEN];
	char *dir, *pathp = path;
	int rv;

	AG_StringCopy(agConfig, path_key, path, path_len);

	for (dir = AG_Strsep(&pathp, ":");
	     dir != NULL;
	     dir = AG_Strsep(&pathp, ":")) {
		strlcpy(file, dir, sizeof(file));

		if (name[0] != AG_PATHSEPC) {
			strlcat(file, AG_PATHSEP, sizeof(file));
		}
		strlcat(file, name, sizeof(file));
		if (ext != NULL) {
			strlcat(file, ".", sizeof(file));
			strlcat(file, ext, sizeof(file));
		}
		if ((rv = AG_FileExists(file)) == 1) {
			if (strlcpy(path, file, path_len) >= path_len) {
				AG_SetError(_("The search path is too big."));
				return (-1);
			}
			return (0);
		} else if (rv == -1) {
			AG_SetError("%s: %s", file, AG_GetError());
			return (-1);
		}
	}
	AG_StringCopy(agConfig, path_key, path, path_len);
	AG_SetError(_("Cannot find %s.%s (in <%s>:%s)."), name,
	    (ext != NULL) ? ext : "", path_key, path);
	return (-1);
}

