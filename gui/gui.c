/*
 * Copyright (c) 2009-2013 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <agar/gui/file_selector.h>
#include <agar/gui/fixed.h>
#include <agar/gui/fspinbutton.h>
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
#include <agar/gui/spinbutton.h>
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
#include <agar/gui/icons_data.h>
#include <agar/gui/text.h>

void *agGUIClasses[] = {
	&agWidgetClass,
	&agWindowClass,
	&agFontClass,
	&agBoxClass,
	&agButtonClass,
	&agCheckboxClass,
	&agComboClass,
	&agConsoleClass,
	&agDirDlgClass,
	&agEditableClass,
	&agFontSelectorClass,
	&agFileDlgClass,
	&agFileSelectorClass,
	&agFixedClass,
	&agFSpinbuttonClass,
	&agFixedPlotterClass,
	&agGraphClass,
#ifdef HAVE_OPENGL
	&agGLViewClass,
#endif
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
	&agSpinbuttonClass,
	&agStatusbarClass,
	&agTitlebarClass,
	&agTableClass,
	&agTreetblClass,
	&agTextboxClass,
	&agTlistClass,
	&agToolbarClass,
	&agUComboClass,
	NULL
};

static int initedGlobals = 0;		/* GUI globals are initialized */

int agGUI = 0;				/* GUI is initialized */
int agRenderingContext = 0;		/* In rendering context */
int agStereo = 0;			/* Stereoscopic display */
int agKbdDelay = 250;			/* Key repeat delay */
int agKbdRepeat = 35;			/* Key repeat interval */
int agMouseDblclickDelay = 250;		/* Mouse double-click delay */
int agMouseSpinDelay = 250;		/* Spinbutton repeat delay */
int agMouseSpinIval = 50;		/* Spinbutton repeat interval */
int agMouseScrollDelay = 100;		/* Scrollbar increment delay */
int agMouseScrollIval = 50;		/* Scrollbar increment interval */
int agTextComposition = 1;		/* Built-in input composition */
int agTextBidi = 0;			/* Bidirectionnal text display */
int agTextCache = 1;			/* Dynamic text caching */
int agTextTabWidth = 40;		/* Tab width (px) */
int agTextBlinkRate = 500;		/* Cursor blink rate (ms) */
int agTextSymbols = 1;			/* Process special symbols in text */
int agPageIncrement = 4;		/* Pgup/Pgdn scrolling increment */
int agIdleThresh = 20;			/* Idling threshold */
int agScreenshotQuality = 100;		/* JPEG quality in % */
int agMsgDelay = 500;			/* Display duration of infoboxes (ms) */
double agZoomValues[AG_ZOOM_RANGE] = {	/* Scale values for zoom */
	30.00, 50.00, 67.00, 80.00, 90.00,
	100.00,
	110.00, 120.00, 133.00, 150.00, 170.00, 200.00, 240.00, 300.00
};

/*
 * Initialize the Agar-GUI globals and built-in classes. This function
 * is invoked internally prior to graphics initialization.
 */
