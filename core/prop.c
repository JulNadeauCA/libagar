/*
 * Copyright (c) 2002-2009 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * LEGACY Interface to AG_Variable(3).
 */

#include <core/core.h>

#include <fcntl.h>
#include <stdarg.h>
#include <string.h>

const AG_Version agPropTblVer = { 2, 0 };

/* LEGACY Encoding of prop types */
enum ag_prop_legacy_type {
	AG_LEGACY_PROP_UINT		= 0,
	AG_LEGACY_PROP_INT		= 1,
	AG_LEGACY_PROP_UINT8		= 2,
	AG_LEGACY_PROP_SINT8		= 3,
	AG_LEGACY_PROP_UINT16		= 4,
	AG_LEGACY_PROP_SINT16		= 5,
	AG_LEGACY_PROP_UINT32		= 6,
	AG_LEGACY_PROP_SINT32		= 7,
	AG_LEGACY_PROP_UINT64		= 8,	/* MD */
	AG_LEGACY_PROP_SINT64		= 9,	/* MD */
	AG_LEGACY_PROP_FLOAT		= 10,
	AG_LEGACY_PROP_DOUBLE		= 11,
	AG_LEGACY_PROP_LONG_DOUBLE	= 12,	/* MD */
	AG_LEGACY_PROP_STRING		= 13,
	AG_LEGACY_PROP_POINTER		= 14,
	AG_LEGACY_PROP_BOOL		= 15,
	AG_LEGACY_PROP_PRIVATE		= 10001
};

/*
 * Test for the existence of a given property.
 * LEGACY Interface to AG_Variable(3).
 */
int
AG_PropDefined(void *p, const char *key)
{
	AG_Object *obj = p;
	Uint i;

	AG_ObjectLock(obj);
	for (i = 0; i < obj->nVars; i++) {
		if (strcmp(key, obj->vars[i].name) == 0) {
			AG_ObjectUnlock(obj);
			return (1);
		}
	}
	AG_ObjectUnlock(obj);
	return (0);
}

/*
 * Create a new property, or modify an existing one.
 * LEGACY Interface to AG_Variable(3).
 */
AG_Prop *
AG_SetProp(void *pObj, const char *key, enum ag_prop_type type, ...)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	va_list ap;

	AG_ObjectLock(obj);
	va_start(ap, type);
	switch (type) {
	case AG_PROP_INT:	AG_SetInt(obj, key, va_arg(ap,int));		break;
	case AG_PROP_UINT:	AG_SetUint(obj, key, va_arg(ap,Uint));		break;
	case AG_PROP_FLOAT:	AG_SetFloat(obj, key, (float)va_arg(ap,double)); break;
	case AG_PROP_DOUBLE:	AG_SetDouble(obj, key, va_arg(ap,double));	break;
	case AG_PROP_STRING:	AG_SetString(obj, key, "%s", va_arg(ap,char *)); break;
	case AG_PROP_POINTER:	AG_SetPointer(obj, key, va_arg(ap,void *));	break;
	case AG_PROP_UINT8:	AG_SetUint8(obj, key, (Uint8)va_arg(ap,int));	break;
	case AG_PROP_SINT8:	AG_SetSint8(obj, key, (Sint8)va_arg(ap,int));	break;
	case AG_PROP_UINT16:	AG_SetUint16(obj, key, (Uint16)va_arg(ap,int));	break;
	case AG_PROP_SINT16:	AG_SetSint16(obj, key, (Sint16)va_arg(ap,int));	break;
	case AG_PROP_UINT32:	AG_SetUint32(obj, key, va_arg(ap,Uint32));	break;
	case AG_PROP_SINT32:	AG_SetSint32(obj, key, va_arg(ap,Sint32));	break;
	default:								break;
	}
	va_end(ap);
	AG_ObjectUnlock(obj);
	return (V);
}

#undef PROP_GET
#define PROP_GET(v,type) do { \
	if (p != NULL) *(type *)p = (type)V->data.v; \
} while (0)

/*
 * Lookup a property by name.
 * LEGACY interface to AG_Variable(3).
 */
