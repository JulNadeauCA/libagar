/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

const AG_Version merge_ver = {
	"agar merge tool",
	6, 0
};

/* XXX */
static TAILQ_HEAD_(ag_object) brushes = TAILQ_HEAD_INITIALIZER(brushes);
static AG_Tlist *brushes_tl;

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
FreeBrushes(void)
{
	AG_Object *ob, *nob;

	for (ob = TAILQ_FIRST(&brushes);
	     ob != TAILQ_END(&brushes);
	     ob = nob) {
		nob = TAILQ_NEXT(ob, cobjs);
		AG_ObjectDestroy(ob);
	}
	TAILQ_INIT(&brushes);
}

static void
Destroy(MAP_Tool *_Nonnull t)
{
	FreeBrushes();
}

static void
CreateBrush(AG_Event *_Nonnull event)
{
	char brush_name[AG_OBJECT_NAME_MAX];
	char m_name[AG_OBJECT_NAME_MAX];
	AG_Textbox *name_tbox = AG_TEXTBOX_PTR(1);
	MAP *m;

	AG_TextboxCopyString(name_tbox, brush_name,
	    sizeof(brush_name) - sizeof("brush()"));
	if (brush_name[0] == '\0') {
		AG_TextMsg(AG_MSG_ERROR, _("No brush name was given."));
		return;
	}
	
	Snprintf(m_name, sizeof(m_name), "brush(%s)", brush_name);
	if (AG_TlistFindText(brushes_tl, m_name) != NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("A `%s' brush exists."), m_name);
		return;
	}

	m = MAP_New(NULL, m_name);
	if (AG_ObjectLoad(m) == -1) {
		extern int mapDefaultBrushWidth, mapDefaultBrushHeight;

		if (MAP_AllocNodes(m, mapDefaultBrushWidth,
		    mapDefaultBrushHeight) == -1) {
			AG_TextMsg(AG_MSG_ERROR, "MAP_AllocNodes: %s",
			    AG_GetError());
			goto fail;
		}
	}

	TAILQ_INSERT_HEAD(&brushes, OBJECT(m), cobjs);
	AG_TlistDeselectAll(brushes_tl);
	AG_TlistSelect(brushes_tl, AG_TlistAddPtrHead(brushes_tl, NULL,
	    m_name, m));
	AG_TextboxPrintf(name_tbox, NULL);
	return;
fail:
	AG_ObjectDestroy(m);
}

static void
EditBrush(AG_Event *_Nonnull event)
{
	AG_TlistItem *it;
	AG_Window *pwin = AG_WINDOW_PTR(1);

	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		MAP *brush = it->p1;
		AG_Window *win;
		AG_Toolbar *tbar;

		if (!it->selected)
			continue;

		if ((win = AG_WindowNewNamed(0, "mapedit-tool-merge-%s",
		    OBJECT(brush)->name)) == NULL)
			continue;

		tbar = AG_ToolbarNew(win, AG_TOOLBAR_HORIZ, 1, 0);
		MAP_ViewNew(win, brush, MAP_VIEW_EDIT|MAP_VIEW_GRID|
		                        MAPVIEW_PROPS, tbar, NULL);
		AG_WindowAttach(pwin, win);
		AG_WindowShow(win);
	}
}

static void
RemoveBrush(AG_Event *_Nonnull event)
{
	AG_TlistItem *it, *nit;

	for (it = TAILQ_FIRST(&brushes_tl->items);
	     it != TAILQ_END(&brushes_tl->items);
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
			AG_TlistDel(brushes_tl, it);
			AG_ObjectDestroy(brush);
		}
	}
}

static void
Interpolate(MAP *_Nonnull sm, MAP_Node *_Nonnull sn, MAP_Item *_Nonnull sr,
    MAP *_Nonnull dm, MAP_Node *_Nonnull dn, MAP_Item *_Nonnull dr)
{
	/* TODO */
}

