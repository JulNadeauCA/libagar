/*	$Csoft: transform.c,v 1.2 2003/01/01 05:18:34 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include "engine.h"

#include <libfobj/fobj.h>

#include "map.h"
#include "view.h"

static SDL_Surface	*transform_scale(SDL_Surface *, struct transform *);
static SDL_Surface	*transform_hflip(SDL_Surface *, struct transform *);
static SDL_Surface	*transform_vflip(SDL_Surface *, struct transform *);
static SDL_Surface	*transform_rotate(SDL_Surface *, struct transform *);
static SDL_Surface	*transform_color(SDL_Surface *, struct transform *);
static SDL_Surface	*transform_pixelize(SDL_Surface *, struct transform *);
static SDL_Surface	*transform_randomize(SDL_Surface *, struct transform *);

static const struct {
	enum transform_type	type;
	SDL_Surface		*(*func)(SDL_Surface *, struct transform *);
} transforms[] = {
	{	TRANSFORM_SCALE,	transform_scale		},
	{	TRANSFORM_HFLIP,	transform_hflip		},
	{	TRANSFORM_VFLIP,	transform_vflip		},
	{	TRANSFORM_ROTATE,	transform_rotate	},
	{	TRANSFORM_COLOR,	transform_color		},
	{	TRANSFORM_PIXELIZE,	transform_pixelize	},
	{	TRANSFORM_RANDOMIZE,	transform_randomize	}
};
static const int ntransforms = sizeof(transforms) / sizeof(transforms[0]);

void
transform_init(struct transform *tr, enum transform_type type)
{
	int i;

	memset(tr, 0, sizeof(struct transform));
	tr->type = type;
	tr->cached = NULL;
	for (i = 0; i < ntransforms; i++) {
		if (transforms[i].type == type) {
			tr->func = transforms[i].func;
			return;
		}
	}
	fatal("no such transform: %d", i);
}

void
transform_destroy(struct transform *tr)
{
	/* No-op */
}

void
transform_copy(struct transform *src, struct transform *dst)
{
	memcpy(dst, src, sizeof(struct transform));
}

void
transform_load(int fd, struct transform *trans)
{
	int i, found = 0;

	/* Read the transform type. */
	trans->type = read_uint8(fd);

	/* Look for a matching function. */
	for (i = 0; i < ntransforms; i++) {
		if (transforms[i].type == trans->type) {
			trans->func = transforms[i].func;
			goto found;
		}
	}
	fatal("unknown transform: %d", trans->type);
found:
	/* Read the transform arguments. */
	switch (trans->type) {
	case TRANSFORM_SCALE:
		trans->args.scale.w = read_uint16(fd);
		trans->args.scale.h = read_uint16(fd);
		break;
	case TRANSFORM_HFLIP:
	case TRANSFORM_VFLIP:
		break;
	case TRANSFORM_ROTATE:
		trans->args.rotate.angle = read_uint16(fd);
		break;
	case TRANSFORM_COLOR:
		trans->args.color.r = read_uint8(fd);
		trans->args.color.g = read_uint8(fd);
		trans->args.color.b = read_uint8(fd);
		trans->args.color.a = read_uint8(fd);
		break;
	case TRANSFORM_PIXELIZE:
		trans->args.pixelize.factor = read_uint16(fd);
		break;
	case TRANSFORM_RANDOMIZE:
		trans->args.randomize.nrounds = read_uint8(fd);
		trans->args.randomize.r_range = read_uint8(fd);
		trans->args.randomize.g_range = read_uint8(fd);
		trans->args.randomize.b_range = read_uint8(fd);
		trans->args.randomize.a_range = read_uint8(fd);
		break;
	default:
		fatal("unknown transform type: %d", trans->type);
	}
}

void
transform_save(void *bufp, struct transform *trans)
{
	struct fobj_buf *buf = bufp;
	Uint8 i;

	/* Save the transform type. */
	buf_write_uint8(buf, trans->type);

	/* Save the transform arguments. */
	switch (trans->type) {
	case TRANSFORM_SCALE:
		buf_write_uint16(buf, trans->args.scale.w);
		buf_write_uint16(buf, trans->args.scale.h);
		break;
	case TRANSFORM_HFLIP:
	case TRANSFORM_VFLIP:
		break;
	case TRANSFORM_ROTATE:
		buf_write_uint16(buf, trans->args.rotate.angle);
		break;
	case TRANSFORM_COLOR:
		buf_write_uint8(buf, trans->args.color.r);
		buf_write_uint8(buf, trans->args.color.g);
		buf_write_uint8(buf, trans->args.color.b);
		buf_write_uint8(buf, trans->args.color.a);
		break;
	case TRANSFORM_PIXELIZE:
		buf_write_uint16(buf, trans->args.pixelize.factor);
		break;
	case TRANSFORM_RANDOMIZE:
		buf_write_uint8(buf, trans->args.randomize.nrounds);
		buf_write_uint8(buf, trans->args.randomize.r_range);
		buf_write_uint8(buf, trans->args.randomize.g_range);
		buf_write_uint8(buf, trans->args.randomize.b_range);
		buf_write_uint8(buf, trans->args.randomize.a_range);
		break;
	}
}

static SDL_Surface *
transform_scale(SDL_Surface *su, struct transform *tr)
{
	/* ... */
	return (SDL_ConvertSurface(su, view->v->format,
	    SDL_SWSURFACE|SDL_SRCALPHA));
}

static SDL_Surface *
transform_hflip(SDL_Surface *su, struct transform *tr)
{
	/* ... */
	return (SDL_ConvertSurface(su, view->v->format,
	    SDL_SWSURFACE|SDL_SRCALPHA));
}

static SDL_Surface *
transform_vflip(SDL_Surface *su, struct transform *tr)
{
	/* ... */
	return (SDL_ConvertSurface(su, view->v->format,
	    SDL_SWSURFACE|SDL_SRCALPHA));
}

static SDL_Surface *
transform_rotate(SDL_Surface *su, struct transform *tr)
{
	/* ... */
	return (SDL_ConvertSurface(su, view->v->format,
	    SDL_SWSURFACE|SDL_SRCALPHA));
}

static SDL_Surface *
transform_color(SDL_Surface *su, struct transform *tr)
{
	/* ... */
	return (SDL_ConvertSurface(su, view->v->format,
	    SDL_SWSURFACE|SDL_SRCALPHA));
}

static SDL_Surface *
transform_pixelize(SDL_Surface *su, struct transform *tr)
{
	/* ... */
	return (SDL_ConvertSurface(su, view->v->format,
	    SDL_SWSURFACE|SDL_SRCALPHA));
}

static SDL_Surface *
transform_randomize(SDL_Surface *su, struct transform *tr)
{
	/* ... */
	return (SDL_ConvertSurface(su, view->v->format,
	    SDL_SWSURFACE|SDL_SRCALPHA));
}
