/*
 * Copyright (c) 2002-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * A "GUI Preferences" dialog for application-global Agar settings.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

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
#include <agar/gui/sdl2.h>

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

#ifdef HAVE_SDL2
static void
PollJoysticks(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	const int nJoysticks = SDL_NumJoysticks();
	int i;

	AG_TlistBegin(tl);
	for (i = 0; i < nJoysticks; i++) {
		AG_TlistItem *it;

		if (SDL_IsGameController(i)) {
			const char *name = SDL_GameControllerNameForIndex(i);

			it = AG_TlistAdd(tl, NULL, "%d. %s (ctrl)", i, name);
		} else {
			const char *name = SDL_JoystickNameForIndex(i);

			it = AG_TlistAdd(tl, NULL, "%d. %s (joy)", i, name);
		}
		it->p1 = NULL;
	}
	if (nJoysticks == 0) {
		AG_TlistAddS(tl, NULL, _("(no controllers detected)"));
	}
	AG_TlistEnd(tl);
}
#endif /* HAVE_SDL2 */

static AG_Window *_Nullable
AG_DEV_ConfigWindow(AG_Config *_Nullable cfg)
{
	AG_Window *win;
	AG_Box *hb;
	AG_Notebook *nb;
	AG_NotebookTab *tab;
	const Uint numFl = AG_NUMERICAL_HFILL;

	if ((win = AG_WindowNewNamedS(0, "DEV_Config")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaptionS(win, _("GUI Preferences"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	AG_SetStyle(win, "padding", "5");

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL | AG_NOTEBOOK_VFILL);

	tab = AG_NotebookAdd(nb, _("Agar GUI"), AG_BOX_VERT);
	{
		const AG_Driver *drv = AGWIDGET(win)->drv;

		AG_LabelNew(tab, 0,
		    _("Driver: " AGSI_COURIER "%s(3)" AGSI_RST "\n"
		      "Depth: %d bpp\n"
		      "SDL: %s, "
		      "OpenGL: %s"),
		    OBJECT_CLASS(drv)->name,
		    (drv->videoFmt) ? drv->videoFmt->BitsPerPixel : 32,
		    (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_SDL) ? _("Yes") : _("No"),
		    (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL) ? _("Yes") : _("No"));

		AG_SeparatorNewHoriz(tab);

		AG_CheckboxNewInt(tab, 0, _("Clipboard integration"),
		                  &agClipboardIntegration);

		if (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL) {
			AG_CheckboxNewInt(tab, 0,
			    _("Enable " AGSI_COURIER "GL_DEBUG_OUTPUT" AGSI_RST),
			    &agGLdebugOutput);

			AG_PushDisabledState(tab);
			AG_CheckboxNewInt(tab, 0, _("Stereo vision"), &agStereo);
			AG_PopDisabledState(tab);
		}
		
#ifndef ENABLE_GL_NO_NPOT
		AG_PushDisabledState(tab);
#endif
		AG_CheckboxNewInt(tab, 0, _("NPOT (non power of two) textures"),
		    &agGLuseNPOT);
#ifndef ENABLE_GL_NO_NPOT
		AG_PopDisabledState(tab);
#endif
		if (AG_OfClass(drv, "AG_Driver:AG_DriverMw:AG_DriverGLX")) {
			AG_PushDisabledState(tab);
			AG_CheckboxNewInt(tab, 0, _("Synchronous X events"),
			    &agXsync);
			AG_PopDisabledState(tab);
		}

		AG_NumericalNewIntR(tab, numFl, "px", _("Tab width: "),
		                    &agTextTabWidth, 0, 1000);
		AG_NumericalNewIntR(tab, numFl, "ms", _("Cursor Blink Rate: "),
		                    &agTextBlinkRate, 0, 500);
		AG_NumericalNewIntR(tab, numFl, "%", _("Screenshot Quality: "),
		                    &agScreenshotQuality, 1, 100);
#ifdef AG_DEBUG
		AG_NumericalNewIntR(tab, numFl, NULL, _("Debug level: "),
		    &agDebugLvl, 0, 255);
#endif
	}

	tab = AG_NotebookAdd(nb, _("Keyboard"), AG_BOX_VERT);
	{
		AG_NumericalNewIntR(tab, numFl, "ms", _("Key repeat delay: "),
		    &agKbdDelay, 1, 1000);
		AG_NumericalNewIntR(tab, numFl, "ms", _("Key repeat interval: "),
		    &agKbdRepeat, 1, 500);
		AG_NumericalNewIntR(tab, numFl, "ms", _("Autocomplete delay: "),
		    &agAutocompleteDelay, 1, 5000);
		AG_NumericalNewIntR(tab, numFl, "ms", _("Autocomplete rate: "),
		    &agAutocompleteRate, 1, 1000);

		AG_SeparatorNewHoriz(tab);

		AG_CheckboxNewInt(tab, 0, _("Unicode character composition"),
		    &agTextComposition);
	}

	tab = AG_NotebookAdd(nb, _("Mouse"), AG_BOX_VERT);
	{
		AG_NumericalNewIntR(tab, numFl, "ms", _("Double click threshold: "),
		    &agMouseDblclickDelay, 1, 10000);

		AG_SeparatorNewHoriz(tab);

		AG_NumericalNewIntR(tab, numFl, "ms", _("Mouse spin delay: "),
		    &agMouseSpinDelay, 1, 10000);
		AG_NumericalNewIntR(tab, numFl, "ms", _("Mouse spin interval: "),
		    &agMouseSpinIval, 1, 10000);
		AG_NumericalNewIntR(tab, numFl, "ms", _("Mouse scroll interval: "),
		    &agMouseScrollIval, 1, 10000);
	}

#ifdef HAVE_SDL2
	if (AG_OfClass(WIDGET(win)->drv, "AG_Driver:AG_DriverMw:AG_DriverSDL2MW:*") ||
	    AG_OfClass(WIDGET(win)->drv, "AG_Driver:AG_DriverSw:AG_DriverSDL2GL:*") ||
	    AG_OfClass(WIDGET(win)->drv, "AG_Driver:AG_DriverSw:AG_DriverSDL2FB:*")) {
		tab = AG_NotebookAdd(nb, _("Joysticks"), AG_BOX_VERT);
		{
			AG_Tlist *tl;

			tl = AG_TlistNewPolled(tab,
			    AG_TLIST_POLL | AG_TLIST_EXPAND,
			    PollJoysticks, NULL);
			AG_TlistSizeHint(tl, "<XXXXXXXXXXXXXXXXXXXXXXXXXXXXX>", 8);
		}
	}
#endif

	hb = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	{
		AG_ButtonNewFn(hb, 0, _("Close"), AGWINDETACH(win));
		AG_ButtonNewFn(hb, 0, _("Save"), SaveConfig, NULL);
	}

	return (win);
}

void
AG_DEV_ConfigShow(void)
{
	AG_Window *win;

	if ((win = AG_DEV_ConfigWindow(agConfig)) != NULL)
		AG_WindowShow(win);
}

#endif /* AG_WIDGETS */
