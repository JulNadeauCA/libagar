/*	$Csoft: sketch.c,v 1.19 2005/06/07 02:01:35 vedge Exp $	*/

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
#include <engine/widget/combo.h>
#include <engine/widget/notebook.h>
#include <engine/widget/units.h>
#include <engine/widget/radio.h>

#include "tileset.h"
#include "tileview.h"
#include "texsel.h"

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
		vw = vg->w;
	} else {
		vw = (float)w/(float)TILESZ/scale;
	}
	if (h == -1) {
		vh = vg->h;
	} else {
		vh = (float)h/(float)TILESZ/scale;
	}

	vg_scale(vg, vw, vh, scale);
	
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
	struct combo *com;
	
	win = window_new(0, NULL);
	window_set_caption(win, _("Sketch %s"), sk->name);
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

	com = vg_layer_selector(win, vg);

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

static void
poll_styles(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct vg *vg = argv[1].p;
	struct vg_style *vgs;

	tlist_clear_items(tl);
	pthread_mutex_lock(&vg->lock);
	TAILQ_FOREACH(vgs, &vg->styles, styles) {
		tlist_insert_item(tl, NULL, vgs->name, vgs);
	}
	pthread_mutex_unlock(&vg->lock);
	tlist_restore_selections(tl);
}

struct window *
sketch_edit_element(struct tileview *tv, struct tile_element *tel,
    struct vg_element *vge)
{
	struct sketch *sk = tel->tel_sketch.sk;
	struct vg *vg = sk->vg;
	struct window *win;
	struct notebook *nb;
	struct notebook_tab *ntab;
	const struct vg_element_ops *ops;
	struct radio *rad;
	struct spinbutton *sb;
	struct combo *com;
	
