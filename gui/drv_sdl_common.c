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
 * Common code for SDL 1.2 drivers.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/packedpixel.h>
#include <agar/gui/cursors.h>
#include <agar/gui/sdl.h>
#include <agar/gui/text.h>

#define AG_SDL_CLIPPED_PIXEL(s, ax, ay)			\
	((ax) <  (s)->clip_rect.x ||			\
	 (ax) >= (s)->clip_rect.x + (s)->clip_rect.w ||	\
	 (ay) <  (s)->clip_rect.y ||			\
	 (ay) >= (s)->clip_rect.y + (s)->clip_rect.h)

static void AG_SDL_SoftBlit_Colorkey(const AG_Surface *_Nonnull, AG_Rect,
                                     SDL_Surface *_Nonnull, AG_Rect);
static void AG_SDL_SoftBlit_NoColorkey(const AG_Surface *_Nonnull, AG_Rect,
                                       SDL_Surface *_Nonnull, AG_Rect);

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
	AG_DriverClass *dc = NULL, **pd;
	int useGL = 0;

	if (AG_InitGUIGlobals() == -1)
		return (-1);
	
	/* Enable OpenGL mode if the surface has SDL_OPENGL set. */
	if ((display->flags & SDL_OPENGL)
#ifdef SDL_OPENGLBLIT
	    || (display->flags & SDL_OPENGLBLIT)
#endif
	    ) {
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
		    ((*pd)->flags & AG_DRIVER_SDL)) {
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
		    useGL ? "SDL/OpenGL" : "SDL");
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
AG_SetVideoSurfaceSDL(void *pDisplay)
{
	if (agDriverSw == NULL ||
	    !(agDriverOps->flags & AG_DRIVER_SDL)) {
		AG_SetError("Current driver is not an SDL driver");
		return (-1);
	}
	if (AGDRIVER_SW_CLASS(agDriverSw)->setVideoContext(agDriverSw,
	    pDisplay) == -1) {
		return (-1);
	}
	AG_PostResizeDisplay(agDriverSw);
	return (0);
}

/* Return the Agar pixel format corresponding to that of an SDL surface. */
AG_PixelFormat *
AG_SDL_GetPixelFormat(const SDL_Surface *s)
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
AG_SDL_GetPixel(SDL_Surface *_Nonnull s, Uint8 *_Nonnull p)
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
AG_SDL_PutPixel(SDL_Surface *_Nonnull s, Uint8 *_Nonnull p, Uint32 px)
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
AG_SDL_PutPixel2(SDL_Surface *_Nonnull S, int x, int y, Uint32 px)
{
	AG_SDL_PutPixel(S,
	    (Uint8 *)S->pixels + y*S->pitch + x*S->format->BytesPerPixel,
	    px);
}

/*
 * Blend an AG_Color c against the pixel at p in an SDL_Surface.
 * The surface must be locked.
 */
