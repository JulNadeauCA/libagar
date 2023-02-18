/*
 * Copyright (c) 2022-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/separator.h>
#include <agar/gui/tlist.h>

#include <agar/map/map.h>

#include <string.h>

/*
 * Insert a dynamic map object (MAP_Object).
 */

typedef struct map_insert_obj_tool {
	MAP_Tool tool;			/* MAP_Tool -> MAP_InsertObjTool */
	Uint flags;
#define MAP_INSERTOBJTOOL_REPLACE 0x01	/* Replace mode */
	Uint32 _pad;
	MAP_Object *selectedObj;	/* Selected object */
} MAP_InsertObjTool;

static void
Init(void *_Nonnull p)
{
	MAP_InsertObjTool *tool = p;

	tool->flags = MAP_INSERTOBJTOOL_REPLACE;
	tool->selectedObj = NULL;
	MAP_ToolPushStatus(tool,
	    _("Select a location on the map. "
	      "Drag to instantiate over multiple nodes."));
}

static void
EditPane(void *_Nonnull p, void *_Nonnull box)
{
	MAP_InsertObjTool *tool = p;
	MAP_View *mv = TOOL(tool)->mv;
	AG_TlistItem *it;
	MAP_Object *mo;
	AG_ObjectClass **hier;
	int          i, nHier;

	if ((it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	    strcmp(it->cat, "object") != 0) {
		AG_LabelNewS(box, 0, _("Select a library object..."));
		return;
	}
	mo = it->p1;

	AG_LabelNew(box, 0, _("Insert " AGSI_YEL "%s" AGSI_RST
	                           " (" AGSI_CYAN "%s" AGSI_RST ")"),
	    OBJECT(mo)->name,
	    OBJECT(mo)->cls->name);
	
	AG_CheckboxNewFlag(box, 0, _("Replace mode"), &tool->flags,
	    MAP_INSERTOBJTOOL_REPLACE);

	AG_SeparatorNewHoriz(box);

	if (AG_ObjectGetInheritHier(mo, &hier, &nHier) != 0) {
		AG_FatalError(NULL);
	}
	for (i = nHier-1; i >= 0; i--) {
		MAP_ObjectClass *clsMo = (MAP_ObjectClass *)hier[i];
		AG_Label *lbl;

		if (!AG_ClassIsNamed(clsMo, "MAP_Object:*") ||
		    clsMo->edit == NULL) {
			continue;
		}
		lbl = AG_LabelNew(box, AG_LABEL_HFILL, "%s:",
		    AGCLASS(clsMo)->name);
		AG_SetFontSize(lbl, "80%");
		clsMo->edit(mo, box, TOOL(tool));
		AG_SeparatorNewHoriz(box);
	}
	free(hier);
}

static int
InsertObjectAt(MAP_InsertObjTool *_Nonnull tool, MAP_Node *_Nonnull node,
    MAP_Object *_Nonnull mo)
{
	MAP_View *mv = TOOL(tool)->mv;
	MAP_Location *loc;

	if (tool->flags & MAP_INSERTOBJTOOL_REPLACE) {
		Uint i;

		for (i = 0; i < node->nLocs; i++) {
			if (node->locs[i].obj == mo)
				break;
		}
		if (i < node->nLocs) {
			MAP_ViewStatus(mv,
			    _("Existing " AGSI_YEL "%s" AGSI_RST
			      " (id %u) at %d,%d : [" AGSI_BOLD "%d" AGSI_RST "] (Replace mode is ON)"),
			    OBJECT(mo)->name, mo->id, mv->cx, mv->cy, i);
			return (0);
		}
	}

	node->locs = Realloc(node->locs, (node->nLocs + 1)*sizeof(MAP_Location));
	loc = &node->locs[node->nLocs++];
	loc->obj = mo;
	loc->flags = (MAP_OBJECT_LOCATION_SELECTED |
	              MAP_OBJECT_LOCATION_VALID);
	loc->x = mv->cx;
	loc->y = mv->cy;
	loc->layer = mv->map->layerCur;
	loc->z = 0.0f;
	loc->h = 1.0f;
	loc->neigh = NULL;
	loc->nNeigh = 0;

	MAP_ViewStatus(mv,
	    _("Inserted " AGSI_YEL "%s" AGSI_RST
	      " (id %u) at %d,%d : [" AGSI_BOLD "%d" AGSI_RST "]"),
	    OBJECT(mo)->name, mo->id, mv->cx, mv->cy, node->nLocs - 1);

	return (1);
}

static int
MouseMotion(void *_Nonnull p, int x, int y, int xrel, int yrel)
{
	MAP_InsertObjTool *tool = p;
	MAP_View *mv = TOOL(tool)->mv;
	MAP *m = mv->map;

	if (tool->selectedObj != NULL) {
		MAP_Node *node;

		if (mv->cx == -1 || mv->cy == -1) {
			return (0);
		}
		node = &m->map[mv->cy][mv->cx];
#ifdef AG_DEBUG
		if ((node->flags & MAP_NODE_VALID) == 0)
			AG_FatalError("Invalid node");
#endif
		return InsertObjectAt(tool, node, tool->selectedObj);
	}
	return (0);
}

static int
MouseButtonDown(void *_Nonnull obj, int x, int y, int button)
{
	MAP_InsertObjTool *tool = obj;
	MAP_View *mv = TOOL(tool)->mv;
	MAP *map = mv->map;
	MAP_Node *node;
	AG_TlistItem *it;
	MAP_Object *moLib, *mo;

	if (button != AG_MOUSE_LEFT)
		return (0);

	if ((it = AG_TlistSelectedItem(mv->lib_tl)) == NULL ||
	    strcmp(it->cat, "object") != 0 ||
	    mv->cx == -1 || mv->cy == -1) {
		return (1);
	}
	moLib = it->p1;
	if (!AG_OfClass(moLib, "MAP_Object:*")) {
		AG_SetErrorS("Library object is not a map object");
		goto fail;
	}
	node = &map->map[mv->cy][mv->cx];
	if ((node->flags & MAP_NODE_VALID) == 0) {
		AG_SetErrorS("Invalid node");
		goto fail;
	}

	if ((mo = TryMalloc(AGOBJECT_CLASS(moLib)->size)) == NULL) {
		goto fail;
	}
	AG_ObjectInit(mo, OBJECT(moLib)->cls);
	OBJECT(mo)->flags |= AG_OBJECT_NAME_ONATTACH;

	AG_ObjectAttach(map, mo);

	map->objs = Realloc(map->objs, (map->nObjs + 1) * sizeof(MAP_Object *));
	mo->id = map->nObjs++;
	map->objs[mo->id] = mo;

	tool->selectedObj = mo;
	return InsertObjectAt(tool, node, mo);
fail:
	MAP_ViewStatus(mv, _("Failed: %s"), AG_GetError());
	return (1);
}

static int
MouseButtonUp(void *_Nonnull obj, int x, int y, int btn)
{
	MAP_InsertObjTool *tool = obj;
//	MAP_View *mv = TOOL(tool)->mv;
//	MAP *m = mv->map;

	if (tool->selectedObj) {
		tool->selectedObj = NULL;
		return (1);
	}
	return (0);
}

static int
Cursor(void *_Nonnull p, AG_Rect *_Nonnull rd)
{
#if 0
	MAP_InsertObjTool *tool = p;
	MAP_View *mv = TOOL(tool)->mv;
	MAP *m = mv->map;
#endif
	return (-1);
}

static int
Effect(void *_Nonnull p, MAP_Node *_Nonnull n)
{
	return (0);
}

const MAP_ToolOps mapInsertObjOps = {
	"InsertObj",
	N_("Insert map object"),
	&mapIconStamp,
	sizeof(MAP_InsertObjTool),
	0,
	1,
	Init,
	NULL,				/* destroy */
	EditPane,
	NULL,				/* edit */
	Cursor,
	Effect,
	MouseMotion,
	MouseButtonDown,
	MouseButtonUp,
	NULL,				/* keydown */
	NULL				/* keyup */
};
