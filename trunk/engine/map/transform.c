/*	$Csoft: transform.c,v 1.4 2005/06/08 06:28:32 vedge Exp $	*/

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

#include <engine/engine.h>

#ifdef MAP

#include <engine/view.h>

#include "map.h"

#include <string.h>
#include <stdarg.h>

const struct transform_ent transforms[];
const int ntransforms;

/*
 * Add a new rotate transformation or update an existing one. If angle is 0,
 * any existing rotation is removed.
 */
struct transform *
transform_rotate(struct noderef *r, int angle)
{
	Uint32 angles = (Uint32)angle;
	struct transform *tr;
	SDL_Surface *su;
	float rad, theta;

	switch (r->type) {
	case NODEREF_SPRITE:
		su = SPRITE(r->r_sprite.obj,r->r_sprite.offs).su;
		break;
	case NODEREF_ANIM:
		su = GFX_ANIM_FRAME(r, &ANIM(r->r_anim.obj,r->r_anim.offs));
		break;
	default:
		return (NULL);
	}

	rad = hypotf(
	    r->r_gfx.xorigin - su->w/2,
	    r->r_gfx.yorigin - su->h/2);
	theta = atan2f(
	    r->r_gfx.yorigin - su->w/2,
	    r->r_gfx.xorigin - su->h/2);

	theta += ((float)angle/360.0)*(2.0*M_PI);
	r->r_gfx.xorigin = rad*cosf(theta) + su->w/2;
	r->r_gfx.yorigin = rad*sinf(theta) + su->h/2;

	TAILQ_FOREACH(tr, &r->transforms, transforms) {
		if (tr->type == TRANSFORM_ROTATE) {
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
		tr = transform_new(TRANSFORM_ROTATE, 1, &angles);
		TAILQ_INSERT_TAIL(&r->transforms, tr, transforms);
	} else {
		tr->args[0] = angles;
	}
	return (tr);
}

struct transform *
transform_new(enum transform_type type, int nargs, Uint32 *args)
{
	struct transform *trans;

	trans = Malloc(sizeof(struct transform), M_NODEXFORM);
	if (transform_init(trans, type, nargs, args) == -1) {
		Free(trans, M_NODEXFORM);
		return (NULL);
	}
	return (trans);
}

int
transform_init(struct transform *trans, enum transform_type type,
    int nargs, Uint32 *args)
{
	int i;

	if (nargs > TRANSFORM_MAX_ARGS) {
		error_set(_("Too many transform args."));
		return (-1);
	}

	memset(trans, 0, sizeof(struct transform));
	trans->type = type;
	trans->func = NULL;
	trans->nargs = nargs;
	if (nargs > 0) {
		trans->args = Malloc(nargs * sizeof(Uint32), M_NODEXFORM);
		memcpy(trans->args, args, nargs * sizeof(Uint32));
	} else {
		trans->args = NULL;
	}

	for (i = 0; i < ntransforms; i++) {
		if (transforms[i].type == type) {
			trans->func = transforms[i].func;
			break;
		}
	}
	return (0);
}

void
transform_destroy(struct transform *trans)
{
	Free(trans->args, M_NODEXFORM);
	Free(trans, M_NODEXFORM);
}

int
transform_compare(const struct transform *tr1, const struct transform *tr2)
{
	return (tr1->type == tr2->type &&
	        tr1->nargs == tr2->nargs &&
		(tr1->nargs == 0 ||
		 memcmp(tr1->args, tr2->args, tr1->nargs*sizeof(Uint32)) == 0));
}

int
transform_load(struct netbuf *buf, struct transform *trans)
{
	int i;

	trans->type = read_uint8(buf);
	trans->func = NULL;
	trans->nargs = (int)read_uint8(buf);
	if (trans->nargs > TRANSFORM_MAX_ARGS) {
		error_set(_("Too many transform args."));
		return (-1);
	}

	Free(trans->args, M_NODEXFORM);
	trans->args = Malloc(trans->nargs * sizeof(Uint32), M_NODEXFORM);
	for (i = 0; i < trans->nargs; i++)
		trans->args[i] = read_uint32(buf);

	/* Look for a matching algorithm. */
	for (i = 0; i < ntransforms; i++) {
		if (transforms[i].type == trans->type) {
			trans->func = transforms[i].func;
			break;
		}
	}
	if (trans->func == NULL) {
		error_set(_("Unknown transform algorithm."));
		return (-1);
	}
	return (0);
}

void
transform_save(struct netbuf *buf, const struct transform *trans)
{
	int i;

	write_uint8(buf, trans->type);
	write_uint8(buf, trans->nargs);

	for (i = 0; i < trans->nargs; i++)
		write_uint32(buf, trans->args[i]);
}

/* Flip a surface horizontally. */
static SDL_Surface *
hflip(SDL_Surface *su, int argc, Uint32 *argv)
{
	Uint8 *row, *rowp;
	Uint8 *fb = su->pixels;
	int x, y;

	row = Malloc(su->pitch, M_NODEXFORM);
	for (y = 0; y < su->h; y++) {
		memcpy(row, fb, su->pitch);
		rowp = row + su->pitch - su->format->BytesPerPixel;
		for (x = 0; x < su->w; x++) {
			PUT_PIXEL(su, fb, GET_PIXEL(su, rowp));
			fb += su->format->BytesPerPixel;
			rowp -= su->format->BytesPerPixel;
		}
	}
	Free(row, M_NODEXFORM);
	return (su);
}

/* Flip a surface vertically. */
static SDL_Surface *
vflip(SDL_Surface *su, int argc, Uint32 *argv)
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
				PUT_PIXEL2(sNew, y, x,
				    GET_PIXEL2(sOrig, x, sOrig->h-y-1));
			}
		}
		break;
	case 180:
		for (y = 0, yp = sOrig->h-1; y < sOrig->h; y++, yp--) {
			for (x = 0, xp = sOrig->w-1; x < sOrig->w; x++, xp--) {
				PUT_PIXEL2(sNew, x, y,
				    GET_PIXEL2(sOrig, xp, yp));
			}
		}
		break;
	case 270:
		for (y = 0; y < sOrig->h; y++) {
			for (x = 0; x < sOrig->w; x++) {
				PUT_PIXEL2(sNew, y, x,
				    GET_PIXEL2(sOrig, sOrig->w-x-1, y));
			}
		}
		break;
	}
	return (sNew);
}

