/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/gui/gui.h>
#include <agar/gui/widget.h>

#include <agar/map/rg_tileset.h>

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

const struct rg_transform_ops rgTransforms[];
const int rgTransformsCount;

#if 0
/*
 * Add a new rotate transformation or update an existing one. If angle is 0,
 * any existing rotation is removed.
 */
RG_Transform *
TransformRotate(struct map_item *r, int angle)
{
	Uint32 angles = (Uint32)angle;
	RG_Transform *tr;
	AG_Surface *su;
	float rad, theta;

	switch (r->type) {
	case MAP_ITEM_TILE:
		su = AG_SPRITE(r->r_sprite.obj,r->r_sprite.offs).su;
		break;
	default:
		return (NULL);
	}

	rad = Hypot(
	    r->r_gfx.xorigin - su->w/2,
	    r->r_gfx.yorigin - su->h/2);
	theta = Atan2(
	    r->r_gfx.yorigin - su->w/2,
	    r->r_gfx.xorigin - su->h/2);

	theta += ((float)angle/360.0)*(2.0*RG_PI);
	r->r_gfx.xorigin = rad*Cos(theta) + su->w/2;
	r->r_gfx.yorigin = rad*Sin(theta) + su->h/2;

	TAILQ_FOREACH(tr, &r->transforms, transforms) {
		if (tr->type == RG_TRANSFORM_ROTATE) {
			if (angle == 0) {
				TAILQ_REMOVE(&r->transforms, tr, transforms);
				Free(tr);
				return (NULL);
			}
			break;
		}
	}
	if (angle == 0) {
		return (NULL);
	}
	if (tr == NULL) {
		tr = RG_TransformNew(RG_TRANSFORM_ROTATE, 1, &angles);
		TAILQ_INSERT_TAIL(&r->transforms, tr, transforms);
	} else {
		tr->args[0] = angles;
	}
	return (tr);
}
#endif

RG_Transform *
RG_TransformNew(enum rg_transform_type type, int nArgs, Uint32 *args)
{
	RG_Transform *xf;

	xf = Malloc(sizeof(RG_Transform));
	if (RG_TransformInit(xf, type, nArgs, args) == -1) {
		Free(xf);
		return (NULL);
	}
	return (xf);
}

int
RG_TransformInit(RG_Transform *xf, enum rg_transform_type type, Uint nArgs,
    Uint32 *args)
{
	int i;

	if (nArgs > RG_TRANSFORM_MAX_ARGS) {
		AG_SetError("Too many transform args");
		return (-1);
	}

	memset(xf, 0, sizeof(RG_Transform));
	xf->type = type;
	if ((xf->nArgs = nArgs) > 0) {
		xf->args = Malloc(nArgs*sizeof(Uint32));
		memcpy(xf->args, args, nArgs * sizeof(Uint32));
	} else {
		xf->args = NULL;
	}

	for (i=0, xf->func = NULL;
	     i < rgTransformsCount;
	     i++) {
		if (rgTransforms[i].type == type) {
			xf->func = rgTransforms[i].func;
			break;
		}
	}
	return (0);
}

void
RG_TransformChainInit(RG_TransformChain *xchain)
{
	TAILQ_INIT(xchain);
}

int
RG_TransformChainLoad(AG_DataSource *buf, RG_TransformChain *xchain)
{
	Uint32 i, count = 0;

	if ((count = AG_ReadUint32(buf)) > RG_TRANSFORM_CHAIN_MAX) {
		AG_SetError("Too many transforms in chain: %u", (Uint)count);
		return (-1);
	}
	for (i = 0; i < count; i++) {
		RG_Transform *xf;

		xf = Malloc(sizeof(RG_Transform));
		RG_TransformInit(xf, 0, 0, NULL);
		if (RG_TransformLoad(buf, xf) == -1) {
			Free(xf);
			return (-1);
		}
		TAILQ_INSERT_TAIL(xchain, xf, transforms);
	}
	return (0);
}

void
RG_TransformChainSave(AG_DataSource *buf, const RG_TransformChain *xchain)
{
	RG_Transform *xf;
	Uint32 count = 0;
	off_t offs;

	offs = AG_Tell(buf);
	AG_WriteUint32(buf, 0);
	TAILQ_FOREACH(xf, xchain, transforms) {
		RG_TransformSave(buf, xf);
		count++;
	}
	AG_WriteUint32At(buf, count, offs);
}

