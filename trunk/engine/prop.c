/*	$Csoft: prop.c,v 1.4 2002/09/19 22:07:53 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, Inc, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <libfobj/fobj.h>
#include <libfobj/buf.h>

#include "compat/vasprintf.h"
#include "engine.h"
#include "version.h"

static const struct version prop_ver = {
	"agar property map",
	1, 0
};

/*
 * Modify a property, or create a new one if it does not exist.
 * The property list must not be locked by the caller thread.
 */
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
		nprop->key = key;
		nprop->type = type;
	} else {
		modify++;
	}

	va_start(ap, type);
	switch (type) {
	case PROP_INT:
		nprop->data.i = va_arg(ap, int);
		dprintf("bool %s: %d\n", nprop->key, nprop->data.i);
		break;
	case PROP_BOOL:
		nprop->data.i = va_arg(ap, int);
		dprintf("int %s: %d\n", nprop->key, nprop->data.i);
		break;
	case PROP_UINT8:
		nprop->data.u8 = va_arg(ap, Uint8);
		dprintf("uint8 %s: %d\n", nprop->key, nprop->data.u8);
		break;
	case PROP_SINT8:
		nprop->data.s8 = va_arg(ap, Sint8);
		dprintf("sint8 %s: %d\n", nprop->key, nprop->data.s8);
		break;
	case PROP_UINT16:
		nprop->data.u16 = va_arg(ap, Uint16);
		dprintf("uint16 %s: %d\n", nprop->key, nprop->data.u16);
		break;
	case PROP_SINT16:
		nprop->data.s16 = va_arg(ap, Sint16);
		dprintf("sint16 %s: %d\n", nprop->key, nprop->data.s16);
		break;
	case PROP_UINT32:
		nprop->data.u32 = va_arg(ap, Uint32);
		dprintf("uint32 %s: %d\n", nprop->key, nprop->data.u32);
		break;
	case PROP_SINT32:
		nprop->data.s32 = va_arg(ap, Sint32);
		dprintf("sint32 %s: %d\n", nprop->key, nprop->data.s32);
		break;
	case PROP_UINT64:
		nprop->data.u64 = va_arg(ap, Uint64);
		dprintf("uint64 %s: %ld\n", nprop->key, (long)nprop->data.u64);
		break;
	case PROP_SINT64:
		nprop->data.s64 = va_arg(ap, Sint64);
		dprintf("sint64 %s: %ld\n", nprop->key, (long)nprop->data.s64);
		break;
	case PROP_STRING:
		nprop->data.s = va_arg(ap, char *);
		dprintf("string %s: %s\n", nprop->key, nprop->data.s);
		break;
	case PROP_POINTER:
		nprop->data.p = va_arg(ap, void *);
		dprintf("pointer %s: %p\n", nprop->key, nprop->data.p);
		break;
	}
	va_end(ap);

	if (!modify) {
		pthread_mutex_lock(&ob->props_lock);
		TAILQ_INSERT_HEAD(&ob->props, nprop, props);
		pthread_mutex_unlock(&ob->props_lock);
	}
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

struct prop *
prop_set_uint64(void *ob, char *key, Uint64 i)
{
	return (prop_set(ob, key, PROP_UINT64, i));
}

struct prop *
prop_set_sint64(void *ob, char *key, Sint64 i)
{
	return (prop_set(ob, key, PROP_SINT64, i));
}

