/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/radio.h>
#include <agar/gui/tlist.h>

#include <agar/map/map.h>

#include <string.h>

struct ginsert_tool {
	MAP_Tool tool;
	enum rg_snap_mode snap_mode;
	int replace_mode;
};

static void
Init(void *_Nonnull p)
{
	struct ginsert_tool *ins = p;

	ins->snap_mode = RG_SNAP_NONE;
	ins->replace_mode = 0;

	MAP_ToolPushStatus(ins, _("Select position on map ($(L)=Insert)"));
}

static void
EditPane(void *_Nonnull p, void *_Nonnull con)
{
	struct ginsert_tool *ins = p;
	MAP_View *mv = TOOL(ins)->mv;
	AG_TlistItem *it;
	
	if ((it = AG_TlistSelectedItem(mv->lib_tl)) != NULL) {
		AG_LabelNew(con, 0, _("Object: %s (%s)"),
		    OBJECT(it->p1)->name, OBJECT(it->p1)->cls->name);
	}
	
	AG_LabelNewS(con, 0, _("Snap to: "));
	AG_RadioNewUint(con, AG_RADIO_HFILL, rgTileSnapModes, &ins->snap_mode);
	AG_CheckboxNewInt(con, 0, _("Replace mode"), &ins->replace_mode);
}

static int
MouseMotion(void *_Nonnull p, int x, int y, int xrel, int yrel, int btn)
{
	struct ginsert_tool *ins = p;
	MAP_View *mv = TOOL(ins)->mv;
	int nx = x/AGMTILESZ(mv);
	int ny = y/AGMTILESZ(mv);
	AG_TlistItem *it;
	AG_Object *ob;

	if (nx == mv->mouse.x && ny == mv->mouse.y)
		return (0);

	if ((it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	    strcmp(it->cat, "object") != 0 ||
	    mv->cx == -1 || mv->cy == -1) {
		return (1);
	}
	ob = it->p1;

	MAP_ToolSetStatus(ins, _("Move object %s at [%d,%d] with $(L)."),
	    ob->name, mv->cx, mv->cy);
	return (0);
}

static int
Cursor(void *_Nonnull p, AG_Rect *_Nonnull rd)
{
#if 0
	struct ginsert_tool *ins = p;
	MAP_View *mv = TOOL(ins)->mv;
	MAP *m = mv->map;
#endif
	return (-1);
}

static int
Effect(void *_Nonnull p, MAP_Node *_Nonnull n)
{
	struct ginsert_tool *ins = p;
	MAP_View *mv = TOOL(ins)->mv;
	MAP *m = mv->map;
	AG_TlistItem *it;
	MAP_Actor *go;

	if ((it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	    strcmp(it->cat, "object") != 0 ||
	    mv->cx == -1 || mv->cy == -1) {
		return (1);
	}
	go = it->p1;

	if (go->parent != NULL) {
		TAILQ_REMOVE(&go->parent->actors, go, actors);
		MAP_DetachActor(go->parent, go);
	}
	go->g_map.x = mv->cx;
	go->g_map.y = mv->cy;
	go->g_map.l0 = m->cur_layer;
	go->g_map.l1 = m->cur_layer;
	
	MAP_AttachActor(m, go);
	TAILQ_INSERT_TAIL(&go->parent->actors, go, actors);
	return (1);
}

const MAP_ToolOps mapGInsertOps = {
	"Ginsert", N_("Insert geometrical object"),
	&mapIconStamp,
	sizeof(struct ginsert_tool),
	TOOL_HIDDEN,
	1,
	Init,
	NULL,				/* destroy */
	EditPane,
	NULL,				/* edit */
	Cursor,
	Effect,
	MouseMotion,
	NULL,
	NULL,				/* mousebuttonup */
	NULL,				/* keydown */
	NULL				/* keyup */
};
