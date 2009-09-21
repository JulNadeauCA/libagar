/*
 * Copyright (c) 2001-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

SDL_Cursor  *agCursorToSet = NULL;	/* Set cursor at end of event cycle */

/* Set the parent window pointer on a widget and its children. */
static void
SetParentWindow(AG_Widget *wid, AG_Window *win)
{
	AG_Widget *chld;

	wid->window = win;
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		SetParentWindow(chld, win);
}

/* Set the style pointer on a widget and its children. */
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
	AG_Widget *wParent = AG_SENDER();
	AG_Widget *w = AG_SELF();

	/* Ignore AG_Window attaching to View */
	if (!AG_OfClass(wParent, "AG_Widget:*"))
		return;
	
	/* Inherit style from parent widget/window. */
	if (wParent->style != NULL)
		SetStyle(w, wParent->style);

	if (AG_OfClass(wParent, "AG_Widget:AG_Window:*")) {
		AG_Window *win = AGWINDOW(wParent);

		/* Update the "window" pointer of widget and children. */
		SetParentWindow(w, win);

		/* Request a geometry update in the parent window. */
		if (win->visible) {
			w->flags |= AG_WIDGET_UPDATE_WINDOW;
			AG_PostEvent(NULL, w, "widget-shown", NULL);
		}
	} else {
		SetParentWindow(w, wParent->window);
	}
}

