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

static void
ChildAttached(AG_Event *event)
{
	AG_Widget *pwid = AG_SELF();
	AG_Widget *wid = AG_SENDER();
	AG_Style *style = pwid->style;

	if (AG_OfClass(pwid, "AG_Widget:AG_Window:*"))
		wid->window = AGWINDOW(pwid);

	/* Inherit style from parent widget. */
	if (style != NULL)
		wid->style = style;
}

static void
ChildDetached(AG_Event *event)
{
	AG_Widget *pwid = AG_SELF();
	AG_Widget *wid = AG_SENDER();
	AG_Style *style = pwid->style;

	if (AG_OfClass(pwid, "AG_Widget:AG_Window:*"))
		wid->window = NULL;

	/* Inherit style from parent widget. */
	if (style != NULL)
		wid->style = style;
}

#ifdef AG_LEGACY
/*
 * Forward object-level `bound' event to `widget-bound'.
 */
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

	AG_SetEvent(wid, "child-attached", ChildAttached, NULL);
	AG_SetEvent(wid, "child-detached", ChildDetached, NULL);
#ifdef AG_LEGACY
	AG_SetEvent(wid, "bound", Bound, NULL);
#endif
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

		TAILQ_FOREACH(win, &disp->windows, windows) {
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

#ifdef AG_LEGACY
/*
 * Duplicate a widget binding.
 * LEGACY Interface.
 */
int
AG_WidgetCopyBinding(void *wDst, const char *nDst, AG_Variable *Vsrc)
{
	AG_Variable *Vdst;

	if ((Vdst = AG_GetVariable(wDst, nDst, NULL)) == NULL) {
		return (-1);
	}
	Vdst->type = Vsrc->type;
	Vdst->mutex = Vsrc->mutex;
	Vdst->data.p = Vsrc->data.p;

	switch (Vdst->type) {
	case AG_VARIABLE_P_FLAG:
	case AG_VARIABLE_P_FLAG8:
	case AG_VARIABLE_P_FLAG16:
	case AG_VARIABLE_P_FLAG32:
		Vdst->info.bitmask = Vsrc->info.bitmask;
		break;
	default:
		break;
	}
	if (AG_VARIABLE_TYPE(Vdst) == AG_VARIABLE_STRING) {
		Vdst->info.size = Vsrc->info.size;
	}

	AG_PostEvent(NULL, wDst, "bound", "%p", Vdst);
	AG_UnlockVariable(Vdst);
	return (0);
}

/*
 * Bind a mutex-protected variable to a widget.
 * LEGACY Interface: new code should use AG_Variable(3).
 */
AG_Variable *
AG_WidgetBindMp(void *obj, const char *name, AG_Mutex *mutex,
    enum ag_variable_type type, ...)
{
	AG_Widget *wid = obj;
	AG_Variable *V;
	va_list ap;
	void *p = NULL;
	Uint bitmask = 0;
	size_t size = 0;
	
	AG_ObjectLock(wid);

	va_start(ap, type);
	switch (type) {
	case AG_VARIABLE_P_FLAG:
	case AG_VARIABLE_P_FLAG8:
	case AG_VARIABLE_P_FLAG16:
	case AG_VARIABLE_P_FLAG32:
		p = va_arg(ap, void *);
		bitmask = va_arg(ap, Uint);
		break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_P_STRING:
	case AG_VARIABLE_CONST_STRING:
	case AG_VARIABLE_P_CONST_STRING:
		p = (void *)va_arg(ap, char *);
		size = va_arg(ap, size_t);
		break;
	default:
		p = va_arg(ap, void *);
		break;
	}
	va_end(ap);
	
	switch (type) {
	case AG_VARIABLE_P_INT:
		V = AG_BindInt(wid, name, (int *)p);
		break;
	case AG_VARIABLE_P_UINT8:
		V = AG_BindUint8(wid, name, (Uint8 *)p);
		break;
	case AG_VARIABLE_P_SINT8:
		V = AG_BindSint8(wid, name, (Sint8 *)p);
		break;
	case AG_VARIABLE_P_UINT16:
		V = AG_BindUint16(wid, name, (Uint16 *)p);
		break;
	case AG_VARIABLE_P_SINT16:
		V = AG_BindSint16(wid, name, (Sint16 *)p);
		break;
	case AG_VARIABLE_P_UINT32:
		V = AG_BindUint32(wid, name, (Uint32 *)p);
		break;
	case AG_VARIABLE_P_SINT32:
		V = AG_BindSint32(wid, name, (Sint32 *)p);
		break;
	case AG_VARIABLE_P_FLOAT:
		V = AG_BindFloat(wid, name, (float *)p);
		break;
	case AG_VARIABLE_P_DOUBLE:
		V = AG_BindDouble(wid, name, (double *)p);
		break;
	case AG_VARIABLE_P_FLAG:
		V = AG_BindFlag(wid, name, (Uint *)p, bitmask);
		break;
	case AG_VARIABLE_P_FLAG8:
		V = AG_BindFlag8(wid, name, (Uint8 *)p, (Uint8)bitmask);
		break;
	case AG_VARIABLE_P_FLAG16:
		V = AG_BindFlag16(wid, name, (Uint16 *)p, (Uint16)bitmask);
		break;
	case AG_VARIABLE_P_FLAG32:
		V = AG_BindFlag32(wid, name, (Uint32 *)p, (Uint32)bitmask);
		break;
	case AG_VARIABLE_P_STRING:
	case AG_VARIABLE_P_CONST_STRING:
		V = AG_BindString(wid, name, (char *)p, size);
		break;
	default:
		AG_ObjectUnlock(wid);
		return (NULL);
	}
	V->mutex = mutex;

	AG_ObjectUnlock(wid);
	return (V);
}

/*
 * Bind a non mutex-protected variable to a widget.
 * LEGACY Interface: new code should use AG_Variable(3).
 */
AG_Variable *
AG_WidgetBind(void *pObj, const char *name, enum ag_variable_type type, ...)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	va_list ap;
	Uint i;

	AG_ObjectLock(obj);

	for (i = 0; i < obj->nVars; i++) {
		V = &obj->vars[i];
		if (strcmp(obj->vars[i].name, name) == 0)
			break;
	}
	if (i == obj->nVars) {			/* Create new binding */
		obj->vars = Realloc(obj->vars,
		    (obj->nVars+1)*sizeof(AG_Variable));
		V = &obj->vars[obj->nVars++];
		Strlcpy(V->name, name, sizeof(V->name));
	}
	V->type = type;
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	
	va_start(ap, type);
	switch (type) {
	case AG_VARIABLE_P_FLAG:
	case AG_VARIABLE_P_FLAG8:
	case AG_VARIABLE_P_FLAG16:
	case AG_VARIABLE_P_FLAG32:
		V->data.p = va_arg(ap, void *);
		V->info.bitmask = va_arg(ap, Uint);
		break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_P_STRING:
	case AG_VARIABLE_CONST_STRING:
	case AG_VARIABLE_P_CONST_STRING:
		V->data.p = va_arg(ap, char *);
		V->info.size = va_arg(ap, size_t);
		break;
	default:
		V->data.p = va_arg(ap, void *);
		V->info.bitmask = 0;
		break;
	}
	va_end(ap);

	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}

