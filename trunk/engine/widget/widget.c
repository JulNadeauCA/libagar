/*	$Csoft: widget.c,v 1.63 2003/06/11 23:21:04 vedge Exp $	*/

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
#include <engine/version.h>
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

pthread_mutex_t widget_lock = PTHREAD_MUTEX_INITIALIZER;

void
widget_init(void *p, const char *type, const void *wops, int flags)
{
	struct widget *wid = p;

	object_init(wid, "widget", type, wops);

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
	pthread_mutex_init(&wid->bindings_lock, NULL);
}

int
widget_load(void *p, struct netbuf *buf)
{
	struct widget *wid = p;
	int i;

	if (version_read(buf, &widget_ver, NULL) != 0)
		return (-1);
	
	copy_string(wid->type, buf, sizeof(wid->type));
	wid->flags = (int)read_uint32(buf);
	wid->ncolors = (int)read_uint32(buf);
	if (wid->ncolors >= WIDGET_COLORS_MAX) {
		error_set("color stack oflow");
		return (-1);
	}
	for (i = 0; i < wid->ncolors; i++) {
		Uint8 r, g, b, a;
		char *name;

		name = read_string(buf, NULL);
		r = read_uint8(buf);
		g = read_uint8(buf);
		b = read_uint8(buf);
		a = read_uint8(buf);
		widget_map_color(wid, i, name, r, g, b, a);
		free(name);
	}
	return (0);
}

int
widget_save(void *p, struct netbuf *buf)
{
	struct widget *wid = p;
	int i;

	version_write(buf, &widget_ver);

	write_string(buf, wid->type);
	write_uint32(buf, (Uint32)wid->flags);
	write_uint32(buf, wid->ncolors);
	for (i = 0; i < wid->ncolors; i++) {
		Uint8 r, g, b, a;

		write_string(buf, wid->color_names[i]);
		SDL_GetRGBA(wid->colors[i], vfmt, &r, &g, &b, &a);
		write_uint8(buf, r);
		write_uint8(buf, g);
		write_uint8(buf, b);
		write_uint8(buf, a);
	}
	return (0);
}

/* Bind a variable to a widget. */
struct widget_binding *
widget_bind(void *widp, const char *name, enum widget_binding_type type, ...)
{
	struct widget *wid = widp;
	struct widget_binding *binding;
	pthread_mutex_t *mu = NULL;
	void *p1, *p2 = NULL;
	size_t size = 0;
	va_list ap;

	va_start(ap, type);
	switch (type) {
	case WIDGET_PROP:
		p1 = va_arg(ap, void *);		/* Object */
		p2 = va_arg(ap, char *);		/* Property name */
		break;
	case WIDGET_STRING:
		mu = va_arg(ap, pthread_mutex_t *);	/* Optional mutex */
		p1 = va_arg(ap, void *);		/* Buffer */
		size = va_arg(ap, size_t);		/* Size of buffer */
		break;
	default:
		mu = va_arg(ap, pthread_mutex_t *);	/* Optional mutex */
		p1 = va_arg(ap, void *);		/* Data */
		break;
	}
	va_end(ap);

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
			binding->size = size;
			binding->mutex = mu;

			debug_n(DEBUG_BINDINGS, " -> %d(%p,%p,%p)\n",
			    binding->type, binding->p1, binding->p2,
			    binding->mutex);

			event_post(wid, "widget-bound", "%p", binding);
			pthread_mutex_unlock(&wid->bindings_lock);
			return (binding);
		}
	}

	binding = Malloc(sizeof(struct widget_binding));	/* Create */
	binding->type = type;
	binding->name = Strdup(name);
	binding->p1 = p1;
	binding->p2 = p2;
	binding->size = size;
	binding->mutex = mu;
	SLIST_INSERT_HEAD(&wid->bindings, binding, bindings);

	debug_n(DEBUG_BINDINGS,
	    "%s: bound `%s' to %p (type=%d p2=%p mutex=%p)\n",
	    OBJECT(wid)->name, name, p1, type, p2, mu);

	event_post(wid, "widget-bound", "%p", binding);
	pthread_mutex_unlock(&wid->bindings_lock);
	return (binding);
}

__inline__ int
widget_get_int(void *wid, const char *name)
{
	int *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}

__inline__ unsigned int
widget_get_uint(void *wid, const char *name)
{
	unsigned int *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}

__inline__ Uint8
widget_get_uint8(void *wid, const char *name)
{
	Uint8 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}
__inline__ Sint8
widget_get_sint8(void *wid, const char *name)
{
	Sint8 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}

__inline__ Uint16
widget_get_uint16(void *wid, const char *name)
{
	Uint16 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}
