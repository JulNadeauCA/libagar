/*	$Csoft: propedit.c,v 1.38 2003/06/06 09:03:57 vedge Exp $	*/

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

#include "propedit.h"

#include <engine/widget/vbox.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>

const struct tool_ops propedit_ops = {
	{
		NULL,		/* init */
		tool_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	propedit_window,
	propedit_cursor,
	propedit_effect,
	NULL			/* mouse */
};

static void
propedit_set_edge(struct mapview *mv, Uint32 edge)
{
	struct node *node;

	if (mv->cx == -1 || mv->cy == -1)
		return;
	node = &mv->map->map[mv->cy][mv->cx];
	node->flags &= ~(NODE_EDGE_ANY);
	node->flags |= edge;
}

static void
propedit_set_flag(struct mapview *mv, Uint32 flag)
{
	struct node *node;

	if (mv->cx == -1 || mv->cy == -1)
		return;
	node = &mv->map->map[mv->cy][mv->cx];
	node->flags |= flag;
}

static void
propedit_clear_flag(struct mapview *mv, Uint32 flag)
{
	struct node *node;

	if (mv->cx == -1 || mv->cy == -1)
		return;
	node = &mv->map->map[mv->cy][mv->cx];
	node->flags &= ~flag;
}

static void
propedit_edge_nw(void *p, struct mapview *mv)
{
	propedit_set_edge(mv, NODE_EDGE_NW);
}

static void
propedit_edge_n(void *p, struct mapview *mv)
{
	propedit_set_edge(mv, NODE_EDGE_N);
}

static void
propedit_edge_ne(void *p, struct mapview *mv)
{
	propedit_set_edge(mv, NODE_EDGE_NE);
}

static void
propedit_edge_w(void *p, struct mapview *mv)
{
	propedit_set_edge(mv, NODE_EDGE_W);
}

static void
propedit_edge_none(void *p, struct mapview *mv)
{
	propedit_set_edge(mv, 0);
}

static void
propedit_edge_e(void *p, struct mapview *mv)
{
	propedit_set_edge(mv, NODE_EDGE_E);
}

static void
propedit_edge_sw(void *p, struct mapview *mv)
{
	propedit_set_edge(mv, NODE_EDGE_SW);
}

static void
propedit_edge_s(void *p, struct mapview *mv)
{
	propedit_set_edge(mv, NODE_EDGE_S);
}

static void
propedit_edge_se(void *p, struct mapview *mv)
{
	propedit_set_edge(mv, NODE_EDGE_SE);
}

static void
propedit_flag_walk(void *p, struct mapview *mv)
{
	propedit_set_flag(mv, NODE_WALK);
}

static void
propedit_flag_block(void *p, struct mapview *mv)
{
	propedit_clear_flag(mv, NODE_WALK);
}

void
propedit_init(void *p)
{
	struct propedit *pe = p;

	tool_init(&pe->tool, "propedit", &propedit_ops);
	TOOL(pe)->icon = SPRITE(&mapedit, MAPEDIT_TOOL_PROPEDIT);
	tool_bind_key(pe, KMOD_NONE, SDLK_KP7, propedit_edge_nw, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_KP8, propedit_edge_n, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_KP9, propedit_edge_ne, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_KP4, propedit_edge_w, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_KP5, propedit_edge_none, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_KP6, propedit_edge_e, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_KP1, propedit_edge_sw, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_KP2, propedit_edge_s, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_KP3, propedit_edge_se, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_w, propedit_flag_walk, 1);
	tool_bind_key(pe, KMOD_NONE, SDLK_b, propedit_flag_block, 1);

	pe->node_flags = 0;
	pe->node_mode = 0;
	pe->origin = 0;
}

static void
set_node_mode(int argc, union evarg *argv)
{
	struct propedit *pe = argv[1].p;
	int index = argv[2].i;
	const Uint32 modes[] = {
		0,
		NODE_WALK,
		NODE_CLIMB
	};

	pe->node_mode = modes[index];
}

static void
toggle_node_flag(int argc, union evarg *argv)
{
	struct propedit *pe = argv[1].p;
	int flag = argv[2].i;
	int state = argv[3].i;

	if (state) {
		pe->node_flags |= flag;
	} else {
		pe->node_flags &= ~(flag);
	}
}

static void
toggle_origin(int argc, union evarg *argv)
{
	struct propedit *pe = argv[1].p;
	int state = argv[2].i;

	pe->origin = state;
}

struct window *
propedit_window(void *p)
{
	struct propedit *pe = p;
	struct window *win;
	struct vbox *vb;
	struct radio *rad;
	struct checkbox *cbox;

	win = window_new("mapedit-tool-propedit");
	window_set_caption(win, _("Node props"));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

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
			{ NODE_BIO,	N_("Bio")	 },
			{ NODE_REGEN,	N_("Regen")	 },
			{ NODE_SLOW,	N_("Slow")	 },
			{ NODE_HASTE,	N_("Haste")	 },
		};
		const int nprops = sizeof(props) / sizeof(props[0]);
		int i;

		rad = radio_new(vb, node_modes);
		widget_focus(rad);
		event_new(rad, "radio-changed", set_node_mode, "%p", pe);

		cbox = checkbox_new(vb, _("Origin"));
		event_new(cbox, "checkbox-changed", toggle_origin, "%p", pe);

		for (i = 0; i < nprops; i++) {
			cbox = checkbox_new(vb, "%s", props[i].name);
			event_new(cbox, "checkbox-changed", toggle_node_flag,
			    "%p, %i", pe, props[i].flag);
		}
	}
	return (win);
}

int
propedit_cursor(void *p, struct mapview *mv, SDL_Rect *rd)
{
	/* TODO */
	return (-1);
}

static void
set_edge(struct node *node, Uint32 edge)
{
	node->flags &= ~NODE_EDGE_ANY;
	node->flags |= edge;
}

void
propedit_effect(void *p, struct mapview *mv, struct node *node)
{
	struct propedit *pe = p;
	struct map *m = mv->map;
	Uint8 *ks;

	if (pe->origin) {
		m->origin.x = mv->cx;
		m->origin.y = mv->cy;
	}

	/* Set the bio/regen and slow/haste flags. */
	node->flags &= ~(NODE_BIO|NODE_REGEN|NODE_SLOW|NODE_HASTE);
	node->flags |= pe->node_flags;

	node->flags &= ~(NODE_WALK|NODE_CLIMB);
	if (pe->node_mode == 0) {
		node->flags &= ~(NODE_WALK);
	} else {
		node->flags |= pe->node_mode;
	}
	
	ks = SDL_GetKeyState(NULL);
	if (ks[SDLK_KP7]) set_edge(node, NODE_EDGE_NW);
	if (ks[SDLK_KP8]) set_edge(node, NODE_EDGE_N);
	if (ks[SDLK_KP9]) set_edge(node, NODE_EDGE_NE);
	if (ks[SDLK_KP4]) set_edge(node, NODE_EDGE_W);
	if (ks[SDLK_KP5]) set_edge(node, 0);
	if (ks[SDLK_KP6]) set_edge(node, NODE_EDGE_E);
	if (ks[SDLK_KP1]) set_edge(node, NODE_EDGE_SW);
	if (ks[SDLK_KP2]) set_edge(node, NODE_EDGE_S);
	if (ks[SDLK_KP3]) set_edge(node, NODE_EDGE_SE);

	if (ks[SDLK_w])	node->flags |= NODE_WALK;
	if (ks[SDLK_b]) node->flags &= ~NODE_WALK;
}

