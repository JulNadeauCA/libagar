/*	$Csoft: fill.c,v 1.10 2004/10/02 07:54:48 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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
#include <engine/view.h>

#include <engine/vg/vg.h>

#ifdef EDITION
#include <engine/widget/window.h>
#include <engine/widget/palette.h>
#endif

#include "tileset.h"
#include "fill.h"

const struct version fill_ver = {
	"agar rg fill feature",
	0, 0
};

const struct feature_ops fill_ops = {
	"fill",
	sizeof(struct fill),
	N_("Fill tile with solid color/pattern."),
	fill_init,
	fill_load,
	fill_save,
	NULL,		/* destroy */
	NULL,		/* apply */
	fill_edit
};

void
fill_init(void *p, const char *name, int flags)
{
	struct fill *f = p;

	feature_init(f, name, flags, &fill_ops);
	f->type = FILL_SOLID;
	f->f_solid.c = SDL_MapRGB(vfmt, 0, 0, 0);
}

int
fill_load(void *p, struct netbuf *buf)
{
	struct fill *f = p;

	if (version_read(buf, &fill_ver, NULL) == -1)
		return (-1);

	f->type = (enum fill_type)read_uint8(buf);
	switch (f->type) {
	case FILL_SOLID:
		f->f_solid.c = read_color(buf, vfmt);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
		f->f_gradient.c1 = read_color(buf, vfmt);
		f->f_gradient.c2 = read_color(buf, vfmt);
		break;
	case FILL_PATTERN:
		f->f_pattern.texid = (int)read_uint32(buf);
		f->f_pattern.tex_xoffs = (int)read_uint32(buf);
		f->f_pattern.tex_yoffs = (int)read_uint32(buf);
		break;
	}
	return (0);
}

void
fill_save(void *p, struct netbuf *buf)
{
	struct fill *f = p;

	version_write(buf, &fill_ver);

	write_uint8(buf, (Uint8)f->type);
	switch (f->type) {
	case FILL_SOLID:
		write_color(buf, vfmt, f->f_solid.c);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
		write_color(buf, vfmt, f->f_gradient.c1);
		write_color(buf, vfmt, f->f_gradient.c2);
		break;
	case FILL_PATTERN:
		write_uint32(buf, (Uint32)f->f_pattern.texid);
		write_uint32(buf, (Uint32)f->f_pattern.tex_xoffs);
		write_uint32(buf, (Uint32)f->f_pattern.tex_yoffs);
		break;
	}
}

struct window *
fill_edit(void *p)
{
	struct fill *f = p;
	struct window *win;
	struct palette *pal;

	win = window_new(0, NULL);
	pal = palette_new(win, PALETTE_RGBA);
	widget_bind(pal, "color", WIDGET_UINT32, &f->f_solid.c);

	return (win);
}
