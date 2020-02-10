/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/core/config.h>

#include <agar/gui/window.h>
#include <agar/gui/box.h>
#include <agar/gui/label.h>
#include <agar/gui/button.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/textbox.h>
#include <agar/gui/tlist.h>
#include <agar/gui/mspinbutton.h>
#include <agar/gui/notebook.h>
#include <agar/gui/numerical.h>
#include <agar/gui/hsvpal.h>
#include <agar/gui/separator.h>
#include <agar/gui/file_dlg.h>
#include <agar/gui/dir_dlg.h>
#include <agar/gui/pane.h>

#include <agar/dev/dev.h>

static AG_Window *_Nullable DEV_ConfigWindow(AG_Config *_Nullable);

void
DEV_ConfigShow(void)
{
	AG_Window *win;

	if ((win = DEV_ConfigWindow(agConfig)) != NULL)
		AG_WindowShow(win);
}

static void
SaveConfig(AG_Event *_Nonnull event)
{
	if (AG_ObjectSave(agConfig) == -1) {
		AG_TextMsgFromError();
	} else {
#ifdef AG_TIMERS
		AG_TextTmsg(AG_MSG_INFO, 750,
		    _("Configuration settings saved successfully."));
#endif
	}
}

#if 0
static void
SelectPathOK(AG_Event *_Nonnull event)
{
	char *key = AG_STRING(1);
	AG_Textbox *tbox = AG_TEXTBOX_PTR(2);
	AG_Window *win = AG_WINDOW_PTR(3);
	char *path = AG_STRING(4);
	
	AG_SetString(agConfig, key, path);
	AG_TextboxSetString(tbox, path);
	AG_ObjectDetach(win);
}

static void
SelectPath(AG_Event *_Nonnull event)
{
	char path[AG_PATHNAME_MAX];
	AG_Window *win;
	AG_DirDlg *dd;
	char *key = AG_STRING(1);
	AG_Textbox *tbox = AG_TEXTBOX_PTR(2);

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	dd = AG_DirDlgNew(win, AG_DIRDLG_EXPAND | AG_DIRDLG_CLOSEWIN);
	AG_GetString(agConfig, key, path, sizeof(path));
	if (AG_DirDlgSetDirectoryS(dd, path) == -1) {
		AG_MkPath(path);
		(void)AG_DirDlgSetDirectoryS(dd, path);
	}
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 30, 30);
	AG_WindowSetCaption(win, _("Select %s directory"), key);
	AG_DirDlgOkAction(dd, SelectPathOK, "%s,%p,%p", key, tbox, win);
	AG_WindowShow(win);
}
#endif

static AG_Window *_Nullable
DEV_ConfigWindow(AG_Config *_Nullable cfg)
{
	AG_Window *win;
	AG_Box *hb;
	AG_Notebook *nb;
	AG_NotebookTab *tab;

	if ((win = AG_WindowNewNamedS(0, "DEV_Config")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("Agar settings"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	AG_WindowSetPadding(win, 5,5,5,5);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL | AG_NOTEBOOK_VFILL);
	tab = AG_NotebookAdd(nb, _("Video"), AG_BOX_VERT);
	{
		AG_NumericalNewIntR(tab, 0, "%", _("Screenshot quality: "),
		    &agScreenshotQuality, 1, 100);
	}

	tab = AG_NotebookAdd(nb, _("GUI"), AG_BOX_VERT);
	{
#ifdef AG_UNICODE
		AG_CheckboxNewInt(tab, 0, _("Built-in key composition"),
		    &agTextComposition);
#endif
		AG_SeparatorNewHoriz(tab);
		AG_LabelNewS(tab, 0, _("Timer settings (milliseconds):"));
		AG_NumericalNewIntR(tab, 0, NULL, _("Double click delay: "),
		    &agMouseDblclickDelay, 1, 10000);
		AG_NumericalNewIntR(tab, 0, NULL, _("Cursor spin delay: "),
		    &agMouseSpinDelay, 1, 10000);
		AG_NumericalNewIntR(tab, 0, NULL, _("Cursor spin interval: "),
		    &agMouseSpinIval, 1, 10000);
		AG_NumericalNewIntR(tab, 0, NULL, _("Key repeat delay: "),
		    &agKbdDelay, 1, 1000);
		AG_NumericalNewIntR(tab, 0, NULL, _("Key repeat interval: "),
		    &agKbdRepeat, 1, 500);
	}

	tab = AG_NotebookAdd(nb, _("Directories"), AG_BOX_VERT);
	{
	}

#ifdef AG_DEBUG
	tab = AG_NotebookAdd(nb, _("Debug"), AG_BOX_VERT);
	{
		AG_NumericalNewIntR(tab, 0, NULL, _("Debug level: "),
		    &agDebugLvl, 0, 255);
	}
#endif

	hb = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	{
		AG_ButtonNewFn(hb, 0, _("Close"), AGWINDETACH(win));
		AG_ButtonNewFn(hb, 0, _("Save"), SaveConfig, NULL);
	}
	return (win);
}
