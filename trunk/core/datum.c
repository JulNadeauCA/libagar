/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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

/* Names for standard datum types */
const char *agDatumTypeNames[] = {
	"NULL",
	"Uint",
	"int",
	"Uint8",
	"Sint8",
	"Uint16",
	"Sint16",
	"Uint32",
	"Sint32",
	"Uint64",
	"Sint64",
	"float",
	"double",
	"long double",
	"String",
	"const String",
	"Pointer",
	"const Pointer",

	"Uint *",
	"int *",
	"Uint8 *",
	"Sint8 *",
	"Uint16 *",
	"Sint16 *",
	"Uint32 *",
	"Sint32 *",
	"Uint64 *",
	"Sint64 *",
	"float *",
	"double *",
	"long double *",
	"String *",
	"const String *",
	"Pointer *",
	"const Pointer *",
	"AG_Object *",
	"Flag *",
	"Flag8 *",
	"Flag16 *",
	"Flag32 *",
	
	"Real",
	"Real *",
	"Range",
	"Range *",
	"Complex",
	"Complex *",
	"Quat",
	"Quat *",
	"Rectangular",
	"Rectangular *",
	"Polar",
	"Polar *",
	"Parabolic",
	"Parabolic *",
	"Spherical",
	"Spherical *",
	"Cylindrical",
	"Cylindrical *",
	"Color",
	"Color *",
	"Vector",
	"Vector *",
	"Vector2",
	"Vector2 *",
	"Vector3",
	"Vector3 *",
	"Vector4",
	"Vector4 *",
	"Matrix",
	"Matrix *",
	"Matrix22",
	"Matrix22 *",
	"Matrix33",
	"Matrix33 *",
	"Matrix44",
	"Matrix44 *",

	NULL
};

#undef SETARG
#define SETARG(t,pt,dmemb,dtype,vtype,ival,fnmemb,fntype)	\
	if (pFlag) {						\
		d.type = (pt);					\
	} else {						\
		d.type = (t);					\
		if (fnFlag) {					\
			d.data.dmemb = (ival);			\
			d.fn.fnmemb = va_arg(ap, fntype);	\
		} else {					\
			d.data.dmemb = (dtype)va_arg(ap, vtype); \
		}						\
	} while (0)

#undef SETARG_STRING
#define SETARG_STRING(t,pt,dmemb,dtype,ival,fnmemb,fntype)	\
	if (pFlag) {						\
		d.type = (pt);					\
	} else {						\
		d.type = (t);					\
		if (fnFlag) {					\
			d.data.dmemb = (ival);			\
			d.fn.fnmemb = va_arg(ap, fntype);	\
			d.info.size = 0;			\
		} else {					\
			d.data.dmemb = va_arg(ap, dtype);	\
			d.info.size = strlen(d.data.dmemb) + 1;	\
		}						\
	} while (0)

#undef SETARG_STRING_BUFFER
#define SETARG_STRING_BUFFER(t,pt,dmemb,dtype,ival,fnmemb,fntype) \
	if (pFlag) {						\
		d.type = (pt);					\
	} else {						\
		d.type = (t);					\
		if (fnFlag) {					\
			d.data.dmemb = (ival);			\
			d.fn.fnmemb = va_arg(ap, fntype);	\
			d.info.size = 0;			\
		} else {					\
			d.data.dmemb = va_arg(ap, dtype);	\
			d.info.size = va_arg(ap, size_t);	\
		}						\
	} while (0)

