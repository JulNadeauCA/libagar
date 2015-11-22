/*
 * Copyright (c) 2001-2015 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Implementation of the AG_Widget(3) object.
 */

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/gui/window.h>
#include <agar/gui/cursors.h>
#include <agar/gui/primitive.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/opengl.h>
#include <agar/gui/text_cache.h>

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

const char *agWidgetPropNames[] = {
	"font-family",
	"font-size",
	"font-weight",
	"font-style",
	"color",
	"text-color",
	"line-color",
	"shape-color",
	"border-color",
	NULL
};
const char *agWidgetStateNames[] = {
	"",
	"#disabled",
	"#focused",
	"#hover",
	"#selected",
	NULL
};
const char *agWidgetColorNames[] = {
	"color",
	"text-color",
	"line-color",
	"shape-color",
	"border-color",
	NULL
};
AG_WidgetPalette agDefaultPalette = {{
	/* "color"           "text-color"        "line-color"    "shape-color"	"border-color" */
/*def*/	{ {125,125,125,255}, {240,240,240,255},  {50,50,50,255}, {200,200,200,255},	{100,100,100,255} },
/*dis*/	{ {160,160,160,255}, {240,240,240,255},  {70,70,70,255}, {150,150,150,255},	{100,100,100,255} },
/*foc*/	{ {125,125,125,255}, {240,240,240,255},  {50,50,50,255}, {200,200,200,255},	{100,100,100,255} },
/*hov*/	{ {130,130,130,255}, {240,240,240,255},  {50,50,50,255}, {220,220,220,255},	{100,100,100,255} },
/*sel*/	{ {50,50,120,255},   {255,255,255,255},  {50,50,60,255}, {50,50,50,255},	{100,100,100,255} },
}};

/* Set the parent window/driver pointers on a widget and its children. */
static void
SetParentWindow(AG_Widget *wid, AG_Window *win)
{
	AG_Widget *chld;
	AG_CursorArea *ca, *caNext;
	
	wid->window = win;

	if (win != NULL) {
		wid->drv = (AG_Driver *)OBJECT(win)->parent;
		wid->drvOps = AGDRIVER_CLASS(wid->drv);

		/*
		 * Commit any previously deferred AG_MapStockCursor()
		 * operation.
		 */
		for (ca = TAILQ_FIRST(&wid->cursorAreas);
		     ca != TAILQ_END(&wid->cursorAreas);
		     ca = caNext) {
			caNext = TAILQ_NEXT(ca, cursorAreas);
			if (ca->stock >= 0 &&
			    ca->stock < wid->drv->nCursors) {
				AG_Cursor *ac;
				int i = 0;

				TAILQ_FOREACH(ac, &wid->drv->cursors, cursors) {
					if (i++ == ca->stock)
						break;
				}
				if (ac != NULL) {
					ca->c = ac;
					TAILQ_INSERT_TAIL(&win->cursorAreas,
					    ca, cursorAreas);
				} else {
					free(ca);
				}
			} else {
				free(ca);
			}
		}
		TAILQ_INIT(&wid->cursorAreas);
	} else {
		wid->drv = NULL;
		wid->drvOps = NULL;
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		SetParentWindow(chld, win);
}

/* Set the parent driver pointers on a widget and its children. */
static void
SetParentDriver(AG_Widget *wid, AG_Driver *drv)
{
	AG_Widget *chld;

	if (drv != NULL) {
		wid->drv = (AG_Driver *)drv;
		wid->drvOps = AGDRIVER_CLASS(drv);
	} else {
		wid->drv = NULL;
		wid->drvOps = NULL;
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		SetParentDriver(chld, drv);
}

static void
OnAttach(AG_Event *event)
{
	void *parent = AG_SENDER();
	AG_Widget *w = AG_SELF();

	if (AG_OfClass(parent, "AG_Widget:AG_Window:*") &&
	    AG_OfClass(w, "AG_Widget:*")) {
		AG_Widget *widParent = (AG_Widget *)parent;
		Uint i;

		Debug(w, "Attach to window (%s)\n", OBJECT(parent)->name);

		SetParentWindow(w, AGWINDOW(widParent));
		if (AGWINDOW(widParent)->visible) {
			w->flags |= AG_WIDGET_UPDATE_WINDOW;
			AG_PostEvent(NULL, w, "widget-shown", NULL);
		}

		/*
		 * Widget may have previously been detached from another
		 * driver; textures may need regenerating.
		 */
		for (i = 0; i < w->nsurfaces; i++) {
			w->textures[i] = 0;
		}
	} else if (AG_OfClass(parent, "AG_Widget:*") &&
	           AG_OfClass(w, "AG_Widget:*")) {
		AG_Widget *widParent = (AG_Widget *)parent;
		
		Debug(w, "Attach to widget (%s in %s)\n", OBJECT(parent)->name,
		    widParent->window != NULL ? OBJECT(widParent->window)->name : "NULL");

		SetParentWindow(w, widParent->window);
		if (widParent->window != NULL &&
		    widParent->window->visible) {
			AG_PostEvent(NULL, w, "widget-shown", NULL);
		}
	} else if (AG_OfClass(parent, "AG_Driver:*") &&
	           AG_OfClass(w, "AG_Widget:AG_Window:*")) {
		AG_Driver *drvParent = (AG_Driver *)parent;

		Debug(w, "Attach to driver (%s)\n", OBJECT(parent)->name);
		SetParentDriver(w, drvParent);
	} else {
		AG_FatalError("Inconsistent widget attach");
	}
}

static void
OnDetach(AG_Event *event)
{
	void *parent = AG_SENDER();
	AG_Widget *w = AG_SELF();
	
	if (AG_OfClass(parent, "AG_Widget:*") &&
	    AG_OfClass(w, "AG_Widget:*")) {
		if (w->window != NULL) {
			AG_UnmapAllCursors(w->window, w);
		}
		SetParentWindow(w, NULL);
	} else if (AG_OfClass(parent, "AG_Driver:*") &&
	           AG_OfClass(w, "AG_Widget:AG_Window:*")) {
		SetParentDriver(w, NULL);
	} else {
		AG_FatalError("Inconsistent widget detach");
	}
}

/* Timer callback for AG_RedrawOnTick(). */
static Uint32
RedrawOnTickTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Widget *wid = event->argv[0].data.p;

	if (wid->window != NULL) {
		wid->window->dirty = 1;
	}
	return (to->ival);
}

/* Timer callback for AG_RedrawOnChange(). */
static Uint32
RedrawOnChangeTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Widget *wid = AG_SELF();
	AG_RedrawTie *rt = AG_PTR(1);
	AG_Variable *V, Vd;
	void *p;

	if ((V = AG_GetVariable(wid, rt->name, &p)) == NULL) {
		return (to->ival);
	}
	AG_DerefVariable(&Vd, V);
	if (!rt->VlastInited || AG_CompareVariables(&Vd, &rt->Vlast) != 0) {
		if (wid->window != NULL) {
			wid->window->dirty = 1;
		}
		AG_CopyVariable(&rt->Vlast, &Vd);
		rt->VlastInited = 1;
	}
	AG_UnlockVariable(V);
	return (to->ival);
}

static void
OnShow(AG_Event *event)
{
	AG_Widget *wid = AG_SELF();
	AG_RedrawTie *rt;

	if (wid->font == NULL)
		AG_FatalError("%s style not compiled", OBJECT(wid)->name);

	wid->flags |= AG_WIDGET_VISIBLE;

	TAILQ_FOREACH(rt, &wid->redrawTies, redrawTies) {
		switch (rt->type) {
		case AG_REDRAW_ON_TICK:
			AG_AddTimer(wid, &rt->to, rt->ival,
			    RedrawOnTickTimeout, NULL);
			break;
		case AG_REDRAW_ON_CHANGE:
			AG_AddTimer(wid, &rt->to, rt->ival,
			    RedrawOnChangeTimeout, "%p", rt);
			break;
		}
	}
}

