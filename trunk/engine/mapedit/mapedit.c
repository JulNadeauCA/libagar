/*	$Csoft: mapedit.c,v 1.181 2003/07/08 00:34:54 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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
#include <engine/map.h>
#include <engine/view.h>
#include <engine/prop.h>

#include <stdlib.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/button.h>

#include "mapedit.h"
#include "mapview.h"

#include "tool/tool.h"
#include "tool/stamp.h"
#include "tool/eraser.h"
#include "tool/magnifier.h"
#include "tool/resize.h"
#include "tool/propedit.h"
#include "tool/select.h"
#include "tool/shift.h"
#include "tool/merge.h"
#include "tool/fill.h"
#include "tool/flip.h"

static void	mapedit_destroy(void *);

const struct object_ops mapedit_ops = {
	NULL,			/* init */
	NULL,			/* reinit */
	mapedit_destroy,
	mapedit_load,
	mapedit_save,
	NULL			/* edit */
};

struct mapedit	mapedit;
int		mapedition = 0;

static const struct tools_ent {
	struct tool	**p;
	size_t		  size;
	void		(*init)(void *);
} tools[] = {
	{ &mapedit.tools[MAPEDIT_STAMP], sizeof(struct stamp), stamp_init },
	{ &mapedit.tools[MAPEDIT_ERASER], sizeof(struct eraser), eraser_init },
	{ &mapedit.tools[MAPEDIT_MAGNIFIER], sizeof(struct magnifier),
	    magnifier_init },
	{ &mapedit.tools[MAPEDIT_RESIZE], sizeof(struct resize), resize_init },
	{ &mapedit.tools[MAPEDIT_PROPEDIT], sizeof(struct propedit),
	    propedit_init },
	{ &mapedit.tools[MAPEDIT_SELECT], sizeof(struct select), select_init },
	{ &mapedit.tools[MAPEDIT_SHIFT], sizeof(struct shift), shift_init },
	{ &mapedit.tools[MAPEDIT_MERGE], sizeof(struct merge), merge_init },
	{ &mapedit.tools[MAPEDIT_FILL], sizeof(struct fill), fill_init },
	{ &mapedit.tools[MAPEDIT_FLIP], sizeof(struct flip), flip_init }
};
static const int ntools = sizeof(tools) / sizeof(tools[0]);

/* Select a map edition tool. */
static void
mapedit_select_tool(int argc, union evarg *argv)
{
	struct tool *newtool = argv[1].p;

	if (mapedit.curtool == newtool) {
		if (newtool->win != NULL) {
			window_hide(newtool->win);
		}
		mapedit.curtool = NULL;
		return;
	}

	if (mapedit.curtool != NULL) {
		widget_set_bool(mapedit.curtool->button, "state", 0);
		if (mapedit.curtool->win != NULL)
			window_hide(mapedit.curtool->win);
	}

	mapedit.curtool = newtool;

	if (newtool->win != NULL)
		window_show(newtool->win);
}

/* Initialize the edition facilities. */
void
mapedit_init(void)
{
	struct window *win, *tilesets_win, *objedit_win;
	struct box *bo;
	int i;
	
	object_init(&mapedit, "object", "map-editor", &mapedit_ops);
	OBJECT(&mapedit)->flags |= OBJECT_RELOAD_PROPS;
	OBJECT(&mapedit)->save_pfx = NULL;
	if (gfx_fetch(&mapedit, "/engine/mapedit/mapedit") == -1) {
		fatal("%s", error_get());
	}
	gfx_wire(OBJECT(&mapedit)->gfx);

	map_init(&mapedit.copybuf, "copybuf");
	mapedit.curtool = NULL;
	mapedit.src_node = NULL;
	TAILQ_INIT(&mapedit.tilesets);
	TAILQ_INIT(&mapedit.dobjs);
	TAILQ_INIT(&mapedit.gobjs);

	/* Attach a pseudo-object for dependency keeping purposes. */
	mapedit.pseudo = object_new(world, "map-editor");
	OBJECT(mapedit.pseudo)->flags |= (OBJECT_NON_PERSISTENT|
	                                  OBJECT_INDESTRUCTIBLE);

	mapedition = 1;
 
	prop_set_uint32(&mapedit, "default-map-width", 64);
	prop_set_uint32(&mapedit, "default-map-height", 32);
	prop_set_uint32(&mapedit, "default-brush-width", 5);
	prop_set_uint32(&mapedit, "default-brush-height", 5);
	prop_set_bool(&mapedit, "sel-bounded-edition", 0);

	/* Initialize the tools, try to load saved settings. */
	for (i = 0; i < ntools; i++) {
		const struct tools_ent *toolent = &tools[i];

		*toolent->p = Malloc(toolent->size);
		toolent->init(*toolent->p);
	}
	if (object_load(&mapedit) == -1) {
		dprintf("loading mapedit: %s\n", error_get());
	}

	tilesets_win = tilesets_window();
	objedit_win = objedit_window();

	window_show(tilesets_win);
	window_show(objedit_win);

	/* Create the toolbar window. */
	win = window_new("mapedit-toolbar");
	window_set_caption(win, "Tools");
	window_set_closure(win, WINDOW_HIDE);
	window_set_position(win, WINDOW_UPPER_LEFT, 0);

	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL|BOX_HFILL);
	box_set_spacing(bo, 0);
	{
		struct button *button;
		int i;
		
		button = button_new(bo, NULL);
		button_set_label(button,
		    SPRITE(&mapedit, MAPEDIT_TOOL_TILESETS));
		event_new(button, "button-pushed", window_generic_show, "%p",
		    tilesets_win);
		
		button = button_new(bo, NULL);
		button_set_label(button,
		    SPRITE(&mapedit, MAPEDIT_TOOL_OBJEDITOR));
		event_new(button, "button-pushed", window_generic_show, "%p",
		    objedit_win);

		for (i = 0; i < MAPEDIT_NTOOLS; i++) {
			button = mapedit.tools[i]->button =
			    button_new(bo, NULL);
			button_set_label(button, mapedit.tools[i]->icon);
			button_set_focusable(button, 0);
			button_set_sticky(button, 1);
			event_new(button, "button-pushed", mapedit_select_tool,
			    "%p", mapedit.tools[i]);
		}
	}
	window_show(win);
}

