/*	$Csoft: sketch.c,v 1.12 2005/05/26 06:46:47 vedge Exp $	*/

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
#include <engine/input.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/hsvpal.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/notebook.h>
#include <engine/widget/units.h>

#include "tileset.h"
#include "tileview.h"

void
sketch_init(struct sketch *sk, struct tileset *ts, int flags)
{
	sk->name[0] = '\0';
	sk->flags = flags;
	sk->ts = ts;
	sk->h = 0.0;
	sk->s = 0.0;
	sk->v = 0.0;
	sk->a = 1.0;

	sk->vg = Malloc(sizeof(struct vg), M_VG);
	vg_init(sk->vg, VG_ANTIALIAS|VG_ALPHA);

	sk->ublks = Malloc(sizeof(struct sketch_undoblk), M_RG);
	sk->nublks = 1;
	sk->curblk = 0;
	sketch_begin_undoblk(sk);
	sk->ublks[0].mods = Malloc(sizeof(struct sketch_mod), M_RG);
	sk->ublks[0].nmods = 0;
}

void
sketch_scale(struct sketch *sk, int w, int h, float scale, int x, int y)
{
	struct vg *vg = sk->vg;
	struct vg_element *vge;
	double xoffs = (float)x/(float)TILESZ/scale;
	double yoffs = (float)y/(float)TILESZ/scale;
	Uint32 i;
	double vw, vh;

	if (w == -1) {
		w = vg->w;
	} else {
		w = (float)w/(float)TILESZ/scale;
	}
	if (h == -1) {
		h = vg->h;
	} else {
		h = (float)h/(float)TILESZ/scale;
	}

	vg_scale(vg, w, h, scale);
	
	TAILQ_FOREACH(vge, &vg->vges, vges) {
		for (i = 0; i < vge->nvtx; i++) {
			vge->vtx[i].x += xoffs;
			vge->vtx[i].y += yoffs;
		}
	}
	vg->redraw++;
}

void
sketch_destroy(struct sketch *sk)
{
	int i;

	vg_destroy(sk->vg);

	for (i = 0; i < sk->nublks; i++) {
		Free(sk->ublks[i].mods, M_RG);
	}
	Free(sk->ublks, M_RG);
}

int
sketch_load(struct sketch *sk, struct netbuf *buf)
{
	int vgflags;
	
	copy_string(sk->name, buf, sizeof(sk->name));
	sk->flags = (int)read_uint32(buf);
	vgflags = (int)read_uint32(buf);

	sk->vg = vg_new(NULL, vgflags);
	if (vg_load(sk->vg, buf) == -1) {
		vg_destroy(sk->vg);
		Free(sk->vg, M_VG);
		return (-1);
	}
	sk->vg->redraw++;
	vg_rasterize(sk->vg);
	return (0);
}

void
sketch_save(struct sketch *sk, struct netbuf *buf)
{
	write_string(buf, sk->name);
	write_uint32(buf, (Uint32)sk->flags);
	write_uint32(buf, (Uint32)sk->vg->flags);
	vg_save(sk->vg, buf);
}

static void
update_sketch(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct tile_element *tel = argv[2].p;
	struct sketch *sk = tel->tel_sketch.sk;

	sketch_scale(sk, -1, -1, tel->tel_sketch.scale, 0, 0);

	tv->tile->flags |= TILE_DIRTY;
}

