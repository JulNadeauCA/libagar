/*
 * Copyright (c) 2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/anim.h>
#include <agar/gui/gui_math.h>

/* Create a new animation of the specified pixel format. */
AG_Anim *
AG_AnimNew(enum ag_anim_type type, Uint w, Uint h, const AG_PixelFormat *pf,
    Uint flags)
{
	AG_Anim *a;

	if ((a = TryMalloc(sizeof(AG_Anim))) == NULL) {
		return (NULL);
	}
	if ((a->format = AG_PixelFormatDup(pf)) == NULL) {
		Free(a);
		return (NULL);
	}
	a->type = type;
	a->flags = flags;
	a->w = w;
	a->h = h;
	a->n = 0;
	a->pitch = w*pf->BytesPerPixel;
	a->clipRect = AG_RECT(0,0,w,h);
	a->f = NULL;
	a->fpsOrig = 1.0;
	AG_MutexInitRecursive(&a->lock);

	return (a);
}

/* Create an empty animation. */
AG_Anim *
AG_AnimEmpty(void)
{
	return AG_AnimNew(AG_ANIM_PACKED, 0,0, agSurfaceFmt, 0);
}

/* Create a new color-index animation of given dimensions and depth. */
AG_Anim *
AG_AnimIndexed(Uint w, Uint h, int bpp, Uint flags)
{
	AG_PixelFormat *pf;
	AG_Anim *a;

	if ((pf = AG_PixelFormatIndexed(bpp)) == NULL) {
		return (NULL);
	}
	a = AG_AnimNew(AG_ANIM_INDEXED, w,h, pf, 0);
	AG_PixelFormatFree(pf);
	return (a);
}

/* Create a new packed-pixel animation with the specified RGB pixel format. */
AG_Anim *
AG_AnimRGB(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask)
{
	AG_PixelFormat *pf;
	AG_Anim *a;

	if ((pf = AG_PixelFormatRGB(bpp, Rmask, Gmask, Bmask)) == NULL) {
		return (NULL);
	}
	a = AG_AnimNew(AG_ANIM_PACKED, w,h, pf, 0);
	AG_PixelFormatFree(pf);
	return (a);
}

/*
 * Create a new packed-pixel animation with the specified RGBA pixel format.
 * The SRCALPHA flag is set implicitely.
 */
AG_Anim *
AG_AnimRGBA(Uint w, Uint h, int bpp, Uint flags, Uint32 Rmask, Uint32 Gmask,
    Uint32 Bmask, Uint32 Amask)
{
	AG_PixelFormat *pf;
	AG_Anim *a;

	if ((pf = AG_PixelFormatRGBA(bpp, Rmask, Gmask, Bmask, Amask)) == NULL) {
		return (NULL);
	}
	a = AG_AnimNew(AG_ANIM_PACKED, w,h, pf, AG_SRCALPHA);
	AG_PixelFormatFree(pf);
	return (a);
}

/* Load an animation from a series of PNG files. */
AG_Anim *
AG_AnimFromPNGs(const char *pattern)
{
	AG_Anim *a = NULL;
	AG_Surface *su;
	char path[AG_PATHNAME_MAX];
	int i;

	for (i = 0; ; i++) {
		Snprintf(path, sizeof(path), pattern, i);
		if (!AG_FileExists(path)) {
			if (i == 0) {
				continue;
			} else {
				break;
			}
		}
		if ((su = AG_SurfaceFromPNG(path)) == NULL) {
			break;
		}
		if (a == NULL) {
			a = AG_AnimRGBA(su->w, su->h,
			    su->format->BitsPerPixel, 0,
			    su->format->Rmask,
			    su->format->Gmask,
			    su->format->Bmask,
			    su->format->Amask);
			if (a == NULL) {
				AG_SurfaceFree(su);
				return (NULL);
			}
		}
		if (AG_AnimFrameNew(a,su) == -1) {
			goto fail;
		}
	}
	return (a);
fail:
	AG_AnimFree(a);
	return (NULL);
}

/* Set one or more entries in an indexed animation's palette. */
int
AG_AnimSetPalette(AG_Anim *a, AG_Color *c, Uint offs, Uint count)
{
	Uint i;

	AG_MutexLock(&a->lock);
	if (a->type != AG_ANIM_INDEXED) {
		AG_SetError("Not an indexed animation");
		goto fail;
	}
	if (offs >= a->format->palette->nColors ||
	    offs+count >= a->format->palette->nColors) {
		AG_SetError("Bad palette offset/count");
		goto fail;
	}
	for (i = 0; i < count; i++) {
		a->format->palette->colors[offs+i] = c[i];
	}
	AG_MutexUnlock(&a->lock);
	return (0);
fail:
	AG_MutexUnlock(&a->lock);
	return (-1);
}

/*
 * Return a newly-allocated duplicate of an animation.
 * The source animation must be locked.
 */
