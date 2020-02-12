/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include <string.h>

const AG_VariableTypeInfo agVariableTypes[] = {
	/* type         indirLvl    name           typeTgt code bytes */
	{ AG_VARIABLE_NULL,    0, "NULL", AG_VARIABLE_NULL,  -1,    0 },
	/*
	 * C integers
	 */
	{ AG_VARIABLE_UINT,    0, "Uint",    AG_VARIABLE_UINT,  0, sizeof(Uint) },
	{ AG_VARIABLE_P_UINT,  1, "Uint *",  AG_VARIABLE_UINT,  0, sizeof(Uint) },
	{ AG_VARIABLE_INT,     0, "int",     AG_VARIABLE_INT,   1, sizeof(int) },
	{ AG_VARIABLE_P_INT,   1, "int *",   AG_VARIABLE_INT,   1, sizeof(int) },
#if AG_MODEL != AG_SMALL
	{ AG_VARIABLE_ULONG,   0, "Ulong",   AG_VARIABLE_ULONG, 14, sizeof(Ulong) },
	{ AG_VARIABLE_P_ULONG, 1, "Ulong *", AG_VARIABLE_ULONG, 14, sizeof(Ulong) },
	{ AG_VARIABLE_LONG,    0, "long",    AG_VARIABLE_LONG,  15, sizeof(long) },
	{ AG_VARIABLE_P_LONG,  1, "long *",  AG_VARIABLE_LONG,  15, sizeof(long) },
#endif
	/*
	 * Fixed-width integers
	 */
	{ AG_VARIABLE_UINT8,	0, "Uint8",     AG_VARIABLE_UINT8,  2,	1 },
	{ AG_VARIABLE_P_UINT8,	1, "Uint8 *",   AG_VARIABLE_UINT8,  2,	1 },
	{ AG_VARIABLE_SINT8,	0, "Sint8",     AG_VARIABLE_SINT8,  3,	1 },
	{ AG_VARIABLE_P_SINT8,	1, "Sint8 *",   AG_VARIABLE_SINT8,  3,	1 },
#if AG_MODEL != AG_SMALL
	{ AG_VARIABLE_UINT16,	0, "Uint16",    AG_VARIABLE_UINT16, 4,	2 },
	{ AG_VARIABLE_P_UINT16,	1, "Uint16 *",  AG_VARIABLE_UINT16, 4,	2 },
	{ AG_VARIABLE_SINT16,	0, "Sint16",    AG_VARIABLE_SINT16, 5,	2 },
	{ AG_VARIABLE_P_SINT16,	1, "Sint16 *",  AG_VARIABLE_SINT16, 5,	2 },
	{ AG_VARIABLE_UINT32,	0, "Uint32",    AG_VARIABLE_UINT32, 6,	4 },
	{ AG_VARIABLE_P_UINT32,	1, "Uint32 *" , AG_VARIABLE_UINT32, 6,	4 },
	{ AG_VARIABLE_SINT32,	0, "Sint32",    AG_VARIABLE_SINT32, 7,	4 },
	{ AG_VARIABLE_P_SINT32,	1, "Sint32 *",  AG_VARIABLE_SINT32, 7,	4 },
	/*
	 * 64-bit integers (needs HAVE_64BIT)
	 */
	{ AG_VARIABLE_UINT64,	0, "Uint64",    AG_VARIABLE_UINT64, 8,	8 },
	{ AG_VARIABLE_P_UINT64,	1, "Uint64 *",  AG_VARIABLE_UINT64, 8,	8 },
	{ AG_VARIABLE_SINT64,	0, "Sint64",    AG_VARIABLE_SINT64, 9,	8 },
	{ AG_VARIABLE_P_SINT64,	1, "Sint64 *",  AG_VARIABLE_SINT64, 9,	8 },
	/*
	 * IEEE 754 Floating-point types (needs HAVE_FLOAT)
	 */
	{ AG_VARIABLE_FLOAT,         0, "float",	 AG_VARIABLE_FLOAT,       10, 4 },
	{ AG_VARIABLE_P_FLOAT,       1, "float *",	 AG_VARIABLE_FLOAT,       10, 4 },
	{ AG_VARIABLE_DOUBLE,        0, "double",	 AG_VARIABLE_DOUBLE,      11, 8 },
	{ AG_VARIABLE_P_DOUBLE,	     1, "double *",	 AG_VARIABLE_DOUBLE,      11, 8 },
#endif /* !AG_SMALL */
	/*
	 * C strings (STRING is auto-allocated; P_STRING is fixed-size buffer).
	 */
	{ AG_VARIABLE_STRING,	0, "String",   AG_VARIABLE_STRING, 13, sizeof(char *) },
	{ AG_VARIABLE_P_STRING,	1, "String *", AG_VARIABLE_STRING, 13, sizeof(char *) },
	/*
	 * Generic pointers.
	 */
	{ AG_VARIABLE_POINTER,	0, "Pointer",   AG_VARIABLE_POINTER, -1, sizeof(void *) },
	{ AG_VARIABLE_P_POINTER,1, "Pointer *", AG_VARIABLE_POINTER, -1, sizeof(void *) },
	/*
	 * Reference to one or more bits in a word.
	 */
	{ AG_VARIABLE_P_FLAG,	1, "Flag *",   AG_VARIABLE_UINT,   1, sizeof(Uint) },
	{ AG_VARIABLE_P_FLAG8,	1, "Flag8 *",  AG_VARIABLE_UINT8,  2, sizeof(Uint8) },
#if AG_MODEL != AG_SMALL
	{ AG_VARIABLE_P_FLAG16,	1, "Flag16 *", AG_VARIABLE_UINT16, 4, sizeof(Uint16) },
	{ AG_VARIABLE_P_FLAG32,	1, "Flag32 *", AG_VARIABLE_UINT32, 6, sizeof(Uint32) },
#endif
	/*
	 * Serializable Object or Object:Variable reference.
	 */
	{ AG_VARIABLE_P_OBJECT,   1, "Object *",   AG_VARIABLE_P_OBJECT,   16, sizeof(void *) },
	{ AG_VARIABLE_P_VARIABLE, 1, "Variable *", AG_VARIABLE_P_VARIABLE, 17, sizeof(void *) },
};

/* Import inlinables */
#undef AG_INLINE_HEADER
#include <agar/core/inline_variable.h>

/*
 * Duplicate a Variable. Preserve pointers, but duplicate allocated strings
 * and P_VARIABLE references.
 */
int
AG_CopyVariable(AG_Variable *Vdst, const AG_Variable *Vsrc)
{
#ifdef AG_DEBUG
	if (Vdst == Vsrc)
		AG_FatalErrorV("E37", "Vdst=Vsrc");
#endif
	memcpy(Vdst, Vsrc, sizeof(AG_Variable));

	if (Vsrc->type == AG_VARIABLE_STRING &&
	    Vsrc->info.size == 0) {
		if ((Vdst->data.s = TryStrdup(Vsrc->data.s)) == NULL)
			return (-1);
	} else if (Vsrc->type == AG_VARIABLE_P_VARIABLE) {
		if ((Vdst->info.varName = TryStrdup(Vsrc->info.varName)) == NULL)
			return (-1);
	}
	return (0);
}

