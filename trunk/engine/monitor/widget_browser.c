/*	$Csoft: widget_browser.c,v 1.44 2005/05/24 08:15:09 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <config/debug.h>

#ifdef DEBUG

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/map/mapview.h>

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/textbox.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/spinbutton.h>

#include "monitor.h"

static void
poll_windows_do(AG_Tlist *tl, AG_Window *win, int depth)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_Window *subwin;
	AG_TlistItem *it;

	if (strcmp(win->caption, "win-popup") == 0)
		return;

	strlcpy(text, win->caption, sizeof(text));
	if (strcmp(AGOBJECT(win)->name, "win-generic") != 0) {
		strlcat(text, " (", sizeof(text));
		strlcat(text, AGOBJECT(win)->name, sizeof(text));
		strlcat(text, ")", sizeof(text));
	}

	it = AG_TlistAddPtr(tl, NULL, text, win);
	it->depth = depth;
	it->class = "window";

	TAILQ_FOREACH(subwin, &win->subwins, swins)
		poll_windows_do(tl, subwin, depth+1);
}

static void
poll_windows(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_Window *win;

	AG_TlistClear(tl);
	pthread_mutex_lock(&agView->lock);
	TAILQ_FOREACH_REVERSE(win, &agView->windows, windows, ag_windowq) {
		poll_windows_do(tl, win, 0);
	}
	pthread_mutex_unlock(&agView->lock);
	AG_TlistRestore(tl);
}

static void
show_window(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;
	AG_TlistItem *it;
	AG_Window *win;

	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;

	win = it->p1;
	AG_WindowShow(win);
}

static void
hide_window(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;
	AG_TlistItem *it;
	AG_Window *win;

	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;

	win = it->p1;
	AG_WindowHide(win);
}

/* Display widget information. */
static void
examine_widget(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[1].p;
	AG_Window *pwin = argv[2].p;
	AG_TlistItem *it;
	AG_Widget *wid;
	AG_Window *win;
	AG_VBox *vb;

	if ((it = AG_TlistSelectedItem(tl)) == NULL) {
		return;
	}
	wid = it->p1;

	win = AG_WindowNew(0, NULL);
	AG_WindowSetCaption(win, _("Widget info: %s"), AGOBJECT(wid)->name);
	
	vb = AG_VBoxNew(win, AG_VBOX_WFILL);
	{
		AG_Label *lab;

		AG_LabelNew(vb, AG_LABEL_STATIC, _("Name: \"%s\""),
		    AGOBJECT(wid)->name);
		AG_LabelNew(vb, AG_LABEL_STATIC, _("Type: %s"), wid->type);
		AG_LabelNew(vb, AG_LABEL_POLLED_MT, _("Flags: 0x%x"),
		    &pwin->lock, &wid->flags);

		lab = AG_LabelNew(vb, AG_LABEL_POLLED_MT,
		    _("Geometry: %dx%d at %d,%d"), &pwin->lock,
		    &wid->w, &wid->h, &wid->x, &wid->y);
		AG_LabelPrescale(lab,
		    _("Geometry: 0000x0000 at 0000x0000"));
		
		lab = AG_LabelNew(vb, AG_LABEL_POLLED_MT,
		    _("View geometry: %d,%d -> %d,%d"), &pwin->lock,
		    &wid->cx, &wid->cy, &wid->cx2, &wid->cy2);
		AG_LabelPrescale(lab,
		    _("View geometry: 0000x0000 -> 0000x0000"));
	}
	AG_WindowShow(win);
}

static void
poll_widgets_do(AG_Widget *wid, AG_Tlist *widtl, int depth)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_TlistItem *it;

	strlcpy(text, AGOBJECT(wid)->name, sizeof(text));
	if (strcmp(wid->type, "window") == 0) {
		AG_Window *win = (AG_Window *)wid;

		strlcat(text, " (", sizeof(text));
		strlcat(text, win->caption, sizeof(text));
		strlcat(text, ")", sizeof(text));
	}
	it = AG_TlistAddPtr(widtl, NULL, text, wid);
	it->depth = depth;
	it->class = "widget";
	
	if (!TAILQ_EMPTY(&AGOBJECT(wid)->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}

	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(widtl, it)) {
		AG_Widget *cwid;

		AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
			poll_widgets_do(cwid, widtl, depth+1);
	}
}

