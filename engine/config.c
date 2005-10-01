/*	$Csoft: config.c,v 1.154 2005/10/01 09:55:38 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <config/sharedir.h>
#include <config/ttfdir.h>
#include <config/have_getpwuid.h>
#include <config/have_getuid.h>
#include <config/have_freetype.h>

#include <compat/dir.h>

#include <engine/engine.h>
#include <engine/config.h>
#include <engine/view.h>
#include <engine/prop.h>
#include <engine/rcs.h>

#include <engine/map/map.h>
#ifdef EDITION
#include <engine/map/mapedit.h>
#endif

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>
#include <engine/widget/tlist.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/notebook.h>
#include <engine/widget/hsvpal.h>
#include <engine/widget/separator.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
#include <pwd.h>
#endif
#include <unistd.h>

const AG_Version agConfigVer = {
	"agar config",
	8, 1
};

const AG_ObjectOps agConfigOps = {
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

extern int agTextComposition;
extern int agTextBidi;
extern int agTextTabWidth;
extern int agWindowAnySize;
extern int agIdleThresh;
extern int agServerMode;
extern int agScreenshotQuality;

static void
set_path(int argc, union evarg *argv)
{
	char path[MAXPATHLEN];
	AG_Textbox *tbox = argv[0].p;
	char *varname = argv[1].s;

	AG_TextboxCopyString(tbox, path, sizeof(path));
	AG_SetString(agConfig, varname, "%s", path);
	AGWIDGET(tbox)->flags &= ~(AG_WIDGET_FOCUSED);
}

static void
set_full_screen(int argc, union evarg *argv)
{
	int enable = argv[1].i;
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
set_opengl(int argc, union evarg *argv)
{
	int enable = argv[1].i;

	if (enable)
		AG_TextMsg(AG_MSG_WARNING,
		    _("Save the configuration and restart %s for OpenGL mode "
		      "to take effect"),
		    agProgName);
}

static void
set_async_blits(int argc, union evarg *argv)
{
	int enable = argv[1].i;

	if (enable)
		AG_TextMsg(AG_MSG_WARNING,
		    _("Save the configuration and restart %s for async blits "
		      "to take effect"),
		    agProgName);
}

static void
set_unitrans(int argc, union evarg *argv)
{
	int enable = argv[1].i;

	if (SDL_EnableUNICODE(enable)) {
		dprintf("disabled unicode translation\n");
	} else {
		dprintf("enabled unicode translation\n");
	}
}

static void
save_config(int argc, union evarg *argv)
{
	if (AG_ObjectSave(agConfig) == -1)
		AG_TextMsg(AG_MSG_ERROR, "%s", AG_GetError());

	AG_TextTmsg(AG_MSG_INFO, 750,
	    _("Configuration settings saved successfully."));
}

void
AG_ConfigInit(AG_Config *cfg)
{
	char udatadir[MAXPATHLEN];
	struct passwd *pwd;
	struct stat sta;

	AG_ObjectInit(cfg, "object", "config", &agConfigOps);
	AGOBJECT(cfg)->flags |= AG_OBJECT_RELOAD_PROPS|AG_OBJECT_DATA_RESIDENT;
	AGOBJECT(cfg)->save_pfx = NULL;
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
	AG_SetUint(cfg, "view.nominal-fps", 60);
	AG_SetBool(cfg, "input.joysticks", 1);

	/* Set the save directory path and create it as needed. */
#if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	pwd = getpwuid(getuid());
	strlcpy(udatadir, pwd->pw_dir, sizeof(udatadir));
	strlcat(udatadir, "/.", sizeof(udatadir));
	strlcat(udatadir, agProgName, sizeof(udatadir));
#else
	udatadir[0] = '.';
	strlcpy(&udatadir[1], agProgName, sizeof(udatadir)-1);
#endif
	if (stat(udatadir, &sta) != 0 &&
	    Mkdir(udatadir) != 0) {
		fatal("%s: %s", udatadir, strerror(errno));
	}
	AG_SetString(cfg, "save-path", "%s", udatadir);

	AG_SetString(cfg, "den-path", "%s", SHAREDIR);
	AG_SetString(cfg, "load-path", "%s:%s", udatadir, SHAREDIR);

#if defined(__APPLE__)
	AG_SetString(cfg, "font-path", "%s/fonts:%s:%s/Library/Fonts:"
	                                  "/Library/Fonts:"
					  "/System/Library/Fonts",
					  udatadir, TTFDIR, pwd->pw_dir);
#elif defined(WIN32)
	AG_SetString(cfg, "font-path", "fonts:%s", TTFDIR);
#else
	AG_SetString(cfg, "font-path", "%s/fonts:%s", udatadir, TTFDIR);
#endif