static void
mapedit_destroy(void *p)
{
	struct mapedit_tileset *tset, *ntset;
	struct mapedit_obj *mobj, *nmobj;
	int i;

	map_destroy(&mapedit.copybuf);

	for (tset = TAILQ_FIRST(&mapedit.tilesets);
	     tset != TAILQ_END(&mapedit.tilesets);
	     tset = ntset) {
		ntset = TAILQ_NEXT(tset, tilesets);
		free(tset);
	}
	for (mobj = TAILQ_FIRST(&mapedit.dobjs);
	     mobj != TAILQ_END(&mapedit.dobjs);
	     mobj = nmobj) {
		nmobj = TAILQ_NEXT(mobj, objs);
		free(mobj);
	}
	for (mobj = TAILQ_FIRST(&mapedit.gobjs);
	     mobj != TAILQ_END(&mapedit.gobjs);
	     mobj = nmobj) {
		nmobj = TAILQ_NEXT(mobj, objs);
		free(mobj);
	}

	for (i = 0; i < MAPEDIT_NTOOLS; i++) {
		object_destroy(mapedit.tools[i]);
		free(mapedit.tools[i]);
	}
}

int
mapedit_load(void *p, struct netbuf *buf)
{
	int i;

	for (i = 0; i < MAPEDIT_NTOOLS; i++) {
		struct tool *tool = mapedit.tools[i];

		if (OBJECT(tool)->ops->load != NULL &&
		    object_load(tool) == -1) {
			text_msg(MSG_ERROR, "%s", error_get());
		}
	}
	return (0);
}

int
mapedit_save(void *p, struct netbuf *buf)
{
	int i;

	for (i = 0; i < MAPEDIT_NTOOLS; i++) {
		struct tool *tool = mapedit.tools[i];

		if (OBJECT(tool)->ops->save != NULL &&
		    object_save(mapedit.tools[i]) == -1) {
			text_msg(MSG_ERROR, "%s", error_get());
		}
	}
	return (0);
}

static void
mapedit_close_objdata(int argc, union evarg *argv)
{
	struct mapedit_obj *mobj = argv[1].p;

	view_detach(mobj->win);
	TAILQ_REMOVE(&mapedit.dobjs, mobj, objs);
	object_del_dep(mapedit.pseudo, mobj->obj);
	free(mobj);
}

/* Edit object derivate data, such as map nodes. */
void
mapedit_edit_objdata(struct object *ob)
{
	struct mapedit_obj *mobj;
	
	TAILQ_FOREACH(mobj, &mapedit.dobjs, objs) {
		if (mobj->obj == ob)
			break;
	}
	if (mobj != NULL) {
		window_show(mobj->win);
		return;
	}
	
	object_add_dep(mapedit.pseudo, ob);
	
	mobj = Malloc(sizeof(struct mapedit_obj));
	mobj->obj = ob;
	mobj->win = ob->ops->edit(ob);
	TAILQ_INSERT_HEAD(&mapedit.dobjs, mobj, objs);
	window_show(mobj->win);
	event_new(mobj->win, "window-close", mapedit_close_objdata, "%p", mobj);
}

static void
mapedit_close_objgen(int argc, union evarg *argv)
{
	struct mapedit_obj *mobj = argv[1].p;

	view_detach(mobj->win);
	TAILQ_REMOVE(&mapedit.gobjs, mobj, objs);
	object_del_dep(mapedit.pseudo, mobj->obj);
	free(mobj);
}

/* Edit object generic data. */
void
mapedit_edit_objgen(struct object *ob)
{
	struct mapedit_obj *mobj;
	
	TAILQ_FOREACH(mobj, &mapedit.gobjs, objs) {
		if (mobj->obj == ob)
			break;
	}
	if (mobj != NULL) {
		window_show(mobj->win);
		return;
	}
	
	object_add_dep(mapedit.pseudo, ob);
	
	mobj = Malloc(sizeof(struct mapedit_obj));
	mobj->obj = ob;
	mobj->win = object_edit(ob);
	TAILQ_INSERT_HEAD(&mapedit.gobjs, mobj, objs);
	window_show(mobj->win);

	event_new(mobj->win, "window-close", mapedit_close_objgen, "%p", mobj);
}

