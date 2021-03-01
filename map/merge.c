/*
 * Copyright (c) 2002-2021 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core.h>
#include <agar/gui.h>
#include <agar/map/map.h>

#include <string.h>

#if 0
static const AG_Version brushSetVersion = {
	"agar-map brush set",
	6, 0
};
#endif

typedef struct merge_tool {
	MAP_Tool tool;                            /* MAP_Tool -> MERGE_Tool */
	TAILQ_HEAD_(ag_object) brushes;           /* Brush maps */
	AG_Tlist *tlBrushes;                      /* Brush selection */
} MERGE_Tool;

enum {
	NODE, FILL,
	EDNW, EDNO, EDNE,
	EDWE, EDEA,
	EDSW, EDSO, EDSE
};

#if 0
static const int
MergeTable[9][9] = {
       /* nw    n     ne    w     fill  e     sw    s     se */
	{ EDNW, EDNO, EDNO, EDWE, NODE, FILL, FILL, FILL, FILL }, /* nw */
	{ EDNO, EDNO, EDNO, FILL, NODE, FILL, FILL, FILL, FILL }, /* n */
	{ EDNO, EDNO, NODE, 0   , NODE, EDEA, FILL, FILL, EDEA }, /* ne */
	{ EDWE, FILL, FILL, EDWE, NODE, FILL, EDWE, FILL, FILL }, /* w */
	{ FILL, FILL, FILL, FILL, NODE, FILL, FILL, FILL, FILL }, /* fill */
	{ FILL, FILL, EDEA, FILL, NODE, EDEA, FILL, FILL, EDEA }, /* e */
	{ EDWE, 0,    EDEA, EDWE, NODE, 0,    EDSW, EDSO, EDSO }, /* sw */
	{ FILL, FILL, FILL, FILL, NODE, FILL, EDSO, EDSO, EDSO }, /* s */
	{ EDWE, 0,    EDEA, EDNO, NODE, EDNO, EDSO, EDSO, EDSE }  /* se */
};
#endif

static void
FreeBrushes(MAP_Tool *_Nonnull t)
{
	MERGE_Tool *mt = t;
	AG_Object *ob, *obNext;

	for (ob = TAILQ_FIRST(&mt->brushes);
	     ob != TAILQ_END(&mt->brushes);
	     ob = obNext) {
		obNext = TAILQ_NEXT(ob, cobjs);
		AG_ObjectDestroy(ob);
	}
	TAILQ_INIT(&mt->brushes);
}

static void
Destroy(MAP_Tool *_Nonnull t)
{
	FreeBrushes(t);
}

static void
CreateBrush(AG_Event *_Nonnull event)
{
	char name[AG_OBJECT_NAME_MAX];
	char mapName[AG_OBJECT_NAME_MAX];
	MERGE_Tool *mt = AG_PTR(1);
	AG_Textbox *tbName = AG_TEXTBOX_PTR(2);
	MAP *m;

	AG_TextboxCopyString(tbName, name, sizeof(name) - sizeof("brush()"));
	if (name[0] == '\0') {
		AG_TextMsg(AG_MSG_ERROR, _("No brush name was given."));
		return;
	}
	
	Snprintf(mapName, sizeof(mapName), "brush(%s)", name);
	if (AG_TlistFindText(mt->tlBrushes, mapName) != NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("A `%s' brush exists."), mapName);
		return;
	}

	m = MAP_New(NULL, mapName);
	if (AG_ObjectLoad(m) == -1) {
		extern int mapDefaultBrushWidth, mapDefaultBrushHeight;

		if (MAP_AllocNodes(map, mapDefaultBrushWidth, mapDefaultBrushHeight) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "MAP_AllocNodes: %s", AG_GetError());
			goto fail;
		}
	}

	TAILQ_INSERT_HEAD(&mt->brushes, OBJECT(m), cobjs);

	AG_TlistDeselectAll(mt->tlBrushes);
	AG_TlistSelect(mt->tlBrushes,
	    AG_TlistAddPtrHead(mt->tlBrushes, NULL, mapName, m));

	AG_TextboxPrintf(tbName, NULL);
	return;
fail:
	AG_ObjectDestroy(m);
}

