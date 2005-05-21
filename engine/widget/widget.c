/*	$Csoft: widget.c,v 1.111 2005/05/20 05:56:40 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include <stdarg.h>
#include <string.h>

const struct version widget_ver = {
	"agar widget",
	0, 0
};

static void
inherit_style(int argc, union evarg *argv)
{
	struct widget *pwid = argv[0].p;
	struct widget *wid = argv[argc].p;
	const struct style *style = pwid->style;

	if (style == NULL)
		return;

	wid->style = style;
}

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
	wid->style = NULL;
	SLIST_INIT(&wid->bindings);
	pthread_mutex_init(&wid->bindings_lock, &recursive_mutexattr);

	wid->nsurfaces = 0;
	wid->surfaces = NULL;
#ifdef HAVE_OPENGL
	wid->textures = NULL;
	wid->texcoords = NULL;
#endif

	/*
	 * Arrange for immediate children to inherit the style settings
	 * of the parent on attachment.
	 */
	event_new(wid, "child-attached", inherit_style, NULL);
}

/* Bind a mutex-protected variable to a widget. */
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
			binding->type = type;
			binding->p1 = p1;
			binding->p2 = p2;
			binding->size = size;
			binding->vtype = widget_vtype(binding);

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
			*(u_int **)res = (u_int *)binding->p1;
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

u_int
widget_get_uint(void *wid, const char *name)
{
	struct widget_binding *b;
	u_int *i, rv;

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
widget_set_uint(void *wid, const char *name, u_int ni)
{
	struct widget_binding *binding;
	u_int *i;

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

/*
 * Register a surface with the given widget. In OpenGL mode, generate
 * a texture as well. Mapped surfaces are automatically freed when the
 * widget is destroyed.
 */
int
widget_map_surface(void *p, SDL_Surface *su)
{
	struct widget *wid = p;
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
#ifdef HAVE_OPENGL
		if (view->opengl) {
			wid->textures = Realloc(wid->textures,
			    (wid->nsurfaces+1)*sizeof(GLuint));
			wid->texcoords = Realloc(wid->texcoords,
			    (wid->nsurfaces+1)*sizeof(GLfloat)*4);
		}
#endif
		idx = wid->nsurfaces++;
	}

	wid->surfaces[idx] = su;
#ifdef HAVE_OPENGL
	if (view->opengl) {
		GLuint texname;
	
		texname = (su == NULL) ? 0 :
		    view_surface_texture(su, &wid->texcoords[idx*4]);
		wid->textures[idx] = texname;
	}
#endif
	return (idx);
}

/*
 * Replace the contents of a registered surface, and regenerate
 * the matching texture in OpenGL mode.
 */
void
widget_replace_surface(void *p, int name, SDL_Surface *su)
{
	struct widget *wid = p;

	if (wid->surfaces[name] != NULL) {
		SDL_FreeSurface(wid->surfaces[name]);
	}
	wid->surfaces[name] = su;
#ifdef HAVE_OPENGL
	if (view->opengl) {
		if (wid->textures[name] != 0) {
			glDeleteTextures(1, &wid->textures[name]);
		}
		wid->textures[name] = (su == NULL) ? 0 :
		    view_surface_texture(su, &wid->texcoords[name*4]);
	}
#endif
}

void
widget_update_surface(void *p, int name)
{
	struct widget *wid = p;

#ifdef HAVE_OPENGL
	if (view->opengl) {
		view_update_texture(wid->surfaces[name],
		    wid->textures[name]);
	}
#endif
}

void
widget_destroy(void *p)
{
	struct widget *wid = p;
	struct widget_binding *bind, *nbind;
	u_int i;

	for (i = 0; i < wid->nsurfaces; i++) {
		if (wid->surfaces[i] != NULL)
			SDL_FreeSurface(wid->surfaces[i]);
#ifdef HAVE_OPENGL
		if (view->opengl &&
		    wid->textures[i] != 0)
			glDeleteTextures(1, &wid->textures[i]);
#endif
	}
	Free(wid->surfaces, M_WIDGET);

#ifdef HAVE_OPENGL
	if (view->opengl) {
		Free(wid->textures, M_WIDGET);
		Free(wid->texcoords, M_WIDGET);
	}
#endif

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
		int alpha = (srcsu->flags & (SDL_SRCALPHA|SDL_SRCCOLORKEY));
		GLboolean blend_sv;
		GLenum blend_sfactor, blend_dfactor;
		GLfloat texenvmode;

		texture = view_surface_texture(srcsu, texcoord);

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
		SDL_BlitSurface(srcsu, NULL, view->v, &rd);
	}
}

