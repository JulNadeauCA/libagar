/*	$Csoft: widget.c,v 1.86 2004/04/10 02:34:05 vedge Exp $	*/

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

#include <config/floating_point.h>

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include <stdarg.h>
#include <string.h>

#ifdef DEBUG
#define DEBUG_BINDINGS		0x01

int	widget_debug = 0;
#define engine_debug widget_debug
#endif

const struct version widget_ver = {
	"agar widget",
	0, 0
};

void
widget_init(void *p, const char *type, const void *wops, int flags)
{
	struct widget *wid = p;

	object_init(wid, "widget", type, wops);
	OBJECT(wid)->save_pfx = "/widgets";

	strlcpy(wid->type, type, sizeof(wid->type));
	wid->flags = flags;
	wid->cx = -1;
	wid->cy = -1;
	wid->x = -1;
	wid->y = -1;
	wid->w = -1;
	wid->h = -1;
	wid->ncolors = 0;
	SLIST_INIT(&wid->bindings);
	pthread_mutex_init(&wid->bindings_lock, &recursive_mutexattr);
}

/* Bind a protected variable to a widget. */
struct widget_binding *
widget_bind_protected(void *widp, const char *name, pthread_mutex_t *mutex,
    enum widget_binding_type type, ...)
{
	struct widget *wid = widp;
	struct widget_binding *b;
	va_list ap;
	
	pthread_mutex_lock(&wid->bindings_lock);
	va_start(ap, type);
	switch (type) {
	case WIDGET_PROP:
		b = widget_bind(wid, name, type,
		    va_arg(ap, void *),
		    va_arg(ap, char *));
		break;
	case WIDGET_STRING:
		b = widget_bind(wid, name, type,
		    va_arg(ap, char *),
		    va_arg(ap, size_t));
		break;
	default:
		b = widget_bind(wid, name, type,
		    va_arg(ap, void *));
		break;
	}
	va_end(ap);
	b->mutex = mutex;
	pthread_mutex_unlock(&wid->bindings_lock);
	return (b);
}

/* Translate property types to widget types. */
static int
widget_vtype(struct widget_binding *binding)
{
	struct prop *prop;

	switch (binding->type) {
	case WIDGET_PROP:
		if ((prop = prop_get(binding->p1, (char *)binding->p2, PROP_ANY,
		    NULL)) == NULL) {
			fatal("%s", error_get());
		}
		switch (prop->type) {
		case PROP_BOOL:
			return (WIDGET_BOOL);
		case PROP_INT:
			return (WIDGET_INT);
		case PROP_UINT8:
			return (WIDGET_UINT8);
		case PROP_SINT8:
			return (WIDGET_SINT8);
		case PROP_UINT16:
			return (WIDGET_UINT16);
		case PROP_SINT16:
			return (WIDGET_SINT16);
		case PROP_UINT32:
			return (WIDGET_UINT32);
		case PROP_SINT32:
			return (WIDGET_SINT32);
		case PROP_FLOAT:
			return (WIDGET_FLOAT);
		case PROP_DOUBLE:
			return (WIDGET_DOUBLE);
		case PROP_STRING:
			return (WIDGET_STRING);
		case PROP_POINTER:
			return (WIDGET_POINTER);
		default:
			return (-1);
		}
	default:
		return (binding->type);
	}
	return (-1);
}

