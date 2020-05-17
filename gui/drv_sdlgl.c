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
 * Driver for OpenGL graphics via the SDL 1.2 library.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/drv.h>
#include <agar/gui/text.h>
#include <agar/gui/window.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/cursors.h>
#include <agar/gui/opengl.h>
#include <agar/gui/sdl.h>

enum ag_sdlgl_out {
	AG_SDLGL_OUT_NONE,		/* No capture */
	AG_SDLGL_OUT_JPEG,		/* Output JPEG files */
	AG_SDLGL_OUT_PNG		/* Output PNG files */
};

typedef struct ag_sdlgl_driver {
	struct ag_driver_sw _inherit;

	SDL_Surface *_Nullable s;	/* Display surface */
	AG_GL_Context gl;		/* Common OpenGL context data */

	Uint8 *_Nullable  outBuf;	/* Output capture buffer */
	char *_Nullable   outPath;	/* Output capture path */
	enum ag_sdlgl_out outMode;	/* Output capture mode */
	Uint              outFrame;	/* Capture frame# counter */
	Uint              outLast;	/* Terminate after this many frames */
	Uint              outJpegQual;	/* Quality (%) for jpeg output */
	Uint              outJpegFlags;	/* DCT options */
	Uint32 _pad;
} AG_DriverSDLGL;

static int nDrivers = 0;				/* Opened driver instances */
static int initedSDL = 0;				/* Used SDL_Init() */
static int initedSDLVideo = 0;				/* Used SDL_INIT_VIDEO */
static AG_EventSink *_Nullable sglEventSpinner = NULL;	/* Standard event sink */
static AG_EventSink *_Nullable sglEventEpilogue = NULL;	/* Standard event epilogue */

static void
Init(void *_Nonnull obj)
{
	AG_DriverSDLGL *sgl = obj;

	sgl->s = NULL;
}

/*
 * Generic driver operations
 */

