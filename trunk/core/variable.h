/*	Public domain	*/

#include <agar/core/begin.h>

typedef enum ag_variable_type {
	/*
	 * Standard types
	 */
	AG_VARIABLE_NULL,		/* No data */
	AG_VARIABLE_UINT,		/* Unsigned int */
	AG_VARIABLE_INT,		/* Natural int */
	AG_VARIABLE_UINT8,		/* Unsigned 8-bit */
	AG_VARIABLE_SINT8,		/* Signed 8-bit */
	AG_VARIABLE_UINT16,		/* Unsigned 16-bit */
	AG_VARIABLE_SINT16,		/* Signed 16-bit */
	AG_VARIABLE_UINT32,		/* Unsigned 32-bit */
	AG_VARIABLE_SINT32,		/* Signed 32-bit */
	AG_VARIABLE_UINT64,		/* Unsigned 64-bit (optional) */
	AG_VARIABLE_SINT64,		/* Signed 64-bit (optional) */
	AG_VARIABLE_FLOAT,		/* Single-precision float */
	AG_VARIABLE_DOUBLE,		/* Double-precision float */
	AG_VARIABLE_LONG_DOUBLE,	/* Quad-precision float (optional) */
	AG_VARIABLE_STRING,		/* C string */
	AG_VARIABLE_CONST_STRING,	/* C string (const) */
	AG_VARIABLE_POINTER,		/* C pointer */
	AG_VARIABLE_CONST_POINTER,	/* C pointer (const) */

	AG_VARIABLE_P_UINT,		/* Pointer to Uint */
	AG_VARIABLE_P_INT,		/* Pointer to int */
	AG_VARIABLE_P_UINT8,		/* Pointer to Uint8 */
	AG_VARIABLE_P_SINT8,		/* Pointer to Sint8 */
	AG_VARIABLE_P_UINT16,		/* Pointer to Uint16 */
	AG_VARIABLE_P_SINT16,		/* Pointer to Sint16 */
	AG_VARIABLE_P_UINT32,		/* Pointer to Uint32 */
	AG_VARIABLE_P_SINT32,		/* Pointer to Sint32 */
	AG_VARIABLE_P_UINT64,		/* Pointer to Uint64 (optional) */
	AG_VARIABLE_P_SINT64,		/* Pointer to Sint64 (optional) */
	AG_VARIABLE_P_FLOAT,		/* Pointer to float */
	AG_VARIABLE_P_DOUBLE,		/* Pointer to double */
	AG_VARIABLE_P_LONG_DOUBLE,	/* Pointer to long double (optional) */
	AG_VARIABLE_P_STRING,		/* Pointer to C string */
	AG_VARIABLE_P_CONST_STRING,	/* Pointer to C string (const) */
	AG_VARIABLE_P_POINTER,		/* Pointer to C pointer */
	AG_VARIABLE_P_CONST_POINTER,	/* Pointer to C pointer (const) */
	AG_VARIABLE_P_OBJECT,		/* Pointer to AG_Object */
	AG_VARIABLE_P_FLAG,		/* Bit in int (uses info.mask) */
	AG_VARIABLE_P_FLAG8,		/* Bit in int8 (uses info.mask) */
	AG_VARIABLE_P_FLAG16,		/* Bit in int16 (uses info.mask) */
	AG_VARIABLE_P_FLAG32,		/* Bit in int32 (uses info.mask) */
	/*
	 * Standard types implemented by external libraries.
	 */
	AG_VARIABLE_REAL,		/* M: Real number */
	AG_VARIABLE_P_REAL,
	AG_VARIABLE_RANGE,		/* M: Interval */
	AG_VARIABLE_P_RANGE,
	AG_VARIABLE_COMPLEX,		/* M: Complex number */
	AG_VARIABLE_P_COMPLEX,
	AG_VARIABLE_QUAT,		/* M: Quaternion */
	AG_VARIABLE_P_QUAT,
	AG_VARIABLE_RECTANGULAR,	/* M: Rectangular coordinates */
	AG_VARIABLE_P_RECTANGULAR,
	AG_VARIABLE_POLAR,		/* M: Polar coordinates */
	AG_VARIABLE_P_POLAR,
	AG_VARIABLE_PARABOLIC,		/* M: Parabolic coordinates */
	AG_VARIABLE_P_PARABOLIC,
	AG_VARIABLE_SPHERICAL,		/* M: Spherical coordinates */
	AG_VARIABLE_P_SPHERICAL,
	AG_VARIABLE_CYLINDRICAL,	/* M: Cylindrical coordinates */
	AG_VARIABLE_P_CYLINDRICAL,
	AG_VARIABLE_COLOR,		/* M: Vector in RGBA color space */
	AG_VARIABLE_P_COLOR,
	AG_VARIABLE_VECTOR,		/* M: Vector in Rn */
	AG_VARIABLE_P_VECTOR,
	AG_VARIABLE_VECTOR2,		/* M: Vector in R2 */
	AG_VARIABLE_P_VECTOR2,
	AG_VARIABLE_VECTOR3,		/* M: Vector in R3 */
	AG_VARIABLE_P_VECTOR3,
	AG_VARIABLE_VECTOR4,		/* M: Vector in R4 */
	AG_VARIABLE_P_VECTOR4,
	AG_VARIABLE_MATRIX,		/* M: mxn matrix */
	AG_VARIABLE_P_MATRIX,
	AG_VARIABLE_MATRIX22,		/* M: 2x2 matrix */
	AG_VARIABLE_P_MATRIX22,
	AG_VARIABLE_MATRIX33,		/* M: 3x3 matrix */
	AG_VARIABLE_P_MATRIX33,
	AG_VARIABLE_MATRIX44,		/* M: 4x4 matrix */
	AG_VARIABLE_P_MATRIX44,
	/*
	 * For future standard types
	 */
	AG_VARIABLE_EXT4,
	AG_VARIABLE_EXT5,
	AG_VARIABLE_EXT6,
	AG_VARIABLE_EXT7,
	AG_VARIABLE_EXT8,
	AG_VARIABLE_EXT9,
	AG_VARIABLE_EXT10,
	AG_VARIABLE_EXT11,
	AG_VARIABLE_EXT12,
	AG_VARIABLE_EXT13,
	AG_VARIABLE_EXT14,
	AG_VARIABLE_EXT15,
	AG_VARIABLE_EXT16,
	AG_VARIABLE_EXT17,
	/*
	 * Application-specific extensions
	 */
	AG_VARIABLE_PRIVATE = 10001,
} AG_VariableType;

