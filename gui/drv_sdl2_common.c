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
 * Common code for SDL 2.0 drivers.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/cursors.h>
#include <agar/gui/sdl2.h>
#include <agar/gui/text.h>

#define AG_SDL2_CLIPPED_PIXEL(s, ax, ay)		\
	((ax) <  (s)->clip_rect.x ||			\
	 (ax) >= (s)->clip_rect.x + (s)->clip_rect.w ||	\
	 (ay) <  (s)->clip_rect.y ||			\
	 (ay) >= (s)->clip_rect.y + (s)->clip_rect.h)

/* #define DEBUG_EVENTS */
/* #define DEBUG_KEYBOARD */
/* #define DEBUG_MOUSE */

static void AG_SDL2_SoftBlit_Colorkey(const AG_Surface *_Nonnull, AG_Rect,
                                      SDL_Surface *_Nonnull, AG_Rect);
static void AG_SDL2_SoftBlit_NoColorkey(const AG_Surface *_Nonnull, AG_Rect,
                                        SDL_Surface *_Nonnull, AG_Rect);

#if 0
/*
 * Initialize Agar with an existing SDL display. If the display surface has
 * flags SDL_OPENGL set, the "sdl2gl" driver is selected; otherwise, the
 * "sdl2fb" driver is used.
 */
int
AG_InitVideoSDL2(void *pDisplay, Uint flags)
{
	SDL_Surface *display = pDisplay;
	AG_Driver *drv = NULL;
	AG_DriverClass *dc = NULL, **pd;
	int useGL = 0;

	if (AG_InitGUIGlobals() == -1)
		return (-1);
	
	/* Enable OpenGL mode if the surface has SDL_OPENGL set. */
	if ((display->flags & SDL_OPENGL)) {
		if (flags & AG_VIDEO_SDL) {
			AG_SetError("AG_VIDEO_SDL flag requested, but "
			            "display surface has SDL_OPENGL set");
			goto fail;
		}
		useGL = 1;
	} else {
		if (flags & AG_VIDEO_OPENGL) {
			AG_SetError("AG_VIDEO_OPENGL flag requested, but "
			            "display surface is missing SDL_OPENGL");
			goto fail;
		}
	}
	for (pd = &agDriverList[0]; *pd != NULL; pd++) {
		if (((*pd)->wm == AG_WM_SINGLE) &&
		    ((*pd)->flags & AG_DRIVER_SDL2)) {
			if (useGL) {
				if (!((*pd)->flags & AG_DRIVER_OPENGL))
					continue;
			} else {
				if ((*pd)->flags & AG_DRIVER_OPENGL)
					continue;
			}
			dc = *pd;
			break;
		}
	}
	if (dc == NULL) {
		AG_SetError("No compatible %s driver is available",
		    useGL ? "SDL2/OpenGL" : "SDL2");
		goto fail;
	}
	if ((drv = AG_DriverOpen(dc)) == NULL)
		goto fail;

	/* Open a video display. */
	if (AGDRIVER_SW_CLASS(drv)->openVideoContext(drv, (void *)display,
	    flags) == -1) {
		AG_DriverClose(drv);
		goto fail;
	}
	if (drv->videoFmt == NULL)
		AG_FatalError("Driver did not set video format");

	/* Generic Agar-GUI initialization. */
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

/*
 * Reattach to a different SDL display surface.
 */
int
AG_SetVideoSurfaceSDL2(void *pDisplay)
{
	if (agDriverSw == NULL ||
	    !(agDriverOps->flags & AG_DRIVER_SDL2)) {
		AG_SetError("Current driver is not an SDL2 driver");
		return (-1);
	}
	if (AGDRIVER_SW_CLASS(agDriverSw)->setVideoContext(agDriverSw,
	    pDisplay) == -1) {
		return (-1);
	}
	AG_PostResizeDisplay(agDriverSw);
	return (0);
}
#endif

/* Return the Agar pixel format corresponding to that of an SDL2 surface. */
AG_PixelFormat *
AG_SDL2_GetPixelFormat(const SDL_Surface *s)
{
	AG_PixelFormat *pf = Malloc(sizeof(AG_PixelFormat));
	SDL_Palette *sp;
	int i;

	if ((sp = s->format->palette) != NULL) {
		AG_PixelFormatIndexed(pf, s->format->BitsPerPixel);

		for (i = 0; i < sp->ncolors; i++) {
			AG_Color *c = &pf->palette->colors[i];

			c->r = AG_8toH(sp->colors[i].r);
			c->g = AG_8toH(sp->colors[i].g);
			c->b = AG_8toH(sp->colors[i].b);
			c->a = AG_OPAQUE;
		}
	} else {
		AG_PixelFormatRGBA(pf, s->format->BitsPerPixel,
		    s->format->Rmask,
		    s->format->Gmask,
		    s->format->Bmask,
		    s->format->Amask);
	}
	return (pf);
}

/* Return a 32-bit representation of the pixel at p in an SDL surface. */
static __inline__ Uint32
AG_SDL2_GetPixel(SDL_Surface *_Nonnull s, Uint8 *_Nonnull p)
{
	switch (s->format->BytesPerPixel) {
	case 4:
		return *(Uint32 *)p;
	case 3:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		return (p[0] << 16) +
		       (p[1] << 8) +
		        p[2];
#else
		return (p[0] +
		       (p[1] << 8) +
		       (p[2] << 16));
#endif
	case 2:
		return *(Uint16 *)p;
	default:
		return *p;
	}
}

/* Write the pixel at p in an SDL surface. */
static __inline__ void
AG_SDL2_PutPixel(SDL_Surface *_Nonnull s, Uint8 *_Nonnull p, Uint32 px)
{
	switch (s->format->BytesPerPixel) {
	case 4:
		*(Uint32 *)p = px;
		break;
	case 3:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		p[0] = (px >> 16) & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px)       & 0xff;
#else
		p[0] = (px >> 16) & 0xff;
		p[1] = (px >> 8)  & 0xff;
		p[2] = (px)       & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)p = px;
		break;
	default:
		*p = px;
		break;
	}
}
static __inline__ void
AG_SDL2_PutPixel2(SDL_Surface *_Nonnull S, int x, int y, Uint32 px)
{
	AG_SDL2_PutPixel(S,
	    (Uint8 *)S->pixels + y*S->pitch + x*S->format->BytesPerPixel,
	    px);
}

/*
 * Blend an AG_Color c against the pixel at p in an SDL_Surface.
 * The surface must be locked.
 */
static void
AG_SDL2_SurfaceBlend(SDL_Surface *_Nonnull s, Uint8 *_Nonnull p, AG_Color c,
    AG_AlphaFn fn)
{
	Uint32 px = AG_SDL2_GetPixel(s,p);
	Uint8 r = AG_Hto8(c.r);
	Uint8 g = AG_Hto8(c.g);
	Uint8 b = AG_Hto8(c.b);
	Uint8 a = AG_Hto8(c.a);
	Uint8 R,G,B,A;

	SDL_GetRGBA(px, s->format, &R,&G,&B,&A);
	
	switch (fn) {
	case AG_ALPHA_OVERLAY:
		a = (Uint8)((A+a) > 255) ? 255 : (A+a);
		break;
	case AG_ALPHA_DST:           a = A;     break;
	case AG_ALPHA_SRC:        /* a = a; */  break;
	case AG_ALPHA_ZERO:          a = 0;     break;
	case AG_ALPHA_ONE_MINUS_DST: a = 255-A; break;
	case AG_ALPHA_ONE_MINUS_SRC: a = 255-a; break;
	case AG_ALPHA_ONE:
	default:                     a = 255;   break;
	}

	AG_SDL2_PutPixel(s, p,
	    SDL_MapRGBA(s->format,
	        R+(((r - R)*a) >> 8),
	        G+(((g - G)*a) >> 8),
	        B+(((b - B)*a) >> 8), a));
}