static int
Effect(MAP_Tool *_Nonnull t, MAP_Node *_Nonnull n)
{
	MAP_View *mv = t->mv;
	MAP *m = mv->map;
	AG_TlistItem *it;
	
	/* Avoid circular references. XXX ugly */
	if (strncmp(OBJECT(m)->name, "brush(", 6) == 0)
		return (1);
	
	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		MAP *sm;
		int sx, sy, dx, dy;

		if (!it->selected)
			continue;

		sm = it->p1;
		for (sy = 0, dy = mv->cy;
		     sy < sm->maph && dy < m->maph;
		     sy++, dy++) {
			for (sx = 0, dx = mv->cx;
			     sx < sm->mapw && dx < m->mapw;
			     sx++, dx++) {
				MAP_Node *sn = &sm->map[sy][sx];
				MAP_Node *dn = &m->map[dy][dx];
				MAP_Item *sr, *dr;

				TAILQ_FOREACH(sr, &sn->nrefs, nrefs) {
					TAILQ_FOREACH(dr, &dn->nrefs, nrefs) {
						Interpolate(sm,sn,sr, m,dn,dr);
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
	Uint32 i, nbrushes;
	
	if (AG_ReadVersion(buf, &merge_ver, NULL) != 0)
		return (-1);
	FreeBrushes();
	AG_TlistClear(brushes_tl);

	nbrushes = AG_ReadUint32(buf);
	for (i = 0; i < nbrushes; i++) {
		char m_name[AG_OBJECT_NAME_MAX];
		MAP *nbrush;

		AG_CopyString(m_name, buf, sizeof(m_name));
		nbrush = Malloc(sizeof(MAP));
		MAP_Init(nbrush, m_name);
		map_load(nbrush, buf);

		TAILQ_INSERT_HEAD(&brushes, OBJECT(nbrush), cobjs);
		AG_TlistAddPtrHead(brushes_tl, NULL, m_name, nbrush);
	}
	return (0);
}

static int
Save(MAP_Tool *_Nonnull t, AG_DataSource *_Nonnull buf)
{
	AG_Object *ob;
	Uint32 nbrushes = 0;
	off_t count_offs;

	AG_WriteVersion(buf, &merge_ver);
	count_offs = AG_Tell(buf);			/* Skip count */
	AG_WriteUint32(buf, 0);	
	TAILQ_FOREACH(ob, &brushes, cobjs) {
		struct brush *brush = (struct brush *)ob;

		AG_WriteString(buf, ob->name);
		map_save(brush, buf);
		nbrushes++;
	}
	AG_WriteUint32At(buf, nbrushes, count_offs);	/* Write count */
	return (0);
}
#endif

static int
Cursor(MAP_Tool *_Nonnull t, AG_Rect *_Nonnull rd)
{
	MAP_View *mv = t->mv;
	MAP_Item *r;
	MAP *sm;
	int sx, sy, dx, dy;
	AG_TlistItem *it;
	int rv = -1;
	
	/* XXX ugly work around circular ref */
	if (strncmp(OBJECT(mv->map)->name, "brush(", 6) == 0)
		return (-1);

	TAILQ_FOREACH(it, &brushes_tl->items, items) {
		if (!it->selected) {
			continue;
		}
		sm = it->p1;
		for (sy = 0, dy = rd->y;
		     sy < sm->maph;
		     sy++, dy += AGMTILESZ(mv)) {
			for (sx = 0, dx = rd->x;
			     sx < sm->mapw;
			     sx++, dx += AGMTILESZ(mv)) {
				MAP_Node *sn = &sm->map[sy][sx];

				TAILQ_FOREACH(r, &sn->nrefs, nrefs) {
					MAP_ItemDraw(mv->map, r,
					    WIDGET(mv)->rView.x1 + dx,
					    WIDGET(mv)->rView.y1 + dy,
					    mv->cam);
					rv = 0;
				}
				if (mv->flags & MAPVIEW_PROPS) {
					MAP_ViewDraw_props(mv, sn, dx, dy,
					    -1, -1);
				}
			}
		}
	}
	return (rv);
}

static void
Init(MAP_Tool *_Nonnull t)
{
	AG_Window *win;
	AG_Box *hb;
	AG_Button *bu;
	AG_Textbox *tb;

	if ((win = MAP_ToolWindow(t, "mapedit-tool-merge")) == NULL)
		return;

	hb = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		tb = AG_TextboxNewS(hb, AG_TEXTBOX_FOCUS, _("Name: "));
		AG_SetEvent(tb, "textbox-return", CreateBrush, "%p", tb);

		bu = AG_ButtonNew(hb, 0, _("Create"));
		AG_ButtonSetPadding(bu, 6);			/* Align */
		AG_ButtonSetFocusable(bu, 0);
		AG_SetEvent(bu, "button-pushed", CreateBrush, "%p", tb);
	}

	hb = AG_BoxNewHoriz(win, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	{
#if 0
		bu = AG_ButtonNew(hb, 0, _("Load set"));
		AG_SetEvent(bu, "button-pushed", merge_load_brush_set, NULL);
		bu = AG_ButtonNew(hb, 0, _("Save set"));
		AG_SetEvent(bu, "button-pushed", merge_save_brush_set, NULL);
#endif
		bu = AG_ButtonNew(hb, 0, _("Edit"));
		AG_SetEvent(bu, "button-pushed", EditBrush, "%p", win);
		bu = AG_ButtonNew(hb, 0, _("Remove"));
		AG_SetEvent(bu, "button-pushed", RemoveBrush, NULL);
	}

	/* XXX */
	brushes_tl = AG_TlistNew(win, AG_TLIST_MULTI | AG_TLIST_EXPAND);
}


const MAP_Tool mapMergeOps = {
	"Merge",
	N_("Merge Pattern"),
	&mapIconMerge,
	sizeof(MAP_Tool),
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