static void
OnDetach(AG_Event *event)
{
	AG_Widget *wParent = AG_SENDER();
	AG_Widget *w = AG_SELF();
	
	if (!AG_OfClass(wParent, "AG_Widget:*"))
		return;				/* Attaching to agView */

	SetParentWindow(w, NULL);
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

	wid->nsurfaces = 0;
	wid->surfaces = NULL;
	wid->surfaceFlags = NULL;
#ifdef HAVE_OPENGL
	wid->textures = NULL;
	wid->texcoords = NULL;
	wid->textureGC = NULL;
	wid->nTextureGC = 0;
#endif
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

/* Run any action tied to a mouse-button-down event. */
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

#ifdef AG_DEBUG
	if (et != AG_ACTION_ON_KEYDOWN &&
	    et != AG_ACTION_ON_KEYUP)
		AG_FatalError("Invalid type arg to AG_ExecKeyAction()");
#endif
	for (i = 0; i < wid->nKeyActions; i++) {
		at = &wid->keyActions[i];
		if (at->type != et) {
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
	    a == NULL) {
		return (0);
	}
	return AG_ExecAction(wid, a);
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
	if (AG_OfClass(parent, "AG_Display:*")) {
		AG_Display *disp = (AG_Display *)parent;
		AG_Window *win;

		VIEW_FOREACH_WINDOW(win, disp) {
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
 * View VFS is locked.
 *
 * This works differently than the general AG_ObjectFind() routine in
 * that the search may include widgets not effectively attached to the
 * View VFS, such as widgets attached to AG_Notebook(3) tabs.
 */
void *
AG_WidgetFind(AG_Display *view, const char *name)
{
	void *rv;

#ifdef AG_DEBUG
	if (name[0] != '/')
		AG_FatalError("WidgetFind: Not an absolute path: %s", name);
#endif
	AG_LockVFS(view);
	rv = WidgetFindPath(OBJECT(view), &name[1]);
	AG_UnlockVFS(view);
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

/*
 * Register a surface with the given widget. In OpenGL mode, a texture will
 * be generated when the surface is first blitted.
 *
 * The surface is not duplicated, but will be freed automatically along
 * with other widget data unless the NODUP flag is set.
 *
 * Safe to call in any context, but the returned name is only valid as long
 * as the widget is locked.
 */
int
AG_WidgetMapSurface(void *p, AG_Surface *su)
{
	AG_Widget *wid = p;
	int i, idx = -1;
		
	AG_ObjectLock(wid);

	for (i = 0; i < wid->nsurfaces; i++) {
		if (wid->surfaces[i] == NULL) {
			idx = i;
			break;
		}
	}
	if (i == wid->nsurfaces) {
		wid->surfaces = Realloc(wid->surfaces,
		    (wid->nsurfaces+1)*sizeof(AG_Surface *));
		wid->surfaceFlags = Realloc(wid->surfaceFlags,
		    (wid->nsurfaces+1)*sizeof(Uint));
#ifdef HAVE_OPENGL
		if (agView->opengl) {
			wid->textures = Realloc(wid->textures,
			    (wid->nsurfaces+1)*sizeof(GLuint));
			wid->texcoords = Realloc(wid->texcoords,
			    (wid->nsurfaces+1)*sizeof(GLfloat)*4);
		}
#endif
		idx = wid->nsurfaces++;
	}
	wid->surfaces[idx] = su;
	wid->surfaceFlags[idx] = 0;
#ifdef HAVE_OPENGL
	if (agView->opengl)
		wid->textures[idx] = 0;
#endif
	AG_ObjectUnlock(wid);
	return (idx);
}

/*
 * Variant of WidgetMapSurface() that sets the NODUP flag such that
 * the surface is not freed automatically with the widget.
 *
 * Safe to call in any context, but the returned name is only valid as long
 * as the widget is locked.
 */
int
AG_WidgetMapSurfaceNODUP(void *p, AG_Surface *su)
{
	AG_Widget *wid = p;
	int name;

	AG_ObjectLock(wid);
	name = AG_WidgetMapSurface(wid, su);
	wid->surfaceFlags[name] |= AG_WIDGET_SURFACE_NODUP;
	AG_ObjectUnlock(wid);
	return (name);
}

/*
 * Replace the contents of a mapped surface. Unless NODUP is set, the current
 * source surface is freed.
 *
 * This is safe to call in any context, with the drawback that in OpenGL mode,
 * the previous texture cannot be deleted immediately and is instead queued
 * for future deletion within rendering context.
 */
void
AG_WidgetReplaceSurface(void *p, int name, AG_Surface *su)
{
	AG_Widget *wid = p;

	AG_ObjectLock(wid);
	if (wid->surfaces[name] != NULL) {
		if (!WSURFACE_NODUP(wid,name))
			AG_SurfaceFree(wid->surfaces[name]);
	}
	wid->surfaces[name] = su;
	wid->surfaceFlags[name] &= ~(AG_WIDGET_SURFACE_NODUP);
#ifdef HAVE_OPENGL
	if (agView->opengl && wid->textures[name] != 0) {
		wid->textureGC = Realloc(wid->textureGC, (wid->nTextureGC+1)*
		                                         sizeof(Uint));
		wid->textureGC[wid->nTextureGC++] = wid->textures[name];
		wid->textures[name] = 0;	/* Will be regenerated */
	}
#endif
	AG_ObjectUnlock(wid);
}

/* Variant of WidgetReplaceSurface() that sets the NODUP flag. */
void
AG_WidgetReplaceSurfaceNODUP(void *p, int name, AG_Surface *su)
{
	AG_Widget *wid = p;

	AG_ObjectLock(wid);
	AG_WidgetReplaceSurface(wid, name, su);
	wid->surfaceFlags[name] |= AG_WIDGET_SURFACE_NODUP;
	AG_ObjectUnlock(wid);
}

#ifdef HAVE_OPENGL
/* Remove surfaces which were previously queued for deletion. */
static void
DeleteQueuedTextures(AG_Widget *wid)
{
	Uint i;

	for (i = 0; i < wid->nTextureGC; i++) {
		glDeleteTextures(1, (GLuint *)&wid->textureGC[i]);
	}
	wid->nTextureGC = 0;
}
#endif

/*
 * NOTE: Texture operations are involved so this is only safe to invoke
 * in rendering context. This is safe, since garbage collection of detached
 * widgets only occurs at the end of the event processing cycle.
 */
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
	 * Free the surfaces; delete associated textures if we are running in
	 * OpenGL mode.
	 */
#ifdef HAVE_OPENGL
	if (wid->textureGC > 0)
		DeleteQueuedTextures(wid);
#endif
	for (i = 0; i < wid->nsurfaces; i++) {
		if (wid->surfaces[i] != NULL && !WSURFACE_NODUP(wid,i))
			AG_SurfaceFree(wid->surfaces[i]);
#ifdef HAVE_OPENGL
		if (agView->opengl) {
			if (wid->textures[i] != 0) {
				glDeleteTextures(1,
				    (GLuint *)&wid->textures[i]);
			}
		}
#endif
	}
	Free(wid->surfaces);
	Free(wid->surfaceFlags);
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		Free(wid->textures);
		Free(wid->texcoords);
		Free(wid->textureGC);
	}
#endif
}

/*
 * Perform a software blit from a source surface to the display, at
 * coordinates relative to the widget, using clipping.
 *
 * Only safe to call from rendering context.
 * XXX glDrawPixels() is probably faster.
 */
void
AG_WidgetBlit(void *p, AG_Surface *srcsu, int wx, int wy)
{
	AG_Widget *wid = p;
	int x = wid->rView.x1 + wx;
	int y = wid->rView.y1 + wy;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		GLuint texture;
		GLfloat texcoord[4];
		int alpha = (srcsu->flags & (AG_SRCALPHA|AG_SRCCOLORKEY));
		GLboolean blend_sv;
		GLint blend_sfactor, blend_dfactor;
		GLfloat texenvmode;

		texture = AG_SurfaceTexture(srcsu, texcoord);

		glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
		if (alpha) {
			glGetBooleanv(GL_BLEND, &blend_sv);
			glGetIntegerv(GL_BLEND_SRC, &blend_sfactor);
			glGetIntegerv(GL_BLEND_DST, &blend_dfactor);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_TRIANGLE_STRIP);
		{
			glTexCoord2f(texcoord[0], texcoord[1]);
			glVertex2i(x, y);
			glTexCoord2f(texcoord[2], texcoord[1]);
			glVertex2i(x+srcsu->w, y);
			glTexCoord2f(texcoord[0], texcoord[3]);
			glVertex2i(x, y+srcsu->h);
			glTexCoord2f(texcoord[2], texcoord[3]);
			glVertex2i(x+srcsu->w, y+srcsu->h);
		}
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &texture);

		if (alpha) {
			if (blend_sv) {
				glEnable(GL_BLEND);
			} else {
				glDisable(GL_BLEND);
			}
			glBlendFunc(blend_sfactor, blend_dfactor);
		}
		
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenvmode);
	} else
#endif /* HAVE_OPENGL */
	{
		AG_SurfaceBlit(srcsu, NULL, agView->v, x,y);
	}
}

