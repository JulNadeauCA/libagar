/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Micro-Agar initialization.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>

#include <agar/micro/gui.h>
#include <agar/micro/widget.h>
#include <agar/micro/window.h>

#if 0
#include <agar/micro/box.h>
#include <agar/micro/button.h>
#include <agar/micro/checkbox.h>
#include <agar/micro/combo.h>
#include <agar/micro/console.h>
#include <agar/micro/file_dlg.h>
#include <agar/micro/fixed.h>
#include <agar/micro/fixed_plotter.h>
#include <agar/micro/graph.h>
#include <agar/micro/label.h>
#include <agar/micro/notebook.h>
#include <agar/micro/numerical.h>
#include <agar/micro/pane.h>
#include <agar/micro/pixmap.h>
#include <agar/micro/radio.h>
#include <agar/micro/textbox.h>
#include <agar/micro/scrollbar.h>
#include <agar/micro/separator.h>
#include <agar/micro/table.h>
#include <agar/micro/tlist.h>
#include <agar/micro/toolbar.h>
#endif

#include <agar/micro/colors.h>
#include <agar/micro/primitive.h>

void *maStdClasses[] = {
	&maDriverClass,
	NULL
};
void *maStdWidgets[] = {
	&maWidgetClass,
	&maWindowClass,
	NULL
};

static Uint8 maInitedGlobals = 0;	/* GUI globals are initialized */

Uint8 maGUI = 0;			/* GUI is initialized */
Uint8 maRenderingContext = 0;		/* In rendering context */
Uint8 maKbdDelay_2 = 125;		/* Key repeat delay (in ms/2) */
Uint8 maKbdRepeat = 30;			/* Key repeat interval */
Uint8 maMouseDblclickDelay = 250;	/* Mouse double-click delay */
Uint8 maPageIncrement = 4;		/* Pgup/Pgdn scrolling increment */

/*
 * Initialize the Micro-Agar globals and built-in classes. This function
 * is invoked internally prior to graphics initialization.
 */
Sint8
MA_InitGUIGlobals(void)
{
	void **cl;
	MA_DriverClass **pd;

	if (maInitedGlobals++ > 0) {
		return (0);
	}
	for (cl = &maStdClasses[0]; *cl != NULL; cl++)
		AG_RegisterClass(*cl);
	for (pd = &maDriverList[0]; *pd != NULL; pd++)
		AG_RegisterClass(*pd);

	/*
	 * Set reference surface format to the most general surface
	 * we can support.
	 */
	if ((maSurfaceFmt = TryMalloc(sizeof(MA_PixelFormat))) == NULL) {
		return (-1);
	}
	MA_PixelFormatRGBA(maSurfaceFmt,
	    16,
#if AG_BYTEORDER == AG_BIG_ENDIAN
	    0xf000, 0x0f00, 0x00f0, 0x000f	/* RGBA */
#else
	    0x000f, 0x00f0, 0x0f00, 0xf000	/* ABGR */
#endif
	);
	
	maGUI = 1;
	maRenderingContext = 0;
	return (0);
}

/*
 * Destroy the Agar-GUI globals and built-in classes. This function
 * is invoked internally after the graphics subsystem is destroyed.
 */
void
AG_DestroyGUIGlobals(void)
{
	MA_DriverClass **pd;
	void **pcl;

	if (--maInitedGlobals > 0)
		return;

	free(maSurfaceFmt);
	maSurfaceFmt = NULL;

	for (pd = &maDriverList[0]; *pd != NULL; pd++)
		AG_UnregisterClass(*pd);

	for (pcl = &maStdClasses[0]; *pcl != NULL; pcl++)
		AG_UnregisterClass(*pcl);

	maRenderingContext = 0;
	maGUI = 0;
}

/*
 * Initialize the Micro-Agar library. This is called internally by
 * MA_InitGraphics().
 *
 * As an alternative to MA_InitGraphics(), applications may also invoke
 * MA_InitGUI() directly (following one or more MA_DriverOpen() calls).
 */
void
MA_InitGUI(void)
{
	void **ops;

	/* Register standard Micro-Agar classes. */
	for (ops = &maStdWidgets[0]; *ops != NULL; ops++)
		AG_RegisterClass(*ops);
}

/*
 * Release all resources allocated by the Micro-Agar library. This is called
 * directly from the application (or implied by an AG_Destroy() call).
 */
void
MA_DestroyGUI(void)
{
	void **ops;
	MA_Window *win;
	
	/* Release all windows and widgets */
	if (maDriver) {
		OBJECT_FOREACH_CHILD(win, maDriver, ma_window) {
			AG_ObjectDetach(win);
		}
		while (!TAILQ_EMPTY(&maWindowDetachQ))
			MA_WindowProcessDetachQueue();

		/* Terminate this driver instance */
		MA_DriverClose(maDriver);
		maDriver = NULL;
	}
	maDriverOps = NULL;

	/* Remove standard Micro-Agar classes. */
	for (ops = &maStdWidgets[0]; *ops != NULL; ops++)
		AG_UnregisterClass(*ops);
	
	MA_DestroyGUIGlobals();
}

/*
 * Initialize the Micro-Agar library. If spec is non-NULL, select the driver
 * by priority from a comma-separated list. If spec is NULL, try to match
 * the "best" driver for the current platform.
 */
Sint8
MA_InitGraphics(const char *spec)
{
	char specBuf[128], *s, *sOpts = "", *tok;
	MA_Driver *drv = NULL;
	MA_DriverClass *dc = NULL, **pd;
	
	if (MA_InitGUIGlobals() == -1)
		return (-1);

	if (maDriver) {
		AG_SetErrorS("Driver exists");
		goto fail;
	}
	if (spec != NULL && spec[0] != '\0') {
		Strlcpy(specBuf, spec, sizeof(specBuf));
		s = &specBuf[0];

		/*
		 * Try explicit list of preferred drivers.
		 */
		while ((tok = AG_Strsep(&s, ",;")) != NULL) {
			for (pd = &maDriverList[0]; *pd != NULL; pd++) {
				size_t len = strlen((*pd)->name);

				if (strncmp((*pd)->name, tok, len) == 0 &&
				    (tok[len] == '\0' || tok[len] == '(') &&
				    (drv = MA_DriverOpen(*pd)) != NULL) {
					sOpts = &tok[len];
					dc = *pd;
					break;
				}
			}
			if (dc != NULL)
				break;
		}
		if (tok == NULL) {
			AG_SetErrorS("No such driver");
			goto fail;
		}
	} else {
		/*
		 * Auto-select best available driver.
		 */
		for (pd = &maDriverList[0]; *pd != NULL; pd++) {
			if ((drv = MA_DriverOpen(*pd)) != NULL) {
				dc = *pd;
				break;
			}
		}
		if (dc == NULL) {
			AG_SetErrorS("No drivers");
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
			Verbose("Syntax error: %s\n", sOpts);
		}
		while ((tok = AG_Strsep(&sOpts, ":")) != NULL) {
			if ((key = AG_Strsep(&tok, "=")) != NULL) {
				if ((val = AG_Strsep(&tok, "=")) != NULL) {
					AG_SetString(drv, key, val);
				} else {
					AG_SetString(drv, key, "");
				}
			}
		}
	}

	maDriverOps = dc;

	MA_InitGUI();
	return (0);
fail:
	MA_DestroyGUIGlobals();
	return (-1);
}

void
MA_DestroyGraphics(void)
{
	MA_DestroyGUI();
}
