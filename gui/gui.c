/*
 * Copyright (c) 2009-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Main Agar-GUI initialization.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>

#include <agar/config/have_opengl.h>

#include <agar/gui/gui.h>
#include <agar/gui/box.h>
#include <agar/gui/button.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/combo.h>
#include <agar/gui/console.h>
#include <agar/gui/dir_dlg.h>
#include <agar/gui/editable.h>
#include <agar/gui/file_dlg.h>
#include <agar/gui/fixed.h>
#include <agar/gui/fixed_plotter.h>
#include <agar/gui/font_selector.h>
#include <agar/gui/glview.h>
#include <agar/gui/graph.h>
#include <agar/gui/hsvpal.h>
#include <agar/gui/icon.h>
#include <agar/gui/label.h>
#include <agar/gui/menu.h>
#include <agar/gui/mfspinbutton.h>
#include <agar/gui/mpane.h>
#include <agar/gui/mspinbutton.h>
#include <agar/gui/notebook.h>
#include <agar/gui/numerical.h>
#include <agar/gui/objsel.h>
#include <agar/gui/pane.h>
#include <agar/gui/pixmap.h>
#include <agar/gui/progress_bar.h>
#include <agar/gui/radio.h>
#include <agar/gui/scrollbar.h>
#include <agar/gui/scrollview.h>
#include <agar/gui/separator.h>
#include <agar/gui/slider.h>
#include <agar/gui/socket.h>
#include <agar/gui/statusbar.h>
#include <agar/gui/table.h>
#include <agar/gui/treetbl.h>
#include <agar/gui/textbox.h>
#include <agar/gui/titlebar.h>
#include <agar/gui/tlist.h>
#include <agar/gui/toolbar.h>
#include <agar/gui/ucombo.h>

#include <agar/gui/colors.h>
#include <agar/gui/cursors.h>
#include <agar/gui/primitive.h>
#include <agar/gui/icons.h>
#include <agar/gui/text.h>

/* Import icon bitmap data */
#include <agar/gui/icons_data.h>

static struct {
	const char *_Nonnull key;
	int        *_Nonnull p;
} agGUIOptions[] = {
	{ "ClipboardIntegration", &agClipboardIntegration },
	{ "KbdDelay",             &agKbdDelay             },
	{ "KbdRepeat",            &agKbdRepeat            },
	{ "MouseDblclickDelay",   &agMouseDblclickDelay   },
	{ "MouseSpinDelay",       &agMouseSpinDelay       },
	{ "MouseSpinIval",        &agMouseSpinIval        },
	{ "MouseScrollIval",      &agMouseScrollIval      },
	{ "ScrollButtonIval",     &agScrollButtonIval     },
	{ "PageIncrement",        &agPageIncrement        },
	{ "AutocompleteDelay",    &agAutocompleteDelay    },
	{ "AutocompleteRate",     &agAutocompleteRate     },
	{ "TextComposition",      &agTextComposition      },
	{ "ScreenshotQuality",    &agScreenshotQuality    },
	{ "TextTabWidth",         &agTextTabWidth         },
	{ "TextBlinkRate",        &agTextBlinkRate        },
	{ "GLdebugOutput",        &agGLdebugOutput        },
	{ "GLuseNPOT",            &agGLuseNPOT            },
};
const Uint agGUIOptionCount = sizeof(agGUIOptions) / sizeof(agGUIOptions[0]);

void *agStdClasses[] = {
	&agDriverClass,
	&agDriverSwClass,
	&agDriverMwClass,
	&agInputDeviceClass,
	&agMouseClass,
	&agKeyboardClass,
	&agFontClass,
	NULL
};
void *agStdWidgets[] = {
	&agWidgetClass,
	&agWindowClass,
#ifdef AG_WIDGETS
	&agBoxClass,
	&agButtonClass,
	&agCheckboxClass,
	&agComboClass,
	&agConsoleClass,
	&agEditableClass,
	&agDirDlgClass,
	&agFontSelectorClass,
	&agFileDlgClass,
	&agFixedClass,
	&agFixedPlotterClass,
	&agGraphClass,
# ifdef HAVE_OPENGL
	&agGLViewClass,
# endif
	&agHSVPalClass,
	&agIconClass,
	&agLabelClass,
	&agMenuClass,
	&agMenuViewClass,
	&agMFSpinbuttonClass,
	&agMPaneClass,
	&agMSpinbuttonClass,
	&agNotebookClass,
	&agNotebookTabClass,
	&agNumericalClass,
	&agObjectSelectorClass,
	&agPaneClass,
	&agPixmapClass,
	&agProgressBarClass,
	&agRadioClass,
	&agScrollbarClass,
	&agScrollviewClass,
	&agSeparatorClass,
	&agSliderClass,
	&agSocketClass,
	&agStatusbarClass,
	&agTitlebarClass,
	&agTableClass,
	&agTreetblClass,
	&agTextboxClass,
	&agTlistClass,
	&agToolbarClass,
	&agUComboClass,
#endif /* AG_WIDGETS */
	NULL
};