static void
OnHide(AG_Event *event)
{
	AG_Widget *wid = AG_SELF();
	AG_RedrawTie *rt;

	wid->flags &= ~(AG_WIDGET_VISIBLE);
	
	TAILQ_FOREACH(rt, &wid->redrawTies, redrawTies) {
		switch (rt->type) {
		case AG_REDRAW_ON_TICK:
		case AG_REDRAW_ON_CHANGE:
			AG_DelTimer(wid, &rt->to);
			break;
		}
	}
}

static void
Init(void *obj)
{
	AG_Widget *wid = obj;
	AG_Event *ev;

	OBJECT(wid)->save_pfx = "/widgets";
	OBJECT(wid)->flags |= AG_OBJECT_NAME_ONATTACH;

	wid->flags = 0;
	wid->rView = AG_RECT2(-1,-1,-1,-1);
	wid->rSens = AG_RECT2(0,0,0,0);
	wid->x = -1;
	wid->y = -1;
	wid->w = -1;
	wid->h = -1;
	wid->focusFwd = NULL;
	wid->window = NULL;
	wid->drv = NULL;
	wid->drvOps = NULL;
	wid->nsurfaces = 0;
	wid->surfaces = NULL;
	wid->surfaceFlags = NULL;
	wid->textures = NULL;
	wid->texcoords = NULL;
	AG_TblInit(&wid->actions, 32, 0);
	TAILQ_INIT(&wid->mouseActions);
	TAILQ_INIT(&wid->keyActions);
	
	wid->css = NULL;
	wid->cState = AG_DEFAULT_STATE;
	wid->font = agDefaultFont;
	wid->pal = agDefaultPalette;

	AG_SetEvent(wid, "attached", OnAttach, NULL);
	AG_SetEvent(wid, "detached", OnDetach, NULL);
	ev = AG_SetEvent(wid, "widget-shown", OnShow, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;
	ev = AG_SetEvent(wid, "widget-hidden", OnHide, NULL);
	ev->flags |= AG_EVENT_PROPAGATE;

	TAILQ_INIT(&wid->redrawTies);
	TAILQ_INIT(&wid->cursorAreas);
}

/* Arrange for a redraw whenever a given binding value changes. */
void
AG_RedrawOnChange(void *obj, int refresh_ms, const char *name)
{
	AG_Widget *wid = obj;
	AG_RedrawTie *rt;
	
	TAILQ_FOREACH(rt, &wid->redrawTies, redrawTies) {
		if (rt->type == AG_REDRAW_ON_CHANGE &&
		    strcmp(rt->name, name) == 0 &&
		    rt->ival == refresh_ms)
			break;
	}
	if (rt != NULL) {
		AG_ResetTimer(wid, &rt->to, refresh_ms);
		return;
	}
	
	rt = Malloc(sizeof(AG_RedrawTie));
	rt->type = AG_REDRAW_ON_CHANGE;
	rt->ival = refresh_ms;
	rt->VlastInited = 0;
	Strlcpy(rt->name, name, sizeof(rt->name));
	AG_InitTimer(&rt->to, "redrawTie-", 0);
#ifdef AG_DEBUG
	Strlcat(rt->to.name, name, sizeof(rt->to.name));
#endif
	TAILQ_INSERT_TAIL(&wid->redrawTies, rt, redrawTies);
	
	if (wid->flags & AG_WIDGET_VISIBLE) {
		AG_AddTimer(wid, &rt->to, rt->ival, RedrawOnChangeTimeout, "%p", rt);
	} else {
		/* Fire from OnShow() */
	}
}

/* Arrange for an unconditional redraw at a periodic interval. */
void
AG_RedrawOnTick(void *obj, int refresh_ms)
{
	AG_Widget *wid = obj;
	AG_RedrawTie *rt;

	if (refresh_ms == -1) {
		TAILQ_FOREACH(rt, &wid->redrawTies, redrawTies) {
			if (rt->type == AG_REDRAW_ON_TICK)
				break;
		}
		if (rt != NULL) {
			TAILQ_REMOVE(&wid->redrawTies, rt, redrawTies);
			AG_DelTimer(wid, &rt->to);
			free(rt);
		}
		return;
	}

	rt = Malloc(sizeof(AG_RedrawTie));
	rt->type = AG_REDRAW_ON_TICK;
	rt->ival = refresh_ms;
	rt->name[0] = '\0';
	AG_InitTimer(&rt->to, "redrawTick", 0);
	TAILQ_INSERT_TAIL(&wid->redrawTies, rt, redrawTies);
	
	if (wid->flags & AG_WIDGET_VISIBLE) {
		AG_AddTimer(wid, &rt->to, rt->ival, RedrawOnTickTimeout, NULL);
	} else {
		/* Fire from OnShow() */
	}
}

/* Default event handler for "key-down" (for widgets using Actions). */
void
AG_WidgetStdKeyDown(AG_Event *event)
{
	AG_Widget *wid = AG_SELF();
	int sym = AG_INT(1);
	int mod = AG_INT(2);

	AG_ExecKeyAction(wid, AG_ACTION_ON_KEYDOWN, sym, mod);
}

/* Default event handler for "key-up" (for widgets using Actions). */
void
AG_WidgetStdKeyUp(AG_Event *event)
{
	AG_Widget *wid = AG_SELF();
	int sym = AG_INT(1);
	int mod = AG_INT(2);

	AG_ExecKeyAction(wid, AG_ACTION_ON_KEYUP, sym, mod);
}

/* Default event handler for "mouse-button-down" (for widgets using Actions). */
void
AG_WidgetStdMouseButtonDown(AG_Event *event)
{
	AG_Widget *wid = AG_SELF();
	int btn = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	if (!AG_WidgetIsFocused(wid)) {
		AG_WidgetFocus(wid);
	}
	AG_ExecMouseAction(wid, AG_ACTION_ON_BUTTONDOWN, btn, x, y);
}

/* Default event handler for "mouse-button-up" (for widgets using Actions). */
void
AG_WidgetStdMouseButtonUp(AG_Event *event)
{
	AG_Widget *wid = AG_SELF();
	int btn = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	AG_ExecMouseAction(wid, AG_ACTION_ON_BUTTONUP, btn, x, y);
}

/* Tie an action to a mouse-button-down event. */
void
AG_ActionOnButtonDown(void *obj, int button, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	at = Malloc(sizeof(AG_ActionTie));
	at->type = AG_ACTION_ON_BUTTONDOWN;
	at->data.button = (AG_MouseButton)button;
	Strlcpy(at->action, action, sizeof(at->action));
	TAILQ_INSERT_TAIL(&wid->mouseActions, at, ties);

	if (AG_FindEventHandler(wid, "mouse-button-down") == NULL)
		AG_SetEvent(wid, "mouse-button-down", AG_WidgetStdMouseButtonDown, NULL);
}

/* Tie an action to a mouse-button-up event. */
void
AG_ActionOnButtonUp(void *obj, int button, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	at = Malloc(sizeof(AG_ActionTie));
	at->type = AG_ACTION_ON_BUTTONUP;
	at->data.button = (AG_MouseButton)button;
	Strlcpy(at->action, action, sizeof(at->action));
	TAILQ_INSERT_TAIL(&wid->mouseActions, at, ties);
	
	if (AG_FindEventHandler(wid, "mouse-button-up") == NULL)
		AG_SetEvent(wid, "mouse-button-up", AG_WidgetStdMouseButtonUp, NULL);
}

/* Tie an action to a key-down event. */
void
AG_ActionOnKeyDown(void *obj, AG_KeySym sym, AG_KeyMod mod, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	at = Malloc(sizeof(AG_ActionTie));
	at->type = AG_ACTION_ON_KEYDOWN;
	at->data.key.sym = sym;
	at->data.key.mod = mod;
	Strlcpy(at->action, action, sizeof(at->action));
	TAILQ_INSERT_TAIL(&wid->keyActions, at, ties);

	if (AG_FindEventHandler(wid, "key-down") == NULL)
		AG_SetEvent(wid, "key-down", AG_WidgetStdKeyDown, NULL);
}

/* Tie an action to a key-up event. */
void
AG_ActionOnKeyUp(void *obj, AG_KeySym sym, AG_KeyMod mod, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	at = Malloc(sizeof(AG_ActionTie));
	at->type = AG_ACTION_ON_KEYUP;
	at->data.key.sym = sym;
	at->data.key.mod = mod;
	Strlcpy(at->action, action, sizeof(at->action));
	TAILQ_INSERT_TAIL(&wid->keyActions, at, ties);
	
	if (AG_FindEventHandler(wid, "key-up") == NULL)
		AG_SetEvent(wid, "key-up", AG_WidgetStdKeyUp, NULL);
}

/* Timer callback for AG_ACTION_ON_KEYREPEAT actions. */
static Uint32
ActionKeyRepeatTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Widget *wid = AG_SELF();
	AG_ActionTie *at = AG_PTR(1);
	AG_Action *a;

	if (AG_TblLookupPointer(&wid->actions, at->action, (void *)&a) == -1 ||
	    a == NULL) {
		return (0);
	}
	(void)AG_ExecAction(wid, a);
	return (agKbdRepeat);
}