/*
 * Blit an AG_Surface directly to an SDL_Surface.
 * If the formats of the two surfaces may differ, convert implicitely.
 * The source surface must be locked. Locks the target SDL surface if needed.
 */
/* XXX TODO optimized cases, per-surface alpha */
void
AG_SDL2_BlitSurface(const AG_Surface *S, const AG_Rect *srcRect,
    SDL_Surface *D, int xDst, int yDst)
{
	AG_Rect sr, dr, cr;

#ifdef AG_DEBUG
	if (S->flags & AG_SURFACE_TRACE)
		Debug(NULL, "Surface <%p>: Blit to SDL_Surface <%p> @ %d,%d\n",
		    S, D, xDst, yDst);
#endif
	if (srcRect != NULL) {
		sr = *srcRect;
		if (sr.x < 0) { sr.x = 0; }
		if (sr.y < 0) { sr.y = 0; }
		if (sr.x+sr.w >= S->w) { sr.w = S->w - sr.x; }
		if (sr.y+sr.h >= S->h) { sr.h = S->h - sr.y; }
		if (sr.w < 0) { sr.w = 0; }
		if (sr.h < 0) { sr.h = 0; }
	} else {
		sr.x = 0;
		sr.y = 0;
		sr.w = S->w;
		sr.h = S->h;
	}
	dr.x = xDst;
	dr.y = yDst;
	dr.w = sr.w;
	dr.h = sr.h;
	cr.x = D->clip_rect.x;
	cr.y = D->clip_rect.y;
	cr.w = D->clip_rect.w;
	cr.h = D->clip_rect.h;
	if (!AG_RectIntersect(&dr, &dr, &cr))
		return;

	if (sr.w > dr.w) {					/* Partial */
		if (xDst < cr.x) {
			sr.x += (cr.x - xDst);
		}
		sr.w -= (sr.w - dr.w);
	}
	if (sr.h > dr.h) {
		if (yDst < cr.y) {
			sr.y += (cr.y - yDst);
		}
		sr.h -= (sr.h - dr.h);
	}
	if (sr.w <= 0 || sr.h <= 0)
		return;

	if (SDL_MUSTLOCK(D)) { SDL_LockSurface(D); }

	if (S->flags & AG_SURFACE_COLORKEY) {
		AG_SDL2_SoftBlit_Colorkey(S,sr, D,dr);
	} else {
		AG_SDL2_SoftBlit_NoColorkey(S,sr, D,dr);
	}

	if (SDL_MUSTLOCK(D)) { SDL_UnlockSurface(D); }
}
static void
AG_SDL2_SoftBlit_Colorkey(const AG_Surface *_Nonnull S, AG_Rect sr,
    SDL_Surface *_Nonnull ds, AG_Rect dr)
{
	const int dsBytesPerPixel = ds->format->BytesPerPixel;
	Uint8 *pDst = (Uint8 *)ds->pixels + dr.y*ds->pitch + dr.x*dsBytesPerPixel;
	const int dsPadding = ds->pitch - dr.w*dsBytesPerPixel;
	int x, y;

	for (y = 0; y < sr.h; y++) {
		for (x = 0; x < sr.w; x++) {
			AG_Pixel px;
			AG_Color c;

			px = AG_SurfaceGet(S, sr.x+x, sr.y+y);
			if (S->colorkey == px) {
				pDst += dsBytesPerPixel;
				continue;
			}
			AG_GetColor(&c, px, &S->format);
			if (c.a < AG_OPAQUE) {
				AG_SDL2_SurfaceBlend(ds, pDst, c, AG_ALPHA_SRC);
			} else {
				AG_SDL2_PutPixel2(ds,
				    dr.x + x,
				    dr.y + y,
				    SDL_MapRGB(ds->format,
				        AG_Hto8(c.r),
				        AG_Hto8(c.g),
				        AG_Hto8(c.b)));
			}
			pDst += dsBytesPerPixel;
		}
		pDst += dsPadding;
	}
}
static void
AG_SDL2_SoftBlit_NoColorkey(const AG_Surface *_Nonnull S, AG_Rect sr,
    SDL_Surface *_Nonnull ds, AG_Rect dr)
{
	const int dsBytesPerPixel = ds->format->BytesPerPixel;
	Uint8 *pDst = (Uint8 *)ds->pixels + dr.y*ds->pitch + dr.x*dsBytesPerPixel;
	const int dsPadding = ds->pitch - dr.w*dsBytesPerPixel;
	int x, y;

	for (y = 0; y < dr.h; y++) {
		for (x = 0; x < dr.w; x++) {
			AG_Pixel px;
			AG_Color c;
	
			px = AG_SurfaceGet(S, sr.x+x, sr.y+y);
			AG_GetColor(&c, px, &S->format);
			if (c.a < AG_OPAQUE) {
				AG_SDL2_SurfaceBlend(ds, pDst, c, AG_ALPHA_SRC);
			} else {
				AG_SDL2_PutPixel2(ds,
				    dr.x + x,
				    dr.y + y,
				    SDL_MapRGB(ds->format,
				        AG_Hto8(c.r),
				        AG_Hto8(c.g),
				        AG_Hto8(c.b)));
			}
			pDst += dsBytesPerPixel;
		}
		pDst += dsPadding;
	}
}

/*
 * Import an SDL surface into a newly allocated AG_Surface (of the same
 * pixel format so the pixel data can be block copied).
 * For indexed surfaces, convert the SDL_Palette to an AG_Palette.
 */
AG_Surface *
AG_SDL2_ImportSurface(SDL_Surface *src)
{
	AG_PixelFormat pf;
	AG_Surface *dst;
	const SDL_Palette *sp;
	int i;

	if ((sp = src->format->palette) != NULL) {
		AG_PixelFormatIndexed(&pf, src->format->BitsPerPixel);
#ifdef AG_DEBUG
		if (sp->ncolors != pf.palette->nColors)
			AG_FatalError("nColors");
#endif
		for (i = 0; i < sp->ncolors; i++) {
			const SDL_Color *sc = &sp->colors[i];
			AG_Color *c = &pf.palette->colors[i];

			c->r = AG_8toH(sc->r);
			c->g = AG_8toH(sc->g);
			c->b = AG_8toH(sc->b);
			c->a = AG_OPAQUE;
		}
	} else {
		AG_PixelFormatRGBA(&pf, src->format->BitsPerPixel,
		    src->format->Rmask,
		    src->format->Gmask,
		    src->format->Bmask,
		    src->format->Amask);
	}
	dst = AG_SurfaceNew(&pf, src->w, src->h, 0);
#if 0
	/* XXX SDL2 */
	if (src->flags & SDL_SRCCOLORKEY) {
		dst->flags |= AG_SURFACE_COLORKEY;
		dst->colorkey = src->format->colorkey;
	}
	if (src->flags & SDL_SRCALPHA)    {
		dst->flags |= AG_SURFACE_ALPHA;
		dst->alpha = src->format->alpha;
	}
#endif
	
	if (SDL_MUSTLOCK(src)) { SDL_LockSurface(src); }

	memcpy(dst->pixels, (Uint8 *)src->pixels, src->h * src->pitch);

	if (SDL_MUSTLOCK(src)) { SDL_UnlockSurface(src); }

	AG_PixelFormatFree(&pf);
	return (dst);
}

/* Convert a SDL2 surface to Agar surface. */
AG_Surface *
AG_SurfaceFromSDL2(void *p)
{
	return AG_SDL2_ImportSurface((SDL_Surface *)p);
}