struct window *
sketch_edit(struct tileview *tv, struct tile_element *tel)
{
	struct sketch *sk = tel->tel_sketch.sk;
	struct vg *vg = sk->vg;
	struct window *win;
	struct notebook *nb;
	struct notebook_tab *ntab;
	
	win = window_new(0, NULL);
	window_set_caption(win, _("Sketch %s"), sk->name);
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

	nb = notebook_new(win, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
	ntab = notebook_add_tab(nb, _("Color"), BOX_VERT);
	{
		struct hsvpal *pal;
		struct fspinbutton *fsb;
		struct box *hb;

		pal = hsvpal_new(ntab);
		WIDGET(pal)->flags |= WIDGET_WFILL|WIDGET_HFILL;
		widget_bind(pal, "pixel-format", WIDGET_POINTER, &tv->ts->fmt);
		widget_bind(pal, "hue", WIDGET_FLOAT, &sk->h);
		widget_bind(pal, "saturation", WIDGET_FLOAT, &sk->s);
		widget_bind(pal, "value", WIDGET_FLOAT, &sk->v);
		widget_bind(pal, "alpha", WIDGET_FLOAT, &sk->a);
	}
	ntab = notebook_add_tab(nb, _("Texture"), BOX_VERT);

	ntab = notebook_add_tab(nb, _("Settings"), BOX_VERT);
	{
		struct fspinbutton *fsb;
		struct spinbutton *sb;
		struct mspinbutton *msb;

		fsb = fspinbutton_new(ntab, NULL, _("Scale: "));
		widget_bind(fsb, "value", WIDGET_FLOAT, &tel->tel_sketch.scale);
		fspinbutton_set_min(fsb, 1.0);
		fspinbutton_set_increment(fsb, 0.1);
		event_new(fsb, "fspinbutton-changed", update_sketch, "%p,%p",
		    tv, tel);
		
		msb = mspinbutton_new(ntab, ",", _("Coordinates: "));
		widget_bind(msb, "xvalue", WIDGET_INT, &tel->tel_sketch.x);
		widget_bind(msb, "yvalue", WIDGET_INT, &tel->tel_sketch.y);
		event_new(fsb, "fspinbutton-changed", update_sketch, "%p,%p",
		    tv, tel);
		
		sb = spinbutton_new(ntab, _("Overall alpha: "));
		spinbutton_set_range(sb, 0, 255);
		widget_bind(sb, "value", WIDGET_INT, &tel->tel_sketch.alpha);
	}

	return (win);
}

void
sketch_begin_undoblk(struct sketch *sk)
{
	struct sketch_undoblk *ublk;

	while (sk->nublks > sk->curblk+1) {
		ublk = &sk->ublks[sk->nublks-1];
		Free(ublk->mods, M_RG);
		sk->nublks--;
	}

	sk->ublks = Realloc(sk->ublks, ++sk->nublks *
	                    sizeof(struct pixmap_mod));
	sk->curblk++;

	ublk = &sk->ublks[sk->curblk];
	ublk->mods = Malloc(sizeof(struct pixmap_mod), M_RG);
	ublk->nmods = 0;
}

void
sketch_undo(struct tileview *tv, struct tile_element *tel)
{
	struct sketch *sk = tel->tel_sketch.sk;
	struct sketch_undoblk *ublk = &sk->ublks[sk->curblk];
	int i;

	if (sk->curblk-1 <= 0)
		return;

	for (i = 0; i < ublk->nmods; i++) {
		struct sketch_mod *mod = &ublk->mods[i];

		dprintf("undo mod %p\n", mod);
	}
	sk->curblk--;
	tv->tile->flags |= TILE_DIRTY;
}

void
sketch_redo(struct tileview *tv, struct tile_element *tel)
{
	/* TODO */
}

void
sketch_mousebuttondown(struct tileview *tv, struct tile_element *tel,
    double x, double y, int button)
{
	if (button == SDL_BUTTON_MIDDLE) {
		int x, y;

		mouse_get_state(&x, &y);
		sketch_open_menu(tv, x, y);
		return;
	} else if (button == SDL_BUTTON_RIGHT) {
		if (tv->cur_tool == NULL ||
		   (tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) == 0) {
			tv->scrolling++;
			return;
		}
	}

	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const struct tileview_sketch_tool_ops *ops =
		    (const struct tileview_sketch_tool_ops *)tv->cur_tool->ops;
		
		if (ops->mousebuttondown != NULL)
			ops->mousebuttondown(tv->cur_tool, tel->tel_sketch.sk,
			    x, y, button);
	}
}

void
sketch_mousebuttonup(struct tileview *tv, struct tile_element *tel,
    double x, double y, int button)
{
	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const struct tileview_sketch_tool_ops *ops =
		    (const struct tileview_sketch_tool_ops *)tv->cur_tool->ops;
		
		if (ops->mousebuttonup != NULL)
			ops->mousebuttonup(tv->cur_tool, tel->tel_sketch.sk,
			    x, y, button);
	}
}

void
sketch_mousemotion(struct tileview *tv, struct tile_element *tel,
    double x, double y, double xrel, double yrel, int state)
{
	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const struct tileview_sketch_tool_ops *ops =
		    (const struct tileview_sketch_tool_ops *)tv->cur_tool->ops;
		
		if (ops->mousemotion != NULL)
			ops->mousemotion(tv->cur_tool, tel->tel_sketch.sk,
			    x, y, xrel, yrel);
	}
}

