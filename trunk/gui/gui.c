/*
 * Copyright (c) 2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include <config/have_sdl.h>
#include <config/have_opengl.h>

#include "gui.h"
#include "box.h"
#include "button.h"
#include "checkbox.h"
#include "combo.h"
#include "console.h"
#include "editable.h"
#include "file_dlg.h"
#include "file_selector.h"
#include "fixed.h"
#include "fspinbutton.h"
#include "fixed_plotter.h"
#include "font_selector.h"
#include "glview.h"
#include "graph.h"
#include "hsvpal.h"
#include "icon.h"
#include "label.h"
#include "menu.h"
#include "mfspinbutton.h"
#include "mpane.h"
#include "mspinbutton.h"
#include "notebook.h"
#include "numerical.h"
#include "objsel.h"
#include "pane.h"
#include "pixmap.h"
#include "progress_bar.h"
#include "radio.h"
#include "scrollbar.h"
#include "scrollview.h"
#include "separator.h"
#include "slider.h"
#include "socket.h"
#include "spinbutton.h"
#include "statusbar.h"
#include "table.h"
#include "treetbl.h"
#include "textbox.h"
#include "titlebar.h"
#include "tlist.h"
#include "toolbar.h"
#include "ucombo.h"

#include "colors.h"
#include "cursors.h"
#include "primitive.h"
#include "icons.h"
#include "icons_data.h"
#include "text.h"

void *agGUIClasses[] = {
	&agWidgetClass,
	&agWindowClass,
	&agFontClass,
	&agBoxClass,
	&agButtonClass,
	&agCheckboxClass,
	&agComboClass,
	&agConsoleClass,
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

int agGUI = 0;				/* GUI is initialized */
static int initedGlobals = 0;		/* GUI globals are initialized */
int agRenderingContext = 0;		/* In rendering context (DEBUG) */

/*
 * Initialize the Agar-GUI globals and built-in classes. This function
 * is invoked internally prior to graphics initialization.
 */
int
AG_InitGUIGlobals(void)
{
	Uint i;

	if (initedGlobals) {
		return (0);
	}
	initedGlobals = 1;
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
	AG_LabelInitFormats();

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

	AG_PixelFormatFree(agSurfaceFmt);
	agSurfaceFmt = NULL;

	AG_LabelDestroyFormats();
	AG_DestroyGlobalKeys();
	
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
	initedGlobals = 0;
}

/*
 * Initialize the Agar-GUI internals. This is invoked after the graphics
 * subsystem has been initialized.
 */
int
AG_InitGUI(Uint flags)
{
	char path[AG_PATHNAME_MAX];
	void **ops;

	/* Register the built-in widget classes. */
	for (ops = &agGUIClasses[0]; *ops != NULL; ops++)
		AG_RegisterClass(*ops);

	/* Initialize the GUI subsystems. */
	AG_ColorsInit();
	agIcon_Init();

	/* Try to load a color scheme from the default path. */
	AG_GetString(agConfig, "save-path", path, sizeof(path));
	Strlcat(path, AG_PATHSEP, sizeof(path));
	Strlcat(path, "gui-colors.acs", sizeof(path));
	(void)AG_ColorsLoad(path);

	/* Initialize the font engine. */
	if (AG_TextInit() == -1)
		return (-1);

	/* Initialize the Window system. */
	AG_InitWindowSystem();
	AG_MutexInitRecursive(&agAppMenuLock);

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
	
	AG_MutexDestroy(&agAppMenuLock);

	/* Destroy the GUI subsystems. */
	AG_DestroyWindowSystem();
	AG_TextDestroy();
	AG_ColorsDestroy();

	/* Unregister the built-in widget classes. */
	for (ops = &agGUIClasses[0]; *ops != NULL; ops++)
		AG_UnregisterClass(*ops);
}

/* Request application termination. */
void
AG_QuitGUI(void)
{
	SDL_Event nev;

	nev.type = SDL_QUIT;
	SDL_PushEvent(&nev);
}