/* Export an AG_Surface to a newly-created SDL_Surface. */
void *
AG_SurfaceExportSDL2(const AG_Surface *ss)
{
	Uint32 sdlFlags = SDL_SWSURFACE;
	SDL_Surface *ds;

#if 0
	/* XXX SDL2 */
	if (ss->flags & AG_SURFACE_COLORKEY) { sdlFlags |= SDL_SRCCOLORKEY; }
	if (ss->flags & AG_SURFACE_ALPHA)    { sdlFlags |= SDL_SRCALPHA; }
#endif

	/* TODO INDEXED->INDEXED & GRAYSCALE->PACKED */

	ds = SDL_CreateRGBSurface(sdlFlags, ss->w, ss->h,
	    ss->format.BitsPerPixel,
	    ss->format.Rmask,
	    ss->format.Gmask,
	    ss->format.Bmask,
	    ss->format.Amask);
	if (ds == NULL) {
		AG_SetError("SDL_CreateRGBSurface: %s", SDL_GetError());
		return (NULL);
	}
	AG_SDL2_BlitSurface(ss, NULL, ds, 0,0);
	return (void *)ds;
}

/* Initialize the default cursor. */
void
AG_SDL2_InitDefaultCursor(void *obj)
{
	AG_Driver *drv = AGDRIVER(obj);
	AG_Cursor *ac;
	SDL_Cursor *sc;
	
	if ((sc = SDL_GetCursor()) == NULL) {
		AG_FatalError("SDL_GetCursor");
	}
	ac = Malloc(sizeof(AG_Cursor));
	AG_CursorInit(ac);
#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(1,3,0)
	ac->w = (Uint)sc->area.w;
	ac->h = (Uint)sc->area.h;
	ac->xHot = (int)sc->hot_x;
	ac->yHot = (int)sc->hot_y;
#else
    // TODO
#endif
	ac->p = sc;

	TAILQ_INSERT_HEAD(&drv->cursors, ac, cursors);
	drv->nCursors++;
}

/* Change the cursor. */
int
AG_SDL2_SetCursor(void *obj, AG_Cursor *ac)
{
	AG_Driver *drv = obj;
	
	if (drv->activeCursor == ac)
		return (0);

	SDL_SetCursor((SDL_Cursor *)ac->p);
	drv->activeCursor = ac;
	return (0);
}

/* Revert to the default cursor. */
void
AG_SDL2_UnsetCursor(void *obj)
{
	AG_Driver *drv = obj;
	AG_Cursor *ac0 = TAILQ_FIRST(&drv->cursors);
	
	if (drv->activeCursor == ac0)
		return;
	
	SDL_SetCursor((SDL_Cursor *)ac0->p);
	drv->activeCursor = ac0;
}

/* Configure refresh rate. */
int
AG_SDL2_SetRefreshRate(void *obj, int fps)
{
	AG_DriverSw *dsw = obj;

	if (fps < 1) {
		AG_SetError("Invalid refresh rate");
		return (-1);
	}
	dsw->rNom = 1000/fps;
	return (0);
}

/* Create a cursor. */
AG_Cursor *
AG_SDL2_CreateCursor(void *obj, Uint w, Uint h, const Uint8 *data,
    const Uint8 *mask, int xHot, int yHot)
{
	AG_Cursor *ac;
	SDL_Cursor *sc;
	Uint size = w*h;
	
	if ((ac = TryMalloc(sizeof(AG_Cursor))) == NULL) {
		return (NULL);
	}
	if ((ac->data = TryMalloc(size)) == NULL) {
		free(ac);
		return (NULL);
	}
	if ((ac->mask = TryMalloc(size)) == NULL) {
		free(ac->data);
		free(ac);
		return (NULL);
	}
	memcpy(ac->data, data, size);
	memcpy(ac->mask, mask, size);
	ac->w = w;
	ac->h = h;
	ac->xHot = xHot;
	ac->yHot = yHot;

	sc = SDL_CreateCursor(ac->data, ac->mask,
	    ac->w, ac->h,
	    ac->xHot, ac->yHot);
	if (sc == NULL) {
		AG_SetError("SDL_CreateCursor failed");
		goto fail;
	}
	ac->p = (void *)sc;
	return (ac);
fail:
	free(ac->data);
	free(ac->mask);
	free(ac);
	return (NULL);
}

/* Release a cursor. */
void
AG_SDL2_FreeCursor(void *obj, AG_Cursor *ac)
{
	AG_Driver *drv = obj;

	if (ac == drv->activeCursor)
		drv->activeCursor = NULL;

	SDL_FreeCursor((SDL_Cursor *)(ac->p));
	free(ac->data);
	free(ac->mask);
	free(ac);
}