struct ag_event;

typedef void        (*AG_VoidFn)(struct ag_event *);
typedef Uint        (*AG_UintFn)(struct ag_event *);
typedef int         (*AG_IntFn)(struct ag_event *);
typedef Uint8       (*AG_Uint8Fn)(struct ag_event *);
typedef Sint8       (*AG_Sint8Fn)(struct ag_event *);
typedef Uint16      (*AG_Uint16Fn)(struct ag_event *);
typedef Sint16      (*AG_Sint16Fn)(struct ag_event *);
typedef Uint32      (*AG_Uint32Fn)(struct ag_event *);
typedef Sint32      (*AG_Sint32Fn)(struct ag_event *);
typedef float       (*AG_FloatFn)(struct ag_event *);
typedef double      (*AG_DoubleFn)(struct ag_event *);
typedef char       *(*AG_StringFn)(struct ag_event *, size_t *);
typedef const char *(*AG_ConstStringFn)(struct ag_event *, size_t *);
typedef void       *(*AG_PointerFn)(struct ag_event *);
typedef const void *(*AG_ConstPointerFn)(struct ag_event *);

typedef struct ag_variable {
	AG_VariableType type;		/* Variable type */
	const char *name;		/* Key */
	AG_Mutex *mutex;		/* Lock protecting data (or NULL) */
	union {
		Uint32 bitmask;		/* Bitmask (for AG_VARIABLE_P_FLAG_*) */
		size_t size;		/* Size (for AG_VARIABLE_STRING_*) */
	} info;
	union {
		void (*fnVoid)(struct ag_event *);
		Uint (*fnUint)(struct ag_event *);
		int (*fnInt)(struct ag_event *);
		Uint8 (*fnUint8)(struct ag_event *);
		Sint8 (*fnSint8)(struct ag_event *);
		Uint16 (*fnUint16)(struct ag_event *);
		Sint16 (*fnSint16)(struct ag_event *);
		Uint32 (*fnUint32)(struct ag_event *);
		Sint32 (*fnSint32)(struct ag_event *);
		float (*fnFloat)(struct ag_event *);
		double (*fnDouble)(struct ag_event *);
		char *(*fnString)(struct ag_event *, size_t *);
		const char *(*fnConstString)(struct ag_event *, size_t *);
		void *(*fnPointer)(struct ag_event *);
		const void *(*fnConstPointer)(struct ag_event *);
	} fn;
	union {
		void *p;
		const void *Cp;
		char *s;
		const char *Cs;
		int i;
		Uint u;
		float flt;
		double dbl;
		Uint8 u8;
		Sint8 s8;
		Uint16 u16;
		Sint16 s16;
		Uint32 u32;
		Sint32 s32;
	} data;
} AG_Variable;