#ifdef HAVE_OPENGL
static __inline__ void
UpdateTexture(AG_Widget *wid, int name)
{
	if (wid->textures[name] == 0) {
		wid->textures[name] = AG_SurfaceTexture(wid->surfaces[name],
		    &wid->texcoords[name*4]);
	} else if (wid->surfaceFlags[name] & AG_WIDGET_SURFACE_REGEN) {
		wid->surfaceFlags[name] &= ~(AG_WIDGET_SURFACE_REGEN);
		AG_UpdateTexture(wid->surfaces[name], wid->textures[name]);
	}
}
#endif

/*
 * Perform a hardware or software blit from a mapped surface to the display
 * at coordinates relative to the widget, using clipping.
 * 
 * Only safe to call from rendering context.
 */
void
AG_WidgetBlitFrom(void *pDst, void *pSrc, int name, AG_Rect *rSrc,
    int wx, int wy)
{
	AG_Widget *wDst = pDst;
	AG_Widget *wSrc = pSrc;
	AG_Surface *su = wSrc->surfaces[name];
	int x, y;

	if (name == -1 || su == NULL)
		return;

	x = wDst->rView.x1 + wx;
	y = wDst->rView.y1 + wy;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		GLfloat tmptexcoord[4];
		GLfloat *texcoord;
		GLboolean blend_sv;
		GLint blend_sfactor, blend_dfactor;
		GLfloat texenvmode;
		int alpha = su->flags & (AG_SRCALPHA|AG_SRCCOLORKEY);

		UpdateTexture(wSrc, name);

		if (rSrc == NULL) {
			texcoord = &wSrc->texcoords[name*4];
		} else {
			texcoord = &tmptexcoord[0];
			texcoord[0] = (GLfloat)rSrc->x/PowOf2i(rSrc->x);
			texcoord[1] = (GLfloat)rSrc->y/PowOf2i(rSrc->y);
			texcoord[2] = (GLfloat)rSrc->w/PowOf2i(rSrc->w);
			texcoord[3] = (GLfloat)rSrc->h/PowOf2i(rSrc->h);
		}

		glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		if (alpha) {
			glGetBooleanv(GL_BLEND, &blend_sv);
			glGetIntegerv(GL_BLEND_SRC, &blend_sfactor);
			glGetIntegerv(GL_BLEND_DST, &blend_dfactor);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	
		glBindTexture(GL_TEXTURE_2D, wSrc->textures[name]);
		glBegin(GL_TRIANGLE_STRIP);
		{
			glTexCoord2f(texcoord[0], texcoord[1]);
			glVertex2i(x, y);
			glTexCoord2f(texcoord[2], texcoord[1]);
			glVertex2i(x+su->w, y);
			glTexCoord2f(texcoord[0], texcoord[3]);
			glVertex2i(x, y+su->h);
			glTexCoord2f(texcoord[2], texcoord[3]);
			glVertex2i(x+su->w, y+su->h);
		}
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);

		if (alpha) {
			if (blend_sv) {
				glEnable(GL_BLEND);
			} else {
				glDisable(GL_BLEND);
			}
			glBlendFunc(blend_sfactor, blend_dfactor);
		}
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenvmode);
	} else
#endif /* HAVE_OPENGL */
	{
		AG_SurfaceBlit(su, rSrc, agView->v, x,y);
#ifdef DEBUG_CLIPPING
		{
			AG_Rect rClip;

			rClip.x = agView->v->clip_rect.x - wDst->rView.x1;
			rClip.y = agView->v->clip_rect.y - wDst->rView.y1;
			rClip.w = agView->v->clip_rect.w;
			rClip.h = agView->v->clip_rect.h;
			AG_DrawRectOutline(wDst, rClip,
			    AG_MapRGB(agVideoFmt,0,0,0));
		}
#endif
	}
}