/*
 * Duplicate a Variable with dereference. Mutate reference types into discrete
 * types (e.g., P_INT=>INT, P_STRING=>STRING, P_FLAG8=>UINT8).
 *
 * Note: P_OBJECT and P_VARIABLE types are never dereferenced.
 */
int
AG_DerefVariable(AG_Variable *Vdst, const AG_Variable *Vsrc)
{
#ifdef AG_DEBUG
	if (Vdst == Vsrc)
		AG_FatalErrorV("E37", "Vdst=Vsrc");
#endif
	memcpy(Vdst, Vsrc, sizeof(AG_Variable));

	Vdst->type = agVariableTypes[Vsrc->type].typeTgt;

	switch (Vsrc->type) {
	case AG_VARIABLE_P_UINT:   case AG_VARIABLE_P_INT:
	case AG_VARIABLE_P_UINT8:  case AG_VARIABLE_P_SINT8:
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_P_ULONG:  case AG_VARIABLE_P_LONG:
	case AG_VARIABLE_P_UINT16: case AG_VARIABLE_P_SINT16:
	case AG_VARIABLE_P_UINT32: case AG_VARIABLE_P_SINT32:
#endif
#ifdef HAVE_64BIT
	case AG_VARIABLE_P_UINT64: case AG_VARIABLE_P_SINT64:
#endif
#ifdef HAVE_FLOAT
	case AG_VARIABLE_P_FLOAT:  case AG_VARIABLE_P_DOUBLE:
#endif
	case AG_VARIABLE_P_FLAG:		/* to UINT */
	case AG_VARIABLE_P_FLAG8:		/* to UINT8 */
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_P_FLAG16:		/* to UINT16 */
	case AG_VARIABLE_P_FLAG32:		/* to UINT32 */
#endif
	case AG_VARIABLE_P_POINTER:
		memcpy(&Vdst->data, Vsrc->data.p,
		       agVariableTypes[Vsrc->type].size);
		break;
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_P_STRING:
		if ((Vdst->data.s = TryStrdup(Vsrc->data.s)) == NULL) {
			return (-1);
		}
		Vdst->info.size = 0;
		break;
	case AG_VARIABLE_P_VARIABLE:
		if ((Vdst->info.varName = TryStrdup(Vsrc->info.varName)) == NULL) {
			return (-1);
		}
		break;
	default:
		break;
	}
	return (0);
}

/*
 * Compare two variables without dereference. Compare discrete types
 * (including STRING) by value, and compare pointers as pointers.
 */
int
AG_CompareVariables(const AG_Variable *a, const AG_Variable *b)
{
	if (a->type != b->type) {
		return (1);
	}
	switch (a->type) {
	case AG_VARIABLE_STRING:
		return strcmp(a->data.s, b->data.s);
	default:
		return memcmp(
		    (const void *)&a->data,
		    (const void *)&b->data,
		    agVariableTypes[a->type].size);
	}
}

/*
 * Lookup a variable by name and return a generic pointer to its current value.
 * If the variable is a reference, the target is accessed.
 *
 * The variable is returned locked. Returns NULL if the variable is undefined.
 */
AG_Variable *
AG_GetVariable(void *pObj, const char *name, void **p)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	
	AG_ObjectLock(obj);
	if ((V = AG_AccessVariable(obj, name)) == NULL) {
#ifdef AG_VERBOSITY
		AG_FatalErrorF("%s: No \"%s\"", obj->name, name);
#else
		AG_FatalErrorV("E20", "No such variable");
#endif
	}
	*p = (agVariableTypes[V->type].indirLvl > 0) ? V->data.p :
	                                               (void *)&V->data;
	AG_ObjectUnlock(obj);
	return (V);
}

#ifdef AG_ENABLE_STRING
/*
 * Format a variable's value into a fixed-size buffer.
 *
 * Return the total length of the string that would have been created
 * were len unlimited.  The variable must be locked.
 */
AG_Size
AG_PrintVariable(char *s, AG_Size len, AG_Variable *V)
{
	switch (V->type) {
	case AG_VARIABLE_UINT:      return StrlcpyUint(s, V->data.u, len);
	case AG_VARIABLE_P_UINT:    return StrlcpyUint(s, *(Uint *)V->data.p, len);
	case AG_VARIABLE_INT:       return StrlcpyInt(s, V->data.i, len);
	case AG_VARIABLE_P_INT:     return StrlcpyInt(s, *(int *)V->data.p, len);
	case AG_VARIABLE_UINT8:     return StrlcpyUint(s, (Uint)V->data.u8, len);
	case AG_VARIABLE_P_UINT8:   return StrlcpyUint(s, (Uint)*(Uint8 *)V->data.p, len);
	case AG_VARIABLE_SINT8:     return StrlcpyInt(s, (int)V->data.s8, len);
	case AG_VARIABLE_P_SINT8:   return StrlcpyInt(s, (int)*(Sint8 *)V->data.p, len);
# if AG_MODEL != AG_SMALL
	case AG_VARIABLE_ULONG:     return Snprintf(s,len, "%lu", V->data.uli);
	case AG_VARIABLE_P_ULONG:   return Snprintf(s,len, "%lu", *(Ulong *)V->data.p);
	case AG_VARIABLE_LONG:      return Snprintf(s,len, "%ld", V->data.li);
	case AG_VARIABLE_P_LONG:    return Snprintf(s,len, "%ld", *(long *)V->data.p);
	case AG_VARIABLE_UINT16:    return StrlcpyUint(s, (Uint)V->data.u16, len);
	case AG_VARIABLE_P_UINT16:  return StrlcpyUint(s, (Uint)*(Uint16 *)V->data.p, len);
	case AG_VARIABLE_SINT16:    return StrlcpyInt(s, (int)V->data.s16, len);
	case AG_VARIABLE_P_SINT16:  return StrlcpyInt(s, (int)*(Sint16 *)V->data.p, len);
	case AG_VARIABLE_UINT32:    return Snprintf(s,len, "%lu", (Ulong)V->data.u32);
	case AG_VARIABLE_P_UINT32:  return Snprintf(s,len, "%lu", (Ulong)*(Uint32 *)V->data.p);
	case AG_VARIABLE_SINT32:    return Snprintf(s,len, "%ld", (long)V->data.s32);
	case AG_VARIABLE_P_SINT32:  return Snprintf(s,len, "%ld", (long)*(Sint32 *)V->data.p);
#endif
#ifdef HAVE_FLOAT
	case AG_VARIABLE_FLOAT:	    return Snprintf(s,len, "%.2f", V->data.flt);
	case AG_VARIABLE_P_FLOAT:   return Snprintf(s,len, "%.2f", *(float *)V->data.p);
	case AG_VARIABLE_DOUBLE:    return Snprintf(s,len, "%.2f", V->data.dbl);
	case AG_VARIABLE_P_DOUBLE:  return Snprintf(s,len, "%.2f", *(double *)V->data.p);
#endif
#ifdef HAVE_64BIT
	case AG_VARIABLE_SINT64:    return Snprintf(s,len, "%lld", (long long)V->data.s64);
	case AG_VARIABLE_P_SINT64:  return Snprintf(s,len, "%lld", (long long)*(Sint64 *)V->data.p);
	case AG_VARIABLE_UINT64:    return Snprintf(s,len, "%llu", (unsigned long long)V->data.u64);
	case AG_VARIABLE_P_UINT64:  return Snprintf(s,len, "%llu", (unsigned long long)*(Sint64 *)V->data.p);
#endif
	case AG_VARIABLE_POINTER:   return Snprintf(s,len, "%p", V->data.p);
	case AG_VARIABLE_P_POINTER: return Snprintf(s,len, "%p", *(void **)V->data.p);
	case AG_VARIABLE_P_FLAG:    return Snprintf(s,len, "%s", (*(Uint *)V->data.p & V->info.bitmask.u) ? "YES" : "NO");
	case AG_VARIABLE_P_FLAG8:   return Snprintf(s,len, "%s", (*(Uint8 *)V->data.p & V->info.bitmask.u8) ? "YES" : "NO");
# if AG_MODEL != AG_SMALL
	case AG_VARIABLE_P_FLAG16:  return Snprintf(s,len, "%s", (*(Uint16 *)V->data.p & V->info.bitmask.u16) ? "YES" : "NO");
	case AG_VARIABLE_P_FLAG32:  return Snprintf(s,len, "%s", (*(Uint32 *)V->data.p & V->info.bitmask.u32) ? "YES" : "NO");
# endif
	case AG_VARIABLE_STRING:
	case AG_VARIABLE_P_STRING: 
		return Snprintf(s,len, "\"%s\"", V->data.s);
	default:
		if (len >= 1) { s[0] = '\0'; }
		return (1);
	}
}
#endif /* AG_ENABLE_STRING */

