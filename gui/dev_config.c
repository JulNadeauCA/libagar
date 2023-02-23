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
#include <agar/gui/font_selector.h>
#include <agar/gui/file_dlg.h>
#include <agar/gui/dir_dlg.h>
#include <agar/gui/pane.h>
#include <agar/gui/radio.h>
#include <agar/gui/sdl2.h>

/*
 * CPU-specific architecture extension names.
 */
AG_FlagDescrRO agArchExtnNames[] = {
	{ AG_EXT_CPUID,            "CPUID" },                 /* CPUID Insn */
	{ AG_EXT_MMX,              "MMX" },
	{ AG_EXT_MMX_EXT,        _("MMX + AMD Extensions") },
	{ AG_EXT_3DNOW,            "3dNow!" },
	{ AG_EXT_3DNOW_EXT,      _("3dNow! + Extensions") },
	{ AG_EXT_3DNOW_PREFETCH,   "3dNow! " AGSI_CODE" PREFETCH/PREFETCHW" AGSI_RST },
	{ AG_EXT_ALTIVEC,          "AltiVec" },
	{ AG_EXT_SSE,              "SSE" },
	{ AG_EXT_SSE2,             "SSE2" },
	{ AG_EXT_SSE3,             "SSE3"  },
	{ AG_EXT_SSSE3,            "SSSE3" },
	{ AG_EXT_SSE4A,          _("SSE4a Extensions") },
	{ AG_EXT_SSE41,            "SSE41" },
	{ AG_EXT_SSE42,            "SSE42" },
	{ AG_EXT_SSE5A,          _("SSE5a Extensions") },
	{ AG_EXT_SSE_MISALIGNED, _("Misaligned SSE Mode") },
	{ AG_EXT_LONG_MODE,      _("Long Mode") },
	{ AG_EXT_RDTSCP,           "RDTSCP" },                /* RDTSC Insn */
	{ AG_EXT_FXSR,             "FXSR" },
	{ AG_EXT_PAGE_NX,          "PAGE_NX" },                      /* W^X */
	{ AG_EXT_ONCHIP_FPU,     _("On-chip FPU") },
	{ AG_EXT_TSC,              "TSC" },           /* Time Stamp Counter */
	{ AG_EXT_CMOV,             "CMOV" },            /* Conditional Move */
	{ AG_EXT_CLFLUSH,          "CLFLUSH" },         /* Cache-Line Flush */
	{ AG_EXT_HTT,              "HTT" },         /* Hyper-Threading Tech */
	{ AG_EXT_MON,              "MON" },          /* MONITOR/MWAIT Insns */
	{ AG_EXT_VMX,              "VMX" },        /* Virtual Machine Extns */
	{ 0,                     NULL }
};

static AG_Font *agConfigSelectedFont = NULL;

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

static void
SetDefaultFont(AG_Event *event)
{
	if (agConfigSelectedFont == NULL) {
		return;
	}

	AG_SetDefaultFont(agConfigSelectedFont);

	if (AG_ConfigSave() == 0) {
		AG_TextTmsg(AG_MSG_INFO, 1000, "Default font has changed.");
	} else {
		AG_TextMsgFromError();
	}
}

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

			it = AG_TlistAdd(tl, NULL,
			    AGSI_IDEOGRAM AGSI_GAME_CONTROLLER AGSI_RST
			    " %d) %s", i, name);
		} else {
			const char *name = SDL_JoystickNameForIndex(i);

			it = AG_TlistAdd(tl, NULL,
			    AGSI_IDEOGRAM AGSI_JOYSTICK AGSI_RST
			    " %d) %s", i, name);
		}
		it->p1 = NULL;
	}
	if (nJoysticks == 0) {
		AG_TlistAddS(tl, NULL, _("(no controllers detected)"));
	}
	AG_TlistEnd(tl);
}
#endif /* HAVE_SDL2 */

/*
 * Configuration dialog for general Agar settings.
 */
