/*	$Csoft: transform.c,v 1.4 2003/03/13 08:41:08 vedge Exp $	*/

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

static SDL_Surface	*transform_hflip(SDL_Surface *, int, Uint32 *);
static SDL_Surface	*transform_vflip(SDL_Surface *, int, Uint32 *);

static const struct {
	enum transform_type	type;
	SDL_Surface		*(*func)(SDL_Surface *, int, Uint32 *);
} transforms[] = {
	{	TRANSFORM_HFLIP,	transform_hflip		},
	{	TRANSFORM_VFLIP,	transform_vflip		}
};
static const int ntransforms = sizeof(transforms) / sizeof(transforms[0]);

enum {
	TRANSFORM_MAX_ARGS =	64
};

int
transform_init(struct transform *trans, enum transform_type type,
    int argc, Uint32 *argv)
{
	int i;

	dprintf("type %d, %d args\n", type, argc);

	if (argc > TRANSFORM_MAX_ARGS) {
		error_set("too many args");
		return (-1);
	}

	memset(trans, 0, sizeof(struct transform));
	trans->type = type;
	trans->func = NULL;
	trans->nargs = argc;
	trans->args = emalloc(argc * sizeof(Uint32));
	memcpy(trans->args, argv, argc * sizeof(Uint32));

	/* Look for a matching algorithm. */
	for (i = 0; i < ntransforms; i++) {
		if (transforms[i].type == type) {
			trans->func = transforms[i].func;
			break;
		}
	}
	if (trans->func == NULL) {
		error_set("bad transform");
		return (-1);
	}
	return (0);
}

void
transform_destroy(struct transform *trans)
{
	Free(trans->args);
}

void
transform_copy(struct transform *src, struct transform *dst)
{
	dst->type = src->type;
	dst->func = src->func;

	Free(dst->args);
	dst->args = emalloc(src->nargs * sizeof(Uint32));
	memcpy(dst->args, src->args, src->nargs * sizeof(Uint32));
}

int
transform_load(int fd, struct transform *trans)
{
	int i;

	trans->type = read_uint8(fd);
	trans->func = NULL;
	trans->nargs = (int)read_uint8(fd);
	if (trans->nargs > TRANSFORM_MAX_ARGS) {
		error_set("too many args");
		return (-1);
	}

	Free(trans->args);
	trans->args = emalloc(trans->nargs * sizeof(Uint32));
	for (i = 0; i < trans->nargs; i++)
		trans->args[i] = read_uint32(fd);

	/* Look for a matching algorithm. */
	for (i = 0; i < ntransforms; i++) {
		if (transforms[i].type == trans->type) {
			trans->func = transforms[i].func;
			break;
		}
	}
	if (trans->func == NULL) {
		error_set("bad transform");
		return (-1);
	}
	return (0);
}

void
transform_save(void *bufp, struct transform *trans)
{
	struct fobj_buf *buf = bufp;
	int i;

	buf_write_uint8(buf, trans->type);
	buf_write_uint8(buf, trans->nargs);

	for (i = 0; i < trans->nargs; i++)
		buf_write_uint32(buf, trans->args[i]);
}

static SDL_Surface *
transform_hflip(SDL_Surface *su, int argc, Uint32 *argv)
{
	/* ... */
	return (SDL_ConvertSurface(su, view->v->format,
	    SDL_SWSURFACE|SDL_SRCALPHA));
}

static SDL_Surface *
transform_vflip(SDL_Surface *su, int argc, Uint32 *argv)
{
	/* ... */
	return (SDL_ConvertSurface(su, view->v->format,
	    SDL_SWSURFACE|SDL_SRCALPHA));
}

