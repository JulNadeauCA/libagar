/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/util.h>

#include "widget.h"
#include "window.h"
#include "cursors.h"
#include "menu.h"
#include "primitive.h"
#include "notebook.h"

#include <stdarg.h>
#include <string.h>

SDL_Cursor *agCursorToSet = NULL;

static void
ChildAttached(AG_Event *event)
{
	AG_Widget *pwid = AG_SELF();
	AG_Widget *wid = AG_SENDER();
	AG_Style *style = pwid->style;

	/* Inherit style from parent widget. */
	if (style != NULL)
		wid->style = style;
}

static void
Init(void *obj)
{
	AG_Widget *wid = obj;

	OBJECT(wid)->save_pfx = "/widgets";
	OBJECT(wid)->flags |= AG_OBJECT_NAME_ONATTACH;

	wid->flags = 0;
	wid->redraw = 1;
	wid->cx = -1;
	wid->cy = -1;
	wid->x = -1;
	wid->y = -1;
	wid->w = -1;
	wid->h = -1;
	wid->style = &agStyleDefault;
	SLIST_INIT(&wid->bindings);
	SLIST_INIT(&wid->menus);
	AG_MutexInitRecursive(&wid->bindings_lock);

	wid->nsurfaces = 0;
	wid->surfaces = NULL;
	wid->surfaceFlags = NULL;
#ifdef HAVE_OPENGL
	wid->textures = NULL;
	wid->texcoords = NULL;
#endif

	/*
	 * Arrange for immediate children to inherit the style settings
	 * of the parent on attachment.
	 */
	AG_SetEvent(wid, "child-attached", ChildAttached, NULL);
}

