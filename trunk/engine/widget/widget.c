/*	$Csoft: widget.c,v 1.47 2003/03/11 00:13:33 vedge Exp $	*/

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

#include <config/floating_point.h>

#include <engine/engine.h>
#include <engine/view.h>

#include <stdarg.h>

#include "widget.h"
#include "window.h"

#ifdef DEBUG
#define DEBUG_BINDINGS		0x01
#define DEBUG_BINDING_LOOKUPS	0x02

int	widget_debug = 0;
#define engine_debug widget_debug
#endif

void
widget_init(struct widget *wid, char *name, const void *wops, int rw, int rh)
{
	static pthread_mutex_t curwidget_lock = PTHREAD_MUTEX_INITIALIZER;
	static unsigned int curwidget = 0;
	char *widname;

	pthread_mutex_lock(&curwidget_lock);
	curwidget++;
	pthread_mutex_unlock(&curwidget_lock);

	Asprintf(&widname, "%s%u", name, curwidget);
	object_init(&wid->obj, "widget", widname, name,
	    OBJECT_ART|OBJECT_ART_CACHE|OBJECT_ART_CAN_FAIL, wops);
	free(widname);

	wid->type = Strdup(name);
	wid->flags = 0;
	wid->win = NULL;
	wid->x = 0;
	wid->y = 0;
	wid->rw = rw;
	wid->rh = rh;
	wid->w = 0;
	wid->h = 0;
	wid->ncolors = 0;
	SLIST_INIT(&wid->colors);
	SLIST_INIT(&wid->bindings);
	pthread_mutex_init(&wid->bindings_lock, NULL);
}

/*
 * Associate a widget with a parent region.
 * The parent region's window must be locked.
 */
void
widget_set_parent(void *child, void *parent)
{
	struct widget *wid = child;
	struct region *reg = parent;

	OBJECT_ASSERT(wid, "widget");
	OBJECT_ASSERT(reg, "window-region");

	wid->reg = reg;
	wid->win = reg->win;
}

/*
 * Bind an arbitrary value to a widget.
 * The data pointed to must remain consistent as long as the widget exists.
 */
struct widget_binding *
widget_bind(void *widp, const char *name, enum widget_binding_type type, ...)
{
	struct widget *wid = widp;
	struct widget_binding *binding;
	pthread_mutex_t *mu = NULL;
	void *p1, *p2 = NULL;
	va_list ap;

	va_start(ap, type);
	switch (type) {
	case WIDGET_PROP:
		p1 = va_arg(ap, void *);
		p2 = va_arg(ap, char *);
		break;
	default:
		mu = va_arg(ap, pthread_mutex_t *);
		p1 = va_arg(ap, void *);
		break;
	}
	va_end(ap);

	OBJECT_ASSERT(wid, "widget");

	pthread_mutex_lock(&wid->bindings_lock);
	SLIST_FOREACH(binding, &wid->bindings, bindings) {	/* Modify */
		if (strcmp(binding->name, name) == 0) {
			debug_n(DEBUG_BINDINGS,
			    "%s: modified binding `%s': %d(%p,%p,%p)",
			    OBJECT(wid)->name, name, binding->type,
			    binding->p1, binding->p2, binding->mutex);

			binding->type = type;
			binding->p1 = p1;
			binding->p2 = p2;
			binding->mutex = mu;

			debug_n(DEBUG_BINDINGS, " -> %d(%p,%p,%p)\n",
			    binding->type, binding->p1, binding->p2,
			    binding->mutex);

			pthread_mutex_unlock(&wid->bindings_lock);
			return (binding);
		}
	}

	binding = emalloc(sizeof(struct widget_binding));	/* Create */
	binding->type = type;
	binding->name = Strdup(name);
	binding->p1 = p1;
	binding->p2 = p2;
	binding->mutex = mu;
	SLIST_INSERT_HEAD(&wid->bindings, binding, bindings);

	debug_n(DEBUG_BINDINGS,
	    "%s: bound `%s' to %p (type=%d p2=%p mutex=%p)\n",
	    OBJECT(wid)->name, name, p1, type, p2, mu);
	pthread_mutex_unlock(&wid->bindings_lock);

	return (binding);
}

int
widget_get_int(void *wid, const char *name)
{
	int *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}

Uint8
widget_get_uint8(void *wid, const char *name)
{
	Uint8 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}
Sint8
widget_get_sint8(void *wid, const char *name)
{
	Sint8 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}