static void
EditBrush(AG_Event *_Nonnull event)
{
	AG_TlistItem *it;
	MERGE_Tool *mt = AG_PTR(1);
	AG_Window *pwin = AG_WINDOW_PTR(2);

	TAILQ_FOREACH(it, &mt->tlBrushes->items, items) {
		MAP *brush = it->p1;
		AG_Window *win;
		AG_Toolbar *tbar;

		if (!it->selected)
			continue;

		if ((win = AG_WindowNewNamed(0, "mapedit-tool-merge-%s",
		    OBJECT(brush)->name)) == NULL)
			continue;

		tbar = AG_ToolbarNew(win, AG_TOOLBAR_HORIZ, 1, 0);

		MAP_ViewNew(win, brush,
		    MAP_VIEW_EDIT | MAP_VIEW_GRID | MAPVIEW_PROPS,
		    tbar, NULL);

		AG_WindowAttach(pwin, win);
		AG_WindowShow(win);
	}
}

static void
RemoveBrush(AG_Event *_Nonnull event)
{
	MERGE_Tool *mt = AG_PTR(1);
	AG_TlistItem *it, *nit;

	for (it = TAILQ_FIRST(&mt->tlBrushes->items);
	     it != TAILQ_END(&mt->tlBrushes->items);
	     it = nit) {
		nit = TAILQ_NEXT(it, items);
		if (it->selected) {
			char wname[AG_OBJECT_NAME_MAX];
			AG_Object *brush = it->p1;
			AG_Window *win;

			Snprintf(wname, sizeof(wname),
			    "win-mapedit-tool-merge-%s",
			    OBJECT(brush)->name);
			if ((win = AG_FindWindow(wname)) != NULL) {
				AG_WindowHide(win);
				AG_ObjectDetach(win);
			}

			TAILQ_REMOVE(&brushes, brush, cobjs);
			AG_TlistDel(mt->tlBrushes, it);
			AG_ObjectDestroy(brush);
		}
	}
}

static void
Interpolate(MAP *_Nonnull mapSrc, MAP_Node *_Nonnull nodeSrc, MAP_Item *_Nonnull miSrc,
    MAP *_Nonnull mapDst, MAP_Node *_Nonnull nodeDest, MAP_Item *_Nonnull miDst)
{
	/* TODO */
}

static int
Effect(MAP_Tool *_Nonnull t, MAP_Node *_Nonnull n)
{
	MERGE_Tool *mt = t;
	MAP_View *mv = t->mv;
	MAP *map = mv->map;
	AG_TlistItem *it;
	
	/* Avoid circular references. XXX ugly */
	if (strncmp(OBJECT(m)->name, "brush(", 6) == 0)
		return (1);
	
	TAILQ_FOREACH(it, &mt->tlBrushes->items, items) {
		MAP *mapSrc;
		int sx, sy, dx, dy;

		if (!it->selected)
			continue;

		mapSrc = it->p1;
		for (sy = 0, dy = mv->cy;
		     sy < mapSrc->h && dy < map->h;
		     sy++, dy++) {
			for (sx = 0, dx = mv->cx;
			     sx < mapSrc->w && dx < map->w;
			     sx++, dx++) {
				MAP_Node *nodeSrc = &mapSrc->map[sy][sx];
				MAP_Node *nodeDst = &map->map[dy][dx];
				MAP_Item *miSrc, *miDst;

				TAILQ_FOREACH(miSrc, &nodeSrc->items, items) {
					TAILQ_FOREACH(miDst, &nodeDst->items, items) {
						Interpolate(
						    mapSrc, nodeSrc, miSrc,
						    m,      nodeDst, miDst);
					}
				}
			}
		}
	}
	return (1);
}

#if 0
static int
Load(MAP_Tool *_Nonnull t, AG_DataSource *_Nonnull buf)
{
	MERGE_Tool *mt = t;
	Uint32 i, nBrushes;
	
	if (AG_ReadVersion(buf, &brushSetVersion, NULL) != 0)
		return (-1);

	FreeBrushes(t);
	AG_TlistClear(mt->tlBrushes);

	nBrushes = AG_ReadUint32(buf);
	for (i = 0; i < nBrushes; i++) {
		char mapName[AG_OBJECT_NAME_MAX];
		MAP *map;

		AG_CopyString(mapName, buf, sizeof(mapName));
		m = Malloc(sizeof(MAP));
		MAP_Init(map, mapName);
		map_load(map, buf);

		TAILQ_INSERT_HEAD(&mt->brushes, OBJECT(nbrush), cobjs);
		AG_TlistAddPtrHead(mt->tlBrushes, NULL, mapName, nbrush);
	}
	return (0);
}

