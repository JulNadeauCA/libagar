/*
 * Copyright (c) 2009-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Base low-level AG_Driver(3) object instance.
 */

#include <agar/config/have_sdl.h>
#include <agar/config/have_opengl.h>
#include <agar/config/have_glx.h>
#include <agar/config/have_wgl.h>
#include <agar/config/have_cocoa.h>

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/gui/window.h>
#include <agar/gui/text.h>

/* Extra debugging output (rendering times in ms) */
/* #define DEBUG_RENDER */

#if defined(HAVE_GLX)
extern AG_DriverClass agDriverGLX;
#endif
#if defined(HAVE_SDL)
extern AG_DriverClass agDriverSDLFB;
#endif
#if defined(HAVE_SDL) && defined(HAVE_OPENGL)
extern AG_DriverClass agDriverSDLGL;
#endif
#if defined(HAVE_WGL)
extern AG_DriverClass agDriverWGL;
#endif
#if defined(HAVE_COCOA)
extern AG_DriverClass agDriverCocoa;
#endif
extern AG_DriverClass agDriverDUMMY;

AG_Object       agDrivers;			/* Drivers VFS */
AG_DriverClass *agDriverOps = NULL;		/* Current driver class */

AG_DriverClass *agDriverList[] = {
#if defined(HAVE_GLX)
	&agDriverGLX,
#endif
#if defined(HAVE_WGL)
	&agDriverWGL,
#endif
#if defined(HAVE_COCOA)
	&agDriverCocoa,
#endif
#if defined(HAVE_SDL) && defined(HAVE_OPENGL)
	&agDriverSDLGL,
#endif
#if defined(HAVE_SDL)
	&agDriverSDLFB,
#endif
	&agDriverDUMMY,
	NULL
};

const char *agDriverTypeNames[] = {
	N_("Framebuffer"),
	N_("Vector")
};
const char *agDriverWmTypeNames[] = {
	N_("Single-window"),
	N_("Multi-window")
};

/* Return a string with the available drivers. */
void
AG_ListDriverNames(char *buf, AG_Size buf_len)
{
	AG_DriverClass **pd;

	if (buf_len == 0) {
		return;
	}
	for (pd = &agDriverList[0], buf[0] = '\0';
	    *pd != NULL;
	     pd++) {
		Strlcat(buf, (*pd)->name, buf_len);
		Strlcat(buf, " ", buf_len);
	}
	buf[strlen(buf)-1] = '\0';
}

/* Create a new driver instance. */
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
 * Lookup a driver instance by ID.
 * The agDrivers VFS must be locked.
 */
AG_Driver *
AG_GetDriverByID(Uint id)
{
	AG_Driver *drv;

	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		if (drv->id == id)
			return (drv);
	}
	return (NULL);
}

/*
 * Enter GUI rendering context. In a standard event loop, single-window drivers
 * will invoke this function once per frame (rendering all windows in batch).
 * Multi-window drivers will invoke it several times (once per window).
 */
void
AG_BeginRendering(void *drv)
{
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_PTHREADS)
	if (agTimeOps == &agTimeOps_renderer)		/* Renderer-aware ops */
		AG_CondBroadcast(&agCondBeginRender);
#endif

	agRenderingContext = 1;

#ifdef DEBUG_RENDER
	AGDRIVER(drv)->tRender = AG_GetTicks();
#endif
	AGDRIVER_CLASS(drv)->beginRendering(drv);
}

/* Leave GUI rendering context. */
void
AG_EndRendering(void *drv)
{
	AGDRIVER_CLASS(drv)->endRendering(drv);

	agRenderingContext = 0;

#ifdef DEBUG_RENDER
	Debug(drv, "Rendered in %u ms\n", AG_GetTicks() - AGDRIVER(drv)->tRender);
#endif

#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_PTHREADS)
	if (agTimeOps == &agTimeOps_renderer)		/* Renderer-aware ops */
		AG_CondBroadcast(&agCondEndRender);
#endif
}

/*
 * Dump the display surface(s) to a jpeg in ~/.appname/screenshot/.
 * It is customary to assign a AG_GlobalKeys(3) shortcut for this function.
 */