/* Tie an action to a key-down event, with key repeat. */
void
AG_ActionOnKey(void *obj, AG_KeySym sym, AG_KeyMod mod, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	at = Malloc(sizeof(AG_ActionTie));
	at->type = AG_ACTION_ON_KEYREPEAT;
	at->data.key.sym = sym;
	at->data.key.mod = mod;
	AG_InitTimer(&at->data.key.toRepeat, "actionKeyRepeat-", 0);
#ifdef AG_DEBUG
	Strlcat(at->data.key.toRepeat.name, action,
	    sizeof(at->data.key.toRepeat.name));
#endif
	Strlcpy(at->action, action, sizeof(at->action));
	TAILQ_INSERT_TAIL(&wid->keyActions, at, ties);
	
	if (AG_FindEventHandler(wid, "key-up") == NULL &&
	    AG_FindEventHandler(wid, "key-down") == NULL) {
		AG_SetEvent(wid, "key-up", AG_WidgetStdKeyUp, NULL);
		AG_SetEvent(wid, "key-down", AG_WidgetStdKeyDown, NULL);
	}
}

/* Configure a widget action. */
AG_Action *
AG_ActionFn(void *obj, const char *name, AG_EventFn fn, const char *fnArgs,...)
{
	AG_Widget *w = obj;
	AG_Action *a;

	AG_ObjectLock(w);
	a = Malloc(sizeof(AG_Action));
	a->type = AG_ACTION_FN;
	a->widget = w;
	a->fn = AG_SetEvent(w, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(a->fn, fnArgs);
	AG_TblInsertPointer(&w->actions, name, a);
	AG_ObjectUnlock(w);
	return (a);
}

/* Configure a widget action for setting an integer flag. */
AG_Action *
AG_ActionSetInt(void *obj, const char *name, int *p, int val)
{
	AG_Widget *w = obj;
	AG_Action *a;

	AG_ObjectLock(w);
	a = Malloc(sizeof(AG_Action));
	a->type = AG_ACTION_SET_INT;
	a->widget = w;
	a->fn = NULL;
	a->p = (void *)p;
	a->val = val;
	AG_TblInsertPointer(&w->actions, name, a);
	AG_ObjectUnlock(w);
	return (a);
}

/* Configure a widget action for toggling an integer flag. */
AG_Action *
AG_ActionToggleInt(void *obj, const char *name, int *p)
{
	AG_Widget *w = obj;
	AG_Action *a;

	AG_ObjectLock(w);
	a = Malloc(sizeof(AG_Action));
	a->type = AG_ACTION_TOGGLE_INT;
	a->widget = w;
	a->fn = NULL;
	a->p = (void *)p;
	AG_TblInsertPointer(&w->actions, name, a);
	AG_ObjectUnlock(w);
	return (a);
}

/* Configure a widget action for setting bitwise flags. */
AG_Action *
AG_ActionSetFlag(void *obj, const char *name, Uint *p, Uint bitmask, int val)
{
	AG_Widget *w = obj;
	AG_Action *a;

	AG_ObjectLock(w);
	a = Malloc(sizeof(AG_Action));
	a->type = AG_ACTION_SET_INT;
	a->widget = w;
	a->fn = NULL;
	a->p = (void *)p;
	a->bitmask = bitmask;
	a->val = val;
	AG_TblInsertPointer(&w->actions, name, a);
	AG_ObjectUnlock(w);
	return (a);
}

/* Configure a widget action for toggling bitwise flags. */
AG_Action *
AG_ActionToggleFlag(void *obj, const char *name, Uint *p, Uint bitmask)
{
	AG_Widget *w = obj;
	AG_Action *a;

	AG_ObjectLock(w);
	a = Malloc(sizeof(AG_Action));
	a->type = AG_ACTION_TOGGLE_FLAG;
	a->widget = w;
	a->fn = NULL;
	a->p = (void *)p;
	a->bitmask = bitmask;
	AG_TblInsertPointer(&w->actions, name, a);
	AG_ObjectUnlock(w);
	return (a);
}

/* Execute an action (usually called internally from AG_ExecFooAction()) */
int
AG_ExecAction(void *obj, AG_Action *a)
{
	switch (a->type) {
	case AG_ACTION_FN:
		if (a->fn != NULL) {
			a->fn->fn.fnVoid(a->fn);
			return (1);
		}
		return (0);
	case AG_ACTION_SET_INT:
		*(int *)a->p = a->val;
		return (1);
	case AG_ACTION_TOGGLE_INT:
		*(int *)a->p = !(*(int *)a->p);
		return (1);
	case AG_ACTION_SET_FLAG:
		if (a->val) {
			*(Uint *)a->p |= a->bitmask;
		} else {
			*(Uint *)a->p &= ~(a->bitmask);
		}
		return (1);
	case AG_ACTION_TOGGLE_FLAG:
		if (*(Uint *)a->p & a->bitmask) {
			*(Uint *)a->p &= ~(a->bitmask);
		} else {
			*(Uint *)a->p |= a->bitmask;
		}
		return (1);
	}
	return (0);
}

/*
 * Run any action tied to a mouse-button event. Exceptionally, we pass
 * mouse event arguments to the function.
 */
int
AG_ExecMouseAction(void *obj, AG_ActionEventType et, int button,
    int xCurs, int yCurs)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;
	AG_Action *a;

#ifdef AG_DEBUG
	if (et != AG_ACTION_ON_BUTTONDOWN &&
	    et != AG_ACTION_ON_BUTTONUP)
		AG_FatalError("Invalid type arg to AG_ExecMouseAction()");
#endif
	TAILQ_FOREACH(at, &wid->mouseActions, ties) {
		if (at->type == et &&
		    ((button == at->data.button) ||
		     (at->data.button == AG_MOUSE_ANY)))
			break;
	}
	if (at == NULL) {
		return (0);
	}
	if (AG_TblLookupPointer(&wid->actions, at->action, (void *)&a) == -1 ||
	    a == NULL) {
		return (0);
	}
	if (a->fn != NULL) {
		AG_PostEventByPtr(NULL, wid, a->fn, "%i,%i,%i", button,
		    xCurs, yCurs);
		return (1);
	}
	return (0);
}