static void
AG_SDL_SurfaceBlend(SDL_Surface *_Nonnull s, Uint8 *_Nonnull p, AG_Color c,
    AG_AlphaFn fn)
{
	Uint32 px = AG_SDL_GetPixel(s,p);
	Uint8 r = AG_Hto8(c.r);
	Uint8 g = AG_Hto8(c.g);
	Uint8 b = AG_Hto8(c.b);
	Uint8 a = AG_Hto8(c.a);
	Uint8 R,G,B,A;

#if SDL_COMPILEDVERSION < SDL_VERSIONNUM(1,3,0)
	if ((s->flags & SDL_SRCCOLORKEY) &&           /* Replace transparent */
	    (px == s->format->colorkey)) {
		AG_SDL_PutPixel(s, p, SDL_MapRGBA(s->format, r,g,b,a));
		return;
	}
#endif
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

	AG_SDL_PutPixel(s, p,
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
AG_SDL_BlitSurface(const AG_Surface *S, const AG_Rect *srcRect,
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
		AG_SDL_SoftBlit_Colorkey(S,sr, D,dr);
	} else {
		AG_SDL_SoftBlit_NoColorkey(S,sr, D,dr);
	}

	if (SDL_MUSTLOCK(D)) { SDL_UnlockSurface(D); }
}
static void
AG_SDL_SoftBlit_Colorkey(const AG_Surface *_Nonnull S, AG_Rect sr,
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
				AG_SDL_SurfaceBlend(ds, pDst, c, AG_ALPHA_SRC);
			} else {
				AG_SDL_PutPixel2(ds,
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
AG_SDL_SoftBlit_NoColorkey(const AG_Surface *_Nonnull S, AG_Rect sr,
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
				AG_SDL_SurfaceBlend(ds, pDst, c, AG_ALPHA_SRC);
			} else {
				AG_SDL_PutPixel2(ds,
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
 * Inherit SDL_SRCCOLORKEY -> AG_SURFACE_COLORKEY flag and colorkey value.
 * Inherit SDL_SRCALPHA -> AG_SURFACE_ALPHA and per-surface alpha.
 */
AG_Surface *
AG_SDL_ImportSurface(SDL_Surface *src)
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
	if (src->flags & SDL_SRCCOLORKEY) {
		dst->flags |= AG_SURFACE_COLORKEY;
		dst->colorkey = src->format->colorkey;
	}
	if (src->flags & SDL_SRCALPHA)    {
		dst->flags |= AG_SURFACE_ALPHA;
		dst->alpha = src->format->alpha;
	}
	
	if (SDL_MUSTLOCK(src)) { SDL_LockSurface(src); }

	memcpy(dst->pixels, (Uint8 *)src->pixels, src->h * src->pitch);

	if (SDL_MUSTLOCK(src)) { SDL_UnlockSurface(src); }

	AG_PixelFormatFree(&pf);
	return (dst);
}

/* Convert a SDL surface to Agar surface. */
AG_Surface *
AG_SurfaceFromSDL(void *p)
{
	return AG_SDL_ImportSurface((SDL_Surface *)p);
}

/* Export an AG_Surface to a newly-created SDL_Surface. */
void *
AG_SurfaceExportSDL(const AG_Surface *ss)
{
	Uint32 sdlFlags = SDL_SWSURFACE;
	SDL_Surface *ds;

	if (ss->flags & AG_SURFACE_COLORKEY) { sdlFlags |= SDL_SRCCOLORKEY; }
	if (ss->flags & AG_SURFACE_ALPHA)    { sdlFlags |= SDL_SRCALPHA; }

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
	AG_SDL_BlitSurface(ss, NULL, ds, 0,0);
	return (void *)ds;
}

/* Initialize the default cursor. */
void
AG_SDL_InitDefaultCursor(void *obj)
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
AG_SDL_SetCursor(void *obj, AG_Cursor *ac)
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
AG_SDL_UnsetCursor(void *obj)
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
AG_SDL_SetRefreshRate(void *obj, int fps)
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
AG_SDL_CreateCursor(void *obj, Uint w, Uint h, const Uint8 *data,
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
AG_SDL_FreeCursor(void *obj, AG_Cursor *ac)
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
AG_SDL_GetCursorVisibility(void *obj)
{
	return (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE);
}

/* Set cursor visibility. */
void
AG_SDL_SetCursorVisibility(void *obj, int flag)
{
	SDL_ShowCursor(flag ? SDL_ENABLE : SDL_DISABLE);
}

/* Return the desktop display size in pixels. */
int
AG_SDL_GetDisplaySize(Uint *w, Uint *h)
{
	const SDL_VideoInfo *vi;

	vi = SDL_GetVideoInfo();
#if SDL_VERSION_ATLEAST(1,2,10)
	*w = (Uint)vi->current_w;
	*h = (Uint)vi->current_h;
#else
	*w = 320;		/* Arbitrary */
	*h = 240;
#endif
	return (0);
}

/* Apply default display settings where 0 is specified. */
void
AG_SDL_GetPrefDisplaySettings(void *obj, Uint *w, Uint *h, int *depth)
{
	char buf[16], *ep;
	AG_Driver *drv = obj;
	AG_DriverSw *dsw = obj;
	Uint wDisp, hDisp;

	if (*w == 0 || *h == 0) {
		AG_SDL_GetDisplaySize(&wDisp, &hDisp);
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
AG_SDL_BeginEventProcessing(void *obj)
{
	/* Nothing to do */
}

/* Standard pendingEvents() method for SDL drivers. */
int
AG_SDL_PendingEvents(void *obj)
{
	return (SDL_PollEvent(NULL) != 0);
}

/* Translate an SDL_Event to an AG_DriverEvent. */
void
AG_SDL_TranslateEvent(void *obj, const SDL_Event *ev, AG_DriverEvent *dev)
{
	AG_Driver *drv = obj;

	switch (ev->type) {
	case SDL_MOUSEMOTION:
		AG_MouseMotionUpdate(drv->mouse, ev->motion.x, ev->motion.y);

		dev->type = AG_DRIVER_MOUSE_MOTION;
		dev->win = NULL;
		dev->data.motion.x = ev->motion.x;
		dev->data.motion.y = ev->motion.y;
		break;
	case SDL_MOUSEBUTTONUP:
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_RELEASED,
		    ev->button.button);

		dev->type = AG_DRIVER_MOUSE_BUTTON_UP;
		dev->win = NULL;
		dev->data.button.which = (AG_MouseButton)ev->button.button;
		dev->data.button.x = ev->button.x;
		dev->data.button.y = ev->button.y;
		break;
	case SDL_MOUSEBUTTONDOWN:
		AG_MouseButtonUpdate(drv->mouse, AG_BUTTON_PRESSED,
		    ev->button.button);

		dev->type = AG_DRIVER_MOUSE_BUTTON_DOWN;
		dev->win = NULL;
		dev->data.button.which = (AG_MouseButton)ev->button.button;
		dev->data.button.x = ev->button.x;
		dev->data.button.y = ev->button.y;
		break;
	case SDL_KEYDOWN:
		AG_KeyboardUpdate(drv->kbd, AG_KEY_PRESSED,
		    (AG_KeySym)ev->key.keysym.sym);
	
		dev->type = AG_DRIVER_KEY_DOWN;
		dev->win = NULL;
		dev->data.key.ks = (AG_KeySym)ev->key.keysym.sym;
		dev->data.key.ucs = (AG_Char)ev->key.keysym.unicode;
		break;
	case SDL_KEYUP:
		AG_KeyboardUpdate(drv->kbd, AG_KEY_RELEASED,
		    (AG_KeySym)ev->key.keysym.sym);

		dev->type = AG_DRIVER_KEY_UP;
		dev->win = NULL;
		dev->data.key.ks = (AG_KeySym)ev->key.keysym.sym;
		dev->data.key.ucs = (AG_Char)ev->key.keysym.unicode;
		break;
	case SDL_VIDEORESIZE:
		dev->type = AG_DRIVER_VIDEORESIZE;
		dev->win = NULL;
		dev->data.videoresize.x = 0;
		dev->data.videoresize.y = 0;
		dev->data.videoresize.w = (int)ev->resize.w;
		dev->data.videoresize.h = (int)ev->resize.h;
		break;
	case SDL_VIDEOEXPOSE:
		dev->type = AG_DRIVER_EXPOSE;
		dev->win = NULL;
		break;
	case SDL_QUIT:
	case SDL_USEREVENT:
		dev->type = AG_DRIVER_CLOSE;
		dev->win = NULL;
		break;
	default:
		dev->type = AG_DRIVER_UNKNOWN;
		dev->win = NULL;
		break;
	}
}

/* Standard getNextEvent() method for SDL drivers. */
int
AG_SDL_GetNextEvent(void *obj, AG_DriverEvent *dev)
{
	AG_Driver *drv = obj;
	SDL_Event ev;

	if (SDL_PollEvent(&ev) == 0) {
		return (0);
	}
	AG_SDL_TranslateEvent(drv, &ev, dev);
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
 * Process an input device event.
 *
 * The agDrivers VFS must be locked.
 *
 * TODO: generalize this code for single-window drivers in general.
 */
static int
ProcessInputEvent(AG_Driver *_Nonnull drv, AG_DriverEvent *_Nonnull dev)
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

/* Standard processEvent() method for SDL drivers. */
int
AG_SDL_ProcessEvent(void *obj, AG_DriverEvent *dev)
{
	AG_Driver *drv = (AG_Driver *)obj;
	AG_DriverSw *dsw = (AG_DriverSw *)obj;
	int rv = 1;

	AG_LockVFS(&agDrivers);

	switch (dev->type) {
	case AG_DRIVER_MOUSE_MOTION:
		rv = ProcessInputEvent(drv, dev);
		break;
	case AG_DRIVER_MOUSE_BUTTON_UP:
		rv = ProcessInputEvent(drv, dev);
		break;
	case AG_DRIVER_MOUSE_BUTTON_DOWN:
		rv = ProcessInputEvent(drv, dev);
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
			rv = ProcessInputEvent(drv, dev);
		} else {
			rv = 1;
		}
		break;
	case AG_DRIVER_KEY_UP:
		rv = ProcessInputEvent(drv, dev);
		break;
	case AG_DRIVER_EXPOSE:
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
 * Standard event sink for AG_EventLoop().
 *
 * TODO where AG_SINK_READ capability and pipes are available,
 * could we create a separate thread running SDL_WaitEvent() and
 * sending notifications over a pipe, instead of using a spinner?
 */
int
AG_SDL_EventSink(AG_EventSink *es, AG_Event *event)
{
	AG_DriverEvent dev;
	AG_Driver *drv = AG_DRIVER_PTR(1);

	if (SDL_PollEvent(NULL) != 0) {
		while (AG_SDL_GetNextEvent(drv, &dev) == 1)
			(void)AG_SDL_ProcessEvent(drv, &dev);
	} else {
		AG_Delay(1);
	}
	return (0);
}
int
AG_SDL_EventEpilogue(AG_EventSink *es, AG_Event *event)
{
	AG_WindowDrawQueued();
	AG_WindowProcessQueued();
	return (0);
}