static int initedGlobals = 0;		/* GUI globals are initialized */

int agGUI = 0;				/* GUI is initialized */
int agRenderingContext = 0;		/* In rendering context */
int agStereo = 0;			/* Stereoscopic display */
int agXsync = 0;			/* Synchronous X events */
int agClipboardIntegration = 1;		/* Native clipboard integration */
int agKbdDelay = 250;			/* Key repeat delay */
int agKbdRepeat = 30;			/* Key repeat interval */
int agMouseDblclickDelay = 250;		/* Mouse double-click delay */
int agMouseSpinDelay = 350;		/* Spinbutton repeat delay */
int agMouseSpinIval = 30;		/* Spinbutton repeat interval */
int agMouseScrollIval = 1;		/* Scrollbar increment interval */
int agScrollButtonIval = 100;		/* Scrollbar button interval */
int agPageIncrement = 4;		/* Pgup/Pgdn scrolling increment */
int agAutocompleteDelay = 1;		/* Delay before autocomplete (ms) */
int agAutocompleteRate = 80;		/* Autocomplete refresh rate (ms) */
int agScreenshotQuality = 100;		/* JPEG quality in % */
int agTextComposition = 1;		/* Input character composition */
int agTextTabWidth = 40;		/* Tab width (px) */
int agTextBlinkRate = 500;		/* Cursor blink rate (ms) */
int agGLdebugOutput = 0;		/* Enable GL_DEBUG_OUTPUT */
int agGLuseNPOT = 0;			/* Use non-power-of-two textures */

double agZoomValues[AG_ZOOM_MAX] = {
	55.0, 60.0, 65.00, 70.00, 75.00, 80.00, 90.00, 95.00,
	100.00, 110.00, 120.00, 133.00, 150.00, 170.00,
	200.00, 210.00, 220.00, 240.00, 250.00, 300.00
};

/*
 * Initialize the Agar-GUI globals and built-in classes. This function
 * is invoked internally prior to graphics initialization.
 */
int
AG_InitGUIGlobals(void)
{
	AG_Config *cfg;
	void **cl;
	AG_DriverClass **pd;
	Uint i;

	if (initedGlobals++ > 0) {
		return (0);
	}
	for (cl = &agStdClasses[0]; *cl != NULL; cl++)
		AG_RegisterClass(*cl);
	for (pd = &agDriverList[0]; *pd != NULL; pd++)
		AG_RegisterClass(*pd);

	AG_InitGlobalKeys();
	AG_EditableInitClipboards();

	if ((agSurfaceFmt = TryMalloc(sizeof(AG_PixelFormat))) == NULL) {
		return (-1);
	}
	AG_PixelFormatRGBA(agSurfaceFmt, 32,
#if AG_BYTEORDER == AG_BIG_ENDIAN
	    0xff000000,
	    0x00ff0000,
	    0x0000ff00,
	    0x000000ff
#else
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000,
	    0xff000000
#endif
	);
	
	agGUI = 1;
	agRenderingContext = 0;

	AG_ObjectInit(&agDrivers, &agObjectClass);
	AG_ObjectSetName(&agDrivers, "agDrivers");
	agDrivers.flags |= AG_OBJECT_STATIC;

	AG_ObjectInit(&agInputDevices, &agObjectClass);
	AG_ObjectSetName(&agInputDevices, "agInputDevices");
	agInputDevices.flags |= AG_OBJECT_STATIC;

	{
#ifdef AG_DEBUG
		const int dbgLvlSave = agDebugLvl;

		agDebugLvl = 0;
#endif
		cfg = AG_ConfigObject();
		for (i = 0; i < agGUIOptionCount; i++) {
			AG_BindInt(cfg, agGUIOptions[i].key, agGUIOptions[i].p);
		}
#ifdef AG_DEBUG
		agDebugLvl = dbgLvlSave;
#endif
		if (AG_LoadStyleSheet(NULL, "_agStyleDefault") == NULL)
			AG_Verbose("Error loading stylesheet: %s\n", AG_GetError());
	}
	return (0);
}