/* Traverse the widget tree using a pathname. */
static void *
WidgetFindPath(const AG_Object *parent, const char *name)
{
	char node_name[AG_OBJECT_PATH_MAX];
	void *rv;
	char *s;
	AG_Object *chld;

	strlcpy(node_name, name, sizeof(node_name));
	if ((s = strchr(node_name, '/')) != NULL) {
		*s = '\0';
	}
	if (AG_ObjectIsClass(parent, "AG_Display:*")) {
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
	} else if (AG_ObjectIsClass(parent, "AG_Widget:AG_Notebook:*")) {
		AG_Notebook *book = (AG_Notebook *)parent;
		AG_NotebookTab *tab;

		TAILQ_FOREACH(tab, &book->tabs, tabs) {
			if (strcmp(OBJECT(tab)->name, node_name) != 0) {
				continue;
			}
			if ((s = strchr(name, '/')) != NULL) {
				rv = WidgetFindPath(OBJECT(tab), &s[1]);
				if (rv != NULL) {
					return (rv);
				} else {
					return (NULL);
				}
			}
			return (tab);
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

/* Find a widget by name. */
void *
AG_WidgetFind(AG_Display *view, const char *name)
{
	void *rv;

#ifdef DEBUG
	if (name[0] != '/')
		fatal("not an absolute path: `%s'", name);
#endif
	AG_LockLinkage();
	rv = WidgetFindPath(OBJECT(view), &name[1]);
	AG_UnlockLinkage();
	if (rv == NULL) {
		AG_SetError(_("The widget `%s' does not exist."), name);
	}
	return (rv);
}

void
AG_WidgetSetFocusable(void *p, int flag)
{
	AG_Widget *wid = p;

	if (flag) {
		wid->flags |= AG_WIDGET_FOCUSABLE;
	} else {
		wid->flags &= ~(AG_WIDGET_FOCUSABLE);
	}
}

void
AG_WidgetSetStyle(void *p, AG_Style *style)
{
	AGWIDGET(p)->style = style;
}

int
AG_WidgetCopyBinding(void *w1, const char *n1, void *w2, const char *n2)
{
	AG_WidgetBinding *b1, *b2;

	if ((b1 = AG_WidgetGetBinding(w1, n1)) == NULL) {
		return (-1);
	}
	if ((b2 = AG_WidgetGetBinding(w2, n2)) == NULL) {
		AG_WidgetUnlockBinding(b1);
		return (-1);
	}
	b1->type = b2->type;
	b1->vtype = b2->vtype;
	b1->mutex = b2->mutex;
	b1->p1 = b2->p1;

	switch (b1->type) {
	case AG_WIDGET_PROP:
		strlcpy(b1->data.prop, b2->data.prop, sizeof(b1->data.prop));
		break;
	case AG_WIDGET_STRING:
		b1->data.size = b2->data.size;
		break;
	case AG_WIDGET_FLAG:
	case AG_WIDGET_FLAG8:
	case AG_WIDGET_FLAG16:
	case AG_WIDGET_FLAG32:
		b1->data.bitmask = b2->data.bitmask;
		break;
	default:
		break;
	}
	AG_WidgetUnlockBinding(b2);
	AG_WidgetUnlockBinding(b1);
	return (0);
}

/* Bind a mutex-protected variable to a widget. */
AG_WidgetBinding *
AG_WidgetBindMp(void *widp, const char *name, AG_Mutex *mutex,
    enum ag_widget_binding_type type, ...)
{
	AG_Widget *wid = widp;
	AG_WidgetBinding *b;
	va_list ap;
	
	AG_MutexLock(&wid->bindings_lock);
	va_start(ap, type);
	switch (type) {
	case AG_WIDGET_PROP:
		b = AG_WidgetBind(wid, name, type,
		    va_arg(ap, void *),
		    va_arg(ap, char *));
		break;
	case AG_WIDGET_STRING:
		b = AG_WidgetBind(wid, name, type,
		    va_arg(ap, char *),
		    va_arg(ap, size_t));
		break;
	default:
		b = AG_WidgetBind(wid, name, type,
		    va_arg(ap, void *));
		break;
	}
	va_end(ap);
	b->mutex = mutex;
	AG_MutexUnlock(&wid->bindings_lock);
	return (b);
}

/*
 * Obtain the virtual type of the given binding. This translates WIDGET_PROP
 * bindings to the equivalent widget binding type.
 */
static int
GetVirtualBindingType(AG_WidgetBinding *binding)
{
	AG_Prop *prop;

	switch (binding->type) {
	case AG_WIDGET_PROP:
		if ((prop = AG_GetProp(binding->p1, binding->data.prop, -1,
		    NULL)) == NULL) {
			fatal("%s", AG_GetError());
		}
		switch (prop->type) {
		case AG_PROP_BOOL:
			return (AG_WIDGET_BOOL);
		case AG_PROP_INT:
			return (AG_WIDGET_INT);
		case AG_PROP_UINT8:
			return (AG_WIDGET_UINT8);
		case AG_PROP_SINT8:
			return (AG_WIDGET_SINT8);
		case AG_PROP_UINT16:
			return (AG_WIDGET_UINT16);
		case AG_PROP_SINT16:
			return (AG_WIDGET_SINT16);
		case AG_PROP_UINT32:
			return (AG_WIDGET_UINT32);
		case AG_PROP_SINT32:
			return (AG_WIDGET_SINT32);
#ifdef HAVE_64BIT
		case AG_PROP_UINT64:
			return (AG_WIDGET_UINT64);
		case AG_PROP_SINT64:
			return (AG_WIDGET_SINT64);
#endif
		case AG_PROP_FLOAT:
			return (AG_WIDGET_FLOAT);
		case AG_PROP_DOUBLE:
			return (AG_WIDGET_DOUBLE);
		case AG_PROP_STRING:
			return (AG_WIDGET_STRING);
		case AG_PROP_POINTER:
			return (AG_WIDGET_POINTER);
		default:
			return (-1);
		}
	default:
		return (binding->type);
	}
	return (-1);
}

/* Bind a variable to a widget. */
AG_WidgetBinding *
AG_WidgetBind(void *widp, const char *name, enum ag_widget_binding_type type,
    ...)
{
	AG_Widget *wid = widp;
	AG_WidgetBinding *binding;
	void *p1;
	size_t size = 0;		/* -Wuninitialized */
	Uint32 bitmask = 0;		/* -Wuninitialized */
	char *prop = NULL;		/* -Wuninitialized */
	va_list ap;

	va_start(ap, type);
	switch (type) {
	case AG_WIDGET_PROP:
		p1 = va_arg(ap, void *);
		prop = va_arg(ap, char *);
		break;
	case AG_WIDGET_STRING:
		p1 = va_arg(ap, char *);
		size = va_arg(ap, size_t);
		break;
	case AG_WIDGET_FLAG:
		p1 = va_arg(ap, void *);
		bitmask = va_arg(ap, Uint);
		break;
	case AG_WIDGET_FLAG8:
		p1 = va_arg(ap, void *);
		bitmask = (Uint8)va_arg(ap, Uint);
		break;
	case AG_WIDGET_FLAG16:
		p1 = va_arg(ap, void *);
		bitmask = (Uint16)va_arg(ap, Uint);
		break;
	case AG_WIDGET_FLAG32:
		p1 = va_arg(ap, void *);
		bitmask = (Uint32)va_arg(ap, Uint);
		break;
	default:
		p1 = va_arg(ap, void *);
		break;
	}
	va_end(ap);

	AG_MutexLock(&wid->bindings_lock);
	SLIST_FOREACH(binding, &wid->bindings, bindings) {
		if (strcmp(binding->name, name) != 0) {
			continue;
		}
		binding->type = type;
		binding->p1 = p1;
		switch (type) {
		case AG_WIDGET_PROP:
			strlcpy(binding->data.prop, prop,
			    sizeof(binding->data.prop));
			break;
		case AG_WIDGET_STRING:
			binding->data.size = size;
			break;
		case AG_WIDGET_FLAG:
		case AG_WIDGET_FLAG8:
		case AG_WIDGET_FLAG16:
		case AG_WIDGET_FLAG32:
			binding->data.bitmask = bitmask;
			break;
		default:
			break;
		}
		binding->vtype = GetVirtualBindingType(binding);
		AG_PostEvent(NULL, wid, "widget-bound", "%p", binding);
		AG_MutexUnlock(&wid->bindings_lock);
		return (binding);
	}

	binding = Malloc(sizeof(AG_WidgetBinding));
	strlcpy(binding->name, name, sizeof(binding->name));
	binding->type = type;
	binding->p1 = p1;
	binding->mutex = NULL;
	switch (type) {
	case AG_WIDGET_PROP:
		strlcpy(binding->data.prop, prop, sizeof(binding->data.prop));
		break;
	case AG_WIDGET_STRING:
		binding->data.size = size;
		break;
	case AG_WIDGET_FLAG:
	case AG_WIDGET_FLAG8:
	case AG_WIDGET_FLAG16:
	case AG_WIDGET_FLAG32:
		binding->data.bitmask = bitmask;
		break;
	default:
		break;
	}
	binding->vtype = GetVirtualBindingType(binding);

	SLIST_INSERT_HEAD(&wid->bindings, binding, bindings);
	AG_PostEvent(NULL, wid, "widget-bound", "%p", binding);
	AG_MutexUnlock(&wid->bindings_lock);
	return (binding);
}

/*
 * Lookup a binding and copy its data to pointers passed as arguments.
 * The caller should invoke AG_WidgetUnlockBinding() when done reading/writing
 * the data.
 */
AG_WidgetBinding *
AG_WidgetGetBinding(void *widp, const char *name, ...)
{
	AG_Widget *wid = widp;
	AG_WidgetBinding *binding;
	AG_Prop *prop;
	void **res;
	va_list ap;

	va_start(ap, name);
	res = va_arg(ap, void **);
	va_end(ap);

	AG_MutexLock(&wid->bindings_lock);
	SLIST_FOREACH(binding, &wid->bindings, bindings) {
		if (strcmp(binding->name, name) != 0)
			continue;

		if (binding->mutex != NULL) {
			AG_MutexLock(binding->mutex);
		}
		switch (binding->type) {
		case AG_WIDGET_BOOL:
		case AG_WIDGET_INT:
			*(int **)res = (int *)binding->p1;
			break;
		case AG_WIDGET_UINT:
			*(Uint **)res = (Uint *)binding->p1;
			break;
		case AG_WIDGET_UINT8:
			*(Uint8 **)res = (Uint8 *)binding->p1;
			break;
		case AG_WIDGET_SINT8:
			*(Sint8 **)res = (Sint8 *)binding->p1;
			break;
		case AG_WIDGET_UINT16:
			*(Uint16 **)res = (Uint16 *)binding->p1;
			break;
		case AG_WIDGET_SINT16:
			*(Sint16 **)res = (Sint16 *)binding->p1;
			break;
		case AG_WIDGET_UINT32:
			*(Uint32 **)res = (Uint32 *)binding->p1;
			break;
		case AG_WIDGET_SINT32:
			*(Sint32 **)res = (Sint32 *)binding->p1;
			break;
#ifdef HAVE_64BIT
		case AG_WIDGET_UINT64:
			*(Uint64 **)res = (Uint64 *)binding->p1;
			break;
		case AG_WIDGET_SINT64:
			*(Sint64 **)res = (Sint64 *)binding->p1;
			break;
#endif
		case AG_WIDGET_FLOAT:
			*(float **)res = (float *)binding->p1;
			break;
		case AG_WIDGET_DOUBLE:
			*(double **)res = (double *)binding->p1;
			break;
		case AG_WIDGET_STRING:
			*(char ***)res = (char **)binding->p1;
			break;
		case AG_WIDGET_POINTER:
			*(void ***)res = (void **)binding->p1;
			break;
		case AG_WIDGET_FLAG:
			*(Uint **)res = (Uint *)binding->p1;
			break;
		case AG_WIDGET_FLAG8:
			*(Uint8 **)res = (Uint8 *)binding->p1;
			break;
		case AG_WIDGET_FLAG16:
			*(Uint16 **)res = (Uint16 *)binding->p1;
			break;
		case AG_WIDGET_FLAG32:
			*(Uint32 **)res = (Uint32 *)binding->p1;
			break;
		case AG_WIDGET_PROP:			/* Convert */
			if ((prop = AG_GetProp(binding->p1, binding->data.prop,
			    -1, NULL)) == NULL) {
				fatal("%s", AG_GetError());
			}
			switch (prop->type) {
			case AG_PROP_BOOL:
			case AG_PROP_INT:
				*(int **)res = (int *)&prop->data.i;
				break;
			case AG_PROP_UINT8:
				*(Uint8 **)res = (Uint8 *)&prop->data.u8;
				break;
			case AG_PROP_SINT8:
				*(Sint8 **)res = (Sint8 *)&prop->data.s8;
				break;
			case AG_PROP_UINT16:
				*(Uint16 **)res = (Uint16 *)&prop->data.u16;
				break;
			case AG_PROP_SINT16:
				*(Sint16 **)res = (Sint16 *)&prop->data.s16;
				break;
			case AG_PROP_UINT32:
				*(Uint32 **)res = (Uint32 *)&prop->data.u32;
				break;
			case AG_PROP_SINT32:
				*(Sint32 **)res = (Sint32 *)&prop->data.s32;
				break;
#ifdef HAVE_64BIT
			case AG_PROP_UINT64:
				*(Uint64 **)res = (Uint64 *)&prop->data.u64;
				break;
			case AG_PROP_SINT64:
				*(Sint64 **)res = (Sint64 *)&prop->data.s64;
				break;
#endif
			case AG_PROP_FLOAT:
				*(float **)res = (float *)&prop->data.f;
				break;
			case AG_PROP_DOUBLE:
				*(double **)res = (double *)&prop->data.d;
				break;
			case AG_PROP_STRING:
				*(char ***)res = (char **)&prop->data.s;
				break;
			case AG_PROP_POINTER:
				*(void ***)res = (void **)&prop->data.p;
				break;
			default:
				AG_SetError("Failed to translate property");
				binding = NULL;
				goto out;
			}
			break;
		default:
			AG_SetError("Bad widget binding type");
			binding = NULL;
			goto out;
		}
out:
		AG_MutexUnlock(&wid->bindings_lock);
		return (binding);			/* Return locked */
	}
	AG_MutexUnlock(&wid->bindings_lock);

	AG_SetError("No such widget binding: %s", name);
	return (NULL);
}

/*
 * Generate a prop-modified event after manipulating the property values
 * manually. The property must be locked.
 */
void
AG_WidgetBindingChanged(AG_WidgetBinding *bind)
{
	if (bind->type == AG_WIDGET_PROP) {
		AG_Object *pobj = bind->p1;
		AG_Prop *prop;

		if ((prop = AG_GetProp(pobj, bind->data.prop, -1, NULL))
		    != NULL) {
			AG_PostEvent(NULL, pobj, "prop-modified", "%p", prop);
		}
	}
}

/*
 * Register a surface with the given widget. In OpenGL mode, generate
 * a texture as well.
 *
 * The surface is not duplicated, but will be freed automatically with the
 * widget unless the NODUP flag is set.
 */
int
AG_WidgetMapSurface(void *p, SDL_Surface *su)
{
	AG_Widget *wid = p;
	int i, idx = -1;

	for (i = 0; i < wid->nsurfaces; i++) {
		if (wid->surfaces[i] == NULL) {
			idx = i;
			break;
		}
	}
	if (i == wid->nsurfaces) {
		wid->surfaces = Realloc(wid->surfaces,
		    (wid->nsurfaces+1)*sizeof(SDL_Surface *));
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
	if (agView->opengl) {
		GLuint texname;

		AG_LockGL();
		texname = (su == NULL) ? 0 :
		    AG_SurfaceTexture(su, &wid->texcoords[idx*4]);
		wid->textures[idx] = texname;
		AG_UnlockGL();
	}
#endif
	return (idx);
}

/*
 * Variant of WidgetMapSurface() that sets the NODUP flag such that
 * the surface is not freed automatically with the widget.
 */
int
AG_WidgetMapSurfaceNODUP(void *p, SDL_Surface *su)
{
	AG_Widget *wid = p;
	int name;

	name = AG_WidgetMapSurface(wid, su);
	wid->surfaceFlags[name] |= AG_WIDGET_SURFACE_NODUP;
	return (name);
}

/*
 * Replace the contents of a registered surface, and regenerate
 * the matching texture in OpenGL mode. Unless NODUP is set, the
 * current source surface is freed.
 */
void
AG_WidgetReplaceSurface(void *p, int name, SDL_Surface *su)
{
	AG_Widget *wid = p;

	if (wid->surfaces[name] != NULL) {
		if (!WSURFACE_NODUP(wid,name))
			SDL_FreeSurface(wid->surfaces[name]);
	}
	wid->surfaces[name] = su;
	wid->surfaceFlags[name] &= ~(AG_WIDGET_SURFACE_NODUP);

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		AG_LockGL();
		if (wid->textures[name] != 0) {
			glDeleteTextures(1, (GLuint *)&wid->textures[name]);
		}
		wid->textures[name] = (su == NULL) ? 0 :
		    AG_SurfaceTexture(su, &wid->texcoords[name*4]);
		AG_UnlockGL();
	}
#endif
}

/* Variant of WidgetReplaceSurface() that sets the NODUP flag. */
void
AG_WidgetReplaceSurfaceNODUP(void *p, int name, SDL_Surface *su)
{
	AG_Widget *wid = p;

	AG_WidgetReplaceSurface(wid, name, su);
	wid->surfaceFlags[name] |= AG_WIDGET_SURFACE_NODUP;
}

void
AG_WidgetUpdateSurface(void *p, int name)
{
#ifdef HAVE_OPENGL
	AG_Widget *wid = p;

	if (agView->opengl)
		AG_UpdateTexture(wid->surfaces[name], wid->textures[name]);
#endif
}

/*
 * Note: Widget objects must be destroyed in event handler context
 * (for texture operations in OpenGL mode).
 */
static void
Destroy(void *obj)
{
	AG_Widget *wid = obj;
	AG_PopupMenu *pm, *pm2;
	AG_WidgetBinding *bind, *nbind;
	Uint i;
	
	for (pm = SLIST_FIRST(&wid->menus);
	     pm != SLIST_END(&wid->menus);
	     pm = pm2) {
		pm2 = SLIST_NEXT(pm, menus);
		AG_PopupDestroy(NULL, pm);
	}
	for (i = 0; i < wid->nsurfaces; i++) {
		if (wid->surfaces[i] != NULL && !WSURFACE_NODUP(wid,i))
			SDL_FreeSurface(wid->surfaces[i]);
#ifdef HAVE_OPENGL
		/* XXX TODO Queue this operation? */
		if (agView->opengl) {
			AG_LockGL();
			if (wid->textures[i] != 0) {
				glDeleteTextures(1,
				    (GLuint *)&wid->textures[i]);
			}
			AG_UnlockGL();
		}
#endif
	}
	Free(wid->surfaces);
	Free(wid->surfaceFlags);

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		Free(wid->textures);
		Free(wid->texcoords);
	}
#endif

	for (bind = SLIST_FIRST(&wid->bindings);
	     bind != SLIST_END(&wid->bindings);
	     bind = nbind) {
		nbind = SLIST_NEXT(bind, bindings);
		Free(bind);
	}
	AG_MutexDestroy(&wid->bindings_lock);
}

/*
 * Perform a software blit from a source surface to the display, at
 * coordinates relative to the widget, using clipping.
 *
 * Only safe to call from widget rendering context.
 */
void
AG_WidgetBlit(void *p, SDL_Surface *srcsu, int x, int y)
{
	AG_Widget *wid = p;
	SDL_Rect rd;

	rd.x = wid->cx + x;
	rd.y = wid->cy + y;
	rd.w = srcsu->w <= wid->w ? srcsu->w : wid->w;		/* Clip */
	rd.h = srcsu->h <= wid->h ? srcsu->h : wid->h;		/* Clip */

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		GLuint texture;
		GLfloat texcoord[4];
		int alpha = (srcsu->flags & (SDL_SRCALPHA|SDL_SRCCOLORKEY));
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
			glVertex2i(rd.x, rd.y);
			glTexCoord2f(texcoord[2], texcoord[1]);
			glVertex2i(rd.x+srcsu->w, rd.y);
			glTexCoord2f(texcoord[0], texcoord[3]);
			glVertex2i(rd.x, rd.y+srcsu->h);
			glTexCoord2f(texcoord[2], texcoord[3]);
			glVertex2i(rd.x+srcsu->w, rd.y+srcsu->h);
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
		SDL_BlitSurface(srcsu, NULL, agView->v, &rd);
	}
}

/*
 * Perform a hardware or software blit from a registered surface to the display
 * at coordinates relative to the widget, using clipping.
 * 
 * Only safe to call from widget rendering context.
 */
void
AG_WidgetBlitFrom(void *p, void *srcp, int name, SDL_Rect *rs, int x, int y)
{
	AG_Widget *wid = p;
	AG_Widget *srcwid = srcp;
	SDL_Surface *su = srcwid->surfaces[name];
	SDL_Rect rd;

	if (name == -1 || su == NULL)
		return;

	rd.x = wid->cx + x;
	rd.y = wid->cy + y;
	rd.w = su->w <= wid->w ? su->w : wid->w;		/* Clip */
	rd.h = su->h <= wid->h ? su->h : wid->h;		/* Clip */

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		GLfloat tmptexcoord[4];
		GLuint texture = srcwid->textures[name];
		GLfloat *texcoord;
		int alpha = su->flags & (SDL_SRCALPHA|SDL_SRCCOLORKEY);
		GLboolean blend_sv;
		GLint blend_sfactor, blend_dfactor;
		GLfloat texenvmode;

		if (rs == NULL) {
			texcoord = &srcwid->texcoords[name*4];
		} else {
			texcoord = &tmptexcoord[0];
			texcoord[0] = (GLfloat)rs->x/AG_PowOf2i(rs->x);
			texcoord[1] = (GLfloat)rs->y/AG_PowOf2i(rs->y);
			texcoord[2] = (GLfloat)rs->w/AG_PowOf2i(rs->w);
			texcoord[3] = (GLfloat)rs->h/AG_PowOf2i(rs->h);
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
	
		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_TRIANGLE_STRIP);
		{
			glTexCoord2f(texcoord[0], texcoord[1]);
			glVertex2i(rd.x, rd.y);
			glTexCoord2f(texcoord[2], texcoord[1]);
			glVertex2i(rd.x+su->w, rd.y);
			glTexCoord2f(texcoord[0], texcoord[3]);
			glVertex2i(rd.x, rd.y+su->h);
			glTexCoord2f(texcoord[2], texcoord[3]);
			glVertex2i(rd.x+su->w, rd.y+su->h);
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
		SDL_BlitSurface(su, rs, agView->v, &rd);
	}
}

#ifdef HAVE_OPENGL
/*
 * OpenGL-only version of AG_WidgetBlitSurface() without explicit
 * source or destination rectangle parameter.
 */
void
AG_WidgetBlitSurfaceGL(void *pWidget, int name, float w, float h)
{
	AG_Widget *wid = pWidget;
	GLuint glname;
	GLfloat *texcoord;
	GLboolean blend_sv;
	GLint blend_sfactor, blend_dfactor;
	GLfloat texenvmode;
	SDL_Surface *su = wid->surfaces[name];
	int alpha = su->flags & (SDL_SRCALPHA|SDL_SRCCOLORKEY);

	glname = wid->textures[name];
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
	
	glBindTexture(GL_TEXTURE_2D, glname);
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
#endif /* HAVE_OPENGL */

/* Clear the AG_WIDGET_FOCUSED bit from a widget and its descendents. */
void
AG_WidgetUnfocus(void *p)
{
	AG_Widget *wid = p, *cwid;

	if (wid->flags & AG_WIDGET_FOCUSED) {
		wid->flags &= ~(AG_WIDGET_FOCUSED);
		AG_PostEvent(NULL, wid, "widget-lostfocus", NULL);
	}

	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
		AG_WidgetUnfocus(cwid);
}

/* Find the parent window of a widget. */
AG_Window *
AG_WidgetParentWindow(void *p)
{
	AG_Widget *wid = p;
	AG_Widget *pwid = wid;

	if (AG_ObjectIsClass(wid, "AG_Widget:AG_Window:*"))
		return ((AG_Window *)wid);

	while ((pwid = OBJECT(pwid)->parent) != NULL) {
		if (AG_ObjectIsClass(pwid, "AG_Widget:AG_Window:*"))
			break;
	}
	return ((AG_Window *)pwid);
}

/* Move the focus over a widget (and its parents). */
void
AG_WidgetFocus(void *p)
{
	AG_Widget *wid = p, *pwid = wid, *fwid;
	AG_Window *pwin;

	if ((wid->flags & AG_WIDGET_FOCUSABLE) == 0)
		return;

	/* See if this widget is already focused. */
	if ((fwid = AG_WidgetFindFocused(wid)) != NULL &&
	    (fwid == wid))
		return;

	/* Remove focus from other widgets inside this window. */
	pwin = AG_WidgetParentWindow(wid);
	if (pwin != NULL) {
		if (pwin->flags & AG_WINDOW_DENYFOCUS) {
			return;
		}
		AG_WidgetUnfocus(pwin);
	}

	/* Set the focus flag on the widget and its parents. */
	do {
		if (AG_ObjectIsClass(pwid, "AG_Widget:AG_Window:*"))
			break;
#if 0
		if ((pwid->flags & AG_WIDGET_FOCUSABLE) == 0) {
			dprintf("parent (%s) is not focusable\n",
			    OBJECT(pwid)->name);
			break;
		}
#endif
		pwid->flags |= AG_WIDGET_FOCUSED;
		AG_PostEvent(OBJECT(pwid)->parent, pwid, "widget-gainfocus",
		    NULL);
	} while ((pwid = OBJECT(pwid)->parent) != NULL);
}

/* Evaluate whether a given widget is at least partially visible. */
/* TODO optimize on a per window basis */
int
AG_WidgetIsOcculted(AG_Widget *wid)
{
	AG_Window *owin;
	AG_Window *wwin;

	if ((wwin = AG_ObjectFindParent(wid, NULL, "AG_Widget:AG_Window"))
	    == NULL ||
	    (owin = TAILQ_NEXT(wwin, windows)) == NULL) {
		return (0);
	}
	for (; owin != TAILQ_END(&agView->windows);
	     owin = TAILQ_NEXT(owin, windows)) {
		if (owin->visible &&
		    wid->cx > WIDGET(owin)->x &&
		    wid->cy > WIDGET(owin)->y &&
		    wid->cx+wid->w < WIDGET(owin)->x+WIDGET(owin)->w &&
		    wid->cy+wid->h < WIDGET(owin)->y+WIDGET(owin)->h) {
			return (1);
		}
	}
	return (0);
}

void
AG_SetCursor(int cursor)
{
	if (agCursorToSet == NULL)
		agCursorToSet = agCursors[cursor];
}

void
AG_UnsetCursor(void)
{
	agCursorToSet = agDefaultCursor;
}

/*
 * Push a clipping rectangle onto the clipping rectangle stack.
 * This is only safe to call from widget draw context.
 */
void
AG_WidgetPushClipRect(void *p, int px, int py, int w, int h)
{
	AG_Widget *wid = p;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		GLdouble eq0[4] = { 1, 0, 0, -(wid->cx+px-1) };
		GLdouble eq1[4] = { 0, 1, 0, -(wid->cy+py-1) };
		GLdouble eq2[4] = { -1, 0, 0, (wid->cx+px+w) };
		GLdouble eq3[4] = { 0, -1, 0, (wid->cy+py+h) };

		glPushAttrib(GL_TRANSFORM_BIT);
		glClipPlane(GL_CLIP_PLANE0, eq0);
		glClipPlane(GL_CLIP_PLANE1, eq1);
		glClipPlane(GL_CLIP_PLANE2, eq2);
		glClipPlane(GL_CLIP_PLANE3, eq3);
		glEnable(GL_CLIP_PLANE0);
		glEnable(GL_CLIP_PLANE1);
		glEnable(GL_CLIP_PLANE2);
		glEnable(GL_CLIP_PLANE3);
	} else
#endif
	{
		SDL_Rect r;
		int x = px + wid->cx;
		int y = py + wid->cy;

		if (x < 0) {
			x = 0;
		} else if (x+w > agView->w) {
			w = agView->w - x;
			if (w < 0) {
				x = 0;
				w = agView->w;
			}
		}
		if (y < 0) {
			y = 0;
		} else if (y+h > agView->h) {
			h = agView->h - y;
			if (h < 0) {
				y = 0;
				h = agView->h;
			}
		}

		r.x = x;
		r.y = y;
		r.w = w;
		r.h = h;
		SDL_GetClipRect(agView->v, &wid->rClipSave);
		SDL_SetClipRect(agView->v, &r);
	}
}

/*
 * Pop a clipping rectangle off the clipping rectangle stack.
 * This is only safe to call from widget draw context.
 */
void
AG_WidgetPopClipRect(void *p)
{
	AG_Widget *wid = p;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		glPopAttrib();
	} else
#endif
	{
		SDL_SetClipRect(agView->v, &wid->rClipSave);
	}
}