/* Print the transform chain. */
void
transform_print(const struct transformq *transq, char *buf, size_t buf_size)
{
	extern const struct transform_ent transforms[];
	extern const int ntransforms;
	struct transform *tr;
	int i, j;

	TAILQ_FOREACH(tr, transq, transforms) {
		for (i = 0; i < ntransforms; i++) {
			if (transforms[i].type == tr->type)
				break;
		}
		if (i < ntransforms) {
			strlcat(buf, "+", buf_size);
			strlcat(buf, transforms[i].name, buf_size);
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
invert(SDL_Surface *su, int argc, Uint32 *argv)
{
	size_t size = su->w*su->h;
	Uint8 *p = su->pixels;
	Uint8 r, g, b, a;
	int i;

	for (i = 0; i < size; i++) {
		SDL_GetRGBA(GET_PIXEL(su, p), su->format, &r, &g, &b, &a);
		PUT_PIXEL(su, p, SDL_MapRGBA(su->format,
		    255 - r,
		    255 - g,
		    255 - b,
		    a));
		p += su->format->BytesPerPixel;
	}
	return (su);
}

const struct transform_ent transforms[] = {
	{ "h-flip",	TRANSFORM_HFLIP,	hflip },
	{ "v-flip",	TRANSFORM_VFLIP,	vflip },
	{ "rotate",	TRANSFORM_ROTATE,	rotate },
	{ "invert",	TRANSFORM_INVERT,	invert }
};
const int ntransforms = sizeof(transforms) / sizeof(transforms[0]);

#endif /* MAP */
