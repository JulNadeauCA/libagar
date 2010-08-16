/*
 * Copyright (c) 2002-2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Generic configuration settings dialog.
 */

#include <core/core.h>
#include <core/config.h>
#include <core/rcs.h>

#include <gui/window.h>
#include <gui/hbox.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/checkbox.h>
#include <gui/textbox.h>
#include <gui/tlist.h>
#include <gui/mspinbutton.h>
#include <gui/notebook.h>
#include <gui/numerical.h>
#include <gui/hsvpal.h>
#include <gui/separator.h>
#include <gui/file_dlg.h>
#include <gui/pane.h>

#include "dev.h"

static AG_Window *DEV_ConfigWindow(AG_Config *);

static void
SetPath(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_Textbox *tbox = AG_SELF();
	char *varname = AG_STRING(1);

	AG_TextboxCopyString(tbox, path, sizeof(path));
	AG_SetString(agConfig, varname, path);
	AG_WidgetUnfocus(tbox);
}

#if 0
static void
WarnRestart(AG_Event *event)
{
	char *key = AG_STRING(1);

	AG_TextWarning(key,
	    _("Note: Save the configuration and restart %s "
	      "for this change to take effect"), agProgName);
}
#endif

static void
BindSelectedColor(AG_Event *event)
{
	AG_HSVPal *hsv = AG_PTR(1);
	AG_TlistItem *it = AG_PTR(2);
	AG_Color *c = it->p1;

	AG_BindUint8(hsv, "RGBAv", (Uint8 *)c);
}

/* Must be invoked from main event/rendering context. */
static void
SetColor(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it = AG_TlistSelectedItem(tl);

	if (it != NULL && it->p1 == &agColors[BG_COLOR]) {
		Uint8 r, g, b;
		AG_ColorsGetRGB(BG_COLOR, &r, &g, &b);
		AG_ColorsSetRGB(BG_COLOR, r, g, b);
	}
}

void
DEV_ConfigShow(void)
{
	AG_Window *win;

	/* Avoid clobbering modal windows */
	if (agDriverSw != NULL &&
	    agDriverSw->Lmodal->n > 0)
		return;

	if ((win = DEV_ConfigWindow(agConfig)) != NULL)
		AG_WindowShow(win);
}

static void
LoadColorSchemeFromACS(AG_Event *event)
{
	char *file = AG_STRING(1);

	if (AG_ColorsLoad(file) == 0) {
		AG_TextTmsg(AG_MSG_INFO, 1000,
		    _("Color scheme loaded from %s."), file);
	} else {
		AG_TextMsgFromError();
	}
}

static void
SaveColorSchemeToACS(AG_Event *event)
{
	char *file = AG_STRING(1);

	if (AG_ColorsSave(file) == 0) {
		AG_TextTmsg(AG_MSG_INFO, 1000, _("Color scheme saved to %s."),
		    file);
	} else {
		AG_TextMsgFromError();
	}
}

static void
LoadColorSchemeDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fd;

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, _("Load color scheme..."));
	fd = AG_FileDlgNewMRU(win, "dev.mru.color-schemes",
	    AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgSetFilenameS(fd, "colors.acs");
	AG_FileDlgAddType(fd, _("Agar Color Scheme"), "*.acs",
	    LoadColorSchemeFromACS, NULL);
	AG_WindowShow(win);
}

static void
SaveColorSchemeDlg(AG_Event *event)
{
	AG_Window *win;
	AG_FileDlg *fd;

	win = AG_WindowNew(0);
	AG_WindowSetCaptionS(win, _("Load color scheme..."));
	fd = AG_FileDlgNewMRU(win, "dev.mru.color-schemes",
	    AG_FILEDLG_CLOSEWIN|AG_FILEDLG_EXPAND);
	AG_FileDlgAddType(fd, _("Agar Color Scheme"), "*.acs",
	    SaveColorSchemeToACS, NULL);
	AG_WindowShow(win);
}

static void
SaveConfig(AG_Event *event)
{
	if (AG_ObjectSave(agConfig) == -1) {
		AG_TextMsgFromError();
	} else {
		AG_TextTmsg(AG_MSG_INFO, 750,
		    _("Configuration settings saved successfully."));
	}
}

