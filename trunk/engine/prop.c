/*	$Csoft: prop.c,v 1.19 2003/01/23 03:58:09 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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
#include <config/have_long_double.h>

#include "compat/vasprintf.h"
#include "engine.h"

#include <fcntl.h>

#include <libfobj/fobj.h>

#include "version.h"

static const struct version prop_ver = {
	"agar property map",
	1, 0
};

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_SET	0x02

int	prop_debug = 0;
#define engine_debug prop_debug
#endif

/* Modify a property, or create a new one if it does not exist. */
struct prop *
prop_set(void *p, char *key, enum prop_type type, ...)
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
		nprop = emalloc(sizeof(struct prop));
		nprop->key = Strdup(key);
		nprop->type = type;
	} else {
		modify++;
	}

	va_start(ap, type);
	switch (type) {
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
#ifdef SDL_HAS_64BIT_TYPE
	case PROP_UINT64:
		nprop->data.u64 = va_arg(ap, Uint64);
		debug(DEBUG_SET, "u64 %s: %ld\n", key,
		    (long)nprop->data.u64);
		break;
	case PROP_SINT64:
		nprop->data.s64 = va_arg(ap, Sint64);
		debug(DEBUG_SET, "s64 %s: %ld\n", key,
		    (long)nprop->data.s64);
		break;
#endif
	case PROP_FLOAT:
		nprop->data.f = (float)va_arg(ap, double);	/* Promoted */
		debug(DEBUG_SET, "float %s: %f\n", key, nprop->data.f);
		break;
	case PROP_DOUBLE:
		nprop->data.d = va_arg(ap, double);
		debug(DEBUG_SET, "double %s: %f\n", key, nprop->data.d);
		break;
#ifdef HAVE_LONG_DOUBLE
	case PROP_LONG_DOUBLE:
		nprop->data.ld = va_arg(ap, long double);
		debug(DEBUG_SET, "long double %s: %Lf\n", key, nprop->data.ld);
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
		fatal("unsupported type: %d\n", type);
	}
	va_end(ap);

	pthread_mutex_lock(&ob->props_lock);
	if (!modify) {
		TAILQ_INSERT_HEAD(&ob->props, nprop, props);
		event_post(ob, "prop-registered", "%p", nprop);
	} else {
		event_post(ob, "prop-modified", "%p", nprop);
	}
	pthread_mutex_unlock(&ob->props_lock);
	return (nprop);
}

struct prop *
prop_set_int(void *ob, char *key, int i)
{
	return (prop_set(ob, key, PROP_INT, i));
}

struct prop *
prop_set_uint8(void *ob, char *key, Uint8 i)
{
	return (prop_set(ob, key, PROP_UINT8, i));
}

struct prop *
prop_set_sint8(void *ob, char *key, Sint8 i)
{
	return (prop_set(ob, key, PROP_SINT8, i));
}

struct prop *
prop_set_uint16(void *ob, char *key, Uint16 i)
{
	return (prop_set(ob, key, PROP_UINT16, i));
}

struct prop *
prop_set_sint16(void *ob, char *key, Sint16 i)
{
	return (prop_set(ob, key, PROP_SINT16, i));
}

struct prop *
prop_set_uint32(void *ob, char *key, Uint32 i)
{
	return (prop_set(ob, key, PROP_UINT32, i));
}

struct prop *
prop_set_sint32(void *ob, char *key, Sint32 i)
{
	return (prop_set(ob, key, PROP_SINT32, i));
}

#ifdef SDL_HAS_64BIT_TYPE
struct prop *
prop_set_uint64(void *ob, char *key, Uint64 i)
{
	return (prop_set(ob, key, PROP_UINT64, i));
}

struct prop *
prop_set_sint64(void *ob, char *key, Sint64 i)
{
	return (prop_set(ob, key, PROP_SINT64, i));
	fatal("SDL does not provide a 64-bit type\n");
	return (NULL);
}
#endif	/* SDL_HAS_64BIT_TYPE */

struct prop *
prop_set_float(void *ob, char *key, float f)
{
	return (prop_set(ob, key, PROP_FLOAT, f));
}

struct prop *
prop_set_double(void *ob, char *key, double d)
{
	return (prop_set(ob, key, PROP_DOUBLE, d));
}

#ifdef HAVE_LONG_DOUBLE
struct prop *
prop_set_long_double(void *ob, char *key, long double ld)
{
	return (prop_set(ob, key, PROP_LONG_DOUBLE, ld));
}
#endif

struct prop *
prop_set_string(void *ob, char *key, char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	return (prop_set(ob, key, PROP_STRING, s));
}

struct prop *
prop_set_pointer(void *ob, char *key, void *p)
{
	return (prop_set(ob, key, PROP_POINTER, p));
}

struct prop *
prop_set_bool(void *ob, char *key, int i)
{
	return (prop_set(ob, key, PROP_BOOL, i));
}

/*
 * Obtain the value of a property.
 * XXX use a hash table
 */