/* Retrieve cursor visibility status. */
int
AG_SDL2_GetCursorVisibility(void *obj)
{
	return (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
}

/* Set cursor visibility. */
void
AG_SDL2_SetCursorVisibility(void *obj, int flag)
{
	SDL_ShowCursor(flag ? SDL_ENABLE : SDL_DISABLE);
}

/* Return the desktop display size in pixels. */
int
AG_SDL2_GetDisplaySize(Uint *w, Uint *h)
{
	SDL_DisplayMode mode;
	int i;

	for (i = 0; i < SDL_GetNumVideoDisplays(); i++) {
		int ret = SDL_GetCurrentDisplayMode(i, &mode);

		if (ret != 0) {
			AG_SetError("SDL_GetCurrentDisplayMode: %s",
			    SDL_GetError());
			return (-1);
		}
		*w = mode.w;
		*h = mode.h;
		return (0);
	}
	AG_SetErrorS("SDL: No video displays");
	return (-1);
}

/* Apply default display settings where 0 is specified. */
void
AG_SDL2_GetPrefDisplaySettings(void *obj, Uint *w, Uint *h, int *depth)
{
	char buf[16], *ep;
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	Uint wDisp, hDisp;

	if (*w == 0 || *h == 0) {
		if (AG_SDL2_GetDisplaySize(&wDisp, &hDisp) == -1) {
			wDisp = 640;
			hDisp = 480;
		}
	}
	if (*w == 0) {
		if (AG_Defined(drv, "width")) {
			AG_GetString(drv, "width", buf, sizeof(buf));
			if (buf[0] == 'a') { /* auto */
				*w = wDisp;
			} else {
				Uint wNew = strtol(buf, &ep, 10);
				if (ep != NULL && ep[0] == '%') {
					*w = wDisp*wNew/100;
				} else {
					*w = wNew;
				}
			}
		} else {
			*w = (wDisp > 1000) ? (wDisp - wDisp*0.05) : wDisp;
		}
	}
	if (*h == 0) {
		if (AG_Defined(drv, "height")) {
			AG_GetString(drv, "height", buf, sizeof(buf));
			if (buf[0] == 'a') { /* auto */
				*h = hDisp;
			} else {
				Uint hNew = strtol(buf, &ep, 10);
				if (ep != NULL && ep[0] == '%') {
					*h = hDisp*hNew/100;
				} else {
					*h = hNew;
				}
			}
		} else {
			*h = (hDisp > 700) ? (hDisp - hDisp*0.1) : hDisp;
		}
	}
	if (*depth == 0) {
		if (AG_Defined(drv, "depth")) {
			AG_GetString(drv, "depth", buf, sizeof(buf));
			*depth = (buf[0] == 'a') ? 32 : atoi(buf);
		} else {
			*depth = 32;
		}
	}

	if (AG_Defined(drv, "fpsMax")) {
		char *ep;
		float v;

		AG_GetString(drv, "fpsMax", buf, sizeof(buf));
		v = (float)strtod(buf, &ep);
		if (*ep == '\0') {
			dsw->rNom = (Uint)(1000.0f/v);
			Verbose("%s: max fps %s (%u ms)\n", OBJECT(drv)->name,
			    buf, dsw->rNom);
		}
	}
	if (AG_Defined(drv, "bgColor")) {
		Verbose("%s: bgColor -> %s\n", OBJECT(drv)->name,
		    AG_GetStringP(drv,"bgColor"));
		AG_ColorFromString(&dsw->bgColor,
		    AG_GetStringP(drv,"bgColor"),
		    NULL);
	}
	if (!AG_Defined(drv, "!bgPopup"))
		dsw->flags |= AG_DRIVER_SW_BGPOPUP;
}

/* Standard beginEventProcessing() method for SDL drivers. */
void
AG_SDL2_BeginEventProcessing(void *obj)
{
	/* Nothing to do */
}

/* Standard pendingEvents() method for SDL drivers. */
int
AG_SDL2_PendingEvents(void *obj)
{
	return (SDL_PollEvent(NULL) != 0);
}

/* Convert an SDL2 SDL_Scancode to an AG_KeySym */
static __inline__ const AG_KeySym
AG_SDL2_KeySymFromScancode(const SDL_Scancode scancode)
{
	switch (scancode) {
	case SDL_SCANCODE_CAPSLOCK:		return AG_KEY_CAPSLOCK;
	case SDL_SCANCODE_F1:			return AG_KEY_F1;
	case SDL_SCANCODE_F2:			return AG_KEY_F2;
	case SDL_SCANCODE_F3:			return AG_KEY_F3;
	case SDL_SCANCODE_F4:			return AG_KEY_F4;
	case SDL_SCANCODE_F5:			return AG_KEY_F5;
	case SDL_SCANCODE_F6:			return AG_KEY_F6;
	case SDL_SCANCODE_F7:			return AG_KEY_F7;
	case SDL_SCANCODE_F8:			return AG_KEY_F8;
	case SDL_SCANCODE_F9:			return AG_KEY_F9;
	case SDL_SCANCODE_F10:			return AG_KEY_F10;
	case SDL_SCANCODE_F11:			return AG_KEY_F11;
	case SDL_SCANCODE_F12:			return AG_KEY_F12;
	case SDL_SCANCODE_PRINTSCREEN:		return AG_KEY_PRINT_SCREEN;
	case SDL_SCANCODE_SCROLLLOCK:		return AG_KEY_SCROLLOCK;
	case SDL_SCANCODE_PAUSE:		return AG_KEY_PAUSE;
	case SDL_SCANCODE_INSERT:		return AG_KEY_INSERT;
	case SDL_SCANCODE_HOME:			return AG_KEY_HOME;
	case SDL_SCANCODE_PAGEUP:		return AG_KEY_PAGEUP;
	case SDL_SCANCODE_DELETE:		return AG_KEY_DELETE;
	case SDL_SCANCODE_END:			return AG_KEY_END;
	case SDL_SCANCODE_PAGEDOWN:		return AG_KEY_PAGEDOWN;
	case SDL_SCANCODE_RIGHT:		return AG_KEY_RIGHT;
	case SDL_SCANCODE_LEFT:			return AG_KEY_LEFT;
	case SDL_SCANCODE_DOWN:			return AG_KEY_DOWN;
	case SDL_SCANCODE_UP:			return AG_KEY_UP;
	case SDL_SCANCODE_NUMLOCKCLEAR:		return AG_KEY_NUMLOCK;
	case SDL_SCANCODE_KP_DIVIDE:		return AG_KEY_KP_DIVIDE;
	case SDL_SCANCODE_KP_MULTIPLY:		return AG_KEY_KP_MULTIPLY;
	case SDL_SCANCODE_KP_MINUS:		return AG_KEY_KP_MINUS;
	case SDL_SCANCODE_KP_PLUS:		return AG_KEY_KP_PLUS;
	case SDL_SCANCODE_KP_ENTER:		return AG_KEY_KP_ENTER;
	case SDL_SCANCODE_KP_1:			return AG_KEY_KP1;
	case SDL_SCANCODE_KP_2:			return AG_KEY_KP2;
	case SDL_SCANCODE_KP_3:			return AG_KEY_KP3;
	case SDL_SCANCODE_KP_4:			return AG_KEY_KP4;
	case SDL_SCANCODE_KP_5:			return AG_KEY_KP5;
	case SDL_SCANCODE_KP_6:			return AG_KEY_KP6;
	case SDL_SCANCODE_KP_7:			return AG_KEY_KP7;
	case SDL_SCANCODE_KP_8:			return AG_KEY_KP8;
	case SDL_SCANCODE_KP_9:			return AG_KEY_KP9;
	case SDL_SCANCODE_KP_0:			return AG_KEY_KP0;
	case SDL_SCANCODE_KP_PERIOD:		return AG_KEY_KP_PERIOD;
	case SDL_SCANCODE_NONUSBACKSLASH:	return AG_KEY_NON_US_BACKSLASH;
	case SDL_SCANCODE_APPLICATION:		return AG_KEY_APPLICATION;
	case SDL_SCANCODE_POWER:		return AG_KEY_POWER;
	case SDL_SCANCODE_KP_EQUALS:		return AG_KEY_KP_EQUALS;
	case SDL_SCANCODE_F13:			return AG_KEY_F13;
	case SDL_SCANCODE_F14:			return AG_KEY_F14;
	case SDL_SCANCODE_F15:			return AG_KEY_F15;
	case SDL_SCANCODE_F16:			return AG_KEY_F16;
	case SDL_SCANCODE_F17:			return AG_KEY_F17;
	case SDL_SCANCODE_F18:			return AG_KEY_F18;
	case SDL_SCANCODE_F19:			return AG_KEY_F19;
	case SDL_SCANCODE_F20:			return AG_KEY_F20;
	case SDL_SCANCODE_F21:			return AG_KEY_F21;
	case SDL_SCANCODE_F22:			return AG_KEY_F22;
	case SDL_SCANCODE_F23:			return AG_KEY_F23;
	case SDL_SCANCODE_F24:			return AG_KEY_F24;
	case SDL_SCANCODE_EXECUTE:		return AG_KEY_EXECUTE;
	case SDL_SCANCODE_HELP:			return AG_KEY_HELP;
	case SDL_SCANCODE_MENU:			return AG_KEY_MENU;
	case SDL_SCANCODE_SELECT:		return AG_KEY_SELECT;
	case SDL_SCANCODE_STOP:			return AG_KEY_STOP;
	case SDL_SCANCODE_AGAIN:		return AG_KEY_AGAIN;
	case SDL_SCANCODE_UNDO:			return AG_KEY_UNDO;
	case SDL_SCANCODE_CUT:			return AG_KEY_CUT;
	case SDL_SCANCODE_PASTE:		return AG_KEY_PASTE;
	case SDL_SCANCODE_FIND:			return AG_KEY_FIND;
	case SDL_SCANCODE_MUTE:			return AG_KEY_VOLUME_MUTE;
	case SDL_SCANCODE_VOLUMEUP:		return AG_KEY_VOLUME_UP;
	case SDL_SCANCODE_VOLUMEDOWN:		return AG_KEY_VOLUME_DOWN;
	case SDL_SCANCODE_KP_COMMA:		return AG_KEY_KP_COMMA;
	case SDL_SCANCODE_KP_EQUALSAS400:	return AG_KEY_KP_EQUALS_AS_400;
	case SDL_SCANCODE_INTERNATIONAL1:	return AG_KEY_INTERNATIONAL_1;
	case SDL_SCANCODE_INTERNATIONAL2:	return AG_KEY_INTERNATIONAL_2;
	case SDL_SCANCODE_INTERNATIONAL3:	return AG_KEY_INTERNATIONAL_3;
	case SDL_SCANCODE_INTERNATIONAL4:	return AG_KEY_INTERNATIONAL_4;
	case SDL_SCANCODE_INTERNATIONAL5:	return AG_KEY_INTERNATIONAL_5;
	case SDL_SCANCODE_INTERNATIONAL6:	return AG_KEY_INTERNATIONAL_6;
	case SDL_SCANCODE_INTERNATIONAL7:	return AG_KEY_INTERNATIONAL_7;
	case SDL_SCANCODE_INTERNATIONAL8:	return AG_KEY_INTERNATIONAL_8;
	case SDL_SCANCODE_INTERNATIONAL9:	return AG_KEY_INTERNATIONAL_9;
	case SDL_SCANCODE_LANG1:		return AG_KEY_LANGUAGE_1;
	case SDL_SCANCODE_LANG2:		return AG_KEY_LANGUAGE_2;
	case SDL_SCANCODE_LANG3:		return AG_KEY_LANGUAGE_3;
	case SDL_SCANCODE_LANG4:		return AG_KEY_LANGUAGE_4;
	case SDL_SCANCODE_LANG5:		return AG_KEY_LANGUAGE_5;
	case SDL_SCANCODE_LANG6:		return AG_KEY_LANGUAGE_6;
	case SDL_SCANCODE_LANG7:		return AG_KEY_LANGUAGE_7;
	case SDL_SCANCODE_LANG8:		return AG_KEY_LANGUAGE_8;
	case SDL_SCANCODE_LANG9:		return AG_KEY_LANGUAGE_9;
	case SDL_SCANCODE_ALTERASE:		return AG_KEY_ALT_ERASE;
	case SDL_SCANCODE_SYSREQ:		return AG_KEY_SYSREQ;
	case SDL_SCANCODE_CANCEL:		return AG_KEY_CANCEL;
	case SDL_SCANCODE_CLEAR:		return AG_KEY_CLEAR;
	case SDL_SCANCODE_PRIOR:		return AG_KEY_PRIOR;
	case SDL_SCANCODE_RETURN2:		return AG_KEY_RETURN2;
	case SDL_SCANCODE_SEPARATOR:		return AG_KEY_SEPARATOR;
	case SDL_SCANCODE_OUT:			return AG_KEY_OUT;
	case SDL_SCANCODE_OPER:			return AG_KEY_OPER;
	case SDL_SCANCODE_CLEARAGAIN:		return AG_KEY_CLEAR_AGAIN;
	case SDL_SCANCODE_CRSEL:		return AG_KEY_CRSEL;
	case SDL_SCANCODE_EXSEL:		return AG_KEY_EXSEL;
	case SDL_SCANCODE_KP_00:		return AG_KEY_KP_00;
	case SDL_SCANCODE_KP_000:		return AG_KEY_KP_000;
	case SDL_SCANCODE_THOUSANDSSEPARATOR:	return AG_KEY_THOUSANDS_SEPARATOR;
	case SDL_SCANCODE_DECIMALSEPARATOR:	return AG_KEY_DECIMALS_SEPARATOR;
	case SDL_SCANCODE_CURRENCYUNIT:		return AG_KEY_CURRENCY_UNIT;
	case SDL_SCANCODE_CURRENCYSUBUNIT:	return AG_KEY_CURRENCY_SUBUNIT;
	case SDL_SCANCODE_KP_LEFTPAREN:		return AG_KEY_KP_LEFT_PAREN;
	case SDL_SCANCODE_KP_RIGHTPAREN:	return AG_KEY_KP_RIGHT_PAREN;
	case SDL_SCANCODE_KP_LEFTBRACE:		return AG_KEY_KP_LEFT_BRACE;
	case SDL_SCANCODE_KP_RIGHTBRACE:	return AG_KEY_KP_RIGHT_BRACE;
	case SDL_SCANCODE_KP_TAB:		return AG_KEY_KP_TAB;
	case SDL_SCANCODE_KP_BACKSPACE:		return AG_KEY_KP_BACKSPACE;
	case SDL_SCANCODE_KP_A:			return AG_KEY_KP_A;
	case SDL_SCANCODE_KP_B:			return AG_KEY_KP_B;
	case SDL_SCANCODE_KP_C:			return AG_KEY_KP_C;
	case SDL_SCANCODE_KP_D:			return AG_KEY_KP_D;
	case SDL_SCANCODE_KP_E:			return AG_KEY_KP_E;
	case SDL_SCANCODE_KP_F:			return AG_KEY_KP_F;
	case SDL_SCANCODE_KP_XOR:		return AG_KEY_KP_XOR;
	case SDL_SCANCODE_KP_POWER:		return AG_KEY_KP_POWER;
	case SDL_SCANCODE_KP_PERCENT:		return AG_KEY_KP_PERCENT;
	case SDL_SCANCODE_KP_LESS:		return AG_KEY_KP_LESS;
	case SDL_SCANCODE_KP_GREATER:		return AG_KEY_KP_GREATER;
	case SDL_SCANCODE_KP_AMPERSAND:		return AG_KEY_KP_AMPERSAND;
	case SDL_SCANCODE_KP_DBLAMPERSAND:	return AG_KEY_KP_DBL_AMPERSAND;
	case SDL_SCANCODE_KP_VERTICALBAR:	return AG_KEY_KP_VERTICAL_BAR;
	case SDL_SCANCODE_KP_DBLVERTICALBAR:	return AG_KEY_KP_DBL_VERTICAL_BAR;
	case SDL_SCANCODE_KP_COLON:		return AG_KEY_KP_COLON;
	case SDL_SCANCODE_KP_HASH:		return AG_KEY_KP_HASH;
	case SDL_SCANCODE_KP_SPACE:		return AG_KEY_KP_SPACE;
	case SDL_SCANCODE_KP_AT:		return AG_KEY_KP_AT;
	case SDL_SCANCODE_KP_EXCLAM:		return AG_KEY_KP_EXCLAM;
	case SDL_SCANCODE_KP_MEMSTORE:		return AG_KEY_KP_MEM_STORE;
	case SDL_SCANCODE_KP_MEMRECALL:		return AG_KEY_KP_MEM_RECALL;
	case SDL_SCANCODE_KP_MEMCLEAR:		return AG_KEY_KP_MEM_CLEAR;
	case SDL_SCANCODE_KP_MEMADD:		return AG_KEY_KP_MEM_ADD;
	case SDL_SCANCODE_KP_MEMSUBTRACT:	return AG_KEY_KP_MEM_SUBTRACT;
	case SDL_SCANCODE_KP_MEMMULTIPLY:	return AG_KEY_KP_MEM_MULTIPLY;
	case SDL_SCANCODE_KP_MEMDIVIDE:		return AG_KEY_KP_MEM_DIVIDE;
	case SDL_SCANCODE_KP_PLUSMINUS:		return AG_KEY_KP_PLUS_MINUS;
	case SDL_SCANCODE_KP_CLEAR:		return AG_KEY_KP_CLEAR;
	case SDL_SCANCODE_KP_CLEARENTRY:	return AG_KEY_KP_CLEAR_ENTRY;
	case SDL_SCANCODE_KP_BINARY:		return AG_KEY_KP_BINARY;
	case SDL_SCANCODE_KP_OCTAL:		return AG_KEY_KP_OCTAL;
	case SDL_SCANCODE_KP_DECIMAL:		return AG_KEY_KP_DECIMAL;
	case SDL_SCANCODE_KP_HEXADECIMAL:	return AG_KEY_KP_HEXADECIMAL;

	case SDL_SCANCODE_LCTRL:		return AG_KEY_LCTRL;
	case SDL_SCANCODE_LSHIFT:		return AG_KEY_LSHIFT;
	case SDL_SCANCODE_LALT:			return AG_KEY_LALT;
	case SDL_SCANCODE_LGUI:			return AG_KEY_LGUI;
	case SDL_SCANCODE_RCTRL:		return AG_KEY_RCTRL;
	case SDL_SCANCODE_RSHIFT:		return AG_KEY_RSHIFT;
	case SDL_SCANCODE_RALT:			return AG_KEY_RALT;
	case SDL_SCANCODE_RGUI:			return AG_KEY_RGUI;
	case SDL_SCANCODE_MODE:			return AG_KEY_MODE;
	}
	return (AG_KEY_LAST);
}

/*
 * Return the AG_Window corresponding to the given SDL Window ID.
 * The agDrivers VFS must be locked.
 */
AG_Window *
AG_SDL_GetWindowFromID(AG_Driver *drv, Uint32 windowID)
{
	AG_DriverMw *dmw;

	if (AGDRIVER_SINGLE(drv)) {
		return (NULL);		/* Not needed (single-window driver) */
	}
	AGOBJECT_FOREACH_CHILD(dmw, &agDrivers, ag_driver_mw) {
		AG_Window *win;

		if (!AGDRIVER_MULTIPLE(dmw) ||
		    dmw->windowID != windowID) {
			continue;
		}
		win = AGDRIVER_MW(dmw)->win;

		if (!(AGDRIVER_MW(dmw)->flags & AG_DRIVER_MW_OPEN) ||
		    (win->flags & AG_WINDOW_DETACHING) ||
		    WIDGET(win)->drv == NULL ||
		    ((void *)win == (void *)&agDrivers)) {           /* XXX */
			return (NULL);
		}
		return (win);
	}
	return (NULL);
}

/*
 * Translate an SDL_Event to an AG_DriverEvent.
 * 
 * Compatible with both single-window and multi-window drivers.
 * For single-window drivers, the win pointer is set to NULL.
 */
void
AG_SDL2_TranslateEvent(void *obj, const SDL_Event *ev, AG_DriverEvent *dev)
{
	AG_Driver *drv = obj;
	const int isMW = AGDRIVER_MULTIPLE(drv);

	if (!isMW)
		dev->win = NULL;

	switch (ev->type) {
	case SDL_MOUSEMOTION:
		if (isMW) {
			if ((dev->win = AG_SDL_GetWindowFromID(drv, ev->motion.windowID)) == NULL) {
				goto fail;
			}
			drv = WIDGET(dev->win)->drv;
		}
		AG_MouseMotionUpdate(drv->mouse, ev->motion.x, ev->motion.y);
		dev->type = AG_DRIVER_MOUSE_MOTION;
		dev->data.motion.x = ev->motion.x;
		dev->data.motion.y = ev->motion.y;
		break;
	case SDL_MOUSEBUTTONUP:
		if (isMW) {
			dev->win = AG_SDL_GetWindowFromID(drv, ev->button.windowID);
			if (dev->win == NULL) {
				goto fail;
			}
#ifdef DEBUG_MOUSE
			Debug(drv, "MOUSEBUTTONUP "
			    "(%s, Which=" AGSI_YEL "%d" AGSI_RST
			    " Pos=" AGSI_YEL "%d,%d" AGSI_RST ")\n",
			    OBJECT(dev->win)->name,
			    ev->button.which,
			    ev->button.x, ev->button.y);
#endif
			drv = WIDGET(dev->win)->drv;
		} else {
#ifdef DEBUG_MOUSE
			Debug(drv, "MOUSEBUTTONUP "
			    "(Which=" AGSI_YEL "%d" AGSI_RST
			    " Pos=" AGSI_YEL "%d,%d" AGSI_RST ")\n",
			    ev->button.which,
			    ev->button.x, ev->button.y);
#endif
		}

		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_RELEASED,
		    ev->button.button);
		dev->type = AG_DRIVER_MOUSE_BUTTON_UP;
		dev->data.button.which = (AG_MouseButton)ev->button.button;
		dev->data.button.x = ev->button.x;
		dev->data.button.y = ev->button.y;
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (isMW) {
			dev->win = AG_SDL_GetWindowFromID(drv, ev->button.windowID);
			if (dev->win == NULL) {
				goto fail;
			}
#ifdef DEBUG_MOUSE
			Debug(drv, "MOUSEBUTTONDOWN "
			    "(%s, Which=" AGSI_YEL "%d" AGSI_RST
			    " Pos=" AGSI_YEL "%d,%d" AGSI_RST ")\n",
			    OBJECT(dev->win)->name,
			    ev->button.which,
			    ev->button.x, ev->button.y);
#endif
			drv = WIDGET(dev->win)->drv;
		} else {
#ifdef DEBUG_MOUSE
			Debug(drv, "MOUSEBUTTONDOWN "
			    "(Which=" AGSI_YEL "%d" AGSI_RST
			    " Pos=" AGSI_YEL "%d,%d" AGSI_RST ")\n",
			    ev->button.which,
			    ev->button.x, ev->button.y);
#endif
		}
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED,
		    ev->button.button);
		dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
		dev->data.button.which = (AG_MouseButton)ev->button.button;
		dev->data.button.x = ev->button.x;
		dev->data.button.y = ev->button.y;
		break;
	case SDL_MOUSEWHEEL:
#ifdef DEBUG_MOUSE
		Debug(drv, "MOUSEWHEEL (Y=" AGSI_YEL "%d" AGSI_RST ")\n",
		    ev->wheel.y);
#endif
		dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
		if (isMW) {
			dev->win = AG_SDL_GetWindowFromID(drv, ev->button.windowID);
			if (dev->win == NULL) {
				goto fail;
			}
			drv = WIDGET(dev->win)->drv;
		}
		if (ev->wheel.y == 1) {
			AG_MouseButtonUpdate(drv->mouse,
			    AG_BUTTON_PRESSED,
			    AG_MOUSE_WHEELUP);
			dev->data.button.which = AG_MOUSE_WHEELUP;
		} else {
			AG_MouseButtonUpdate(drv->mouse,
			    AG_BUTTON_PRESSED,
			    AG_MOUSE_WHEELDOWN);
			dev->data.button.which = AG_MOUSE_WHEELDOWN;
		}
		AG_MouseGetState(drv->mouse,
		    &dev->data.button.x,
		    &dev->data.button.y);
		break;
	case SDL_KEYDOWN:
		if (ev->key.repeat) {
			/* Agar implements its own keyrepeat */
			goto fail;
		}
#ifdef DEBUG_KEYBOARD
		Debug(drv, "KEYDOWN "
		    "(Sym=" AGSI_YEL "0x%x" AGSI_RST
		    " Scan=" AGSI_YEL "0x%x" AGSI_RST ")\n",
		    (Uint)ev->key.keysym.sym,
		    (Uint)ev->key.keysym.scancode);
#endif
		if ((ev->key.keysym.sym & SDLK_SCANCODE_MASK) != 0) {
			dev->data.key.ks = AG_SDL2_KeySymFromScancode(
			    ev->key.keysym.scancode);
		} else {
			dev->data.key.ks = (AG_KeySym)ev->key.keysym.sym;
		}
		if (isMW) {
			dev->win = AG_SDL_GetWindowFromID(drv, ev->key.windowID);
			if (dev->win == NULL) {
				goto fail;
			}
			drv = WIDGET(dev->win)->drv;
		}

		AG_KeyboardUpdate(drv->kbd, AG_KEY_PRESSED, dev->data.key.ks);

		dev->type = AG_DRIVER_KEY_DOWN;
		dev->data.key.ucs = AG_SDL_KeySymToUcs4(ev->key.keysym.sym);
		break;
	case SDL_KEYUP:
#ifdef DEBUG_KEYBOARD
		Debug(drv, "KEYUP "
		    "(Sym=" AGSI_YEL "0x%x" AGSI_RST
		    " Scan=" AGSI_YEL "0x%x" AGSI_RST ")\n",
		    (Uint)ev->key.keysym.sym,
		    (Uint)ev->key.keysym.scancode);
#endif
		if (isMW) {
			dev->win = AG_SDL_GetWindowFromID(drv, ev->key.windowID);
			if (dev->win == NULL) {
				goto fail;
			}
			drv = WIDGET(dev->win)->drv;
		}
		if ((ev->key.keysym.sym & SDLK_SCANCODE_MASK) != 0) {
			dev->data.key.ks = AG_SDL2_KeySymFromScancode(
			    ev->key.keysym.scancode);
		} else {
			dev->data.key.ks = (AG_KeySym)ev->key.keysym.sym;
		}

		AG_KeyboardUpdate(drv->kbd, AG_KEY_RELEASED, dev->data.key.ks);

		dev->type = AG_DRIVER_KEY_UP;
		dev->data.key.ucs = AG_SDL_KeySymToUcs4(ev->key.keysym.sym);
		break;
	case SDL_WINDOWEVENT:
		if (isMW) {
			dev->win = AG_SDL_GetWindowFromID(drv, ev->window.windowID);
			if (dev->win == NULL)
				goto fail;
		}
		switch (ev->window.event) {
		case SDL_WINDOWEVENT_EXPOSED:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW EXPOSED (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW EXPOSED\n");
			}
#endif
			dev->type = AG_DRIVER_EXPOSE;
			break;
		case SDL_WINDOWEVENT_MOVED:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW MOVED (%s, " AGSI_YEL "%d, %d" AGSI_RST ")\n",
				    OBJECT(dev->win)->name,
				    (int)ev->window.data1,
				    (int)ev->window.data2);
			} else {
				Debug(drv, "WINDOW MOVED (" AGSI_YEL "%d, %d" AGSI_RST ")\n",
				    (int)ev->window.data1,
				    (int)ev->window.data2);
			}
