/*	Public domain	*/

/*
 * LEGACY Interface to AG_Variable(3).
 */

#include <agar/core/core.h>

#ifdef AG_LEGACY

#include <stdarg.h>
#include <string.h>

#undef PROP_GET
#define PROP_GET(v,type) do { \
	if (p != NULL) *(type *)p = (type)V->data.v; \
} while (0)

static AG_Variable *
GetProp(void *pObj, const char *key, int t, void *p)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
	TAILQ_FOREACH(V, &obj->vars, vars) {
		if ((t >= 0 && t != V->type) || strcmp(key, V->name) != 0) {
			continue;
		}
		switch (AG_VARIABLE_TYPE(V)) {
		case AG_VARIABLE_INT:    PROP_GET(i, int);	break;
		case AG_VARIABLE_UINT:   PROP_GET(u, unsigned);	break;
		case AG_VARIABLE_UINT8:  PROP_GET(u8, Uint8);	break;
		case AG_VARIABLE_SINT8:  PROP_GET(s8, Sint8);	break;
		case AG_VARIABLE_UINT16: PROP_GET(u16, Uint16);	break;
		case AG_VARIABLE_SINT16: PROP_GET(s16, Sint16);	break;
		case AG_VARIABLE_UINT32: PROP_GET(u32, Uint32);	break;
		case AG_VARIABLE_SINT32: PROP_GET(s32, Sint32);	break;
		case AG_VARIABLE_FLOAT:  PROP_GET(flt, float);	break;
		case AG_VARIABLE_DOUBLE: PROP_GET(dbl, double);	break;
		case AG_VARIABLE_STRING: PROP_GET(s, char *);	break;
		case AG_VARIABLE_POINTER:PROP_GET(p, void *);	break;
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

AG_Variable *
AG_GetProp(void *pObj, const char *key, int t, void *p)
{
	return GetProp(pObj, key, t, p);
}

AG_Prop *
AG_SetProp(void *pObj, const char *key, enum ag_prop_type type, ...)
{
	AG_Object *obj = pObj;
	va_list ap;

	AG_ObjectLock(obj);
	va_start(ap, type);
	switch (type) {
	case AG_PROP_INT:	AG_SetInt(obj, key, va_arg(ap,int));		break;
	case AG_PROP_UINT:	AG_SetUint(obj, key, va_arg(ap,Uint));		break;
	case AG_PROP_FLOAT:	AG_SetFloat(obj, key, (float)va_arg(ap,double)); break;
	case AG_PROP_DOUBLE:	AG_SetDouble(obj, key, va_arg(ap,double));	break;
	case AG_PROP_STRING:	AG_SetString(obj, key, va_arg(ap,char *));	break;
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

	return GetProp(obj, key, type, NULL);
}

#endif /* AG_LEGACY */