static int
SDLGL_Open(void *_Nonnull obj, const char *_Nullable spec)
{
	AG_Driver *drv = obj;
	AG_DriverSDLGL *sgl = obj;
	
	if (nDrivers != 0) {
		AG_SetError("Multiple SDL displays are not supported");
		return (-1);
	}

	/* Initialize SDL's video subsystem. */
	if (!initedSDL) {
		if (SDL_Init(0) == -1) {
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
	/* Initialize this driver instance. */
	if ((drv->mouse = AG_MouseNew(sgl, "SDL mouse")) == NULL ||
	    (drv->kbd = AG_KeyboardNew(sgl, "SDL keyboard")) == NULL) {
		goto fail;
	}
	sgl->outBuf = NULL;
	sgl->outPath = NULL;
	sgl->outMode = AG_SDLGL_OUT_NONE;
	sgl->outFrame = 0;
	sgl->outLast = 0;
	sgl->outJpegQual = 100;
	sgl->outJpegFlags = 0;
	
	/* Configure the window caption */
	if (agProgName != NULL)
		SDL_WM_SetCaption(agProgName, agProgName);

	/*
	 * TODO where AG_SINK_READ capability and pipes are available,
	 * could we create a separate thread running SDL_WaitEvent() and
	 * sending notifications over a pipe, instead of using a spinner?
	 */
	if ((sglEventSpinner = AG_AddEventSpinner(AG_SDL_EventSink, "%p", drv)) == NULL ||
	    (sglEventEpilogue = AG_AddEventEpilogue(AG_SDL_EventEpilogue, NULL)) == NULL)
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
SDLGL_Close(void *_Nonnull obj)
{
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDLGL *sgl = obj;
	
	AG_DelEventSpinner(sglEventSpinner); sglEventSpinner = NULL;
	AG_DelEventEpilogue(sglEventEpilogue); sglEventEpilogue = NULL;

	if (drv->gl != NULL)
		AG_GL_DestroyContext(sgl);

#ifdef AG_DEBUG
	if (nDrivers != 1) { AG_FatalError("Driver close without open"); }
#endif
	AG_FreeCursors(AGDRIVER(sgl));

	if (dsw->flags & AG_DRIVER_SW_FULLSCREEN) {
		if (SDL_WM_ToggleFullScreen(sgl->s))
			dsw->flags &= ~(AG_DRIVER_SW_FULLSCREEN);
	}
	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
	AG_ObjectDelete(drv->mouse); drv->mouse = NULL;
	AG_ObjectDelete(drv->kbd); drv->kbd = NULL;

	if (sgl->outBuf != NULL) {
		free(sgl->outBuf);
		sgl->outBuf = NULL;
	}

	nDrivers = 0;
}

static void
SDLGL_BeginRendering(void *_Nonnull obj)
{
	AG_DriverSDLGL *sgl = obj;
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
SDLGL_RenderWindow(struct ag_window *_Nonnull win)
{
	AG_WidgetDraw(win);
}

static void
SDLGL_CaptureOutput(AG_DriverSDLGL *_Nonnull sgl)
{
	char path[AG_PATHNAME_MAX];
	AG_DriverSw *dsw = (AG_DriverSw *)sgl;
	int w = dsw->w;
	int h = dsw->h;
	AG_Surface *S;

	Snprintf(path, sizeof(path), sgl->outPath, sgl->outFrame);
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
	case AG_SDLGL_OUT_JPEG:
		if (AG_SurfaceExportJPEG(S, path, sgl->outJpegQual,
		    sgl->outJpegFlags) == -1) {
			goto fail;
		}
		break;
	case AG_SDLGL_OUT_PNG:
		if (AG_SurfaceExportPNG(S, path, 0) == -1) {
			goto fail;
		}
		break;
	default:
		break;
	}

	if (++sgl->outFrame == sgl->outLast) {
		Verbose("SDLGL: Reached last frame; terminating\n");
		AG_Terminate(0);
	}
	AG_SurfaceFree(S);
	return;
fail:
	AG_SurfaceFree(S);
fail_disable:
	Verbose("SDLGL: %s; disabling capture\n", AG_GetError());
	sgl->outMode = AG_SDLGL_OUT_NONE;
}

static void
SDLGL_EndRendering(void *_Nonnull drv)
{
	AG_DriverSDLGL *sgl = drv;
	
	if (sgl->outMode != AG_SDLGL_OUT_NONE)            /* Capture output */
		SDLGL_CaptureOutput(sgl);

	if (AGDRIVER_SW(sgl)->flags & AG_DRIVER_SW_OVERLAY) {
		glPopAttrib();
		AG_GL_DestroyContext(&sgl->gl);     /* Restore former state */
	} else {
		SDL_GL_SwapBuffers();
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
SDLGL_OpenVideo(void *_Nonnull obj, Uint w, Uint h, int depth, Uint flags)
{
	char buf[256];
	AG_Rect rVP;
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDLGL *sgl = obj;
	Uint32 sFlags = SDL_OPENGL;
	int newDepth;

	/* Set the requested display options. */
	if (flags & AG_VIDEO_RESIZABLE) { sFlags |= SDL_RESIZABLE; }
	if (flags & AG_VIDEO_ANYFORMAT) { sFlags |= SDL_ANYFORMAT; }
	if (flags & AG_VIDEO_HWPALETTE) { sFlags |= SDL_HWPALETTE; }
	if (flags & AG_VIDEO_DOUBLEBUF) { sFlags |= SDL_DOUBLEBUF; }
	if (flags & AG_VIDEO_FULLSCREEN) { sFlags |= SDL_FULLSCREEN; }
	if (flags & AG_VIDEO_NOFRAME) { sFlags |= SDL_NOFRAME; }
	
	if (flags & AG_VIDEO_OVERLAY)
		dsw->flags |= AG_DRIVER_SW_OVERLAY;
	if (flags & AG_VIDEO_BGPOPUPMENU)
		dsw->flags |= AG_DRIVER_SW_BGPOPUP;
	
	if (AG_Defined(drv, "out")) {
		char *ext;

		AG_GetString(drv, "out", buf, sizeof(buf));
		if ((ext = strrchr(buf, '.')) != NULL &&
		    ext[1] != '\0') {
			if (Strcasecmp(&ext[1], "jpeg") == 0 ||
			    Strcasecmp(&ext[1], "jpg") == 0) {
				sgl->outMode = AG_SDLGL_OUT_JPEG;
				if ((sgl->outPath = TryStrdup(buf)) == NULL)
					return (-1);
			} else if (Strcasecmp(&ext[1], "png") == 0) {
				sgl->outMode = AG_SDLGL_OUT_PNG;
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
	AG_SDL_GetPrefDisplaySettings(drv, &w, &h, &depth);
	Verbose(_("SDLGL: Setting mode %ux%u (%d bpp)\n"), w, h, depth);
	newDepth = SDL_VideoModeOK(w, h, depth, sFlags);
	if (newDepth == 8) {
		Verbose(_("SDLGL: Using hardware palette\n"));
		sFlags |= SDL_HWPALETTE;
	}
	if ((sgl->s = SDL_SetVideoMode((int)w, (int)h, newDepth, sFlags))
	    == NULL) {
		AG_SetError("Setting %dx%dx%d mode: %s", w, h, newDepth,
		    SDL_GetError());
		return (-1);
	}
	SDL_EnableUNICODE(1);

	if ((drv->videoFmt = AG_SDL_GetPixelFormat(sgl->s)) == NULL) {
		goto fail;
	}
	dsw->w = sgl->s->w;
	dsw->h = sgl->s->h;
	dsw->depth = (Uint)drv->videoFmt->BitsPerPixel;

	Verbose(_("SDLGL: New display (%dbpp)\n"),
	     (int)drv->videoFmt->BitsPerPixel);
	
	/* Create the cursors. */
	AG_SDL_InitDefaultCursor(sgl);
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
	if (sgl->outMode != AG_SDLGL_OUT_NONE) {
		if ((sgl->outBuf = AG_TryMalloc(dsw->w*dsw->h*4)) == NULL) {
			Verbose("SDLGL: Out of memory; disabling capture\n");
			sgl->outMode = AG_SDLGL_OUT_NONE;
		}
	}
	
	if (flags & AG_VIDEO_FULLSCREEN) {
		if (SDL_WM_ToggleFullScreen(sgl->s))
			dsw->flags |= AG_DRIVER_SW_FULLSCREEN;
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
SDLGL_OpenVideoContext(void *_Nonnull obj, void *_Nonnull ctx, Uint flags)
{
	AG_DriverSDLGL *sgl = obj;
	AG_DriverSw *dsw = obj;
	AG_Driver *drv = obj;
	AG_Rect rVP;
	SDL_Surface *ctxSu = (SDL_Surface *)ctx;

	if (!(ctxSu->flags & SDL_OPENGL)) {
		AG_SetError("Given display surface is not SDL_OPENGL");
		return (-1);
	}

	/* Set the requested display options. */
	if (flags & AG_VIDEO_OVERLAY)
		dsw->flags |= AG_DRIVER_SW_OVERLAY;
	if (flags & AG_VIDEO_BGPOPUPMENU)
		dsw->flags |= AG_DRIVER_SW_BGPOPUP;

	/* Use the given display surface. */
	sgl->s = (SDL_Surface *)ctx;
	if ((drv->videoFmt = AG_SDL_GetPixelFormat(sgl->s)) == NULL) {
		goto fail;
	}
	dsw->w = sgl->s->w;
	dsw->h = sgl->s->h;
	dsw->depth = (Uint)drv->videoFmt->BitsPerPixel;

	Verbose(_("SDLGL: Using existing display (%ux%ux%d bpp)\n"),
	    dsw->w, dsw->h, (int)drv->videoFmt->BitsPerPixel);
	
	/* Initialize our OpenGL context and viewport. */
	AG_GL_InitContext(sgl, &sgl->gl);
	rVP.x = 0;
	rVP.y = 0;
	rVP.w = dsw->w;
	rVP.h = dsw->h;
	AG_GL_SetViewport(&sgl->gl, &rVP);

	/* Create the cursors. */
	AG_SDL_InitDefaultCursor(sgl);
	AG_InitStockCursors(drv);
	
	return (0);
fail:
	if (drv->videoFmt) {
		AG_PixelFormatFree(drv->videoFmt);
		free(drv->videoFmt);
		drv->videoFmt = NULL;
	}
	return (-1);
}

static void
SDLGL_CloseVideo(void *_Nonnull obj)
{
	if (initedSDLVideo) {
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		initedSDLVideo = 0;
	}
}

static int
SDLGL_VideoResize(void *_Nonnull obj, Uint w, Uint h)
{
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	AG_DriverSDLGL *sgl = obj;
	AG_Rect rVP;
	Uint32 sFlags;
	SDL_Surface *S;
	AG_Window *win;

	Debug(sgl, "VideoResize event (%u x %u)\n", w,h);

	sFlags = sgl->s->flags & (SDL_SWSURFACE | SDL_HWSURFACE | SDL_ASYNCBLIT |
	                          SDL_ANYFORMAT | SDL_HWPALETTE | SDL_DOUBLEBUF |
	                          SDL_FULLSCREEN | SDL_OPENGL | SDL_OPENGLBLIT |
	                          SDL_RESIZABLE | SDL_NOFRAME);

	AG_FOREACH_WINDOW(win, sgl)                 /* Save mapped textures */
		AG_WidgetFreeResourcesGL(win);

	AG_TextClearGlyphCache(drv);
	
	if ((S = SDL_SetVideoMode(w, h, 0, sFlags)) == NULL) {
		AG_SetError("SDL_SetVideoMode(%ux%u): %s", w,h, SDL_GetError());
		return (-1);
	}
	sgl->s = S;

	Debug(sgl, "Resized to: %u x %u x %d-bpp (flags 0x%x)\n",
	    S->w, S->h, S->format->BitsPerPixel, S->flags);

	dsw->w = S->w;
	dsw->h = S->h;
	dsw->depth = (Uint)S->format->BitsPerPixel;

	if (sgl->outBuf != NULL) {          /* Resize output capture buffer */
		free(sgl->outBuf);
		if ((sgl->outBuf = AG_TryMalloc(dsw->w * dsw->h * 4)) == NULL) {
			Verbose("SDLGL: Out of memory; disabling capture\n");
			sgl->outMode = AG_SDLGL_OUT_NONE;
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
SDLGL_VideoCapture(void *_Nonnull obj)
{
	const AG_DriverSw *dsw = obj;
	const Uint w = dsw->w;
	const Uint h = dsw->h;
	Uint8 *pixels;
	AG_Surface *S;

	if ((pixels = AG_TryMalloc((w * h) << 2)) == NULL) {
		return (NULL);
	}
	glReadPixels(0,0, w,h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	AG_PackedPixelFlip(pixels, h, (w << 2));

	S = AG_SurfaceFromPixelsRGBA(pixels, w,h, 32,
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000, 0);

	free(pixels);
	return (S);
}

static void
SDLGL_VideoClear(void *_Nonnull obj, const AG_Color *_Nonnull c)
{
	glClearColor(c->r / AG_COLOR_LASTF,
	             c->g / AG_COLOR_LASTF,
		     c->b / AG_COLOR_LASTF, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static int
SDLGL_SetVideoContext(void *_Nonnull obj, void *_Nonnull pSurface)
{
	AG_DriverSDLGL *sgl = obj;
	AG_GL_Context *gl = &sgl->gl;
	AG_DriverSw *dsw = obj;
	SDL_Surface *S = pSurface;

	sgl->s = S;
	dsw->w = S->w;
	dsw->h = S->h;
	dsw->depth = (Uint)S->format->BitsPerPixel;

	if (dsw->flags & AG_DRIVER_SW_OVERLAY) {
		AG_ClipRect *cr0 = &gl->clipRects[0];

		/* Just update clipping rectangle 0. */
		cr0->r.w = S->w;
		cr0->r.h = S->h;
	} else {
		AG_Rect rVP;

		rVP.x = 0;
		rVP.y = 0;
		rVP.w = S->w;
		rVP.h = S->h;
		AG_GL_SetViewport(gl, &rVP);
	}
	return (0);
}

AG_DriverSwClass agDriverSDLGL = {
	{
		{
			"AG_Driver:AG_DriverSw:AG_DriverSDLGL",
			sizeof(AG_DriverSDLGL),
			{ 1,6 },
			Init,
			NULL,		/* reset */
			NULL,		/* destroy */
			NULL,		/* load */
			NULL,		/* save */
			NULL,		/* edit */
		},
		"sdlgl",
		AG_VECTOR,
		AG_WM_SINGLE,
		AG_DRIVER_SDL | AG_DRIVER_OPENGL | AG_DRIVER_TEXTURES,
		SDLGL_Open,
		SDLGL_Close,
		AG_SDL_GetDisplaySize,
		AG_SDL_BeginEventProcessing,
		AG_SDL_PendingEvents,
		AG_SDL_GetNextEvent,
		AG_SDL_ProcessEvent,
		NULL,				/* genericEventLoop */
		NULL,				/* endEventProcessing */
		NULL,				/* terminate */
		SDLGL_BeginRendering,
		SDLGL_RenderWindow,
		SDLGL_EndRendering,
		AG_GL_FillRect,
		NULL,				/* updateRegion */
		AG_GL_StdUploadTexture,
		AG_GL_StdUpdateTexture,
		AG_GL_StdDeleteTexture,
		AG_SDL_SetRefreshRate,
		AG_GL_StdPushClipRect,
		AG_GL_StdPopClipRect,
		AG_GL_StdPushBlendingMode,
		AG_GL_StdPopBlendingMode,
		AG_SDL_CreateCursor,
		AG_SDL_FreeCursor,
		AG_SDL_SetCursor,
		AG_SDL_UnsetCursor,
		AG_SDL_GetCursorVisibility,
		AG_SDL_SetCursorVisibility,
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
		NULL				/* setClipboardText */
	},
	0,
	SDLGL_OpenVideo,
	SDLGL_OpenVideoContext,
	SDLGL_SetVideoContext,
	SDLGL_CloseVideo,
	SDLGL_VideoResize,
	SDLGL_VideoCapture,
	SDLGL_VideoClear
};