	if ((win = window_new(0, "%s-%p-%p", sk->name, tel, vge)) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Sketch element (%s)"), sk->name);
	window_set_position(win, WINDOW_MIDDLE_RIGHT, 0);

	ops = vg_element_types[vge->type];
	label_staticf(win, _("Element type: %s"), ops->name);
	
	sb = spinbutton_new(win, _("Layer: "));
	widget_bind(sb, "value", WIDGET_INT, &vge->layer);
	spinbutton_set_min(sb, 0);

	com = combo_new(win, COMBO_POLL, _("Style: "));
	event_new(com->list, "tlist-poll", poll_styles, "%p", vg);

	nb = notebook_new(win, NOTEBOOK_WFILL|NOTEBOOK_HFILL);
	ntab = notebook_add_tab(nb, _("Color"), BOX_VERT);
	{
		struct hsvpal *pal;

		pal = hsvpal_new(ntab);
		WIDGET(pal)->flags |= WIDGET_WFILL|WIDGET_HFILL;
		widget_bind(pal, "pixel-format", WIDGET_POINTER, &sk->vg->fmt);
		widget_bind(pal, "pixel", WIDGET_UINT32, &vge->color);
	}
	ntab = notebook_add_tab(nb, _("Line style"), BOX_VERT);
	{
		const char *line_styles[] = {
			N_("Continuous"),
			N_("Stippled"),
			NULL
		};
		const char *endpoint_styles[] = {
			N_("Square"),
			N_("Beveled"),
			N_("Rounded"),
			N_("Mitered"),
			NULL
		};
	
		label_static(ntab, _("Line style: "));
		rad = radio_new(ntab, line_styles);
		widget_bind(rad, "value", WIDGET_INT, &vge->line_st.style);
		
		label_static(ntab, _("Endpoint style: "));
		rad = radio_new(ntab, endpoint_styles);
		widget_bind(rad, "value", WIDGET_INT,
		    &vge->line_st.endpoint_style);

		sb = spinbutton_new(ntab, _("Stipple pattern: "));
		widget_bind(sb, "value", WIDGET_UINT16, &vge->line_st.stipple);
		
		sb = spinbutton_new(ntab, _("Thickness: "));
		widget_bind(sb, "value", WIDGET_UINT8, &vge->line_st.thickness);
		spinbutton_set_min(sb, 1);
		
		sb = spinbutton_new(ntab, _("Miter length: "));
		widget_bind(sb, "value", WIDGET_UINT8, &vge->line_st.miter_len);
	}
	ntab = notebook_add_tab(nb, _("Filling style"), BOX_VERT);
	{
		const char *fill_styles[] = {
			N_("None"),
			N_("Solid"),
			N_("Textured"),
			NULL
		};
		struct texsel *texsel;

		label_static(ntab, _("Filling style: "));
		rad = radio_new(ntab, fill_styles);
		widget_bind(rad, "value", WIDGET_INT, &vge->fill_st.style);
			
		label_static(ntab, _("Texture: "));
		texsel = texsel_new(ntab, tv->ts, 0);
		WIDGET(texsel)->flags |= WIDGET_WFILL|WIDGET_HFILL;
		widget_bind(texsel, "texture-name", WIDGET_STRING,
		    vge->fill_st.texture, sizeof(vge->fill_st.texture));
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

static void
redraw_sketch(int argc, union evarg *argv)
{
	struct vg *vg = argv[1].p;

	vg->redraw++;
}

static void
update_circle_radius(int argc, union evarg *argv)
{
	struct vg *vg = argv[1].p;
	struct vg_element *vge = argv[2].p;

	vge->vg_circle.radius = sqrt(
	    pow(vge->vtx[1].x - vge->vtx[0].x, 2) +
	    pow(vge->vtx[1].y - vge->vtx[0].y, 2));
}

static void
update_circle_vertex(int argc, union evarg *argv)
{
	struct vg *vg = argv[1].p;
	struct vg_element *vge = argv[2].p;

	vge->vtx[1].x = vge->vtx[0].x + vge->vg_circle.radius;
	vge->vtx[1].y = vge->vtx[0].y;
}

struct window *
sketch_select(struct tileview *tv, struct tile_element *tel,
    struct vg_element *vge)
{
	struct sketch *sk = tel->tel_sketch.sk;
	struct vg *vg = sk->vg;
	struct tileview_ctrl *ctrl;
	u_int i;

	vge->selected = 1;

	switch (vge->type) {
	case VG_LINES:
	case VG_LINE_STRIP:
	case VG_LINE_LOOP:
	case VG_POLYGON:
		for (i = 0; i < vge->nvtx; i++) {
			ctrl = tileview_insert_ctrl(tv, TILEVIEW_VERTEX,
			    "%*d,%*d", &vge->vtx[i].x, &vge->vtx[i].y);
			ctrl->vg = vg;
			ctrl->vge = vge;
			ctrl->buttonup = event_new(tv, NULL, redraw_sketch,
			    "%p", vg);
		}
		break;
	case VG_CIRCLE:
		ctrl = tileview_insert_ctrl(tv, TILEVIEW_VERTEX, "%*d,%*d",
		    &vge->vtx[0].x, &vge->vtx[0].y);
		ctrl->vg = vg;
		ctrl->vge = vge;
		ctrl->buttonup = event_new(tv, NULL, redraw_sketch, "%p", vg);
		ctrl->motion = event_new(tv, NULL, update_circle_vertex,
		    "%p,%p", vg, vge);
		
		ctrl = tileview_insert_ctrl(tv, TILEVIEW_VERTEX, "%*d,%*d",
		    &vge->vtx[1].x, &vge->vtx[1].y);
		ctrl->vg = vg;
		ctrl->vge = vge;
		ctrl->buttonup = event_new(tv, NULL, redraw_sketch, "%p", vg);
		ctrl->motion = event_new(tv, NULL, update_circle_radius,
		    "%p,%p", vg, vge);
		break;
	default:
		break;
	}
	return (sketch_edit_element(tv, tel, vge));
}

void
sketch_unselect(struct tileview *tv, struct tile_element *tel,
    struct vg_element *vge)
{
	char name[OBJECT_NAME_MAX];
	struct sketch *sk = tel->tel_sketch.sk;
	struct vg *vg = sk->vg;
	struct tileview_ctrl *ctrl, *nctrl;
	struct window *win;

	TAILQ_FOREACH(win, &view->windows, windows) {
		snprintf(name, sizeof(name), "win-%s-%p-%p", sk->name, tel,
		    vge);
		if (strcmp(name, OBJECT(win)->name) == 0) {
			view_detach(win);
			break;
		}
	}

	for (ctrl = TAILQ_FIRST(&tv->ctrls);
	     ctrl != TAILQ_END(&tv->ctrls);
	     ctrl = nctrl) {
		nctrl = TAILQ_NEXT(ctrl, ctrls);
		if (ctrl->vg == vg && ctrl->vge == vge)
			tileview_remove_ctrl(tv, ctrl);
	}

	vge->selected = 0;
}

void
sketch_mousebuttondown(struct tileview *tv, struct tile_element *tel,
    double x, double y, int button)
{
	struct sketch *sk = tel->tel_sketch.sk;
	struct vg *vg = sk->vg;

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
		
		if (ops->mousebuttondown != NULL) {
			ops->mousebuttondown(tv->cur_tool, sk, x, y, button);
			return;
		}
	}

	{
		struct vg_element *vge;
		float idx, closest_idx = FLT_MAX;
		struct vg_element *closest_vge = NULL;

		TAILQ_FOREACH(vge, &vg->vges, vges) {
			if (vge->ops->intsect != NULL) {
				idx = vge->ops->intsect(vg, vge, x, y);
				if (idx < closest_idx) {
					closest_idx = idx;
					closest_vge = vge;
					if (idx == 0)
						break;
				}
			}
		}
		if (closest_vge != NULL && closest_idx < FLT_MAX-2) {
			if (closest_vge->selected) {
				sketch_unselect(tv, tel, closest_vge);
			} else {
				struct window *pwin = widget_parent_window(tv);
				struct window *win;

				if ((SDL_GetModState() & KMOD_CTRL) == 0) {
					TAILQ_FOREACH(vge, &vg->vges, vges) {
						if (vge->selected)
							sketch_unselect(tv,
							    tel, vge);
					}
				}

				win = sketch_select(tv, tel, closest_vge);
				if (win != NULL) {
					window_attach(pwin, win);
					window_show(win);
					view->focus_win = pwin;
					widget_focus(tv);
				}
			}
			vg->redraw++;
		}
	}
}

/* Evaluate the intersection between two rectangles. */
int
vg_rcollision(struct vg *vg, struct vg_rect *r1, struct vg_rect *r2,
    struct vg_rect *ixion)
{
	double r1xmin, r1xmax, r1ymin, r1ymax;
	double r2xmin, r2xmax, r2ymin, r2ymax;
	double ixw, ixh;

	r1xmin = r1->x;
	r1ymin = r1->y;
	r1xmax = r1->x+r1->w;
	r1ymax = r1->y+r1->h;

	r2xmin = r2->x;
	r2ymin = r2->y;
	r2xmax = r2->x+r2->w;
	r2ymax = r2->y+r2->h;

	if (r2xmin > r1xmin)
		r1xmin = r2xmin;
	if (r2ymin > r1ymin)
		r1ymin = r2ymin;

	if (r2xmax < r1xmax)
		r1xmax = r2xmax;
	if (r2ymax < r1ymax)
		r1ymax = r2ymax;
	
	ixw = r1xmax - (r1xmin > 0 ? r1xmax-r1xmin : 0);
	ixh = r1ymax - (r1ymin > 0 ? r1ymax-r1ymin : 0);
	if (ixion != NULL) {
		ixion->x = r1xmin;
		ixion->y = r1ymin;
		ixion->w = ixw;
		ixion->h = ixh;
	}
	return (ixw > 0 && ixh > 0);
}

/*
 * Rasterize an element as well as other overlapping elements.
 * The vg must be locked.
 */
void
vg_rasterize_element(struct vg *vg, struct vg_element *vge)
{
	struct vg_element *ovge;
	struct vg_rect r1, r2;

	if (!vge->drawn) {
		vge->ops->draw(vg, vge);
		vge->drawn = 1;
	}
	if (vge->ops->bbox != NULL) {
		vge->ops->bbox(vg, vge, &r1);
		TAILQ_FOREACH(ovge, &vg->vges, vges) {
			if (ovge->drawn || ovge == vge ||
			    ovge->ops->bbox == NULL) {
				continue;
			}
			ovge->ops->bbox(vg, ovge, &r2);
			if (vg_rcollision(vg, &r1, &r2, NULL))
				vg_rasterize_element(vg, ovge);
		}
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
		
		if (ops->mousemotion != NULL) {
			ops->mousemotion(tv->cur_tool, tel->tel_sketch.sk,
			    x, y, xrel, yrel);
			return;
		}
	}

	{
		struct sketch *sk = tel->tel_sketch.sk;
		struct vg *vg = sk->vg;
		float idx, closest_idx = FLT_MAX;
		struct vg_element *vge, *closest_vge = NULL;

		TAILQ_FOREACH(vge, &vg->vges, vges) {
			vge->mouseover = 0;
			if (vge->ops->intsect != NULL) {
				idx = vge->ops->intsect(vg, vge, x, y);
				if (idx < closest_idx) {
					closest_idx = idx;
					closest_vge = vge;
					if (idx == 0)
						break;
				}
			}
		}
		if (closest_vge != NULL && closest_idx < FLT_MAX-2) {
			closest_vge->mouseover = 1;
			vg->redraw++;
		}
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
	struct sketch *sk = tel->tel_sketch.sk;
	struct vg *vg = sk->vg;
	struct vg_element *vge, *nvge;

	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const struct tileview_sketch_tool_ops *ops =
		    (const struct tileview_sketch_tool_ops *)tv->cur_tool->ops;
		
		if (ops->keydown != NULL) {
			ops->keydown(tv->cur_tool, sk, keysym, keymod);
			return;
		}
	}
	switch (keysym) {
	case SDLK_DELETE:
		for (vge = TAILQ_FIRST(&vg->vges);
		     vge != TAILQ_END(&vg->vges);
		     vge = nvge) {
		     	nvge = TAILQ_NEXT(vge, vges);
			if (vge->selected) {
				sketch_unselect(tv, tel, vge);
				vg_destroy_element(vg, vge);
				vg->redraw++;
			}
		}
		break;
	}
}

void
sketch_keyup(struct tileview *tv, struct tile_element *tel, int keysym,
    int keymod)
{
	struct sketch *sk = tel->tel_sketch.sk;

	if (tv->cur_tool != NULL &&
	    tv->cur_tool->flags & TILEVIEW_SKETCH_TOOL) {
		const struct tileview_sketch_tool_ops *ops =
		    (const struct tileview_sketch_tool_ops *)tv->cur_tool->ops;
		
		if (ops->keyup != NULL) {
			ops->keyup(tv->cur_tool, sk, keysym, keymod);
			return;
		}
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
	
		menu_int_flags(mi, _("Show sketch origin"), VGORIGIN_ICON,
		    &sk->vg->flags, VG_VISORIGIN, 0);
		menu_int_flags(mi, _("Show sketch grid"), SNAP_GRID_ICON,
		    &sk->vg->flags, VG_VISGRID, 0);
		menu_int_flags(mi, _("Show sketch extents"), VGBLOCK_ICON,
		    &sk->vg->flags, VG_VISBBOXES, 0);

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