static int
Save(MAP_Tool *_Nonnull t, AG_DataSource *_Nonnull buf)
{
	MERGE_Tool *mt = t;
	AG_Object *ob;
	Uint32 nBrushes = 0;
	off_t count_offs;

	AG_WriteVersion(buf, &brushSetVersion);
	count_offs = AG_Tell(buf);			/* Skip count */
	AG_WriteUint32(buf, 0);	
	TAILQ_FOREACH(ob, &mt->brushes, cobjs) {
		struct brush *brush = (struct brush *)ob;

		AG_WriteString(buf, ob->name);
		map_save(brush, buf);
		nBrushes++;
	}
	AG_WriteUint32At(buf, nBrushes, count_offs);	/* Write count */
	return (0);
}
#endif

static int
Cursor(MAP_Tool *_Nonnull t, AG_Rect *_Nonnull rd)
{
	MERGE_Tool *mt = t;
	MAP_View *mv = t->mv;
	MAP *map = mv->map, *mapSrc;
	AG_TlistItem *it;
	int sx,sy, dx,dy;
	int rv = -1;
	const int tileSz = AGMTILESZ(mv);
	
	/* XXX ugly work around circular ref */
	if (strncmp(OBJECT(map)->name, "brush(", 6) == 0)
		return (-1);

	TAILQ_FOREACH(it, &mt->tlBrushes->items, items) {
		if (!it->selected) {
			continue;
		}
		mapSrc = it->p1;
		for (sy = 0, dy = rd->y;
		     sy < mapSrc->h;
		     sy++, dy += tileSz) {
			for (sx = 0, dx = rd->x;
			     sx < mapSrc->w;
			     sx++, dx += tileSz) {
				MAP_Node *nodeSrc = &mapSrc->map[sy][sx];
				MAP_Item *mi;

				TAILQ_FOREACH(mi, &nodeSrc->items, items) {
					MAP_ItemDraw(map, mi,
					    WIDGET(mv)->rView.x1 + dx,
					    WIDGET(mv)->rView.y1 + dy,
					    mv->cam);
					rv = 0;
				}
				if (mv->flags & MAPVIEW_PROPS)
					MAP_ViewDraw_props(mv, nodeSrc, dx,dy,
					    -1, -1);
			}
		}
	}
	return (rv);
}

static void
Init(MAP_Tool *_Nonnull t)
{
	MERGE_Tool *mt = t;
	AG_Window *win;
	AG_Box *hb;
	AG_Button *bu;
	AG_Textbox *tb;

	if ((win = MAP_ToolWindow(t, "mapedit-tool-merge")) == NULL)
		return;

	hb = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		tb = AG_TextboxNewS(hb, AG_TEXTBOX_FOCUS, _("Name: "));
		AG_SetEvent(tb, "textbox-return", CreateBrush, "%p,%p", t, tb);

		bu = AG_ButtonNew(hb, 0, _("Create"));
		AG_ButtonSetPadding(bu, 6);			/* Align */
		AG_ButtonSetFocusable(bu, 0);
		AG_SetEvent(bu, "button-pushed", CreateBrush, "%p,%p", t, tb);
	}

	hb = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	{
#if 0
		bu = AG_ButtonNew(hb, 0, _("Load set"));
		AG_SetEvent(bu, "button-pushed", LoadBrushes, NULL);
		bu = AG_ButtonNew(hb, 0, _("Save set"));
		AG_SetEvent(bu, "button-pushed", SaveBrushes, NULL);
#endif
		bu = AG_ButtonNew(hb, 0, _("Edit"));
		AG_SetEvent(bu, "button-pushed", EditBrush, "%p,%p", t, win);
		bu = AG_ButtonNew(hb, 0, _("Remove"));
		AG_SetEvent(bu, "button-pushed", RemoveBrush, "%p", t);
	}

	TAILQ_INIT(&mt->brushes);

	mt->tlBrushes = AG_TlistNew(win, AG_TLIST_MULTI | AG_TLIST_EXPAND);
}


const MAP_Tool mapMergeOps = {
	"Merge",
	N_("Merge Pattern"),
	&mapIconMerge,
	sizeof(MERGE_Tool),
	0,
	Init,
	Destroy,
	NULL,			/* pane */
	NULL,			/* edit */
	Cursor,
	Effect,
	NULL,			/* mousemotion */
	NULL,			/* mousebuttondown */
	NULL,			/* mousebuttonup */
	NULL,			/* keydown */
	NULL,			/* keyup */
	NULL			/* pane */
};