#ifdef HAVE_FREETYPE
	AG_SetBool(cfg, "font-engine", 1);
# ifdef __APPLE__
	AG_SetString(cfg, "font-engine.default-font", "Geneva.dfont");
	AG_SetInt(cfg, "font-engine.default-size", 12);
	AG_SetInt(cfg, "font-engine.default-style", 0);
# else
	AG_SetString(cfg, "font-engine.default-font", "Vera.ttf");
	AG_SetInt(cfg, "font-engine.default-size", 11);
	AG_SetInt(cfg, "font-engine.default-style", 0);
# endif
#else
	AG_SetBool(cfg, "font-engine", 1);
	AG_SetString(cfg, "font-engine.default-font", "bitmap.xcf");
	AG_SetInt(cfg, "font-engine.default-size", -1);
	AG_SetInt(cfg, "font-engine.default-style", -1);
#endif /* HAVE_FREETYPE */
}

int
AG_ConfigLoad(void *p, AG_Netbuf *buf)
{
	if (AG_ReadVersion(buf, &agConfigVer, NULL) != 0)
		return (-1);

#ifdef DEBUG
	agDebugLvl = AG_ReadUint8(buf);
#else
	AG_ReadUint8(buf);
#endif
	agServerMode = AG_ReadUint8(buf);
	agIdleThresh = (int)AG_ReadUint8(buf);
	agWindowAnySize = AG_ReadUint8(buf);
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
	
	agRcsMode = (int)AG_ReadUint8(buf);
	AG_CopyString(agRcsHostname, buf, sizeof(agRcsHostname));
	agRcsPort = (u_int)AG_ReadUint16(buf);
	AG_CopyString(agRcsUsername, buf, sizeof(agRcsUsername));
	AG_CopyString(agRcsPassword, buf, sizeof(agRcsPassword));

	AG_MapEditorLoad(buf);
	return (0);
}

int
AG_ConfigSave(void *p, AG_Netbuf *buf)
{
	AG_WriteVersion(buf, &agConfigVer);

#ifdef DEBUG
	AG_WriteUint8(buf, (Uint8)agDebugLvl);
#else
	AG_WriteUint8(buf, 0);
#endif
	AG_WriteUint8(buf, agServerMode);
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

	AG_WriteUint8(buf, (Uint8)agRcsMode);
	AG_WriteString(buf, agRcsHostname);
	AG_WriteUint16(buf, (Uint16)agRcsPort);
	AG_WriteString(buf, agRcsUsername);
	AG_WriteString(buf, agRcsPassword);

	AG_MapEditorSave(buf);
	return (0);
}

static void
selected_color(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_HSVPal *hsv = argv[1].p;
	AG_TlistItem *it = argv[2].p;
	Uint32 *c = it->p1;

	AG_WidgetBind(hsv, "pixel", AG_WIDGET_UINT32, c);
}

static void
updated_bg(int argc, union evarg *argv)
{
	AG_HSVPal *hsv = argv[0].p;
	AG_Tlist *tl = argv[1].p;
	AG_TlistItem *it;
	Uint8 r, g, b;

	if ((it = AG_TlistSelectedItem(tl)) != NULL &&
	    it->p1 == &agColors[BG_COLOR]) {
#ifdef HAVE_OPENGL
		if (agView->opengl) {
			SDL_GetRGB(AG_COLOR(BG_COLOR), agVideoFmt, &r, &g, &b);
			glClearColor(r/255.0, g/255.0, b/255.0, 1.0);
		}
#endif
	}
}

void
AG_ShowSettings(void)
{
	if (!agConfig->window->visible) {
		AG_WindowShow(agConfig->window);
	} else {
		AG_WindowFocus(agConfig->window);
	}
}

