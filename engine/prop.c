/*	$Csoft: prop.c,v 1.53 2005/03/11 08:59:30 vedge Exp $	*/

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

const AG_Version prop_ver = {
	"agar property map",
	2, 0
};

#ifdef DEBUG
#define DEBUG_STATE	0x01
#define DEBUG_SET	0x02

int	prop_debug = 0;
#define agDebugLvl prop_debug
#endif

/* Return the copy of a property. */
AG_Prop *
AG_CopyProp(const AG_Prop *prop)
{
	AG_Prop *nprop;

	nprop = Malloc(sizeof(AG_Prop), M_PROP);
	memcpy(nprop, prop, sizeof(AG_Prop));

	switch (prop->type) {
	case AG_PROP_STRING:
		nprop->data.s = strdup(prop->data.s);
		if (nprop->data.s == NULL) {
			Free(nprop, M_PROP);
			AG_SetError(_("Out of memory for string."));
			return (NULL);
		}
		break;
	default:
		break;
	}
	return (0);
}

/* Modify a property, or create a new one if it does not exist. */
AG_Prop *
AG_SetProp(void *p, const char *key, enum ag_prop_type type, ...)
{
	AG_Object *ob = p;
	AG_Prop *nprop = NULL, *oprop;
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
		nprop = Malloc(sizeof(AG_Prop), M_PROP);
		strlcpy(nprop->key, key, sizeof(nprop->key));
		nprop->type = type;
	} else {
		modify++;
	}

	va_start(ap, type);
	switch (type) {
	case AG_PROP_UINT:
		nprop->data.u = va_arg(ap, unsigned int);
		debug(DEBUG_SET, "uint %s: %d\n", key, nprop->data.u);
		break;
	case AG_PROP_INT:
		nprop->data.i = va_arg(ap, int);
		debug(DEBUG_SET, "int %s: %d\n", key, nprop->data.i);
		break;
	case AG_PROP_BOOL:
		nprop->data.i = va_arg(ap, int);		/* Promoted */
		debug(DEBUG_SET, "bool %s: %d\n", key, nprop->data.i);
		break;
	case AG_PROP_UINT8:
		nprop->data.u8 = (Uint8)va_arg(ap, int);	/* Promoted */
		debug(DEBUG_SET, "u8 %s: %d\n", key, nprop->data.u8);
		break;
	case AG_PROP_SINT8:
		nprop->data.s8 = (Sint8)va_arg(ap, int);	/* Promoted */
		debug(DEBUG_SET, "s8 %s: %d\n", key, nprop->data.s8);
		break;
	case AG_PROP_UINT16:
		nprop->data.u16 = (Uint16)va_arg(ap, int);	/* Promoted */
		debug(DEBUG_SET, "u16 %s: %d\n", key, nprop->data.u16);
		break;
	case AG_PROP_SINT16:
		nprop->data.s16 = (Sint16)va_arg(ap, int);	/* Promoted */
		debug(DEBUG_SET, "s16 %s: %d\n", key, nprop->data.s16);
		break;
	case AG_PROP_UINT32:
		nprop->data.u32 = va_arg(ap, Uint32);
		debug(DEBUG_SET, "u32 %s: %d\n", key, nprop->data.u32);
		break;
	case AG_PROP_SINT32:
		nprop->data.s32 = va_arg(ap, Sint32);
		debug(DEBUG_SET, "s32 %s: %d\n", key, nprop->data.s32);
		break;
#ifdef FLOATING_POINT
	case AG_PROP_FLOAT:
		nprop->data.f = (float)va_arg(ap, double);	/* Promoted */
		debug(DEBUG_SET, "float %s: %f\n", key, nprop->data.f);
		break;
	case AG_PROP_DOUBLE:
		nprop->data.d = va_arg(ap, double);
		debug(DEBUG_SET, "double %s: %f\n", key, nprop->data.d);
		break;
#endif
	case AG_PROP_STRING:
		nprop->data.s = va_arg(ap, char *);
		debug(DEBUG_SET, "string %s: %s\n", key, nprop->data.s);
		break;
	case AG_PROP_POINTER:
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
		AG_PostEvent(NULL, ob, "prop-modified", "%p", nprop);
	}
	pthread_mutex_unlock(&ob->lock);
	return (nprop);
}

AG_Prop *
AG_SetUint(void *ob, const char *key, u_int i)
{
	return (AG_SetProp(ob, key, AG_PROP_UINT, i));
}

AG_Prop *
AG_SetInt(void *ob, const char *key, int i)
{
	return (AG_SetProp(ob, key, AG_PROP_INT, i));
}

