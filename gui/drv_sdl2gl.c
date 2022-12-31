/*
 * Copyright (c) 2022 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Single-window driver for OpenGL graphics via SDL2.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/drv.h>
#include <agar/gui/text.h>
#include <agar/gui/window.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/cursors.h>
#include <agar/gui/opengl.h>
#include <agar/gui/sdl2.h>

#if defined(AG_WIDGETS) && defined(AG_DEBUG)
#include <agar/gui/box.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/tlist.h>
#include <agar/gui/notebook.h>
#endif

/* #define DEBUG_DISPLAY */
/* #define DEBUG_CAPTURE */

enum ag_sdl2gl_out {
	AG_SDL2GL_OUT_NONE,		/* No capture */
	AG_SDL2GL_OUT_JPEG,		/* Output JPEG files */
	AG_SDL2GL_OUT_PNG		/* Output PNG files */
};

typedef struct ag_sdl2gl_driver {
	struct ag_driver_sw _inherit;	/* AG_Driver -> AG_DriverSw */

	SDL_Window *_Nullable window;	 /* SDL window */
	AG_GL_Context gl;		 /* Common OpenGL context data */

	Uint8 *_Nullable   outBuf;	 /* Output capture buffer */
	char *_Nullable    outPath;	 /* Output capture path */
	enum ag_sdl2gl_out outMode;	 /* Output capture mode */
	Uint               outFrame;	 /* Capture frame# counter */
	Uint               outLast;	 /* Terminate after this many frames */
	Uint               outJpegQual;	 /* Quality (%) for jpeg output */
	Uint               outJpegFlags; /* DCT options */
	Uint32 _pad;
} AG_DriverSDL2GL;

static int nDrivers = 0;			/* Opened driver instances */
static int initedSDL = 0;			/* Inited TIMERS and EVENTS */
static int initedSDLVideo = 0;			/* Inited VIDEO */
static AG_EventSink *_Nullable sglEventSpinner = NULL;
static AG_EventSink *_Nullable sglEventEpilogue = NULL;

static void
Init(void *_Nonnull obj)
{
	AG_DriverSDL2GL *sgl = obj;

	sgl->window = NULL;
}

/*
 * Generic driver operations
 */

static int
SDL2GL_Open(void *_Nonnull obj, const char *_Nullable spec)
{
	AG_Driver *drv = obj;
	AG_DriverSDL2GL *sgl = obj;
	
	if (nDrivers != 0) {
		AG_SetError("Multiple SDL2 driver instances are not supported");
		return (-1);
	}

	/* Initialize SDL's video subsystem. */
	if (!initedSDL) {
		if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS) == -1) {
			AG_SetError("SDL_Init() failed: %s", SDL_GetError());
			return (-1);
		}
		initedSDL = 1;
	}
	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
			AG_SetError("SDL_INIT_VIDEO failed: %s", SDL_GetError());
			return (-1);
		}
		initedSDLVideo = 1;
	}
#if 0
	/* Use SDL's time interface. */
	AG_SetTimeOps(&agTimeOps_SDL);
	AG_DestroyEventSubsystem();
	AG_InitEventSubsystem(AG_SOFT_TIMERS);