/*
 * Render a widget and its descendents, recursively.
 * The view must be locked. In OpenGL mode, GL must be locked as well.
 */
void
AG_WidgetDraw(void *p)
{
	AG_Widget *wid = p;
	AG_Widget *chld;

	if (wid->flags & (AG_WIDGET_HIDE|AG_WIDGET_UNDERSIZE))
		return;

	if (((wid->flags & AG_WIDGET_STATIC)==0 || wid->redraw) &&
	    !AG_WidgetIsOcculted(wid) &&
	    WIDGET_OPS(wid)->draw != NULL &&
	    WIDGET(wid)->w > 0 &&
	    WIDGET(wid)->h > 0) {
		int clip = 0;

		if (wid->flags & AG_WIDGET_CLIPPING) {
			AG_WidgetPushClipRect(wid, 0, 0, wid->w, wid->h);
			clip = 1;
		}
		WIDGET_OPS(wid)->draw(wid);

		if (clip) {
			AG_WidgetPopClipRect(wid);
		}
		if (wid->flags & AG_WIDGET_STATIC)
			wid->redraw = 0;
	}

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		AG_WidgetDraw(chld);
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
	if (WIDGET_OPS(w)->size_request != NULL)
		WIDGET_OPS(w)->size_request(w, r);
}

