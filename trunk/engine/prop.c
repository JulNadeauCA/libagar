/*	$Csoft: prop.c,v 1.51 2004/05/24 00:36:52 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <config/have_ieee754.h>

#include <engine/engine.h>

#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

const struct version prop_ver = {
	"agar property map",
	2, 0
};

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_SET	0x02

int	prop_debug = 0;
#define engine_debug prop_debug
#endif

/* Return the copy of a property. */
struct prop *
prop_copy(const struct prop *prop)
{
	struct prop *nprop;

	nprop = Malloc(sizeof(struct prop), M_PROP);
	memcpy(nprop, prop, sizeof(struct prop));

	switch (prop->type) {
	case PROP_STRING:
		nprop->data.s = strdup(prop->data.s);
		if (nprop->data.s == NULL) {
			Free(nprop, M_PROP);
			error_set(_("Out of memory for string."));
			return (NULL);
		}
		break;
	default:
		break;
	}
	return (0);
}

/* Modify a property, or create a new one if it does not exist. */
struct prop *
prop_set(void *p, const char *key, enum prop_type type, ...)
{
	struct object *ob = p;
	struct prop *nprop = NULL, *oprop;
	int modify = 0;
	va_list ap;

	TAILQ_FOREACH(oprop, &ob->props, props) {
		if (oprop->type == type &&
		    strcmp(oprop->key, key) == 0) {
			nprop = oprop;
			break;
		}
	}
	if (nprop == NULL) {
		nprop = Malloc(sizeof(struct prop), M_PROP);
		strlcpy(nprop->key, key, sizeof(nprop->key));
		nprop->type = type;
	} else {
		modify++;
	}

	va_start(ap, type);
	switch (type) {
	case PROP_UINT:
		nprop->data.u = va_arg(ap, unsigned int);
		debug(DEBUG_SET, "uint %s: %d\n", key, nprop->data.u);
		break;
	case PROP_INT:
		nprop->data.i = va_arg(ap, int);
		debug(DEBUG_SET, "int %s: %d\n", key, nprop->data.i);
		break;
	case PROP_BOOL:
		nprop->data.i = va_arg(ap, int);		/* Promoted */
		debug(DEBUG_SET, "bool %s: %d\n", key, nprop->data.i);
		break;
	case PROP_UINT8:
		nprop->data.u8 = (Uint8)va_arg(ap, int);	/* Promoted */
		debug(DEBUG_SET, "u8 %s: %d\n", key, nprop->data.u8);
		break;
	case PROP_SINT8:
		nprop->data.s8 = (Sint8)va_arg(ap, int);	/* Promoted */
		debug(DEBUG_SET, "s8 %s: %d\n", key, nprop->data.s8);
		break;
	case PROP_UINT16:
		nprop->data.u16 = (Uint16)va_arg(ap, int);	/* Promoted */
		debug(DEBUG_SET, "u16 %s: %d\n", key, nprop->data.u16);
		break;
	case PROP_SINT16:
		nprop->data.s16 = (Sint16)va_arg(ap, int);	/* Promoted */
		debug(DEBUG_SET, "s16 %s: %d\n", key, nprop->data.s16);
		break;
	case PROP_UINT32:
		nprop->data.u32 = va_arg(ap, Uint32);
		debug(DEBUG_SET, "u32 %s: %d\n", key, nprop->data.u32);
		break;
	case PROP_SINT32:
		nprop->data.s32 = va_arg(ap, Sint32);
		debug(DEBUG_SET, "s32 %s: %d\n", key, nprop->data.s32);
		break;
#ifdef FLOATING_POINT
	case PROP_FLOAT:
		nprop->data.f = (float)va_arg(ap, double);	/* Promoted */
		debug(DEBUG_SET, "float %s: %f\n", key, nprop->data.f);
		break;
	case PROP_DOUBLE:
		nprop->data.d = va_arg(ap, double);
		debug(DEBUG_SET, "double %s: %f\n", key, nprop->data.d);
		break;
#endif
	case PROP_STRING:
		nprop->data.s = va_arg(ap, char *);
		debug(DEBUG_SET, "string %s: %s\n", key, nprop->data.s);
		break;
	case PROP_POINTER:
		nprop->data.p = va_arg(ap, void *);
		debug(DEBUG_SET, "pointer %s: %p\n", key, nprop->data.p);
		break;
	default:
		fatal("unsupported type: %d", type);
	}
	va_end(ap);

	pthread_mutex_lock(&ob->lock);
	if (!modify) {
		TAILQ_INSERT_HEAD(&ob->props, nprop, props);
	} else {
		event_post(NULL, ob, "prop-modified", "%p", nprop);
	}
	pthread_mutex_unlock(&ob->lock);
	return (nprop);
}