/*
 * Destroy the Agar-GUI globals and built-in classes. This function
 * is invoked internally after the graphics subsystem is destroyed.
 */
void
AG_DestroyGUIGlobals(void)
{
	AG_DriverClass **pd;
	void **pcl;
	Uint i;

	if (--initedGlobals > 0)
		return;

	AG_DestroyStyleSheet(&agDefaultCSS);
	{
		AG_Config *cfg = AG_ConfigObject();
#ifdef AG_DEBUG
		int debugLvlSave;
#endif
		Debug_Mute(debugLvlSave);
		for (i = 0; i < agGUIOptionCount; i++) {
			AG_Unset(cfg, agGUIOptions[i].key);
		}
		Debug_Unmute(debugLvlSave);
	}
	AG_ObjectDestroy(&agInputDevices);
#ifndef __APPLE__ /* XXX mutex issue */
	AG_ObjectDestroy(&agDrivers);
#endif

	AG_PixelFormatFree(agSurfaceFmt);
	free(agSurfaceFmt);
	agSurfaceFmt = NULL;

	AG_EditableDestroyClipboards();
	AG_DestroyGlobalKeys();
	
	for (pd = &agDriverList[0]; *pd != NULL; pd++)
		AG_UnregisterClass(*pd);

	for (pcl = &agStdClasses[0]; *pcl != NULL; pcl++)
		AG_UnregisterClass(*pcl);

	agRenderingContext = 0;
	agGUI = 0;
}

/*
 * Initialize the Agar-GUI library. This is called internally by
 * AG_InitGraphics().
 *
 * As an alternative to AG_InitGraphics(), applications may also invoke
 * AG_InitGUI() directly (following one or more AG_DriverOpen() calls).
 */
int
AG_InitGUI(Uint flags)
{
	void **ops;

	/* Register standard GUI widget classes. */
	for (ops = &agStdWidgets[0]; *ops != NULL; ops++)
		AG_RegisterClass(*ops);
	
	/* Initialize the statically-compiled icon data. */
	agIcon_Init();

	/* Start the font engine. */
	if (AG_InitTextSubsystem() == -1)
		return (-1);

	/* Initialize global Window lists and pointers. */
	TAILQ_INIT(&agWindowDetachQ);
	TAILQ_INIT(&agWindowShowQ);
	TAILQ_INIT(&agWindowHideQ);
	agWindowToFocus = NULL;
	agWindowFocused = NULL;

	return (0);
}

/*
 * Release all resources allocated by the Agar-GUI library. This is called
 * directly from the application, or from AG_Destroy().
 */
void
AG_DestroyGUI(void)
{
	void **ops;
	AG_Object *drv, *drvNext;
	AG_Window *win;
	
	AG_LockVFS(&agDrivers);

	/* Destroy all windows */
	OBJECT_FOREACH_CHILD(drv, &agDrivers, ag_object) {
		OBJECT_FOREACH_CHILD(win, drv, ag_window) {
			AG_ObjectDetach(win);
		}
	}
	while (!TAILQ_EMPTY(&agWindowDetachQ)) {
		AG_WindowProcessDetachQueue();
	}

	for (drv = TAILQ_FIRST(&agDrivers.children);
	     drv != TAILQ_END(&agDrivers.children);
	     drv = drvNext) {
		drvNext = TAILQ_NEXT(drv, cobjs);
		TAILQ_INIT(&drv->children);
		AG_DriverClose((AG_Driver *)drv);
	}

	agDriverSw = NULL;
	agDriverMw = NULL;
	agDriverOps = NULL;
	AG_UnlockVFS(&agDrivers);

	AG_DestroyTextSubsystem();

	agIcon_Destroy();

	for (ops = &agStdWidgets[0]; *ops != NULL; ops++)
		AG_UnregisterClass(*ops);
	
	AG_DestroyGUIGlobals();
}

