/*	$Csoft: vg_text.c,v 1.4 2004/05/05 16:45:35 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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
#ifdef EDITION
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/mapedit/mapview.h>
#include <engine/mapedit/tool.h>
#endif

#include "vg.h"
#include "vg_text.h"

#include <stdarg.h>

const struct vg_element_ops vg_text_ops = {
	N_("Text"),
	vg_text_init,
	vg_text_destroy,
	vg_draw_text,
	NULL				/* bbox */
};

void
vg_text_init(struct vg *vg, struct vg_element *vge)
{
	vge->vg_text.text[0] = '\0';
	vge->vg_text.angle = 0;
	vge->vg_text.align = VG_ALIGN_MC;
	vge->vg_text.style[0] = '\0';
	vge->vg_text.face[0] = '\0';
	vge->vg_text.flags = 0;
	vge->vg_text.su = NULL;

}

void
vg_text_destroy(struct vg *vg, struct vg_element *vge)
{
	if (vge->vg_text.su != NULL)
		SDL_FreeSurface(vge->vg_text.su);
}

/* Specify the text alignment around the central vertex. */
void
vg_text_align(struct vg *vg, enum vg_alignment align)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_text.align = align;
}

/* Specify the angle relative to the central vertex. */
void
vg_text_angle(struct vg *vg, double angle)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	vge->vg_text.angle = angle;
}

/* Specify a drawing-specific font style. */
int
vg_text_style(struct vg *vg, const char *name)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);
	struct vg_text_style *ts;

	TAILQ_FOREACH(ts, &vg->txtstyles, txtstyles) {
		if (strcmp(ts->name, name) == 0)
			break;
	}
	if (ts == NULL) {
		return (-1);
	}
	strlcpy(vge->vg_text.face, ts->face, sizeof(vge->vg_text.face));
	vge->vg_text.size = ts->size;
	vge->vg_text.flags = ts->flags;
	vge->color = ts->color;
	return (0);
}

/* Specify the font face, size and style. */
int
vg_text_font(struct vg *vg, const char *face, int size, int flags)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);

	if (size < VG_FONT_SIZE_MIN || size > VG_FONT_SIZE_MAX) {
		error_set(_("Illegal font size."));
		return (-1);
	}
	strlcpy(vge->vg_text.face, face, sizeof(vge->vg_text.face));
	vge->vg_text.size = size;
	vge->vg_text.flags = flags;
	return (0);
}

/* Specify the text to display. */
void
vg_printf(struct vg *vg, const char *fmt, ...)
{
	struct vg_element *vge = TAILQ_FIRST(&vg->vges);
	va_list args;

	if (fmt != NULL) {
		va_start(args, fmt);
		vsnprintf(vge->vg_text.text, sizeof(vge->vg_text.text), fmt,
		    args);
		va_end(args);
	} else {
		vge->vg_text.text[0] = '\0';
	}

	if (vge->vg_text.su != NULL) {
		SDL_FillRect(vge->vg_text.su, NULL,
		    SDL_MapRGBA(vge->vg_text.su->format, 0, 0, 0,
		    SDL_ALPHA_TRANSPARENT));
	}
}

void
vg_draw_text(struct vg *vg, struct vg_element *vge)
{
	SDL_Rect rd;
	int x, y;

#ifdef DEBUG
	if (vge->nvtx < 1)
		fatal("nvtx < 1");
#endif
	vg_rcoords2(vg, vge->vtx[0].x, vge->vtx[0].y, &x, &y);
	rd.x = (Sint16)x;
	rd.y = (Sint16)y;

	/* XXX align ... */

	if (vge->vg_text.su == NULL) {
		vge->vg_text.su = text_render(NULL, -1, vge->color,
		    vge->vg_text.text);
	}
	SDL_BlitSurface(vge->vg_text.su, NULL, vg->su, &rd);
}

void
vg_text_bbox(struct vg *vg, struct vg_element *vge, struct vg_rect *r)
{
	double vx, vy;
	SDL_Surface *su = vge->vg_text.su;

	if (su == NULL) {
		r->x = 0;
		r->y = 0;
		r->w = 0;
		r->h = 0;
		return;
	}
}

#ifdef EDITION
static struct vg_element *cur_text;
static struct vg_vertex *cur_vtx;

static void
text_tool_init(struct tool *t)
{
	struct window *win;
	struct ucombo *ucom;
	struct textbox *tb;

	tool_push_status(t, _("Specify the text's center point.\n"));
	cur_text = NULL;
	cur_vtx = NULL;
}

static void
text_mousemotion(struct tool *t, int tx, int ty, int txrel, int tyrel,
    int txoff, int tyoff, int txorel, int tyorel, int b)
{
	struct vg *vg = t->p;
	double x, y;
	
	if ((b & SDL_BUTTON(1)) == 0)
	
	vg_vcoords2(vg, tx, ty, txoff, tyoff, &x, &y);
	if (cur_vtx != NULL) {
		cur_vtx->x = x;
		cur_vtx->y = y;
	} else {
		vg->origin[2].x = x;
		vg->origin[2].y = y;
	}
	if (cur_text != NULL) {
		cur_text->redraw++;
		vg->redraw++;
	}
	vg->origin[1].x = x;
	vg->origin[1].y = y;
}

static void
text_mousebuttondown(struct tool *t, int tx, int ty, int txoff, int tyoff,
    int b)
{
	struct vg *vg = t->p;
	double vx, vy;

	switch (b) {
	case 1:
		cur_text = vg_begin_element(vg, VG_TEXT);
		vg_printf(vg, "Foo");
		vg_vcoords2(vg, tx, ty, txoff, tyoff, &vx, &vy);
		cur_vtx = vg_vertex2(vg, vx, vy);
		tool_push_status(t, _("Type the text or [undo element].\n"));
		break;
	default:
		if (cur_text != NULL) {
			vg_destroy_element(vg, cur_text);
		}
		goto finish;
	}
	return;
finish:
	cur_text = NULL;
	cur_vtx = NULL;
	tool_pop_status(t);
}

const struct tool text_tool = {
	N_("Text"),
	N_("Insert text labels."),
	VGTEXT_ICON,
	-1,
	text_tool_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	NULL,			/* cursor */
	NULL,			/* effect */
	text_mousemotion,
	text_mousebuttondown,
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL			/* keyup */
};
#endif /* EDITION */
