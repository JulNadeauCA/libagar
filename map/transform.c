/*	$Csoft: transform.c,v 1.5 2005/07/16 15:55:34 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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
#include <core/view.h>

#include <stdarg.h>
#include <string.h>

const struct map_transform_ent mapTransforms[];
const int mapTransformsCount;

#if 0
/*
 * Add a new rotate transformation or update an existing one. If angle is 0,
 * any existing rotation is removed.
 */
MAP_Transform *
MAP_TransformRotate(struct map_item *r, int angle)
{
	Uint32 angles = (Uint32)angle;
	MAP_Transform *tr;
	SDL_Surface *su;
	double rad, theta;

	switch (r->type) {
	case MAP_ITEM_TILE:
		su = AG_SPRITE(r->r_sprite.obj,r->r_sprite.offs).su;
		break;
	case AG_NITEM_ANIM:
		su = AG_ANIM_FRAME(r, &AG_ANIM(r->r_anim.obj,r->r_anim.offs));
		break;
	default:
		return (NULL);
	}

	rad = hypot(
	    r->r_gfx.xorigin - su->w/2,
	    r->r_gfx.yorigin - su->h/2);
	theta = atan2(
	    r->r_gfx.yorigin - su->w/2,
	    r->r_gfx.xorigin - su->h/2);

	theta += ((float)angle/360.0)*(2.0*M_PI);
	r->r_gfx.xorigin = rad*cos(theta) + su->w/2;
	r->r_gfx.yorigin = rad*sin(theta) + su->h/2;

	TAILQ_FOREACH(tr, &r->transforms, transforms) {
		if (tr->type == MAP_TRANSFORM_ROTATE) {
			if (angle == 0) {
				TAILQ_REMOVE(&r->transforms, tr, transforms);
				Free(tr, M_NODEXFORM);
				return (NULL);
			}
			break;
		}
	}
	if (angle == 0) {
		return (NULL);
	}
	if (tr == NULL) {
		tr = MAP_TransformNew(MAP_TRANSFORM_ROTATE, 1, &angles);
		TAILQ_INSERT_TAIL(&r->transforms, tr, transforms);
	} else {
		tr->args[0] = angles;
	}
	return (tr);
}
#endif

MAP_Transform *
MAP_TransformNew(enum map_transform_type type, int nargs, Uint32 *args)
{
	MAP_Transform *trans;

	trans = Malloc(sizeof(MAP_Transform), M_NODEXFORM);
	if (MAP_TransformInit(trans, type, nargs, args) == -1) {
		Free(trans, M_NODEXFORM);
		return (NULL);
	}
	return (trans);
}

int
MAP_TransformInit(MAP_Transform *trans, enum map_transform_type type,
    int nargs, Uint32 *args)
{
	int i;

	if (nargs > MAP_TRANSFORM_MAX_ARGS) {
		AG_SetError(_("Too many transform args."));
		return (-1);
	}

	memset(trans, 0, sizeof(MAP_Transform));
	trans->type = type;
	trans->func = NULL;
	trans->nargs = nargs;
	if (nargs > 0) {
		trans->args = Malloc(nargs * sizeof(Uint32), M_NODEXFORM);
		memcpy(trans->args, args, nargs * sizeof(Uint32));
	} else {
		trans->args = NULL;
	}

	for (i = 0; i < mapTransformsCount; i++) {
		if (mapTransforms[i].type == type) {
			trans->func = mapTransforms[i].func;
			break;
		}
	}
	return (0);
}

void
MAP_TransformDestroy(MAP_Transform *trans)
{
	Free(trans->args, M_NODEXFORM);
	Free(trans, M_NODEXFORM);
}

int
MAP_TransformCompare(const MAP_Transform *tr1, const MAP_Transform *tr2)
{
	return (tr1->type == tr2->type &&
	        tr1->nargs == tr2->nargs &&
		(tr1->nargs == 0 ||
		 memcmp(tr1->args, tr2->args, tr1->nargs*sizeof(Uint32)) == 0));
}

int
MAP_TransformLoad(AG_Netbuf *buf, MAP_Transform *trans)
{
	int i;

	trans->type = AG_ReadUint8(buf);
	trans->func = NULL;
	trans->nargs = (int)AG_ReadUint8(buf);
	if (trans->nargs > MAP_TRANSFORM_MAX_ARGS) {
		AG_SetError(_("Too many transform args."));
		return (-1);
	}

	Free(trans->args, M_NODEXFORM);
	trans->args = Malloc(trans->nargs * sizeof(Uint32), M_NODEXFORM);
	for (i = 0; i < trans->nargs; i++)
		trans->args[i] = AG_ReadUint32(buf);

	/* Look for a matching algorithm. */
	for (i = 0; i < mapTransformsCount; i++) {
		if (mapTransforms[i].type == trans->type) {
			trans->func = mapTransforms[i].func;
			break;
		}
	}
	if (trans->func == NULL) {
		AG_SetError(_("Unknown transform algorithm."));
		return (-1);
	}
	return (0);
}