int
AG_InitGUIGlobals(void)
{
	AG_Config *cfg;
	char acsPath[AG_PATHNAME_MAX];
	Uint i;

	if (initedGlobals++ > 0) {
		return (0);
	}
	agGUI = 1;

	AG_RegisterClass(&agDriverClass);
	AG_RegisterClass(&agDriverSwClass);
	AG_RegisterClass(&agDriverMwClass);
	AG_RegisterClass(&agInputDeviceClass);
	AG_RegisterClass(&agMouseClass);
	AG_RegisterClass(&agKeyboardClass);

	for (i = 0; i < agDriverListSize; i++)
		AG_RegisterClass(agDriverList[i]);

	AG_InitGlobalKeys();
	AG_EditableInitClipboards();

	agSurfaceFmt = AG_PixelFormatRGBA(32,
#if AG_BYTEORDER == AG_BIG_ENDIAN
	    0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
	);
	
	agRenderingContext = 0;
	AG_ObjectInitStatic(&agDrivers, &agObjectClass);
	AG_ObjectInitStatic(&agInputDevices, &agObjectClass);

	/* Save GUI globals in agConfig. */
	cfg = AG_ConfigObject();
	AG_BindInt(cfg, "ag_kbd_delay", &agKbdDelay);
	AG_BindInt(cfg, "ag_kbd_repeat", &agKbdRepeat);
	AG_BindInt(cfg, "ag_mouse_dblclick_delay", &agMouseDblclickDelay);
	AG_BindInt(cfg, "ag_mouse_spin_delay", &agMouseSpinDelay);
	AG_BindInt(cfg, "ag_mouse_spin_interval", &agMouseSpinIval);
	AG_BindInt(cfg, "ag_text_composition", &agTextComposition);
	AG_BindInt(cfg, "ag_text_bidi", &agTextBidi);
	AG_BindInt(cfg, "ag_text_cache", &agTextCache);
	AG_BindInt(cfg, "ag_text_tab_width", &agTextTabWidth);
	AG_BindInt(cfg, "ag_text_blink_rate", &agTextBlinkRate);
	AG_BindInt(cfg, "ag_text_symbols", &agTextSymbols);
	AG_BindInt(cfg, "ag_page_increment", &agPageIncrement);
	AG_BindInt(cfg, "ag_idle_threshold", &agIdleThresh);
	AG_BindInt(cfg, "ag_screenshot_quality", &agScreenshotQuality);
	AG_BindInt(cfg, "ag_msg_delay", &agMsgDelay);

	/*
	 * Load the default style sheet (statically compiled from
	 * gui/style.css in the Agar sources).
	 */
	if (AG_LoadStyleSheet(NULL, "_agStyleDefault") == NULL) {
		Verbose("_agStyleDefault: %s\n", AG_GetError());
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
	Uint i;
	
	if (--initedGlobals > 0)
		return;

	AG_PixelFormatFree(agSurfaceFmt);
	agSurfaceFmt = NULL;

	AG_DestroyGlobalKeys();
	AG_EditableDestroyClipboards();
	
	AG_UnregisterClass(&agDriverClass);
	AG_UnregisterClass(&agDriverSwClass);
	AG_UnregisterClass(&agDriverMwClass);
	AG_UnregisterClass(&agInputDeviceClass);
	AG_UnregisterClass(&agMouseClass);
	AG_UnregisterClass(&agKeyboardClass);
	
	for (i = 0; i < agDriverListSize; i++)
		AG_UnregisterClass(agDriverList[i]);
	
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
/*	char path[AG_PATHNAME_MAX]; */
	void **ops;

	/* Register the built-in widget classes. */
	for (ops = &agGUIClasses[0]; *ops != NULL; ops++)
		AG_RegisterClass(*ops);

	/* Initialize the GUI subsystems. */
	agIcon_Init();
#if 0
	/* Try to load a color scheme from the default path. */
	AG_GetString(AG_ConfigObject(), "save-path", path, sizeof(path));
	Strlcat(path, AG_PATHSEP, sizeof(path));
	Strlcat(path, "gui-colors.acs", sizeof(path));
	(void)AG_ColorsLoad(path);
#endif

	/* Initialize the font engine. */
	if (AG_InitTextSubsystem() == -1)
		return (-1);

	/* Initialize the Window system. */
	AG_InitWindowSystem();
	AG_InitAppMenu();
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
	AG_Driver *drv;
	
	AG_LockVFS(&agDrivers);
	OBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_ObjectFreeChildren(drv);
	}
rescan:
	OBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_DriverClose(drv);
		if (drv == (AG_Driver *)agDriverSw) { agDriverSw = NULL; }
		if (drv == (AG_Driver *)agDriverMw) { agDriverMw = NULL; }
		goto rescan;
	}
	agDriverOps = NULL;
	AG_UnlockVFS(&agDrivers);

	/* Destroy the GUI subsystems. */
	AG_DestroyWindowSystem();
	AG_DestroyTextSubsystem();

	/* Unregister the built-in widget classes. */
	for (ops = &agGUIClasses[0]; *ops != NULL; ops++)
		AG_UnregisterClass(*ops);
}