int
AG_WidgetSizeAlloc(void *p, AG_SizeAlloc *a)
{
	AG_Widget *w = p;

	if (a->w < 0 || a->h < 0) {
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
			return (-1);
		} else {
			w->flags &= ~(AG_WIDGET_UNDERSIZE);
		}
	}
	return (0);
}

/*
 * Blend with the pixel at widget-relative x,y coordinates, with clipping
 * to display area.
 *
 * Must be invoked from widget rendering context. In SDL mode, the
 * display surface must be locked.
 */
void
AG_WidgetBlendPixelRGBA(void *p, int wx, int wy, Uint8 c[4], AG_BlendFn fn)
{
	AG_Widget *wid = p;
	int vx = wid->cx+wx;
	int vy = wid->cy+wy;

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
		glVertex2s(vx, vy);
		glEnd();

		if (!svBlendBit) {
			glDisable(GL_BLEND);
		}
		glBlendFunc(GL_SRC_ALPHA, svBlendSrc);
		glBlendFunc(GL_DST_ALPHA, svBlendDst);
	} else
#endif /* HAVE_OPENGL */
	{
		AG_BLEND_RGBA2_CLIPPED(agView->v, vx, vy,
		    c[0], c[1], c[2], c[3], fn);
	}
}

/*
 * Post a mousemotion event to widgets that either hold focus or have the
 * AG_WIDGET_UNFOCUSED_MOTION flag set.
 */
