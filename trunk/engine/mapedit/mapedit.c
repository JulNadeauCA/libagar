/*	$Csoft: mapedit.c,v 1.194 2004/03/17 12:42:06 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/button.h>

#include <string.h>

#include "mapedit.h"
#include "mapview.h"

extern struct tool stamp_tool;
extern struct tool eraser_tool;
extern struct tool magnifier_tool;
extern struct tool resize_tool;
extern struct tool position_tool;
extern struct tool propedit_tool;
extern struct tool select_tool;
extern struct tool shift_tool;
extern struct tool merge_tool;
extern struct tool fill_tool;
extern struct tool flip_tool;
extern struct tool invert_tool;

struct tool *mapedit_tools[] = {
	&stamp_tool,
	&eraser_tool,
	&magnifier_tool,
	&resize_tool,
	&position_tool,
	&propedit_tool,
	&select_tool,
	&shift_tool,
	&merge_tool,
	&fill_tool,
	&flip_tool,
	&invert_tool
};
const int mapedit_ntools = sizeof(mapedit_tools) / sizeof(mapedit_tools[0]);

const struct version mapedit_ver = {
	"agar map editor",
	0, 0
};

const struct object_ops mapedit_ops = {
	NULL,				/* init */
	NULL,				/* reinit */
	mapedit_destroy,
	mapedit_load,
	mapedit_save,
	NULL				/* edit */
};

struct mapedit mapedit;
int mapedition = 0;			/* Start up in edition mode */

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
		widget_set_bool(mapedit.curtool->trigger, "state", 0);
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
	struct window *win, *objedit_win;
	struct box *bo;
	int i;
	
	object_init(&mapedit, "object", "map-editor", &mapedit_ops);
	object_wire_gfx(&mapedit, "/engine/mapedit/mapedit");
	OBJECT(&mapedit)->flags |= (OBJECT_RELOAD_PROPS|OBJECT_STATIC);
	OBJECT(&mapedit)->save_pfx = "/map-editor";
	mapedit.curtool = NULL;

	/* Attach a pseudo-object for dependency keeping purposes. */
	object_init(&mapedit.pseudo, "object", "map-editor", NULL);
	OBJECT(&mapedit.pseudo)->flags |= (OBJECT_NON_PERSISTENT|OBJECT_STATIC|
	                                   OBJECT_INDESTRUCTIBLE);
	object_attach(world, &mapedit.pseudo);

	/*
	 * Allocate the copy/paste buffer.
	 * Use OBJECT_READONLY to avoid circular reference in case a user
	 * attempts to paste contents of the copy buffer into itself.
	 */
	map_init(&mapedit.copybuf, "copybuf");
	OBJECT(&mapedit.copybuf)->flags |= (OBJECT_NON_PERSISTENT|OBJECT_STATIC|
	                                    OBJECT_INDESTRUCTIBLE|
					    OBJECT_READONLY);
	object_attach(&mapedit.pseudo, &mapedit.copybuf);

	mapedition = 1;

	/* Initialize the default tunables. */
	prop_set_uint32(&mapedit, "default-map-width", 64);
	prop_set_uint32(&mapedit, "default-map-height", 32);
	prop_set_uint32(&mapedit, "default-brush-width", 5);
	prop_set_uint32(&mapedit, "default-brush-height", 5);
	prop_set_bool(&mapedit, "sel-bounded-edition", 0);

	/* Initialize the map editor tools. */
	for (i = 0; i < mapedit_ntools; i++) {
		struct tool *tool = mapedit_tools[i];

		tool->win = NULL;
		tool->cursor_su = tool->cursor_index >= 0 ?
		    SPRITE(&mapedit, tool->cursor_index) : NULL;
		SLIST_INIT(&tool->kbindings);
	
		if (tool->init != NULL)
			tool->init();
	}

	/* Try to restore the persistent tool states. */
	object_load(&mapedit);

	/* Create the object editor window. */
	objedit_init();
	objedit_win = objedit_window();
	window_show(objedit_win);

	/* Create the toolbar window. */
	win = window_new("mapedit-toolbar");
	window_set_caption(win, _("Tools"));
	window_set_closure(win, WINDOW_HIDE);
	window_set_position(win, WINDOW_UPPER_LEFT, 0);

	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL|BOX_HFILL);
	box_set_spacing(bo, 0);
	{
		struct button *button;
		int i;
		
		button = button_new(bo, NULL);
		button_set_label(button,
		    SPRITE(&mapedit, MAPEDIT_TOOL_OBJEDITOR));
		event_new(button, "button-pushed", window_generic_show, "%p",
		    objedit_win);

		for (i = 0; i < mapedit_ntools; i++) {
			button = mapedit_tools[i]->trigger =
			    button_new(bo, NULL);

			if (mapedit_tools[i]->icon >= 0) {
				button_set_label(button,
				    SPRITE(&mapedit, mapedit_tools[i]->icon));
			} else {
				button_printf(button, "%s",
				    _(mapedit_tools[i]->name));
			}
			button_set_focusable(button, 0);
			button_set_sticky(button, 1);
			event_new(button, "button-pushed", mapedit_select_tool,
			    "%p", mapedit_tools[i]);
		}
	}
	window_show(win);
}