Uint16
widget_get_uint16(void *wid, const char *name)
{
	Uint16 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}
Sint16
widget_get_sint16(void *wid, const char *name)
{
	Sint16 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}

Uint32
widget_get_uint32(void *wid, const char *name)
{
	Uint32 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}
Sint32
widget_get_sint32(void *wid, const char *name)
{
	Sint32 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}

#ifdef FLOATING_POINT
float
widget_get_float(void *wid, const char *name)
{
	float *f;

	if (widget_binding_get(wid, name, &f) == NULL)
		fatal("%s", error_get());
	return (*f);
}

double
widget_get_double(void *wid, const char *name)
{
	double *d;

	if (widget_binding_get(wid, name, &d) == NULL)
		fatal("%s", error_get());
	return (*d);
}
#endif /* FLOATING_POINT */

char *
widget_get_string(void *wid, const char *name)
{
	char *s;

	if (widget_binding_get(wid, name, &s) == NULL)
		fatal("%s", error_get());
	return (s);
}

void *
widget_get_pointer(void *wid, const char *name)
{
	void *p;

	if (widget_binding_get(wid, name, &p) == NULL)
		fatal("%s", error_get());
	return (p);
}

void
widget_set_int(void *wid, const char *name, int ni)
{
	struct widget_binding *binding;
	int *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_uint8(void *wid, const char *name, Uint8 ni)
{
	struct widget_binding *binding;
	Uint8 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_sint8(void *wid, const char *name, Sint8 ni)
{
	struct widget_binding *binding;
	Sint8 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_uint16(void *wid, const char *name, Uint16 ni)
{
	struct widget_binding *binding;
	Uint16 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_sint16(void *wid, const char *name, Sint16 ni)
{
	struct widget_binding *binding;
	Sint16 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_uint32(void *wid, const char *name, Uint32 ni)
{
	struct widget_binding *binding;
	Uint32 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_sint32(void *wid, const char *name, Sint32 ni)
{
	struct widget_binding *binding;
	Sint32 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

#ifdef FLOATING_POINT
void
widget_set_float(void *wid, const char *name, float nf)
{
	struct widget_binding *binding;
	float *f;

	if ((binding = widget_binding_get_locked(wid, name, &f)) == NULL)
		fatal("%s", error_get());
	*f = nf;
	widget_binding_unlock(binding);
}

void
widget_set_double(void *wid, const char *name, double nd)
{
	struct widget_binding *binding;
	double *d;

	if ((binding = widget_binding_get_locked(wid, name, &d)) == NULL)
		fatal("%s", error_get());
	*d = nd;
	widget_binding_unlock(binding);
}
#endif /* FLOATING_POINT */

void
widget_set_string(void *wid, const char *name, char *ns)
{
	struct widget_binding *binding;
	char **s;

	if ((binding = widget_binding_get_locked(wid, name, &s)) == NULL)
		fatal("%s", error_get());

	/* XXX realloc */
	free(*s);
	*s = Strdup(ns);
	widget_binding_unlock(binding);
}

void
widget_set_pointer(void *wid, const char *name, void *np)
{
	struct widget_binding *binding;
	void **p;

	if ((binding = widget_binding_get_locked(wid, name, &p)) == NULL)
		fatal("%s", error_get());

	*p = np;
	widget_binding_unlock(binding);
}

/*
 * Look for a binding and return the pointer value in p.
 * XXX use a hash table
 */
struct widget_binding *
_widget_binding_get(void *widp, const char *name, void *res, int return_locked)
{
	struct widget *wid = widp;
	struct widget_binding *binding;
	struct prop *prop;

	OBJECT_ASSERT(wid, "widget");

	debug(DEBUG_BINDING_LOOKUPS, "look up `%s' in %s, return in %p%s.\n",
	    name, OBJECT(wid)->name, res,
	    return_locked ? ", return locked" : "");

	pthread_mutex_lock(&wid->bindings_lock);
	SLIST_FOREACH(binding, &wid->bindings, bindings) {
		if (strcmp(binding->name, name) == 0) {
			if (binding->mutex != NULL)
				pthread_mutex_lock(binding->mutex);
			switch (binding->type) {
			case WIDGET_BOOL:
			case WIDGET_INT:
				*(int **)res = (int *)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\t%s %s = %d\n",
				    (binding->type == WIDGET_BOOL) ?
				    "bool" : "int", name, *(int *)binding->p1);
				break;
			case WIDGET_UINT8:
				*(Uint8 **)res = (Uint8 *)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tUint8 %s = %d\n",
				    name, *(Uint8 *)binding->p1);
				break;
			case WIDGET_SINT8:
				*(Sint8 **)res = (Sint8 *)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tSint8 %s = %d\n",
				    name, *(Sint8 *)binding->p1);
				break;
			case WIDGET_UINT16:
				*(Uint16 **)res = (Uint16 *)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tUint16 %s = %d\n",
				    name, *(Uint16 *)binding->p1);
				break;
			case WIDGET_SINT16:
				*(Sint16 **)res = (Sint16 *)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tSint16 %s = %d\n",
				    name, *(Sint16 *)binding->p1);
				break;
			case WIDGET_UINT32:
				*(Uint32 **)res = (Uint32 *)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tUint32 %s = %d\n",
				    name, *(Uint32 *)binding->p1);
				break;
			case WIDGET_SINT32:
				*(Sint32 **)res = (Sint32 *)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tSint32 %s = %d\n",
				    name, *(Sint32 *)binding->p1);
				break;
#ifdef FLOATING_POINT
			case WIDGET_FLOAT:
				*(float **)res = (float *)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tfloat %s = %f\n",
				    name, *(float *)binding->p1);
				break;
			case WIDGET_DOUBLE:
				*(double **)res = (double *)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tdouble %s = %f\n",
				    name, *(double *)binding->p1);
				break;
#endif
			case WIDGET_STRING:
				*(char ***)res = (char **)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tchar *%s = \"%s\"\n",
				    name, *(char **)binding->p1);
				break;
			case WIDGET_POINTER:
				*(void ***)res = (void **)binding->p1;
				debug(DEBUG_BINDING_LOOKUPS,
				    "\tvoid *%s = %p\n",
				    name, *(void **)binding->p1);
				break;
			case WIDGET_PROP:			/* Convert */
				prop = prop_get(binding->p1,
				    (char *)binding->p2, PROP_ANY, NULL);
				if (prop == NULL) {
					fatal("%s", error_get());
				}

				switch (prop->type) {
				case PROP_BOOL:
				case PROP_INT:
					*(int **)res = (int *)&prop->data.i;
					break;
				case PROP_UINT8:
					*(Uint8 **)res =
					    (Uint8 *)&prop->data.u8;
					break;
				case PROP_SINT8:
					*(Sint8 **)res =
					    (Sint8 *)&prop->data.s8;
					break;
				case PROP_UINT16:
					*(Uint16 **)res =
					    (Uint16 *)&prop->data.u16;
					break;
				case PROP_SINT16:
					*(Sint16 **)res =
					    (Sint16 *)&prop->data.s16;
					break;
				case PROP_UINT32:
					*(Uint32 **)res =
					    (Uint32 *)&prop->data.u32;
					break;
				case PROP_SINT32:
					*(Sint32 **)res =
					    (Sint32 *)&prop->data.s32;
					break;
#ifdef FLOATING_POINT
				case PROP_FLOAT:
					*(float **)res =
					    (float *)&prop->data.f;
					break;
				case PROP_DOUBLE:
					*(double **)res =
					    (double *)&prop->data.d;
					break;
#endif
				case PROP_STRING:
					*(char ***)res =
					    (char **)&prop->data.s;
					break;
				case PROP_POINTER:
					*(void ***)res =
					    (void **)&prop->data.p;
					break;
				default:
					fatal("cannot translate prop");
				}
				break;
			default:
				fatal("unknown binding type");
			}
			if (binding->mutex != NULL && !return_locked)
				pthread_mutex_unlock(binding->mutex);
			pthread_mutex_unlock(&wid->bindings_lock);
			return (binding);
		}
	}
	pthread_mutex_unlock(&wid->bindings_lock);

	error_set("%s: no such binding: `%s'", OBJECT(wid)->name, name);
	return (NULL);
}

