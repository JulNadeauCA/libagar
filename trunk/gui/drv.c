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
 * Generic graphics/input driver framework.
 */

#include <config/have_sdl.h>
#include <config/have_opengl.h>
#include <config/have_glx.h>

#include <core/core.h>
#include <core/config.h>

#include "window.h"

#ifdef HAVE_GLX
extern AG_Driver agDriverGLX;
#endif
#ifdef HAVE_SDL
extern AG_Driver agDriverSDLFB;
#endif

AG_Object         agDrivers;			/* Drivers VFS */
AG_DriverClass   *agDriverOps = NULL;
AG_DriverSwClass *agDriverOpsSw = NULL;
AG_DriverMwClass *agDriverOpsMw = NULL;
AG_Driver        *agDriver = NULL;
#ifdef AG_LEGACY
AG_Driver        *agView = NULL;  		/* Pre-1.4 */
#endif
AG_DriverSw      *agDriverSw = NULL;
AG_DriverMw      *agDriverMw = NULL;

void *agDriverList[] = {
#ifdef HAVE_GLX
	&agDriverGLX,
#endif
#ifdef HAVE_SDL
	&agDriverSDLFB,
# if 0
# ifdef HAVE_OPENGL
	&agDriverSDLGL,
# endif
# endif
#endif
};
Uint agDriverListSize = sizeof(agDriverList) / sizeof(agDriverList[0]);

/* Create an instance of the given driver class, if it opens successfully. */
AG_Driver *
AG_DriverOpen(AG_DriverClass *dc)
{
	AG_Driver *drv;

	if ((drv = AG_ObjectNew(NULL, dc->name, AGCLASS(dc))) == NULL) {
		return (NULL);
	}
	if (dc->open(drv, NULL) == -1) {
		AG_ObjectDestroy(drv);
		return (NULL);
	}
	for (drv->id = 1; ; drv->id++) {
		if (AG_GetDriverByID(drv->id) == NULL)
			break;
	}
	AG_ObjectSetName(drv, "%s%u", dc->name, drv->id);
	AG_ObjectAttach(&agDrivers, drv);
	return (drv);
}

/* Close and destroy a driver. */
void
AG_DriverClose(AG_Driver *drv)
{
	AG_ObjectDetach(drv);
	AGDRIVER_CLASS(drv)->close(drv);
	AG_ObjectDestroy(drv);
}

/*
 * Dump the display surface(s) to a jpeg in ~/.appname/screenshot/.
 * Typically called via AG_GlobalKeys(3).
 */
void
AG_ViewCapture(void)
{
	AG_Surface *s;
	char dir[AG_PATHNAME_MAX];
	char file[AG_PATHNAME_MAX];
	Uint seq;

	/* Get a copy of the display. */
	switch (AGDRIVER_CLASS(agDriver)->wm) {
	case AG_WM_SINGLE:
		if (AGDRIVER_SW_CLASS(agDriver)->videoCapture(agDriver, &s)
		    == -1) {
			Verbose("Capture failed: %s\n", AG_GetError());
			return;
		}
		break;
	case AG_WM_MULTIPLE:
		/* TODO iterate over each window. */
		Verbose("Capture not implemented (AG_WM_MULTIPLE)\n");
		break;
	}

	/* Save to a new file. */
	AG_GetString(agConfig, "save-path", dir, sizeof(dir));
	Strlcat(dir, AG_PATHSEP, sizeof(dir));
	Strlcat(dir, "screenshot", sizeof(dir));
	if (!AG_FileExists(dir) && AG_MkPath(dir) == -1) {
		Verbose("Capture failed: %s\n", AG_GetError());
		return;
	}
	for (seq = 0; ; seq++) {
		Snprintf(file, sizeof(file), "%s%c%s%u.jpg",
		    dir, AG_PATHSEPCHAR, agProgName, seq++);
		if (!AG_FileExists(file))
			break;			/* XXX race condition */
	}
	if (AG_SurfaceExportJPEG(s, file) == 0) {
		Verbose("Saved capture to: %s\n", file);
	} else {
		Verbose("Capture failed: %s\n", AG_GetError());
	}
	AG_SurfaceFree(s);
}

static void
Init(void *obj)
{
	AG_Driver *drv = obj;

	drv->id = 0;
	drv->flags = 0;
	drv->sRef = AG_SurfaceRGBA(1,1, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
 	    0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
	    0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
	);
	if (drv->sRef == NULL) {
		AG_FatalError(NULL);
	}
	drv->videoFmt = NULL;
	drv->kbd = NULL;
	drv->mouse = NULL;
	drv->cursors = NULL;
	drv->nCursors = 0;
	drv->activeCursor = NULL;
}

static void
Destroy(void *obj)
{
	AG_Driver *drv = obj;
	
	if (drv->sRef != NULL)
		AG_SurfaceFree(drv->sRef);
	if (drv->videoFmt != NULL)
		AG_PixelFormatFree(drv->videoFmt);
}

AG_ObjectClass agDriverClass = {
	"AG_Driver",
	sizeof(AG_Driver),
	{ 1,4 },
	Init,
	NULL,		/* reinit */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};