/* Bind a variable to a widget. */
struct widget_binding *
widget_bind(void *widp, const char *name, enum widget_binding_type type, ...)
{
	struct widget *wid = widp;
	struct widget_binding *binding;
	void *p1, *p2 = NULL;
	size_t size = 0;
	va_list ap;

	va_start(ap, type);
	switch (type) {
	case WIDGET_PROP:
		p1 = va_arg(ap, void *);
		p2 = va_arg(ap, char *);
		break;
	case WIDGET_STRING:
		p1 = va_arg(ap, char *);
		size = va_arg(ap, size_t);
		break;
	default:
		p1 = va_arg(ap, void *);
		break;
	}
	va_end(ap);

	pthread_mutex_lock(&wid->bindings_lock);
	SLIST_FOREACH(binding, &wid->bindings, bindings) {
		if (strcmp(binding->name, name) == 0) {
			debug_n(DEBUG_BINDINGS,
			    "%s: modified binding `%s': %d(%p,%p)",
			    OBJECT(wid)->name, name, binding->type,
			    binding->p1, binding->p2);

			binding->type = type;
			binding->p1 = p1;
			binding->p2 = p2;
			binding->size = size;
			binding->vtype = widget_vtype(binding);

			debug_n(DEBUG_BINDINGS, " -> %d(%p,%p)\n",
			    binding->type, binding->p1, binding->p2);

			event_post(NULL, wid, "widget-bound", "%p", binding);
			pthread_mutex_unlock(&wid->bindings_lock);
			return (binding);
		}
	}

	binding = Malloc(sizeof(struct widget_binding), M_WIDGET);
	strlcpy(binding->name, name, sizeof(binding->name));
	binding->type = type;
	binding->p1 = p1;
	binding->p2 = p2;
	binding->size = size;
	binding->mutex = NULL;
	binding->vtype = widget_vtype(binding);
	SLIST_INSERT_HEAD(&wid->bindings, binding, bindings);

	debug_n(DEBUG_BINDINGS, "%s: bound `%s' to %p (type=%d)\n",
	    OBJECT(wid)->name, name, p1, type);

	event_post(NULL, wid, "widget-bound", "%p", binding);
	pthread_mutex_unlock(&wid->bindings_lock);
	return (binding);
}

/*
 * Lookup a binding and copy its data to pointers passed as arguments.
 * The caller should invoke widget_binding_unlock() when done reading/writing
 * the data.
 */
struct widget_binding *
widget_get_binding(void *widp, const char *name, ...)
{
	struct widget *wid = widp;
	struct widget_binding *binding;
	struct prop *prop;
	void **res;
	va_list ap;

	va_start(ap, name);
	res = va_arg(ap, void **);
	va_end(ap);

	pthread_mutex_lock(&wid->bindings_lock);
	SLIST_FOREACH(binding, &wid->bindings, bindings) {
		if (strcmp(binding->name, name) != 0)
			continue;

		if (binding->mutex != NULL) {
			pthread_mutex_lock(binding->mutex);
		}
		switch (binding->type) {
		case WIDGET_BOOL:
		case WIDGET_INT:
			*(int **)res = (int *)binding->p1;
			break;
		case WIDGET_UINT:
			*(unsigned int **)res = (unsigned int *)binding->p1;
			break;
		case WIDGET_UINT8:
			*(Uint8 **)res = (Uint8 *)binding->p1;
			break;
		case WIDGET_SINT8:
			*(Sint8 **)res = (Sint8 *)binding->p1;
			break;
		case WIDGET_UINT16:
			*(Uint16 **)res = (Uint16 *)binding->p1;
			break;
		case WIDGET_SINT16:
			*(Sint16 **)res = (Sint16 *)binding->p1;
			break;
		case WIDGET_UINT32:
			*(Uint32 **)res = (Uint32 *)binding->p1;
			break;
		case WIDGET_SINT32:
			*(Sint32 **)res = (Sint32 *)binding->p1;
			break;
		case WIDGET_FLOAT:
			*(float **)res = (float *)binding->p1;
			break;
		case WIDGET_DOUBLE:
			*(double **)res = (double *)binding->p1;
			break;
		case WIDGET_STRING:
			*(char ***)res = (char **)binding->p1;
			break;
		case WIDGET_POINTER:
			*(void ***)res = (void **)binding->p1;
			break;
		case WIDGET_PROP:			/* Convert */
			if ((prop = prop_get(binding->p1, (char *)binding->p2,
			    PROP_ANY, NULL)) == NULL) {
				fatal("%s", error_get());
			}
			switch (prop->type) {
			case PROP_BOOL:
			case PROP_INT:
				*(int **)res = (int *)&prop->data.i;
				break;
			case PROP_UINT8:
				*(Uint8 **)res = (Uint8 *)&prop->data.u8;
				break;
			case PROP_SINT8:
				*(Sint8 **)res = (Sint8 *)&prop->data.s8;
				break;
			case PROP_UINT16:
				*(Uint16 **)res = (Uint16 *)&prop->data.u16;
				break;
			case PROP_SINT16:
				*(Sint16 **)res = (Sint16 *)&prop->data.s16;
				break;
			case PROP_UINT32:
				*(Uint32 **)res = (Uint32 *)&prop->data.u32;
				break;
			case PROP_SINT32:
				*(Sint32 **)res = (Sint32 *)&prop->data.s32;
				break;
			case PROP_FLOAT:
				*(float **)res = (float *)&prop->data.f;
				break;
			case PROP_DOUBLE:
				*(double **)res = (double *)&prop->data.d;
				break;
			case PROP_STRING:
				*(char ***)res = (char **)&prop->data.s;
				break;
			case PROP_POINTER:
				*(void ***)res = (void **)&prop->data.p;
				break;
			default:
				error_set("Failed to translate property.");
				binding = NULL;
				goto out;
			}
			break;
		default:
			error_set("Unknown type of widget binding.");
			binding = NULL;
			goto out;
		}
out:
		pthread_mutex_unlock(&wid->bindings_lock);
		return (binding);			/* Return locked */
	}
	pthread_mutex_unlock(&wid->bindings_lock);

	error_set("No such widget binding: `%s'.", name);
	return (NULL);
}