struct prop *
prop_set_uint(void *ob, const char *key, unsigned int i)
{
	return (prop_set(ob, key, PROP_UINT, i));
}

struct prop *
prop_set_int(void *ob, const char *key, int i)
{
	return (prop_set(ob, key, PROP_INT, i));
}

struct prop *
prop_set_uint8(void *ob, const char *key, Uint8 i)
{
	return (prop_set(ob, key, PROP_UINT8, i));
}

struct prop *
prop_set_sint8(void *ob, const char *key, Sint8 i)
{
	return (prop_set(ob, key, PROP_SINT8, i));
}

struct prop *
prop_set_uint16(void *ob, const char *key, Uint16 i)
{
	return (prop_set(ob, key, PROP_UINT16, i));
}

struct prop *
prop_set_sint16(void *ob, const char *key, Sint16 i)
{
	return (prop_set(ob, key, PROP_SINT16, i));
}

struct prop *
prop_set_uint32(void *ob, const char *key, Uint32 i)
{
	return (prop_set(ob, key, PROP_UINT32, i));
}

struct prop *
prop_set_sint32(void *ob, const char *key, Sint32 i)
{
	return (prop_set(ob, key, PROP_SINT32, i));
}

#ifdef FLOATING_POINT
struct prop *
prop_set_float(void *ob, const char *key, float f)
{
	return (prop_set(ob, key, PROP_FLOAT, f));
}

struct prop *
prop_set_double(void *ob, const char *key, double d)
{
	return (prop_set(ob, key, PROP_DOUBLE, d));
}
#endif

struct prop *
prop_set_string(void *ob, const char *key, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	return (prop_set(ob, key, PROP_STRING, s));
}

struct prop *
prop_set_pointer(void *ob, const char *key, void *p)
{
	return (prop_set(ob, key, PROP_POINTER, p));
}

struct prop *
prop_set_bool(void *ob, const char *key, int i)
{
	return (prop_set(ob, key, PROP_BOOL, i));
}

void
prop_lock(void *p)
{
	pthread_mutex_lock(&OBJECT(p)->lock);
}

void
prop_unlock(void *p)
{
	pthread_mutex_unlock(&OBJECT(p)->lock);
}

/* Return a pointer to the data of a property. */
struct prop *
prop_get(void *obp, const char *key, enum prop_type t, void *p)
{
	struct object *ob = obp;
	struct prop *prop;

	pthread_mutex_lock(&ob->lock);
	TAILQ_FOREACH(prop, &ob->props, props) {
		if (strcmp(key, prop->key) != 0)
			continue;
		if (t != PROP_ANY && t != prop->type)
			continue;

		if (p != NULL) {
			switch (prop->type) {
			case PROP_INT:
			case PROP_BOOL:
				*(int *)p = prop->data.i;
				break;
			case PROP_UINT:
				*(unsigned int *)p = prop->data.u;
				break;
			case PROP_UINT8:
				*(Uint8 *)p = prop->data.u8;
				break;
			case PROP_SINT8:
				*(Sint8 *)p = prop->data.s8;
				break;
			case PROP_UINT16:
				*(Uint16 *)p = prop->data.u16;
				break;
			case PROP_SINT16:
				*(Sint16 *)p = prop->data.s16;
				break;
			case PROP_UINT32:
				*(Uint32 *)p = prop->data.u32;
				break;
			case PROP_SINT32:
				*(Sint32 *)p = prop->data.s32;
				break;
#ifdef FLOATING_POINT
			case PROP_FLOAT:
				*(float *)p = prop->data.f;
				break;
			case PROP_DOUBLE:
				*(double *)p = prop->data.d;
				break;
#endif
			case PROP_STRING:
				*(char **)p = prop->data.s;
				break;
			case PROP_POINTER:
				*(void **)p = prop->data.p;
				break;
			default:
				error_set(_("Unsupported property type: %d."),
				    t);
				goto fail;
			}
		}
		pthread_mutex_unlock(&ob->lock);
		return (prop);
	}

	error_set(_("%s has no `%s' property (type %d)."), ob->name, key, t);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (NULL);
}

unsigned int
prop_get_uint(void *p, const char *key)
{
	unsigned int i;

	if (prop_get(p, key, PROP_UINT, &i) == NULL) {
		fatal("%s", error_get());
	}
	return (i);
}

int
prop_get_int(void *p, const char *key)
{
	int i;

	if (prop_get(p, key, PROP_INT, &i) == NULL) {
		fatal("%s", error_get());
	}
	return (i);
}

