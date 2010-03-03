/*
 * Copyright (c) 2001-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "opengl.h"

#include <core/core.h>
#include <core/config.h>

#include "gui.h"
#include "widget.h"
#include "window.h"
#include "cursors.h"
#include "menu.h"
#include "primitive.h"
#include "notebook.h"
#include "gui_math.h"

#include <stdarg.h>
#include <string.h>
#include <ctype.h>

/* #define DEBUG_CLIPPING */

/* Set the parent window/driver pointers on a widget and its children. */
static void
SetParentWindow(AG_Widget *wid, AG_Window *win)
{
	AG_Widget *chld;
	
	wid->window = win;
	if (win != NULL) {
		wid->drv = (AG_Driver *)OBJECT(win)->parent;
		wid->drvOps = AGDRIVER_CLASS(wid->drv);
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

/* Set the style pointers on a widget and its children. */
static void
SetStyle(AG_Widget *wid, AG_Style *style)
{
	AG_Widget *chld;

	wid->style = style;
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		SetStyle(chld, style);
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

		if (widParent->style != NULL) {
			SetStyle(w, widParent->style);
		}
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

		if (widParent->style != NULL) {
			SetStyle(w, widParent->style);
		}
		SetParentWindow(w, widParent->window);
	} else if (AG_OfClass(parent, "AG_Driver:*") &&
	           AG_OfClass(w, "AG_Widget:AG_Window:*")) {
		AG_Driver *drvParent = (AG_Driver *)parent;

		AG_SetStyle(w,
		    (AGDRIVER_CLASS(drvParent)->wm == AG_WM_SINGLE) ?
		    AGDRIVER_SW(drvParent)->style : &agStyleDefault);
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

#ifdef AG_LEGACY
/* "widget-bound" event; replaced by AG_Variable(3) in 1.3.4. */
static void
Bound(AG_Event *event)
{
	AG_Widget *wid = AG_SELF();
	AG_Variable *V = AG_PTR(1);
	AG_PostEvent(NULL, wid, "widget-bound", "%p", V);
}
#endif /* AG_LEGACY */

static void
Init(void *obj)
{
	AG_Widget *wid = obj;

	OBJECT(wid)->save_pfx = "/widgets";
	OBJECT(wid)->flags |= AG_OBJECT_NAME_ONATTACH;

	wid->flags = 0;
	wid->rView = AG_RECT2(-1,-1,-1,-1);
	wid->rSens = AG_RECT2(0,0,0,0);
	wid->x = -1;
	wid->y = -1;
	wid->w = -1;
	wid->h = -1;
	wid->style = &agStyleDefault;
	SLIST_INIT(&wid->menus);
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
	wid->mouseActions = NULL;
	wid->nMouseActions = 0;
	wid->keyActions = NULL;
	wid->nKeyActions = 0;

	AG_SetEvent(wid, "attached", OnAttach, NULL);
	AG_SetEvent(wid, "detached", OnDetach, NULL);
#ifdef AG_LEGACY
	/* "widget-bound" event; replaced by AG_Variable(3) in 1.3.4. */
	AG_SetEvent(wid, "bound", Bound, NULL);
#endif
}

/* Tie an action to a mouse-button-down event. */
void
AG_ActionOnButtonDown(void *obj, int button, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	wid->mouseActions = Realloc(wid->mouseActions, (wid->nMouseActions+1)*
	                                               sizeof(AG_ActionTie));
	at = &wid->mouseActions[wid->nMouseActions++];
	at->type = AG_ACTION_ON_BUTTONDOWN;
	at->data.button = (AG_MouseButton)button;
	Strlcpy(at->action, action, sizeof(at->action));
}

/* Tie an action to a mouse-button-up event. */
void
AG_ActionOnButtonUp(void *obj, int button, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	wid->mouseActions = Realloc(wid->mouseActions, (wid->nMouseActions+1)*
	                                               sizeof(AG_ActionTie));
	at = &wid->mouseActions[wid->nMouseActions++];
	at->type = AG_ACTION_ON_BUTTONUP;
	at->data.button = (AG_MouseButton)button;
	Strlcpy(at->action, action, sizeof(at->action));
}

/* Tie an action to a key-down event. */
void
AG_ActionOnKeyDown(void *obj, AG_KeySym sym, AG_KeyMod mod, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	wid->keyActions = Realloc(wid->keyActions, (wid->nKeyActions+1)*
	                                           sizeof(AG_ActionTie));
	at = &wid->keyActions[wid->nKeyActions++];
	at->type = AG_ACTION_ON_KEYDOWN;
	at->data.key.sym = sym;
	at->data.key.mod = mod;
	Strlcpy(at->action, action, sizeof(at->action));
}

/* Tie an action to a key-up event. */
void
AG_ActionOnKeyUp(void *obj, AG_KeySym sym, AG_KeyMod mod, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	wid->keyActions = Realloc(wid->keyActions, (wid->nKeyActions+1)*
	                                           sizeof(AG_ActionTie));
	at = &wid->keyActions[wid->nKeyActions++];
	at->type = AG_ACTION_ON_KEYUP;
	at->data.key.sym = sym;
	at->data.key.mod = mod;
	Strlcpy(at->action, action, sizeof(at->action));
}

static Uint32
ActionKeyRepeat(void *obj, Uint32 ival, void *arg)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at = arg;
	AG_Action *a;
	
	if (AG_TblLookupPointer(&wid->actions, at->action, (void *)&a) == -1 ||
	    a == NULL) {
		return (0);
	}
	(void)AG_ExecAction(wid, a);
	return (agKbdRepeat);
}

static Uint32
ActionKeyDelay(void *obj, Uint32 ival, void *arg)
{
	AG_ActionTie *at = arg;

	AG_ScheduleTimeout(obj, &at->data.key.toRepeat, agKbdRepeat);
	return (0);
}

/* Tie an action to a key-down event, with key repeat. */
void
AG_ActionOnKey(void *obj, AG_KeySym sym, AG_KeyMod mod, const char *action)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;

	wid->keyActions = Realloc(wid->keyActions, (wid->nKeyActions+1)*
	                                           sizeof(AG_ActionTie));
	at = &wid->keyActions[wid->nKeyActions++];
	at->type = AG_ACTION_ON_KEYREPEAT;
	at->data.key.sym = sym;
	at->data.key.mod = mod;
	Strlcpy(at->action, action, sizeof(at->action));
	AG_SetTimeout(&at->data.key.toDelay, ActionKeyDelay, at, 0);
	AG_SetTimeout(&at->data.key.toRepeat, ActionKeyRepeat, at, 0);
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
			AG_ExecEventFn(obj, a->fn);
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

/* Run any action tied to a mouse-button event. */
int
AG_ExecMouseAction(void *obj, AG_ActionEventType et, int button,
    int xCurs, int yCurs)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;
	AG_Action *a;
	Uint i;

#ifdef AG_DEBUG
	if (et != AG_ACTION_ON_BUTTONDOWN &&
	    et != AG_ACTION_ON_BUTTONUP)
		AG_FatalError("Invalid type arg to AG_ExecMouseAction()");
#endif
	for (i = 0; i < wid->nMouseActions; i++) {
		at = &wid->mouseActions[i];
		if (at->type == et &&
		    ((button == at->data.button) ||
		     (at->data.button == AG_MOUSE_ANY)))
			break;
	}
	if (i == wid->nMouseActions) {
		return (0);
	}
	if (AG_TblLookupPointer(&wid->actions, at->action, (void *)&a) == -1 ||
	    a == NULL) {
		return (0);
	}
	return AG_ExecAction(wid, a);
}

/* Run any action tied to a key-down event. */
int
AG_ExecKeyAction(void *obj, AG_ActionEventType et, AG_KeySym sym, AG_KeyMod mod)
{
	AG_Widget *wid = obj;
	AG_ActionTie *at;
	AG_Action *a;
	Uint i;
	int rv;

#ifdef AG_DEBUG
	if (et != AG_ACTION_ON_KEYDOWN &&
	    et != AG_ACTION_ON_KEYUP)
		AG_FatalError("AG_ExecKeyAction() type");
#endif
	for (i = 0; i < wid->nKeyActions; i++) {
		at = &wid->keyActions[i];
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
	if (i == wid->nKeyActions) {
		return (0);
	}
	if (AG_TblLookupPointer(&wid->actions, at->action, (void *)&a) == -1 ||
	    a == NULL)
		return (0);

	rv = AG_ExecAction(wid, a);

	if (at->type == AG_ACTION_ON_KEYREPEAT) {
		if (et == AG_ACTION_ON_KEYDOWN) {
			AG_ScheduleTimeout(wid, &at->data.key.toDelay,
			    agKbdDelay);
		} else {
			AG_DelTimeout(wid, &at->data.key.toDelay);
			AG_DelTimeout(wid, &at->data.key.toRepeat);
		}
	}
	return (rv);
}

/* Traverse the widget tree using a pathname. */
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
	} else if (AG_OfClass(parent, "AG_Widget:AG_Notebook:*")) {
		AG_Notebook *book = (AG_Notebook *)parent;
		AG_NotebookTab *tab;
		/*
		 * This hack allows Notebook tabs to be treated as
		 * separate objects, even though they are not attached
		 * to the widget hierarchy.
		 */
		AG_ObjectLock(book);
		TAILQ_FOREACH(tab, &book->tabs, tabs) {
			if (strcmp(OBJECT(tab)->name, node_name) != 0) {
				continue;
			}
			if ((s = strchr(name, '/')) != NULL) {
				rv = WidgetFindPath(OBJECT(tab), &s[1]);
				if (rv != NULL) {
					AG_ObjectUnlock(book);
					return (rv);
				} else {
					AG_ObjectUnlock(book);
					return (NULL);
				}
			}
			AG_ObjectUnlock(book);
			return (tab);
		}
		AG_ObjectUnlock(book);
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
 * Find a widget by name. Return value is only valid as long as the
 * Driver VFS is locked.
 *
 * This works differently than the general AG_ObjectFind() routine in
 * that the search may include widgets not effectively attached to the
 * Driver VFS, such as widgets attached to AG_Notebook(3) tabs.
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
	AG_PopupMenu *pm, *pm2;
	AG_Variable *V;
	Uint i, j;

	/* Destroy any attached popup menu. */
	for (pm = SLIST_FIRST(&wid->menus);
	     pm != SLIST_END(&wid->menus);
	     pm = pm2) {
		pm2 = SLIST_NEXT(pm, menus);
		AG_PopupDestroy(NULL, pm);
	}

	/* Free the action tables. */
	AG_TBL_FOREACH(V, i,j, &wid->actions) {
		Free(V->data.p);
	}
	AG_TblDestroy(&wid->actions);
	Free(wid->mouseActions);
	Free(wid->keyActions);

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
 * Release/backup and regenerate the GL resources associated with a widget.
 * If some textures exist without a corresponding surface, allocate a
 * software surface and copy their contents to be later restored. These
 * routines are necessary for dealing with GL context loss.
 */
void
AG_WidgetFreeResourcesGL(AG_Widget *wid)
{
	if (wid->drvOps->backupSurfaces != NULL)
		wid->drvOps->backupSurfaces(wid->drv, wid);
}
void
AG_WidgetRegenResourcesGL(AG_Widget *wid)
{
	if (wid->drvOps->restoreSurfaces != NULL)
		wid->drvOps->restoreSurfaces(wid->drv, wid);
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

/*
 * Evaluate whether a given widget is at least partially visible.
 * The Widget and Driver VFS must be locked.
 */
/* TODO optimize on a per window basis */
static __inline__ int
OccultedWidget(AG_Widget *wid)
{
	AG_Window *wParent;
	AG_Window *w;

	if ((wParent = AG_ObjectFindParent(wid, NULL, "AG_Widget:AG_Window")) == NULL ||
	    (w = OBJECT_NEXT_CHILD(wParent, ag_window)) == NULL) {
		return (0);
	}
#if 1
	for (; w != NULL; w = OBJECT_NEXT_CHILD(w, ag_window)) {
		if (w->visible &&
		    wid->rView.x1 > WIDGET(w)->x &&
		    wid->rView.y1 > WIDGET(w)->y &&
		    wid->rView.x2 < WIDGET(w)->x+WIDGET(w)->w &&
		    wid->rView.y2 < WIDGET(w)->y+WIDGET(w)->h)
			return (1);
	}
#endif
	return (0);
}

/*
 * Render a widget to the display.
 * Must be invoked from GUI rendering context.
 */
void
AG_WidgetDraw(void *p)
{
	AG_Widget *wid = p;

	AG_ObjectLock(wid);

#ifdef AG_DEBUG
	if (wid->drv == NULL)
		AG_FatalError("AG_WidgetDraw() on unattached widget");
	if (!AG_OfClass(wid, "AG_Widget:AG_Window:*") && wid->window == NULL)
		AG_FatalError("Widget is not attached to a window");
#endif

	if (!(wid->flags & (AG_WIDGET_HIDE|AG_WIDGET_UNDERSIZE)) &&
	    !OccultedWidget(wid) &&
	    WIDGET_OPS(wid)->draw != NULL) {
		WIDGET_OPS(wid)->draw(wid);
#ifdef AG_DEBUG
		if (wid->flags & AG_WIDGET_DEBUG_RSENS) {
			AG_Rect r = AG_Rect2ToRect(wid->rSens);

/*			if (r.x != wid->rView.x1 || r.y != wid->rView.y1 || */
/*			    r.w != wid->rView.w || r.h != wid->rView.h) { */
				r.x -= wid->rView.x1;
				r.y -= wid->rView.y1;
				AG_DrawRectBlended(wid, r,
				    AG_ColorRGBA(200,0,0,25),
				    AG_ALPHA_SRC);
				AG_DrawRectOutline(wid, r, AG_ColorRGB(100,0,0));
/*			} */
		}
#endif /* AG_DEBUG */
	}
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
AG_WidgetSizeReq(void *w, AG_SizeReq *r)
{
	r->w = 0;
	r->h = 0;

	AG_ObjectLock(w);
	if (WIDGET_OPS(w)->size_request != NULL) {
		WIDGET_OPS(w)->size_request(w, r);
	}
	AG_ObjectUnlock(w);
}

int
AG_WidgetSizeAlloc(void *obj, AG_SizeAlloc *a)
{
	AG_Widget *w = obj;

	AG_ObjectLock(w);

	if (a->w <= 0 || a->h <= 0) {
		a->w = 0;
		a->h = 0;
		w->flags |= AG_WIDGET_UNDERSIZE;
	}
	w->x = a->x;
	w->y = a->y;
	w->w = a->w;
	w->h = a->h;

	if (WIDGET_OPS(w)->size_allocate != NULL) {
		if (WIDGET_OPS(w)->size_allocate(w, a) == -1) {
			w->flags |= AG_WIDGET_UNDERSIZE;
			goto fail;
		} else {
			w->flags &= ~(AG_WIDGET_UNDERSIZE);
		}
	}

	AG_ObjectUnlock(w);
	return (0);
fail:
	AG_ObjectUnlock(w);
	return (-1);
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
		if (AG_ObjectIsClass(wtParent, "AG_Widget:AG_Window:*")) {
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

	if (!AG_OfClass(wid, "AG_Widget:AG_Window:*") &&
	    (wid->flags & AG_WIDGET_FOCUSED) == 0) {
		goto fail;
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
	char spec[AG_SIZE_SPEC_MAX], *p;
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
	default:
		break;
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

/*
 * Raise `widget-shown' on a widget and its children.
 * Used by containers such as Notebook.
 */
void
AG_WidgetShownRecursive(void *p)
{
	AG_Widget *wid = p;
	AG_Widget *chld;

	AG_LockVFS(wid);
	AG_ObjectLock(wid);

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_WidgetShownRecursive(chld);
	}
	AG_PostEvent(NULL, wid, "widget-shown", NULL);
	
	AG_ObjectUnlock(wid);
	AG_UnlockVFS(wid);
}

/*
 * Raise `widget-hidden' on a widget and its children.
 * Used by containers such as Notebook.
 */
void
AG_WidgetHiddenRecursive(void *p)
{
	AG_Widget *wid = p;
	AG_Widget *chld;

	AG_LockVFS(wid);
	AG_ObjectLock(wid);
	
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_WidgetHiddenRecursive(chld);
	}
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
	if (!(parent->flags & AG_WIDGET_HIDE) &&
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
		    (wid->nsurfaces+1)*sizeof(GLuint));
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

/* replace the contents of a mapped surface. */
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