void
RG_TransformChainDestroy(RG_TransformChain *xchain)
{
	RG_Transform *xf, *xf_next;
	
	for (xf = TAILQ_FIRST(xchain);
	     xf != TAILQ_END(xchain);
	     xf = xf_next) {
		xf_next = TAILQ_NEXT(xf, transforms);
		RG_TransformDestroy(xf);
		Free(xf);
	}
}

void
RG_TransformChainPrint(const RG_TransformChain *xchain, char *buf,
    AG_Size buf_size)
{
	extern const struct rg_transform_ops rgTransforms[];
	extern const int rgTransformsCount;
	RG_Transform *tr;
	int i, j;

	TAILQ_FOREACH(tr, xchain, transforms) {
		for (i = 0; i < rgTransformsCount; i++) {
			if (rgTransforms[i].type == tr->type)
				break;
		}
		if (i < rgTransformsCount) {
			Strlcat(buf, "+", buf_size);
			Strlcat(buf, rgTransforms[i].name, buf_size);
			for (j = 0; j < tr->nArgs; j++) {
				char num[32];

				Snprintf(num, sizeof(num), "(%lu)",
				    (unsigned long)tr->args[i]);
				Strlcat(buf, num, buf_size);
			}
		}
	}
	if (!TAILQ_EMPTY(xchain))
		Strlcat(buf, "\n", buf_size);
}

void
RG_TransformChainDup(const RG_TransformChain *xc, RG_TransformChain *xc_dup)
{
	RG_Transform *xf, *xf_dup;

	TAILQ_FOREACH(xf, xc, transforms) {
		xf_dup = Malloc(sizeof(RG_Transform));
		RG_TransformInit(xf_dup, xf->type, xf->nArgs, xf->args);
		TAILQ_INSERT_TAIL(xc_dup, xf_dup, transforms);
	}
}

int
RG_TransformCompare(const RG_Transform *xf1, const RG_Transform *xf2)
{
	return (xf1->type == xf2->type &&
	        xf1->nArgs == xf2->nArgs &&
		(xf1->nArgs == 0 ||
		 memcmp(xf1->args, xf2->args, xf1->nArgs*sizeof(Uint32)) == 0));
}

void
RG_TransformDestroy(RG_Transform *xf)
{
	Free(xf->args);
}

int
RG_TransformLoad(AG_DataSource *buf, RG_Transform *xf)
{
	int i;

	xf->type = AG_ReadUint8(buf);
	xf->func = NULL;
	xf->nArgs = (int)AG_ReadUint8(buf);
	if (xf->nArgs > RG_TRANSFORM_MAX_ARGS) {
		AG_SetError("Too many transform args: %u", (Uint)xf->nArgs);
		return (-1);
	}

	Free(xf->args);
	xf->args = Malloc(xf->nArgs * sizeof(Uint32));
	for (i = 0; i < xf->nArgs; i++)
		xf->args[i] = AG_ReadUint32(buf);

	/* Look for a matching algorithm. */
	for (i = 0; i < rgTransformsCount; i++) {
		if (rgTransforms[i].type == xf->type) {
			xf->func = rgTransforms[i].func;
			break;
		}
	}
	if (xf->func == NULL) {
		AG_SetError("Unimplemented transform: %u", (Uint)xf->type);
		return (-1);
	}
	return (0);
}

void
RG_TransformSave(AG_DataSource *buf, const RG_Transform *xf)
{
	int i;

	AG_WriteUint8(buf, xf->type);
	AG_WriteUint8(buf, xf->nArgs);

	for (i = 0; i < xf->nArgs; i++)
		AG_WriteUint32(buf, xf->args[i]);
}