void
widget_binding_lock(struct widget_binding *bind)
{
	if (bind->mutex != NULL) {
		pthread_mutex_lock(bind->mutex);
	}
}

void
widget_binding_unlock(struct widget_binding *bind)
{
	if (bind->mutex != NULL) {
		pthread_mutex_unlock(bind->mutex);
	}
}

/*
 * Generate a prop-modified event after manipulating the property values
 * manually. The property must be locked.
 */
void
widget_binding_modified(struct widget_binding *bind)
{
	if (bind->type == WIDGET_PROP) {
		struct object *pobj = bind->p1;
		char *name = (char *)bind->p2;
		struct prop *prop;

		prop = prop_get(pobj, name, PROP_ANY, NULL);
		event_post(pobj, "prop-modified", "%p", prop);
	}
}

/*
 * Add a color scheme entry with a default value.
 * The widget's parent window must be locked, if the widget is attached.
 */
void
widget_map_color(void *p, int ind, char *name, Uint8 r, Uint8 g, Uint8 b)
{
	struct widget *wid = p;
	struct widget_color *col;
	
	if (strcmp(OBJECT(wid)->name, "widget") == 0 ||
	    strcmp(OBJECT(wid)->name, "window") == 0) {
		fatal("%s is not a widget/window", OBJECT(wid)->name);
	}
	
	if (ind > WIDGET_MAX_COLORS)
		fatal("%d colors > %d", ind, WIDGET_MAX_COLORS);
	if (ind > wid->ncolors)
		wid->ncolors++;

	wid->color[ind] = SDL_MapRGB(view->v->format, r, g, b);

	col = emalloc(sizeof(struct widget_color));
	col->name = Strdup(name);
	col->ind = ind;
	SLIST_INSERT_HEAD(&wid->colors, col, colors);
}