#ifdef HAVE_OPENGL
/*
 * OpenGL-only version of AG_WidgetBlit() without explicit source or
 * destination rectangle parameter.
 *
 * Only safe to call from rendering context.
 * XXX glDrawPixels() is probably faster.
 */
void
AG_WidgetBlitGL(void *pWidget, AG_Surface *su, float w, float h)
{
	GLuint texname;
	GLfloat texcoord[4];
	GLboolean blend_sv;
	GLint blend_sfactor, blend_dfactor;
	GLfloat texenvmode;
	int alpha = su->flags & (AG_SRCALPHA|AG_SRCCOLORKEY);
	float w2 = w/2.0f;
	float h2 = h/2.0f;

	texname = AG_SurfaceTexture(su, texcoord);
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	if (alpha) {
		glGetBooleanv(GL_BLEND, &blend_sv);
		glGetIntegerv(GL_BLEND_SRC, &blend_sfactor);
		glGetIntegerv(GL_BLEND_DST, &blend_dfactor);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	
	glBindTexture(GL_TEXTURE_2D, texname);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(texcoord[0],	texcoord[1]);
		glVertex2f(w2,			h2);
		glTexCoord2f(texcoord[2],	texcoord[1]);
		glVertex2f(-w2,			h2);
		glTexCoord2f(texcoord[0],	texcoord[3]);
		glVertex2f(w2,			-h2);
		glTexCoord2f(texcoord[2],	texcoord[3]);
		glVertex2f(-w2,			-h2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &texname);

	if (alpha) {
		if (blend_sv) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		glBlendFunc(blend_sfactor, blend_dfactor);
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenvmode);
}

/*
 * OpenGL-only version of AG_WidgetBlitSurface() without explicit
 * source or destination rectangle parameter.
 */
void
AG_WidgetBlitSurfaceGL(void *pWidget, int name, float w, float h)
{
	AG_Widget *wid = pWidget;
	GLuint texname;
	GLfloat *texcoord;
	GLboolean blend_sv;
	GLint blend_sfactor, blend_dfactor;
	GLfloat texenvmode;
	AG_Surface *su = wid->surfaces[name];
	int alpha = su->flags & (AG_SRCALPHA|AG_SRCCOLORKEY);
	float w2 = w/2.0f;
	float h2 = h/2.0f;
	
	UpdateTexture(wid, name);

	texname = wid->textures[name];
	texcoord = &wid->texcoords[name*4];
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	if (alpha) {
		glGetBooleanv(GL_BLEND, &blend_sv);
		glGetIntegerv(GL_BLEND_SRC, &blend_sfactor);
		glGetIntegerv(GL_BLEND_DST, &blend_dfactor);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	
	glBindTexture(GL_TEXTURE_2D, texname);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(texcoord[0],	texcoord[1]);
		glVertex2f(w2,			h2);
		glTexCoord2f(texcoord[2],	texcoord[1]);
		glVertex2f(-w2,			h2);
		glTexCoord2f(texcoord[0],	texcoord[3]);
		glVertex2f(w2,			-h2);
		glTexCoord2f(texcoord[2],	texcoord[3]);
		glVertex2f(-w2,			-h2);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	if (alpha) {
		if (blend_sv) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		glBlendFunc(blend_sfactor, blend_dfactor);
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenvmode);
}

/*
 * OpenGL-only version of AG_WidgetBlitSurface() without explicit
 * source or destination rectangle parameter (flipped).
 */
void
AG_WidgetBlitSurfaceFlippedGL(void *pWidget, int name, float w, float h)
{
	AG_Widget *wid = pWidget;
	GLuint texname;
	GLfloat *texcoord;
	GLboolean blend_sv;
	GLint blend_sfactor, blend_dfactor;
	GLfloat texenvmode;
	AG_Surface *su = wid->surfaces[name];
	int alpha = su->flags & (AG_SRCALPHA|AG_SRCCOLORKEY);
	
	UpdateTexture(wid, name);

	texname = wid->textures[name];
	texcoord = &wid->texcoords[name*4];
	glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	if (alpha) {
		glGetBooleanv(GL_BLEND, &blend_sv);
		glGetIntegerv(GL_BLEND_SRC, &blend_sfactor);
		glGetIntegerv(GL_BLEND_DST, &blend_dfactor);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	
	glBindTexture(GL_TEXTURE_2D, texname);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glTexCoord2f(texcoord[2],	texcoord[1]);
		glVertex2f(0.0,			0.0);
		glTexCoord2f(texcoord[0],	texcoord[1]);
		glVertex2f((GLfloat)w,		0.0);
		glTexCoord2f(texcoord[2],	texcoord[3]);
		glVertex2f(0.0,			(GLfloat)h);
		glTexCoord2f(texcoord[0],	texcoord[3]);
		glVertex2f((GLfloat)w,		(GLfloat)h);
	}
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);

	if (alpha) {
		if (blend_sv) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		glBlendFunc(blend_sfactor, blend_dfactor);
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texenvmode);
}

/* Emulate 32-bit putpixel behavior */
void
AG_WidgetPutPixel32_GL(void *p, int x, int y, Uint32 color)
{
	Uint8 r, g, b;

	AG_GetRGB(color, agVideoFmt, &r,&g,&b);
	glBegin(GL_POINTS);
	glColor3ub(r, g, b);
	glVertex2s(x, y);
	glEnd();
}

/* Emulate RGB putpixel behavior */
void
AG_WidgetPutPixelRGB_GL(void *p, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
	glBegin(GL_POINTS);
	glColor3ub(r, g, b);
	glVertex2s(x, y);
	glEnd();
}

/*
 * Release the OpenGL resources associated with a widget. If some textures
 * exist without a corresponding surface, allocate a software surface and
 * copy their contents to be later restored.
 * GL lock must be held.
 */
void
AG_WidgetFreeResourcesGL(AG_Widget *wid)
{
	AG_Surface *su;
	GLint w, h;
	Uint i;

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nsurfaces; i++)  {
		if (wid->textures[i] == 0 ||
		    wid->surfaces[i] != NULL) {
			continue;
		}
		glBindTexture(GL_TEXTURE_2D, (GLuint)wid->textures[i]);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,
		    &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT,
		    &h);

		su = AG_SurfaceRGBA(w, h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
			0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
			0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif
		);
		if (su == NULL) {
			AG_FatalError("Allocating texture: %s", AG_GetError());
		}
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		    su->pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
		wid->surfaces[i] = su;
	}
	glDeleteTextures(wid->nsurfaces, (GLuint *)wid->textures);
	memset(wid->textures, 0, wid->nsurfaces*sizeof(Uint));
	AG_ObjectUnlock(wid);
}