#undef AG_VARIABLE_SETARG
#define AG_VARIABLE_SETARG(V,t,pt,dmemb,dtype,vtype,ival,fnmemb,fntype)	\
	if (pFlag) {							\
		(V)->type = (pt);					\
	} else {							\
		(V)->type = (t);					\
		if (fnFlag) {						\
			(V)->data.dmemb = (ival);			\
			(V)->fn.fnmemb = va_arg(ap, fntype);		\
		} else {						\
			(V)->data.dmemb = (dtype)va_arg(ap, vtype);	\
		}							\
	} while (0)

#undef AG_VARIABLE_SETARG_STRING
#define AG_VARIABLE_SETARG_STRING(V,t,pt,dmemb,dtype,ival,fnmemb,fntype) \
	if (pFlag) {							\
		(V)->type = (pt);					\
	} else {							\
		(V)->type = (t);					\
		if (fnFlag) {						\
			(V)->data.dmemb = (ival);			\
			(V)->fn.fnmemb = va_arg(ap, fntype);		\
			(V)->info.size = 0;				\
		} else {						\
			(V)->data.dmemb = va_arg(ap, dtype);		\
			(V)->info.size = strlen((V)->data.dmemb)+1;	\
		}							\
	} while (0)

#undef AG_VARIABLE_SETARG_STRING_BUFFER
#define AG_VARIABLE_SETARG_STRING_BUFFER(V,t,pt,dmemb,dtype,ival,fnmemb,fntype) \
	if (pFlag) {							\
		(V)->type = (pt);					\
	} else {							\
		(V)->type = (t);					\
		if (fnFlag) {						\
			(V)->data.dmemb = (ival);			\
			(V)->fn.fnmemb = va_arg(ap, fntype);		\
			(V)->info.size = 0;				\
		} else {						\
			(V)->data.dmemb = va_arg(ap, dtype);		\
			(V)->info.size = va_arg(ap, size_t);		\
		}							\
	} while (0)