#ifdef AG_EVENT_LOOP
/*
 * Break out of the event loop.
 */
void
AG_QuitGUI(void)
{
	AG_Terminate(0);
}
#endif

/*
 * Initialize the Agar-GUI library. If spec is non-NULL, select the driver
 * by priority from a comma-separated list. If spec is NULL, try to match
 * the "best" driver for the current platform.
 *
 * Note: Instead of using AG_InitGraphics(), applications may also use the
 * low-level AG_DriverOpen(), AG_InitGUI() and AG_DestroyGUI() interface
 * (as is needed when creating multiple driver instances).
 */
int
AG_InitGraphics(const char *spec)
{
	char specBuf[128], *s, *sOpts = "", *tok;
	AG_Driver *drv = NULL;
	AG_DriverClass *dc = NULL, **pd;
	
	if (AG_InitGUIGlobals() == -1)
		return (-1);

	if (agDriverMw != NULL || agDriverSw != NULL) {
		AG_SetErrorS("agDriver is already set");
		goto fail;
	}
	if (spec != NULL && spec[0] != '\0') {
		Strlcpy(specBuf, spec, sizeof(specBuf));
		s = &specBuf[0];

		if (Strncasecmp(s, "<OpenGL>", 8) == 0) {    /* Any GL driver */
			sOpts = &s[8];
			for (pd = &agDriverList[0]; *pd != NULL; pd++) {
				if ((*pd)->flags & AG_DRIVER_OPENGL &&
				   (drv = AG_DriverOpen(*pd)) != NULL) {
				   	dc = *pd;
					break;
				}
			}
			if (dc == NULL) {
				AG_SetErrorS(_("No OpenGL drivers are available"));
				goto fail;
			}
		} else if (Strncasecmp(s, "<SDL>", 5) == 0) { /* Any SDL driver */
			sOpts = &s[5];
			for (pd = &agDriverList[0]; *pd != NULL; pd++) {
				if ((*pd)->flags & AG_DRIVER_SDL &&
				   (drv = AG_DriverOpen(*pd)) != NULL) {
					dc = *pd;
					break;
				}
			}
			if (dc == NULL) {
				AG_SetErrorS(_("No SDL drivers are available"));
				goto fail;
			}
		} else if (Strncasecmp(s, "<FB>", 4) == 0) { /* Any framebuffer driver */
			sOpts = &s[5];
			for (pd = &agDriverList[0]; *pd != NULL; pd++) {
				if ((*pd)->type == AG_FRAMEBUFFER &&
				   (drv = AG_DriverOpen(*pd)) != NULL) {
					dc = *pd;
					break;
				}
			}
			if (dc == NULL) {
				AG_SetErrorS(_("No framebuffer drivers are available"));
				goto fail;
			}
		} else {
			/*
			 * Try explicit list of preferred drivers.
			 */
			while ((tok = AG_Strsep(&s, ",;")) != NULL) {
				for (pd = &agDriverList[0]; *pd != NULL; pd++) {
					size_t len = strlen((*pd)->name);

					if (strncmp((*pd)->name, tok, len) == 0 &&
					    (tok[len] == '\0' || tok[len] == '(') &&
					    (drv = AG_DriverOpen(*pd)) != NULL) {
						sOpts = &tok[len];
						dc = *pd;
						break;
					}
				}
				if (dc != NULL)
					break;
			}
			if (tok == NULL) {
				AG_SetError(_("No such Agar driver: \"%s\""), specBuf);
				AG_SetErrorCode(AG_ENOENT);
				goto fail;
			}
		}
	} else {
		/*
		 * Auto-select best available driver.
		 */
		for (pd = &agDriverList[0]; *pd != NULL; pd++) {
			if ((drv = AG_DriverOpen(*pd)) != NULL) {
				dc = *pd;
				break;
			}
		}
		if (dc == NULL) {
			AG_SetError(_("No Agar drivers are available"));
			goto fail;
		}
	}
	
	/* Process driver options */
	if (sOpts[0] == '(' && sOpts[1] != '\0') {
		char *key, *val, *ep;

		sOpts++;
		if ((ep = strrchr(sOpts, ')')) != NULL) {
			*ep = '\0';
		} else {
			Verbose(_("Syntax error in driver options: %s\n"), sOpts);
		}
		while ((tok = AG_Strsep(&sOpts, ":")) != NULL) {
			if ((key = AG_Strsep(&tok, "=")) != NULL) {
				if (Strcasecmp(key, "stereo") == 0) {
					agStereo = 1;
					continue;
				} else if (Strcasecmp(key, "xsync") == 0) {
					agXsync = 1;
					continue;
				} else if (Strcasecmp(key, "debug") == 0) {
					agGLdebugOutput = 1;
					continue;
				}
				if ((val = AG_Strsep(&tok, "=")) != NULL) {
					AG_SetString(drv, key, val);
				} else {
					AG_SetString(drv, key, "");
				}
			}
		}
	}

	switch (dc->wm) {
	case AG_WM_MULTIPLE:
		agDriverMw = AGDRIVER_MW(drv);
		break;
	case AG_WM_SINGLE:
		if (AGDRIVER_SW_CLASS(drv)->openVideo(drv, 0,0,0,
		    AG_VIDEO_RESIZABLE) == -1) {
			AG_SetError("%s: %s", OBJECT(drv)->name,
			    AG_GetError());
			goto fail_close;
		}
		agDriverSw = AGDRIVER_SW(drv);
		break;
	}
	agDriverOps = dc;

	if (AG_InitGUI(0) == -1) {
		goto fail_close;
	}
	return (0);
fail_close:
	if (drv != NULL) { AG_DriverClose(drv); }
	agDriverOps = NULL;
	agDriverSw = NULL;
	agDriverMw = NULL;
fail:
	AG_DestroyGUIGlobals();
	return (-1);
}