AG_Anim *
AG_AnimDup(const AG_Anim *sa)
{
	AG_Anim *a;
	int i;

	a = AG_AnimNew(sa->type, sa->w, sa->h, sa->format,
	    (sa->flags & AG_SAVED_ANIM_FLAGS));
	if (a == NULL) {
		return (NULL);
	}
	if ((a->f = TryMalloc(sa->n*sizeof(AG_AnimFrame))) == NULL) {
		goto fail;
	}
	for (i = 0; i < sa->n; i++) {
		AG_AnimFrame *af = &a->f[i];

		if ((af->pixels = TryMalloc(sa->h*sa->pitch)) == NULL) {
			goto fail;
		}
		memcpy(af->pixels, sa->f[i].pixels, sa->h*sa->pitch);
		a->n++;
	}
	return (a);
fail:
	AG_AnimFree(a);
	return (NULL);
}

/* Resize an animation; pixels are left uninitialized. */
int
AG_AnimResize(AG_Anim *a, Uint w, Uint h)
{
	Uint8 *pixelsNew;
	int i;
	int pitchNew;

	AG_MutexLock(&a->lock);

	pitchNew = w*a->format->BytesPerPixel;
	for (i = 0; i < a->n; i++) {
		AG_AnimFrame *af = &a->f[i];
		if ((pixelsNew = TryRealloc(af->pixels, h*pitchNew)) == NULL) {
			for (i--; i >= 0; i--) {
				Free(af->pixels);
			}
			goto fail;
		}
		af->pixels = pixelsNew;
	}
	a->pitch = pitchNew;
	a->w = w;
	a->h = h;
	a->clipRect = AG_RECT(0,0,w,h);

	AG_MutexUnlock(&a->lock);
	return (0);
fail:
	AG_MutexUnlock(&a->lock);
	return (-1);
}

/* Free the specified animation. */
void
AG_AnimFree(AG_Anim *a)
{
	int i;

	AG_PixelFormatFree(a->format);
	AG_MutexDestroy(&a->lock);

	for (i = 0; i < a->n; i++) {
		AG_AnimFrame *af = &a->f[i];
		Free(af->pixels);
	}
	Free(a->f);
	Free(a);
}

void
AG_AnimStateInit(AG_Anim *an, AG_AnimState *ast)
{
	AG_MutexInitRecursive(&ast->lock);
	ast->an = an;
	ast->flags = 0;
	ast->play = 0;
	ast->f = 0;
	
	AG_MutexLock(&an->lock);
	ast->fps = an->fpsOrig;
	AG_MutexUnlock(&an->lock);
}

void
AG_AnimStateDestroy(AG_Anim *an, AG_AnimState *ast)
{
	ast->play = 0;
	AG_MutexDestroy(&ast->lock);
}

/* Set original playback speed. */
void
AG_AnimSetOrigFPS(AG_Anim *an, double fps)
{
	AG_MutexLock(&an->lock);
	an->fpsOrig = fps;
	AG_MutexUnlock(&an->lock);
}

/* Set effective playback speed. */
void
AG_AnimSetFPS(AG_AnimState *ast, double fps)
{
	AG_MutexLock(&ast->lock);
	ast->fps = fps;
	AG_MutexUnlock(&ast->lock);
}

void
AG_AnimSetLoop(AG_AnimState *ast, int enable)
{
	AG_MutexLock(&ast->lock);
	if (enable) {
		ast->flags |= AG_ANIM_LOOP;
		ast->flags &= ~(AG_ANIM_PINGPONG);
	} else {
		ast->flags &= ~(AG_ANIM_LOOP);
	}
	AG_MutexUnlock(&ast->lock);
}

void
AG_AnimSetPingPong(AG_AnimState *ast, int enable)
{
	AG_MutexLock(&ast->lock);
	if (enable) {
		ast->flags |= AG_ANIM_PINGPONG;
		ast->flags &= ~(AG_ANIM_LOOP);
	} else {
		ast->flags &= ~(AG_ANIM_PINGPONG);
	}
	AG_MutexUnlock(&ast->lock);
}

/* Set the source alpha flag and per-animation alpha. */
void
AG_AnimSetAlpha(AG_Anim *an, Uint flags, Uint8 alpha)
{
	AG_MutexLock(&an->lock);
	if (flags & AG_SRCALPHA) {
		an->flags |= AG_SRCALPHA;
	} else {
		an->flags &= ~(AG_SRCALPHA);
	}
	an->format->alpha = alpha;
	AG_MutexUnlock(&an->lock);
}

/* Set the source colorkey flag and per-animation colorkey. */
void
AG_AnimSetColorKey(AG_Anim *an, Uint flags, Uint32 colorkey)
{
	AG_MutexLock(&an->lock);
	if (flags & AG_SRCCOLORKEY) {
		an->flags |= AG_SRCCOLORKEY;
	} else {
		an->flags &= ~(AG_SRCCOLORKEY);
	}
	an->format->colorkey = colorkey;
	AG_MutexUnlock(&an->lock);
}