#endif
			dev->type = AG_DRIVER_MOVED;
			dev->data.moved.x = (int)ev->window.data1;
			dev->data.moved.y = (int)ev->window.data2;
			break;
		case SDL_WINDOWEVENT_RESIZED:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW RESIZED (%s, " AGSI_YEL "%d x %d" AGSI_RST ")\n",
				    OBJECT(dev->win)->name,
				    (int)ev->window.data1,
				    (int)ev->window.data2);
			} else {
				Debug(drv, "WINDOW RESIZED (" AGSI_YEL "%d x %d" AGSI_RST ")\n",
				    (int)ev->window.data1,
				    (int)ev->window.data2);
			}
#endif
			dev->type = AG_DRIVER_VIDEORESIZE;
			dev->data.videoresize.x = 0;
			dev->data.videoresize.y = 0;
			dev->data.videoresize.w = (int)ev->window.data1;
			dev->data.videoresize.h = (int)ev->window.data2;
			break;
		case SDL_WINDOWEVENT_SHOWN:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW SHOWN (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW SHOWN\n");
			}
#endif
			dev->type = AG_DRIVER_SHOWN;
			break;
		case SDL_WINDOWEVENT_HIDDEN:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW HIDDEN (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW HIDDEN\n");
			}
#endif
			dev->type = AG_DRIVER_HIDDEN;
			break;
		case SDL_WINDOWEVENT_ENTER:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW ENTER (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW ENTER\n");
			}