struct prop *
prop_set_string(void *ob, char *key, char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	if (vasprintf(&s, fmt, ap) == -1) {
		fatal("vasprintf: %s\n", strerror(errno));
	}
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

void
prop_get(void *obp, char *key, enum prop_type t, void *p)
{
	struct object *ob = obp;
	struct prop *prop;

	pthread_mutex_lock(&ob->props_lock);
	TAILQ_FOREACH(prop, &ob->props, props) {
		if (prop->type == t && strcmp(key, prop->key) == 0) {
			switch (t) {
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
			case PROP_UINT64:
				*(Uint64 *)p = prop->data.u64;
				break;
			case PROP_SINT64:
				*(Sint64 *)p = prop->data.s64;
				break;
			case PROP_STRING:
				*(char **)p = strdup(prop->data.s);
				break;
			case PROP_POINTER:
				*(void **)p = prop->data.p;
				break;
			}
			pthread_mutex_unlock(&ob->props_lock);
			return;
		}
	}
	fatal("%s has no \"%s\" property (type 0x%x)\n", ob->name, key, t);
}

int
prop_int(void *p, char *key)
{
	struct object *ob = p;
	int i;

	prop_get(ob, key, PROP_INT, &i);
	return (i);
}

Uint8
prop_uint8(void *p, char *key)
{
	struct object *ob = p;
	Uint8 i;

	prop_get(ob, key, PROP_UINT8, &i);
	return (i);
}

Sint8
prop_sint8(void *p, char *key)
{
	struct object *ob = p;
	Sint8 i;

	prop_get(ob, key, PROP_SINT8, &i);
	return (i);
}

Uint16
prop_uint16(void *p, char *key)
{
	struct object *ob = p;
	Uint16 i;

	prop_get(ob, key, PROP_UINT16, &i);
	return (i);
}

Sint16
prop_sint16(void *p, char *key)
{
	struct object *ob = p;
	Sint16 i;

	prop_get(ob, key, PROP_SINT16, &i);
	return (i);
}

Uint32
prop_uint32(void *p, char *key)
{
	struct object *ob = p;
	Uint32 i;

	prop_get(ob, key, PROP_UINT32, &i);
	return (i);
}

Sint32
prop_sint32(void *p, char *key)
{
	struct object *ob = p;
	Sint32 i;

	prop_get(ob, key, PROP_SINT32, &i);
	return (i);
}

Uint64
prop_uint64(void *p, char *key)
{
	struct object *ob = p;
	Uint64 i;

	prop_get(ob, key, PROP_UINT64, &i);
	return (i);
}

Sint64
prop_sint64(void *p, char *key)
{
	struct object *ob = p;
	Sint64 i;

	prop_get(ob, key, PROP_SINT64, &i);
	return (i);
}

char *
prop_string(void *p, char *key)
{
	struct object *ob = p;
	char *s;

	prop_get(ob, key, PROP_STRING, &s);
	return (s);
}

void *
prop_pointer(void *p, char *key)
{
	struct object *ob = p;
	void *np;

	prop_get(ob, key, PROP_POINTER, &np);
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

	if (version_read(fd, &prop_ver) == -1) {
		return (-1);
	}

	pthread_mutex_lock(&ob->props_lock);
	nprops = read_uint32(fd);

	for (i = 0; i < nprops; i++) {
		key = read_string(fd);
		t = read_uint32(fd);
		
		dprintf("prop %s (%d)\n", key, t);
	
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
		case PROP_UINT64:
			prop_set_uint64(ob, key, read_uint64(fd));
			break;
		case PROP_SINT64:
			prop_set_sint64(ob, key, read_sint64(fd));
			break;
		case PROP_STRING:
			prop_set_string(ob, key, "%s", read_string(fd));
			break;
		default:
			fatal("unknown property type: 0x%x\n", t);
		}
	}
	pthread_mutex_unlock(&ob->props_lock);

	return (0);
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

	count_offs = buf->offs;		/* Skip */
	buf_write_uint32(buf, 0);

	TAILQ_FOREACH(prop, &ob->props, props) {
		buf_write_string(buf, (char *)prop->key);
		buf_write_uint32(buf, prop->type);
		dprintf("%s -> %s\n", ob->name, prop->key);
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
		case PROP_UINT64:
			buf_write_uint64(buf, prop->data.u64);
			break;
		case PROP_SINT64:
			buf_write_sint64(buf, prop->data.s64);
			break;
		case PROP_STRING:
			buf_write_string(buf, prop->data.s);
			break;
		case PROP_INT:
		case PROP_POINTER:
			dprintf("ignored property \"%s\"\n", prop->key);
			break;
		}
		nprops++;
	}
	pthread_mutex_unlock(&ob->props_lock);
	
	buf_pwrite_uint32(buf, nprops, count_offs);

	version_write(fd, &prop_ver);
	fobj_flush_buf(buf, fd);
	fobj_destroy_buf(buf);
	return (0);
}