/* Parse arguments into a list of AG_Datum structures. */
AG_List *
AG_ParseDatumList(const char *argSpec, ...)
{
	char *asDup, *as, *s;
	AG_Datum d;
	AG_List *L;
	va_list ap;

	if ((L = AG_ListNew()) == NULL) {
		return (NULL);
	}
	asDup = Strdup(argSpec);
	as = &asDup[0];

	va_start(ap, argSpec);
	while ((s = AG_Strsep(&as, ":, ")) != NULL) {
		char *sc;
		int pFlag = 0, fnFlag = 0, lFlag = 0, isExtended = 0;
		int infmt = 0;

		d.type = AG_DATUM_NULL;
		d.name = NULL;
		d.mutex = NULL;
		d.fn.fnVoid = NULL;

		for (sc = &s[0]; *sc != '\0'; sc++) {
			if (*sc == '%') {
				infmt = 1;
				continue;
			}
			if (*sc == '*' && sc[1] != '\0') {
				pFlag++;
			} else if (*sc == 'l' && sc[1] != '\0') {
				lFlag++;
			} else if (*sc == 'F' && sc[1] != '\0') {
				fnFlag++;
			} else if (*sc == '[' && sc[1] != '\0') {
				isExtended++;
				break;
			} else if (infmt && strchr("*Csdiufgp]", *sc) != NULL) {
				break;
			} else if (strchr(".0123456789", *sc)) {
				continue;
			} else {
				infmt = 0;
			}
		}
		if (*sc == '\0' || !infmt) {
			AG_ListAppend(L, &d);
			continue;
		}
		if (pFlag) {
			d.data.p = va_arg(ap, void *);
		}
		if (isExtended) {
			sc++;
			if (sc[0] == 's') {
				/*
				 * Signed types: %[s*]
				 */
				if (sc[1] == '3' && sc[2] == '2') {
					SETARG(AG_DATUM_SINT32, AG_DATUM_P_SINT32,
					       s32, Sint32, int, 0,
					       fnSint32, AG_Sint32Fn);
				} else if (s[1] == '1' && sc[2] == '6') {
					SETARG(AG_DATUM_SINT16, AG_DATUM_P_SINT16,
					       s16, Sint16, int, 0,
					       fnSint16, AG_Sint16Fn);
				} else if (s[1] == '8') {
					SETARG(AG_DATUM_SINT8, AG_DATUM_P_SINT8,
					       s8, Sint8, int, 0,
					       fnSint8, AG_Sint8Fn);
				}
			} else if (sc[0] == 'u') {
				/*
				 * Unsigned types: %[u*]
				 */
				if (sc[1] == '3' && sc[2] == '2') {
					SETARG(AG_DATUM_UINT32, AG_DATUM_P_UINT32,
					       u32, Uint32, Uint, 0,
					       fnUint32, AG_Uint32Fn);
				} else if (s[1] == '1' && sc[2] == '6') {
					SETARG(AG_DATUM_UINT16, AG_DATUM_P_UINT16,
					       u16, Uint16, Uint, 0,
					       fnUint16, AG_Uint16Fn);
				} else if (s[1] == '8') {
					SETARG(AG_DATUM_UINT8, AG_DATUM_P_UINT8,
					       u8, Uint8, int, 0,
					       fnUint8, AG_Uint8Fn);
				}
			} else if (sc[0] == 'C') {
				/*
				 * Const types: %[C*]
				 */
				switch (sc[1]) {
				case 'p':
					SETARG(AG_DATUM_CONST_POINTER, AG_DATUM_P_CONST_POINTER,
					       Cp, const void *, const void *, NULL,
					       fnConstPointer, AG_ConstPointerFn);
					break;
				case 's':
					SETARG_STRING(AG_DATUM_CONST_STRING, AG_DATUM_P_CONST_STRING,
					              Cs, const char *, NULL,
					              fnConstString, AG_ConstStringFn);
					break;
				}
			} else if (sc[0] == 'B') {
				/*
				 * Explicitely sized string buffer: %[B]
				 */
				SETARG_STRING_BUFFER(AG_DATUM_STRING, AG_DATUM_P_STRING,
				                     s, char *, NULL,
				                     fnString, AG_StringFn);
			}
			break;
		}

		switch (sc[0]) {
		case 'p':
			SETARG(AG_DATUM_POINTER, AG_DATUM_P_POINTER,
			       p, void *, void *, NULL,
			       fnPointer, AG_PointerFn);
			break;
		case 's':
			SETARG_STRING(AG_DATUM_STRING, AG_DATUM_P_STRING,
			              s, char *, NULL,
				      fnString, AG_StringFn);
			break;
		case 'd':
		case 'i':
			if (lFlag == 0) {
				SETARG(AG_DATUM_INT, AG_DATUM_P_INT,
				       i, int, int, 0,
				       fnInt, AG_IntFn);
#ifdef HAVE_64BIT
			} else if (lFlag == 2) {	/* Alias for %[s64] */
				SETARG(AG_DATUM_SINT64, AG_DATUM_P_SINT64,
				       s64, Sint64, Sint64, 0,
				       fnSint64, AG_Sint64Fn);
#endif
			} else {			/* Alias for %[s32] */
				SETARG(AG_DATUM_SINT32, AG_DATUM_P_SINT32,
				       s32, Sint32, Sint32, 0,
				       fnSint32, AG_Sint32Fn);
			}
			break;
		case 'u':
			if (lFlag == 0) {
				SETARG(AG_DATUM_UINT, AG_DATUM_P_UINT,
				       u, Uint, Uint, 0,
				       fnUint, AG_UintFn);
#ifdef HAVE_64BIT
			} else if (lFlag == 2) {	/* Alias for %[u64] */
				SETARG(AG_DATUM_UINT64, AG_DATUM_P_UINT64,
				       u64, Uint64, Uint64, 0,
				       fnUint64, AG_Uint64Fn);
#endif
			} else {			/* Alias for %[u32] */
				SETARG(AG_DATUM_UINT32, AG_DATUM_P_UINT32,
				       u32, Uint32, Uint32, 0,
				       fnUint32, AG_Uint32Fn);
			}
			break;
		case 'f':
		case 'g':
			if (lFlag == 0) {
				SETARG(AG_DATUM_FLOAT, AG_DATUM_P_FLOAT,
				       flt, float, double, 0.0f,
				       fnFloat, AG_FloatFn);
#ifdef HAVE_LONG_DOUBLE
			} else if (lFlag == 2) {
				SETARG(AG_DATUM_LONG_DOUBLE, AG_DATUM_P_LONG_DOUBLE,
				       ldbl, long double, long double, 0.0l,
				       fnLongDouble, AG_LongDoubleFn);
#endif
			} else {
				SETARG(AG_DATUM_DOUBLE, AG_DATUM_P_DOUBLE,
				       dbl, double, double, 0.0,
				       fnDouble, AG_DoubleFn);
			}
			break;
		default:
			break;
		}
		AG_ListAppend(L, &d);
	}
	va_end(ap);
	Free(asDup);
	return (L);
}