/* Run any action tied to a key-down event. */
int
AG_ExecKeyAction(void *obj, AG_ActionEventType et, AG_KeySym sym, AG_KeyMod mod)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;
	AG_Action *a;
	int rv;

#ifdef AG_DEBUG
	if (et != AG_ACTION_ON_KEYDOWN &&
	    et != AG_ACTION_ON_KEYUP)
		AG_FatalError("AG_ExecKeyAction() type");
#endif
	TAILQ_FOREACH(at, &wid->keyActions, ties) {
		if (at->type != et &&
		    at->type != AG_ACTION_ON_KEYREPEAT) {
			continue;
		}
		if ((at->data.key.mod == AG_KEYMOD_ANY ||
		     at->data.key.mod & mod) &&
		    (at->data.key.sym == AG_KEY_ANY ||
		     at->data.key.sym == sym))
			break;
	}
	if (at == NULL) {
		return (0);
	}
	if (AG_TblLookupPointer(&wid->actions, at->action, (void *)&a) == -1 ||
	    a == NULL)
		return (0);

	rv = AG_ExecAction(wid, a);

	if (at->type == AG_ACTION_ON_KEYREPEAT) {
		if (et == AG_ACTION_ON_KEYDOWN) {
			AG_AddTimer(wid, &at->data.key.toRepeat, agKbdDelay,
			    ActionKeyRepeatTimeout, "%p", at);
		} else {
			AG_DelTimer(wid, &at->data.key.toRepeat);
		}
	}
	return (rv);
}

static void *
WidgetFindPath(const AG_Object *parent, const char *name)
{
	char node_name[AG_OBJECT_PATH_MAX];
	void *rv;
	char *s;
	AG_Object *chld;

	Strlcpy(node_name, name, sizeof(node_name));
	if ((s = strchr(node_name, '/')) != NULL) {
		*s = '\0';
	}
	if (AG_OfClass(parent, "AG_View:*")) {
		AG_Driver *drv = AGDRIVER(parent);
		AG_Window *win;

		AG_FOREACH_WINDOW(win, drv) {
			if (strcmp(AGOBJECT(win)->name, node_name) != 0) {
				continue;
			}
			if ((s = strchr(name, '/')) != NULL) {
				rv = WidgetFindPath(AGOBJECT(win), &s[1]);
				if (rv != NULL) {
					return (rv);
				} else {
					return (NULL);
				}
			}
			return (win);
		}
	} else {
		TAILQ_FOREACH(chld, &parent->children, cobjs) {
			if (strcmp(chld->name, node_name) != 0) {
				continue;
			}
			if ((s = strchr(name, '/')) != NULL) {
				rv = WidgetFindPath(chld, &s[1]);
				if (rv != NULL) {
					return (rv);
				} else {
					return (NULL);
				}
			}
			return (chld);
		}
	}
	return (NULL);
}

/*
 * Find a widget by name (e.g., "Window/Widget1/Widget2"). This works
 * similarly to the more general AG_ObjectFind(3). Return value is only
 * valid as long as the Driver VFS is locked.
 */
void *
AG_WidgetFind(void *obj, const char *name)
{
	AG_Driver *drv = obj;
	void *rv;

#ifdef AG_DEBUG
	if (name[0] != '/')
		AG_FatalError("WidgetFind: Not an absolute path: %s", name);
#endif
	AG_LockVFS(drv);
	rv = WidgetFindPath(OBJECT(drv), &name[1]);
	AG_UnlockVFS(drv);
	if (rv == NULL) {
		AG_SetError(_("The widget `%s' does not exist."), name);
	}
	return (rv);
}

/* Set the FOCUSABLE flag on a widget. */
void
AG_WidgetSetFocusable(void *obj, int flag)
{
	AG_Widget *wid = obj;

	AG_ObjectLock(wid);
	AG_SETFLAGS(wid->flags, AG_WIDGET_FOCUSABLE, flag);
	AG_ObjectUnlock(wid);
}

/* Set widget to "enabled" state for input. */
void
AG_WidgetEnable(void *obj)
{
	AG_Widget *wid = obj;

	AG_ObjectLock(wid);
	if (wid->flags & AG_WIDGET_DISABLED) {
		wid->flags &= ~(AG_WIDGET_DISABLED);
		AG_PostEvent(NULL, wid, "widget-enabled", NULL);
		AG_Redraw(wid);
	}
	AG_ObjectUnlock(wid);
}

/* Set widget to "disabled" state for input. */
void
AG_WidgetDisable(void *obj)
{
	AG_Widget *wid = obj;

	AG_ObjectLock(wid);
	if (!(wid->flags & AG_WIDGET_DISABLED)) {
		wid->flags |= AG_WIDGET_DISABLED;
		AG_PostEvent(NULL, wid, "widget-disabled", NULL);
		AG_Redraw(wid);
	}
	AG_ObjectUnlock(wid);
}

/* Arrange for a widget to automatically forward focus to another widget. */
void
AG_WidgetForwardFocus(void *obj, void *objFwd)
{
	AG_Widget *wid = obj;

	AG_ObjectLock(wid);
	if (objFwd != NULL) {
		wid->flags |= AG_WIDGET_FOCUSABLE;
		wid->focusFwd = WIDGET(objFwd);
	} else {
		wid->flags &= ~(AG_WIDGET_FOCUSABLE);
		wid->focusFwd = NULL;
	}
	AG_ObjectUnlock(wid);
}

static void
Destroy(void *obj)
{
	AG_Widget *wid = obj;
	AG_CursorArea *ca, *caNext;
	AG_RedrawTie *rt, *rtNext;
	AG_ActionTie *at, *atNext;
	AG_Variable *V;
	Uint i, j;

	for (ca = TAILQ_FIRST(&wid->cursorAreas);
	     ca != TAILQ_END(&wid->cursorAreas);
	     ca = caNext) {
		caNext = TAILQ_NEXT(ca, cursorAreas);
		free(ca);
	}
	for (rt = TAILQ_FIRST(&wid->redrawTies);
	     rt != TAILQ_END(&wid->redrawTies);
	     rt = rtNext) {
		rtNext = TAILQ_NEXT(rt, redrawTies);
		free(rt);
	}
	for (at = TAILQ_FIRST(&wid->mouseActions);
	     at != TAILQ_END(&wid->mouseActions);
	     at = atNext) {
		atNext = TAILQ_NEXT(at, ties);
		free(at);
	}
	for (at = TAILQ_FIRST(&wid->keyActions);
	     at != TAILQ_END(&wid->keyActions);
	     at = atNext) {
		atNext = TAILQ_NEXT(at, ties);
		free(at);
	}

	/* Free the action tables. */
	AG_TBL_FOREACH(V, i,j, &wid->actions) {
		Free(V->data.p);
	}
	AG_TblDestroy(&wid->actions);

	/*
	 * Free surfaces. We can assume that drivers have already deleted
	 * any associated resources.
	 */
	for (i = 0; i < wid->nsurfaces; i++) {
		if (wid->surfaces[i] != NULL && !WSURFACE_NODUP(wid,i))
			AG_SurfaceFree(wid->surfaces[i]);
	}
	Free(wid->surfaces);
	Free(wid->surfaceFlags);
	Free(wid->textures);
	Free(wid->texcoords);
}

#ifdef HAVE_OPENGL
/*
 * Variants of AG_WidgetBlit*() without explicit source or destination
 * rectangle parameter (for OpenGL-only widgets).
 */