AG_Variable *
AG_GetProp(void *pObj, const char *key, int t, void *p)
{
	AG_Object *obj = pObj;
	Uint i;

	AG_ObjectLock(obj);
	for (i = 0; i < obj->nVars; i++) {
		AG_Variable *V = &obj->vars[i];

		if ((t >= 0 && t != V->type) || strcmp(key, V->name) != 0) {
			continue;
		}
		switch (V->type) {
		case AG_PROP_INT: 	PROP_GET(i, int);	break;
		case AG_PROP_UINT:	PROP_GET(u, unsigned);	break;
		case AG_PROP_UINT8:	PROP_GET(u8, Uint8);	break;
		case AG_PROP_SINT8:	PROP_GET(s8, Sint8);	break;
		case AG_PROP_UINT16:	PROP_GET(u16, Uint16);	break;
		case AG_PROP_SINT16:	PROP_GET(s16, Sint16);	break;
		case AG_PROP_UINT32:	PROP_GET(u32, Uint32);	break;
		case AG_PROP_SINT32:	PROP_GET(s32, Sint32);	break;
		case AG_PROP_FLOAT:	PROP_GET(flt, float);	break;
		case AG_PROP_DOUBLE:	PROP_GET(dbl, double);	break;
		case AG_PROP_STRING:	PROP_GET(s, char *);	break;
		case AG_PROP_POINTER:	PROP_GET(p, void *);	break;
		default:
			AG_SetError("Bad prop %d", V->type);
			goto fail;
		}
		AG_ObjectUnlock(obj);
		return (V);
	}
	AG_SetError("%s: No such property: \"%s\" (%d)", obj->name, key, t);
fail:
	AG_ObjectUnlock(obj);
	return (NULL);
}

/*
 * Search for a property referenced by a "object-name:prop-name" string,
 * relative to the specified VFS.
 *
 * LEGACY Interface to AG_Variable(3).
 */
AG_Prop *
AG_FindProp(void *vfsRoot, const char *spec, int type, void *rval)
{
	char sb[AG_OBJECT_PATH_MAX+65];
	char *s = &sb[0], *objname, *propname;
	void *obj;

	Strlcpy(sb, spec, sizeof(sb));
	objname = Strsep(&s, ":");
	propname = Strsep(&s, ":");
	if (objname == NULL || propname == NULL ||
	    objname[0] == '\0' || propname[0] == '\0') {
		AG_SetError(_("Invalid property path: `%s'"), spec);
		return (NULL);
	}
	if ((obj = AG_ObjectFind(vfsRoot, objname)) == NULL) {
		return (NULL);
	}
	return (AG_GetProp(obj, propname, -1, rval));
}

/* LEGACY Interface to AG_Variable(3). */
int
AG_PropCopyPath(char *dst, size_t size, void *obj, const char *prop_name)
{
	if (AG_ObjectCopyName(obj, dst, size) == -1 ||
	    Strlcat(dst, ":", size) >= size ||
	    Strlcat(dst, prop_name, size) >= size) {
		AG_SetError("String overflow");
		return (-1);
	}
	return (0);
}

int
AG_PropLoad(void *p, AG_DataSource *ds)
{
	AG_Object *ob = p;
	Uint32 i, nprops;

	if (AG_ReadVersion(ds, "AG_PropTbl", &agPropTblVer, NULL) == -1)
		return (-1);

	AG_ObjectLock(ob);

	if ((ob->flags & AG_OBJECT_RELOAD_PROPS) == 0)
		AG_ObjectFreeProps(ob);

	nprops = AG_ReadUint32(ds);
	for (i = 0; i < nprops; i++) {
		char key[64];
		Uint32 t;

		if (AG_CopyString(key, ds, sizeof(key)) >= sizeof(key)) {
			AG_SetError("key %lu >= %lu", (Ulong)strlen(key),
			    (Ulong)sizeof(key));
			goto fail;
		}
		t = AG_ReadUint32(ds);
		
		switch (t) {
		case AG_LEGACY_PROP_BOOL:
			AG_SetBool(ob, key, (int)AG_ReadUint8(ds));
			break;
		case AG_LEGACY_PROP_UINT8:
			AG_SetBool(ob, key, AG_ReadUint8(ds));
			break;
		case AG_LEGACY_PROP_SINT8:
			AG_SetBool(ob, key, AG_ReadSint8(ds));
			break;
		case AG_LEGACY_PROP_UINT16:
			AG_SetUint16(ob, key, AG_ReadUint16(ds));
			break;
		case AG_LEGACY_PROP_SINT16:
			AG_SetSint16(ob, key, AG_ReadSint16(ds));
			break;
		case AG_LEGACY_PROP_UINT32:
			AG_SetUint32(ob, key, AG_ReadUint32(ds));
			break;
		case AG_LEGACY_PROP_SINT32:
			AG_SetSint32(ob, key, AG_ReadSint32(ds));
			break;
		case AG_LEGACY_PROP_UINT:
			AG_SetUint(ob, key, (Uint)AG_ReadUint32(ds));
			break;
		case AG_LEGACY_PROP_INT:
			AG_SetInt(ob, key, (int)AG_ReadSint32(ds));
			break;
		case AG_LEGACY_PROP_FLOAT:
			AG_SetFloat(ob, key, AG_ReadFloat(ds));
			break;
		case AG_LEGACY_PROP_DOUBLE:
			AG_SetDouble(ob, key, AG_ReadDouble(ds));
			break;
		case AG_LEGACY_PROP_STRING:
			{
				char *s = AG_ReadString(ds);
				AG_SetString(ob, key, "%s", s);
				Free(s);
			}
			break;
		default:
			AG_SetError("Cannot load property of type %d", (int)t);
			goto fail;
		}
	}
	AG_ObjectUnlock(ob);
	return (0);
fail:
	AG_ObjectUnlock(ob);
	return (-1);
}