void
AG_ConfigWindow(AG_Config *cfg, u_int flags)
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

	win = AG_WindowNew(0, "config-engine-settings");
	AG_WindowSetCaption(win, _("Agar settings"));

	nb = AG_NotebookNew(win, AG_NOTEBOOK_WFILL|AG_NOTEBOOK_HFILL);
	tab = AG_NotebookAddTab(nb, _("Video"), AG_BOX_VERT);
	{
		if (flags & AG_CONFIG_FULLSCREEN) {
			cbox = AG_CheckboxNew(tab, _("Full screen"));
			AG_WidgetBind(cbox, "state", AG_WIDGET_PROP, agConfig,
			    "view.full-screen");
			AG_SetEvent(cbox, "checkbox-changed", set_full_screen,
			    NULL);
		}

		cbox = AG_CheckboxNew(tab, _("Asynchronous blits"));
		AG_WidgetBind(cbox, "state", AG_WIDGET_PROP, agConfig,
		    "view.async-blits");
		AG_SetEvent(cbox, "checkbox-changed", set_async_blits, NULL);

		if (flags & AG_CONFIG_GL) {
			cbox = AG_CheckboxNew(tab, _("OpenGL mode"));
			AG_WidgetBind(cbox, "state", AG_WIDGET_PROP, agConfig,
			    "view.opengl");
			AG_SetEvent(cbox, "checkbox-changed", set_opengl, NULL);
		}
	
		if (flags & AG_CONFIG_RESOLUTION) {
			msb = AG_MSpinbuttonNew(tab, "x",
			    _("Default resolution: "));
			AG_WidgetBind(msb, "xvalue", AG_WIDGET_UINT16,
			    &agView->w);
			AG_WidgetBind(msb, "yvalue", AG_WIDGET_UINT16,
			    &agView->h);
			AG_MSpinbuttonSetRange(msb, 320, 4096);
		}
		
		sbu = AG_SpinbuttonNew(tab, _("Screenshot quality (%%): "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_INT,
		    &agScreenshotQuality);
		AG_SpinbuttonSetMin(sbu, 1);
		AG_SpinbuttonSetMax(sbu, 100);
	
		sbu = AG_SpinbuttonNew(tab, _("Idling threshold (ms): "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &agIdleThresh);
		AG_SpinbuttonSetMin(sbu, 0);
		AG_SpinbuttonSetMax(sbu, 255);
		
		cbox = AG_CheckboxNew(tab, _("Unrestricted window resize"));
		AG_WidgetBind(cbox, "state", AG_WIDGET_INT, &agWindowAnySize);
	}

	tab = AG_NotebookAddTab(nb, _("Input devices"), AG_BOX_VERT);
	{
		cbox = AG_CheckboxNew(tab, _("Unicode keyboard translation"));
		AG_WidgetBind(cbox, "state", AG_WIDGET_INT, &agKbdUnicode);
		AG_SetEvent(cbox, "checkbox-changed", set_unitrans, NULL);

		cbox = AG_CheckboxNew(tab, _("Input composition"));
		AG_WidgetBind(cbox, "state", AG_WIDGET_INT, &agTextComposition);

		cbox = AG_CheckboxNew(tab,
		    _("Right->left (Arabic, Hebrew, ...)"));
		AG_WidgetBind(cbox, "state", AG_WIDGET_INT, &agTextBidi);
		
		sbu = AG_SpinbuttonNew(tab,
		    _("Mouse double click delay (ms): "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_INT,
		    &agMouseDblclickDelay);
		AG_SpinbuttonSetMin(sbu, 1);
		
		sbu = AG_SpinbuttonNew(tab, _("Mouse spin delay (ms): "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &agMouseSpinDelay);
		AG_SpinbuttonSetMin(sbu, 1);

		sbu = AG_SpinbuttonNew(tab, _("Mouse spin interval (ms): "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &agMouseSpinIval);
		AG_SpinbuttonSetMin(sbu, 1);

		sbu = AG_SpinbuttonNew(tab, _("Keyboard repeat delay (ms): "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &agKbdDelay);
		AG_SpinbuttonSetMin(sbu, 1);
		
		sbu = AG_SpinbuttonNew(tab,
		    _("Keyboard repeat interval (ms): "));
		AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &agKbdRepeat);
		AG_SpinbuttonSetMin(sbu, 1);
	}

	if (flags & AG_CONFIG_DIRECTORIES) {
		char path[MAXPATHLEN];

		tab = AG_NotebookAddTab(nb, _("Directories"), AG_BOX_VERT);

		tbox = AG_TextboxNew(tab, _("Data save dir: "));
		AG_StringCopy(agConfig, "save-path", path, sizeof(path));
		AG_TextboxPrintf(tbox, "%s", path);
		AG_SetEvent(tbox, "textbox-return", set_path, "%s",
		    "save-path");
	
		tbox = AG_TextboxNew(tab, _("Data load path: "));
		AG_StringCopy(agConfig, "load-path", path, sizeof(path));
		AG_TextboxPrintf(tbox, "%s", path);
		AG_SetEvent(tbox, "textbox-return", set_path, "%s",
		    "load-path");
	
		tbox = AG_TextboxNew(tab, _("Font path: "));
		AG_StringCopy(agConfig, "font-path", path, sizeof(path));
		AG_TextboxPrintf(tbox, "%s", path);
		AG_SetEvent(tbox, "textbox-return", set_path, "%s",
		    "font-path");
		
		tbox = AG_TextboxNew(tab, _("Den path: "));
		AG_StringCopy(agConfig, "den-path", path, sizeof(path));
		AG_TextboxPrintf(tbox, "%s", path);
		AG_SetEvent(tbox, "textbox-return", set_path, "%s", "den-path");
	}
	
	tab = AG_NotebookAddTab(nb, _("Colors"), AG_BOX_HORIZ);
	{
		AG_HSVPal *hsv;
		AG_Tlist *tl;
		AG_TlistItem *it;
		int i;

		tl = AG_TlistNew(tab, 0);
		AGWIDGET(tl)->flags &= ~AG_WIDGET_WFILL;
		for (i = 0; i < LAST_COLOR; i++) {
			it = AG_TlistAdd(tl, NULL, _(agColorNames[i]));
			it->p1 = &agColors[i];
		}
		for (i = 0; i < agColorsBorderSize; i++) {
			it = AG_TlistAdd(tl, NULL, _("Window border #%i"),
			    i);
			it->p1 = &agColorsBorder[i];
		}

		hsv = AG_HSVPalNew(tab);
		AGWIDGET(hsv)->flags |= AG_WIDGET_WFILL|AG_WIDGET_HFILL;
		AG_WidgetBind(hsv, "pixel-format", AG_WIDGET_POINTER,
		    &agVideoFmt);
		AG_SetEvent(hsv, "h-changed", updated_bg, "%p", tl);
		AG_SetEvent(hsv, "sv-changed", updated_bg, "%p", tl);
		AG_SetEvent(tl, "tlist-selected", selected_color, "%p", hsv);
	}

#ifdef NETWORK
	tab = AG_NotebookAddTab(nb, _("RCS"), AG_BOX_VERT);
	{
		AG_Textbox *tb;
		AG_Spinbutton *sb;
		AG_Box *box;
		AG_Checkbox *cb;

		cb = AG_CheckboxNew(tab, _("Enable RCS"));
		AG_WidgetBind(cb, "state", AG_WIDGET_INT, &agRcsMode);

		tb = AG_TextboxNew(tab, _("Server hostname: "));
		AG_WidgetBind(tb, "string", AG_WIDGET_STRING, agRcsHostname,
		    sizeof(agRcsHostname));
	
		sb = AG_SpinbuttonNew(tab, _("Server port: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_UINT, &agRcsPort);

		AG_SeparatorNew(tab, AG_SEPARATOR_HORIZ);

		box = AG_BoxNew(tab, AG_BOX_HORIZ, AG_BOX_WFILL|
				                   AG_BOX_HOMOGENOUS);
		{
			tb = AG_TextboxNew(box, _("Username: "));
			AG_WidgetBind(tb, "string", AG_WIDGET_STRING,
			    agRcsUsername, sizeof(agRcsUsername));

			tb = AG_TextboxNew(box, _("Password: "));
			AG_TextboxSetPassword(tb, 1);
			AG_WidgetBind(tb, "string", AG_WIDGET_STRING,
			    agRcsPassword, sizeof(agRcsPassword));
		}
	}
#endif /* NETWORK */

#ifdef DEBUG
	tab = AG_NotebookAddTab(nb, _("Debug"), AG_BOX_VERT);
	{
		cbox = AG_CheckboxNew(tab, _("Debugging"));
		AG_WidgetBind(cbox, "state", AG_WIDGET_INT, &agDebugLvl);

		cbox = AG_CheckboxNew(tab, _("Debug server mode"));
		AG_WidgetBind(cbox, "state", AG_WIDGET_INT, &agServerMode);
	}
#endif

	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_WFILL);
	{
		AG_ButtonAct(hb, _("Close"), 0, AGWINHIDE(win));
		AG_ButtonAct(hb, _("Save"), 0, save_config, NULL);
	}
	agConfig->window = win;
}

/* Copy the full pathname to a data file to a fixed-size buffer. */
int
AG_ConfigFile(const char *path_key, const char *name, const char *ext,
    char *path, size_t path_len)
{
	char file[MAXPATHLEN];
	struct stat sta;
	char *dir, *pathp = path;

	AG_StringCopy(agConfig, path_key, path, path_len);

	for (dir = strsep(&pathp, ":");
	     dir != NULL;
	     dir = strsep(&pathp, ":")) {
		strlcpy(file, dir, sizeof(file));

		if (name[0] != '/') {
			strlcat(file, "/", sizeof(file));
		}
		strlcat(file, name, sizeof(file));
		if (ext != NULL) {
			strlcat(file, ".", sizeof(file));
			strlcat(file, ext, sizeof(file));
		}
		if (stat(file, &sta) == 0) {
			if (strlcpy(path, file, path_len) >= path_len) {
				AG_SetError(_("The search path is too big."));
				return (-1);
			}
			return (0);
		}
	}
	AG_StringCopy(agConfig, path_key, path, path_len);
	AG_SetError(_("Cannot find %s.%s (in <%s>:%s)."), name,
	    ext != NULL ? ext : "", path_key, path);
	return (-1);
}