/* Unset a variable */
void
AG_Unset(void *pObj, const char *name)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

#ifdef AG_DEBUG
	Debug(obj, "Unset \"" AGSI_YEL "%s" AGSI_RST "\"\n", name);
#endif
	TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(V->name, name) == 0) {
			TAILQ_REMOVE(&obj->vars, V, vars);
			AG_FreeVariable(V);
			free(V);
			break;
		}
	}
}

/* Generate "bound" event if BOUND_EVENTS flag is set */
#undef  FN_POST_BOUND_EVENT
#define FN_POST_BOUND_EVENT(obj, V) \
	if (OBJECT(obj)->flags & AG_OBJECT_BOUND_EVENTS) \
		AG_PostEvent((obj), "bound", "%p", (V))

#ifdef AG_VERBOSITY
# define FN_VARIABLE_GET_ACCESS_VARIABLE				\
	if ((V = AG_AccessVariable(obj,name)) == NULL) 			\
		AG_FatalErrorF("%s: No \"%s\"", OBJECT(obj)->name, name)
#else
# define FN_VARIABLE_GET_ACCESS_VARIABLE				\
	if ((V = AG_AccessVariable(obj,name)) == NULL) 			\
		AG_FatalErrorV("E20", "No such variable")
#endif

/* Body of AG_GetFoo() routines. */
#undef  FN_VARIABLE_GET
#define FN_VARIABLE_GET(_memb,_type) {				\
	_type rv;						\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	FN_VARIABLE_GET_ACCESS_VARIABLE;			\
	if (agVariableTypes[V->type].indirLvl > 0) {		\
		rv = *(_type *)V->data.p;			\
	} else {						\
		rv = V->data._memb;				\
	}							\
	AG_UnlockVariable(V);					\
	AG_ObjectUnlock(obj);					\
	return (rv);						\
}								\

/* Body of AG_SetFoo() routines. */
#undef  FN_VARIABLE_SET
#define FN_VARIABLE_SET(_memb,_type,ntype) {			\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = AG_FetchVariable(obj, name, ntype);			\
	if (agVariableTypes[V->type].indirLvl > 0) {		\
		*(_type *)V->data.p = v;			\
	} else {						\
		V->data._memb = v;				\
	}							\
	AG_ObjectUnlock(obj);					\
	return (V);						\
}

/* Body of AG_BindFoo() routines. */
#undef  FN_VARIABLE_BIND
#define FN_VARIABLE_BIND(ntype) {				\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = AG_FetchVariableOfType(obj, name, ntype);		\
	V->data.p = (void *)v;					\
	FN_POST_BOUND_EVENT(obj, V);				\
	AG_ObjectUnlock(obj);					\
	return (V);						\
}

/* Body of AG_BindFooMp() routines. */
#undef  FN_VARIABLE_BIND_MP
#define FN_VARIABLE_BIND_MP(ntype) 				\
	AG_Variable *V;						\
								\
	AG_ObjectLock(obj);					\
	V = AG_FetchVariableOfType(obj, name, ntype);		\
	V->mutex = mutex;					\
	V->data.p = (void *)v;					\
	FN_POST_BOUND_EVENT(obj, V);				\
	AG_ObjectUnlock(obj)

/*
 * Unsigned integer
 */
Uint
AG_GetUint(void *obj, const char *name)
{
	FN_VARIABLE_GET(u, Uint);
}
AG_Variable *
AG_SetUint(void *obj, const char *name, Uint v)
{
#ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Uint" AGSI_RST ") "
		   AGSI_BOLD "%u" AGSI_RST "\n", name, v);
#endif
	FN_VARIABLE_SET(u, Uint, AG_VARIABLE_UINT);
}
#if AG_MODEL != AG_SMALL
void
AG_InitUint(AG_Variable *V, Uint v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT, "");
	V->data.u = v;
}
#endif
AG_Variable *
AG_BindUint(void *obj, const char *name, Uint *v)
{
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint" AGSI_RST " *)%p (= "
		   AGSI_BOLD "0x%x" AGSI_RST ")\n",
		   name, v, *v);
#endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT);
}
#ifdef AG_THREADS
AG_Variable *
AG_BindUintMp(void *obj, const char *name, Uint *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint" AGSI_RST " *)%p (= "
		   AGSI_BOLD "%u" AGSI_RST ") mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, mutex);
# endif
	return (V);
}
#endif

/*
 * Signed integer
 */
int
AG_GetInt(void *obj, const char *name)
{
	FN_VARIABLE_GET(i, int);
}
AG_Variable *
AG_SetInt(void *obj, const char *name, int v)
{
#ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "int" AGSI_RST ") "
		   AGSI_BOLD "%d" AGSI_RST "\n",
		   name, v);
#endif
	FN_VARIABLE_SET(i, int, AG_VARIABLE_INT);
}
#if AG_MODEL != AG_SMALL
void
AG_InitInt(AG_Variable *V, int v)
{
	AG_InitVariable(V, AG_VARIABLE_INT, "");
	V->data.i = v;
}
#endif
AG_Variable *
AG_BindInt(void *obj, const char *name, int *v)
{
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	    AGSI_CYAN "int" AGSI_RST " *)%p (= "
	    AGSI_BOLD "%d" AGSI_RST ")\n",
	    name, v, *v);
#endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_INT);
}
#ifdef AG_THREADS
AG_Variable *
AG_BindIntMp(void *obj, const char *name, int *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_INT);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "int" AGSI_RST " *)%p (= "
	           AGSI_BOLD "%d" AGSI_RST ") mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
	           name, v, *v, mutex);
# endif
	return (V);
}
#endif

#if AG_MODEL != AG_SMALL
/*
 * Unsigned long integer
 */
Ulong
AG_GetUlong(void *obj, const char *name)
{
	FN_VARIABLE_GET(uli, Ulong);
}
AG_Variable *
AG_SetUlong(void *obj, const char *name, Ulong v)
{
#ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Ulong" AGSI_RST ") "
		   AGSI_BOLD "%lu" AGSI_RST "\n", name, v);
#endif
	FN_VARIABLE_SET(uli, Ulong, AG_VARIABLE_ULONG);
}
void
AG_InitUlong(AG_Variable *V, Ulong v)
{
	AG_InitVariable(V, AG_VARIABLE_ULONG, "");
	V->data.uli = v;
}
AG_Variable *
AG_BindUlong(void *obj, const char *name, Ulong *v)
{
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Ulong" AGSI_RST " *)%p (= "
		   AGSI_BOLD "%lu" AGSI_RST ")\n", name, v, *v);
#endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_ULONG);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindUlongMp(void *obj, const char *name, Ulong *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_ULONG);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Ulong" AGSI_RST " *)%p (= "
		   AGSI_BOLD "%lu" AGSI_RST ") mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, mutex);
# endif
	return (V);
}
# endif /* AG_THREADS */

/*
 * Signed long integer.
 */
long
AG_GetLong(void *obj, const char *name)
{
	FN_VARIABLE_GET(li, long);
}
AG_Variable *
AG_SetLong(void *obj, const char *name, long v)
{
#ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "long" AGSI_RST ") "
	           AGSI_BOLD "%ld" AGSI_RST "\n", name, v);
#endif
	FN_VARIABLE_SET(li, long, AG_VARIABLE_LONG);
}
void
AG_InitLong(AG_Variable *V, long v)
{
	AG_InitVariable(V, AG_VARIABLE_LONG, "");
	V->data.li = v;
}
AG_Variable *
AG_BindLong(void *obj, const char *name, long *v)
{
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "long" AGSI_RST " *)%p (= "
		   AGSI_BOLD "%ld" AGSI_RST ")\n", name, v, *v);
#endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_LONG);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindLongMp(void *obj, const char *name, long *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_LONG);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "long" AGSI_RST " *)%p (= "
		   AGSI_BOLD "%ld" AGSI_RST ") mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, mutex);
#  endif
	return (V);
}
# endif /* AG_THREADS */
#endif /* !AG_SMALL */

/*
 * Unsigned 8-bit integer
 */
Uint8
AG_GetUint8(void *obj, const char *name)
{
	FN_VARIABLE_GET(u8, Uint8);
}
AG_Variable *
AG_SetUint8(void *obj, const char *name, Uint8 v)
{
#ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Uint8" AGSI_RST ") "
		   AGSI_BOLD "0x%02x" AGSI_RST "\n",
		   name, v);
#endif
	FN_VARIABLE_SET(u8, Uint8, AG_VARIABLE_UINT8);
}
#if AG_MODEL != AG_SMALL
void
AG_InitUint8(AG_Variable *V, Uint8 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT8, "");
	V->data.u8 = v;
}
#endif
AG_Variable *
AG_BindUint8(void *obj, const char *name, Uint8 *v)
{
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint8" AGSI_RST " *)%p (= "
		   AGSI_BOLD "0x%02x" AGSI_RST ")\n",
		   name, v, *v);
#endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT8);
}
#ifdef AG_THREADS
AG_Variable *
AG_BindUint8Mp(void *obj, const char *name, Uint8 *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT8);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint8" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%02x" AGSI_RST ") mutex "
	           AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, mutex);
# endif
	return (V);
}
#endif

/*
 * Signed 8-bit integer
 */
Sint8
AG_GetSint8(void *obj, const char *name)
{
	FN_VARIABLE_GET(s8, Sint8);
}
AG_Variable *
AG_SetSint8(void *obj, const char *name, Sint8 v)
{
#ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Sint8" AGSI_RST ") "
		   AGSI_BOLD "0x%02x" AGSI_RST "\n",
		   name, v);
#endif
	FN_VARIABLE_SET(s8, Sint8, AG_VARIABLE_SINT8);
}
#if AG_MODEL != AG_SMALL
void
AG_InitSint8(AG_Variable *V, Sint8 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT8, "");
	V->data.s8 = v;
}
#endif
AG_Variable *
AG_BindSint8(void *obj, const char *name, Sint8 *v)
{
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Sint8" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%02x" AGSI_RST ")\n",
	           name, v, *v);
#endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_SINT8);
}
#ifdef AG_THREADS
AG_Variable *
AG_BindSint8Mp(void *obj, const char *name, Sint8 *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_SINT8);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Sint8" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%02x" AGSI_RST ") mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
	           name, v, *v, mutex);
# endif
	return (V);
}
#endif

#if AG_MODEL != AG_SMALL
/*
 * Unsigned 16-bit integer
 */
Uint16
AG_GetUint16(void *obj, const char *name)
{
	FN_VARIABLE_GET(u16, Uint16);
}
AG_Variable *
AG_SetUint16(void *obj, const char *name, Uint16 v)
{
# ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Uint16" AGSI_RST ") "
	           AGSI_BOLD "0x%04x" AGSI_RST "\n",
		   name, v);
# endif
	FN_VARIABLE_SET(u16, Uint16, AG_VARIABLE_UINT16);
}
void
AG_InitUint16(AG_Variable *V, Uint16 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT16, "");
	V->data.u16 = v;
}
AG_Variable *
AG_BindUint16(void *obj, const char *name, Uint16 *v)
{
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint16" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%04x" AGSI_RST ")\n",
		   name, v, *v);
# endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT16);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindUint16Mp(void *obj, const char *name, Uint16 *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT16);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint16" AGSI_RST " *)%p (= "
		   AGSI_BOLD "0x%04x" AGSI_RST ") mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, mutex);
#  endif
	return (V);
}
# endif

/*
 * Signed 16-bit integer
 */
Sint16
AG_GetSint16(void *obj, const char *name)
{
	FN_VARIABLE_GET(s16, Sint16);
}
AG_Variable *
AG_SetSint16(void *obj, const char *name, Sint16 v)
{
# ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Sint16" AGSI_RST ") "
		   AGSI_BOLD "0x%04x" AGSI_RST "\n",
		   name, v);
# endif
	FN_VARIABLE_SET(s16, Sint16, AG_VARIABLE_SINT16);
}
void
AG_InitSint16(AG_Variable *V, Sint16 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT16, "");
	V->data.s16 = v;
}
AG_Variable *
AG_BindSint16(void *obj, const char *name, Sint16 *v)
{
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Sint16" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%04x" AGSI_RST ")\n",
	           name, v, *v);
# endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_SINT16);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindSint16Mp(void *obj, const char *name, Sint16 *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_SINT16);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Sint16" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%04x" AGSI_RST ") mutex "
	           AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, mutex);
#  endif
	return (V);
}
# endif

/*
 * Unsigned 32-bit integer
 */
Uint32
AG_GetUint32(void *obj, const char *name)
{
	FN_VARIABLE_GET(u32, Uint32);
}
AG_Variable *
AG_SetUint32(void *obj, const char *name, Uint32 v)
{
# ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Uint32" AGSI_RST ") "
		   AGSI_BOLD "0x%08x" AGSI_RST "\n",
	           name, v);
# endif
	FN_VARIABLE_SET(u32, Uint32, AG_VARIABLE_UINT32);
}
void
AG_InitUint32(AG_Variable *V, Uint32 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT32, "");
	V->data.u32 = v;
}
AG_Variable *
AG_BindUint32(void *obj, const char *name, Uint32 *v)
{
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint32" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%08x" AGSI_RST ")\n",
		   name, v, *v);
# endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT32);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindUint32Mp(void *obj, const char *name, Uint32 *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT32);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint32" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%08x" AGSI_RST ") mutex "
	           AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, mutex);
#  endif
	return (V);
}
# endif /* AG_THREADS */

/*
 * Signed 32-bit integer
 */
Sint32
AG_GetSint32(void *obj, const char *name)
{
	FN_VARIABLE_GET(s32, Sint32);
}
AG_Variable *
AG_SetSint32(void *obj, const char *name, Sint32 v)
{
# ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Sint32" AGSI_RST ") "
	           AGSI_BOLD "0x%08x" AGSI_RST "\n",
		   name, v);
# endif
	FN_VARIABLE_SET(s32, Sint32, AG_VARIABLE_SINT32);
}
void
AG_InitSint32(AG_Variable *V, Sint32 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT32, "");
	V->data.s32 = v;
}
AG_Variable *
AG_BindSint32(void *obj, const char *name, Sint32 *v)
{
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Sint32" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%08x" AGSI_RST ")\n",
	           name, v, *v);
# endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_SINT32);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindSint32Mp(void *obj, const char *name, Sint32 *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_SINT32);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Sint32" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%08x" AGSI_RST ") mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, mutex);
#  endif
	return (V);
}
# endif /* AG_THREADS */

#endif /* !AG_SMALL */

#ifdef HAVE_64BIT
/*
 * Unsigned 64-bit integer
 */
Uint64
AG_GetUint64(void *obj, const char *name)
{
	FN_VARIABLE_GET(u64, Uint64);
}
AG_Variable *
AG_SetUint64(void *obj, const char *name, Uint64 v)
{
# ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Uint64" AGSI_RST ") "
	           AGSI_BOLD "0x%llx" AGSI_RST "\n",
	           name, (unsigned long long)v);
# endif
	FN_VARIABLE_SET(u64, Uint64, AG_VARIABLE_UINT64);
}
void
AG_InitUint64(AG_Variable *V, Uint64 v)
{
	AG_InitVariable(V, AG_VARIABLE_UINT64, "");
	V->data.u64 = v;
}
AG_Variable *
AG_BindUint64(void *obj, const char *name, Uint64 *v)
{
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint64" AGSI_RST ")%p (= "
	           AGSI_BOLD "0x%llx" AGSI_RST ")\n",
	           name, v, (unsigned long long)*v);
# endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_UINT64);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindUint64Mp(void *obj, const char *name, Uint64 *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_UINT64);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint64" AGSI_RST ")%p (= "
	           AGSI_BOLD "0x%llx" AGSI_RST ") mutex "
	           AGSI_BR_MAG "%p" AGSI_RST "\n",
	           name, v, (unsigned long long)*v, mutex);
#  endif
	return (V);
}
# endif /* AG_THREADS */

/*
 * Signed 64-bit integer
 */
Sint64
AG_GetSint64(void *obj, const char *name)
{
	FN_VARIABLE_GET(s64, Sint64);
}
AG_Variable *
AG_SetSint64(void *obj, const char *name, Sint64 v)
{
# if AG_MODEL == AG_LARGE
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "Sint64" AGSI_RST ") "
	           AGSI_BOLD "0x%llx" AGSI_RST "\n",
	           name, (long long)v);
# endif
	FN_VARIABLE_SET(s64, Sint64, AG_VARIABLE_SINT64);
}
void
AG_InitSint64(AG_Variable *V, Sint64 v)
{
	AG_InitVariable(V, AG_VARIABLE_SINT64, "");
	V->data.s64 = v;
}
AG_Variable *
AG_BindSint64(void *obj, const char *name, Sint64 *v)
{
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Sint64" AGSI_RST ")%p (= "
	           AGSI_BOLD "0x%llx" AGSI_RST ")\n",
	           name, v, (long long)*v);
# endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_SINT64);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindSint64Mp(void *obj, const char *name, Sint64 *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_SINT64);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Sint64" AGSI_RST ")%p (= "
	           AGSI_BOLD "0x%llx" AGSI_RST ") mutex "
	           AGSI_BR_MAG "%p" AGSI_RST "\n",
	           name, v, (long long)*v, mutex);
#  endif
	return (V);
}
# endif /* AG_THREADS */
#endif /* HAVE_64BIT */

#ifdef HAVE_FLOAT
/*
 * Single-precision floating-point number.
 */
float
AG_GetFloat(void *obj, const char *name)
{
	FN_VARIABLE_GET(flt, float);
}
AG_Variable *
AG_SetFloat(void *obj, const char *name, float v)
{
# ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "float" AGSI_RST ") "
		   AGSI_BOLD "%f" AGSI_RST "\n",
	           name, v);
# endif
	FN_VARIABLE_SET(flt, float, AG_VARIABLE_FLOAT);
}
void
AG_InitFloat(AG_Variable *V, float v)
{
	AG_InitVariable(V, AG_VARIABLE_FLOAT, "");
	V->data.flt = v;
}
AG_Variable *
AG_BindFloat(void *obj, const char *name, float *v)
{
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "float" AGSI_RST ")%p (= "
	           AGSI_BOLD "%f" AGSI_RST ")\n",
		   name, v, *v);
# endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_FLOAT);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindFloatMp(void *obj, const char *name, float *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_FLOAT);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "float" AGSI_RST ")%p (= "
	           AGSI_BOLD "%f" AGSI_RST ") mutex "
	           AGSI_BR_MAG "%p" AGSI_RST "\n",
	           name, v, *v, mutex);
#  endif
	return (V);
}
# endif /* AG_THREADS */

/*
 * Double-precision floating-point number.
 */
double
AG_GetDouble(void *obj, const char *name)
{
	FN_VARIABLE_GET(dbl, double);
}
AG_Variable *
AG_SetDouble(void *obj, const char *name, double v)
{
# ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "double" AGSI_RST ") "
	           AGSI_BOLD "%f" AGSI_RST "\n",
		   name, v);