int
AG_PropSave(void *p, AG_DataSource *ds)
{
	AG_Object *ob = p;
	off_t count_offs;
	Uint32 nVars = 0;
	Uint i;
	Uint8 c;
	
	AG_WriteVersion(ds, "AG_PropTbl", &agPropTblVer);
	
	AG_ObjectLock(ob);

	count_offs = AG_Tell(ds);			/* Skip count */
	AG_WriteUint32(ds, 0);

	for (i = 0; i < ob->nVars; i++) {
		AG_Variable *V = &ob->vars[i];

		AG_WriteString(ds, (char *)V->name);
		AG_WriteUint32(ds, V->type);
		switch (V->type) {
		case AG_LEGACY_PROP_BOOL:
			c = (V->data.i == 1) ? 1 : 0;
			AG_WriteUint8(ds, c);
			break;
		case AG_LEGACY_PROP_UINT8:
			AG_WriteUint8(ds, V->data.u8);
			break;
		case AG_LEGACY_PROP_SINT8:
			AG_WriteSint8(ds, V->data.s8);
			break;
		case AG_LEGACY_PROP_UINT16:
			AG_WriteUint16(ds, V->data.u16);
			break;
		case AG_LEGACY_PROP_SINT16:
			AG_WriteSint16(ds, V->data.s16);
			break;
		case AG_LEGACY_PROP_UINT32:
			AG_WriteUint32(ds, V->data.u32);
			break;
		case AG_LEGACY_PROP_SINT32:
			AG_WriteSint32(ds, V->data.s32);
			break;
		case AG_LEGACY_PROP_UINT:
			AG_WriteUint32(ds, (Uint32)V->data.u);
			break;
		case AG_LEGACY_PROP_INT:
			AG_WriteSint32(ds, (Sint32)V->data.i);
			break;
		case AG_LEGACY_PROP_FLOAT:
			AG_WriteFloat(ds, V->data.flt);
			break;
		case AG_LEGACY_PROP_DOUBLE:
			AG_WriteDouble(ds, V->data.dbl);
			break;
		case AG_LEGACY_PROP_STRING:
			AG_WriteString(ds, V->data.s);
			break;
		case AG_LEGACY_PROP_POINTER:
			break;
		default:
			AG_SetError("Property of type %d cannot be saved.",
			    V->type);
			goto fail;
		}
		nVars++;
	}
	AG_ObjectUnlock(ob);
	AG_WriteUint32At(ds, nVars, count_offs);	/* Write count */
	return (0);
fail:
	AG_ObjectUnlock(ob);
	return (-1);
}

size_t
AG_GetStringCopy(void *obj, const char *key, char *buf, size_t bufsize)
{
	size_t sl;
	char *s;

	AG_ObjectLock(obj);
	if (AG_GetProp(obj, key, AG_PROP_STRING, &s) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	sl = Strlcpy(buf, s, bufsize);
	AG_ObjectUnlock(obj);
	return (sl);
}