void
AG_WidgetMouseMotion(AG_Window *win, AG_Widget *wid, int x, int y,
    int xrel, int yrel, int state)
{
	AG_Widget *cwid;

	if ((AG_WINDOW_FOCUSED(win) && AG_WidgetFocused(wid)) ||
	    (wid->flags & AG_WIDGET_UNFOCUSED_MOTION)) {
		AG_PostEvent(NULL, wid, "window-mousemotion",
		    "%i, %i, %i, %i, %i", x-wid->cx, y-wid->cy,
		    xrel, yrel, state);
		if (wid->flags & AG_WIDGET_PRIO_MOTION)
			return;
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
		AG_WidgetMouseMotion(win, cwid, x, y, xrel, yrel, state);
}

/*
 * Post a mousebuttonup event to widgets that either hold focus or have the
 * AG_WIDGET_UNFOCUSED_BUTTONUP flag set.
 */
void
AG_WidgetMouseButtonUp(AG_Window *win, AG_Widget *wid, int button,
    int x, int y)
{
	AG_Widget *cwid;

	if ((AG_WINDOW_FOCUSED(win) && AG_WidgetFocused(wid)) ||
	    (wid->flags & AG_WIDGET_UNFOCUSED_BUTTONUP)) {
		AG_PostEvent(NULL, wid,  "window-mousebuttonup", "%i, %i, %i",
		    button, x-wid->cx, y-wid->cy);
	}
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
		AG_WidgetMouseButtonUp(win, cwid, button, x, y);
}

/* Process a mousebuttondown event. */
int
AG_WidgetMouseButtonDown(AG_Window *win, AG_Widget *wid, int button,
    int x, int y)
{
	AG_Widget *cwid;
	AG_Event *ev;

	/* Search for a better match. */
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		if (AG_WidgetMouseButtonDown(win, cwid, button, x, y))
			return (1);
	}
	if (!AG_WidgetArea(wid, x, y)) {
		return (0);
	}
	TAILQ_FOREACH(ev, &OBJECT(wid)->events, events) {
		if (strcmp(ev->name, "window-mousebuttondown") == 0)
			break;
	}
	if (ev != NULL) {
		AG_PostEvent(NULL, wid, "window-mousebuttondown", "%i, %i, %i",
		    button, x-wid->cx, y-wid->cy);
		return (1);
	}
	return (0);
}