AG_Prop *
AG_SetUint8(void *ob, const char *key, Uint8 i)
{
	return (AG_SetProp(ob, key, AG_PROP_UINT8, i));
}

AG_Prop *
AG_SetSint8(void *ob, const char *key, Sint8 i)
{
	return (AG_SetProp(ob, key, AG_PROP_SINT8, i));
}

AG_Prop *
AG_SetUint16(void *ob, const char *key, Uint16 i)
{
	return (AG_SetProp(ob, key, AG_PROP_UINT16, i));
}

AG_Prop *
AG_SetSint16(void *ob, const char *key, Sint16 i)
{
	return (AG_SetProp(ob, key, AG_PROP_SINT16, i));
}

AG_Prop *
AG_SetUint32(void *ob, const char *key, Uint32 i)
{
	return (AG_SetProp(ob, key, AG_PROP_UINT32, i));
}

AG_Prop *
AG_SetSint32(void *ob, const char *key, Sint32 i)
{
	return (AG_SetProp(ob, key, AG_PROP_SINT32, i));
}

#ifdef FLOATING_POINT
AG_Prop *
AG_SetFloat(void *ob, const char *key, float f)
{
	return (AG_SetProp(ob, key, AG_PROP_FLOAT, f));
}

AG_Prop *
AG_SetDouble(void *ob, const char *key, double d)
{
	return (AG_SetProp(ob, key, AG_PROP_DOUBLE, d));
}
#endif

AG_Prop *
AG_SetString(void *ob, const char *key, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	return (AG_SetProp(ob, key, AG_PROP_STRING, s));
}

AG_Prop *
AG_SetPointer(void *ob, const char *key, void *p)
{
	return (AG_SetProp(ob, key, AG_PROP_POINTER, p));
}

AG_Prop *
AG_SetBool(void *ob, const char *key, int i)
{
	return (AG_SetProp(ob, key, AG_PROP_BOOL, i));
}

void
AG_LockProps(void *p)
{
	pthread_mutex_lock(&AGOBJECT(p)->lock);
}

void
AG_UnlockProps(void *p)
{
	pthread_mutex_unlock(&AGOBJECT(p)->lock);
}

/* Return a pointer to the data of a property. */
AG_Prop *
AG_GetProp(void *obp, const char *key, enum ag_prop_type t, void *p)
{
	AG_Object *ob = obp;
	AG_Prop *prop;

	pthread_mutex_lock(&ob->lock);
	TAILQ_FOREACH(prop, &ob->props, props) {
		if (strcmp(key, prop->key) != 0)
			continue;
		if (t != AG_PROP_ANY && t != prop->type)
			continue;

		if (p != NULL) {
			switch (prop->type) {
			case AG_PROP_INT:
			case AG_PROP_BOOL:
				*(int *)p = prop->data.i;
				break;
			case AG_PROP_UINT:
				*(u_int *)p = prop->data.u;
				break;
			case AG_PROP_UINT8:
				*(Uint8 *)p = prop->data.u8;
				break;
			case AG_PROP_SINT8:
				*(Sint8 *)p = prop->data.s8;
				break;
			case AG_PROP_UINT16:
				*(Uint16 *)p = prop->data.u16;
				break;
			case AG_PROP_SINT16:
				*(Sint16 *)p = prop->data.s16;
				break;
			case AG_PROP_UINT32:
				*(Uint32 *)p = prop->data.u32;
				break;
			case AG_PROP_SINT32:
				*(Sint32 *)p = prop->data.s32;
				break;
#ifdef FLOATING_POINT
			case AG_PROP_FLOAT:
				*(float *)p = prop->data.f;
				break;
			case AG_PROP_DOUBLE:
				*(double *)p = prop->data.d;
				break;
#endif
			case AG_PROP_STRING:
				*(char **)p = prop->data.s;
				break;
			case AG_PROP_POINTER:
				*(void **)p = prop->data.p;
				break;
			default:
				AG_SetError(_("Unsupported property type: %d."),
				    t);
				goto fail;
			}
		}
		pthread_mutex_unlock(&ob->lock);
		return (prop);
	}

	AG_SetError(_("%s has no `%s' property (type %d)."), ob->name, key, t);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (NULL);
}