/* Parse a variable specifier and get the value from varargs. */
#undef AG_VARIABLE_GET
#define AG_VARIABLE_GET(ap, fmtSpec, V)					\
	do {								\
		const char *sc;						\
		int pFlag = 0, fnFlag = 0, lFlag = 0, isExtended = 0;	\
		int inFmt = 0;						\
									\
		(V)->type = AG_VARIABLE_NULL;				\
		(V)->name = NULL;					\
		(V)->mutex = NULL;					\
		(V)->fn.fnVoid = NULL;					\
		(V)->info.bitmask = 0;					\
									\
		for (sc = &(fmtSpec)[0]; *sc != '\0'; sc++) {		\
			if (*sc == '%') {				\
				inFmt = 1;				\
				continue;				\
			}						\
			if (*sc == '*' && sc[1] != '\0') {		\
				pFlag++;				\
			} else if (*sc == 'l' && sc[1] != '\0') {	\
				lFlag++;				\
			} else if (*sc == 'F' && sc[1] != '\0') {	\
				fnFlag++;				\
			} else if (*sc == '[' && sc[1] != '\0') {	\
				isExtended++;				\
				break;					\
			} else if (inFmt && strchr("*Csdiufgp]", *sc)) { \
				break;					\
			} else if (strchr(".0123456789", *sc)) {	\
				continue;				\
			} else {					\
				inFmt = 0;				\
			}						\
		}							\
		if (*sc == '\0' || !inFmt) {				\
			break; 						\
		}							\
		if (pFlag) {						\
			(V)->data.p = va_arg(ap, void *);		\
		}							\
		if (isExtended) {					\
			sc++;						\
			if (sc[0] == 's') {				\
				if (sc[1] == '3' && sc[2] == '2') {	\
					AG_VARIABLE_SETARG((V),		\
					    AG_VARIABLE_SINT32,		\
					    AG_VARIABLE_P_SINT32,	\
					    s32, Sint32, int, 0,	\
					    fnSint32, AG_Sint32Fn);	\
				} else if (sc[1] == '1' && sc[2] == '6') { \
					AG_VARIABLE_SETARG((V),		\
					    AG_VARIABLE_SINT16,		\
					    AG_VARIABLE_P_SINT16,	\
					    s16, Sint16, int, 0,	\
					    fnSint16, AG_Sint16Fn);	\
				} else if (sc[1] == '8') {		\
					AG_VARIABLE_SETARG((V),		\
					    AG_VARIABLE_SINT8,		\
					    AG_VARIABLE_P_SINT8,	\
					    s8, Sint8, int, 0,		\
					    fnSint8, AG_Sint8Fn);	\
				}					\
			} else if (sc[0] == 'u') {			\
				if (sc[1] == '3' && sc[2] == '2') {	\
					AG_VARIABLE_SETARG((V),		\
					    AG_VARIABLE_UINT32,		\
					    AG_VARIABLE_P_UINT32,	\
					    u32, Uint32, Uint, 0,	\
					    fnUint32, AG_Uint32Fn);	\
				} else if (sc[1] == '1' && sc[2] == '6') { \
					AG_VARIABLE_SETARG((V),		\
					    AG_VARIABLE_UINT16,		\
					    AG_VARIABLE_P_UINT16,	\
					    u16, Uint16, Uint, 0,	\
					    fnUint16, AG_Uint16Fn);	\
				} else if (sc[1] == '8') {		\
					AG_VARIABLE_SETARG((V),		\
					    AG_VARIABLE_UINT8, 		\
					    AG_VARIABLE_P_UINT8,	\
					    u8, Uint8, int, 0,		\
					    fnUint8, AG_Uint8Fn);	\
				}					\
			} else if (sc[0] == 'C') {			\
				switch (sc[1]) {			\
				case 'p':				\
					AG_VARIABLE_SETARG((V),		\
					    AG_VARIABLE_CONST_POINTER,	\
					    AG_VARIABLE_P_CONST_POINTER, \
					    Cp, const void *, const void *, \
					    NULL,			\
					    fnConstPointer,		\
					    AG_ConstPointerFn);		\
					break;				\
				case 's':				\
					AG_VARIABLE_SETARG_STRING((V),	\
					    AG_VARIABLE_CONST_STRING,	\
					    AG_VARIABLE_P_CONST_STRING,	\
					    Cs, const char *, NULL,	\
					    fnConstString,		\
					    AG_ConstStringFn);		\
					break;				\
				}					\
			} else if (sc[0] == 'B') {			\
				AG_VARIABLE_SETARG_STRING_BUFFER((V),	\
				    AG_VARIABLE_STRING,			\
				    AG_VARIABLE_P_STRING,		\
				    s, char *, NULL,			\
				    fnString, AG_StringFn);		\
			}						\
			break;						\
		}							\
									\
		switch (sc[0]) {					\
		case 'p':						\
			AG_VARIABLE_SETARG((V),AG_VARIABLE_POINTER,	\
			    AG_VARIABLE_P_POINTER,			\
			    p, void *, void *, NULL,			\
			    fnPointer, AG_PointerFn);			\
			break;						\
		case 's':						\
			AG_VARIABLE_SETARG_STRING((V),AG_VARIABLE_STRING, \
			    AG_VARIABLE_P_STRING,			\
			    s, char *, NULL,				\
			    fnString, AG_StringFn);			\
			break;						\
		case 'd':						\
		case 'i':						\
			if (lFlag == 0) {				\
				AG_VARIABLE_SETARG((V),AG_VARIABLE_INT,	\
				    AG_VARIABLE_P_INT,			\
				    i, int, int, 0,			\
				    fnInt, AG_IntFn);			\
			} else {					\
				AG_VARIABLE_SETARG((V),AG_VARIABLE_SINT32, \
				    AG_VARIABLE_P_SINT32,		\
				    s32, Sint32, Sint32, 0,		\
				    fnSint32, AG_Sint32Fn);		\
			}						\
			break;						\
		case 'u':						\
			if (lFlag == 0) {				\
				AG_VARIABLE_SETARG((V),AG_VARIABLE_UINT, \
				    AG_VARIABLE_P_UINT,			\
				    u, Uint, Uint, 0,			\
				    fnUint, AG_UintFn);			\
			} else {					\
				AG_VARIABLE_SETARG((V),AG_VARIABLE_UINT32, \
				    AG_VARIABLE_P_UINT32,		\
				    u32, Uint32, Uint32, 0,		\
				    fnUint32, AG_Uint32Fn);		\
			}						\
			break;						\
		case 'f':						\
		case 'g':						\
			if (lFlag == 0) {				\
				AG_VARIABLE_SETARG((V),AG_VARIABLE_FLOAT, \
				    AG_VARIABLE_P_FLOAT,		\
				    flt, float, double, 0.0f,		\
				    fnFloat, AG_FloatFn);		\
			} else {					\
				AG_VARIABLE_SETARG((V),AG_VARIABLE_DOUBLE, \
				    AG_VARIABLE_P_DOUBLE,		\
				    dbl, double, double, 0.0,		\
				    fnDouble, AG_DoubleFn);		\
			}						\
			break;						\
		default:						\
			break;						\
		}							\
	} while (0)