struct prop *
prop_get(void *obp, char *key, enum prop_type t, void *p)
{
	struct object *ob = obp;
	struct prop *prop;

	pthread_mutex_lock(&ob->props_lock);
	TAILQ_FOREACH(prop, &ob->props, props) {
		if (strcmp(key, prop->key) != 0) {
			continue;
		}
		if (t != PROP_ANY && t != prop->type) {
			continue;
		}
		if (p != NULL) {
			switch (prop->type) {
			case PROP_INT:
			case PROP_BOOL:
				*(int *)p = prop->data.i;
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
#ifdef SDL_HAS_64BIT_TYPE
			case PROP_UINT64:
				*(Uint64 *)p = prop->data.u64;
				break;
			case PROP_SINT64:
				*(Sint64 *)p = prop->data.s64;
				break;
#endif
			case PROP_FLOAT:
				*(float *)p = prop->data.f;
				break;
			case PROP_DOUBLE:
				*(double *)p = prop->data.d;
				break;
#ifdef HAVE_LONG_DOUBLE
			case PROP_LONG_DOUBLE:
				*(long double *)p = prop->data.ld;
				break;
#endif
			case PROP_STRING:
				/* Strdup'd for thread safety. */
				*(char **)p = Strdup(prop->data.s);
				break;
			case PROP_POINTER:
				*(void **)p = prop->data.p;
				break;
			default:
				error_set("unsupported property type: %d\n", t);
				goto fail;
			}
		}
		pthread_mutex_unlock(&ob->props_lock);
		return (prop);
	}

	error_set("%s has no `%s' property (type %d)\n", ob->name, key, t);
fail:
	pthread_mutex_unlock(&ob->props_lock);
	return (NULL);
}