u_int
AG_Uint(void *p, const char *key)
{
	u_int i;

	if (AG_GetProp(p, key, AG_PROP_UINT, &i) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (i);
}

int
AG_Int(void *p, const char *key)
{
	int i;

	if (AG_GetProp(p, key, AG_PROP_INT, &i) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (i);
}

int
AG_Bool(void *p, const char *key)
{
	int i;

	if (AG_GetProp(p, key, AG_PROP_BOOL, &i) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (i);
}

Uint8
AG_Uint8(void *p, const char *key)
{
	Uint8 i;

	if (AG_GetProp(p, key, AG_PROP_UINT8, &i) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (i);
}

Sint8
AG_Sint8(void *p, const char *key)
{
	Sint8 i;

	if (AG_GetProp(p, key, AG_PROP_SINT8, &i) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (i);
}

Uint16
AG_Uint16(void *p, const char *key)
{
	Uint16 i;

	if (AG_GetProp(p, key, AG_PROP_UINT16, &i) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (i);
}

Sint16
AG_Sint16(void *p, const char *key)
{
	Sint16 i;

	if (AG_GetProp(p, key, AG_PROP_SINT16, &i) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (i);
}

Uint32
AG_Uint32(void *p, const char *key)
{
	Uint32 i;

	if (AG_GetProp(p, key, AG_PROP_UINT32, &i) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (i);
}

Sint32
AG_Sint32(void *p, const char *key)
{
	Sint32 i;

	if (AG_GetProp(p, key, AG_PROP_SINT32, &i) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (i);
}

#ifdef FLOATING_POINT
float
AG_Float(void *p, const char *key)
{
	float f;

	if (AG_GetProp(p, key, AG_PROP_FLOAT, &f) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (f);
}

double
AG_Double(void *p, const char *key)
{
	double d;

	if (AG_GetProp(p, key, AG_PROP_DOUBLE, &d) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (d);
}
#endif /* FLOATING_POINT */

/* The object must be locked. */
char *
AG_String(void *p, const char *key)
{
	char *s;

	if (AG_GetProp(p, key, AG_PROP_STRING, &s) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (s);
}

size_t
AG_StringCopy(void *p, const char *key, char *buf, size_t bufsize)
{
	size_t sl;
	char *s;

	AG_LockProps(p);
	if (AG_GetProp(p, key, AG_PROP_STRING, &s) == NULL) {
		fatal("%s", AG_GetError());
	}
	sl = strlcpy(buf, s, bufsize);
	AG_UnlockProps(p);
	return (sl);
}

void *
AG_Pointer(void *p, const char *key)
{
	void *np;

	if (AG_GetProp(p, key, AG_PROP_POINTER, &np) == NULL) {
		fatal("%s", AG_GetError());
	}
	return (np);
}

int
AG_PropLoad(void *p, AG_Netbuf *buf)
{
	AG_Object *ob = p;
	Uint32 i, nprops;

	if (AG_ReadVersion(buf, &prop_ver, NULL) == -1)
		return (-1);

	pthread_mutex_lock(&ob->lock);

	if ((ob->flags & AG_OBJECT_RELOAD_PROPS) == 0)
		AG_ObjectFreeProps(ob);

	nprops = AG_ReadUint32(buf);
	for (i = 0; i < nprops; i++) {
		char key[AG_PROP_KEY_MAX];
		Uint32 t;

		if (AG_CopyString(key, buf, sizeof(key)) >= sizeof(key)) {
			AG_SetError(_("The prop key too big."));
			goto fail;
		}
		t = AG_ReadUint32(buf);
		
		debug(DEBUG_STATE, "prop %s (%d)\n", key, t);
	
		switch (t) {
		case AG_PROP_BOOL:
			AG_SetBool(ob, key, (int)AG_ReadUint8(buf));
			break;
		case AG_PROP_UINT8:
			AG_SetBool(ob, key, AG_ReadUint8(buf));
			break;
		case AG_PROP_SINT8:
			AG_SetBool(ob, key, AG_ReadSint8(buf));
			break;
		case AG_PROP_UINT16:
			AG_SetUint16(ob, key, AG_ReadUint16(buf));
			break;
		case AG_PROP_SINT16:
			AG_SetSint16(ob, key, AG_ReadSint16(buf));
			break;
		case AG_PROP_UINT32:
			AG_SetUint32(ob, key, AG_ReadUint32(buf));
			break;
		case AG_PROP_SINT32:
			AG_SetSint32(ob, key, AG_ReadSint32(buf));
			break;
		case AG_PROP_UINT:
			AG_SetUint(ob, key, (u_int)AG_ReadUint32(buf));
			break;
		case AG_PROP_INT:
			AG_SetInt(ob, key, (int)AG_ReadSint32(buf));
			break;
#if defined(FLOATING_POINT) && defined(HAVE_IEEE754)
		case AG_PROP_FLOAT:
			AG_SetFloat(ob, key, AG_ReadFloat(buf));
			break;
		case AG_PROP_DOUBLE:
			AG_SetDouble(ob, key, AG_ReadDouble(buf));
			break;
#endif
		case AG_PROP_STRING:
			{
				char *s;

				s = AG_ReadString(buf);
				if (strlen(s) >= AG_PROP_STRING_MAX) {
					AG_SetError(_("String too big."));
					Free(s, 0);
					goto fail;
				}
				AG_SetString(ob, key, "%s", s);
				Free(s, 0);
			}
			break;
		default:
			AG_SetError(_("Cannot load prop of type 0x%x."), t);
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
AG_PropSave(void *p, AG_Netbuf *buf)
{
	AG_Object *ob = p;
	off_t count_offs;
	Uint32 nprops = 0;
	AG_Prop *prop;
	Uint8 c;
	
	AG_WriteVersion(buf, &prop_ver);
	
	pthread_mutex_lock(&ob->lock);

	count_offs = AG_NetbufTell(buf);			/* Skip count */
	AG_WriteUint32(buf, 0);

	TAILQ_FOREACH(prop, &ob->props, props) {
		AG_WriteString(buf, (char *)prop->key);
		AG_WriteUint32(buf, prop->type);
		debug(DEBUG_STATE, "%s -> %s\n", ob->name, prop->key);
		switch (prop->type) {
		case AG_PROP_BOOL:
			c = (prop->data.i == 1) ? 1 : 0;
			AG_WriteUint8(buf, c);
			break;
		case AG_PROP_UINT8:
			AG_WriteUint8(buf, prop->data.u8);
			break;
		case AG_PROP_SINT8:
			AG_WriteSint8(buf, prop->data.s8);
			break;
		case AG_PROP_UINT16:
			AG_WriteUint16(buf, prop->data.u16);
			break;
		case AG_PROP_SINT16:
			AG_WriteSint16(buf, prop->data.s16);
			break;
		case AG_PROP_UINT32:
			AG_WriteUint32(buf, prop->data.u32);
			break;
		case AG_PROP_SINT32:
			AG_WriteSint32(buf, prop->data.s32);
			break;
		case AG_PROP_UINT:
			AG_WriteUint32(buf, (Uint32)prop->data.u);
			break;
		case AG_PROP_INT:
			AG_WriteSint32(buf, (Sint32)prop->data.i);
			break;
#if defined(FLOATING_POINT) && defined(HAVE_IEEE754)
		case AG_PROP_FLOAT:
			AG_WriteFloat(buf, prop->data.f);
			break;
		case AG_PROP_DOUBLE:
			AG_WriteDouble(buf, prop->data.d);
			break;
#endif
		case AG_PROP_STRING:
			AG_WriteString(buf, prop->data.s);
			break;
		case AG_PROP_POINTER:
			debug(DEBUG_STATE,
			    "skipped machine-dependent property `%s'\n",
			    prop->key);
			break;
		default:
			AG_SetError(_("Cannot save prop of type 0x%x."),
			    prop->type);
			goto fail;
		}
		nprops++;
	}
	pthread_mutex_unlock(&ob->lock);
	AG_PwriteUint32(buf, nprops, count_offs);	/* Write count */
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	return (-1);
}

void
AG_PropDestroy(AG_Prop *prop)
{
	switch (prop->type) {
	case AG_PROP_STRING:
		Free(prop->data.s, 0);
		break;
	}
}

void
AG_PropPrint(char *s, size_t len, AG_Prop *prop)
{
	switch (prop->type) {
	case AG_PROP_UINT:
		snprintf(s, len, "%u", prop->data.u);
		break;
	case AG_PROP_INT:
		snprintf(s, len, "%d", prop->data.i);
		break;
	case AG_PROP_UINT8:
		snprintf(s, len, "%u", prop->data.u8);
		break;
	case AG_PROP_SINT8:
		snprintf(s, len, "%d", prop->data.s8);
		break;
	case AG_PROP_UINT16:
		snprintf(s, len, "%u", prop->data.u16);
		break;
	case AG_PROP_SINT16:
		snprintf(s, len, "%d", prop->data.s16);
		break;
	case AG_PROP_UINT32:
		snprintf(s, len, "%u", prop->data.u32);
		break;
	case AG_PROP_SINT32:
		snprintf(s, len, "%d", prop->data.s32);
		break;
	case AG_PROP_FLOAT:
		snprintf(s, len, "%f", prop->data.f);
		break;
	case AG_PROP_DOUBLE:
		snprintf(s, len, "%f", prop->data.d);
		break;
	case AG_PROP_STRING:
		strlcpy(s, prop->data.s, len);
		break;
	case AG_PROP_POINTER:
		snprintf(s, len, "%p", prop->data.p);
		break;
	case AG_PROP_BOOL:
		strlcpy(s, prop->data.i ? "true" : "false", len);
		break;
	default:
		snprintf(s, len, "?");
		break;
	}
}