void
AG_WidgetBlitGL(void *obj, AG_Surface *su, float w, float h)
{
	AG_Widget *wid = obj;
	wid->drvOps->blitSurfaceGL(wid->drv, wid, su, w, h);
}
void
AG_WidgetBlitSurfaceGL(void *obj, int name, float w, float h)
{
	AG_Widget *wid = obj;
	wid->drvOps->blitSurfaceFromGL(wid->drv, wid, name, w, h);
}
void
AG_WidgetBlitSurfaceFlippedGL(void *obj, int name, float w, float h)
{
	AG_Widget *wid = obj;
	wid->drvOps->blitSurfaceFlippedGL(wid->drv, wid, name, w, h);
}

/*
 * Release/backup and regenerate the GL resources associated with a widget
 * and its descendents.
 *
 * If some textures exist without a corresponding surface, allocate a
 * software surface and copy their contents to be later restored. These
 * routines are necessary for dealing with GL context loss.
 */
void
AG_WidgetFreeResourcesGL(void *obj)
{
	AG_Widget *wid = obj, *cwid;

	if (wid->drvOps->backupSurfaces != NULL) {
		wid->drvOps->backupSurfaces(wid->drv, wid);
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
		AG_WidgetFreeResourcesGL(cwid);
}
void
AG_WidgetRegenResourcesGL(void *obj)
{
	AG_Widget *wid = obj, *cwid;

	if (wid->drvOps->restoreSurfaces != NULL) {
		wid->drvOps->restoreSurfaces(wid->drv, wid);
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
		AG_WidgetRegenResourcesGL(cwid);
}
#endif /* HAVE_OPENGL */

/* Acquire widget focus */
static __inline__ void
FocusWidget(AG_Widget *w)
{
	w->flags |= AG_WIDGET_FOCUSED;
	if (w->window != NULL) {
		AG_PostEvent(w->window, w, "widget-gainfocus", NULL);
		w->window->nFocused++;
		w->window->dirty = 1;
	} else {
		Verbose("%s: Gained focus, but no parent window\n",
		    OBJECT(w)->name);
	}
}

/* Give up widget focus */
static __inline__ void
UnfocusWidget(AG_Widget *w)
{
	w->flags &= ~(AG_WIDGET_FOCUSED);
	if (w->window != NULL) {
		AG_PostEvent(w->window, w, "widget-lostfocus", NULL);
		w->window->nFocused--;
		w->window->dirty = 1;
	}
}

/* Remove focus from a widget and its children. */
void
AG_WidgetUnfocus(void *p)
{
	AG_Widget *wid = p, *cwid;

	AG_ObjectLock(wid);
	if (wid->focusFwd != NULL) {
		AG_ObjectLock(wid->focusFwd);
		if (wid->focusFwd->flags & AG_WIDGET_FOCUSED) {
			UnfocusWidget(wid->focusFwd);
		}
		AG_ObjectUnlock(wid->focusFwd);
	}
	if (wid->flags & AG_WIDGET_FOCUSED) {
		UnfocusWidget(wid);
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		AG_WidgetUnfocus(cwid);
	}
	AG_ObjectUnlock(wid);
}

/* Move the focus over a widget (and its parents). */
int
AG_WidgetFocus(void *obj)
{
	AG_Widget *wid = obj, *wParent = wid;
	AG_Window *win = wid->window;

	AG_LockVFS(wid);
	AG_ObjectLock(wid);

	if (AG_WidgetIsFocused(wid))
		goto out;

	if (!(wid->flags & AG_WIDGET_FOCUSABLE)) {
		if (wid->focusFwd != NULL &&
		    !(wid->focusFwd->flags & AG_WIDGET_FOCUSED)) {
			AG_ObjectLock(wid->focusFwd);
			FocusWidget(wid->focusFwd);
			AG_ObjectUnlock(wid->focusFwd);
			goto out;
		}
		goto fail;
	}

	/* Remove any existing focus. XXX inefficient */
	if (win != NULL && win->nFocused > 0)
		AG_WidgetUnfocus(win);

	/*
	 * Set the focus flag on the widget and its parents, up
	 * to the parent window.
	 */
	do {
		if (AG_OfClass(wParent, "AG_Widget:AG_Window:*")) {
			AG_WindowFocus(AGWINDOW(wParent));
			break;
		}
		AG_ObjectLock(wParent);
		if ((wParent->flags & AG_WIDGET_FOCUSED) == 0) {
			if (wParent->focusFwd != NULL &&
			    !(wParent->focusFwd->flags & AG_WIDGET_FOCUSED)) {
				FocusWidget(wParent->focusFwd);
			}
			FocusWidget(wParent);
		}
		AG_ObjectUnlock(wParent);
	} while ((wParent = OBJECT(wParent)->parent) != NULL);
out:
	AG_ObjectUnlock(wid);
	AG_UnlockVFS(wid);
	return (1);
fail:
	AG_ObjectUnlock(wid);
	AG_UnlockVFS(wid);
	return (0);
}

#ifdef HAVE_OPENGL

static void
DrawPrologueGL_Reshape(AG_Widget *wid)
{
	glMatrixMode(GL_PROJECTION); glPushMatrix();
	glMatrixMode(GL_MODELVIEW);  glPushMatrix();

	AG_PostEvent(NULL, wid, "widget-reshape", NULL);
	wid->flags &= ~(AG_WIDGET_GL_RESHAPE);
		
	glGetFloatv(GL_PROJECTION, wid->gl.mProjection);
	glGetFloatv(GL_MODELVIEW, wid->gl.mModelview);
		
	glMatrixMode(GL_PROJECTION); glPopMatrix();
	glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}

static void
DrawPrologueGL(AG_Widget *wid)
{
	Uint hView;

	AG_PostEvent(NULL, wid, "widget-underlay", NULL);

	glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_TEXTURE_BIT);

	if (wid->flags & AG_WIDGET_GL_RESHAPE)
		DrawPrologueGL_Reshape(wid);

	hView = AGDRIVER_SINGLE(wid->drv) ? AGDRIVER_SW(wid->drv)->h :
	                                    HEIGHT(wid->window);
	glViewport(wid->rView.x1, (hView - wid->rView.y2),
	           WIDTH(wid), HEIGHT(wid));

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(wid->gl.mProjection);
		
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(wid->gl.mModelview);

	glDisable(GL_CLIP_PLANE0);
	glDisable(GL_CLIP_PLANE1);
	glDisable(GL_CLIP_PLANE2);
	glDisable(GL_CLIP_PLANE3);
}

static void
DrawEpilogueGL(AG_Widget *wid)
{
	glMatrixMode(GL_MODELVIEW);	glPopMatrix();
	glMatrixMode(GL_PROJECTION);	glPopMatrix();
	glMatrixMode(GL_TEXTURE);	glPopMatrix();

	glPopAttrib(); /* GL_TRANSFORM_BIT | GL_VIEWPORT_BIT */
	
	AG_PostEvent(NULL, wid, "widget-overlay", NULL);
}
#endif /* HAVE_OPENGL */

/*
 * Render a widget to the display.
 * Must be invoked from GUI rendering context.
 */
void
AG_WidgetDraw(void *p)
{
	AG_Widget *wid = p;

	AG_ObjectLock(wid);

	if (!(wid->flags & AG_WIDGET_VISIBLE) ||
	     (wid->flags & AG_WIDGET_UNDERSIZE) ||
	     WIDGET_OPS(wid)->draw == NULL)
		goto out;

	if (wid->flags & AG_WIDGET_DISABLED) {       wid->cState = AG_DISABLED_STATE; }
	else if (wid->flags & AG_WIDGET_MOUSEOVER) { wid->cState = AG_HOVER_STATE; }
	else if (wid->flags & AG_WIDGET_FOCUSED) {   wid->cState = AG_FOCUSED_STATE; }
	else {                                       wid->cState = AG_DEFAULT_STATE; }

	if (wid->flags & AG_WIDGET_USE_TEXT) {
		AG_PushTextState();
		AG_TextFont(wid->font);
		AG_TextColor(wid->pal.c[wid->cState][AG_TEXT_COLOR]);
	}
#ifdef HAVE_OPENGL
	if (wid->flags & AG_WIDGET_USE_OPENGL)
		DrawPrologueGL(wid);
#endif

	WIDGET_OPS(wid)->draw(wid);
	
#ifdef HAVE_OPENGL
	if (wid->flags & AG_WIDGET_USE_OPENGL)
		DrawEpilogueGL(wid);
#endif
	if (wid->flags & AG_WIDGET_USE_TEXT)
		AG_PopTextState();
out:
	AG_ObjectUnlock(wid);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	r->w = 0;
	r->h = 0;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	return (0);
}

void
AG_WidgetSizeReq(void *obj, AG_SizeReq *r)
{
	AG_Widget *w = obj;

	r->w = 0;
	r->h = 0;

	AG_ObjectLock(w);
	if (w->flags & AG_WIDGET_USE_TEXT) {
		AG_PushTextState();
		AG_TextFont(w->font);
	}
	if (WIDGET_OPS(w)->size_request != NULL) {
		WIDGET_OPS(w)->size_request(w, r);
	}
	if (w->flags & AG_WIDGET_USE_TEXT) {
		AG_PopTextState();
	}
	AG_ObjectUnlock(w);
}

void
AG_WidgetSizeAlloc(void *obj, AG_SizeAlloc *a)
{
	AG_Widget *w = obj;

	AG_ObjectLock(w);

	if (w->flags & AG_WIDGET_USE_TEXT) {
		AG_PushTextState();
		AG_TextFont(w->font);
	}
	if (a->w <= 0 || a->h <= 0) {
		a->w = 0;
		a->h = 0;
		w->flags |= AG_WIDGET_UNDERSIZE;
	} else {
		w->flags &= ~(AG_WIDGET_UNDERSIZE);
	}
	w->x = a->x;
	w->y = a->y;
	w->w = a->w;
	w->h = a->h;
	if (WIDGET_OPS(w)->size_allocate != NULL) {
		if (WIDGET_OPS(w)->size_allocate(w, a) == -1) {
			w->flags |= AG_WIDGET_UNDERSIZE;
		} else {
			w->flags &= ~(AG_WIDGET_UNDERSIZE);
		}
	}
	if (w->flags & AG_WIDGET_USE_TEXT) {
		AG_PopTextState();
	}
#ifdef HAVE_OPENGL
	w->flags |= AG_WIDGET_GL_RESHAPE;
#endif
	AG_ObjectUnlock(w);
}

/*
 * Test whether view coordinates x,y lie in widget's sensitivity rectangle
 * (intersected against those of all parent widgets).
 */
int
AG_WidgetSensitive(void *obj, int x, int y)
{
	AG_Widget *wt = WIDGET(obj);
	AG_Widget *wtParent = wt;
	AG_Rect2 rx = wt->rSens;

	while ((wtParent = OBJECT(wtParent)->parent) != NULL) {
		if (AG_OfClass(wtParent, "AG_Widget:AG_Window:*")) {
			break;
		}
		rx = AG_RectIntersect2(&rx, &wtParent->rSens);
	}
	return AG_RectInside2(&rx, x,y);
}

/*
 * Search for a focused widget inside a window. Return value is only valid
 * as long as the Driver VFS is locked.
 */
AG_Widget *
AG_WidgetFindFocused(void *p)
{
	AG_Widget *wid = p;
	AG_Widget *cwid, *fwid;

	AG_LockVFS(wid);
	AG_ObjectLock(wid);

	if (!AG_OfClass(wid, "AG_Widget:AG_Window:*")) {
		if ((wid->flags & AG_WIDGET_FOCUSED) == 0 ||
		    (wid->flags & AG_WIDGET_VISIBLE) == 0 ||
		    (wid->flags & AG_WIDGET_DISABLED)) {
			goto fail;
		}
	}
	/* Search for a better match. */
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		if ((fwid = AG_WidgetFindFocused(cwid)) != NULL) {
			AG_ObjectUnlock(wid);
			AG_UnlockVFS(wid);
			return (fwid);
		}
	}

	AG_ObjectUnlock(wid);
	AG_UnlockVFS(wid);
	return (wid);
fail:
	AG_ObjectUnlock(wid);
	AG_UnlockVFS(wid);
	return (NULL);
}