static AG_Window *_Nullable
AG_DEV_ConfigWindow(AG_Config *_Nullable cfg)
{
	AG_Window *win;
	AG_Box *box;
	AG_Notebook *nb;
	AG_NotebookTab *tab;
	const AG_Driver *drv;
	AG_Checkbox *cb;

	if ((win = AG_WindowNewNamedS(0, "DEV_Config")) == NULL) {
		return (NULL);
	}
	drv = AGWIDGET(win)->drv;

	AG_WindowSetCaptionS(win, _("GUI Preferences"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	AG_SetPadding(win, "5");

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);

	tab = AG_NotebookAdd(nb,
	    _(AGSI_IDEOGRAM AGSI_AGAR_AG AGSI_RST " Settings"),
	    AG_BOX_VERT);
	{
		AG_Box *boxArch;
		AG_Label *lbl;

		AG_SetPadding(tab, "8");

		boxArch = AG_BoxNewHoriz(tab, AG_BOX_HFILL);
		{
			char extns[160], *c;
			AG_FlagDescrRO *fd;
			int i;

			extns[0] = '\0';
			for (fd = &agArchExtnNames[0]; fd->bitmask != 0; fd++) {
				if (agCPU.ext & fd->bitmask) {
					Strlcat(extns, fd->descr, sizeof(extns));
					if (i++ == 9) {
						i = 0;
						Strlcat(extns, ",\n", sizeof(extns));
					} else {
						Strlcat(extns, ", ", sizeof(extns));
					}
				}
			}
			if ((c = strrchr(extns, ',')) != NULL)
				*c = '\0';

			if (agCPU.icon != 0) {
				Uint32 uch[2];
				char iconText[16];

				uch[0] = agCPU.icon;
				uch[1] = '\0';
				if (AG_ExportUnicode("UTF-8", iconText, uch,
				    sizeof(iconText)) == -1) {
					iconText[0] = '\0';
				}
				lbl = AG_LabelNew(boxArch, AG_LABEL_HFILL,
				    _("Platform: " AGSI_LEAGUE_SPARTAN" %s " AGSI_RST
				      AGSI_IDEOGRAM "%s" AGSI_RST " "
				      AGSI_ITALIC "\"%s\"" AGSI_RST
				      " (%s)."),
			   	    (agCPU.arch[0]!='\0') ? agCPU.arch : _("Unknown"),
				    iconText,
				    agCPU.vendorID, extns);
			} else {
				lbl = AG_LabelNew(boxArch, 0,
				    _("Platform: " AGSI_LEAGUE_SPARTAN "%s " AGSI_RST
				      AGSI_ITALIC "\"%s\"" AGSI_RST
				      " (%s)."),
			   	    (agCPU.arch[0]!='\0') ? agCPU.arch : _("Unknown"),
				    agCPU.vendorID, extns);
			}
			AG_SetFontSize(lbl, "90%");
		}

		lbl = AG_LabelNew(tab, AG_LABEL_HFILL,
		    _("Driver class: "
		      AGSI_CYAN AGSI_LEAGUE_SPARTAN "%s" AGSI_RST
		      " (%d bpp, SDL=%s, OpenGL=%s)."),
		    OBJECT_CLASS(drv)->name,
		    (drv->videoFmt) ? drv->videoFmt->BitsPerPixel : 32,
		    (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_SDL) ? _("Yes") : _("No"),
		    (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL) ? _("Yes") : _("No"));
		AG_SetFontSize(lbl, "90%");
		AG_SetPadding(lbl, "0 0 0 10");

		AG_SeparatorNewHoriz(tab);

		AG_CheckboxNewInt(tab, 0, _("Enable Clipboard Integration"),
		    &agClipboardIntegration);

		if (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL) {
			AG_CheckboxNewInt(tab, 0,
			    _("Enable " AGSI_CODE "GL_DEBUG_OUTPUT" AGSI_RST),
			    &agGLdebugOutput);

			cb = AG_CheckboxNewInt(tab, 0,
			    _("Enable Stereo Vision"), &agStereo);
			AG_WidgetDisable(cb);

			cb = AG_CheckboxNewInt(tab, 0,
			    _("Use Non-Power-Of-Two Textures"), &agGLuseNPOT);
#ifndef ENABLE_GL_NO_NPOT
			AG_WidgetDisable(cb);
#endif
		}

#ifdef AG_DEBUG
		{
			const char *debugLvlNames[] = {
				N_("0) No Debugging"),
				N_("1) Debug Objects"),
				N_("2) Debug Variables"),
				NULL
			};

			if (AG_OfClass(drv, "AG_Driver:AG_DriverMw:AG_DriverGLX")) {
				cb = AG_CheckboxNewInt(tab, 0,
				    _("Use Synchronous X Events"), &agXsync);
				AG_WidgetDisable(cb);
			}
			AG_SeparatorNewHoriz(tab);
			AG_LabelNewS(tab, 0, _("Debug Level:"));
			AG_RadioNewInt(tab, 0, _(debugLvlNames), &agDebugLvl);
		}
#endif
	}

	tab = AG_NotebookAdd(nb,
	    _(AGSI_IDEOGRAM AGSI_TYPOGRAPHY AGSI_RST " Font"),
	    AG_BOX_VERT);
	{
		AG_FontSelector *fs;

		fs = AG_FontSelectorNew(tab, AG_FONTSELECTOR_EXPAND);
		agConfigSelectedFont = agDefaultFont;
		AG_BindPointer(fs, "font", (void *)&agConfigSelectedFont);
		(void)fs;

		box = AG_BoxNewHoriz(tab, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
		AG_ButtonNewFn(box, 0, _("Set As Default Font"), SetDefaultFont,NULL);
	}

	tab = AG_NotebookAdd(nb,
	    _(AGSI_IDEOGRAM AGSI_KEYBOARD_KEY AGSI_RST " Keyboard"),
	    AG_BOX_VERT);
	{
		AG_SetPadding(tab, "8");

		AG_NumericalNewIntR(tab, AG_NUMERICAL_HFILL, "ms",
		    _("Cursor Blink Rate: "), &agTextBlinkRate, 0, 500);

		AG_SpacerNewHoriz(tab);

		AG_NumericalNewIntR(tab, AG_NUMERICAL_HFILL, "ms",
		    _("Key Repeat Delay: "), &agKbdDelay, 1, 1000);
		AG_NumericalNewIntR(tab, AG_NUMERICAL_HFILL, "ms",
		    _("Key Repeat Interval: "), &agKbdRepeat, 1, 500);

		AG_SpacerNewHoriz(tab);

		AG_NumericalNewIntR(tab, AG_NUMERICAL_HFILL, "ms",
		    _("Autocomplete Delay: "), &agAutocompleteDelay, 1, 5000);
		AG_NumericalNewIntR(tab, AG_NUMERICAL_HFILL, "ms",
		    _("Autocomplete Rate: "), &agAutocompleteRate, 1, 1000);

		AG_SpacerNewHoriz(tab);

		AG_CheckboxNewInt(tab, 0, _("Unicode Character Composition"),
		    &agTextComposition);
	}

	tab = AG_NotebookAdd(nb,
	    _(AGSI_IDEOGRAM AGSI_TWO_BUTTON_MOUSE AGSI_RST " Mouse"),
	    AG_BOX_VERT);
	{
		AG_SetPadding(tab, "8");

		AG_NumericalNewIntR(tab, AG_NUMERICAL_HFILL, "ms",
		    _("Double Click Threshold: "),
		    &agMouseDblclickDelay, 1, 10000);

		AG_SpacerNewHoriz(tab);

		AG_NumericalNewIntR(tab, AG_NUMERICAL_HFILL, "ms",
		    _("Spin Delay: "), &agMouseSpinDelay, 1, 10000);
		AG_NumericalNewIntR(tab, AG_NUMERICAL_HFILL, "ms",
		    _("Spin Interval: "), &agMouseSpinIval, 1, 10000);
		AG_NumericalNewIntR(tab, AG_NUMERICAL_HFILL, "ms",
		    _("Scroll Interval: "), &agMouseScrollIval, 1, 10000);
	}

	tab = AG_NotebookAdd(nb,
	    _(AGSI_IDEOGRAM AGSI_GAME_CONTROLLER AGSI_RST " Controllers"),
	    AG_BOX_VERT);
	{
#ifdef HAVE_SDL2
		AG_Driver *drv = WIDGET(win)->drv;
#endif
		AG_Tlist *tl;

		box = AG_BoxNewVert(tab, AG_BOX_EXPAND);
#ifdef HAVE_SDL2
		if (AG_OfClass(drv, "AG_Driver:AG_DriverMw:AG_DriverSDL2MW:*") ||
		    AG_OfClass(drv, "AG_Driver:AG_DriverSw:AG_DriverSDL2GL:*") ||
		    AG_OfClass(drv, "AG_Driver:AG_DriverSw:AG_DriverSDL2FB:*"))
		{
			AG_LabelNewS(box, 0,
			    _("Joystick support is available."));

			tl = AG_TlistNewPolled(box,
			    AG_TLIST_POLL | AG_TLIST_EXPAND,
			    PollJoysticks, NULL);
			AG_TlistSizeHint(tl, "<XXXXXXXXXXXXXXXXXXXXXXXXXXXXX>", 8);
		} else
#endif
		{
			AG_LabelNew(box, 0,
			    _(AGSI_IDEOGRAM AGSI_TRI_CONSTRUCTION_SIGN AGSI_RST
			      " Joystick support is not available in "
			      AGSI_CODE AGSI_CYAN "%s" AGSI_WHT "(3)" AGSI_RST
			      " at this time.\n"
			      "To use joysticks or game controllers, restart "
			      "with one of:\n\n"
			      AGSI_CODE
			      "  -d 'sdl2mw(ctrl)'\n"
			      "  -d 'sdl2gl(ctrl)'\n"
			      "  -d 'sdl2fb(ctrl)'\n"),
			      AGOBJECT_CLASS(WIDGET(win)->drv)->name);

			AG_PushDisabledState(box);
			tl = AG_TlistNew(box, AG_TLIST_EXPAND);
			AG_TlistAddS(tl, NULL, "(No joystick support)");
			AG_PopDisabledState(box);
		}
	}

	box = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	{
		AG_ButtonNewFn(box, 0, _("Close"), AGWINDETACH(win));
		AG_ButtonNewFn(box, 0, _("Save"), SaveConfig, NULL);
	}

	AG_WindowSetPosition(win, AG_WINDOW_MC, 0);

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