static void
poll_widgets(int argc, union evarg *argv)
{
	AG_Tlist *widtl = argv[0].p;
	AG_Window *win = argv[1].p;
	
	AG_TlistClear(widtl);
	AG_LockLinkage();
	pthread_mutex_lock(&win->lock);

	poll_widgets_do(AGWIDGET(win), widtl, 0);

	pthread_mutex_unlock(&win->lock);
	AG_UnlockLinkage();
	AG_TlistRestore(widtl);
}

static void
scale_window(int argc, union evarg *argv)
{
	AG_Window *win = argv[1].p;

	AG_WindowScale(win, -1, -1);
	AG_WindowScale(win, AGWIDGET(win)->w, AGWIDGET(win)->h);
	AG_WINDOW_UPDATE(win);
}

static void
examine_window(int argc, union evarg *argv)
{
	AG_Tlist *wintl = argv[1].p;
	AG_TlistItem *it;
	AG_Window *pwin, *win;
	AG_Tlist *tl;
	AG_MenuItem *mi;
	AG_Spinbutton *sb;

	if ((it = AG_TlistSelectedItem(wintl)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("No window is selected."));
		return;
	}
	pwin = it->p1;

	if ((win = AG_WindowNew(AG_WINDOW_DETACH, "monitor-win-%s",
	    AGOBJECT(pwin)->name)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "%s", AGOBJECT(pwin)->name);

	AG_LabelNew(win, AG_LABEL_STATIC, "Name: \"%s\"", AGOBJECT(pwin)->name);
	AG_LabelNew(win, AG_LABEL_POLLED_MT, "Flags: 0x%x", &pwin->lock,
	    &pwin->flags);

	sb = AG_SpinbuttonNew(win, _("Widget spacing: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &pwin->spacing);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SetEvent(sb, "spinbutton-changed", scale_window, "%p", pwin);
	
	sb = AG_SpinbuttonNew(win, _("Top padding: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &pwin->ypadding_top);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SetEvent(sb, "spinbutton-changed", scale_window, "%p", pwin);
	
	sb = AG_SpinbuttonNew(win, _("Bottom padding: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &pwin->ypadding_bot);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SetEvent(sb, "spinbutton-changed", scale_window, "%p", pwin);
	
	sb = AG_SpinbuttonNew(win, _("Horizontal padding: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &pwin->xpadding);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SetEvent(sb, "spinbutton-changed", scale_window, "%p", pwin);

	tl = AG_TlistNew(win, AG_TLIST_TREE|AG_TLIST_POLL);
	AG_SetEvent(tl, "tlist-poll", poll_widgets, "%p", pwin);
	AG_SetEvent(tl, "tlist-dblclick", examine_widget, "%p,%p", tl, pwin);

	mi = AG_TlistSetPopup(tl, "widget");
	{
		AG_MenuAction(mi, _("Display informations..."), -1,
		    examine_widget, "%p,%p", tl, pwin);
	}
	AG_WindowShow(win);
}

AG_Window *
AG_DebugWidgetBrowser(void)
{
	AG_Window *win;
	AG_Tlist *tl;
	AG_MenuItem *it;

	if ((win = AG_WindowNew(AG_WINDOW_DETACH, "monitor-window-stack"))
	    == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Window stack"));

	tl = AG_TlistNew(win, AG_TLIST_POLL);
	AG_SetEvent(tl, "tlist-poll", poll_windows, NULL);
	AG_SetEvent(tl, "tlist-dblclick", examine_window, "%p", tl);

	it = AG_TlistSetPopup(tl, "window");
	{
		AG_MenuAction(it, _("Display informations..."), -1,
		    examine_window, "%p", tl);
		AG_MenuSeparator(it);
		AG_MenuAction(it, _("Show this window"), -1,
		    show_window, "%p", tl);
		AG_MenuAction(it, _("Hide this window"), -1,
		    hide_window, "%p", tl);
	}
	AG_WindowSetGeometry(win, agView->w/4, agView->h/3, agView->w/2,
	    agView->h/4);
	return (win);
}

#endif	/* DEBUG */