/* Compute the absolute view coordinates of a widget and its descendents. */
void
AG_WidgetUpdateCoords(void *obj, int x, int y)
{
	AG_Widget *wid = obj, *chld;
	AG_Rect2 rPrev;

	AG_LockVFS(wid);
	AG_ObjectLock(wid);
	wid->flags &= ~(AG_WIDGET_UPDATE_WINDOW);

	if (wid->drv != NULL && AGDRIVER_MULTIPLE(wid->drv) &&
	    AG_OfClass(wid, "AG_Widget:AG_Window:*")) {
		/* Multiple-window drivers use window coordinate systems */
		x = 0;
		y = 0;
	}

	rPrev = wid->rView;
	wid->rView.x1 = x;
	wid->rView.y1 = y;
	wid->rView.w = wid->w;
	wid->rView.h = wid->h;
	wid->rView.x2 = x + wid->w;
	wid->rView.y2 = y + wid->h;
	
	wid->rSens.x1 = x;
	wid->rSens.y1 = y;
	wid->rSens.w = wid->w;
	wid->rSens.h = wid->h;
	wid->rSens.x2 = x + wid->w;
	wid->rSens.y2 = y + wid->h;

	if (AG_RectCompare2(&wid->rView, &rPrev) != 0) {
		AG_PostEvent(NULL, wid, "widget-moved", NULL);
#ifdef HAVE_OPENGL
		wid->flags |= AG_WIDGET_GL_RESHAPE;
#endif
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_WidgetUpdateCoords(chld,
		    wid->rView.x1 + chld->x,
		    wid->rView.y1 + chld->y);
	}

	AG_ObjectUnlock(wid);
	AG_UnlockVFS(wid);
}

/* Parse a generic size specification. */
enum ag_widget_sizespec
AG_WidgetParseSizeSpec(const char *input, int *w)
{
	char spec[1024], *p;
	size_t len;

	Strlcpy(spec, input, sizeof(spec));
	len = strlen(spec);
	if (len == 0) { goto syntax; }
	p = &spec[len-1];

	switch (*p) {
	case '-':
		*w = 0;
		return (AG_WIDGET_FILL);
	case '%':
		*p = '\0';
		*w = (int)strtol(spec, NULL, 10);
		return (AG_WIDGET_PERCENT);
	case '>':
		if (spec[0] != '<') { goto syntax; }
		*p = '\0';
		AG_TextSize(&spec[1], w, NULL);
		return (AG_WIDGET_STRINGLEN);
	case 'x':
		if (p > &spec[0] && p[-1] != 'p') { goto syntax; }
		p[-1] = '\0';
		*w = (int)strtol(spec, NULL, 10);
		return (AG_WIDGET_PIXELS);
	}
syntax:
	Verbose("Warning: Bad SizeSpec: \"%s\"\n", input);
	*w = 0;
	return (AG_WIDGET_BAD_SPEC);
}

int
AG_WidgetScrollDelta(Uint32 *t1)
{
	Uint32 t2 = AG_GetTicks();
	int delta;

	if (*t1 != 0 && ((delta = (t2 - *t1))) < 250) {
		return (((250-delta)<<3)>>9);
	}
	*t1 = AG_GetTicks();
	return (1);
}