/*
 * Initialize the graphics driver. If spec is non-NULL, select the driver
 * by priority from a comma-separated list. If spec is NULL, try to match
 * the "best" driver for the current platform.
 */
int
AG_InitGraphics(const char *spec)
{
	AG_Driver *drv = NULL;
	AG_DriverClass *dc = NULL;
	char specBuf[128], *s, *ds;
	int i;
	
	if (AG_InitGUIGlobals() == -1)
		return (-1);

	if (spec != NULL && spec[0] != '\0') {
		if (strcmp(spec, "<OpenGL>") == 0) {
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
		} else if (strcmp(spec, "<SDL>") == 0) {
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
			Strlcpy(specBuf, spec, sizeof(specBuf));
			s = &specBuf[0];
			while ((ds = AG_Strsep(&s, ",;")) != NULL) {
				for (i = 0; i < agDriverListSize; i++) {
					dc = agDriverList[i];
					if (strcmp(dc->name, ds) == 0 &&
					    (drv = AG_DriverOpen(dc)) != NULL)
						break;
				}
				if (i < agDriverListSize)
					break;
			}
			if (ds == NULL) {
				AG_SetError(_("Requested drivers (%s) are not "
				              "available"), spec);
				goto fail;
			}
		}
	} else {
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
	Verbose(_("Selected graphics driver: %s\n"), dc->name);

	switch (dc->wm) {
	case AG_WM_MULTIPLE:
		/* Driver instances will be created along with windows. */
		AG_DriverClose(drv);
		drv = NULL;
		break;
	case AG_WM_SINGLE:
		{
			Uint wView, hView;

			if (AG_GetDisplaySize(drv, &wView, &hView) == -1 ||
			    wView == 0 || hView == 0) {
				wView = 640;
				hView = 480;
			}
			/* Open the video display. */
			if (AGDRIVER_SW_CLASS(drv)->openVideo(drv,
			    wView,hView,32, AG_VIDEO_RESIZABLE) == -1) {
				AG_DriverClose(drv);
				goto fail;
			}
#ifdef AG_DEBUG
			if (drv->videoFmt == NULL)
				AG_FatalError("Driver did not set video format");
#endif
		}
		break;
	}

	/* Generic Agar-GUI initialization. */
	if (AG_InitGUI(0) == -1)
		goto fail;

	agDriverOps = dc;
	agDriverSw = (dc->wm == AG_WM_SINGLE) ? AGDRIVER_SW(drv) : NULL;
#ifdef AG_LEGACY
	agView = drv;
#endif
	return (0);
fail:
	if (drv != NULL) { AG_DriverClose(drv); }
	AG_DestroyGUIGlobals();
	return (-1);
}

/*
 * Initialize Agar with a single-video display. Unlike AG_InitGraphics(),
 * AG_InitVideo() matches only graphics drivers without multiple-window
 * support (e.g., sdlfb, sdlgl).
 * 
 * OpenGL or SDL-only drivers may be requested with flags AG_VIDEO_OPENGL,
 * AG_VIDEO_SDL or AG_VIDEO_OPENGL_OR_SDL (this interface predates the
 * AG_InitGraphics() introduced in Agar-1.4).
 */
int
AG_InitVideo(int w, int h, int depth, Uint flags)
{
	AG_Driver *drv = NULL;
	AG_DriverClass *dc = NULL;
	int i;
	
	if (AG_InitGUIGlobals() == -1)
		return (-1);

	if (depth < 1 || w < 16 || h < 16) {
		AG_SetError(_("The resolution is too small."));
		goto fail;
	}

	/* Initialize the driver. */
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
					AG_SetError("AG_DRIVER_OPENGL_OR_SDL "
					            "requested, but neither "
						    "OpenGL nor SDL drivers "
					            "are available");
					goto fail;
				}
			} else {
				AG_SetError("AG_DRIVER_OPENGL requested, but "
				            "no OpenGL drivers available");
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
			AG_SetError("AG_DRIVER_SDL was requested, but no "
			            "SDL drivers are available");
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

	/* Open a video display. */
	if (AGDRIVER_SW_CLASS(drv)->openVideo(drv, w,h, depth, flags) == -1) {
		AG_DriverClose(drv);
		goto fail;
	}
#ifdef AG_DEBUG
	if (drv->videoFmt == NULL)
		AG_FatalError("Driver did not set video format");
#endif

	Verbose("Opened driver: %s\n", AGDRIVER_CLASS(drv)->name);

	/* Generic Agar-GUI initialization. */
	if (AG_InitGUI(0) == -1) {
		AG_DriverClose(drv);
		goto fail;
	}

	agDriverOps = dc;
	agDriverSw = AGDRIVER_SW(drv);
#ifdef AG_LEGACY
	agView = drv;
#endif
	return (0);
fail:
	AG_DestroyGUIGlobals();
	return (-1);
}