/* Provided for symmetry. */
void
AG_DestroyGraphics(void)
{
	AG_DestroyGUI();
}

/*
 * Zoom in/out the GUI elements of the active window, changing
 * the default font size accordingly. Depending on style settings,
 * the default font size of a window may affect that of its logical
 * child windows as well.
 *
 * It is customary to assign AG_GlobalKeys(3) shortcuts to those
 * routines. Most users will expect Ctrl+{Plus,Minus,0} to work.
 */
void
AG_ZoomIn(void)
{
	AG_Window *win;

	AG_LockVFS(&agDrivers);
	if ((win = agWindowFocused) != NULL) {
		AG_WindowSetZoom(win, win->zoom+1);
	} else {
		Verbose("No window is focused for zoom\n");
	}
	AG_UnlockVFS(&agDrivers);
}
void
AG_ZoomOut(void)
{
	AG_Window *win;

	AG_LockVFS(&agDrivers);
	if ((win = agWindowFocused) != NULL) {
		AG_WindowSetZoom(win, win->zoom-1);
	} else {
		Verbose("No window is focused for zoom-out\n");
	}
	AG_UnlockVFS(&agDrivers);
}
void
AG_ZoomReset(void)
{
	AG_Window *win;

	AG_LockVFS(&agDrivers);
	if ((win = agWindowFocused) != NULL) {
		AG_WindowSetZoom(win, AG_ZOOM_DEFAULT);
	} else {
		Verbose("No window is focused for zoom-in\n");
	}
	AG_UnlockVFS(&agDrivers);
}