void
AG_ViewCapture(void)
{
	char dir[AG_PATHNAME_MAX];
	char file[AG_FILENAME_MAX+7];
	AG_Surface *S;
	char *pname;
	Uint seq;

	if (agDriverSw == NULL) {
		AG_TextError(_("Screenshot is currently only available "
		               "with single-window drivers\n"));
		return;
	}

	AG_LockVFS(&agDrivers);

	if ((S = AGDRIVER_SW_CLASS(agDriverSw)->videoCapture(agDriverSw)) == NULL) {
		AG_TextError(_("%s: Screenshot failed (%s)"),
		    OBJECT(agDriverSw)->name, AG_GetError());
		goto out;
	}

	/* Save to a new file. */
	if (AG_ConfigGetPath(AG_CONFIG_PATH_DATA, 0, dir, sizeof(dir)) >= sizeof(dir)) {
		AG_TextError(_("Path overflow"));
		goto out;
	}
	Strlcat(dir, AG_PATHSEP, sizeof(dir));
	Strlcat(dir, "screenshot", sizeof(dir));
	if (!AG_FileExists(dir) && AG_MkPath(dir) == -1) {
		AG_TextError(_("Screenshot failed: %s"), AG_GetError());
		goto out;
	}
	pname = (agProgName != NULL) ? agProgName : "agar";
	for (seq = 0; ; seq++) {
		Snprintf(file, sizeof(file), "%s%c%s%u.jpg",
		    dir, AG_PATHSEPCHAR, pname, seq);

		if (!AG_FileExists(file))
			break;			/* XXX race condition */
	}
	if (AG_SurfaceExportJPEG(S, file, 100, 0) == 0) {
		Verbose(_("%s: Saved %u x %u x %ubpp screenshot to %s\n"),
		    OBJECT(agDriverSw)->name,
		    S->w, S->h, S->format.BitsPerPixel, AG_ShortFilename(file));
	} else {
		AG_TextError(_("Screenshot failed (%s)"), AG_GetError());
	}
	AG_SurfaceFree(S);
out:
	AG_UnlockVFS(&agDrivers);
}

/* Return whether Agar is using OpenGL. */
int
AG_UsingGL(void *drv)
{
	if (drv != NULL) {
		return (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL);
	} else {
		return (agDriverOps->flags & AG_DRIVER_OPENGL);
	}
}

/* Return whether Agar is using SDL. */
int
AG_UsingSDL(void *drv)
{
	AG_DriverClass *dc = (drv != NULL) ? AGDRIVER_CLASS(drv) :
	                                     agDriverOps;

	return (dc->flags & AG_DRIVER_SDL);
}

/* Return the resolution (px) of the parent display device, if applicable. */
int
AG_GetDisplaySize(void *drv, Uint *w, Uint *h)
{
	AG_DriverClass *dc = (drv != NULL) ? AGDRIVER_CLASS(drv) : agDriverOps;
	AG_DriverSw *dsw = (drv != NULL) ? (AG_DriverSw *)drv : agDriverSw;

	switch (dc->wm) {
	case AG_WM_SINGLE:
		*w = dsw->w;
		*h = dsw->h;
		return (0);
	case AG_WM_MULTIPLE:
		return dc->getDisplaySize(w, h);
	}
	return (-1);
}

static void
Init(void *_Nonnull obj)
{
	AG_Driver *drv = obj;
	Uint i;

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
	drv->glyphCache = Malloc(AG_GLYPH_NBUCKETS*sizeof(AG_GlyphCache));
	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		SLIST_INIT(&drv->glyphCache[i].glyphs);
	}
	drv->gl = NULL;
	drv->activeCursor = NULL;
	TAILQ_INIT(&drv->cursors);
	drv->nCursors = 0;
}

static void
Destroy(void *_Nonnull obj)
{
	AG_Driver *drv = obj;

	if (drv->sRef != NULL) {
		AG_SurfaceFree(drv->sRef);
	}
	if (drv->videoFmt != NULL) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
	}
	AG_TextClearGlyphCache(drv);
	free(drv->glyphCache);
	drv->glyphCache = NULL;
}

AG_ObjectClass agDriverClass = {
	"AG_Driver",
	sizeof(AG_Driver),
	{ 1,6 },
	Init,
	NULL,		/* reset */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