/* Animation processing loop */
static void *
AnimProc(void *arg)
{
	AG_AnimState *ast = arg;
	Uint32 delay;

	while (ast->play) {
		AG_MutexLock(&ast->lock);
		AG_MutexLock(&ast->an->lock);

		if (ast->an->n < 1) {
			AG_MutexUnlock(&ast->an->lock);
			AG_MutexUnlock(&ast->lock);
			goto out;
		}
		if (ast->f & AG_ANIM_REVERSE) {
			if (--ast->f < 0) {
				if (ast->flags & AG_ANIM_LOOP) {
					ast->f = (ast->an->n - 1);
				} else if (ast->flags & AG_ANIM_PINGPONG) {
					ast->f = 0;
					ast->flags &= ~(AG_ANIM_REVERSE);
				} else {
					ast->play = 0;
					AG_MutexUnlock(&ast->an->lock);
					AG_MutexUnlock(&ast->lock);
					goto out;
				}
			}
		} else {
			if (++ast->f >= ast->an->n) {
				if (ast->flags & AG_ANIM_LOOP) {
					ast->f = 0;
				} else if (ast->flags & AG_ANIM_PINGPONG) {
					ast->f--;
					ast->flags |= AG_ANIM_REVERSE;
				} else {
					ast->play = 0;
					AG_MutexUnlock(&ast->an->lock);
					AG_MutexUnlock(&ast->lock);
					goto out;
				}
			}
		}

		delay = (Uint32)(1000.0/ast->fps);

		AG_MutexUnlock(&ast->an->lock);
		AG_MutexUnlock(&ast->lock);

		AG_Delay(delay);
	}
out:
	return (NULL);
}

int
AG_AnimPlay(AG_AnimState *ast)
{
	int rv = 0;

	AG_MutexLock(&ast->lock);
	ast->play = 1;
#ifdef AG_THREADS
	if (AG_ThreadTryCreate(&ast->th, AnimProc, ast) != 0) {
		AG_SetError("Failed to create playback thread");
		rv = -1;
		ast->play = 0;
	}
#else
	AG_SetError("AG_AnimPlay() requires threads support");
	rv = -1;
#endif
	AG_MutexUnlock(&ast->lock);
	return (rv);
}

void
AG_AnimStop(AG_AnimState *ast)
{
	AG_MutexLock(&ast->lock);
	ast->play = 0;
	AG_MutexUnlock(&ast->lock);
}

/* Insert a new animation frame. */
int
AG_AnimFrameNew(AG_Anim *a, const AG_Surface *su)
{
	AG_AnimFrame *afNew, *af;
	AG_Surface *suTmp = NULL;
	int nf;

	AG_MutexLock(&a->lock);

	if (su->w != a->w || su->h != a->h) {
		if (AG_ScaleSurface(su, a->w, a->h, &suTmp) == -1)
			goto fail;
	} else {
		if ((suTmp = AG_SurfaceConvert(su, a->format)) == NULL)
			goto fail;
	}

	if ((afNew = TryRealloc(a->f, (a->n+1)*sizeof(AG_AnimFrame))) == NULL) {
		goto fail;
	}
	a->f = afNew;
	af = &a->f[a->n];
	af->flags = 0;
	if ((af->pixels = TryMalloc(su->h*a->pitch)) == NULL) {
		a->n--;
		goto fail;
	}
	memcpy(af->pixels, suTmp->pixels, su->h*a->pitch);
	nf = a->n++;
	AG_MutexUnlock(&a->lock);

	AG_SurfaceFree(suTmp);
	return (nf);
fail:
	AG_MutexUnlock(&a->lock);
	if (suTmp != NULL) {
		AG_SurfaceFree(suTmp);
	}
	return (-1);
}

/* Return a new surface from a given frame#. */
AG_Surface *
AG_AnimFrameToSurface(AG_Anim *a, int f)
{
	AG_Surface *su;
	AG_AnimFrame *af;

	AG_MutexLock(&a->lock);
	if (f < 0 || f >= a->n) {
		AG_SetError("No such frame#");
		AG_MutexUnlock(&a->lock);
		return (NULL);
	}
	af = &a->f[f];
	if (a->format->Amask != 0) {
		su = AG_SurfaceFromPixelsRGBA(af->pixels, a->w, a->h,
		    a->format->BitsPerPixel,
		    a->format->Rmask,
		    a->format->Gmask,
		    a->format->Bmask,
		    a->format->Amask);
	} else {
		su = AG_SurfaceFromPixelsRGB(af->pixels, a->w, a->h,
		    a->format->BitsPerPixel,
		    a->format->Rmask,
		    a->format->Gmask,
		    a->format->Bmask);
	}
	AG_MutexUnlock(&a->lock);
	return (su);
}