int
prop_get_bool(void *p, const char *key)
{
	int i;

	if (prop_get(p, key, PROP_BOOL, &i) == NULL) {
		fatal("%s", error_get());
	}
	return (i);
}

Uint8
prop_get_uint8(void *p, const char *key)
{
	Uint8 i;

	if (prop_get(p, key, PROP_UINT8, &i) == NULL) {
		fatal("%s", error_get());
	}
	return (i);
}

Sint8
prop_get_sint8(void *p, const char *key)
{
	Sint8 i;

	if (prop_get(p, key, PROP_SINT8, &i) == NULL) {
		fatal("%s", error_get());
	}
	return (i);
}

Uint16
prop_get_uint16(void *p, const char *key)
{
	Uint16 i;

	if (prop_get(p, key, PROP_UINT16, &i) == NULL) {
		fatal("%s", error_get());
	}
	return (i);
}

Sint16
prop_get_sint16(void *p, const char *key)
{
	Sint16 i;

	if (prop_get(p, key, PROP_SINT16, &i) == NULL) {
		fatal("%s", error_get());
	}
	return (i);
}

Uint32
prop_get_uint32(void *p, const char *key)
{
	Uint32 i;

	if (prop_get(p, key, PROP_UINT32, &i) == NULL) {
		fatal("%s", error_get());
	}
	return (i);
}

Sint32
prop_get_sint32(void *p, const char *key)
{
	Sint32 i;

	if (prop_get(p, key, PROP_SINT32, &i) == NULL) {
		fatal("%s", error_get());
	}
	return (i);
}

#ifdef FLOATING_POINT
float
prop_get_float(void *p, const char *key)
{
	float f;

	if (prop_get(p, key, PROP_FLOAT, &f) == NULL) {
		fatal("%s", error_get());
	}
	return (f);
}

double
prop_get_double(void *p, const char *key)
{
	double d;

	if (prop_get(p, key, PROP_DOUBLE, &d) == NULL) {
		fatal("%s", error_get());
	}
	return (d);
}
#endif /* FLOATING_POINT */

/* The object must be locked. */
char *
prop_get_string(void *p, const char *key)
{
	char *s;

	if (prop_get(p, key, PROP_STRING, &s) == NULL) {
		fatal("%s", error_get());
	}
	return (s);
}

size_t
prop_copy_string(void *p, const char *key, char *buf, size_t bufsize)
{
	size_t sl;
	char *s;

	prop_lock(p);
	if (prop_get(p, key, PROP_STRING, &s) == NULL) {
		fatal("%s", error_get());
	}
	sl = strlcpy(buf, s, bufsize);
	prop_unlock(p);
	return (sl);
}

void *
prop_get_pointer(void *p, const char *key)
{
	void *np;

	if (prop_get(p, key, PROP_POINTER, &np) == NULL) {
		fatal("%s", error_get());
	}
	return (np);
}

