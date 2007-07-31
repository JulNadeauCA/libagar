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
#include <core/view.h>
#include <core/math.h>

#include "widget.h"
#include "window.h"
#include "cursors.h"
#include "menu.h"

#include <stdarg.h>
#include <string.h>

const AG_WidgetOps agWidgetOps = {
	{
		"AG_Widget",
		sizeof(AG_Widget),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* draw */
	AG_WidgetScaleGeneric
};

SDL_Cursor *agCursorToSet = NULL;

static void
child_attached(AG_Event *event)
{
	AG_Widget *pwid = AG_SELF();
	AG_Widget *wid = AG_SENDER();
	const AG_WidgetStyleMod *style = pwid->style;

	/* Inherit style from parent widget. */
	if (style != NULL)
		wid->style = style;
}

AG_Widget *
AG_WidgetNew(void *parent, Uint flags)
{
	AG_Widget *w;

	w = Malloc(sizeof(AG_Widget), M_OBJECT);
	AG_WidgetInit(w, &agWidgetOps, flags);
	AG_ObjectAttach(parent, w);
	return (w);
}

void
AG_WidgetInit(void *p, const void *wops, Uint flags)
{
	AG_Widget *wid = p;

	AG_ObjectInit(wid, "widget", wops);
	AGOBJECT(wid)->save_pfx = "/widgets";

	wid->flags = flags;
	wid->redraw = 1;
	wid->cx = -1;
	wid->cy = -1;
	wid->x = -1;
	wid->y = -1;
	wid->w = -1;
	wid->h = -1;
	wid->style = NULL;
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
	AG_SetEvent(wid, "child-attached", child_attached, NULL);
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
AG_WidgetEnable(void *p)
{
	AG_Widget *wid = p;

	wid->flags &= ~(AG_WIDGET_DISABLED);
}

void
AG_WidgetDisable(void *p)
{
	AG_Widget *wid = p;

	wid->flags |= AG_WIDGET_DISABLED;
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
		dprintf("Translating property: %s:%s\n",
		    AGOBJECT(binding->p1)->name, binding->data.prop);
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
#ifdef SDL_HAS_64BIT_TYPE
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

	binding = Malloc(sizeof(AG_WidgetBinding), M_WIDGET);
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
#ifdef SDL_HAS_64BIT_TYPE
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
#ifdef SDL_HAS_64BIT_TYPE
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

int
AG_WidgetInt(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	int *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

Uint
AG_WidgetUint(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	Uint *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

Uint8
AG_WidgetUint8(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	Uint8 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

Sint8
AG_WidgetSint8(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	Sint8 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

Uint16
AG_WidgetUint16(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	Uint16 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

Sint16
AG_WidgetSint16(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	Sint16 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

Uint32
AG_WidgetUint32(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	Uint32 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

Sint32
AG_WidgetSint32(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	Sint32 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

#ifdef SDL_HAS_64BIT_TYPE
Uint64
AG_WidgetUint64(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	Uint64 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

Sint64
AG_WidgetSint64(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	Sint64 *i, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *i;
	AG_WidgetUnlockBinding(b);
	return (rv);
}
#endif /* SDL_HAS_64BIT_TYPE */

float
AG_WidgetFloat(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	float *f, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &f)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *f;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

double
AG_WidgetDouble(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	double *d, rv;

	if ((b = AG_WidgetGetBinding(wid, name, &d)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *d;
	AG_WidgetUnlockBinding(b);
	return (rv);
}

char *
AG_WidgetString(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	char *s, *sd;

	if ((b = AG_WidgetGetBinding(wid, name, &s)) == NULL) {
		fatal("%s", AG_GetError());
	}
	sd = Strdup(s);
	AG_WidgetUnlockBinding(b);
	return (sd);
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

void *
AG_WidgetPointer(void *wid, const char *name)
{
	AG_WidgetBinding *b;
	void **p, *rv;

	if ((b = AG_WidgetGetBinding(wid, name, &p)) == NULL) {
		fatal("%s", AG_GetError());
	}
	rv = *p;
	AG_WidgetUnlockBinding(b);
	return (p);
}

void
AG_WidgetSetInt(void *wid, const char *name, int ni)
{
	AG_WidgetBinding *binding;
	int *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetSetUint(void *wid, const char *name, Uint ni)
{
	AG_WidgetBinding *binding;
	Uint *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetSetUint8(void *wid, const char *name, Uint8 ni)
{
	AG_WidgetBinding *binding;
	Uint8 *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetSetSint8(void *wid, const char *name, Sint8 ni)
{
	AG_WidgetBinding *binding;
	Sint8 *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetSetUint16(void *wid, const char *name, Uint16 ni)
{
	AG_WidgetBinding *binding;
	Uint16 *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetSetSint16(void *wid, const char *name, Sint16 ni)
{
	AG_WidgetBinding *binding;
	Sint16 *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetSetUint32(void *wid, const char *name, Uint32 ni)
{
	AG_WidgetBinding *binding;
	Uint32 *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetSetSint32(void *wid, const char *name, Sint32 ni)
{
	AG_WidgetBinding *binding;
	Sint32 *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}

#ifdef SDL_HAS_64BIT_TYPE
void
AG_WidgetSetUint64(void *wid, const char *name, Uint64 ni)
{
	AG_WidgetBinding *binding;
	Uint64 *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetSetSint64(void *wid, const char *name, Sint64 ni)
{
	AG_WidgetBinding *binding;
	Sint64 *i;

	if ((binding = AG_WidgetGetBinding(wid, name, &i)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*i = ni;
	AG_WidgetUnlockBinding(binding);
}
#endif /* SDL_HAS_64BIT_TYPE */

void
AG_WidgetSetFloat(void *wid, const char *name, float nf)
{
	AG_WidgetBinding *binding;
	float *f;

	if ((binding = AG_WidgetGetBinding(wid, name, &f)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*f = nf;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetSetDouble(void *wid, const char *name, double nd)
{
	AG_WidgetBinding *binding;
	double *d;

	if ((binding = AG_WidgetGetBinding(wid, name, &d)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*d = nd;
	AG_WidgetUnlockBinding(binding);
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

void
AG_WidgetSetPointer(void *wid, const char *name, void *np)
{
	AG_WidgetBinding *binding;
	void **p;

	if ((binding = AG_WidgetGetBinding(wid, name, &p)) == NULL) {
		fatal("%s", AG_GetError());
	}
	*p = np;
	AG_WidgetUnlockBinding(binding);
}

void
AG_WidgetLockBinding(AG_WidgetBinding *bind)
{
	if (bind->mutex != NULL)
		AG_MutexLock(bind->mutex);
}

void
AG_WidgetUnlockBinding(AG_WidgetBinding *bind)
{
	if (bind->mutex != NULL)
		AG_MutexUnlock(bind->mutex);
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
		if (!AGWIDGET_SURFACE_NODUP(wid,name))
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
 * Free a widget structure.
 *
 * Must be invoked from main event handler context (for texture operations
 * in OpenGL mode).
 */
void
AG_WidgetDestroy(void *p)
{
	AG_Widget *wid = p;
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
		if (wid->surfaces[i] != NULL &&
		    !AGWIDGET_SURFACE_NODUP(wid,i)) {
			SDL_FreeSurface(wid->surfaces[i]);
		}
#ifdef HAVE_OPENGL
		/* TODO Queue this operation? */
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
	Free(wid->surfaces, M_WIDGET);
	Free(wid->surfaceFlags, M_WIDGET);

#ifdef HAVE_OPENGL
	if (agView->opengl) {
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
	if (glname == 0) {
		fatal("bad texture name: %d (widget name = %d)",
		    glname, name);
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

	AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
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

	while ((pwid = AGOBJECT(pwid)->parent) != NULL) {
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
	} else {
		dprintf("%s: no parent window\n", AGOBJECT(wid)->name);
	}

	/* Set the focus flag on the widget and its parents. */
	do {
		if (AG_ObjectIsClass(pwid, "AG_Widget:AG_Window:*")) {
			if (wid->flags & AG_WIDGET_FOCUS_PARENT_WIN) {
				AG_WindowFocus((AG_Window *)pwid);
			}
			break;
		}
#if 0
		if ((pwid->flags & AG_WIDGET_FOCUSABLE) == 0) {
			dprintf("parent (%s) is not focusable\n",
			    AGOBJECT(pwid)->name);
			break;
		}
#endif
		pwid->flags |= AG_WIDGET_FOCUSED;
		AG_PostEvent(AGOBJECT(pwid)->parent, pwid, "widget-gainfocus",
		    NULL);
	} while ((pwid = AGOBJECT(pwid)->parent) != NULL);
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
		    wid->cx > AGWIDGET(owin)->x &&
		    wid->cy > AGWIDGET(owin)->y &&
		    wid->cx+wid->w < AGWIDGET(owin)->x+AGWIDGET(owin)->w &&
		    wid->cy+wid->h < AGWIDGET(owin)->y+AGWIDGET(owin)->h) {
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
AG_WidgetPushClipRect(void *p, int x, int y, Uint w, Uint h)
{
	AG_Widget *wid = p;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		GLdouble eq0[4] = { 1, 0, 0, -(wid->cx+x-1) };
		GLdouble eq1[4] = { 0, 1, 0, -(wid->cy+y-1) };
		GLdouble eq2[4] = { -1, 0, 0, (wid->cx+x+w) };
		GLdouble eq3[4] = { 0, -1, 0, (wid->cy+y+h) };

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

		r.x = wid->cx+x;
		r.y = wid->cy+y;
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
	AG_Widget *cwid;
	SDL_Rect clip_save;

	if (wid->flags & AG_WIDGET_HIDE)
		return;

	if (((wid->flags & AG_WIDGET_STATIC)==0 || wid->redraw) &&
	    !AG_WidgetIsOcculted(wid) && AGWIDGET_OPS(wid)->draw != NULL &&
	    AGWIDGET(wid)->w > 0 && AGWIDGET(wid)->h > 0) {
		if (wid->flags & AG_WIDGET_CLIPPING) {
			AG_WidgetPushClipRect(wid, 0, 0, wid->w, wid->h);
		}
		AGWIDGET_OPS(wid)->draw(wid);
		if (wid->flags & AG_WIDGET_CLIPPING) {
			AG_WidgetPopClipRect(wid);
		}
		if (wid->flags & AG_WIDGET_STATIC)
			wid->redraw = 0;
	}

	AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
		AG_WidgetDraw(cwid);
}

/* Set the geometry of a widget and invoke its scale operation. */
void
AG_WidgetScale(void *p, int w, int h)
{
	AG_Widget *wid = p;

	wid->w = w;
	wid->h = h;
	AGWIDGET_OPS(wid)->scale(wid, wid->w, wid->h);
}

void
AG_WidgetScaleGeneric(void *p, int w, int h)
{
	AG_Widget *wid = p;

	if (w == -1 || h == -1) {
		wid->w = 0;
		wid->h = 0;
	}
}

/*
 * Write a 32-bit pixel value (in agVideoFmt format) to the display,
 * at x,y coordinates with respect to widget's origin. If coordinates
 * are outside of display boundaries, this is a no-op.
 *
 * Must be invoked from widget rendering context. In SDL mode, the
 * display surface must be locked.
 */
void
AG_WidgetPutPixel32(void *p, int wx, int wy, Uint32 color)
{
	AG_Widget *wid = p;
	int vx = wid->cx+wx;
	int vy = wid->cy+wy;
	
	if (AG_CLIPPED_PIXEL(agView->v, vx, vy))
		return;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		Uint8 r, g, b;

		SDL_GetRGB(color, agVideoFmt, &r, &g, &b);
		glBegin(GL_POINTS);
		glColor3ub(r, g, b);
		glVertex2s(vx, vy);
		glEnd();
	} else
#endif
		AG_PUT_PIXEL2(agView->v, vx, vy, color);
}

/*
 * Version of WidgetPutPixel32() with clipping of pixels outside of
 * the widget's allocated area, as opposed to the entire display.
 *
 * Must be invoked from widget rendering context. In SDL mode, the
 * display surface must be locked.
 */
void
AG_WidgetPutPixel32OrClip(void *p, int wx, int wy, Uint32 color)
{
	AG_Widget *wid = p;
	int vx = wid->cx+wx, vy = wid->cy+wy;
	
	if (!AG_WidgetArea(wid, vx, vy))
		return;
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		Uint8 r, g, b;

		SDL_GetRGB(color, agVideoFmt, &r, &g, &b);
		glBegin(GL_POINTS);
		glColor3ub(r, g, b);
		glVertex2s(vx, vy);
		glEnd();
	} else
#endif
		AG_PUT_PIXEL2(agView->v, vx, vy, color);
}

/*
 * Write a pixel value (given in RGB components) to the display,
 * at x,y coordinates with respect to widget's origin. In SDL mode,
 * the display surface must be locked; clipping is done.
 *
 * Must be invoked from widget rendering context. In SDL mode, the
 * display surface must be locked.
 */
void
AG_WidgetPutPixelRGB(void *p, int wx, int wy, Uint8 r, Uint8 g, Uint8 b)
{
	AG_Widget *wid = p;
	int vx = wid->cx+wx, vy = wid->cy+wy;
	
	if (AG_CLIPPED_PIXEL(agView->v, vx, vy))
		return;
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		glBegin(GL_POINTS);
		glColor3ub(r, g, b);
		glVertex2s(vx, vy);
		glEnd();
	} else
#endif
		AG_PUT_PIXEL2(agView->v, vx, vy, SDL_MapRGB(agVideoFmt, r,g,b));
}

/*
 * Version of WidgetPutPixelRGB() with clipping of pixels outside of
 * the widget's allocated area, as opposed to the entire display.
 *
 * Must be invoked from widget rendering context. In SDL mode, the
 * display surface must be locked.
 */
void
AG_WidgetPutPixelRGBOrClip(void *p, int wx, int wy, Uint8 r, Uint8 g, Uint8 b)
{
	AG_Widget *wid = p;
	int vx = wid->cx+wx, vy = wid->cy+wy;
	
	if (!AG_WidgetArea(wid, vx, vy))
		return;
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		glBegin(GL_POINTS);
		glColor3ub(r, g, b);
		glVertex2s(vx, vy);
		glEnd();
	} else
#endif
		AG_PUT_PIXEL2(agView->v, vx, vy, SDL_MapRGB(agVideoFmt, r,g,b));
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
		glColor4ubv((GLbyte *)c);
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
 * Variant of WidgetBlendPixel which accepts a 32-bit pixel in standard
 * reference surface format (agSurfaceFmt).
 */
void
AG_WidgetBlendPixel32(void *p, int wx, int wy, Uint32 pixel, AG_BlendFn fn)
{
	Uint8 c[4];

	SDL_GetRGBA(pixel, agSurfaceFmt, &c[0], &c[1], &c[2], &c[3]);
	AG_WidgetBlendPixelRGBA(p, wx, wy, c, fn);
}

/* Evaluate to true if absolute view coords x,y are inside the widget area. */
int
AG_WidgetArea(void *p, int x, int y)
{
	AG_Widget *wid = p;

	return (x > wid->cx &&
	        y > wid->cy &&
	        x < wid->cx+wid->w &&
		y < wid->cy+wid->h);
}

/* Evaluate to true if widget coords x,y are inside the widget area. */
int
AG_WidgetRelativeArea(void *p, int x, int y)
{
	AG_Widget *wid = p;

	return (x >= 0 &&
	        y >= 0 &&
	        x < wid->w &&
		y < wid->h);
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
	AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
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
	AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
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
	AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
		if (AG_WidgetMouseButtonDown(win, cwid, button, x, y))
			return (1);
	}
	if (!AG_WidgetArea(wid, x, y)) {
		return (0);
	}
	TAILQ_FOREACH(ev, &AGOBJECT(wid)->events, events) {
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
	AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget) {
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

	AGOBJECT_FOREACH_CHILD(cwid, pwid, ag_widget)
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

