/*
 * Copyright (c) 2001-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Legacy AG_Widget(3) interfaces. These functions have been replaced by
 * the generic AG_Variable(3) API in agar-1.3.4.
 */

#include <agar/core/core.h>
#include <agar/gui/widget.h>

#include <stdarg.h>

#ifdef AG_LEGACY

/* Duplicate a widget binding. */
int
AG_WidgetCopyBinding(void *wDst, const char *nDst, AG_Variable *Vsrc)
{
	AG_Variable *Vdst;

	if ((Vdst = AG_GetVariable(wDst, nDst, NULL)) == NULL) {
		return (-1);
	}
	Vdst->type = Vsrc->type;
	Vdst->mutex = Vsrc->mutex;
	Vdst->data.p = Vsrc->data.p;

	switch (Vdst->type) {
	case AG_VARIABLE_P_FLAG:
	case AG_VARIABLE_P_FLAG8:
	case AG_VARIABLE_P_FLAG16:
	case AG_VARIABLE_P_FLAG32:
		Vdst->info.bitmask = Vsrc->info.bitmask;
		break;
	default:
		break;
	}
	if (AG_VARIABLE_TYPE(Vdst) == AG_VARIABLE_STRING) {
		Vdst->info.size = Vsrc->info.size;
	}

	AG_PostEvent(NULL, wDst, "bound", "%p", Vdst);
	AG_UnlockVariable(Vdst);
	return (0);
}

/* Bind a mutex-protected variable to a widget. */
AG_Variable *
AG_WidgetBindMp(void *obj, const char *name, AG_Mutex *mutex,
    enum ag_variable_type type, ...)
{
	AG_Widget *wid = obj;
	AG_Variable *V;
	va_list ap;
	void *p = NULL;
	Uint bitmask = 0;
	size_t size = 0;
	
	AG_ObjectLock(wid);

	va_start(ap, type);
	switch (type) {
	case AG_VARIABLE_P_FLAG:
	case AG_VARIABLE_P_FLAG8:
	case AG_VARIABLE_P_FLAG16:
	case AG_VARIABLE_P_FLAG32:
		p = va_arg(ap, void *);
		bitmask = va_arg(ap, Uint);
		break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_P_STRING:
	case AG_VARIABLE_CONST_STRING:
	case AG_VARIABLE_P_CONST_STRING:
		p = (void *)va_arg(ap, char *);
		size = va_arg(ap, size_t);
		break;
	default:
		p = va_arg(ap, void *);
		break;
	}
	va_end(ap);
	
	switch (type) {
	case AG_VARIABLE_P_INT:
		V = AG_BindInt(wid, name, (int *)p);
		break;
	case AG_VARIABLE_P_UINT8:
		V = AG_BindUint8(wid, name, (Uint8 *)p);
		break;
	case AG_VARIABLE_P_SINT8:
		V = AG_BindSint8(wid, name, (Sint8 *)p);
		break;
	case AG_VARIABLE_P_UINT16:
		V = AG_BindUint16(wid, name, (Uint16 *)p);
		break;
	case AG_VARIABLE_P_SINT16:
		V = AG_BindSint16(wid, name, (Sint16 *)p);
		break;
	case AG_VARIABLE_P_UINT32:
		V = AG_BindUint32(wid, name, (Uint32 *)p);
		break;
	case AG_VARIABLE_P_SINT32:
		V = AG_BindSint32(wid, name, (Sint32 *)p);
		break;
	case AG_VARIABLE_P_FLOAT:
		V = AG_BindFloat(wid, name, (float *)p);
		break;
	case AG_VARIABLE_P_DOUBLE:
		V = AG_BindDouble(wid, name, (double *)p);
		break;
	case AG_VARIABLE_P_FLAG:
		V = AG_BindFlag(wid, name, (Uint *)p, bitmask);
		break;
	case AG_VARIABLE_P_FLAG8:
		V = AG_BindFlag8(wid, name, (Uint8 *)p, (Uint8)bitmask);
		break;
	case AG_VARIABLE_P_FLAG16:
		V = AG_BindFlag16(wid, name, (Uint16 *)p, (Uint16)bitmask);
		break;
	case AG_VARIABLE_P_FLAG32:
		V = AG_BindFlag32(wid, name, (Uint32 *)p, (Uint32)bitmask);
		break;
	case AG_VARIABLE_P_STRING:
	case AG_VARIABLE_P_CONST_STRING:
		V = AG_BindString(wid, name, (char *)p, size);
		break;
	default:
		AG_ObjectUnlock(wid);
		return (NULL);
	}
	V->mutex = mutex;

	AG_ObjectUnlock(wid);
	return (V);
}