__BEGIN_DECLS
struct ag_list;
extern const char *agVariableTypeNames[];

struct ag_list *AG_VariableList(const char *, ...);
void            AG_VariablePrint(char *, size_t, void *, const char *);

AG_Variable *AG_Set(void *, const char *, const char *, ...);

AG_Variable *AG_SetUint(void *, const char *, Uint);
AG_Variable *AG_BindUint(void *, const char *, Uint *);
AG_Variable *AG_BindUint_MP(void *, const char *, Uint *, AG_Mutex *);
AG_Variable *AG_SetInt(void *, const char *, int);
AG_Variable *AG_BindInt(void *, const char *, int *);
AG_Variable *AG_BindInt_MP(void *, const char *, int *, AG_Mutex *);

AG_Variable *AG_SetUint8(void *, const char *, Uint8);
AG_Variable *AG_BindUint8(void *, const char *, Uint8 *);
AG_Variable *AG_BindUint8_MP(void *, const char *, Uint8 *, AG_Mutex *);
AG_Variable *AG_SetSint8(void *, const char *, Sint8);
AG_Variable *AG_BindSint8(void *, const char *, Sint8 *);
AG_Variable *AG_BindSint8_MP(void *, const char *, Sint8 *, AG_Mutex *);

AG_Variable *AG_SetUint16(void *, const char *, Uint16);
AG_Variable *AG_BindUint16(void *, const char *, Uint16 *);
AG_Variable *AG_BindUint16_MP(void *, const char *, Uint16 *, AG_Mutex *);
AG_Variable *AG_SetSint16(void *, const char *, Sint16);
AG_Variable *AG_BindSint16(void *, const char *, Sint16 *);
AG_Variable *AG_BindSint16_MP(void *, const char *, Sint16 *, AG_Mutex *);