/*
 * Perform a fast blit from a registered surface to the display
 * at coordinates relative to the widget; clipping is done.
 */
void
widget_blit_from(void *p, void *srcp, int name, SDL_Rect *rs, int x, int y)
{
	struct widget *wid = p;
	struct widget *srcwid = srcp;
	SDL_Surface *su = srcwid->surfaces[name];
	SDL_Rect rd;

	if (name == -1 || su == NULL)
		return;

	rd.x = wid->cx + x;
	rd.y = wid->cy + y;
	rd.w = su->w <= wid->w ? su->w : wid->w;		/* Clip */
	rd.h = su->h <= wid->h ? su->h : wid->h;		/* Clip */

#ifdef HAVE_OPENGL
	if (view->opengl) {
		GLfloat tmptexcoord[4];
		GLuint texture = srcwid->textures[name];
		GLfloat *texcoord;
		int alpha = su->flags & (SDL_SRCALPHA|SDL_SRCCOLORKEY);
		GLboolean blend_sv;
		GLenum blend_sfactor, blend_dfactor;
		GLfloat texenvmode;

		if (rs == NULL) {
			texcoord = &srcwid->texcoords[name*4];
		} else {
			texcoord = &tmptexcoord[0];
			texcoord[0] = (GLfloat)rs->x/powof2(rs->x);
			texcoord[1] = (GLfloat)rs->y/powof2(rs->y);
			texcoord[2] = (GLfloat)rs->w/powof2(rs->w);
			texcoord[3] = (GLfloat)rs->h/powof2(rs->h);
		}
#ifdef DEBUG
		if (texture == 0)
			fatal("invalid texture name");
#endif
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
		SDL_BlitSurface(su, rs, view->v, &rd);
	}
}

/* Evaluate to true if a widget is holding focus (inside its parent). */
int
widget_holds_focus(void *p)
{
	return (WIDGET(p)->flags & WIDGET_FOCUSED);
}

