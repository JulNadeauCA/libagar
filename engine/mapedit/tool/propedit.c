/*	$Csoft: propedit.c,v 1.44 2003/09/07 04:17:37 vedge Exp $	*/

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
#include <engine/view.h>
#include <engine/mapedit/mapedit.h>

#include <engine/widget/vbox.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>

static void propedit_init(void);
static void propedit_effect(struct mapview *, struct map *, struct node *);
static void set_node_mode(int, union evarg *);
static void toggle_node_flag(int, union evarg *);
static void toggle_origin(int, union evarg *);

struct tool propedit_tool = {
	N_("Property editor"),
	N_("Alter the properties of node references."),
	MAPEDIT_TOOL_PROPEDIT,
	-1,	
	propedit_init,
	NULL,			/* destroy */
	NULL,			/* load */
	NULL,			/* save */
	propedit_effect,
	NULL,
	NULL			/* mouse */
};

static Uint32 node_mode = 0;			/* Set the physics mode */
static Uint32 node_flags = 0;			/* Set the physics flags */
static int origin = 0;				/* Move point of origin */

static void
propedit_set_edge(struct mapview *mv, enum noderef_edge edge)
{
	struct node *node;
	struct noderef *r;

	if (mv->cx == -1 || mv->cy == -1)
		return;

	node = &mv->map->map[mv->cy][mv->cx];
	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (r->layer != mv->map->cur_layer) {
			continue;
		}
		r->r_gfx.edge = (Uint8)edge;
	}
}

static void
propedit_set_flag(struct mapview *mv, Uint32 flag)
{
	struct node *node;
	struct noderef *r;

	if (mv->cx == -1 || mv->cy == -1)
		return;

	node = &mv->map->map[mv->cy][mv->cx];
	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (r->layer != mv->map->cur_layer) {
			continue;
		}
		r->flags |= flag;
	}
}

static void
propedit_clear_flag(struct mapview *mv, Uint32 flag)
{
	struct node *node;
	struct noderef *r;

	if (mv->cx == -1 || mv->cy == -1)
		return;

	node = &mv->map->map[mv->cy][mv->cx];
	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (r->layer != mv->map->cur_layer) {
			continue;
		}
		r->flags &= ~flag;
	}
}

static void
propedit_edge_nw(struct mapview *mv)
{
	propedit_set_edge(mv, NODEREF_EDGE_NW);
}

static void
propedit_edge_n(struct mapview *mv)
{
	propedit_set_edge(mv, NODEREF_EDGE_N);
}

static void
propedit_edge_ne(struct mapview *mv)
{
	propedit_set_edge(mv, NODEREF_EDGE_NE);
}

static void
propedit_edge_w(struct mapview *mv)
{
	propedit_set_edge(mv, NODEREF_EDGE_W);
}

static void
propedit_no_edge(struct mapview *mv)
{
	propedit_set_edge(mv, 0);
}

static void
propedit_edge_e(struct mapview *mv)
{
	propedit_set_edge(mv, NODEREF_EDGE_E);
}

static void
propedit_edge_sw(struct mapview *mv)
{
	propedit_set_edge(mv, NODEREF_EDGE_SW);
}

static void
propedit_edge_s(struct mapview *mv)
{
	propedit_set_edge(mv, NODEREF_EDGE_S);
}

static void
propedit_edge_se(struct mapview *mv)
{
	propedit_set_edge(mv, NODEREF_EDGE_SE);
}

static void
propedit_flag_walk(struct mapview *mv)
{
	propedit_set_flag(mv, NODEREF_WALK);
}

static void
propedit_flag_blk(struct mapview *mv)
{
	propedit_clear_flag(mv, NODEREF_WALK);
}