#endif
			dev->type = AG_DRIVER_MOUSE_ENTER;
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW FOCUS GAINED (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW FOCUS GAINED\n");
			}
#endif
			dev->type = AG_DRIVER_FOCUS_IN;
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW FOCUS LOST (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW FOCUS LOST\n");
			}
#endif
			dev->type = AG_DRIVER_FOCUS_OUT;
			break;
		case SDL_WINDOWEVENT_LEAVE:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW LEAVE (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW LEAVE\n");
			}
#endif
			dev->type = AG_DRIVER_MOUSE_LEAVE;
			break;
		case SDL_WINDOWEVENT_MINIMIZED:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW MINIMIZED (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW MINIMIZED\n");
			}
#endif
			dev->type = AG_DRIVER_MINIMIZED;
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW MAXIMIZED (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW MAXIMIZED\n");
			}
#endif
			dev->type = AG_DRIVER_MAXIMIZED;
			break;
		case SDL_WINDOWEVENT_RESTORED:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW RESTORED (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW RESTORED \n");
			}
#endif
			dev->type = AG_DRIVER_RESTORED;
			break;
		case SDL_WINDOWEVENT_CLOSE:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW CLOSE (%s)\n", OBJECT(dev->win)->name);
			} else {
				Debug(drv, "WINDOW CLOSE\n");
			}