/*
 * LEGACY Interfaces: new code should use AG_Variable(3).
 */
size_t
AG_WidgetCopyString(void *wid, const char *name, char *dst, size_t dst_size)
{
	AG_Variable *V;
	char *s;
	size_t rv;

	if ((V = AG_GetVariable(wid, name, &s)) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	rv = Strlcpy(dst, s, dst_size);
	AG_UnlockVariable(V);
	return (rv);
}
int
AG_WidgetInt(void *wid, const char *name)
{
	AG_Variable *b;
	int *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Uint
AG_WidgetUint(void *wid, const char *name)
{
	AG_Variable *b;
	Uint *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Uint8
AG_WidgetUint8(void *wid, const char *name)
{
	AG_Variable *b;
	Uint8 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Sint8
AG_WidgetSint8(void *wid, const char *name)
{
	AG_Variable *b;
	Sint8 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Uint16
AG_WidgetUint16(void *wid, const char *name)
{
	AG_Variable *b;
	Uint16 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Sint16
AG_WidgetSint16(void *wid, const char *name)
{
	AG_Variable *b;
	Sint16 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Uint32
AG_WidgetUint32(void *wid, const char *name)
{
	AG_Variable *b;
	Uint32 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Sint32
AG_WidgetSint32(void *wid, const char *name)
{
	AG_Variable *b;
	Sint32 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
float
AG_WidgetFloat(void *wid, const char *name)
{
	AG_Variable *b;
	float *f, rv;
	if ((b = AG_GetVariable(wid, name, &f)) == NULL) { AG_FatalError(NULL); }
	rv = *f;
	AG_UnlockVariable(b);
	return (rv);
}
double
AG_WidgetDouble(void *wid, const char *name)
{
	AG_Variable *b;
	double *d, rv;
	if ((b = AG_GetVariable(wid, name, &d)) == NULL) { AG_FatalError(NULL); }
	rv = *d;
	AG_UnlockVariable(b);
	return (rv);
}
char *
AG_WidgetString(void *wid, const char *name)
{
	AG_Variable *b;
	char *s, *sd;
	if ((b = AG_GetVariable(wid, name, &s)) == NULL) { AG_FatalError(NULL); }
	sd = AG_Strdup(s);
	AG_UnlockVariable(b);
	return (sd);
}
void *
AG_WidgetPointer(void *wid, const char *name)
{
	AG_Variable *b;
	void **p, *rv;
	if ((b = AG_GetVariable(wid, name, &p)) == NULL) { AG_FatalError(NULL); }
	rv = *p;
	AG_UnlockVariable(b);
	return (p);
}
void
AG_WidgetSetInt(void *wid, const char *name, int ni)
{
	AG_Variable *V;
	int *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetUint(void *wid, const char *name, Uint ni)
{
	AG_Variable *V;
	Uint *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetUint8(void *wid, const char *name, Uint8 ni)
{
	AG_Variable *V;
	Uint8 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetSint8(void *wid, const char *name, Sint8 ni)
{
	AG_Variable *V;
	Sint8 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetUint16(void *wid, const char *name, Uint16 ni)
{
	AG_Variable *V;
	Uint16 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetSint16(void *wid, const char *name, Sint16 ni)
{
	AG_Variable *V;
	Sint16 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetUint32(void *wid, const char *name, Uint32 ni)
{
	AG_Variable *V;
	Uint32 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetSint32(void *wid, const char *name, Sint32 ni)
{
	AG_Variable *V;
	Sint32 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetFloat(void *wid, const char *name, float nf)
{
	AG_Variable *V;
	float *f;
	if ((V = AG_GetVariable(wid, name, &f)) == NULL) { AG_FatalError(NULL); }
	*f = nf;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetDouble(void *wid, const char *name, double nd)
{
	AG_Variable *V;
	double *d;
	if ((V = AG_GetVariable(wid, name, &d)) == NULL) { AG_FatalError(NULL); }
	*d = nd;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetPointer(void *wid, const char *name, void *np)
{
	AG_Variable *V;
	void **p;
	if ((V = AG_GetVariable(wid, name, &p)) == NULL) { AG_FatalError(NULL); }
	*p = np;
	AG_UnlockVariable(V);
}
#endif /* AG_LEGACY */

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
 * in rendering context.
 */
static void
Destroy(void *obj)
{
	AG_Widget *wid = obj;
	AG_PopupMenu *pm, *pm2;
	Uint i;
	
	for (pm = SLIST_FIRST(&wid->menus);
	     pm != SLIST_END(&wid->menus);
	     pm = pm2) {
		pm2 = SLIST_NEXT(pm, menus);
		AG_PopupDestroy(NULL, pm);
	}

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
	AG_Window *winParent;

	w->flags |= AG_WIDGET_FOCUSED;
	if ((winParent = AG_ParentWindow(w)) != NULL) {
		AG_PostEvent(winParent, w, "widget-gainfocus", NULL);
		winParent->nFocused++;
	}
}

/* Give up widget focus */
static __inline__ void
UnfocusWidget(AG_Widget *w)
{
	AG_Window *winParent;

	w->flags &= ~(AG_WIDGET_FOCUSED);
	if ((winParent = AG_ParentWindow(w)) != NULL) {
		AG_PostEvent(winParent, w, "widget-lostfocus", NULL);
		winParent->nFocused--;
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
	TAILQ_FOREACH(win, &agView->windows, windows) {
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
	    (w = TAILQ_NEXT(wParent, windows)) == NULL) {
		return (0);
	}
	for (; w != TAILQ_END(&agView->windows); w = TAILQ_NEXT(w, windows)) {
		if (w->visible &&
		    wid->rView.x1 > WIDGET(w)->x &&
		    wid->rView.y1 > WIDGET(w)->y &&
		    wid->rView.x2 < WIDGET(w)->x+WIDGET(w)->w &&
		    wid->rView.y2 < WIDGET(w)->y+WIDGET(w)->h)
			return (1);
	}
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
		AG_PostEvent(NULL, wid, "window-mousemotion",
		    "%i,%i,%i,%i,%i",
		    x - wid->rView.x1,
		    y - wid->rView.y1,
		    xrel,
		    yrel,
		    state);
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
		AG_PostEvent(NULL, wid, "window-mousebuttonup", "%i,%i,%i",
		    button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		AG_WidgetMouseButtonUp(win, cwid, button, x, y);
	}
	AG_ObjectUnlock(wid);
}

/* Process a mousebuttondown event. View must be locked. */
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
		if (strcmp(ev->name, "window-mousebuttondown") == 0)
			break;
	}
	if (ev != NULL) {
		AG_PostEvent(NULL, wid, "window-mousebuttondown", "%i,%i,%i",
		    button,
		    x - wid->rView.x1,
		    y - wid->rView.y1);
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
		AG_PostEvent(NULL, wid,  "window-keyup", "%i, %i, %i",
		    ksym, kmod, unicode);
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
		AG_PostEvent(NULL, wid,  "window-keydown", "%i, %i, %i",
		    ksym, kmod, unicode);
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
	TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq, windows) {
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
	TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq, windows) {
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

	AG_LockVFS(agView);

	if (wid->window == NULL) {
		AG_SetError("Widget is unattached");
		goto fail;
	}
	AG_BeginRendering();
	visiblePrev = wid->window->visible;
	wid->window->visible = 1;
	AG_WindowDraw(wid->window);
	wid->window->visible = visiblePrev;
	AG_EndRendering();

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		Uint8 *pixels;

		if ((pixels = AG_TryMalloc(wid->w*wid->h*3)) == NULL) {
			goto fail;
		}
		glReadPixels(wid->rView.x1, wid->rView.y2, wid->w, wid->h,
		    GL_RGB, GL_UNSIGNED_BYTE, pixels);
		AG_FlipSurface(pixels, wid->h, wid->w*3);
		su = AG_SurfaceFromPixelsRGBA(pixels, wid->w, wid->h, 0, 24,
		    wid->w*3, 0x000000ff, 0x0000ff00, 0x00ff0000);
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