static void
propedit_init(void)
{
	struct window *win;
	struct vbox *vb;

	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_KP7, propedit_edge_nw, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_KP8, propedit_edge_n, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_KP9, propedit_edge_ne, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_KP4, propedit_edge_w, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_KP5, propedit_no_edge, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_KP6, propedit_edge_e, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_KP1, propedit_edge_sw, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_KP2, propedit_edge_s, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_KP3, propedit_edge_se, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_w, propedit_flag_walk, 1);
	tool_bind_key(&propedit_tool, KMOD_NONE, SDLK_b, propedit_flag_blk, 1);

	win = tool_window_new(&propedit_tool, "mapedit-tool-propedit");

	vb = vbox_new(win, 0);
	{
		static const char *node_modes[] = {
			N_("Block"),
			N_("Walk"),
			N_("Climb"),
			NULL
		};
		static const struct {
			Uint32	flag;
			char	*name;
		} props[] = {
			{ NODEREF_BIO,		N_("Bio")	 },
			{ NODEREF_REGEN,	N_("Regen")	 },
			{ NODEREF_SLOW,		N_("Slow")	 },
			{ NODEREF_HASTE,	N_("Haste")	 },
		};
		const int nprops = sizeof(props) / sizeof(props[0]);
		struct radio *rad;
		struct checkbox *cbox;
		int i;

		rad = radio_new(vb, node_modes);
		event_new(rad, "radio-changed", set_node_mode, NULL);

		cbox = checkbox_new(vb, _("Origin"));
		event_new(cbox, "checkbox-changed", toggle_origin, NULL);

		for (i = 0; i < nprops; i++) {
			cbox = checkbox_new(vb, "%s", _(props[i].name));
			event_new(cbox, "checkbox-changed", toggle_node_flag,
			    "%i", props[i].flag);
		}
	}
}

static void
set_node_mode(int argc, union evarg *argv)
{
	int index = argv[1].i;
	const Uint32 modes[] = {
		0,
		NODEREF_WALK,
		NODEREF_CLIMB
	};

	node_mode = modes[index];
}

static void
toggle_node_flag(int argc, union evarg *argv)
{
	int flag = argv[1].i;
	int state = argv[2].i;

	if (state) {
		node_flags |= flag;
	} else {
		node_flags &= ~(flag);
	}
}

static void
toggle_origin(int argc, union evarg *argv)
{
	int state = argv[1].i;

	origin = state;
}

static void
set_edge(struct map *m, struct node *node, Uint32 edge)
{
	struct noderef *r;

	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (r->layer != m->cur_layer) {
			continue;
		}
		r->r_gfx.edge = edge;
	}
}

static void
propedit_effect(struct mapview *mv, struct map *m, struct node *node)
{
	struct noderef *r;
	Uint8 *ks;

	if (origin) {
		m->origin.x = mv->cx;
		m->origin.y = mv->cy;
	}

	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (r->layer != m->cur_layer)
			continue;

		r->flags &= ~(NODEREF_BIO|NODEREF_REGEN|NODEREF_SLOW|
		              NODEREF_HASTE|NODEREF_WALK|NODEREF_CLIMB);
		r->flags |= node_flags;
	
		if (node_mode == 0) {
			r->flags &= ~(NODEREF_WALK);
		} else {
			r->flags |= node_mode;
		}
	}

	ks = SDL_GetKeyState(NULL);
	if (ks[SDLK_KP7]) set_edge(m, node, NODEREF_EDGE_NW);
	if (ks[SDLK_KP8]) set_edge(m, node, NODEREF_EDGE_N);
	if (ks[SDLK_KP9]) set_edge(m, node, NODEREF_EDGE_NE);
	if (ks[SDLK_KP4]) set_edge(m, node, NODEREF_EDGE_W);
	if (ks[SDLK_KP5]) set_edge(m, node, NODEREF_EDGE_FILL);
	if (ks[SDLK_KP6]) set_edge(m, node, NODEREF_EDGE_E);
	if (ks[SDLK_KP1]) set_edge(m, node, NODEREF_EDGE_SW);
	if (ks[SDLK_KP2]) set_edge(m, node, NODEREF_EDGE_S);
	if (ks[SDLK_KP3]) set_edge(m, node, NODEREF_EDGE_SE);

	if (ks[SDLK_w])	{
		TAILQ_FOREACH(r, &node->nrefs, nrefs) {
			if (r->layer != m->cur_layer) {
				continue;
			}
			r->flags |= NODEREF_WALK;
		}
	}
	if (ks[SDLK_b]) {
		TAILQ_FOREACH(r, &node->nrefs, nrefs) {
			if (r->layer != m->cur_layer) {
				continue;
			}
			r->flags &= ~(NODEREF_WALK);
		}
	}
}