#endif
			dev->type = AG_DRIVER_CLOSE;
			break;
		default:
#ifdef DEBUG_EVENTS
			if (isMW) {
				Debug(drv, "WINDOW EVENT (%s, " AGSI_YEL "0x%x, 0x%x,0x%x" AGSI_RST ")\n",
				    OBJECT(dev->win)->name,
				    (Uint)ev->window.event,
				    (Uint)ev->window.data1,
				    (Uint)ev->window.data2);
			} else {
				Debug(drv, "WINDOW EVENT (" AGSI_YEL "0x%x, 0x%x,0x%x" AGSI_RST ")\n",
				    (Uint)ev->window.event,
				    (Uint)ev->window.data1,
				    (Uint)ev->window.data2);
			}
#endif
			goto fail;
		}
		break;
	case SDL_QUIT:
	case SDL_USEREVENT:
		dev->type = AG_DRIVER_CLOSE;
		break;
	default:
		dev->type = AG_DRIVER_UNKNOWN;
		break;
	}
	return;
fail:
	dev->type = AG_DRIVER_UNKNOWN;
	dev->win = NULL;
}

/*
 * Standard getNextEvent() method.
 * For both single-window and multi-window drivers.
 */
int
AG_SDL2_GetNextEvent(void *obj, AG_DriverEvent *dev)
{
	AG_Driver *drv = obj;
	SDL_Event ev;

	if (SDL_PollEvent(&ev) == 0) {
		return (0);
	}
	AG_SDL2_TranslateEvent(drv, &ev, dev);
	return (1);
}

/* Test if the given coordinates overlap a window resize control. */
static __inline__ int
GenericMouseOverCtrl(AG_Window *_Nonnull win, int x, int y)
{
	int xRel;

	if ((y - WIDGET(win)->y) < (HEIGHT(win) - win->wBorderBot)) {
		return (AG_WINOP_NONE);
	}
	xRel = x - WIDGET(win)->x;
    	if (xRel < win->wResizeCtrl) {
		return (AG_WINOP_LRESIZE);
	} else if (xRel > (WIDTH(win) - win->wResizeCtrl)) {
		return (AG_WINOP_RRESIZE);
	} else if ((win->flags & AG_WINDOW_NOVRESIZE) == 0) {
		return (AG_WINOP_HRESIZE);
	}
	return (AG_WINOP_NONE);
}

/*
 * Process an input device event (single-window drivers).
 * The agDrivers VFS must be locked.
 */
static int
ProcessInputEvent_SW(AG_Driver *_Nonnull drv, AG_DriverEvent *_Nonnull dev)
{
	AG_DriverSw *dsw = (AG_DriverSw *)drv;
	AG_Window *win, *winTop = NULL;
	int useText;

	if (dev->type == AG_DRIVER_MOUSE_BUTTON_UP) {
		dsw->winop = AG_WINOP_NONE;
		dsw->winSelected = NULL;
	}
	AG_FOREACH_WINDOW_REVERSE(win, dsw) {
		if (!win->visible) {
			continue;
		}
		AG_ObjectLock(win);
		if ((useText = (win->flags & AG_WINDOW_USE_TEXT))) {
			AG_PushTextState();
			AG_TextFont(WIDGET(win)->font);
			AG_TextColor(&WIDGET(win)->pal.c[WIDGET(win)->state]
			                                [AG_TEXT_COLOR]);
		}
		switch (dev->type) {
		case AG_DRIVER_MOUSE_MOTION:
			if (dsw->winop != AG_WINOP_NONE) {
				if (dsw->winSelected != win) {
					goto next_window;
				}
				AG_WM_MouseMotion(dsw, win,
				    drv->mouse->xRel,
				    drv->mouse->yRel);
			}
			AG_ProcessMouseMotion(win,
			    dev->data.motion.x, dev->data.motion.y,
			    drv->mouse->xRel, drv->mouse->yRel,
			    drv->mouse->btnState);
			if (winTop == NULL &&
			    AG_WidgetArea(win, dev->data.motion.x, dev->data.motion.y)) {
				winTop = win;
				AG_MouseCursorUpdate(win,
				    dev->data.motion.x,
				    dev->data.motion.y);
			}
			break;
		case AG_DRIVER_MOUSE_BUTTON_UP:
			AG_ProcessMouseButtonUp(win,
			    dev->data.button.x, dev->data.button.y,
			    dev->data.button.which);
			if (agWindowToFocus != NULL ||
			    !TAILQ_EMPTY(&agWindowDetachQ)) {
				goto event_processed;
			}
			break;
		case AG_DRIVER_MOUSE_BUTTON_DOWN:
			if (!AG_WidgetArea(win, dev->data.button.x,
			    dev->data.button.y)) {
				AG_ProcessMouseButtonDown(win,
				    dev->data.button.x, dev->data.button.y,
				    dev->data.button.which);
				goto next_window;
			}
			if (win->wBorderBot > 0 &&
			    !(win->flags & AG_WINDOW_NORESIZE)) {
				dsw->winop = GenericMouseOverCtrl(win,
				    dev->data.button.x, dev->data.button.y);
				if (dsw->winop != AG_WINOP_NONE) {
					win->dirty = 1;
					dsw->winSelected = win;
					goto event_processed;
				}
			}
			AG_ProcessMouseButtonDown(win,
			    dev->data.button.x, dev->data.button.y,
			    dev->data.button.which);
			goto event_processed;
		case AG_DRIVER_KEY_UP:
			if (dsw->winLastKeydown != NULL &&
			    dsw->winLastKeydown != win) {
				/*
				 * Key was initially pressed while another
				 * window was holding focus, ignore.
				 */
				dsw->winLastKeydown = NULL;
				break;
			}
			if (AG_ProcessKey(drv->kbd, win, AG_KEY_RELEASED,
			    dev->data.key.ks, dev->data.key.ucs) == 1) {
				goto event_processed;           /* Break out */
			}
			break;
		case AG_DRIVER_KEY_DOWN:
			if (AG_ProcessKey(drv->kbd, win, AG_KEY_PRESSED,
			    dev->data.key.ks, dev->data.key.ucs) == 1) {
				goto event_processed;           /* Break out */
			}
			break;
		default:
			break;
		}
next_window:
		if (useText) { AG_PopTextState(); }
		AG_ObjectUnlock(win);
	}
	if (dev->type == AG_DRIVER_MOUSE_MOTION &&
	    winTop == NULL) {
		AGDRIVER_CLASS(drv)->unsetCursor(drv);
	}
	return (0);
event_processed:
	if (useText) { AG_PopTextState(); }
	AG_ObjectUnlock(win);
	return (1);
}