int
prop_load(void *p, struct netbuf *buf)
{
	struct object *ob = p;
	Uint32 i, nprops;

	if (version_read(buf, &prop_ver, NULL) == -1)
		return (-1);

	pthread_mutex_lock(&ob->lock);

	if ((ob->flags & OBJECT_RELOAD_PROPS) == 0)
		object_free_props(ob);

	nprops = read_uint32(buf);
	for (i = 0; i < nprops; i++) {
		char key[PROP_KEY_MAX];
		Uint32 t;

		if (copy_string(key, buf, sizeof(key)) >= sizeof(key)) {
			error_set(_("The prop key too big."));
			goto fail;
		}
		t = read_uint32(buf);
		
		debug(DEBUG_STATE, "prop %s (%d)\n", key, t);
	
		switch (t) {
		case PROP_BOOL:
			prop_set_bool(ob, key, (int)read_uint8(buf));
			break;
		case PROP_UINT8:
			prop_set_bool(ob, key, read_uint8(buf));
			break;
		case PROP_SINT8:
			prop_set_bool(ob, key, read_sint8(buf));
			break;
		case PROP_UINT16:
			prop_set_uint16(ob, key, read_uint16(buf));
			break;
		case PROP_SINT16:
			prop_set_sint16(ob, key, read_sint16(buf));
			break;
		case PROP_UINT32:
			prop_set_uint32(ob, key, read_uint32(buf));
			break;
		case PROP_SINT32:
			prop_set_sint32(ob, key, read_sint32(buf));
			break;
		case PROP_UINT:
			prop_set_uint(ob, key, (unsigned int)read_uint32(buf));
			break;
		case PROP_INT:
			prop_set_int(ob, key, (int)read_sint32(buf));
			break;
#if defined(FLOATING_POINT) && defined(HAVE_IEEE754)
		case PROP_FLOAT:
			prop_set_float(ob, key, read_float(buf));
			break;
		case PROP_DOUBLE:
			prop_set_double(ob, key, read_double(buf));
			break;
#endif
		case PROP_STRING:
			{
				char *s;

				s = read_string(buf);
				if (strlen(s) >= PROP_STRING_MAX) {
					error_set(_("String too big."));
					Free(s, 0);
					goto fail;
				}
				prop_set_string(ob, key, "%s", s);
				Free(s, 0);
			}
			break;
		default:
			error_set(_("Cannot load prop of type 0x%x."), t);
			goto fail;
		}
	}
	pthread_mutex_unlock(&ob->lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

int
prop_save(void *p, struct netbuf *buf)
{
	struct object *ob = p;
	off_t count_offs;
	Uint32 nprops = 0;
	struct prop *prop;
	Uint8 c;
	
	version_write(buf, &prop_ver);
	
	pthread_mutex_lock(&ob->lock);

	count_offs = netbuf_tell(buf);				/* Skip count */
	write_uint32(buf, 0);

	TAILQ_FOREACH(prop, &ob->props, props) {
		write_string(buf, (char *)prop->key);
		write_uint32(buf, prop->type);
		debug(DEBUG_STATE, "%s -> %s\n", ob->name, prop->key);
		switch (prop->type) {
		case PROP_BOOL:
			c = (prop->data.i == 1) ? 1 : 0;
			write_uint8(buf, c);
			break;
		case PROP_UINT8:
			write_uint8(buf, prop->data.u8);
			break;
		case PROP_SINT8:
			write_sint8(buf, prop->data.s8);
			break;
		case PROP_UINT16:
			write_uint16(buf, prop->data.u16);
			break;
		case PROP_SINT16:
			write_sint16(buf, prop->data.s16);
			break;
		case PROP_UINT32:
			write_uint32(buf, prop->data.u32);
			break;
		case PROP_SINT32:
			write_sint32(buf, prop->data.s32);
			break;
		case PROP_UINT:
			write_uint32(buf, (Uint32)prop->data.u);
			break;
		case PROP_INT:
			write_sint32(buf, (Sint32)prop->data.i);
			break;
#if defined(FLOATING_POINT) && defined(HAVE_IEEE754)
		case PROP_FLOAT:
			write_float(buf, prop->data.f);
			break;
		case PROP_DOUBLE:
			write_double(buf, prop->data.d);
			break;
#endif
		case PROP_STRING:
			write_string(buf, prop->data.s);
			break;
		case PROP_POINTER:
			debug(DEBUG_STATE,
			    "skipped machine-dependent property `%s'\n",
			    prop->key);
			break;
		default:
			error_set(_("Cannot save prop of type 0x%x."),
			    prop->type);
			goto fail;
		}
		nprops++;
	}
	pthread_mutex_unlock(&ob->lock);
	pwrite_uint32(buf, nprops, count_offs);		/* Write count */
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

void
prop_destroy(struct prop *prop)
{
	switch (prop->type) {
	case PROP_STRING:
		Free(prop->data.s, 0);
		break;
	}
}

void
prop_print_value(char *s, size_t len, struct prop *prop)
{
	switch (prop->type) {
	case PROP_UINT:
		snprintf(s, len, "%u", prop->data.u);
		break;
	case PROP_INT:
		snprintf(s, len, "%d", prop->data.i);
		break;
	case PROP_UINT8:
		snprintf(s, len, "%u", prop->data.u8);
		break;
	case PROP_SINT8:
		snprintf(s, len, "%d", prop->data.s8);
		break;
	case PROP_UINT16:
		snprintf(s, len, "%u", prop->data.u16);
		break;
	case PROP_SINT16:
		snprintf(s, len, "%d", prop->data.s16);
		break;
	case PROP_UINT32:
		snprintf(s, len, "%u", prop->data.u32);
		break;
	case PROP_SINT32:
		snprintf(s, len, "%d", prop->data.s32);
		break;
	case PROP_FLOAT:
		snprintf(s, len, "%f", prop->data.f);
		break;
	case PROP_DOUBLE:
		snprintf(s, len, "%f", prop->data.d);
		break;
	case PROP_STRING:
		strlcpy(s, prop->data.s, len);
		break;
	case PROP_POINTER:
		snprintf(s, len, "%p", prop->data.p);
		break;
	case PROP_BOOL:
		strlcpy(s, prop->data.i ? "true" : "false", len);
		break;
	default:
		snprintf(s, len, "?");
		break;
	}
}

