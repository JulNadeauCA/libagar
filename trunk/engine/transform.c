/*	$Csoft: transform.c,v 1.12 2003/08/29 05:06:48 vedge Exp $	*/

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

#include <engine/engine.h>

#include <engine/map.h>
#include <engine/view.h>

#include <string.h>

static void	transform_hflip(SDL_Surface **, int, Uint32 *);
static void	transform_vflip(SDL_Surface **, int, Uint32 *);

const struct transform_ent transforms[] = {
	{ "h-flip",	TRANSFORM_HFLIP,	transform_hflip },
	{ "v-flip",	TRANSFORM_VFLIP,	transform_vflip }
};
const int ntransforms = sizeof(transforms) / sizeof(transforms[0]);

struct transform *
transform_new(enum transform_type type, int nargs, Uint32 *args)
{
	struct transform *trans;

	trans = Malloc(sizeof(struct transform));
	if (transform_init(trans, type, nargs, args) == -1) {
		free(trans);
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

	dprintf("type %d, %d args\n", type, nargs);

	memset(trans, 0, sizeof(struct transform));
	trans->type = type;
	trans->func = NULL;
	trans->nargs = nargs;
	if (nargs > 0) {
		trans->args = Malloc(nargs * sizeof(Uint32));
		memcpy(trans->args, args, nargs * sizeof(Uint32));
	} else {
		trans->args = NULL;
	}

	/* Look for a matching algorithm. */
	for (i = 0; i < ntransforms; i++) {
		if (transforms[i].type == type) {
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
transform_destroy(struct transform *trans)
{
	Free(trans->args);
	free(trans);
}

int
transform_compare(const struct transform *tr1, const struct transform *tr2)
{
	return (tr1->type == tr2->type &&
	    tr1->nargs == tr2->nargs &&
	    (tr1->nargs == 0 ||
	     memcmp(tr1->args, tr2->args, tr1->nargs*sizeof(Uint32))) == 0);
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

	Free(trans->args);
	trans->args = Malloc(trans->nargs * sizeof(Uint32));
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
static void
transform_hflip(SDL_Surface **sup, int argc, Uint32 *argv)
{
	SDL_Surface *su = *sup;
	Uint8 *row, *rowp;
	Uint8 *fb = su->pixels;
	int x, y;

	row = Malloc(su->pitch);

	for (y = 0; y < su->h; y++) {
		memcpy(row, fb, su->pitch);
		rowp = row + su->pitch - su->format->BytesPerPixel;
		for (x = 0; x < su->w; x++) {
			switch (su->format->BytesPerPixel) {
			case 4:
				*(Uint32 *)fb = *(Uint32 *)rowp;
				break;
			case 3:
			case 2:
				*(Uint16 *)fb = *(Uint16 *)rowp;
				break;
			case 1:
				*fb = *rowp;
				break;
			}
			fb += su->format->BytesPerPixel;
			rowp -= su->format->BytesPerPixel;
		}
	}
	free(row);
}

/* Flip a surface vertically. */
static void
transform_vflip(SDL_Surface **sup, int argc, Uint32 *argv)
{
	SDL_Surface *su = *sup;
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
	free(rowbuf);
}