# endif
	FN_VARIABLE_SET(dbl, double, AG_VARIABLE_DOUBLE);
}
void
AG_InitDouble(AG_Variable *V, double v)
{
	AG_InitVariable(V, AG_VARIABLE_DOUBLE, "");
	V->data.dbl = v;
}
AG_Variable *
AG_BindDouble(void *obj, const char *name, double *v)
{
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "double" AGSI_RST ")%p (= "
	           AGSI_BOLD "%f" AGSI_RST ")\n",
		   name, v, *v);
# endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_DOUBLE);
}
# ifdef AG_THREADS
AG_Variable *
AG_BindDoubleMp(void *obj, const char *name, double *v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_DOUBLE);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "double" AGSI_RST ")%p (= "
	           AGSI_BOLD "%f" AGSI_RST ") mutex "
	           AGSI_BR_MAG "%p" AGSI_RST "\n",
	           name, v, *v, mutex);
#  endif
	return (V);
}
# endif /* AG_THREADS */
#endif /* HAVE_FLOAT */

/*
 * Pointer routines.
 */
void *
AG_GetPointer(void *obj, const char *name)
{
	AG_Variable *V;
	void *p;

	AG_ObjectLock(obj);
	if ((V = AG_AccessVariable(obj, name)) == NULL) {
		AG_FatalErrorV("E20", "No such Pointer variable");
	}
#ifdef AG_TYPE_SAFETY
	if ((V->info.pFlags & AG_VARIABLE_P_READONLY))
		AG_FatalErrorV("E30", "Pointer is const. "
		                      "Did you mean AG_CONST_PTR()?");
#endif
	p = V->data.p;
	AG_UnlockVariable(V);
	AG_ObjectUnlock(obj);
	return (p);
}

const void *
AG_GetConstPointer(void *obj, const char *name)
{
	AG_Variable *V;
	void *p;

	AG_ObjectLock(obj);
	if ((V = AG_AccessVariable(obj, name)) == NULL) {
		AG_FatalErrorV("E20", "No such Pointer variable");
	}
	if (!(V->info.pFlags & AG_VARIABLE_P_READONLY)) {
		AG_FatalErrorV("E30", "Pointer is !const. "
		                      "Did you mean AG_PTR()?");
	}
	p = V->data.p;
	AG_UnlockVariable(V);
	AG_ObjectUnlock(obj);
	return (const void *)p;
}

AG_Variable *
AG_SetPointer(void *obj, const char *name, void *v)
{
	AG_Variable *V;
#ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "void" AGSI_RST " *)%p\n", name, v);
#endif
	AG_ObjectLock(obj);
	V = AG_FetchVariable(obj, name, AG_VARIABLE_POINTER);
	if (agVariableTypes[V->type].indirLvl > 0) {
		*(void **)V->data.p = v;
	} else {
		V->data.p = v;
	}
	V->info.pFlags = 0;
	AG_ObjectUnlock(obj);
	return (V);
}

#if AG_MODEL != AG_SMALL
AG_Variable *
AG_SetConstPointer(void *obj, const char *name, const void *v)
{
	AG_Variable *V;
# ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "const void" AGSI_RST " *)%p\n",
		   name, v);
# endif
	AG_ObjectLock(obj);
	V = AG_FetchVariable(obj, name, AG_VARIABLE_POINTER);
	V->data.p = (void *)v;
	V->info.pFlags = AG_VARIABLE_P_READONLY;
	AG_ObjectUnlock(obj);
	return (V);
}

void
AG_InitConstPointer(AG_Variable *V, const void *v)
{
	AG_InitPointer(V, (void *)v);
	V->info.pFlags |= AG_VARIABLE_P_READONLY;
}
#endif /* !AG_SMALL */

void
AG_InitPointer(AG_Variable *V, void *v)
{
	AG_InitVariable(V, AG_VARIABLE_POINTER, "");
	V->data.p = v;
}

AG_Variable *
AG_BindPointer(void *obj, const char *name, void **v)
{
#ifdef AG_DEBUG
	if (v != NULL) {
		Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
		           AGSI_CYAN "void" AGSI_RST " *)%p (= "
			   AGSI_BOLD "%p" AGSI_RST ")\n",
			   name, v, *v);
	} else {
		Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
		           AGSI_CYAN "void" AGSI_RST " *)%p (= "
			   AGSI_RED "NULL" AGSI_RST ")\n",
			   name, v);
	}
#endif
	FN_VARIABLE_BIND(AG_VARIABLE_P_POINTER);
}
#ifdef AG_THREADS
AG_Variable *
AG_BindPointerMp(void *obj, const char *name, void **v, AG_Mutex *mutex)
{
	FN_VARIABLE_BIND_MP(AG_VARIABLE_P_POINTER);
# ifdef AG_DEBUG
	if (v != NULL) {
		Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
		           AGSI_CYAN "void" AGSI_RST " *)%p (= "
			   AGSI_BOLD "%p" AGSI_RST ") mutex "
			   AGSI_BR_MAG "%p" AGSI_RST "\n",
			   name, v, *v, mutex);

	} else {
		Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
		           AGSI_CYAN "void" AGSI_RST " *)%p (= "
			   AGSI_BOLD "NULL" AGSI_RST ") mutex "
			   AGSI_BR_MAG "%p" AGSI_RST "\n",
			   name, v, mutex);
	}
# endif
	return (V);
}
#endif /* AG_THREADS */

/*
 * String routines.
 */

/*
 * Copy the contents of a string variable to a fixed-size buffer.
 *
 * Return the length of the string that would have been copied were
 * dstSize unlimited.
 */
AG_Size
AG_GetString(void *pObj, const char *name, char *dst, AG_Size dstSize)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	AG_Size rv;
	
	AG_ObjectLock(obj);

	if ((V = AG_AccessVariable(obj, name)) == NULL) {
#ifdef AG_VERBOSITY
		AG_FatalErrorF("%s: No \"%s\"", obj->name, name);
#else
		AG_FatalErrorV("E20", "No such variable");
#endif
	}
#ifdef AG_DEBUG
	if (!V->data.s) { AG_FatalError("!V->data.s"); }
#endif
	rv = Strlcpy(dst, V->data.s, dstSize);
	AG_UnlockVariable(V);
	AG_ObjectUnlock(obj);
	return (rv);
}

#if AG_MODEL != AG_SMALL
/* Return a newly-allocated copy of the contents of a string variable. */
char *
AG_GetStringDup(void *pObj, const char *name)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	char *s;

	AG_ObjectLock(obj);
	if ((V = AG_AccessVariable(obj, name)) == NULL) {
#ifdef AG_VERBOSITY
		AG_FatalErrorF("%s: No \"%s\"", obj->name, name);
#else
		AG_FatalErrorV("E20", "No such variable");
#endif
	}
	s = TryStrdup(V->data.s);
	AG_UnlockVariable(V);
	AG_ObjectUnlock(obj);
	return (s);
}

/*
 * Return a direct pointer to a string buffer (potentially unsafe).
 * The object must be locked (not free-threaded).
 */