#endif
	if ((drv->mouse = AG_MouseNew(sgl, "SDL2 mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(sgl, "SDL2 keyboard")) == NULL) {
		goto fail;
	}
	sgl->outBuf = NULL;
	sgl->outPath = NULL;
	sgl->outMode = AG_SDL2GL_OUT_NONE;
	sgl->outFrame = 0;
	sgl->outLast = 0;
	sgl->outJpegQual = 100;
	sgl->outJpegFlags = 0;
	
	/*
	 * TODO where AG_SINK_READ capability and pipes are available,
	 * could we create a separate thread running SDL_WaitEvent() and
	 * sending notifications over a pipe, instead of using a spinner?
	 */
	if ((sglEventSpinner = AG_AddEventSpinner(AG_SDL2_EventSink_SW, "%p", drv)) == NULL ||
	    (sglEventEpilogue = AG_AddEventEpilogue(AG_SDL2_EventEpilogue, NULL)) == NULL)
		goto fail;

	/* Set up event filters for standard AG_EventLoop(). */
	nDrivers = 1;
	return (0);
fail:
	if (sglEventSpinner != NULL) { AG_DelEventSpinner(sglEventSpinner); sglEventSpinner = NULL; }
	if (sglEventEpilogue != NULL) { AG_DelEventEpilogue(sglEventEpilogue); sglEventEpilogue = NULL; }
	if (drv->kbd != NULL) { AG_ObjectDelete(drv->kbd); drv->kbd = NULL; }
	if (drv->mouse != NULL) { AG_ObjectDelete(drv->mouse); drv->mouse = NULL; }
	return (-1);
}

static void
SDL2GL_Close(void *_Nonnull obj)
{
	AG_Driver *drv = obj;
	AG_DriverSDL2GL *sgl = obj;
	
	AG_DelEventSpinner(sglEventSpinner); sglEventSpinner = NULL;
	AG_DelEventEpilogue(sglEventEpilogue); sglEventEpilogue = NULL;

	if (drv->gl != NULL)
		AG_GL_DestroyContext(sgl);

#ifdef AG_DEBUG
	if (nDrivers != 1) { AG_FatalError("Driver close without open"); }
#endif
	AG_FreeCursors(AGDRIVER(sgl));

	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
	AG_ObjectDelete(drv->mouse); drv->mouse = NULL;
	AG_ObjectDelete(drv->kbd);   drv->kbd = NULL;

	if (sgl->outBuf != NULL) {
		free(sgl->outBuf);
		sgl->outBuf = NULL;
	}

	nDrivers = 0;
}

static void
SDL2GL_BeginRendering(void *_Nonnull obj)
{
	AG_DriverSDL2GL *sgl = obj;
	AG_GL_Context *gl = &sgl->gl;

	if (AGDRIVER_SW(sgl)->flags & AG_DRIVER_SW_OVERLAY) {
		AG_Rect r;
		AG_Driver *drv = obj;

		glPushAttrib(GL_VIEWPORT_BIT | GL_TRANSFORM_BIT |
		             GL_LIGHTING_BIT | GL_ENABLE_BIT);

		/* Reinitialize Agar's OpenGL context. */
		if (drv->gl != NULL) {
			AG_GL_DestroyContext(drv);
		}
		AG_GL_InitContext(drv, gl);
		r.x = 0;
		r.y = 0;
		r.w = AGDRIVER_SW(sgl)->w;
		r.h = AGDRIVER_SW(sgl)->h;
		AG_GL_SetViewport(gl, &r);
	} else {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

static void
SDL2GL_RenderWindow(struct ag_window *_Nonnull win)
{
	AG_WidgetDraw(win);
}

static void
SDL2GL_CaptureOutput(AG_DriverSDL2GL *_Nonnull sgl)
{
	char path[AG_PATHNAME_MAX];
	AG_DriverSw *dsw = (AG_DriverSw *)sgl;
	int w = dsw->w;
	int h = dsw->h;
	AG_Surface *S;

	Snprintf(path, sizeof(path), sgl->outPath, sgl->outFrame);
#ifdef DEBUG_CAPTURE
	Debug(sgl, "Capture(%s)\n", path);
#endif
	glReadPixels(0,0, w,h, GL_RGBA, GL_UNSIGNED_BYTE, sgl->outBuf);

	if (AG_PackedPixelFlip(sgl->outBuf, h,w*4) == -1) {
		goto fail_disable;
	}
	S = AG_SurfaceFromPixelsRGBA(sgl->outBuf, w,h, 32,
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000, 0);
	if (S == NULL)
		goto fail;

	switch (sgl->outMode) {
	case AG_SDL2GL_OUT_JPEG:
		if (AG_SurfaceExportJPEG(S, path, sgl->outJpegQual,
		    sgl->outJpegFlags) == -1) {
			goto fail;
		}
		break;
	case AG_SDL2GL_OUT_PNG:
		if (AG_SurfaceExportPNG(S, path, 0) == -1) {
			goto fail;
		}
		break;
	default:
		break;
	}

	if (++sgl->outFrame == sgl->outLast) {
		Verbose("SDL2GL: Reached last frame; terminating\n");
		AG_Terminate(0);
	}
	AG_SurfaceFree(S);
	return;
fail:
	AG_SurfaceFree(S);
fail_disable:
	Verbose("SDL2GL: %s; disabling capture\n", AG_GetError());
	sgl->outMode = AG_SDL2GL_OUT_NONE;
}

static void
SDL2GL_EndRendering(void *_Nonnull drv)
{
	AG_DriverSDL2GL *sgl = drv;
	AG_GL_Context *gl = &sgl->gl;
	int i;
	
	if (sgl->outMode != AG_SDL2GL_OUT_NONE)            /* Capture output */
		SDL2GL_CaptureOutput(sgl);

	/* Remove textures and display lists queued for deletion. */
	glDeleteTextures(gl->nTextureGC, (const GLuint *)gl->textureGC);
	for (i = 0; i < gl->nListGC; i++) {
		glDeleteLists(gl->listGC[i], 1);
	}
	gl->nTextureGC = 0;
	gl->nListGC = 0;

	if (AGDRIVER_SW(sgl)->flags & AG_DRIVER_SW_OVERLAY) {
		glPopAttrib();
		AG_GL_DestroyContext(&sgl->gl);     /* Restore former state */
	} else {
		SDL_GL_SwapWindow(sgl->window);
	}
}

/*
 * Operations specific to single-display drivers.
 */

static __inline__ void
ClearBackground(AG_DriverSw *_Nonnull dsw)
{
	glClearColor(dsw->bgColor.r/AG_COLOR_LASTF,
	             dsw->bgColor.g/AG_COLOR_LASTF,
		     dsw->bgColor.b/AG_COLOR_LASTF, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static int
SDL2GL_OpenVideo(void *_Nonnull obj, Uint w, Uint h, int depth, Uint flags)
{
	char buf[256];
	AG_Rect rVP;
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDL2GL *sgl = obj;
	SDL_Surface *Swin;
	Uint32 swFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

	/* Set the requested display options. */
	if (flags & AG_VIDEO_FULLSCREEN) {
		swFlags |= SDL_WINDOW_FULLSCREEN;
		dsw->flags |= AG_DRIVER_SW_FULLSCREEN;
	}
	if (flags & AG_VIDEO_NOFRAME)     { swFlags |= SDL_WINDOW_BORDERLESS; }
 	if (flags & AG_VIDEO_FIXED)       { swFlags &= ~(SDL_WINDOW_RESIZABLE); }

	if (flags & AG_VIDEO_OVERLAY)     { dsw->flags |= AG_DRIVER_SW_OVERLAY; }
	if (flags & AG_VIDEO_BGPOPUPMENU) { dsw->flags |= AG_DRIVER_SW_BGPOPUP; }
	
	if (AG_Defined(drv, "out")) {
		char *ext;

		AG_GetString(drv, "out", buf, sizeof(buf));
		if ((ext = strrchr(buf, '.')) != NULL &&
		    ext[1] != '\0') {
			if (Strcasecmp(&ext[1], "jpeg") == 0 ||
			    Strcasecmp(&ext[1], "jpg") == 0) {
				sgl->outMode = AG_SDL2GL_OUT_JPEG;
				if ((sgl->outPath = TryStrdup(buf)) == NULL)
					return (-1);
			} else if (Strcasecmp(&ext[1], "png") == 0) {
				sgl->outMode = AG_SDL2GL_OUT_PNG;
				if ((sgl->outPath = TryStrdup(buf)) == NULL)
					return (-1);
			} else {
				AG_SetError("Invalid out= argument: `%s'", buf);
				return (-1);
			}
			if (AG_Defined(drv, "outFirst")) {
				AG_GetString(drv, "outFirst", buf, sizeof(buf));
				sgl->outFrame = atoi(buf);
			} else {
				sgl->outFrame = 0;
			}
			if (AG_Defined(drv, "outLast")) {
				AG_GetString(drv, "outLast", buf, sizeof(buf));
				sgl->outLast = atoi(buf);
			}
		}
		if (AG_Defined(drv, "jpegQual")) {
			AG_GetString(drv, "jpegQual", buf, sizeof(buf));
			sgl->outJpegQual = atoi(buf);
		}
		if (AG_Defined(drv, "jpegDCT")) {
			AG_GetString(drv, "jpegDCT", buf, sizeof(buf));
			if (Strcasecmp(buf, "islow")) {
				sgl->outJpegFlags = AG_EXPORT_JPEG_JDCT_ISLOW;
			} else if (Strcasecmp(buf, "ifast")) {
				sgl->outJpegFlags = AG_EXPORT_JPEG_JDCT_IFAST;
			} else if (Strcasecmp(buf, "float")) {
				sgl->outJpegFlags = AG_EXPORT_JPEG_JDCT_FLOAT;
			}
		}
	}

	/* Set the video mode. Force hardware palette in 8bpp. */
	AG_SDL2_GetPrefDisplaySettings(drv, &w, &h, &depth);
#ifdef DEBUG_DISPLAY
	Debug(sgl, "Opened display (%u x %u x %d bpp)\n", w, h, depth);
#endif
	sgl->window = SDL_CreateWindow(agProgName,
	    SDL_WINDOWPOS_UNDEFINED,
	    SDL_WINDOWPOS_UNDEFINED,
	    w, h, swFlags);
	if (sgl->window == NULL) {
		AG_SetError("SDL_CreateWindow(%d,%d, 0x%x): %s", w, h, swFlags,
		    SDL_GetError());
		goto fail;
	}

	Swin = SDL_GetWindowSurface(sgl->window);
	drv->videoFmt = Malloc(sizeof(AG_PixelFormat));
	AG_PixelFormatRGBA(drv->videoFmt,
	    Swin->format->BitsPerPixel,
	    Swin->format->Rmask,
	    Swin->format->Gmask,
	    Swin->format->Bmask,
	    Swin->format->Amask);

	dsw->w = Swin->w;
	dsw->h = Swin->h;
	dsw->depth = Swin->format->BitsPerPixel;
#ifdef DEBUG_DISPLAY
	Debug(sgl, "New display (%d x %d x %d bpp)\n",
	    Swin->w, Swin->h, Swin->format->BitsPerPixel);
#endif
	/* Create the cursors. */
	AG_SDL2_InitDefaultCursor(sgl);
	AG_InitStockCursors(drv);
	
	/* Initialize our OpenGL context and viewport. */
	AG_GL_InitContext(sgl, &sgl->gl);
	rVP.x = 0;
	rVP.y = 0;
	rVP.w = dsw->w;
	rVP.h = dsw->h;
	AG_GL_SetViewport(&sgl->gl, &rVP);

	if (!(dsw->flags & AG_DRIVER_SW_OVERLAY))
		ClearBackground(dsw);

	/* Initialize the output capture buffer. */
	Free(sgl->outBuf);
	if (sgl->outMode != AG_SDL2GL_OUT_NONE) {
		if ((sgl->outBuf = AG_TryMalloc(dsw->w * dsw->h * 4)) == NULL) {
			Verbose("SDL2GL: Out of memory; disabling capture\n");
			sgl->outMode = AG_SDL2GL_OUT_NONE;
		}
	}
	return (0);
fail:
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	return (-1);
}

static int
SDL2GL_OpenVideoContext(void *_Nonnull obj, void *_Nonnull ctx, Uint flags)
{
	AG_SetError("openVideoContext() not implemented yet");
	return (-1);
}

static void
SDL2GL_CloseVideo(void *_Nonnull obj)
{
	AG_DriverSDL2GL *sgl = obj;

	SDL_DestroyWindow(sgl->window);

	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
}

static int
SDL2GL_VideoResize(void *_Nonnull obj, Uint w, Uint h)
{
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDL2GL *sgl = obj;
	AG_Rect rVP;
	AG_Window *win;
	int wRet, hRet;

	AG_FOREACH_WINDOW(win, sgl)                 /* Save mapped textures */
		AG_WidgetFreeResourcesGL(win);

	AG_TextClearGlyphCache(drv);

	SDL_SetWindowSize(sgl->window, w, h);
	SDL_GetWindowSize(sgl->window, &wRet, &hRet);
	dsw->w = wRet;
	dsw->h = hRet;
	
	if (sgl->outBuf != NULL) {          /* Resize output capture buffer */
		free(sgl->outBuf);
		if ((sgl->outBuf = AG_TryMalloc(dsw->w * dsw->h * 4)) == NULL) {
			Verbose("SDL2GL: Out of memory; disabling capture\n");
			sgl->outMode = AG_SDL2GL_OUT_NONE;
		}
	}

	rVP.x = 0;                                    /* Resize GL viewport */
	rVP.y = 0;
	rVP.w = dsw->w;
	rVP.h = dsw->h;
	AG_GL_SetViewport(&sgl->gl, &rVP);
	
	AG_FOREACH_WINDOW(win, sgl) {            /* Restore mapped textures */
		AG_WidgetRegenResourcesGL(win);
		win->dirty = 1;
	}

	if (!(dsw->flags & AG_DRIVER_SW_OVERLAY))
		ClearBackground(dsw);

	return (0);
}

static AG_Surface *
SDL2GL_VideoCapture(void *_Nonnull obj)
{
	const AG_DriverSw *dsw = obj;
	const Uint w = dsw->w;
	const Uint h = dsw->h;
	Uint8 *pixels;
	AG_Surface *S;

	if ((pixels = AG_TryMalloc(w*h*4)) == NULL) {
		return (NULL);
	}
	glReadPixels(0,0, w,h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	AG_PackedPixelFlip(pixels, h, w*4);

	S = AG_SurfaceFromPixelsRGBA(pixels, w,h, 32,
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000, 0);

	free(pixels);
	return (S);
}

static void
SDL2GL_VideoClear(void *_Nonnull obj, const AG_Color *_Nonnull c)
{
	glClearColor(c->r / AG_COLOR_LASTF,
	             c->g / AG_COLOR_LASTF,
		     c->b / AG_COLOR_LASTF, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static int
SDL2GL_SetVideoContext(void *_Nonnull obj, void *_Nonnull pSurface)
{
	AG_SetError("setVideoContext() not implemented yet");
	return (-1);
}

#if defined(AG_WIDGETS) && defined(AG_DEBUG)

static void
PollGLContext(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_DriverSDL2GL *sgl = (AG_DriverSDL2GL *)AG_PTR(1);
	const AG_GL_Context *ctx = &sgl->gl;
	AG_TlistItem *it;
	Uint i;

	if (!AG_OBJECT_VALID(sgl) ||
	    !AG_OfClass(sgl, "AG_Driver:AG_DriverSw:AG_DriverSDL2GL:*")) {
		if (WIDGET(tl)->window != NULL) {
			AG_ObjectDetach(WIDGET(tl)->window);
			return;
		}
		return;
	}

	AG_TlistBegin(tl);

	for (i = 0; i < ctx->nClipRects; i++) {
		AG_ClipRect *cr = &ctx->clipRects[i];

		it = AG_TlistAdd(tl, NULL,
		    _("Clipping Rectangle #" AGSI_BOLD "%d" AGSI_RST
		      " (%dx%d) at [" AGSI_BOLD "%d,%d" AGSI_BOLD "]"),
		    i, cr->r.x, cr->r.y, cr->r.w, cr->r.h);
		it->p1 = cr;
	}

	for (i = 0; i < ctx->nBlendStates; i++) {
		AG_GL_BlendState *bs = &ctx->blendStates[i];

		it = AG_TlistAdd(tl, NULL,
		    _("Blending State #" AGSI_BOLD "%d" AGSI_RST
		      " (" AGSI_BOLD "%s" AGSI_RST ", SrcFac=%d, DstFac=%d)"),
		    i, bs->enabled ? _("Enabled") : _("Disabled"),
		    bs->srcFactor, bs->dstFactor);
		it->p1 = bs;
	}

	for (i = 0; i < ctx->nTextureGC; i++) {
		it = AG_TlistAdd(tl, NULL,
		    _("Texture Delete #" AGSI_BOLD "%d" AGSI_RST " (Texture=%u)"),
		    i, ctx->textureGC[i]);
		it->p1 = &ctx->textureGC[i];
	}

	for (i = 0; i < ctx->nListGC; i++) {
		it = AG_TlistAdd(tl, NULL,
		    _("List Delete #" AGSI_BOLD "%d" AGSI_RST " (List=%u)"),
		    i, ctx->listGC[i]);
		it->p1 = &ctx->listGC[i];
	}

	AG_TlistEnd(tl);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	AG_DriverSDL2GL *sgl = obj;
	AG_Window *win;
	AG_Label *lbl;
	AG_Tlist *tl;
	AG_Keyboard *kbd = AGDRIVER(sgl)->kbd;
	AG_Mouse *mouse = AGDRIVER(sgl)->mouse;
	AG_Notebook *nb;
	AG_NotebookTab *nt;

	if ((win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	AG_WindowSetPosition(win, AG_WINDOW_BL, 0);

	lbl = AG_LabelNew(win, 0, _("SDL2GL Driver: %s"), OBJECT(sgl)->name);
	AG_SetStyle(lbl, "font-family", "cm-sans");
	AG_SetStyle(lbl, "font-size", "150%");

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);

	if (OBJECT_CLASS(kbd)->edit != NULL) {
		nt = AG_NotebookAdd(nb, _("Keyboard"), AG_BOX_VERT);
		AG_ObjectAttach(nt, OBJECT_CLASS(kbd)->edit(kbd));
	}
	if (OBJECT_CLASS(mouse)->edit != NULL) {
		nt = AG_NotebookAdd(nb, _("Mouse"), AG_BOX_VERT);
		AG_ObjectAttach(nt, OBJECT_CLASS(mouse)->edit(mouse));
	}
	nt = AG_NotebookAdd(nb, _("OpenGL"), AG_BOX_VERT);
	{
		AG_LabelNewS(nt, 0, _("Pushed GL States:"));
		tl = AG_TlistNewPolled(nt, AG_TLIST_EXPAND, PollGLContext,"%p",sgl);
		AG_SetStyle(tl, "font-size", "80%");
		AG_TlistSizeHint(tl, "<XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX>", 4);
	}
	return (win);
}
#endif /* AG_WIDGETS and AG_DEBUG */

AG_DriverSwClass agDriverSDL2GL = {
	{
		{
			"AG_Driver:AG_DriverSw:AG_DriverSDL2GL",
			sizeof(AG_DriverSDL2GL),
			{ 1,7 },
			Init,
			NULL,		/* reset */
			NULL,		/* destroy */
			NULL,		/* load */
			NULL,		/* save */
#if defined(AG_WIDGETS) && defined(AG_DEBUG)
			Edit
#else
			NULL		/* edit */
#endif
		},
		"sdl2gl",
		AG_VECTOR,
		AG_WM_SINGLE,
		AG_DRIVER_OPENGL | AG_DRIVER_TEXTURES | AG_DRIVER_SDL2,
		SDL2GL_Open,
		SDL2GL_Close,
		AG_SDL2_GetDisplaySize,
		AG_SDL2_BeginEventProcessing,
		AG_SDL2_PendingEvents,
		AG_SDL2_GetNextEvent,
		AG_SDL2_ProcessEvent_SW,
		NULL,				/* genericEventLoop */
		NULL,				/* endEventProcessing */
		NULL,				/* terminate */
		SDL2GL_BeginRendering,
		SDL2GL_RenderWindow,
		SDL2GL_EndRendering,
		AG_GL_FillRect,
		NULL,				/* updateRegion */
		AG_GL_StdUploadTexture,
		AG_GL_StdUpdateTexture,
		AG_GL_StdDeleteTexture,
		AG_SDL2_SetRefreshRate,
		AG_GL_StdPushClipRect,
		AG_GL_StdPopClipRect,
		AG_GL_StdPushBlendingMode,
		AG_GL_StdPopBlendingMode,
		AG_SDL2_CreateCursor,
		AG_SDL2_FreeCursor,
		AG_SDL2_SetCursor,
		AG_SDL2_UnsetCursor,
		AG_SDL2_GetCursorVisibility,
		AG_SDL2_SetCursorVisibility,
		AG_GL_BlitSurface,
		AG_GL_BlitSurfaceFrom,
#ifdef HAVE_OPENGL
		AG_GL_BlitSurfaceGL,
		AG_GL_BlitSurfaceFromGL,
		AG_GL_BlitSurfaceFlippedGL,
		AG_GL_BackupSurfaces,
		AG_GL_RestoreSurfaces,
#else
		NULL,                           /* backupSurfaces */
		NULL,                           /* restoreSurfaces */
#endif
		AG_GL_RenderToSurface,
		AG_GL_PutPixel,
		AG_GL_PutPixel32,
		AG_GL_PutPixelRGB8,
#if AG_MODEL == AG_LARGE
		AG_GL_PutPixel64,
		AG_GL_PutPixelRGB16,
#endif
		AG_GL_BlendPixel,
		AG_GL_DrawLine,
		AG_GL_DrawLineH,
		AG_GL_DrawLineV,
		AG_GL_DrawLineBlended,
		AG_GL_DrawLineW,
		AG_GL_DrawLineW_Sti16,
		AG_GL_DrawTriangle,
		AG_GL_DrawPolygon,
		AG_GL_DrawPolygon_Sti32,
		AG_GL_DrawArrow,
		AG_GL_DrawBoxRounded,
		AG_GL_DrawBoxRoundedTop,
		AG_GL_DrawCircle,
		AG_GL_DrawCircleFilled,
		AG_GL_DrawRectFilled,
		AG_GL_DrawRectBlended,
		AG_GL_DrawRectDithered,
		AG_GL_UpdateGlyph,
		AG_GL_DrawGlyph,
		AG_GL_StdDeleteList,
		NULL,				/* getClipboardText */
		NULL,				/* setClipboardText */
		NULL				/* setMouseAutoCapture */
	},
	0,
	SDL2GL_OpenVideo,
	SDL2GL_OpenVideoContext,
	SDL2GL_SetVideoContext,
	SDL2GL_CloseVideo,
	SDL2GL_VideoResize,
	SDL2GL_VideoCapture,
	SDL2GL_VideoClear
};