void
mapedit_destroy(void *p)
{
	int i;

	for (i = 0; i < mapedit_ntools; i++) {
		struct tool *tool = mapedit_tools[i];
		struct tool_kbinding *kbinding, *nkbinding;

		for (kbinding = SLIST_FIRST(&tool->kbindings);
		     kbinding != SLIST_END(&tool->kbindings);
		     kbinding = nkbinding) {
			nkbinding = SLIST_NEXT(kbinding, kbindings);
			Free(kbinding, M_MAPEDIT);
		}
		if (tool->destroy != NULL)
			tool->destroy();
	}

	map_destroy(&mapedit.copybuf);
	objedit_destroy();
}

int
mapedit_load(void *p, struct netbuf *buf)
{
	int i;
	
	if (version_read(buf, &mapedit_ver, NULL) != 0)
		return (-1);

	for (i = 0; i < mapedit_ntools; i++) {
		struct tool *tool = mapedit_tools[i];

		if (tool->load != NULL &&
		    tool->load(buf) == -1)
			text_msg(MSG_ERROR, "%s", error_get());
	}
	return (0);
}

int
mapedit_save(void *p, struct netbuf *buf)
{
	int i;
	
	version_write(buf, &mapedit_ver);

	for (i = 0; i < mapedit_ntools; i++) {
		struct tool *tool = mapedit_tools[i];

		if (tool->save != NULL &&
		    tool->save(buf) == -1)
			text_msg(MSG_ERROR, "%s", error_get());
	}
	return (0);
}

static void
close_tool_window(int argc, union evarg *argv)
{
	struct tool *tool = argv[1].p;

	widget_set_int(tool->trigger, "state", 0);
	mapedit.curtool = NULL;
	window_hide(tool->win);
}

/* Create a tool settings window. */
struct window *
tool_window_new(struct tool *tool, const char *name)
{
	struct window *win;
	
	win = tool->win = window_new(name);
	window_set_caption(win, _(tool->name));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
	
	event_new(win, "window-close", close_tool_window, "%p", tool);
	return (win);
}

/* Return the first focused mapview(3) widget found. */
static struct mapview *
find_mapview(struct widget *pwid)
{
	struct widget *cwid;
	
	if (strcmp(pwid->type, "mapview") == 0)
		return ((struct mapview *)pwid);

	OBJECT_FOREACH_CHILD(cwid, pwid, widget) {
		struct mapview *mv;

		if (!widget_holds_focus(cwid))
			continue;
		if ((mv = find_mapview(cwid)) != NULL)
			return (mv);
	}
	return (NULL);
}

/* Return the first visible mapview widget. */
struct mapview *
tool_mapview(void)
{
	struct window *win;
	struct mapview *mv;

	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		if (!win->visible)
			continue;
		if ((mv = find_mapview(WIDGET(win))) != NULL)
			return (mv);
	}
	return (NULL);
}

/* Create a new key binding for purposes of map edition. */
void
tool_bind_key(void *p, SDLMod keymod, SDLKey keysym,
    void (*func)(struct mapview *), int edit)
{
	struct tool *tool = p;
	struct tool_kbinding *kbinding;

	kbinding = Malloc(sizeof(struct tool_kbinding), M_MAPEDIT);
	kbinding->key = keysym;
	kbinding->mod = keymod;
	kbinding->func = func;
	kbinding->edit = edit;
	SLIST_INSERT_HEAD(&tool->kbindings, kbinding, kbindings);
}