__inline__ Sint16
widget_get_sint16(void *wid, const char *name)
{
	Sint16 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}

__inline__ Uint32
widget_get_uint32(void *wid, const char *name)
{
	Uint32 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}
__inline__ Sint32
widget_get_sint32(void *wid, const char *name)
{
	Sint32 *i;

	if (widget_binding_get(wid, name, &i) == NULL)
		fatal("%s", error_get());
	return (*i);
}

#ifdef FLOATING_POINT
__inline__ float
widget_get_float(void *wid, const char *name)
{
	float *f;

	if (widget_binding_get(wid, name, &f) == NULL)
		fatal("%s", error_get());
	return (*f);
}

__inline__ double
widget_get_double(void *wid, const char *name)
{
	double *d;

	if (widget_binding_get(wid, name, &d) == NULL)
		fatal("%s", error_get());
	return (*d);
}
#endif /* FLOATING_POINT */

__inline__ char *
widget_get_string(void *wid, const char *name)
{
	struct widget_binding *binding;
	char *s, *sd;

	if ((binding = widget_binding_get_locked(wid, name, &s)) == NULL)
		fatal("%s", error_get());
	sd = Strdup(s);
	widget_binding_unlock(binding);
	return (s);
}

__inline__ size_t
widget_copy_string(void *wid, const char *name, char *dst, size_t dst_size)
{
	struct widget_binding *binding;
	char *s;
	size_t rv;

	if ((binding = widget_binding_get_locked(wid, name, &s)) == NULL)
		fatal("%s", error_get());
	rv = strlcpy(dst, s, dst_size);
	widget_binding_unlock(binding);
	return (rv);
}

__inline__ void *
widget_get_pointer(void *wid, const char *name)
{
	void *p;

	if (widget_binding_get(wid, name, &p) == NULL)
		fatal("%s", error_get());
	return (p);
}

__inline__ void
widget_set_int(void *wid, const char *name, int ni)
{
	struct widget_binding *binding;
	int *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

__inline__ void
widget_set_uint(void *wid, const char *name, unsigned int ni)
{
	struct widget_binding *binding;
	unsigned int *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

__inline__ void
widget_set_uint8(void *wid, const char *name, Uint8 ni)
{
	struct widget_binding *binding;
	Uint8 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

__inline__ void
widget_set_sint8(void *wid, const char *name, Sint8 ni)
{
	struct widget_binding *binding;
	Sint8 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

__inline__ void
widget_set_uint16(void *wid, const char *name, Uint16 ni)
{
	struct widget_binding *binding;
	Uint16 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

__inline__ void
widget_set_sint16(void *wid, const char *name, Sint16 ni)
{
	struct widget_binding *binding;
	Sint16 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

__inline__ void
widget_set_uint32(void *wid, const char *name, Uint32 ni)
{
	struct widget_binding *binding;
	Uint32 *i;

	if ((binding = widget_binding_get_locked(wid, name, &i)) == NULL)
		fatal("%s", error_get());
	*i = ni;
	widget_binding_unlock(binding);
}

__inline__ void
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
__inline__ void
widget_set_float(void *wid, const char *name, float nf)
{
	struct widget_binding *binding;
	float *f;

	if ((binding = widget_binding_get_locked(wid, name, &f)) == NULL)
		fatal("%s", error_get());
	*f = nf;
	widget_binding_unlock(binding);
}

__inline__ void
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

__inline__ void
widget_set_string(void *wid, const char *name, char *ns)
{
	struct widget_binding *binding;
	char *s;

	if ((binding = widget_binding_get_locked(wid, name, &s)) == NULL)
		fatal("%s", error_get());
	strlcpy(s, ns, binding->size);
	widget_binding_unlock(binding);
}

__inline__ void
widget_set_pointer(void *wid, const char *name, void *np)
{
	struct widget_binding *binding;
	void **p;

	if ((binding = widget_binding_get_locked(wid, name, &p)) == NULL)
		fatal("%s", error_get());
	*p = np;
	widget_binding_unlock(binding);
}

/* Look for a binding and return the pointer value in p. */
struct widget_binding *
_widget_binding_get(void *widp, const char *name, void *res, int return_locked)
{
	struct widget *wid = widp;
	struct widget_binding *binding;
	struct prop *prop;

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
#ifdef FLOATING_POINT
		case WIDGET_FLOAT:
			*(float **)res = (float *)binding->p1;
			break;
		case WIDGET_DOUBLE:
			*(double **)res = (double *)binding->p1;
			break;
#endif
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
#ifdef FLOATING_POINT
			case PROP_FLOAT:
				*(float **)res = (float *)&prop->data.f;
				break;
			case PROP_DOUBLE:
				*(double **)res = (double *)&prop->data.d;
				break;
#endif
			case PROP_STRING:
				*(char ***)res = (char **)&prop->data.s;
				break;
			case PROP_POINTER:
				*(void ***)res = (void **)&prop->data.p;
				break;
			default:
				fatal("cannot translate prop");
			}
			break;
		default:
			fatal("unknown binding type");
		}
		if (binding->mutex != NULL && !return_locked) {
			pthread_mutex_unlock(binding->mutex);
		}
		pthread_mutex_unlock(&wid->bindings_lock);
		return (binding);
	}
	pthread_mutex_unlock(&wid->bindings_lock);

	error_set("%s: no such binding: `%s'", OBJECT(wid)->name, name);
	return (NULL);
}

__inline__ void
widget_binding_lock(struct widget_binding *bind)
{
	if (bind->mutex != NULL)
		pthread_mutex_lock(bind->mutex);
}

__inline__ void
widget_binding_unlock(struct widget_binding *bind)
{
	if (bind->mutex != NULL)
		pthread_mutex_unlock(bind->mutex);
}

/*
 * Generate a prop-modified event after manipulating the property values
 * manually. The property must be locked.
 */
__inline__ void
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

/* Add a named entry to a widget's color stack. */
void
widget_map_color(void *p, int ind, const char *name, Uint8 r, Uint8 g, Uint8 b,
    Uint8 a)
{
	struct widget *wid = p;

#ifdef DEBUG
	if (ind >= WIDGET_COLORS_MAX)
		fatal("color stack oflow");
#endif
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
		free(bind->name);
		free(bind);
	}
	SLIST_INIT(&wid->bindings);
}

/*
 * Perform a blit from a source surface to the display, at coordinates
 * relative to the widget; clipping is done.
 */
__inline__ void
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
__inline__ int
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
		event_post(wid, "widget-lostfocus", NULL);
	}

	OBJECT_FOREACH_CHILD(cwid, wid, widget)
		widget_clear_focus(cwid);
}