void
MAP_TransformSave(AG_Netbuf *buf, const MAP_Transform *trans)
{
	int i;

	AG_WriteUint8(buf, trans->type);
	AG_WriteUint8(buf, trans->nargs);

	for (i = 0; i < trans->nargs; i++)
		AG_WriteUint32(buf, trans->args[i]);
}

/* Flip a surface horizontally. */
static SDL_Surface *
mirror(SDL_Surface *su, int argc, Uint32 *argv)
{
	Uint8 *row, *rowp;
	Uint8 *fb = su->pixels;
	int x, y;

	row = Malloc(su->pitch, M_NODEXFORM);
	for (y = 0; y < su->h; y++) {
		memcpy(row, fb, su->pitch);
		rowp = row + su->pitch - su->format->BytesPerPixel;
		for (x = 0; x < su->w; x++) {
			AG_PUT_PIXEL(su, fb, AG_GET_PIXEL(su, rowp));
			fb += su->format->BytesPerPixel;
			rowp -= su->format->BytesPerPixel;
		}
	}
	Free(row, M_NODEXFORM);
	return (su);
}

/* Flip a surface vertically. */
static SDL_Surface *
flip(SDL_Surface *su, int argc, Uint32 *argv)
{
	size_t totsize = su->h*su->pitch;
	Uint8 *row, *rowbuf;
	Uint8 *fb = su->pixels;
	int y;

	rowbuf = Malloc(totsize, M_NODEXFORM);
	memcpy(rowbuf, fb, totsize);
	row = rowbuf + totsize - su->pitch;
	for (y = 0; y < su->h; y++) {
		memcpy(fb, row, su->pitch);
		row -= su->pitch;
		fb += su->pitch;
	}
	Free(rowbuf, M_NODEXFORM);
	return (su);
}

/* Rotate a surface by the given number of degrees. */
static SDL_Surface *
rotate(SDL_Surface *sOrig, int argc, Uint32 *argv)
{
	SDL_Surface *sNew;
	Uint32 theta = argv[0];
	int x, y;
	int xp, yp;
	int swapdims = (theta == 90 || theta == 270);

	sNew = SDL_CreateRGBSurface(SDL_SWSURFACE |
	    (sOrig->flags&(SDL_SRCALPHA|SDL_SRCCOLORKEY|SDL_RLEACCEL)),
	    swapdims ? sOrig->h : sOrig->w,
	    swapdims ? sOrig->w : sOrig->h,
	    sOrig->format->BitsPerPixel,
	    sOrig->format->Rmask,
	    sOrig->format->Gmask,
	    sOrig->format->Bmask,
	    sOrig->format->Amask);
	if (sNew == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());

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
	}
	return (sNew);
}

/* Print the transform chain. */
void
MAP_TransformPrint(const struct map_transformq *transq, char *buf,
    size_t buf_size)
{
	extern const struct map_transform_ent mapTransforms[];
	extern const int mapTransformsCount;
	MAP_Transform *tr;
	int i, j;

	TAILQ_FOREACH(tr, transq, transforms) {
		for (i = 0; i < mapTransformsCount; i++) {
			if (mapTransforms[i].type == tr->type)
				break;
		}
		if (i < mapTransformsCount) {
			strlcat(buf, "+", buf_size);
			strlcat(buf, mapTransforms[i].name, buf_size);
			for (j = 0; j < tr->nargs; j++) {
				char num[32];

				snprintf(num, sizeof(num), "(%lu)",
				    (unsigned long)tr->args[i]);
				strlcat(buf, num, buf_size);
			}
		}
	}
	if (!TAILQ_EMPTY(transq))
		strlcat(buf, "\n", buf_size);
}

/* Invert the colors of a surface. */
static SDL_Surface *
rgbinvert(SDL_Surface *su, int argc, Uint32 *argv)
{
	size_t size = su->w*su->h;
	Uint8 *p = su->pixels;
	Uint8 r, g, b, a;
	int i;

	for (i = 0; i < size; i++) {
		SDL_GetRGBA(AG_GET_PIXEL(su, p), su->format, &r, &g, &b, &a);
		AG_PUT_PIXEL(su, p, SDL_MapRGBA(su->format,
		    255 - r,
		    255 - g,
		    255 - b,
		    a));
		p += su->format->BytesPerPixel;
	}
	return (su);
}

const struct map_transform_ent mapTransforms[] = {
	{ "mirror",	MAP_TRANSFORM_MIRROR,		mirror },
	{ "flip",	MAP_TRANSFORM_FLIP,		flip },
	{ "rotate",	MAP_TRANSFORM_ROTATE,		rotate },
	{ "rgb-invert",	MAP_TRANSFORM_RGB_INVERT,	rgbinvert }
};
const int mapTransformsCount = sizeof(mapTransforms) / sizeof(mapTransforms[0]);

