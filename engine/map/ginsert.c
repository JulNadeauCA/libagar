/*	$Csoft: ginsert.c,v 1.3 2005/08/15 02:27:26 vedge Exp $	*/

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
#include <engine/actor.h>

#ifdef MAP

#include <engine/rg/tileset.h>

#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/primitive.h>

#include "map.h"
#include "mapedit.h"

struct ginsert_tool {
	struct tool tool;
	enum gfx_snap_mode snap_mode;
	int replace_mode;
};

static void
ginsert_init(void *p)
{
	struct ginsert_tool *ins = p;

	ins->snap_mode = GFX_SNAP_NOT;
	ins->replace_mode = 0;

	tool_push_status(ins, _("Select position on map ($(L)=Insert)"));
}

static void
ginsert_pane(void *p, void *con)
{
	struct ginsert_tool *ins = p;
	struct mapview *mv = TOOL(ins)->mv;
	struct radio *rad;
	struct checkbox *cb;
	struct combo *com;
	struct tlist_item *it;
	
	if ((it = tlist_selected_item(mv->lib_tl)) != NULL) {
		label_new(con, LABEL_STATIC, _("Object: %s (%s)"),
		    OBJECT(it->p1)->name, OBJECT(it->p1)->type);
	}
	
	label_new(con, LABEL_STATIC, _("Snap to: "));
	rad = radio_new(con, gfx_snap_names);
	widget_bind(rad, "value", WIDGET_INT, &ins->snap_mode);

	cb = checkbox_new(con, _("Replace mode"));
	widget_bind(cb, "state", WIDGET_INT, &ins->replace_mode);
}

static int
ginsert_mousemotion(void *p, int x, int y, int xrel, int yrel, int btn)
{
	struct ginsert_tool *ins = p;
	struct mapview *mv = TOOL(ins)->mv;
	int nx = x/MV_TILESZ(mv);
	int ny = y/MV_TILESZ(mv);
	struct tlist_item *it;
	struct object *ob;

	if (nx == mv->mouse.x && ny == mv->mouse.y)
		return (0);

	if ((it = tlist_selected_item(mv->lib_tl)) == NULL ||
	    strcmp(it->class, "object") != 0 ||
	    mv->cx == -1 || mv->cy == -1) {
		return (1);
	}
	ob = it->p1;

	tool_set_status(ins, _("Move object %s at [%d,%d] with $(L)."),
	    ob->name, mv->cx, mv->cy);
	return (0);
}

static int
ginsert_cursor(void *p, SDL_Rect *rd)
{
	struct ginsert_tool *ins = p;
	struct mapview *mv = TOOL(ins)->mv;
	struct map *m = mv->map;

	return (-1);
}

static int
ginsert_effect(void *p, struct node *n)
{
	struct ginsert_tool *ins = p;
	struct mapview *mv = TOOL(ins)->mv;
	struct map *m = mv->map;
	struct tlist_item *it;
	struct actor *go;

	if ((it = tlist_selected_item(mv->lib_tl)) == NULL ||
	    strcmp(it->class, "object") != 0 ||
	    mv->cx == -1 || mv->cy == -1) {
		return (1);
	}
	go = it->p1;

	if (go->parent != NULL) {
		dprintf("detaching from %s\n", OBJECT(go->parent)->name);
		TAILQ_REMOVE(&SPACE(go->parent)->actors, go, actors);
		space_detach(go->parent, go);
	}
	go->g_map.x = mv->cx;
	go->g_map.y = mv->cy;
	go->g_map.l0 = m->cur_layer;
	go->g_map.l1 = m->cur_layer;
	
	if (space_attach(m, go) == -1) {
		text_tmsg(MSG_ERROR, 1000, "%s: %s", OBJECT(go)->name,
		    error_get());
	} else {
		TAILQ_INSERT_TAIL(&SPACE(go->parent)->actors, go, actors);
	}
	return (1);
}

const struct tool_ops ginsert_ops = {
	"Ginsert", N_("Insert geometrical object"),
	STAMP_TOOL_ICON,
	sizeof(struct ginsert_tool),
	TOOL_HIDDEN,
	ginsert_init,
	NULL,				/* destroy */
	ginsert_pane,
	NULL,				/* edit */
	ginsert_cursor,
	ginsert_effect,

	ginsert_mousemotion,
	NULL,
	NULL,				/* mousebuttonup */
	NULL,				/* keydown */
	NULL				/* keyup */
};

#endif /* MAP */