void
widget_destroy(void *p)
{
	struct widget *wid = p;
	struct widget_color *color, *next_color;
	struct widget_binding *bind, *next_bind;

	OBJECT_ASSERT(wid, "widget");

	free(wid->type);

	/* Free the color scheme */
	for (color = SLIST_FIRST(&wid->colors);
	     color != SLIST_END(&wid->colors);
	     color = next_color) {
		next_color = SLIST_NEXT(color, colors);
		free(color->name);
	}
	SLIST_INIT(&wid->colors);
	
	/* Free the binding list */
	for (bind = SLIST_FIRST(&wid->bindings);
	     bind != SLIST_END(&wid->bindings);
	     bind = next_bind) {
		next_bind = SLIST_NEXT(bind, bindings);
		free(bind->name);
		free(bind);
	}
	SLIST_INIT(&wid->bindings);
}

/*
 * Alter the widget's effective position.
 * The widget's parent window must be locked, if the widget is attached.
 * Changes are not visible until a window_resize() operation is performed.
 */
void
widget_set_position(void *p, Sint16 x, Sint16 y)
{
	struct widget *wid = p;

	wid->x = x;
	wid->y = y;
}

/*
 * Obtain the widget's effective position.
 * The widget's parent window must be locked, if the widget is attached.
 */
void
widget_get_position(void *p, Sint16 *x, Sint16 *y)
{
	struct widget *wid = p;

	*x = wid->x;
	*y = wid->y;
}

/*
 * Alter the widget's effective geometry.
 * The widget's parent window must be locked, if the widget is attached.
 * Changes are not visible until a window_resize() operation is performed.
 */
void
widget_set_geometry(void *p, Uint16 w, Uint16 h)
{
	struct widget *wid = p;

	wid->w = w;
	wid->h = h;
}

/*
 * Obtain the widget's effective geometry.
 * The widget's parent window must be locked, if the widget is attached.
 */
void
widget_get_geometry(void *p, Uint16 *w, Uint16 *h)
{
	struct widget *wid = p;

	*w = wid->h;
	*h = wid->h;
}

/*
 * Perform a blit from a source surface to the display, at coordinates
 * relative to the widget.
 */
void
widget_blit(void *p, SDL_Surface *srcsu, int xoffs, int yoffs)
{
	struct widget *wid = p;
	SDL_Rect rd;

	rd.x = WIDGET_ABSX(wid) + xoffs; /* XXX move rect to widget structure */
	rd.y = WIDGET_ABSY(wid) + yoffs;
	rd.w = srcsu->w;
	rd.h = srcsu->h;

#ifdef HAVE_OPENGL
	if (view->opengl) {
		GLuint texture;
		GLfloat texcoord[4];

		/* XXX very inefficient */

		texture = view_surface_texture(srcsu, texcoord);
		glBindTexture(GL_TEXTURE_2D, texture);
		if (srcsu->flags & (SDL_SRCALPHA|SDL_SRCCOLORKEY)) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
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
		glDeleteTextures(1, &texture);
		if (srcsu->flags & (SDL_SRCALPHA|SDL_SRCCOLORKEY)) {
			glDisable(GL_BLEND);
		}
	} else
#endif
	{
		SDL_BlitSurface(srcsu, NULL, WIDGET_SURFACE(wid), &rd);
	}
}

