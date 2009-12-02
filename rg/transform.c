/*
 * Copyright (c) 2002-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <gui/geometry.h>
#include <gui/surface.h>
#include <gui/widget.h>

#include "tileset.h"

#include <stdarg.h>
#include <string.h>

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
	case MAP_ITEM_ANIM:
		su = AG_ANIM_FRAME(r, &AG_ANIM(r->r_anim.obj,r->r_anim.offs));
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
RG_TransformNew(enum rg_transform_type type, int nargs, Uint32 *args)
{
	RG_Transform *xf;

	xf = Malloc(sizeof(RG_Transform));
	if (RG_TransformInit(xf, type, nargs, args) == -1) {
		Free(xf);
		return (NULL);
	}
	return (xf);
}

int
RG_TransformInit(RG_Transform *xf, enum rg_transform_type type, int nargs,
    Uint32 *args)
{
	int i;

	if (nargs > RG_TRANSFORM_MAX_ARGS) {
		AG_SetError("Too many transform args");
		return (-1);
	}

	memset(xf, 0, sizeof(RG_Transform));
	xf->type = type;
	xf->func = NULL;
	xf->nargs = nargs;
	if (nargs > 0) {
		xf->args = Malloc(nargs*sizeof(Uint32));
		memcpy(xf->args, args, nargs * sizeof(Uint32));
	} else {
		xf->args = NULL;
	}

	for (i = 0; i < rgTransformsCount; i++) {
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
    size_t buf_size)
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
			for (j = 0; j < tr->nargs; j++) {
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
		RG_TransformInit(xf_dup, xf->type, xf->nargs, xf->args);
		TAILQ_INSERT_TAIL(xc_dup, xf_dup, transforms);
	}
}

int
RG_TransformCompare(const RG_Transform *xf1, const RG_Transform *xf2)
{
	return (xf1->type == xf2->type &&
	        xf1->nargs == xf2->nargs &&
		(xf1->nargs == 0 ||
		 memcmp(xf1->args, xf2->args, xf1->nargs*sizeof(Uint32)) == 0));
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
	xf->nargs = (int)AG_ReadUint8(buf);
	if (xf->nargs > RG_TRANSFORM_MAX_ARGS) {
		AG_SetError("Too many transform args: %u", (Uint)xf->nargs);
		return (-1);
	}

	Free(xf->args);
	xf->args = Malloc(xf->nargs * sizeof(Uint32));
	for (i = 0; i < xf->nargs; i++)
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
	AG_WriteUint8(buf, xf->nargs);

	for (i = 0; i < xf->nargs; i++)
		AG_WriteUint32(buf, xf->args[i]);
}

/* Flip a surface horizontally. */
static AG_Surface *
TransformMirror(AG_Surface *su, int argc, Uint32 *argv)
{
	Uint8 *row, *rowp;
	Uint8 *fb = su->pixels;
	int x, y;

	row = Malloc(su->pitch);
	for (y = 0; y < su->h; y++) {
		memcpy(row, fb, su->pitch);
		rowp = row + su->pitch - su->format->BytesPerPixel;
		for (x = 0; x < su->w; x++) {
			AG_PUT_PIXEL(su, fb, AG_GET_PIXEL(su, rowp));
			fb += su->format->BytesPerPixel;
			rowp -= su->format->BytesPerPixel;
		}
	}
	Free(row);
	return (su);
}

/* Flip a surface vertically. */
static AG_Surface *
TransformFlip(AG_Surface *su, int argc, Uint32 *argv)
{
	size_t totsize = su->h*su->pitch;
	Uint8 *row, *rowbuf;
	Uint8 *fb = su->pixels;
	int y;

	rowbuf = Malloc(totsize);
	memcpy(rowbuf, fb, totsize);
	row = rowbuf + totsize - su->pitch;
	for (y = 0; y < su->h; y++) {
		memcpy(fb, row, su->pitch);
		row -= su->pitch;
		fb += su->pitch;
	}
	Free(rowbuf);
	return (su);
}

/* Rotate a surface by the given number of degrees. */
static AG_Surface *
TransformRotate(AG_Surface *sOrig, int argc, Uint32 *argv)
{
	AG_Surface *sNew;
	Uint32 theta = argv[0];
	int x, y;
	int xp, yp;
	int swapdims = (theta == 90 || theta == 270);

	sNew = AG_SurfaceRGBA(
	    swapdims ? sOrig->h : sOrig->w,
	    swapdims ? sOrig->w : sOrig->h,
	    sOrig->format->BitsPerPixel,
	    (sOrig->flags & (AG_SRCALPHA|AG_SRCCOLORKEY)),
	    sOrig->format->Rmask,
	    sOrig->format->Gmask,
	    sOrig->format->Bmask,
	    sOrig->format->Amask);
	if (sNew == NULL) {
		AG_FatalError(NULL);
	}
	switch (theta) {
	case 90:
		for (y = 0; y < sOrig->h; y++) {
			for (x = 0; x < sOrig->w; x++) {
				AG_PUT_PIXEL2(sNew, y, x,
				    AG_GET_PIXEL2(sOrig, x, sOrig->h-y-1));
			}
		}
		break;
	case 180:
		for (y = 0, yp = sOrig->h-1; y < sOrig->h; y++, yp--) {
			for (x = 0, xp = sOrig->w-1; x < sOrig->w; x++, xp--) {
				AG_PUT_PIXEL2(sNew, x, y,
				    AG_GET_PIXEL2(sOrig, xp, yp));
			}
		}
		break;
	case 270:
		for (y = 0; y < sOrig->h; y++) {
			for (x = 0; x < sOrig->w; x++) {
				AG_PUT_PIXEL2(sNew, y, x,
				    AG_GET_PIXEL2(sOrig, sOrig->w-x-1, y));
			}
		}
		break;
	default:
		/* TODO */
		break;
	}
	return (sNew);
}

/* Invert the colors of a surface. */
static AG_Surface *
TransformInvertRGB(AG_Surface *su, int argc, Uint32 *argv)
{
	size_t size = su->w*su->h;
	Uint8 *p = su->pixels;
	AG_Color C;
	int i;

	for (i = 0; i < size; i++) {
		C = AG_GetColorRGBA(AG_GET_PIXEL(su,p), su->format);
		C.r = 255-C.r;
		C.g = 255-C.g;
		C.b = 255-C.b;
		AG_PUT_PIXEL(su, p, AG_MapColorRGBA(su->format, C));
		p += su->format->BytesPerPixel;
	}
	return (su);
}

const struct rg_transform_ops rgTransforms[] = {
	{ "mirror",	RG_TRANSFORM_MIRROR,		TransformMirror },
	{ "flip",	RG_TRANSFORM_FLIP,		TransformFlip },
	{ "rotate",	RG_TRANSFORM_ROTATE,		TransformRotate },
	{ "rgb-invert",	RG_TRANSFORM_RGB_INVERT,	TransformInvertRGB }
};
const int rgTransformsCount = sizeof(rgTransforms) / sizeof(rgTransforms[0]);