char *
AG_GetStringP(void *pObj, const char *name)
{
	AG_Object *obj = pObj;
	AG_Variable *V;
	char *s;

	if ((V = AG_AccessVariable(obj, name)) == NULL) {
#ifdef AG_VERBOSITY
		AG_FatalErrorF("%s: No \"%s\"", obj->name, name);
#else
		AG_FatalErrorV("E20", "No such variable");
#endif
	}
	s = V->data.s;
	AG_UnlockVariable(V);
	return (s);
}
#endif /* !AG_SMALL */

/*
 * Create (or replace the contents of) a string variable.
 *
 * - If var exists as an unbounded string, then replace it with a copy of s.
 * - If var references a bounded fixed-size buffer, then copy s into it safely
 *   (possibly truncating to fit the buffer).
 * - If var is a different type, then mutate into a STRING with a copy of s.
 */
AG_Variable *
AG_SetString(void *pObj, const char *name, const char *s)
{
	AG_Object *obj = pObj;
	AG_Variable *V;

	AG_ObjectLock(obj);
#ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> \""
	           AGSI_BOLD "%s" AGSI_RST "\"\n",
		   name, s);
#endif
	TAILQ_FOREACH(V, &obj->vars, vars) {
		if (strcmp(V->name, name) == 0)
			break;
	}
	if (V == NULL) {
		V = Malloc(sizeof(AG_Variable));
		AG_InitVariable(V, AG_VARIABLE_STRING, name);
		TAILQ_INSERT_TAIL(&obj->vars, V, vars);

		V->info.size = 0;				/* Allocated */
		V->data.s = Strdup(s);
	} else {
		switch (V->type) {
		case AG_VARIABLE_STRING:
			if (V->data.s != NULL && V->info.size == 0) {
				free(V->data.s);
			}
			V->data.s = Strdup(s);
			V->info.size = 0;
			break;
		case AG_VARIABLE_P_STRING:
			AG_LockVariable(V);
#ifdef AG_DEBUG
			if (Strlcpy(V->data.s, s, V->info.size) >= V->info.size)
				Debug(obj, "%s: Truncated to fit %lu bytes\n",
				    name, (Ulong)V->info.size);
#else
			Strlcpy(V->data.s, s, V->info.size);
#endif
			AG_UnlockVariable(V);
			break;
		default:
#ifdef AG_DEBUG
			Debug(obj, "Mutate \"" AGSI_YEL "%s" AGSI_RST "\" "
			           "from (" AGSI_BR_GRN "%s" AGSI_RST ") to ("
				            AGSI_BR_GRN "%s" AGSI_RST ")\n",
					    name,
					    agVariableTypes[V->type].name,
			                    agVariableTypes[AG_VARIABLE_STRING].name);
#endif
			AG_FreeVariable(V);
			AG_InitVariable(V, AG_VARIABLE_STRING, name);
			V->info.size = 0;
			V->data.s = Strdup(s);
			break;
		}
	}
	AG_ObjectUnlock(obj);
	return (V);
}

#if AG_MODEL != AG_SMALL
/* Initialize an AG_Variable to be a string containing the given value. */
void
AG_InitString(AG_Variable *V, const char *v)
{
	AG_InitVariable(V, AG_VARIABLE_STRING, "");
	V->data.s = Strdup(v);
	V->info.size = 0;
}
#endif

/* Printf-style variant of AG_SetString() */
AG_Variable *
AG_SetStringF(void *obj, const char *name, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	return AG_SetStringNODUP(obj, name, s);
}

/*
 * Potentially-unsafe variant of AG_SetString() where s is assumed to be
 * a freeable, auto-allocated string (which must not be freed externally).
 */
AG_Variable *
AG_SetStringNODUP(void *obj, const char *name, char *s)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
#ifdef AG_DEBUG
	Debug(obj, "Set \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "char" AGSI_RST " *)"
		   AGSI_BR_RED "%p" AGSI_RST "<NODUP> (= \""
		   AGSI_BOLD "%s" AGSI_RST "\")\n", name, s, s);
#endif
	V = AG_FetchVariable(obj, name, AG_VARIABLE_STRING);
	switch (V->type) {
	case AG_VARIABLE_STRING:
		if (V->data.s != NULL && V->info.size == 0) {
			free(V->data.s);
		}
		V->data.s = s;
		V->info.size = 0;
		break;
	case AG_VARIABLE_P_STRING:
		AG_LockVariable(V);
#ifdef AG_DEBUG
		if (Strlcpy(V->data.s, s, V->info.size) >= V->info.size)
			Debug(obj, "%s: Truncated to fit %lu bytes\n",
			    name, (Ulong)V->info.size);
#else
		Strlcpy(V->data.s, s, V->info.size);
#endif
		AG_UnlockVariable(V);
		break;
	default:
#ifdef AG_DEBUG
		Debug(obj, "Mutate \"" AGSI_YEL "%s" AGSI_RST "\" from ("
		           AGSI_BR_GRN "%s" AGSI_RST ") to ("
			   AGSI_BR_GRN "%s" AGSI_RST ")\n",
		    name,
		    agVariableTypes[V->type].name,
		    agVariableTypes[AG_VARIABLE_STRING].name);
#endif
		AG_FreeVariable(V);
		AG_InitVariable(V, AG_VARIABLE_STRING, name);
		V->data.s = s;
		V->info.size = 0;
		break;
	}
	AG_ObjectUnlock(obj);
	return (V);
}

AG_Variable *
AG_BindString(void *obj, const char *name, char *buf, AG_Size bufSize)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
#ifdef AG_DEBUG
	if (strlen(buf) > 500) {
		Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST
		           "\" -> (" AGSI_CYAN "char" AGSI_RST " *)%p"
			   "[+" AGSI_BR_RED "%lu" AGSI_RST "]\n",
			   name, buf, (Ulong)bufSize);
	} else {
		Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST
		           "\" -> (" AGSI_CYAN "char" AGSI_RST " *)%p"
			   "[+" AGSI_BR_RED "%lu" AGSI_RST "] (= \""
			   AGSI_BOLD "%s" AGSI_RST "\")\n",
		    name, buf, (Ulong)bufSize, buf);
	}
#endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_STRING);
	V->data.s = buf;
	V->info.size = bufSize;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
#ifdef AG_THREADS
AG_Variable *
AG_BindStringMp(void *obj, const char *name, char *v, AG_Size size,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_BOLD "%s" AGSI_RST "\" -> ("
	           AGSI_CYAN "char" AGSI_RST " *)%p"
		   "[+" AGSI_BR_RED "%lu" AGSI_RST "] (= \""
		   AGSI_BOLD "%s" AGSI_RST "\") mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
	    name, v, (Ulong)size, v, mutex);
# endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_STRING);
	V->mutex = mutex;
	V->data.s = v;
	V->info.size = size;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
#endif /* AG_THREADS */

/*
 * Bitwise flag routines.
 */
AG_Variable *
AG_BindFlag(void *obj, const char *name, Uint *v, Uint bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "int" AGSI_RST " *)%p (= "
		   AGSI_BOLD "0x%x" AGSI_RST ") mask "
		   AGSI_BR_RED "0x%x" AGSI_RST "\n",
		   name, v, *v, bitmask);
#endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG);
	V->data.p = v;
	V->info.bitmask.u = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
#ifdef AG_THREADS
AG_Variable *
AG_BindFlagMp(void *obj, const char *name, Uint *v, Uint bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "int" AGSI_RST " *)%p (= "
		   AGSI_BOLD "0x%x" AGSI_RST ") mask "
		   AGSI_BR_RED "0x%x" AGSI_RST " mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, bitmask, mutex);
# endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask.u = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
#endif /* AG_THREADS */

AG_Variable *
AG_BindFlag8(void *obj, const char *name, Uint8 *v, Uint8 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint8" AGSI_RST " *)%p (= "
		   AGSI_BOLD "0x%02x" AGSI_RST ") mask "
		   AGSI_BR_RED "0x%02x" AGSI_RST "\n",
		   name, v, *v, bitmask);
#endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG8);
	V->data.p = v;
	V->info.bitmask.u8 = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
#ifdef AG_THREADS
AG_Variable *
AG_BindFlag8Mp(void *obj, const char *name, Uint8 *v, Uint8 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint8" AGSI_RST " *)%p (= "
		   AGSI_BOLD "0x%02x" AGSI_RST ") mask "
		   AGSI_BR_RED "0x%02x" AGSI_RST " mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, bitmask, mutex);
# endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG8);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask.u8 = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
#endif /* AG_THREADS */

#if AG_MODEL != AG_SMALL

AG_Variable *
AG_BindFlag16(void *obj, const char *name, Uint16 *v, Uint16 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint16" AGSI_RST " *)%p (= "
	           AGSI_BOLD "0x%04x" AGSI_RST ") mask "
		   AGSI_BR_RED "0x%04x" AGSI_RST "\n",
		   name, v, *v, bitmask);
# endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG16);
	V->data.p = v;
	V->info.bitmask.u16 = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}

# ifdef AG_THREADS
AG_Variable *
AG_BindFlag16Mp(void *obj, const char *name, Uint16 *v, Uint16 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint16" AGSI_RST " *)%p (= "
		   AGSI_BOLD "0x%04x" AGSI_RST ") mask "
		   AGSI_BR_RED "0x%04x" AGSI_RST " mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
	           name, v, *v, bitmask, mutex);
#  endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG16);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask.u16 = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
# endif /* AG_THREADS */

AG_Variable *
AG_BindFlag32(void *obj, const char *name, Uint32 *v, Uint32 bitmask)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
# ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint32" AGSI_RST " *)%p (= "
		   AGSI_BOLD "0x%08x" AGSI_RST ") mask "
		   AGSI_BR_RED "0x%08x" AGSI_RST "\n",
		   name, v, *v, bitmask);
# endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG32);
	V->data.p = v;
	V->info.bitmask.u32 = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}

# ifdef AG_THREADS
AG_Variable *
AG_BindFlag32Mp(void *obj, const char *name, Uint32 *v, Uint32 bitmask,
    AG_Mutex *mutex)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
#  ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_CYAN "Uint32" AGSI_RST " *)%p ("
		   AGSI_BOLD "0x%08x" AGSI_RST ") mask "
		   AGSI_BR_RED "0x%08x" AGSI_RST " mutex "
		   AGSI_BR_MAG "%p" AGSI_RST "\n",
		   name, v, *v, bitmask, mutex);
#  endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_FLAG32);
	V->mutex = mutex;
	V->data.p = v;
	V->info.bitmask.u32 = bitmask;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}
# endif /* AG_THREADS */

#endif /* !AG_SMALL */

/* Create a Variable to Object reference. */
AG_Variable *
AG_BindObject(void *obj, const char *name, void *tgtObj)
{
	AG_Variable *V;

	AG_ObjectLock(obj);
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> *("
	           AGSI_BR_CYAN "%s" AGSI_RST " *)%p (= <"
	           AGSI_BR_YEL "%s" AGSI_RST ">)\n",
	           name, OBJECT_CLASS(tgtObj)->name,
	           tgtObj, OBJECT(tgtObj)->name);
#endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_OBJECT);
	V->data.p = tgtObj;
	V->info.objName = NULL;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}

/* Create a Variable to Object:[Variable] reference. */
AG_Variable *
AG_BindVariable(void *obj, const char *name, void *tgtObj, const char *tgtKey)
{
	AG_Variable *V;
	char *keyDup = Strdup(tgtKey);

	AG_ObjectLock(obj);
#ifdef AG_DEBUG
	Debug(obj, "Bind \"" AGSI_YEL "%s" AGSI_RST "\" -> [*("
	           AGSI_BR_CYAN "%s" AGSI_RST " *)%p (= "
		   AGSI_BR_YEL "%s" AGSI_RST ")]:"
		   AGSI_YEL "%s" AGSI_RST "\n",
		   name, OBJECT_CLASS(tgtObj)->name,
		   tgtObj, OBJECT(tgtObj)->name, tgtKey);
#endif
	V = AG_FetchVariableOfType(obj, name, AG_VARIABLE_P_VARIABLE);
	V->data.p = tgtObj;
	V->info.varName = keyDup;
	FN_POST_BOUND_EVENT(obj, V);
	AG_ObjectUnlock(obj);
	return (V);
}

#ifdef AG_ENABLE_STRING
/*
 * Substitute variable references of the form "$(foo)" in a string.
 */
void
AG_VariableSubst(void *obj, const char *s, char *dst, AG_Size len)
{
	char key[AG_VARIABLE_NAME_MAX], val[64];
	const char *c, *cEnd;
	AG_Variable *V;
	AG_Size kLen;

	if (len < 1) {
		return;
	}
	AG_ObjectLock(obj);
	dst[0] = '\0';
	for (c = &s[0]; *c != '\0'; ) {
		if (c[0] == '$' && c[1] == '(' &&
		    (cEnd = strchr(&c[2], ')')) != NULL) {
			if ((kLen = cEnd-&c[1]) >= sizeof(key)) {
				c = &cEnd[1];
				continue;
			}
			memcpy(key, &c[2], kLen);
			key[kLen-1] = '\0';
			if (strcmp(key, "name") == 0) {
				Strlcat(dst, OBJECT(obj)->name, len);
			} else if (strcmp(key, "class") == 0) {
				Strlcat(dst, OBJECT(obj)->cls->name, len);
			} else {
				if ((V = AG_AccessVariable(obj, key)) != NULL) {
					AG_PrintVariable(val, sizeof(val), V);
					Strlcat(dst, val, len);
					AG_UnlockVariable(V);
				}
#if 0
				} else {
					Strlcat(dst, "$(", len);
					Strlcat(dst, key, len);
					Strlcat(dst, ")", len);
				}
#endif
			}
			c = cEnd+1;
			continue;
		}
		val[0] = *c;
		val[1] = '\0';
		Strlcat(dst, val, len);
		c++;
	}
	AG_ObjectUnlock(obj);
}
#endif /* AG_ENABLE_STRING */