/* Show a widget */
void
AG_WidgetShow(void *obj)
{
	AG_Widget *wid = obj;

	AG_ObjectLock(wid);
	wid->flags &= ~(AG_WIDGET_HIDE);
	AG_PostEvent(NULL, wid, "widget-shown", NULL);
	AG_WindowUpdate(wid->window);
	AG_ObjectUnlock(wid);
}

/* Hide a widget */
void
AG_WidgetHide(void *obj)
{
	AG_Widget *wid = obj;

	AG_ObjectLock(wid);
	wid->flags |= AG_WIDGET_HIDE;
	AG_PostEvent(NULL, wid, "widget-hidden", NULL);
	AG_WindowUpdate(wid->window);
	AG_ObjectUnlock(wid);
}

/* Make a widget and all of its children visible. */
void
AG_WidgetShowAll(void *p)
{
	AG_Widget *wid = p;
	AG_Widget *chld;

	AG_LockVFS(wid);
	AG_ObjectLock(wid);

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		AG_WidgetShowAll(chld);

	AG_PostEvent(NULL, wid, "widget-shown", NULL);

	AG_ObjectUnlock(wid);
	AG_UnlockVFS(wid);
}

/* Make a widget and all of its children invisible. */
void
AG_WidgetHideAll(void *p)
{
	AG_Widget *wid = p;
	AG_Widget *chld;

	AG_LockVFS(wid);
	AG_ObjectLock(wid);
	
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		AG_WidgetHideAll(chld);

	AG_PostEvent(NULL, wid, "widget-hidden", NULL);
	
	AG_ObjectUnlock(wid);
	AG_UnlockVFS(wid);
}

static void *
FindAtPoint(AG_Widget *parent, const char *type, int x, int y)
{
	AG_Widget *chld;
	void *p;

	OBJECT_FOREACH_CHILD(chld, parent, ag_widget) {
		if ((p = FindAtPoint(chld, type, x, y)) != NULL)
			return (p);
	}
	if ((parent->flags & AG_WIDGET_VISIBLE) &&
	    AG_OfClass(parent, type) &&
	    AG_WidgetArea(parent, x, y)) {
		return (parent);
	}
	return (NULL);
}

/* Search for widgets of the specified class enclosing the given point. */
void *
AG_WidgetFindPoint(const char *type, int x, int y)
{
	AG_Driver *drv;
	AG_Window *win;
	void *p;

	AG_LockVFS(&agDrivers);
	OBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW_REVERSE(win, drv) {
			if ((p = FindAtPoint(WIDGET(win), type, x, y)) != NULL) {
				AG_UnlockVFS(&agDrivers);
				return (p);
			}
		}
	}
	AG_UnlockVFS(&agDrivers);
	return (NULL);
}

static void *
FindRectOverlap(AG_Widget *parent, const char *type, int x, int y, int w, int h)
{
	AG_Widget *chld;
	void *p;

	OBJECT_FOREACH_CHILD(chld, parent, ag_widget) {
		if ((p = FindRectOverlap(chld, type, x,y,w,h)) != NULL)
			return (p);
	}
	if (AG_OfClass(parent, type) &&
	    !(x+w < parent->rView.x1 || x > parent->rView.x2 ||
	      y+w < parent->rView.y1 || y > parent->rView.y2)) {
		return (parent);
	}
	return (NULL);
}

/*
 * Search for widgets of the specified class enclosing the given rectangle.
 * Result is only accurate as long as the Driver VFS is locked.
 */
void *
AG_WidgetFindRect(const char *type, int x, int y, int w, int h)
{
	AG_Driver *drv;
	AG_Window *win;
	void *p;
	
	AG_LockVFS(&agDrivers);
	OBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW_REVERSE(win, drv) {
			if ((p = FindRectOverlap(WIDGET(win), type, x,y,w,h)) != NULL) {
				AG_UnlockVFS(&agDrivers);
				return (p);
			}
		}
	}
	AG_UnlockVFS(&agDrivers);
	return (NULL);
}

/* Generic inherited draw() routine. */
void
AG_WidgetInheritDraw(void *obj)
{
	WIDGET_SUPER_OPS(obj)->draw(obj);
}

/* Generic inherited size_request() routine. */
void
AG_WidgetInheritSizeRequest(void *obj, AG_SizeReq *r)
{
	WIDGET_SUPER_OPS(obj)->size_request(obj, r);
}

/* Generic inherited size_allocate() routine. */
int
AG_WidgetInheritSizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	return WIDGET_SUPER_OPS(obj)->size_allocate(obj, a);
}

/* Render a widget to an AG_Surface(3). */
AG_Surface *
AG_WidgetSurface(void *obj)
{
	AG_Widget *wid = obj;
	AG_Surface *su;
	int rv;

	AG_LockVFS(wid);
	rv = wid->drvOps->renderToSurface(wid->drv, wid, &su);
	AG_UnlockVFS(wid);
	return (rv == 0) ? su : NULL;
}

/* Map a new AG_Surface(3) with a widget; return the new surface handle. */
int
AG_WidgetMapSurface(void *obj, AG_Surface *su)
{
	AG_Widget *wid = obj;
	int i, s = -1;

	if (su == NULL)
		return (-1);

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nsurfaces; i++) {
		if (wid->surfaces[i] == NULL) {
			s = i;
			break;
		}
	}
	if (i == wid->nsurfaces) {
		wid->surfaces = Realloc(wid->surfaces,
		    (wid->nsurfaces+1)*sizeof(AG_Surface *));
		wid->surfaceFlags = Realloc(wid->surfaceFlags,
		    (wid->nsurfaces+1)*sizeof(Uint));
		wid->textures = Realloc(wid->textures,
		    (wid->nsurfaces+1)*sizeof(Uint));
		wid->texcoords = Realloc(wid->texcoords,
		    (wid->nsurfaces+1)*sizeof(AG_TexCoord));
		s = wid->nsurfaces++;
	}
	wid->surfaces[s] = su;
	wid->surfaceFlags[s] = 0;
	wid->textures[s] = 0;
	AG_ObjectUnlock(wid);
	return (s);
}

/* Replace the contents of a mapped surface. */
void
AG_WidgetReplaceSurface(void *obj, int s, AG_Surface *su)
{
	AG_Widget *wid = (AG_Widget *)obj;

	AG_ObjectLock(wid);
#ifdef AG_DEBUG
	if (s < 0 || s >= wid->nsurfaces)
		AG_FatalError("Invalid surface handle");
#endif
	if (wid->surfaces[s] != NULL) {
		if (!WSURFACE_NODUP(wid,s))
			AG_SurfaceFree(wid->surfaces[s]);
	}
	wid->surfaces[s] = su;
	wid->surfaceFlags[s] &= ~(AG_WIDGET_SURFACE_NODUP);

	/*
	 * Queue the previous texture for deletion and set the texture handle
	 * to 0 so the texture will be regenerated at the next blit.
	 */
	if (wid->textures[s] != 0 &&
	    wid->drv != NULL &&
	    wid->drvOps->deleteTexture != NULL) {
		wid->drvOps->deleteTexture(wid->drv, wid->textures[s]);
		wid->textures[s] = 0;
	}
	AG_ObjectUnlock(wid);
}

/*
 * Rebuild the effective style parameters of a widget and its descendants
 * based on its style attributes. Any required fonts are loaded. This
 * should be called whenever style attributes are changed.
 */