/*
 * Regenerate the OpenGL textures associated with a widget.
 * GL lock must be held.
 */
void
AG_WidgetRegenResourcesGL(AG_Widget *wid)
{
	Uint i;

	AG_ObjectLock(wid);
	for (i = 0; i < wid->nsurfaces; i++)  {
		if (wid->surfaces[i] != NULL) {
			wid->textures[i] = AG_SurfaceTexture(wid->surfaces[i],
			    &wid->texcoords[i*4]);
		} else {
			wid->textures[i] = 0;
		}
	}
	AG_ObjectUnlock(wid);
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
AG_WidgetFocus(void *p)
{
	AG_Widget *wid = p, *wParent = wid;
	AG_Window *win;

	AG_LockVFS(agView);
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
	VIEW_FOREACH_WINDOW(win, agView) {
		if (win->nFocused > 0)
			AG_WidgetUnfocus(win);
	}

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
	AG_UnlockVFS(agView);
	return (1);
fail:
	AG_ObjectUnlock(wid);
	AG_UnlockVFS(agView);
	return (0);
}

/*
 * Evaluate whether a given widget is at least partially visible.
 * The Widget and View must be locked.
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

void
AG_SetCursor(int cursor)
{
	AG_LockVFS(agView);
	if (agCursorToSet == NULL) {
		agCursorToSet = agCursors[cursor];
	}
	AG_UnlockVFS(agView);
}

void
AG_UnsetCursor(void)
{
	AG_LockVFS(agView);
	agCursorToSet = agDefaultCursor;
	AG_UnlockVFS(agView);
}

/*
 * Push a clipping rectangle onto the clipping rectangle stack.
 * Must be invoked from GUI rendering context.
 */
void
AG_PushClipRect(void *obj, AG_Rect r)
{
	AG_ClipRect *cr, *crPrev;

	r.x += WIDGET(obj)->rView.x1;
	r.y += WIDGET(obj)->rView.y1;

	agClipRects = Realloc(agClipRects, (agClipRectCount+1) *
	                                   sizeof(AG_ClipRect));
	crPrev = &agClipRects[agClipRectCount-1];
	cr = &agClipRects[agClipRectCount++];

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		cr->eqns[0][0] = 1.0;
		cr->eqns[0][1] = 0.0;
		cr->eqns[0][2] = 0.0;
		cr->eqns[0][3] = MIN(crPrev->eqns[0][3], -(double)(r.x));
		glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
		
		cr->eqns[1][0] = 0.0;
		cr->eqns[1][1] = 1.0;
		cr->eqns[1][2] = 0.0;
		cr->eqns[1][3] = MIN(crPrev->eqns[1][3], -(double)(r.y));
		glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
		
		cr->eqns[2][0] = -1.0;
		cr->eqns[2][1] = 0.0;
		cr->eqns[2][2] = 0.0;
		cr->eqns[2][3] = MIN(crPrev->eqns[2][3], (double)(r.x+r.w));
		glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
		
		cr->eqns[3][0] = 0.0;
		cr->eqns[3][1] = -1.0;
		cr->eqns[3][2] = 0.0;
		cr->eqns[3][3] = MIN(crPrev->eqns[3][3], (double)(r.y+r.h));
		glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
	} else
#endif
	{
		cr->r = AG_RectIntersect(&crPrev->r, &r);
		AG_SetClipRect(agView->v, &cr->r);
	}
}