int
sketch_mousewheel(struct tileview *tv, struct tile_element *tel, int which)
{
	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const struct tileview_sketch_tool_ops *ops =
		    (const struct tileview_sketch_tool_ops *)tv->cur_tool->ops;
		
		if (ops->mousewheel != NULL)
			return (ops->mousewheel(tv->cur_tool,
			    tel->tel_sketch.sk, which));
	}
	return (0);
}

void
sketch_keydown(struct tileview *tv, struct tile_element *tel, int keysym,
    int keymod)
{
	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const struct tileview_sketch_tool_ops *ops =
		    (const struct tileview_sketch_tool_ops *)tv->cur_tool->ops;
		
		if (ops->keydown != NULL)
			ops->keydown(tv->cur_tool, tel->tel_sketch.sk,
			    keysym, keymod);
	}
}

void
sketch_keyup(struct tileview *tv, struct tile_element *tel, int keysym,
    int keymod)
{
	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const struct tileview_sketch_tool_ops *ops =
		    (const struct tileview_sketch_tool_ops *)tv->cur_tool->ops;
		
		if (ops->keyup != NULL)
			ops->keyup(tv->cur_tool, tel->tel_sketch.sk,
			    keysym, keymod);
	}
}

static void
select_tool(int argc, union evarg *argv)
{
	struct tileview *tv = argv[1].p;
	struct tileview_tool *tvt = argv[2].p;

	tileview_select_tool(tv, tvt);
}

static void
select_tool_tbar(int argc, union evarg *argv)
{
	struct button *btn = argv[0].p;
	struct toolbar *tbar = argv[1].p;
	struct tileview *tv = argv[2].p;
	struct tileview_tool *tvt = argv[3].p;

	toolbar_select_unique(tbar, btn);
	if (tv->cur_tool == tvt) {
		tileview_unselect_tool(tv);
	} else {
		tileview_select_tool(tv, tvt);
	}
}

void
sketch_open_menu(struct tileview *tv, int x, int y)
{
	struct sketch *sk = tv->tv_sketch.sk;
	struct AGMenu *me;
	struct AGMenuItem *mi;
	
	if (tv->tv_sketch.menu != NULL)
		sketch_close_menu(tv);

	me = tv->tv_sketch.menu = Malloc(sizeof(struct AGMenu), M_OBJECT);
	menu_init(me);

	mi = tv->tv_sketch.menu_item = menu_add_item(me, NULL);
	{
		struct tileview_tool *tvt;
		struct AGMenuItem *m_tool;

		m_tool = menu_action(mi, _("Tools"), OBJEDIT_ICON,
		    NULL, NULL);
		TAILQ_FOREACH(tvt, &tv->tools, tools) {
			if ((tvt->flags & TILEVIEW_SKETCH_TOOL) == 0) {
				continue;
			}
			menu_action(m_tool, _(tvt->ops->name), tvt->ops->icon,
			    select_tool, "%p,%p", tv, tvt);
		}

		menu_separator(mi);
		tileview_generic_menu(tv, mi);
	}
	tv->tv_sketch.menu->sel_item = mi;
	tv->tv_sketch.menu_win = menu_expand(me, mi, x, y);
}

void
sketch_close_menu(struct tileview *tv)
{
	struct AGMenu *me = tv->tv_sketch.menu;
	struct AGMenuItem *mi = tv->tv_sketch.menu_item;

	menu_collapse(me, mi);
	object_destroy(me);
	Free(me, M_OBJECT);

	tv->tv_sketch.menu = NULL;
	tv->tv_sketch.menu_item = NULL;
	tv->tv_sketch.menu_win = NULL;
}

struct toolbar *
sketch_toolbar(struct tileview *tv, struct tile_element *tel)
{
	struct sketch *sk = tel->tel_sketch.sk;
	struct toolbar *tbar;
	struct tileview_tool *tvt;

	tbar = toolbar_new(tv->tel_box, TOOLBAR_VERT, 1, 0);
	TAILQ_FOREACH(tvt, &tv->tools, tools) {
		if ((tvt->flags & TILEVIEW_SKETCH_TOOL) == 0) {
			continue;
		}
		toolbar_add_button(tbar, 0, tvt->ops->icon >= 0 ?
		    ICON(tvt->ops->icon) : NULL, 1, 0,
		    select_tool_tbar, "%p,%p,%p", tbar, tv, tvt);
	}
	return (tbar);
}