/* Break out of the event loop. */
void
AG_QuitGUI(void)
{
	AG_Terminate(0);
}

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
	AG_DriverClass *dc = NULL;
	int i;
	size_t len;
	
	if (AG_InitGUIGlobals() == -1)
		return (-1);

	if (agDriverMw != NULL || agDriverSw != NULL) {
		AG_SetError(_("Root driver already initialized"));
		goto fail;
	}
	if (spec != NULL && spec[0] != '\0') {
		Strlcpy(specBuf, spec, sizeof(specBuf));
		s = &specBuf[0];

		if (strncmp(s, "<OpenGL>", 8) == 0) {
			/*
			 * Select preferred OpenGL-compatible driver.
			 */
			sOpts = &s[8];
			for (i = 0; i < agDriverListSize; i++) {
				dc = agDriverList[i];
				if (dc->flags & AG_DRIVER_OPENGL &&
				   (drv = AG_DriverOpen(dc)) != NULL)
					break;
			}
			if (i == agDriverListSize) {
				AG_SetError(_("No OpenGL drivers are available"));
				goto fail;
			}
		} else if (strncmp(s, "<SDL>", 5) == 0) {
			/*
			 * Select preferred SDL-compatible driver.
			 */
			sOpts = &s[5];
			for (i = 0; i < agDriverListSize; i++) {
				dc = agDriverList[i];
				if (dc->flags & AG_DRIVER_SDL &&
				   (drv = AG_DriverOpen(dc)) != NULL)
					break;
			}
			if (i == agDriverListSize) {
				AG_SetError(_("No SDL drivers are available"));
				goto fail;
			}
		} else {
			/*
			 * Try explicit list of preferred drivers.
			 */
			while ((tok = AG_Strsep(&s, ",;")) != NULL) {
				for (i = 0; i < agDriverListSize; i++) {
					dc = agDriverList[i];
					len = strlen(dc->name);
					if (strncmp(dc->name, tok, len) == 0 &&
					    (drv = AG_DriverOpen(dc)) != NULL) {
						sOpts = &tok[len];
						break;
					}
				}
				if (i < agDriverListSize)
					break;
			}
			if (tok == NULL) {
				AG_SetError(_("Requested drivers (%s) are not "
				              "available"), specBuf);
				goto fail;
			}
		}
	} else {
		/*
		 * Auto-select best available driver.
		 */
		for (i = 0; i < agDriverListSize; i++) {
			dc = agDriverList[i];
			if ((drv = AG_DriverOpen(dc)) != NULL)
				break;
		}
		if (i == agDriverListSize) {
			AG_SetError(_("No graphics drivers are available"));
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
			Verbose(_("Syntax error in driver options: %s"), sOpts);
		}
		while ((tok = AG_Strsep(&sOpts, ":")) != NULL) {
			if ((key = AG_Strsep(&tok, "=")) != NULL) {
				if (Strcasecmp(key, "stereo") == 0) {
					agStereo = 1;
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

#ifdef AG_LEGACY
/*
 * Initialize Agar with a single-window driver of specified resolution.
 * As of Agar-1.4, this interface obsolete but kept for backward compat.
 */
int
AG_InitVideo(int w, int h, int depth, Uint flags)
{
	AG_Driver *drv = NULL;
	AG_DriverClass *dc = NULL;
	int i;
	
	if (AG_InitGUIGlobals() == -1) {
		return (-1);
	}
	if (agDriverMw != NULL || agDriverSw != NULL) {
		AG_SetError("Root driver already initialized");
		goto fail;
	}
	if (depth < 1 || w < 16 || h < 16) {
		AG_SetError("Resolution too small");
		goto fail;
	}
	if (flags & (AG_VIDEO_OPENGL|AG_VIDEO_OPENGL_OR_SDL)) {
		for (i = 0; i < agDriverListSize; i++) {
			dc = agDriverList[i];
			if (dc->wm == AG_WM_SINGLE &&
			    (dc->flags & AG_DRIVER_OPENGL) &&
			    (drv = AG_DriverOpen(dc)) != NULL)
				break;
		}
		if (i == agDriverListSize) {
			if (flags & AG_VIDEO_OPENGL_OR_SDL) {
				for (i = 0; i < agDriverListSize; i++) {
					dc = agDriverList[i];
					if (dc->wm == AG_WM_SINGLE &&
					    (dc->flags & AG_DRIVER_SDL) &&
					    (drv = AG_DriverOpen(dc)) != NULL)
						break;
				}
				if (i == agDriverListSize) {
					AG_SetError("SDL/GL not available");
					goto fail;
				}
			} else {
				AG_SetError("GL not available");
				goto fail;
			}
		}
	} else if (flags & AG_VIDEO_SDL) {
		for (i = 0; i < agDriverListSize; i++) {
			dc = agDriverList[i];
			if (dc->wm == AG_WM_SINGLE &&
			    (dc->flags & AG_DRIVER_SDL) &&
			    (drv = AG_DriverOpen(dc)) != NULL)
				break;
		}
		if (i == agDriverListSize) {
			AG_SetError("SDL not available");
			goto fail;
		}
	} else {
		for (i = 0; i < agDriverListSize; i++) {
			dc = agDriverList[i];
			if (dc->wm == AG_WM_SINGLE &&
			    (drv = AG_DriverOpen(dc)) != NULL)
				break;
		}
		if (i == agDriverListSize) {
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