int
widget_get_int(void *wid, const char *name)
{
	struct widget_binding *b;
	int *i, rv;

	if ((b = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *i;
	widget_binding_unlock(b);
	return (rv);
}

unsigned int
widget_get_uint(void *wid, const char *name)
{
	struct widget_binding *b;
	unsigned int *i, rv;

	if ((b = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *i;
	widget_binding_unlock(b);
	return (rv);
}

Uint8
widget_get_uint8(void *wid, const char *name)
{
	struct widget_binding *b;
	Uint8 *i, rv;

	if ((b = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *i;
	widget_binding_unlock(b);
	return (rv);
}

Sint8
widget_get_sint8(void *wid, const char *name)
{
	struct widget_binding *b;
	Sint8 *i, rv;

	if ((b = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *i;
	widget_binding_unlock(b);
	return (rv);
}

Uint16
widget_get_uint16(void *wid, const char *name)
{
	struct widget_binding *b;
	Uint16 *i, rv;

	if ((b = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *i;
	widget_binding_unlock(b);
	return (rv);
}

Sint16
widget_get_sint16(void *wid, const char *name)
{
	struct widget_binding *b;
	Sint16 *i, rv;

	if ((b = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *i;
	widget_binding_unlock(b);
	return (rv);
}

Uint32
widget_get_uint32(void *wid, const char *name)
{
	struct widget_binding *b;
	Uint32 *i, rv;

	if ((b = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *i;
	widget_binding_unlock(b);
	return (rv);
}

Sint32
widget_get_sint32(void *wid, const char *name)
{
	struct widget_binding *b;
	Sint32 *i, rv;

	if ((b = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *i;
	widget_binding_unlock(b);
	return (rv);
}

float
widget_get_float(void *wid, const char *name)
{
	struct widget_binding *b;
	float *f, rv;

	if ((b = widget_get_binding(wid, name, &f)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *f;
	widget_binding_unlock(b);
	return (rv);
}

double
widget_get_double(void *wid, const char *name)
{
	struct widget_binding *b;
	double *d, rv;

	if ((b = widget_get_binding(wid, name, &d)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *d;
	widget_binding_unlock(b);
	return (rv);
}

char *
widget_get_string(void *wid, const char *name)
{
	struct widget_binding *b;
	char *s, *sd;

	if ((b = widget_get_binding(wid, name, &s)) == NULL) {
		fatal("%s", error_get());
	}
	sd = Strdup(s);
	widget_binding_unlock(b);
	return (sd);
}

size_t
widget_copy_string(void *wid, const char *name, char *dst, size_t dst_size)
{
	struct widget_binding *b;
	char *s;
	size_t rv;

	if ((b = widget_get_binding(wid, name, &s)) == NULL) {
		fatal("%s", error_get());
	}
	rv = strlcpy(dst, s, dst_size);
	widget_binding_unlock(b);
	return (rv);
}

void *
widget_get_pointer(void *wid, const char *name)
{
	struct widget_binding *b;
	void **p, *rv;

	if ((b = widget_get_binding(wid, name, &p)) == NULL) {
		fatal("%s", error_get());
	}
	rv = *p;
	widget_binding_unlock(b);
	return (p);
}

void
widget_set_int(void *wid, const char *name, int ni)
{
	struct widget_binding *binding;
	int *i;

	if ((binding = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_uint(void *wid, const char *name, unsigned int ni)
{
	struct widget_binding *binding;
	unsigned int *i;

	if ((binding = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_uint8(void *wid, const char *name, Uint8 ni)
{
	struct widget_binding *binding;
	Uint8 *i;

	if ((binding = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_sint8(void *wid, const char *name, Sint8 ni)
{
	struct widget_binding *binding;
	Sint8 *i;

	if ((binding = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_uint16(void *wid, const char *name, Uint16 ni)
{
	struct widget_binding *binding;
	Uint16 *i;

	if ((binding = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_sint16(void *wid, const char *name, Sint16 ni)
{
	struct widget_binding *binding;
	Sint16 *i;

	if ((binding = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_uint32(void *wid, const char *name, Uint32 ni)
{
	struct widget_binding *binding;
	Uint32 *i;

	if ((binding = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_sint32(void *wid, const char *name, Sint32 ni)
{
	struct widget_binding *binding;
	Sint32 *i;

	if ((binding = widget_get_binding(wid, name, &i)) == NULL) {
		fatal("%s", error_get());
	}
	*i = ni;
	widget_binding_unlock(binding);
}

void
widget_set_float(void *wid, const char *name, float nf)
{
	struct widget_binding *binding;
	float *f;

	if ((binding = widget_get_binding(wid, name, &f)) == NULL) {
		fatal("%s", error_get());
	}
	*f = nf;
	widget_binding_unlock(binding);
}

void
widget_set_double(void *wid, const char *name, double nd)
{
	struct widget_binding *binding;
	double *d;

	if ((binding = widget_get_binding(wid, name, &d)) == NULL) {
		fatal("%s", error_get());
	}
	*d = nd;
	widget_binding_unlock(binding);
}

void
widget_set_string(void *wid, const char *name, const char *ns)
{
	struct widget_binding *binding;
	char *s;

	if ((binding = widget_get_binding(wid, name, &s)) == NULL) {
		fatal("%s", error_get());
	}
	strlcpy(s, ns, binding->size);
	widget_binding_unlock(binding);
}

void
widget_set_pointer(void *wid, const char *name, void *np)
{
	struct widget_binding *binding;
	void **p;

	if ((binding = widget_get_binding(wid, name, &p)) == NULL) {
		fatal("%s", error_get());
	}
	*p = np;
	widget_binding_unlock(binding);
}

void
widget_binding_lock(struct widget_binding *bind)
{
	if (bind->mutex != NULL)
		pthread_mutex_lock(bind->mutex);
}

void
widget_binding_unlock(struct widget_binding *bind)
{
	if (bind->mutex != NULL)
		pthread_mutex_unlock(bind->mutex);
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
		event_post(NULL, pobj, "prop-modified", "%p", prop);
	}
}

/* Add a named entry to a widget's color stack. */
void
widget_map_color(void *p, int ind, const char *name, Uint8 r, Uint8 g, Uint8 b,
    Uint8 a)
{
	struct widget *wid = p;

	if (ind >= WIDGET_COLORS_MAX)
		fatal("color stack oflow");

	wid->colors[ind] = SDL_MapRGBA(vfmt, r, g, b, a);
	strlcpy(wid->color_names[ind], name, sizeof(wid->color_names[ind]));

	if (ind >= wid->ncolors)
		wid->ncolors = ind+1;
}

void
widget_destroy(void *p)
{
	struct widget *wid = p;
	struct widget_binding *bind, *nbind;

	for (bind = SLIST_FIRST(&wid->bindings);
	     bind != SLIST_END(&wid->bindings);
	     bind = nbind) {
		nbind = SLIST_NEXT(bind, bindings);
		Free(bind, M_WIDGET);
	}
	pthread_mutex_destroy(&wid->bindings_lock);
}

/*
 * Perform a fast blit from a source surface to the display, at coordinates
 * relative to the widget; clipping is done.
 */
void
widget_blit(void *p, SDL_Surface *srcsu, int x, int y)
{
	struct widget *wid = p;
	SDL_Rect rd;

	rd.x = wid->cx + x;
	rd.y = wid->cy + y;
	rd.w = srcsu->w <= wid->w ? srcsu->w : wid->w;		/* Clip */
	rd.h = srcsu->h <= wid->h ? srcsu->h : wid->h;		/* Clip */

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
		SDL_BlitSurface(srcsu, NULL, view->v, &rd);
	}
}

/* Evaluate to true if a widget is holding focus (inside its parent). */
int
widget_holds_focus(void *p)
{
	return (WIDGET(p)->flags & WIDGET_FOCUSED);
}


/* Clear the WIDGET_FOCUSED bit from a widget and its descendents. */
static void
widget_clear_focus(void *p)
{
	struct widget *wid = p, *cwid;

	if (wid->flags & WIDGET_FOCUSED) {
		wid->flags &= ~(WIDGET_FOCUSED);
		event_post(NULL, wid, "widget-lostfocus", NULL);
	}

	OBJECT_FOREACH_CHILD(cwid, wid, widget)
		widget_clear_focus(cwid);
}

/* Find the parent window of a widget. */
struct window *
widget_parent_window(void *p)
{
	struct widget *wid = p;
	struct widget *pwid = wid;

	if (OBJECT_TYPE(wid, "window"))
		return ((struct window *)wid);

	while ((pwid = OBJECT(pwid)->parent) != NULL) {
		if (OBJECT_TYPE(pwid, "window"))
			break;
	}
	return ((struct window *)pwid);
}

/* Move the focus over a widget (and its parents). */
void
widget_focus(void *p)
{
	struct widget *wid = p, *pwid = wid;
	struct window *pwin;

	/* Remove focus from other widgets inside this window. */
	pwin = widget_parent_window(wid);
	if (pwin != NULL) {
		widget_clear_focus(pwin);
	} else {
		dprintf("%s: no parent window\n", OBJECT(wid)->name);
	}

	/* Set the focus flag on the widget and its parents. */
	do {
		if (OBJECT_TYPE(pwid, "window"))
			break;
#if 0
		if ((pwid->flags & WIDGET_FOCUSABLE) == 0) {
			dprintf("parent (%s) is not focusable\n",
			    OBJECT(pwid)->name);
			break;
		}
#endif
		pwid->flags |= WIDGET_FOCUSED;
		event_post(OBJECT(pwid)->parent, pwid, "widget-gainfocus",
		    NULL);
	} while ((pwid = OBJECT(pwid)->parent) != NULL);
}

/*
 * Render a widget and its descendents, recursively.
 * The view must be locked.
 */
void
widget_draw(void *p)
{
	struct widget *wid = p;
	struct widget *cwid;

	if (WIDGET_OPS(wid)->draw != NULL) {
		SDL_Rect clip_save;

		if (wid->flags & WIDGET_CLIPPING) {
			SDL_Rect clip;

			clip.x = wid->cx;
			clip.y = wid->cy;
			clip.w = wid->w;
			clip.h = wid->h;
			SDL_GetClipRect(view->v, &clip_save);
			SDL_SetClipRect(view->v, &clip);
		}

		WIDGET_OPS(wid)->draw(wid);
		
		if (wid->flags & WIDGET_CLIPPING) {
			SDL_SetClipRect(view->v, &clip_save);
		}
	}

	OBJECT_FOREACH_CHILD(cwid, wid, widget)
		widget_draw(cwid);
}

/* Set the geometry of a widget and invoke its scale operation. */
void
widget_scale(void *p, int w, int h)
{
	struct widget *wid = p;

	wid->w = w;
	wid->h = h;
	WIDGET_OPS(wid)->scale(wid, wid->w, wid->h);
}

/*
 * Write to the pixel at widget-relative x,y coordinates.
 * The display surface must be locked; clipping is done.
 */
void
widget_put_pixel(void *p, int wx, int wy, Uint32 color)
{
	struct widget *wid = p;
	int x = wid->cx+wx;
	int y = wid->cy+wy;
	Uint8 *d;

	if (!VIEW_INSIDE_CLIP_RECT(view->v, x, y))
		return;

	d = (Uint8 *)view->v->pixels + y*view->v->pitch + x*vfmt->BytesPerPixel;
	switch (vfmt->BytesPerPixel) {
		_VIEW_PUTPIXEL_32(d, color);
		_VIEW_PUTPIXEL_24(d, color);
		_VIEW_PUTPIXEL_16(d, color);
		_VIEW_PUTPIXEL_8(d, color);
	}
}

/* Evaluate to true if absolute view coords x,y are inside the widget area. */
int
widget_area(void *p, int x, int y)
{
	struct widget *wid = p;

	return (x > wid->cx &&
	        y > wid->cy &&
	        x < wid->cx+wid->w &&
		y < wid->cy+wid->h);
}

/* Evaluate to true if widget coords x,y are inside the widget area. */
int
widget_relative_area(void *p, int x, int y)
{
	struct widget *wid = p;

	return (x >= 0 &&
	        y >= 0 &&
	        x < wid->w &&
		y < wid->h);
}

/*
 * Post a mousemotion event to widgets that either hold focus or have the
 * WIDGET_UNFOCUSED_MOTION flag set.
 */
void
widget_mousemotion(struct window *win, struct widget *wid, int x, int y,
    int xrel, int yrel, int state)
{
	struct widget *cwid;

	if ((WINDOW_FOCUSED(win) && widget_holds_focus(wid)) ||
	    (wid->flags & WIDGET_UNFOCUSED_MOTION)) {
		event_post(NULL, wid, "window-mousemotion",
		    "%i, %i, %i, %i, %i", x-wid->cx, y-wid->cy,
		    xrel, yrel, state);
	}
	OBJECT_FOREACH_CHILD(cwid, wid, widget)
		widget_mousemotion(win, cwid, x, y, xrel, yrel, state);
}

/*
 * Post a mousebuttonup event to widgets that either hold focus or have the
 * WIDGET_UNFOCUSED_BUTTONUP flag set.
 */
void
widget_mousebuttonup(struct window *win, struct widget *wid, int button,
    int x, int y)
{
	struct widget *cwid;

	if ((WINDOW_FOCUSED(win) && widget_holds_focus(wid)) ||
	    (wid->flags & WIDGET_UNFOCUSED_BUTTONUP)) {
		event_post(NULL, wid,  "window-mousebuttonup", "%i, %i, %i",
		    button, x-wid->cx, y-wid->cy);
	}
	OBJECT_FOREACH_CHILD(cwid, wid, widget)
		widget_mousebuttonup(win, cwid, button, x, y);
}

/* Process a mousebuttondown event. */
int
widget_mousebuttondown(struct window *win, struct widget *wid, int button,
    int x, int y)
{
	struct widget *cwid;

	/* Widgets cannot overlap. */
	if (!widget_area(wid, x, y))
		return (0);

	/* Search for a better match. */
	OBJECT_FOREACH_CHILD(cwid, wid, widget) {
		if (widget_mousebuttondown(win, cwid, button, x, y))
			return (1);
	}
	return (event_post(NULL, wid, "window-mousebuttondown", "%i, %i, %i",
	    button, x-wid->cx, y-wid->cy));
}

/* Search for a focused widget inside a window. */
struct widget *
widget_find_focus(void *p)
{
	struct widget *wid = p;
	struct widget *cwid, *fwid;

	if (!OBJECT_TYPE(wid, "window") &&
	    (wid->flags & WIDGET_FOCUSED) == 0)
		return (NULL);

	/* Search for a better match. */
	OBJECT_FOREACH_CHILD(cwid, wid, widget) {
		if ((fwid = widget_find_focus(cwid)) != NULL)
			return (fwid);
	}
	return (wid);
}

void
widget_set_type(void *p, const char *name)
{
	struct widget *wid = p;

	strlcpy(wid->type, name, sizeof(wid->type));
	strlcpy(OBJECT(wid)->name, name, sizeof(OBJECT(wid)->name));
}

/* Push an unnamed entry onto a widget's color stack. */
int	
widget_push_color(struct widget *wid, Uint32 color)
{
	int ncolor;

#ifdef DEBUG
	if (wid->ncolors+1 >= WIDGET_COLORS_MAX)
		fatal("color stack oflow");
#endif
	ncolor = wid->ncolors++;
	wid->colors[ncolor] = color;
	wid->color_names[ncolor][0] = '\0';
	return (ncolor);
}

/* Pop the highest color off a widget's color stack. */
void
widget_pop_color(struct widget *wid)
{
	wid->ncolors--;
}

/*
 * Cache the absolute view coordinates of a widget and its descendents.
 * The view must be locked.
 */
void
widget_update_coords(void *parent, int x, int y)
{
	struct widget *pwid = parent, *cwid;

	pwid->cx = x;
	pwid->cy = y;

	OBJECT_FOREACH_CHILD(cwid, pwid, widget)
		widget_update_coords(cwid, pwid->cx+cwid->x, pwid->cy+cwid->y);
}