/* Search for a focused widget inside a window. */
AG_Widget *
AG_WidgetFindFocused(void *p)
{
	AG_Widget *wid = p;
	AG_Widget *cwid, *fwid;

	if (!AG_ObjectIsClass(wid, "AG_Widget:AG_Window:*") &&
	    (wid->flags & AG_WIDGET_FOCUSED) == 0)
		return (NULL);

	/* Search for a better match. */
	OBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		if ((fwid = AG_WidgetFindFocused(cwid)) != NULL)
			return (fwid);
	}
	return (wid);
}

/*
 * Cache the absolute view coordinates of a widget and its descendents.
 * The view must be locked.
 */
void
AG_WidgetUpdateCoords(void *parent, int x, int y)
{
	AG_Widget *pwid = parent, *cwid;

	pwid->cx = x;
	pwid->cy = y;
	pwid->cx2 = x + pwid->w;
	pwid->cy2 = y + pwid->h;
	AG_PostEvent(NULL, pwid, "widget-moved", NULL);

	OBJECT_FOREACH_CHILD(cwid, pwid, ag_widget)
		AG_WidgetUpdateCoords(cwid, pwid->cx+cwid->x, pwid->cy+cwid->y);
}

/* Parse a generic size specification. */
enum ag_widget_sizespec
AG_WidgetParseSizeSpec(const char *spec_text, int *w)
{
	char spec[256];
	char *p;

	strlcpy(spec, spec_text, sizeof(spec));
	for (p = &spec[0]; (p[0] != '\0' && p[1] != '\0'); p++) {
		break;
	}
	switch (*p) {
	case '%':
		*p = '\0';
		*w = (int)strtol(spec, NULL, 10);
		return (AG_WIDGET_PERCENT);
	case '>':
		if (spec[0] != '<') {
			return (AG_WIDGET_BAD_SPEC);
		}
		*p = '\0';
		AG_TextSize(&spec[1], w, NULL);
		return (AG_WIDGET_STRINGLEN);
	case 'x':
		{
			const char *ep;

			if ((ep = strchr(spec, 'p')) == NULL) {
				return (AG_WIDGET_BAD_SPEC);
			}
			ep = '\0';
			*w = (int)strtol(ep, NULL, 10);
			return (AG_WIDGET_PIXELS);
		}
	default:
		AG_TextSize(spec, w, NULL);
		break;
	}
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

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_WidgetShownRecursive(chld);
	}
	AG_PostEvent(NULL, wid, "widget-shown", NULL);
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

	OBJECT_FOREACH_CHILD(chld, wid, ag_widget) {
		AG_WidgetHiddenRecursive(chld);
	}
	AG_PostEvent(NULL, wid, "widget-hidden", NULL);
}