static AG_Window *
DEV_ConfigWindow(AG_Config *cfg)
{
	char path[AG_PATHNAME_MAX];
	AG_Window *win;
	AG_HBox *hb;
	AG_Textbox *tbox;
/*	AG_Checkbox *cb; */
	AG_Notebook *nb;
	AG_NotebookTab *tab;

	if ((win = AG_WindowNewNamedS(0, "DEV_Config")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("Agar settings"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	tab = AG_NotebookAddTab(nb, _("Video"), AG_BOX_VERT);
	{
		AG_NumericalNewIntR(tab, 0, "%", _("Screenshot quality: "),
		    &agScreenshotQuality, 1, 100);
	}

	tab = AG_NotebookAddTab(nb, _("GUI"), AG_BOX_VERT);
	{
		AG_CheckboxNewInt(tab, 0, _("Built-in key composition"),
		    &agTextComposition);
		AG_CheckboxNewInt(tab, 0, _("Bidirectional"),
		    &agTextBidi);
		
		AG_SpacerNewHoriz(tab);

		AG_NumericalNewIntR(tab, 0, "ms", _("Double click delay: "),
		    &agMouseDblclickDelay, 1, 10000);
		AG_NumericalNewIntR(tab, 0, "ms", _("Cursor spin delay: "),
		    &agMouseSpinDelay, 1, 10000);
		AG_NumericalNewIntR(tab, 0, "ms", _("Cursor spin interval: "),
		    &agMouseSpinIval, 1, 10000);
		AG_NumericalNewIntR(tab, 0, "ms", _("Key repeat delay: "),
		    &agKbdDelay, 1, 1000);
		AG_NumericalNewIntR(tab, 0, "ms", _("Key repeat interval: "),
		    &agKbdRepeat, 1, 500);
	}

	tab = AG_NotebookAddTab(nb, _("Directories"), AG_BOX_VERT);
	{
		tbox = AG_TextboxNewS(tab, AG_TEXTBOX_HFILL, _("Temporary dir: "));
		AG_GetString(agConfig, "tmp-path", path, sizeof(path));
		AG_TextboxSetString(tbox, path);
		AG_SetEvent(tbox, "textbox-return", SetPath, "%s", "tmp-path");

		tbox = AG_TextboxNewS(tab, AG_TEXTBOX_HFILL, _("Data save dir: "));
		AG_GetString(agConfig, "save-path", path, sizeof(path));
		AG_TextboxSetString(tbox, path);
		AG_SetEvent(tbox, "textbox-return", SetPath, "%s", "save-path");
	
		tbox = AG_TextboxNewS(tab, AG_TEXTBOX_HFILL, _("Data load path: "));
		AG_GetString(agConfig, "load-path", path, sizeof(path));
		AG_TextboxSetString(tbox, path);
		AG_SetEvent(tbox, "textbox-return", SetPath, "%s", "load-path");
	
		tbox = AG_TextboxNewS(tab, AG_TEXTBOX_HFILL, _("Font path: "));
		AG_GetString(agConfig, "font-path", path, sizeof(path));
		AG_TextboxSetString(tbox, path);
		AG_SetEvent(tbox, "textbox-return", SetPath, "%s", "font-path");
		
		tbox = AG_TextboxNewS(tab, AG_TEXTBOX_HFILL, _("Den path: "));
		AG_GetString(agConfig, "den-path", path, sizeof(path));
		AG_TextboxSetString(tbox, path);
		AG_SetEvent(tbox, "textbox-return", SetPath, "%s", "den-path");
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
			AG_TlistSizeHint(tl, "Tileview text background", 10);
			for (i = 0; i < LAST_COLOR; i++) {
				it = AG_TlistAdd(tl, NULL, _(agColorNames[i]));
				it->p1 = &agColors[i];
			}
			hsv = AG_HSVPalNew(hPane->div[1], AG_HSVPAL_EXPAND);
			AG_SetEvent(hsv, "h-changed", SetColor, "%p", tl);
			AG_SetEvent(hsv, "sv-changed", SetColor, "%p", tl);
			AG_SetEvent(tl, "tlist-selected", BindSelectedColor,
			    "%p", hsv);
		}
		
		lbl = AG_LabelNew(tab, 0,
		    _("Warning: Some color changes will not "
		      "take effect until %s is restarted."), agProgName);
		AG_LabelSetPaddingLeft(lbl, 10);
		AG_LabelSetPaddingRight(lbl, 10);
		
		hb = AG_HBoxNew(tab, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL);
		{
			AG_ButtonNewFn(hb, 0, _("Load scheme"),
			    LoadColorSchemeDlg, NULL);
			AG_ButtonNewFn(hb, 0, _("Save scheme"),
			    SaveColorSchemeDlg, NULL);
		}
	}

#if 0
#ifdef AG_NETWORK
	tab = AG_NotebookAddTab(nb, _("RCS"), AG_BOX_VERT);
	{
		AG_Textbox *tb;
		AG_Box *box;

		AG_CheckboxNewInt(tab, 0, _("Enable RCS"), &agRcsMode);

		AG_SpacerNewHoriz(tab);

		tb = AG_TextboxNewS(tab, AG_TEXTBOX_HFILL, _("Host: "));
		AG_TextboxSizeHint(tb, "XXXXXXXXXXXXXXXXXXXX");
		AG_TextboxBindUTF8(tb, agRcsHostname, sizeof(agRcsHostname));
		AG_NumericalNewUint(tab, 0, NULL, _("Port: "), &agRcsPort);
		AG_SeparatorNewHoriz(tab);

		box = AG_BoxNewHoriz(tab, AG_BOX_HFILL|AG_BOX_HOMOGENOUS);
		{
			tb = AG_TextboxNewS(box, AG_TEXTBOX_HFILL, _("Username: "));
			AG_TextboxBindUTF8(tb, agRcsUsername,
			    sizeof(agRcsUsername));

			tb = AG_TextboxNewS(box, AG_TEXTBOX_HFILL, _("Password: "));
			AG_TextboxSetPassword(tb, 1);
			AG_TextboxBindUTF8(tb, agRcsPassword,
			    sizeof(agRcsPassword));
		}
	}
#endif /* AG_NETWORK */
#endif

#ifdef AG_DEBUG
	tab = AG_NotebookAddTab(nb, _("Debug"), AG_BOX_VERT);
	{
		AG_NumericalNewIntR(tab, 0, NULL, _("Debug level: "),
		    &agDebugLvl, 0, 255);
	}
#endif

	hb = AG_HBoxNew(win, AG_HBOX_HOMOGENOUS|AG_HBOX_HFILL);
	{
		AG_ButtonNewFn(hb, 0, _("Close"), AGWINDETACH(win));
		AG_ButtonNewFn(hb, 0, _("Save"), SaveConfig, NULL);
	}
	return (win);
}