/* Find the parent window of a widget. */
static struct window *
widget_parent_window(struct widget *wid)
{
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
		event_post(pwid, "widget-gainfocus", NULL);
	} while ((pwid = OBJECT(pwid)->parent) != NULL);
}

/* Render a widget and its descendents, recursively. */
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

	lock_linkage();
	OBJECT_FOREACH_CHILD(cwid, wid, widget) {
		widget_draw(cwid);
	}
	unlock_linkage();
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
 * Write to the pixel at widget-relative x,y coordinates; clipping is done.
 * The display surface must be locked.
 */
__inline__ void
widget_put_pixel(void *p, int x, int y, Uint32 color)
{
	struct widget *wid = p;
	Uint8 *pixel = (Uint8 *)view->v->pixels + y*view->v->pitch +
	    x*vfmt->BytesPerPixel;

	if (!VIEW_INSIDE_CLIP_RECT(view->v, wid->cx+x, wid->cy+y))
		return;

	switch (vfmt->BytesPerPixel) {
		_VIEW_PUTPIXEL_32(pixel, color);
		_VIEW_PUTPIXEL_24(pixel, color);
		_VIEW_PUTPIXEL_16(pixel, color);
		_VIEW_PUTPIXEL_8(pixel, color);
	}
}

/* Evaluate to true if absolute view coords x,y are inside the widget area. */
__inline__ int
widget_area(void *p, int x, int y)
{
	struct widget *wid = p;

	return (x > wid->cx &&
	        y > wid->cy &&
	        x < wid->cx + wid->w &&
		y < wid->cy + wid->h);
}

/* Evaluate to true if widget coords x,y are inside the widget area. */
__inline__ int
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
    int xrel, int yrel)
{
	struct widget *cwid;

	if ((WINDOW_FOCUSED(win) && widget_holds_focus(wid)) ||
	    (wid->flags & WIDGET_UNFOCUSED_MOTION)) {
		event_post(wid,  "window-mousemotion", "%i, %i, %i, %i",
		    x - wid->cx,
		    y - wid->cy,
		    xrel,
		    yrel);
	}

	OBJECT_FOREACH_CHILD(cwid, wid, widget)
		widget_mousemotion(win, cwid, x, y, xrel, yrel);
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
		event_post(wid,  "window-mousebuttonup", "%i, %i, %i",
		    button,
		    x - wid->cx,
		    y - wid->cy);
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

	return (event_post(wid, "window-mousebuttondown", "%i, %i, %i", button,
	    x - wid->cx,
	    y - wid->cy));
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
__inline__ int	
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
__inline__ void
widget_pop_color(struct widget *wid)
{
	wid->ncolors--;
}