#ifdef HAVE_OPENGL
/*
 * Release the OpenGL resources associated with a widget.
 * GL lock must be held.
 */
void
AG_WidgetFreeResourcesGL(AG_Widget *wid)
{
	glDeleteTextures(wid->nsurfaces, (GLuint *)wid->textures);
	memset(wid->textures, 0, wid->nsurfaces*sizeof(Uint));
}

/*
 * Regenerate the OpenGL textures associated with a widget.
 * GL lock must be held.
 */
void
AG_WidgetRegenResourcesGL(AG_Widget *wid)
{
	Uint i;

	for (i = 0; i < wid->nsurfaces; i++)
		wid->textures[i] = AG_SurfaceTexture(wid->surfaces[i],
		    &wid->texcoords[i*4]);
}
#endif /* HAVE_OPENGL */

static void *
FindAtPoint(AG_Widget *parent, const char *type, int x, int y)
{
	AG_Widget *chld;
	void *p;

	OBJECT_FOREACH_CHILD(chld, parent, ag_widget) {
		if ((p = FindAtPoint(chld, type, x, y)) != NULL)
			return (p);
	}
	if (AG_ObjectIsClass(parent, type) &&
	    AG_WidgetArea(parent, x, y)) {
		return (parent);
	}
	return (NULL);
}