/* Generate "About Agar" dialog window. */
void
AG_About(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_Window *win;
	AG_Label *lbl;
	AG_Box *box;
	AG_Textbox *tb;
	FILE *f;

	if ((win = AG_WindowNewNamedS(0, "_agAbout")) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, _("About Agar GUI"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	AG_BoxSetHorizAlign(box, AG_BOX_CENTER);
	AG_SetStyle(box, "spacing", "50");
	{
		AG_AgarVersion av;

		AG_GetVersion(&av);

		lbl = AG_LabelNew(box, 0,
		    "Agar %d.%d.%d (" AGSI_FRAK "%s" AGSI_RST ")",
		    av.major, av.minor, av.patch,
		    (av.release) ? av.release : "beta");

		AG_SetStyle(lbl, "font-family", "cm-sans");
		AG_SetStyle(lbl, "font-size", "220%");

		if (AG_ConfigFind(AG_CONFIG_PATH_DATA, "sq-agar.bmp",
		    path, sizeof(path)) == 0)
			AG_PixmapFromFile(box, 0, path);
	}

	tb = AG_TextboxNewS(win, AG_TEXTBOX_MULTILINE | AG_TEXTBOX_EXPAND |
	                         AG_TEXTBOX_READONLY | AG_TEXTBOX_WORDWRAP, NULL);
	AG_SetStyle(tb, "font-family", "vera-mono");
	AG_TextboxSizeHintLines(tb, 20);

	if (AG_ConfigFind(AG_CONFIG_PATH_DATA, "license.txt", path, sizeof(path)) == 0 &&
	   (f = fopen(path, "r")) != NULL) {
		char *s;
		AG_Size size;

		fseek(f, 0, SEEK_END);
		size = (AG_Size)ftell(f);
		fseek(f, 0, SEEK_SET);
		s = Malloc(size + 1);
		fread(s, size, 1, f);
		fclose(f);
		s[size] = '\0';

		AG_TextboxBindASCII(tb, s, size);
	} else {
		AG_TextboxPrintf(tb, _("Failed to open license.txt"));
	}

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Close"), AGWINCLOSE(win));
	AG_WindowShow(win);
}

#ifdef AG_LEGACY
/*
 * Initialize Agar with a single-window driver of specified resolution.
 * Kept for backward compatibility with Agar < 1.4.
 */
int
AG_InitVideo(int w, int h, int depth, Uint flags)
{
	AG_Driver *drv = NULL;
	AG_DriverClass *dc = NULL, **pd;
	
	if (AG_InitGUIGlobals() == -1) {
		return (-1);
	}
	if (agDriverMw != NULL || agDriverSw != NULL) {
		AG_SetErrorS("agDriver is already set");
		goto fail;
	}
	if (depth < 1 || w < 16 || h < 16) {
		AG_SetError("Resolution too small");
		goto fail;
	}
	if (flags & (AG_VIDEO_OPENGL | AG_VIDEO_OPENGL_OR_SDL)) {
		for (pd = &agDriverList[0]; *pd != NULL; pd++) {
			if ((*pd)->wm == AG_WM_SINGLE &&
			   ((*pd)->flags & AG_DRIVER_OPENGL) &&
			    (drv = AG_DriverOpen(*pd)) != NULL) {
				dc = *pd;
				break;
			}
		}
		if (dc == NULL) {
			if (flags & AG_VIDEO_OPENGL_OR_SDL) {
				for (pd = &agDriverList[0]; *pd != NULL; pd++) {
					if ((*pd)->wm == AG_WM_SINGLE &&
					    ((*pd)->flags & AG_DRIVER_SDL) &&
					    (drv = AG_DriverOpen(*pd)) != NULL) {
						dc = *pd;
						break;
					}
				}
				if (dc == NULL) {
					AG_SetError("SDL/GL not available");
					goto fail;
				}
			} else {
				AG_SetError("GL not available");
				goto fail;
			}
		}
	} else if (flags & AG_VIDEO_SDL) {
		for (pd = &agDriverList[0]; *pd != NULL; pd++) {
			if ((*pd)->wm == AG_WM_SINGLE &&
			   ((*pd)->flags & AG_DRIVER_SDL) &&
			    (drv = AG_DriverOpen(*pd)) != NULL) {
				dc = *pd;
				break;
			}
		}
		if (dc == NULL) {
			AG_SetError("SDL not available");
			goto fail;
		}
	} else {
		for (pd = &agDriverList[0]; *pd != NULL; pd++) {
			if ((*pd)->wm == AG_WM_SINGLE &&
			    (drv = AG_DriverOpen(*pd)) != NULL) {
				dc = *pd;
				break;
			}
		}
		if (dc == NULL) {
			AG_SetError("No graphics drivers are available");
			goto fail;
		}
	}
	if (AGDRIVER_SW_CLASS(drv)->openVideo(drv, w,h, depth, flags) == -1) {
		AG_DriverClose(drv);
		goto fail;
	}
	if (AG_InitGUI(0) == -1) {
		AG_DriverClose(drv);
		goto fail;
	}
	agDriverOps = dc;
	agDriverSw = AGDRIVER_SW(drv);
	return (0);
fail:
	AG_DestroyGUIGlobals();
	return (-1);
}
void
AG_DestroyVideo(void)
{
	AG_DestroyGUI();
}
#endif /* AG_LEGACY */
