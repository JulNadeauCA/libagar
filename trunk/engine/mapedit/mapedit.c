/*	$Csoft: mapedit.c,v 1.180 2003/07/01 04:56:07 vedge Exp $	*/

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
#include <engine/widget/hbox.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>

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

/* Create the toolbar. */
static struct window *
toolbar_window(struct window *tilesets_win)
{
	struct window *win;
	struct hbox *hb;
	struct mapedit *med = &mapedit;

	win = window_new("mapedit-toolbar");
	window_set_caption(win, "Tools");
	window_set_closure(win, WINDOW_HIDE);
	window_set_position(win, WINDOW_UPPER_LEFT, 0);

	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL|HBOX_HFILL);
	hbox_set_spacing(hb, 0);
	{
		struct button *button;
		int i;
		
		button = button_new(hb, NULL);
		button_set_label(button, SPRITE(med, MAPEDIT_TOOL_OBJLIST));
		event_new(button, "button-pushed", window_generic_show,
		    "%p", tilesets_win);

		for (i = 0; i < MAPEDIT_NTOOLS; i++) {
			button = med->tools[i]->button = button_new(hb, NULL);
			button_set_label(button, med->tools[i]->icon);
			button_set_focusable(button, 0);
			button_set_sticky(button, 1);
			event_new(button, "button-pushed", mapedit_select_tool,
			    "%p", med->tools[i]);
		}
	}
	return (win);
}

/* Initialize the edition facilities. */
void
mapedit_init(void)
{
	struct window *toolbar_win, *tilesets_win, *objedit_win;
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
	
	for (i = 0; i < ntools; i++) {
		const struct tools_ent *toolent = &tools[i];

		*toolent->p = Malloc(toolent->size);
		toolent->init(*toolent->p);
	}
	if (object_load(&mapedit) == -1) {
		dprintf("loading mapedit: %s\n", error_get());
	}

	tilesets_win = tilesets_window();
	toolbar_win = toolbar_window(tilesets_win);
	objedit_win = objedit_window();

	window_show(toolbar_win);
	window_show(tilesets_win);
	window_show(objedit_win);
}

static void
mapedit_destroy(void *p)
{
	struct mapedit *med = p;
	int i;

	map_destroy(&med->copybuf);

	for (i = 0; i < MAPEDIT_NTOOLS; i++) {
		object_destroy(med->tools[i]);
		free(med->tools[i]);
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