void *
AG_WidgetFindPoint(const char *type, int x, int y)
{
	AG_Window *win;
	void *p;
	
	TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq, windows) {
		if ((p = FindAtPoint(WIDGET(win), type, x, y)) != NULL)
			return (p);
	}
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
	if (AG_ObjectIsClass(parent, type) &&
	    AG_WidgetRectIntersect(parent, x,y,w,h)) {
		return (parent);
	}
	return (NULL);
}

void *
AG_WidgetFindRect(const char *type, int x, int y, int w, int h)
{
	AG_Window *win;
	void *p;
	
	TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq, windows) {
		if ((p = FindRectOverlap(WIDGET(win), type, x,y,w,h)) != NULL)
			return (p);
	}
	return (NULL);
}

void
AG_WidgetSetString(void *wid, const char *name, const char *ns)
{
	AG_WidgetBinding *binding;
	char *s;

	if ((binding = AG_WidgetGetBinding(wid, name, &s)) == NULL) {
		fatal("%s", AG_GetError());
	}
	strlcpy(s, ns, binding->data.size);
	AG_WidgetUnlockBinding(binding);
}

size_t
AG_WidgetCopyString(void *wid, const char *name, char *dst, size_t dst_size)
{
	AG_WidgetBinding *b;
	char *s;
	size_t rv;

	if ((b = AG_WidgetGetBinding(wid, name, &s)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = strlcpy(dst, s, dst_size);
	AG_WidgetUnlockBinding(b);
	return (rv);
}

const AG_WidgetOps agWidgetOps = {
	{
		"AG_Widget",
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