static void
CompileStyleRecursive(AG_Widget *wid, const char *parentFace,
    double parentPtSize, Uint parentFlags,
    AG_WidgetPalette parentPalette)
{
	AG_StyleSheet *css = &agDefaultCSS;
	char face[256];
	double ptSize;
	Uint flags = parentFlags;
	AG_Widget *chld;
	AG_Variable *V;
	int i, j;
	AG_Object *po;
	char *cssData;
	double v;
	char *ep;

	/* Select the effective style sheet for this widget. */
	for (po = OBJECT(wid);
	     po->parent != NULL && AG_OfClass(po->parent, "AG_Widget:*");
	     po = po->parent) {
		if (WIDGET(po)->css != NULL) {
			css = WIDGET(po)->css;
			break;
		}
	}

	/* Set the font attributes. */
	if ((V = AG_GetVariableLocked(wid, "font-family")) != NULL) {
		Strlcpy(face, V->data.s, sizeof(face));
		AG_UnlockVariable(V);
	} else if (AG_LookupStyleSheet(css, wid, "font-family", &cssData)) {
		Strlcpy(face, cssData, sizeof(face));
	} else {
		Strlcpy(face, parentFace, sizeof(face));
	}
	if ((V = AG_GetVariableLocked(wid, "font-size")) != NULL) {
		v = strtod(V->data.s, &ep);
		ptSize = (*ep == '%') ? parentPtSize*(v/100.0) : v;
		AG_UnlockVariable(V);
	} else if (AG_LookupStyleSheet(css, wid, "font-size", &cssData)) {
		v = strtod(cssData, &ep);
		ptSize = (*ep == '%') ? parentPtSize*(v/100.0) : v;
	} else {
		ptSize = parentPtSize;
	}
	if ((V = AG_GetVariableLocked(wid, "font-weight")) != NULL) {
		if (AG_Strcasecmp(V->data.s, "bold") == 0) {
			flags |= AG_FONT_BOLD;
		} else if (AG_Strcasecmp(V->data.s, "normal") == 0) {
			flags &= ~(AG_FONT_BOLD);
		}
		AG_UnlockVariable(V);
	} else if (AG_LookupStyleSheet(css, wid, "font-weight", &cssData)) {
		if (AG_Strcasecmp(cssData, "bold") == 0) {
			flags |= AG_FONT_BOLD;
		} else if (AG_Strcasecmp(cssData, "normal") == 0) {
			flags &= ~(AG_FONT_BOLD);
		}
	}
	if ((V = AG_GetVariableLocked(wid, "font-style")) != NULL) {
		if (AG_Strcasecmp(V->data.s, "italic") == 0) {
			flags |= AG_FONT_ITALIC;
		} else if (AG_Strcasecmp(V->data.s, "normal") == 0) {
			flags &= ~(AG_FONT_ITALIC);
		}
		AG_UnlockVariable(V);
	}
	
	/* Set the color attributes. */
	for (i = 0; i < AG_WIDGET_NSTATES; i++) {
		for (j = 0; j < AG_WIDGET_NCOLORS; j++) {
			AG_Color *parentColor = &parentPalette.c[i][j];
			char vName[AG_VARIABLE_NAME_MAX];

			Strlcpy(vName, agWidgetColorNames[j], sizeof(vName));
			Strlcat(vName, agWidgetStateNames[i], sizeof(vName));
			if ((V = AG_GetVariableLocked(wid, vName)) != NULL) {
				wid->pal.c[i][j] = AG_ColorFromString(V->data.s,
				    parentColor);
				AG_UnlockVariable(V);
			} else if (AG_LookupStyleSheet(css, wid, vName, &cssData)) {
				wid->pal.c[i][j] = AG_ColorFromString(cssData,
				    parentColor);
			} else {
				Strlcpy(vName, agWidgetColorNames[j], sizeof(vName));
				if (AG_LookupStyleSheet(css, wid, vName, &cssData)) {
					wid->pal.c[i][j] = AG_ColorFromString(cssData,
					    parentColor);
				} else {
					wid->pal.c[i][j] = *parentColor;
				}
			}
		}
	}

	if (wid->flags & AG_WIDGET_USE_TEXT) {
		char *pFace = face, *tok;
		AG_Font *fontNew = NULL;

		while ((tok = AG_Strsep(&pFace, ",")) != NULL) {
			fontNew = AG_FetchFont(face, (int)ptSize, (int)flags);
			if (fontNew != NULL)
				break;
		}
		if (fontNew == NULL) {
			fontNew = AG_FetchFont(NULL, (int)ptSize, (int)flags);
		}
		if (fontNew != NULL && wid->font != fontNew) {
			if (wid->font != NULL) {
				AG_UnusedFont(wid->font);
			}
			wid->font = fontNew;
			AG_PushTextState();
			AG_TextFont(wid->font);
			AG_PostEvent(NULL, wid, "font-changed", NULL);
			AG_PopTextState();
			AG_Redraw(wid);
		}
	}


	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		CompileStyleRecursive(chld, face, ptSize, flags, wid->pal);
}
void
AG_WidgetCompileStyle(void *obj)
{
	AG_Widget *wid = obj;
	AG_Widget *parent;

	AG_LockVFS(wid);
	AG_MutexLock(&agTextLock);

	if ((parent = OBJECT(wid)->parent) != NULL &&
	    AG_OfClass(parent, "AG_Widget:*") &&
	    parent->font != NULL) {
		CompileStyleRecursive(wid,
		    OBJECT(parent->font)->name,
		    parent->font->spec.size,
		    parent->font->flags,
		    parent->pal);
	} else {

		CompileStyleRecursive(wid,
		    OBJECT(agDefaultFont)->name,
		    agDefaultFont->spec.size,
		    agDefaultFont->flags,
		    agDefaultPalette);
	}
	AG_MutexUnlock(&agTextLock);
	AG_UnlockVFS(wid);
}

/*
 * Clear the style parameters of a widget and its descendants. Called
 * on detach. The widget's VFS must be locked.
 */
void
AG_WidgetFreeStyle(void *obj)
{
	AG_Widget *wid = obj;
	AG_Widget *chld;
	
	if (wid->font != NULL) {
		AG_UnusedFont(wid->font);
		wid->font = NULL;
	}
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		AG_WidgetFreeStyle(chld);
}

/* Copy all style properties from one widget to another. */
void
AG_WidgetCopyStyle(void *objDst, void *objSrc)
{
	AG_Widget *widSrc = objSrc;
	AG_Widget *widDst = objDst;
	AG_Variable *V;
	const char **s;

	AG_ObjectLock(widSrc);
	AG_ObjectLock(widDst);
	for (s = &agWidgetPropNames[0]; *s != NULL; s++) {
		if ((V = AG_GetVariableLocked(widSrc, *s)) != NULL) {
			AG_SetString(widDst, *s, V->data.s);
			AG_UnlockVariable(V);
		}

	}
	AG_ObjectUnlock(widDst);
	AG_ObjectUnlock(widSrc);

	AG_WidgetCompileStyle(widDst);
	AG_Redraw(widDst);
}

/*
 * Set the default font parameters for a widget.
 * If a NULL argument is provided, the parameter is inherited from parent.
 */
void
AG_SetFont(void *obj, const AG_Font *font)
{
	AG_Widget *wid = obj;

	AG_SetString(wid, "font-family", OBJECT(font)->name);
	AG_SetString(wid, "font-size", AG_Printf("%.2fpts", font->spec.size));
	AG_SetString(wid, "font-weight", (font->flags & AG_FONT_BOLD) ? "bold" : "normal");
	AG_SetString(wid, "font-style", (font->flags & AG_FONT_ITALIC) ? "italic" : "normal");
	AG_WidgetCompileStyle(wid);
	AG_Redraw(wid);
}
void
AG_SetStyle(void *obj, const char *which, const char *value)
{
	AG_Widget *wid = obj;

	if (value != NULL) {
		AG_SetString(wid, which, value);
	} else {
		AG_Unset(wid, which);				/* inherit */
	}
	AG_WidgetCompileStyle(wid);
	AG_Redraw(wid);
}

AG_WidgetClass agWidgetClass = {
	{
		"Agar(Widget)",
		sizeof(AG_Widget),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* draw */
	SizeRequest,
	SizeAllocate
};