/* Flip a surface horizontally. */
static AG_Surface *_Nonnull
TransformMirror(AG_Surface *_Nonnull S, int argc, Uint32 *_Nonnull argv)
{
	Uint8 *row, *pRow;
	Uint8 *p = S->pixels;
	int Bpp = S->format.BytesPerPixel;
	int pitch = S->pitch;
	int x, y;

	row = Malloc(pitch);
	for (y = 0; y < S->h; y++) {
		memcpy(row, p, pitch);
		pRow = row + pitch - Bpp;
		for (x = 0; x < S->w; x++) {
			AG_SurfacePut32_At(S, p,
			    AG_SurfaceGet32_At(S, pRow));
			p += Bpp;
			pRow -= Bpp;
		}
	}
	free(row);
	return (S);
}

/* Flip a surface vertically. */
static AG_Surface *_Nonnull
TransformFlip(AG_Surface *_Nonnull S, int argc, Uint32 *_Nonnull argv)
{
	Uint8 *pRow, *row;
	Uint8 *p = S->pixels;
	int pitch = S->pitch;
	AG_Size totsize = S->h*pitch;
	int y;

	row = Malloc(totsize);
	memcpy(row, p, totsize);
	pRow = row + totsize - pitch;
	for (y = 0; y < S->h; y++) {
		memcpy(p, pRow, pitch);
		pRow -= pitch;
		p += pitch;
	}
	free(row);
	return (S);
}

/* Rotate a surface by the given number of degrees. */
static AG_Surface *_Nonnull
TransformRotate(AG_Surface *_Nonnull S, int argc, Uint32 *_Nonnull argv)
{
	AG_Surface *Snew;
	Uint32 theta = argv[0];
	int x, y;
	int xp, yp;
	int swapdims = (theta == 90 || theta == 270);

	Snew = AG_SurfaceRGBA(
	    swapdims ? S->h : S->w,
	    swapdims ? S->w : S->h,
	    S->format.BitsPerPixel,
	    (S->flags & (AG_SURFACE_ALPHA|AG_SURFACE_COLORKEY)),
	    S->format.Rmask,
	    S->format.Gmask,
	    S->format.Bmask,
	    S->format.Amask);
	if (Snew == NULL) {
		AG_FatalError(NULL);
	}
	switch (theta) {
	case 90:
		for (y = 0; y < S->h; y++) {
			for (x = 0; x < S->w; x++) {
				AG_SurfacePut32(Snew, y, x,
				    AG_SurfaceGet32(S, x, (S->h - y - 1)));
			}
		}
		break;
	case 180:
		for (y = 0, yp = S->h-1; y < S->h; y++, yp--) {
			for (x = 0, xp = S->w-1; x < S->w; x++, xp--) {
				AG_SurfacePut32(Snew, x,y,
				    AG_SurfaceGet32(S, xp,yp));
			}
		}
		break;
	case 270:
		for (y = 0; y < S->h; y++) {
			for (x = 0; x < S->w; x++) {
				AG_SurfacePut32(Snew, y, x,
				    AG_SurfaceGet32(S, (S->w - x - 1), y));
			}
		}
		break;
	default:
		/* TODO */
		break;
	}
	return (Snew);
}

/* Invert the colors of a surface. */
static AG_Surface *_Nonnull
TransformInvertRGB(AG_Surface *_Nonnull S, int argc, Uint32 *_Nonnull argv)
{
	AG_Size size = S->w*S->h;
	Uint8 *p = S->pixels;
	AG_Color c;
	int i;

	for (i = 0; i < size; i++) {
		AG_GetColor32(&c, AG_SurfaceGet32_At(S,p), &S->format);
		c.r = AG_OPAQUE - c.r;
		c.g = AG_OPAQUE - c.g;
		c.b = AG_OPAQUE - c.b;
		AG_SurfacePut32_At(S, p, AG_MapPixel32(&S->format, &c));
		p += S->format.BytesPerPixel;
	}
	return (S);
}

const struct rg_transform_ops rgTransforms[] = {
	{ "mirror",	RG_TRANSFORM_MIRROR,     0, TransformMirror },
	{ "flip",	RG_TRANSFORM_FLIP,       0, TransformFlip },
	{ "rotate",	RG_TRANSFORM_ROTATE,     0, TransformRotate },
	{ "rgb-invert",	RG_TRANSFORM_RGB_INVERT, 0, TransformInvertRGB }
};
const int rgTransformsCount = sizeof(rgTransforms) / sizeof(rgTransforms[0]);