/* Clear the WIDGET_FOCUSED bit from a widget and its descendents. */
void
widget_unset_focus(void *p)
{
	struct widget *wid = p, *cwid;

	if (wid->flags & WIDGET_FOCUSED) {
		wid->flags &= ~(WIDGET_FOCUSED);
		event_post(NULL, wid, "widget-lostfocus", NULL);
	}

	OBJECT_FOREACH_CHILD(cwid, wid, widget)
		widget_unset_focus(cwid);
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

	if ((wid->flags & WIDGET_FOCUSABLE) == 0)
		return;

	/* Remove focus from other widgets inside this window. */
	pwin = widget_parent_window(wid);
	if (pwin != NULL) {
		if (pwin->flags & WINDOW_INHIBIT_FOCUS) {
			return;
		}
		widget_unset_focus(pwin);
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

/* Evaluate whether a given widget is at least partially visible. */
/* TODO optimize on a per window basis */
static __inline__ int
widget_completely_occulted(struct widget *wid)
{
	struct window *owin;
	struct window *wwin;

	if ((wwin = object_find_parent(wid, NULL, "window")) == NULL ||
	    (owin = TAILQ_NEXT(wwin, windows)) == NULL) {
		return (0);
	}
	for (; owin != TAILQ_END(&view->windows);
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

/*
 * Render a widget and its descendents, recursively.
 * The view must be locked.
 */
void
widget_draw(void *p)
{
	struct widget *wid = p;
	struct widget *cwid;
#ifdef HAVE_OPENGL
	GLdouble plane0sv[4];
	GLdouble plane1sv[4];
	GLdouble plane2sv[4];
	GLdouble plane3sv[4];
	int plane0ena = 0;		/* XXX warnings */
	int plane1ena = 0;
	int plane2ena = 0;
	int plane3ena = 0;
#ifdef DEBUG
	GLdouble plane4sv[4];
	int plane4ena;
#endif
#endif /* HAVE_OPENGL */

	if (WIDGET_OPS(wid)->draw != NULL &&
	    !widget_completely_occulted(wid)) {
		SDL_Rect clip_save;

		if (wid->flags & WIDGET_CLIPPING) {
			if (!view->opengl) {
				SDL_Rect clip;

				clip.x = wid->cx;
				clip.y = wid->cy;
				clip.w = wid->w;
				clip.h = wid->h;
				SDL_GetClipRect(view->v, &clip_save);
				SDL_SetClipRect(view->v, &clip);
			} else {
#ifdef HAVE_OPENGL
				GLdouble eq0[4] = { 1, 0, 0, -wid->cx };
				GLdouble eq1[4] = { 0, 1, 0, -wid->cy };
				GLdouble eq2[4] = { -1, 0, 0, wid->cx2-1 };
				GLdouble eq3[4] = { 0, -1, 0, wid->cy2-1 };

				plane0ena = glIsEnabled(GL_CLIP_PLANE0);
				plane1ena = glIsEnabled(GL_CLIP_PLANE1);
				plane2ena = glIsEnabled(GL_CLIP_PLANE2);
				plane3ena = glIsEnabled(GL_CLIP_PLANE3);

				glGetClipPlane(GL_CLIP_PLANE0, plane0sv);
				glGetClipPlane(GL_CLIP_PLANE1, plane1sv);
				glGetClipPlane(GL_CLIP_PLANE2, plane2sv);
				glGetClipPlane(GL_CLIP_PLANE3, plane3sv);

				glClipPlane(GL_CLIP_PLANE0, eq0);
				glClipPlane(GL_CLIP_PLANE1, eq1);
				glClipPlane(GL_CLIP_PLANE2, eq2);
				glClipPlane(GL_CLIP_PLANE3, eq3);
				
				glEnable(GL_CLIP_PLANE0);
				glEnable(GL_CLIP_PLANE1);
				glEnable(GL_CLIP_PLANE2);
				glEnable(GL_CLIP_PLANE3);
#endif /* HAVE_OPENGL */
			}
		}

		WIDGET_OPS(wid)->draw(wid);
		
		if (wid->flags & WIDGET_CLIPPING) {
			if (!view->opengl) {
				SDL_SetClipRect(view->v, &clip_save);
			} else {
#ifdef HAVE_OPENGL
				glClipPlane(GL_CLIP_PLANE0, plane0sv);
				glClipPlane(GL_CLIP_PLANE1, plane1sv);
				glClipPlane(GL_CLIP_PLANE2, plane2sv);
				glClipPlane(GL_CLIP_PLANE3, plane3sv);
				
				if (plane0ena) {
					glEnable(GL_CLIP_PLANE0);
				} else {
					glDisable(GL_CLIP_PLANE0);
				}
				if (plane1ena) {
					glEnable(GL_CLIP_PLANE1);
				} else {
					glDisable(GL_CLIP_PLANE1);
				}
				if (plane2ena) {
					glEnable(GL_CLIP_PLANE2);
				} else {
					glDisable(GL_CLIP_PLANE2);
				}
				if (plane3ena) {
					glEnable(GL_CLIP_PLANE3);
				} else {
					glDisable(GL_CLIP_PLANE3);
				}
#endif /* HAVE_OPENGL */
			}
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

	VIEW_PUT_PIXEL2_CLIPPED(wid->cx+wx, wid->cy+wy, color);
}

/*
 * Blend with the pixel at widget-relative x,y coordinates.
 * The display surface must be locked; clipping is done.
 */
void
widget_blend_pixel(void *p, int wx, int wy, Uint8 c[4],
    enum view_blend_func func)
{
	struct widget *wid = p;

	BLEND_RGBA2_CLIPPED(view->v, wid->cx+wx, wid->cy+wy, c[0], c[1],
	    c[2], c[3], func);
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
	pwid->cx2 = x + pwid->w;
	pwid->cy2 = y + pwid->h;

	OBJECT_FOREACH_CHILD(cwid, pwid, widget)
		widget_update_coords(cwid, pwid->cx+cwid->x, pwid->cy+cwid->y);
}

/* Parse a generic size specification. */
enum widget_size_spec
widget_parse_sizespec(const char *spec_text, int *w)
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
		return (WIDGET_PERCENT);
	case '>':
		if (spec[0] != '<') {
			return (WIDGET_BAD_SPEC);
		}
		*p = '\0';
		text_prescale(&spec[1], w, NULL);
		return (WIDGET_STRINGLEN);
	case 'x':
		{
			const char *ep;

			if ((ep = strchr(spec, 'p')) == NULL) {
				return (WIDGET_BAD_SPEC);
			}
			ep = '\0';
			*w = (int)strtol(ep, NULL, 10);
			return (WIDGET_PIXELS);
		}
	default:
		text_prescale(spec, w, NULL);
		break;
	}
	return (WIDGET_BAD_SPEC);
}