int
prop_get_int(void *p, char *key)
{
	int i;

	if (prop_get(p, key, PROP_INT, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}

int
prop_get_bool(void *p, char *key)
{
	int i;

	if (prop_get(p, key, PROP_BOOL, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}

Uint8
prop_get_uint8(void *p, char *key)
{
	Uint8 i;

	if (prop_get(p, key, PROP_UINT8, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}

Sint8
prop_get_sint8(void *p, char *key)
{
	Sint8 i;

	if (prop_get(p, key, PROP_SINT8, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}

Uint16
prop_get_uint16(void *p, char *key)
{
	Uint16 i;

	if (prop_get(p, key, PROP_UINT16, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}

Sint16
prop_get_sint16(void *p, char *key)
{
	Sint16 i;

	if (prop_get(p, key, PROP_SINT16, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}

Uint32
prop_get_uint32(void *p, char *key)
{
	Uint32 i;

	if (prop_get(p, key, PROP_UINT32, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}

Sint32
prop_get_sint32(void *p, char *key)
{
	Sint32 i;

	if (prop_get(p, key, PROP_SINT32, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}

#ifdef SDL_HAS_64BIT_TYPE
Uint64
prop_get_uint64(void *p, char *key)
{
	Uint64 i;

	if (prop_get(p, key, PROP_UINT64, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}

Sint64
prop_get_sint64(void *p, char *key)
{
	Sint64 i;

	if (prop_get(p, key, PROP_SINT64, &i) == NULL) {
		fatal("%s\n", error_get());
	}
	return (i);
}
#endif /* SDL_HAS_64BIT_TYPE */

float
prop_get_float(void *p, char *key)
{
	float f;

	if (prop_get(p, key, PROP_FLOAT, &f) == NULL) {
		fatal("%s\n", error_get());
	}
	return (f);
}

double
prop_get_double(void *p, char *key)
{
	double d;

	if (prop_get(p, key, PROP_DOUBLE, &d) == NULL) {
		fatal("%s\n", error_get());
	}
	return (d);
}

#ifdef HAVE_LONG_DOUBLE
long double
prop_get_long_double(void *p, char *key)
{
	long double ld;

	if (prop_get(p, key, PROP_LONG_DOUBLE, &ld) == NULL) {
		fatal("%s\n", error_get());
	}
	return (ld);
}
#endif

char *
prop_get_string(void *p, char *key)
{
	char *s;

	if (prop_get(p, key, PROP_STRING, &s) == NULL) {
		fatal("%s\n", error_get());
	}
	return (s);
}

void *
prop_get_pointer(void *p, char *key)
{
	void *np;

	if (prop_get(p, key, PROP_POINTER, &np) == NULL) {
		fatal("%s\n", error_get());
	}
	return (np);
}

int
prop_load(void *p, int fd)
{
	struct object *ob = p;
	struct prop *prop;
	Uint32 nprops, i, t;
	char *key;
	Uint8 c;
	Sint8 sc;

	if (version_read(fd, &prop_ver, NULL) == -1) {
		return (-1);
	}

	pthread_mutex_lock(&ob->props_lock);
	nprops = read_uint32(fd);

	for (i = 0; i < nprops; i++) {
		key = read_string(fd, NULL);
		t = read_uint32(fd);
		
		debug(DEBUG_STATE, "prop %s (%d)\n", key, t);
	
		switch (t) {
		case PROP_BOOL:
			Read(fd, &c, 1);
			prop_set_bool(ob, key, (int)c);
			break;
		case PROP_UINT8:
			Read(fd, &c, 1);
			prop_set_uint8(ob, key, c);
			break;
		case PROP_SINT8:
			Read(fd, &sc, 1);
			prop_set_sint8(ob, key, sc);
			break;
		case PROP_UINT16:
			prop_set_uint16(ob, key, read_uint16(fd));
			break;
		case PROP_SINT16:
			prop_set_sint16(ob, key, read_sint16(fd));
			break;
		case PROP_UINT32:
			prop_set_uint32(ob, key, read_uint32(fd));
			break;
		case PROP_SINT32:
			prop_set_sint32(ob, key, read_sint32(fd));
			break;
		case PROP_INT:
			prop_set_int(ob, key, (int)read_sint32(fd));
			break;
#ifdef SDL_HAS_64BIT_TYPE
		case PROP_UINT64:
			prop_set_uint64(ob, key, read_uint64(fd));
			break;
		case PROP_SINT64:
			prop_set_sint64(ob, key, read_sint64(fd));
			break;
#endif
		/* XXX vax floating point */
#ifdef HAVE_IEEE754
		case PROP_FLOAT:
			prop_set_float(ob, key, read_float(fd));
			break;
		case PROP_DOUBLE:
			prop_set_double(ob, key, read_double(fd));
			break;
# ifdef HAVE_LONG_DOUBLE
		case PROP_LONG_DOUBLE:
			prop_set_long_double(ob, key, read_long_double(fd));
			break;
# endif
#endif
		case PROP_STRING:
			{
				char *sd;

				sd = read_string(fd, NULL);
				prop_set_string(ob, key, "%s", sd);
				free(sd);
			}
			break;
		default:
			fatal("cannot load property of type %d\n", t);
			break;
		}
		free(key);
	}
	pthread_mutex_unlock(&ob->props_lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->props_lock);
	return (-1);
}

int
prop_save(void *p, int fd)
{
	struct object *ob = p;
	off_t count_offs;
	struct fobj_buf *buf;
	Uint32 nprops = 0;
	struct prop *prop;
	Uint8 c;
	
	pthread_mutex_lock(&ob->props_lock);

	buf = fobj_create_buf(64, 128);

	count_offs = buf->offs;				/* Skip count */
	buf_write_uint32(buf, 0);

	TAILQ_FOREACH(prop, &ob->props, props) {
		buf_write_string(buf, (char *)prop->key);
		buf_write_uint32(buf, prop->type);
		debug(DEBUG_STATE, "%s -> %s\n", ob->name, prop->key);
		switch (prop->type) {
		case PROP_BOOL:
			c = (prop->data.i == 1) ? 1 : 0;
			buf_write(buf, &c, 1);
			break;
		case PROP_UINT8:
			buf_write(buf, &prop->data.u8, 1);
			break;
		case PROP_SINT8:
			buf_write(buf, &prop->data.s8, 1);
			break;
		case PROP_UINT16:
			buf_write_uint16(buf, prop->data.u16);
			break;
		case PROP_SINT16:
			buf_write_sint16(buf, prop->data.s16);
			break;
		case PROP_UINT32:
			buf_write_uint32(buf, prop->data.u32);
			break;
		case PROP_SINT32:
			buf_write_sint32(buf, prop->data.s32);
			break;
		case PROP_INT:
			buf_write_sint32(buf, (Sint32)prop->data.i);
			break;
#ifdef SDL_HAS_64BIT_TYPE
		case PROP_UINT64:
			buf_write_uint64(buf, prop->data.u64);
			break;
		case PROP_SINT64:
			buf_write_sint64(buf, prop->data.s64);
			break;
#endif
		/* XXX vax floating point */
#ifdef HAVE_IEEE754
		case PROP_FLOAT:
			buf_write_float(buf, prop->data.f);
			break;
		case PROP_DOUBLE:
			buf_write_double(buf, prop->data.d);
			break;
# ifdef HAVE_LONG_DOUBLE
		case PROP_LONG_DOUBLE:
			buf_write_long_double(buf, prop->data.ld);
			break;
# endif
#endif
		case PROP_STRING:
			buf_write_string(buf, prop->data.s);
			break;
		case PROP_POINTER:
			debug(DEBUG_STATE,
			    "skipped machine-dependent property `%s'\n",
			    prop->key);
			break;
		default:
			fatal("unknown property type: %d\n", prop->type);
		}
		nprops++;
	}
	pthread_mutex_unlock(&ob->props_lock);
	
	buf_pwrite_uint32(buf, nprops, count_offs);	/* Write count */

	version_write(fd, &prop_ver);
	fobj_flush_buf(buf, fd);

	fobj_destroy_buf(buf);
	return (0);
}

void
prop_destroy(struct prop *prop)
{
	switch (prop->type) {
	case PROP_STRING:
		Free(prop->data.s);
		break;
	default:
		break;
	}
	free(prop->key);
	free(prop);
}