/*
 * Pop a clipping rectangle off the clipping rectangle stack.
 * Must be invoked from GUI rendering context.
 */
void
AG_PopClipRect(void)
{
	AG_ClipRect *cr;

#ifdef AG_DEBUG
	if (agClipRectCount < 1)
		AG_FatalError("PopClipRect() without PushClipRect()");
#endif
	cr = &agClipRects[agClipRectCount-2];
	agClipRectCount--;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		glClipPlane(GL_CLIP_PLANE0, (const GLdouble *)&cr->eqns[0]);
		glClipPlane(GL_CLIP_PLANE1, (const GLdouble *)&cr->eqns[1]);
		glClipPlane(GL_CLIP_PLANE2, (const GLdouble *)&cr->eqns[2]);
		glClipPlane(GL_CLIP_PLANE3, (const GLdouble *)&cr->eqns[3]);
	} else
#endif
	{
		AG_SetClipRect(agView->v, &cr->r);
	}
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
#ifdef HAVE_OPENGL
	if (wid->textureGC > 0)
		DeleteQueuedTextures(wid);
#endif
	if (!(wid->flags & (AG_WIDGET_HIDE|AG_WIDGET_UNDERSIZE)) &&
	    !OccultedWidget(wid) &&
	    WIDGET_OPS(wid)->draw != NULL) {
		WIDGET_OPS(wid)->draw(wid);
#ifdef AG_DEBUG
		if (wid->flags & AG_WIDGET_DEBUG_RSENS) {
			static Uint8 c1[4] = { 200, 0, 0, 25 };
			AG_Rect r = AG_Rect2ToRect(wid->rSens);

/*			if (r.x != wid->rView.x1 || r.y != wid->rView.y1 || */
/*			    r.w != wid->rView.w || r.h != wid->rView.h) { */
				r.x -= wid->rView.x1;
				r.y -= wid->rView.y1;
				AG_DrawRectBlended(wid, r,
				    c1, AG_ALPHA_SRC);
				AG_DrawRectOutline(wid, r,
				    AG_MapRGB(agVideoFmt,100,0,0));
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
 * Blend with the pixel at widget-relative x,y coordinates, with clipping
 * to display area.
 *
 * Must be invoked from GUI rendering context. In SDL mode, the display
 * surface must be locked.
 */
void
AG_WidgetBlendPixelRGBA(void *p, int x, int y, Uint8 c[4], AG_BlendFn fn)
{
	AG_Widget *wid = p;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		GLboolean svBlendBit;
		GLint svBlendSrc, svBlendDst;

		glGetBooleanv(GL_BLEND, &svBlendBit);
		glGetIntegerv(GL_BLEND_SRC, &svBlendSrc);
		glGetIntegerv(GL_BLEND_DST, &svBlendDst);
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_ALPHA, GL_ZERO);

		switch (fn) {
		case AG_ALPHA_OVERLAY:
			/* XXX TODO emulate using glReadPixels()? */
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case AG_ALPHA_SRC:
			glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
			break;
		case AG_ALPHA_DST:
			glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
			break;
		case AG_ALPHA_ONE_MINUS_DST:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
			break;
		case AG_ALPHA_ONE_MINUS_SRC:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
		glBegin(GL_POINTS);
		glColor4ubv((GLubyte *)c);
		glVertex2s(wid->rView.x1 + x,
		           wid->rView.y1 + y);
		glEnd();

		if (!svBlendBit) {
			glDisable(GL_BLEND);
		}
		glBlendFunc(GL_SRC_ALPHA, svBlendSrc);
		glBlendFunc(GL_DST_ALPHA, svBlendDst);
	} else
#endif /* HAVE_OPENGL */
	{
		AG_BLEND_RGBA2_CLIPPED(agView->v,
		    wid->rView.x1 + x,
		    wid->rView.y1 + y,
		    c[0], c[1], c[2], c[3], fn);
	}
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
 * Post a mousemotion event to widgets that either hold focus or have the
 * UNFOCUSED_MOTION flag set. View must be locked.
 */
void
AG_WidgetMouseMotion(AG_Window *win, AG_Widget *wid, int x, int y,
    int xrel, int yrel, int state)
{
	AG_Widget *cwid;

	AG_ObjectLock(wid);
	if ((AG_WidgetIsFocusedInWindow(wid)) ||
	    (wid->flags & AG_WIDGET_UNFOCUSED_MOTION)) {
		AG_PostEvent(NULL, wid, "mouse-motion",
		    "%i(x),%i(y),%i(xRel),%i(yRel),%i(buttons)",
		    x - wid->rView.x1,
		    y - wid->rView.y1,
		    xrel,
		    yrel,
		    state);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid, "window-mousemotion",
		    "%i,%i,%i,%i,%i",
		    x - wid->rView.x1,
		    y - wid->rView.y1,
		    xrel,
		    yrel,
		    state);
#endif
		if (wid->flags & AG_WIDGET_PRIO_MOTION)
			goto out;
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
		AG_WidgetMouseMotion(win, cwid, x, y, xrel, yrel, state);
out:
	AG_ObjectUnlock(wid);
}

/*
 * Post a mousebuttonup event to widgets that either hold focus or have the
 * UNFOCUSED_BUTTONUP flag set. View must be locked.
 */
void
AG_WidgetMouseButtonUp(AG_Window *win, AG_Widget *wid, int button,
    int x, int y)
{
	AG_Widget *cwid;

	AG_ObjectLock(wid);
	if ((AG_WidgetIsFocusedInWindow(wid)) ||
	    (wid->flags & AG_WIDGET_UNFOCUSED_BUTTONUP)) {
		AG_PostEvent(NULL, wid, "mouse-button-up",
		    "%i(button),%i(x),%i(y)",
		    button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid, "window-mousebuttonup",
		    "%i,%i,%i",
		    button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
#endif
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		AG_WidgetMouseButtonUp(win, cwid, button, x, y);
	}
	AG_ObjectUnlock(wid);
}

/* Process a mouse-button-down event. View must be locked. */
int
AG_WidgetMouseButtonDown(AG_Window *win, AG_Widget *wid, int button,
    int x, int y)
{
	AG_Widget *cwid;
	AG_Event *ev;
	
	AG_ObjectLock(wid);

	/* Search for a better match. */
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		if (AG_WidgetMouseButtonDown(win, cwid, button, x, y))
			goto match;
	}
	if (!AG_WidgetSensitive(wid, x, y)) {
		goto out;
	}
	TAILQ_FOREACH(ev, &OBJECT(wid)->events, events) {
		if (strcmp(ev->name, "mouse-button-down") == 0)
			break;
#ifdef AG_LEGACY
		if (strcmp(ev->name, "window-mousebuttondown") == 0)
			break;
#endif
	}
	if (ev != NULL) {
		AG_PostEvent(NULL, wid, "mouse-button-down",
		    "%i(button),%i(x),%i(y)",
		    button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid, "window-mousebuttondown",
		    "%i,%i,%i",
		    button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
#endif
		goto match;
	}
out:
	AG_ObjectUnlock(wid);
	return (0);
match:
	AG_ObjectUnlock(wid);
	return (1);
}

/*
 * Post a keyup event to widgets with the UNFOCUSED_KEYUP flag set.
 * View must be locked.
 */
void
AG_WidgetUnfocusedKeyUp(AG_Widget *wid, int ksym, int kmod, int unicode)
{
	AG_Widget *cwid;

	AG_ObjectLock(wid);
	if (wid->flags & AG_WIDGET_UNFOCUSED_KEYUP) {
		AG_PostEvent(NULL, wid,  "key-up",
		    "%i(key),%i(mod),%i(unicode)",
		    ksym, kmod, unicode);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid,  "window-keyup",
		    "%i,%i,%i",
		    ksym, kmod, unicode);
#endif
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		AG_WidgetUnfocusedKeyUp(cwid, ksym, kmod, unicode);
	}
	AG_ObjectUnlock(wid);
}

/*
 * Post a keydown event to widgets with the UNFOCUSED_KEYDOWN flag set.
 * View must be locked.
 */
void
AG_WidgetUnfocusedKeyDown(AG_Widget *wid, int ksym, int kmod, int unicode)
{
	AG_Widget *cwid;

	AG_ObjectLock(wid);
	if (wid->flags & AG_WIDGET_UNFOCUSED_KEYDOWN) {
		AG_PostEvent(NULL, wid,  "key-down",
		    "%i(key),%i(mod),%i(unicode)",
		    ksym, kmod, unicode);
#ifdef AG_LEGACY
		AG_PostEvent(NULL, wid,  "window-keydown",
		    "%i,%i,%i",
		    ksym, kmod, unicode);
#endif
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		AG_WidgetUnfocusedKeyDown(cwid, ksym, kmod, unicode);
	}
	AG_ObjectUnlock(wid);
}

/*
 * Search for a focused widget inside a window. Return value is only valid
 * as long as the View VFS is locked.
 */
AG_Widget *
AG_WidgetFindFocused(void *p)
{
	AG_Widget *wid = p;
	AG_Widget *cwid, *fwid;

	AG_LockVFS(agView);
	AG_ObjectLock(wid);

	if (!AG_OfClass(wid, "AG_Widget:AG_Window:*") &&
	    (wid->flags & AG_WIDGET_FOCUSED) == 0) {
		goto fail;
	}
	/* Search for a better match. */
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		if ((fwid = AG_WidgetFindFocused(cwid)) != NULL) {
			AG_ObjectUnlock(wid);
			AG_UnlockVFS(agView);
			return (fwid);
		}
	}

	AG_ObjectUnlock(wid);
	AG_UnlockVFS(agView);
	return (wid);
fail:
	AG_ObjectUnlock(wid);
	AG_UnlockVFS(agView);
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
	Uint32 t2 = SDL_GetTicks();
	int delta;

	if (*t1 != 0 && ((delta = (t2 - *t1))) < 250) {
		return (((250-delta)<<3)>>9);
	}
	*t1 = SDL_GetTicks();
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

	AG_LockVFS(agView);
	AG_ObjectLock(wid);

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_WidgetShownRecursive(chld);
	}
	AG_PostEvent(NULL, wid, "widget-shown", NULL);
	
	AG_ObjectUnlock(wid);
	AG_UnlockVFS(agView);
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

	AG_LockVFS(agView);
	AG_ObjectLock(wid);
	
	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_WidgetHiddenRecursive(chld);
	}
	AG_PostEvent(NULL, wid, "widget-hidden", NULL);
	
	AG_ObjectUnlock(wid);
	AG_UnlockVFS(agView);
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