#ifdef HAVE_SDL
/*
 * Initialize Agar with an existing SDL display. If the display surface has
 * flags SDL_OPENGL / SDL_OPENGLBLIT set, the "sdlgl" driver is selected;
 * otherwise, the "sdlfb" driver is used.
 */
int
AG_InitVideoSDL(void *pDisplay, Uint flags)
{
	SDL_Surface *display = pDisplay;
	AG_Driver *drv = NULL;
	AG_DriverClass *dc = NULL;
	Uint dcFlags = AG_DRIVER_SDL;
	int i;

	if (AG_InitGUIGlobals() == -1)
		return (-1);
	
	/* Enable OpenGL mode if the surface has SDL_OPENGL set. */
	if (display->flags & (SDL_OPENGL|SDL_OPENGLBLIT)) {
		if (flags & AG_VIDEO_SDL) {
			AG_SetError("AG_VIDEO_SDL flag requested, but "
			            "display surface has SDL_OPENGL set");
			goto fail;
		}
		dcFlags |= AG_DRIVER_OPENGL;
	} else {
		if (flags & AG_VIDEO_OPENGL) {
			AG_SetError("AG_VIDEO_OPENGL flag requested, but "
			            "display surface is missing SDL_OPENGL");
			goto fail;
		}
	}
	for (i = 0; i < agDriverListSize; i++) {
		dc = agDriverList[i];
		if (dc->wm == AG_WM_SINGLE &&
		    (dc->flags & dcFlags) &&
		    (drv = AG_DriverOpen(dc)) != NULL)
			break;
	}
	if (i == agDriverListSize) {
		AG_SetError("No compatible SDL driver is available");
		goto fail;
	}

	/* Open a video display. */
	if (AGDRIVER_SW_CLASS(drv)->openVideoContext(drv, (void *)display,
	    flags) == -1) {
		AG_DriverClose(drv);
		goto fail;
	}
#ifdef AG_DEBUG
	if (drv->videoFmt == NULL)
		AG_FatalError("Driver did not set video format");
#endif

	/* Generic Agar-GUI initialization. */
	if (AG_InitGUI(0) == -1) {
		AG_DriverClose(drv);
		goto fail;
	}

	agDriverOps = dc;
	agDriverSw = AGDRIVER_SW(drv);
#ifdef AG_LEGACY
	agView = drv;
#endif
	return (0);
fail:
	AG_DestroyGUIGlobals();
	return (-1);
}
#endif /* HAVE_SDL */

void
AG_DestroyVideo(void)
{
	AG_TextDestroy();
	AG_ColorsDestroy();
	AG_ClearGlobalKeys();

	AG_DestroyGUIGlobals();
}

/* Stock event loop routine. */
void
AG_EventLoop_FixedFPS(void)
{
	agDriverOps->genericEventLoop(agDriverSw);
}