/* Bind a non mutex-protected variable to a widget. */
AG_Variable *
AG_WidgetBind(void *pObj, const char *name, enum ag_variable_type type, ...)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	va_list ap;

	AG_ObjectLock(obj);

	TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(V->name, name) == 0)
			break;
	}
	if (V == NULL) {
		V = Malloc(sizeof(AG_Variable));
		Strlcpy(V->name, name, sizeof(V->name));
	}
	V->type = type;
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	
	va_start(ap, type);
	switch (type) {
	case AG_VARIABLE_P_FLAG:
	case AG_VARIABLE_P_FLAG8:
	case AG_VARIABLE_P_FLAG16:
	case AG_VARIABLE_P_FLAG32:
		V->data.p = va_arg(ap, void *);
		V->info.bitmask = va_arg(ap, Uint);
		break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_P_STRING:
	case AG_VARIABLE_CONST_STRING:
	case AG_VARIABLE_P_CONST_STRING:
		V->data.p = va_arg(ap, char *);
		V->info.size = va_arg(ap, size_t);
		break;
	default:
		V->data.p = va_arg(ap, void *);
		V->info.bitmask = 0;
		break;
	}
	va_end(ap);

	AG_PostEvent(NULL, obj, "bound", "%p", V);
	AG_ObjectUnlock(obj);
	return (V);
}

/* Copy string from widget binding */
size_t
AG_WidgetCopyString(void *wid, const char *name, char *dst, size_t dst_size)
{
	AG_Variable *V;
	char *s;
	size_t rv;

	if ((V = AG_GetVariable(wid, name, &s)) == NULL) {
		AG_FatalError(NULL);
	}
	rv = Strlcpy(dst, s, dst_size);
	AG_UnlockVariable(V);
	return (rv);
}

/* Widget binding get/set routines */
int
AG_WidgetInt(void *wid, const char *name)
{
	AG_Variable *b;
	int *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Uint
AG_WidgetUint(void *wid, const char *name)
{
	AG_Variable *b;
	Uint *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Uint8
AG_WidgetUint8(void *wid, const char *name)
{
	AG_Variable *b;
	Uint8 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Sint8
AG_WidgetSint8(void *wid, const char *name)
{
	AG_Variable *b;
	Sint8 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Uint16
AG_WidgetUint16(void *wid, const char *name)
{
	AG_Variable *b;
	Uint16 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Sint16
AG_WidgetSint16(void *wid, const char *name)
{
	AG_Variable *b;
	Sint16 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Uint32
AG_WidgetUint32(void *wid, const char *name)
{
	AG_Variable *b;
	Uint32 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
Sint32
AG_WidgetSint32(void *wid, const char *name)
{
	AG_Variable *b;
	Sint32 *i, rv;
	if ((b = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	rv = *i;
	AG_UnlockVariable(b);
	return (rv);
}
float
AG_WidgetFloat(void *wid, const char *name)
{
	AG_Variable *b;
	float *f, rv;
	if ((b = AG_GetVariable(wid, name, &f)) == NULL) { AG_FatalError(NULL); }
	rv = *f;
	AG_UnlockVariable(b);
	return (rv);
}
double
AG_WidgetDouble(void *wid, const char *name)
{
	AG_Variable *b;
	double *d, rv;
	if ((b = AG_GetVariable(wid, name, &d)) == NULL) { AG_FatalError(NULL); }
	rv = *d;
	AG_UnlockVariable(b);
	return (rv);
}
char *
AG_WidgetString(void *wid, const char *name)
{
	AG_Variable *b;
	char *s, *sd;
	if ((b = AG_GetVariable(wid, name, &s)) == NULL) { AG_FatalError(NULL); }
	sd = Strdup(s);
	AG_UnlockVariable(b);
	return (sd);
}
void *
AG_WidgetPointer(void *wid, const char *name)
{
	AG_Variable *b;
	void **p;
	if ((b = AG_GetVariable(wid, name, &p)) == NULL) { AG_FatalError(NULL); }
	AG_UnlockVariable(b);
	return (p);
}
void
AG_WidgetSetInt(void *wid, const char *name, int ni)
{
	AG_Variable *V;
	int *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetUint(void *wid, const char *name, Uint ni)
{
	AG_Variable *V;
	Uint *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetUint8(void *wid, const char *name, Uint8 ni)
{
	AG_Variable *V;
	Uint8 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetSint8(void *wid, const char *name, Sint8 ni)
{
	AG_Variable *V;
	Sint8 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetUint16(void *wid, const char *name, Uint16 ni)
{
	AG_Variable *V;
	Uint16 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetSint16(void *wid, const char *name, Sint16 ni)
{
	AG_Variable *V;
	Sint16 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetUint32(void *wid, const char *name, Uint32 ni)
{
	AG_Variable *V;
	Uint32 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetSint32(void *wid, const char *name, Sint32 ni)
{
	AG_Variable *V;
	Sint32 *i;
	if ((V = AG_GetVariable(wid, name, &i)) == NULL) { AG_FatalError(NULL); }
	*i = ni;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetFloat(void *wid, const char *name, float nf)
{
	AG_Variable *V;
	float *f;
	if ((V = AG_GetVariable(wid, name, &f)) == NULL) { AG_FatalError(NULL); }
	*f = nf;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetDouble(void *wid, const char *name, double nd)
{
	AG_Variable *V;
	double *d;
	if ((V = AG_GetVariable(wid, name, &d)) == NULL) { AG_FatalError(NULL); }
	*d = nd;
	AG_UnlockVariable(V);
}
void
AG_WidgetSetPointer(void *wid, const char *name, void *np)
{
	AG_Variable *V;
	void **p;
	if ((V = AG_GetVariable(wid, name, &p)) == NULL) { AG_FatalError(NULL); }
	*p = np;
	AG_UnlockVariable(V);
}

#endif /* AG_LEGACY */