/*
 * Search for widgets of the specified class enclosing the given point.
 * Result is only accurate as long as the View VFS is locked.
 */
void *
AG_WidgetFindPoint(const char *type, int x, int y)
{
	AG_Window *win;
	void *p;

	AG_LockVFS(agView);
	VIEW_FOREACH_WINDOW_REVERSE(win, agView) {
		if ((p = FindAtPoint(WIDGET(win), type, x, y)) != NULL) {
			AG_UnlockVFS(agView);
			return (p);
		}
	}
	AG_UnlockVFS(agView);
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
 * Result is only accurate as long as the View VFS is locked.
 */
void *
AG_WidgetFindRect(const char *type, int x, int y, int w, int h)
{
	AG_Window *win;
	void *p;
	
	AG_LockVFS(agView);
	VIEW_FOREACH_WINDOW_REVERSE(win, agView) {
		if ((p = FindRectOverlap(WIDGET(win), type, x,y,w,h)) != NULL) {
			AG_UnlockVFS(agView);
			return (p);
		}
	}
	AG_UnlockVFS(agView);
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
	int visiblePrev;

	if (wid->window == NULL) {
		AG_SetError("Widget is unattached");
		return (NULL);
	}
	AG_LockVFS(agView);
	AG_BeginRendering();
	visiblePrev = wid->window->visible;
	wid->window->visible = 1;
	AG_WindowDraw(wid->window);
	wid->window->visible = visiblePrev;
	AG_EndRendering();

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		Uint8 *pixels;

		if ((pixels = AG_TryMalloc(wid->w*wid->h*4)) == NULL) {
			goto fail;
		}
		glReadPixels(
		    wid->rView.x1,
		    agView->h - wid->rView.y2,
		    wid->w,
		    wid->h,
		    GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		AG_FlipSurface(pixels, wid->h, wid->w*4);
		su = AG_SurfaceFromPixelsRGBA(pixels, wid->w, wid->h, 32,
		    (wid->w*4), 0x000000ff, 0x0000ff00, 0x00ff0000, 0);
		if (su == NULL) {
			free(pixels);
			goto fail;
		}
	} else
#endif /* HAVE_OPENGL */
	{
		AG_Rect r;
		if ((su = AG_SurfaceStdRGB(wid->w, wid->h)) == NULL) {
			goto fail;
		}
		r = AG_Rect2ToRect(wid->rView);
		AG_SurfaceBlit(agView->v, &r, su, 0, 0);
	}
	AG_UnlockVFS(agView);
	return (su);
fail:
	AG_UnlockVFS(agView);
	return (NULL);
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