/*
 * Standard processEvent() event processing method for SDL2 drivers.
 * For single-window drivers (sdl2fb and sdl2gl).
 */
int
AG_SDL2_ProcessEvent_SW(void *obj, AG_DriverEvent *dev)
{
	AG_Driver *drv = (AG_Driver *)obj;
	AG_DriverSw *dsw = (AG_DriverSw *)obj;
	int rv = 1;
	
	AG_OBJECT_ISA(drv, "AG_Driver:AG_DriverSw:*");
	AG_LockVFS(&agDrivers);

	switch (dev->type) {
	case AG_DRIVER_MOUSE_MOTION:
		rv = ProcessInputEvent_SW(drv, dev);
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:
		rv = ProcessInputEvent_SW(drv, dev);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:
		rv = ProcessInputEvent_SW(drv, dev);
		if (rv == 0 &&
		    (dsw->flags & AG_DRIVER_SW_BGPOPUP) &&
		    (dev->data.button.which == AG_MOUSE_MIDDLE ||
		     dev->data.button.which == AG_MOUSE_RIGHT)) {
			AG_WM_BackgroundPopupMenu(dsw);
			break;
		}
		break;
	case AG_DRIVER_KEY_DOWN:
		if (AG_ExecGlobalKeys(dev->data.key.ks, drv->kbd->modState) == 0) {
			rv = ProcessInputEvent_SW(drv, dev);
		} else {
			rv = 1;
		}
		break;
	case AG_DRIVER_KEY_UP:
		rv = ProcessInputEvent_SW(drv, dev);
		break;
	case AG_DRIVER_EXPOSE:
		if (!(dsw->flags & AG_DRIVER_SW_OVERLAY)) {
			AG_ClearBackground();
		}
		break;
	case AG_DRIVER_VIDEORESIZE:
		if (AG_ResizeDisplay(dev->data.videoresize.w,
		                     dev->data.videoresize.h) == -1) {
			Verbose("ResizeDisplay: %s\n", AG_GetError());
		}
		break;
	case AG_DRIVER_CLOSE:
		AG_UnlockVFS(&agDrivers);
		AG_Terminate(0);
		/* NOTREACHED */
		return (rv);
	default:
		rv = 0;
		break;
	}

	AG_UnlockVFS(&agDrivers);
	return (rv);
}

/*
 * Standard processEvent() event processing method for SDL2 drivers.
 * For multi-window drivers (sdl2mw).
 */
int
AG_SDL2_ProcessEvent_MW(void *obj, AG_DriverEvent *dev)
{
	AG_Window *win;
	AG_Driver *drv;
	int useText;
	int rv = 1;

	if ((win = dev->win) == NULL || win->flags & AG_WINDOW_DETACHING)
		return (0);

	AG_LockVFS(&agDrivers);
	AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
	drv = WIDGET(win)->drv;
	AG_OBJECT_ISA(drv, "AG_Driver:AG_DriverMw:*");

	if ((useText = (win->flags & AG_WINDOW_USE_TEXT))) {
		AG_PushTextState();
		AG_TextFont(WIDGET(win)->font);
		AG_TextColor(&WIDGET(win)->pal.c[WIDGET(win)->state]
		                                [AG_TEXT_COLOR]);
	}
	switch (dev->type) {
	case AG_DRIVER_MOUSE_MOTION:
		AG_ProcessMouseMotion(win,
		    dev->data.motion.x, dev->data.motion.y,
		    drv->mouse->xRel, drv->mouse->yRel, drv->mouse->btnState);
		AG_MouseCursorUpdate(win,
		     dev->data.motion.x, dev->data.motion.y);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:
		AG_ProcessMouseButtonDown(win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:
		AG_ProcessMouseButtonUp(win,
		    dev->data.button.x, dev->data.button.y,
		    dev->data.button.which);
		break;
	case AG_DRIVER_KEY_UP:
		AG_ProcessKey(drv->kbd, win, AG_KEY_RELEASED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_KEY_DOWN:
		AG_ProcessKey(drv->kbd, win, AG_KEY_PRESSED,
		    dev->data.key.ks, dev->data.key.ucs);
		break;
	case AG_DRIVER_MOUSE_ENTER:
		AG_PostEvent(win, "window-enter", NULL);
		break;
	case AG_DRIVER_MOUSE_LEAVE:
		AG_PostEvent(win, "window-leave", NULL);
		break;
	case AG_DRIVER_FOCUS_IN:
		if (agWindowFocused != win) {
			agWindowFocused = win;
			AG_PostEvent(win, "window-gainfocus", NULL);
		}
		break;
	case AG_DRIVER_FOCUS_OUT:
		if (agWindowFocused == win) {
			AG_PostEvent(win, "window-lostfocus", NULL);
			agWindowFocused = NULL;
		}
		break;
	case AG_DRIVER_CLOSE:
		AG_PostEvent(win, "window-close", NULL);
		break;
	default:
		rv = 0;
		break;
	}

	if (useText) {
		AG_PopTextState();
	}
	AG_UnlockVFS(&agDrivers);
	return (rv);
}

/*
 * Standard event sink for AG_EventLoop() for single-window
 * SDL2 drivers (sdl2fb, sdl2gl).
 */
int
AG_SDL2_EventSink_SW(AG_EventSink *es, AG_Event *event)
{
	AG_DriverEvent dev;
	AG_Driver *drv = AG_DRIVER_PTR(1);

	if (SDL_PollEvent(NULL) != 0) {
		while (AG_SDL2_GetNextEvent(drv, &dev) == 1) {
			if (dev.type != AG_DRIVER_UNKNOWN)
				(void)AG_SDL2_ProcessEvent_SW(drv, &dev);
		}
	} else {
		AG_Delay(1);
	}
	return (0);
}

int
AG_SDL2_EventEpilogue(AG_EventSink *es, AG_Event *event)
{
	AG_WindowDrawQueued();
	AG_WindowProcessQueued();
	return (0);
}