AG_Variable *AG_SetUint32(void *, const char *, Uint32);
AG_Variable *AG_BindUint32(void *, const char *, Uint32 *);
AG_Variable *AG_BindUint32_MP(void *, const char *, Uint32 *, AG_Mutex *);
AG_Variable *AG_SetSint32(void *, const char *, Sint32);
AG_Variable *AG_BindSint32(void *, const char *, Sint32 *);
AG_Variable *AG_BindSint32_MP(void *, const char *, Sint32 *, AG_Mutex *);

AG_Variable *AG_SetFloat(void *, const char *, float);
AG_Variable *AG_BindFloat(void *, const char *, float *);
AG_Variable *AG_BindFloat_MP(void *, const char *, float *, AG_Mutex *);
AG_Variable *AG_SetDouble(void *, const char *, double);
AG_Variable *AG_BindDouble(void *, const char *, double *);
AG_Variable *AG_BindDouble_MP(void *, const char *, double *, AG_Mutex *);

AG_Variable *AG_SetString(void *, const char *, const char *, ...);
AG_Variable *AG_BindString(void *, const char *, char *, size_t);
AG_Variable *AG_BindString_MP(void *, const char *, char *, size_t, AG_Mutex *);
AG_Variable *AG_SetConstString(void *, const char *, const char *);
AG_Variable *AG_BindConstString(void *, const char *, const char **);
AG_Variable *AG_BindConstString_MP(void *, const char *, const char **, AG_Mutex *);

AG_Variable *AG_SetPointer(void *, const char *, void *);
AG_Variable *AG_BindPointer(void *, const char *, void **);
AG_Variable *AG_BindPointer_MP(void *, const char *, void **, AG_Mutex *);
AG_Variable *AG_SetConstPointer(void *, const char *, const void *);
AG_Variable *AG_BindConstPointer(void *, const char *, const void **);
AG_Variable *AG_BindConstPointer_MP(void *, const char *, const void **, AG_Mutex *);

AG_Variable *AG_BindFlag(void *, const char *, Uint *, Uint);
AG_Variable *AG_BindFlag_MP(void *, const char *, Uint *, Uint, AG_Mutex *);
AG_Variable *AG_BindFlag8(void *, const char *, Uint8 *, Uint8);
AG_Variable *AG_BindFlag8_MP(void *, const char *, Uint8 *, Uint8, AG_Mutex *);
AG_Variable *AG_BindFlag16(void *, const char *, Uint16 *, Uint16);
AG_Variable *AG_BindFlag16_MP(void *, const char *, Uint16 *, Uint16, AG_Mutex *);
AG_Variable *AG_BindFlag32(void *, const char *, Uint32 *, Uint32);
AG_Variable *AG_BindFlag32_MP(void *, const char *, Uint32 *, Uint32, AG_Mutex *);

static __inline__ void
AG_VariableChanged(AG_Variable *V)
{
	/* TODO */
}
__END_DECLS

#include <agar/core/close.h>
