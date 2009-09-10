/*	Public domain	*/

#include <agar/core/begin.h>

typedef enum ag_variable_type {
	AG_VARIABLE_NULL,		/* No data */
	/* Primitive */
	AG_VARIABLE_UINT,		/* Unsigned int */
	AG_VARIABLE_P_UINT,		/* Pointer to Uint */
	AG_VARIABLE_INT,		/* Natural int */
	AG_VARIABLE_P_INT,		/* Pointer to int */
	AG_VARIABLE_UINT8,		/* Unsigned 8-bit */
	AG_VARIABLE_P_UINT8,		/* Pointer to Uint8 */
	AG_VARIABLE_SINT8,		/* Signed 8-bit */
	AG_VARIABLE_P_SINT8,		/* Pointer to Sint8 */
	AG_VARIABLE_UINT16,		/* Unsigned 16-bit */
	AG_VARIABLE_P_UINT16,		/* Pointer to Uint16 */
	AG_VARIABLE_SINT16,		/* Signed 16-bit */
	AG_VARIABLE_P_SINT16,		/* Pointer to Sint16 */
	AG_VARIABLE_UINT32,		/* Unsigned 32-bit */
	AG_VARIABLE_P_UINT32,		/* Pointer to Uint32 */
	AG_VARIABLE_SINT32,		/* Signed 32-bit */
	AG_VARIABLE_P_SINT32,		/* Pointer to Sint32 */
	AG_VARIABLE_UINT64,		/* Unsigned 64-bit (optional) */
	AG_VARIABLE_P_UINT64,		/* Pointer to Uint64 (optional) */
	AG_VARIABLE_SINT64,		/* Signed 64-bit (optional) */
	AG_VARIABLE_P_SINT64,		/* Pointer to Sint64 (optional) */
	AG_VARIABLE_FLOAT,		/* Single-precision float */
	AG_VARIABLE_P_FLOAT,		/* Pointer to float */
	AG_VARIABLE_DOUBLE,		/* Double-precision float */
	AG_VARIABLE_P_DOUBLE,		/* Pointer to double */
	AG_VARIABLE_LONG_DOUBLE,	/* Quad-precision float (optional) */
	AG_VARIABLE_P_LONG_DOUBLE,	/* Pointer to long double (optional) */
	AG_VARIABLE_STRING,		/* C string */
	AG_VARIABLE_P_STRING,		/* Pointer to C string */
	AG_VARIABLE_CONST_STRING,	/* C string (const) */
	AG_VARIABLE_P_CONST_STRING,	/* Pointer to C string (const) */
	AG_VARIABLE_POINTER,		/* C pointer */
	AG_VARIABLE_P_POINTER,		/* Pointer to C pointer */
	AG_VARIABLE_CONST_POINTER,	/* C pointer (const) */
	AG_VARIABLE_P_CONST_POINTER, 	/* Pointer to C pointer (const) */
	/* Bitmask */
	AG_VARIABLE_P_FLAG,		/* Bit in int (uses info.mask) */
	AG_VARIABLE_P_FLAG8,		/* Bit in int8 (uses info.mask) */
	AG_VARIABLE_P_FLAG16,		/* Bit in int16 (uses info.mask) */
	AG_VARIABLE_P_FLAG32,		/* Bit in int32 (uses info.mask) */
	/* Agar-Core */
	AG_VARIABLE_P_OBJECT,		/* Pointer to AG_Object */
#if 0
	/* Agar-Math */
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
#endif
	AG_VARIABLE_TYPE_LAST
} AG_VariableType;

typedef struct ag_variable_type_info {
	enum ag_variable_type type;		/* Variable type */
	int indirLvl;				/* Indirection level */
	const char *name;			/* Name string */
	enum ag_variable_type typeTgt;		/* Pointer target type (or AG_VARIABLE_NULL) */
	int code;				/* Numerical code (-1 = non persistent) */
} AG_VariableTypeInfo;

#define AG_VARIABLE_NAME_MAX	40
#define AG_VARIABLE_BOOL	AG_VARIABLE_INT

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
typedef size_t      (*AG_StringFn)(struct ag_event *, char *, size_t);
typedef void       *(*AG_PointerFn)(struct ag_event *);
typedef const void *(*AG_ConstPointerFn)(struct ag_event *);

union ag_variable_fn {
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
	size_t (*fnString)(struct ag_event *, char *, size_t);
	void *(*fnPointer)(struct ag_event *);
	const void *(*fnConstPointer)(struct ag_event *);
};
	
union ag_variable_data {
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
};

typedef struct ag_variable {
	char name[AG_VARIABLE_NAME_MAX]; /* Variable name */
	AG_VariableType type;	 	 /* Variable type */
	AG_Mutex *mutex;		 /* Lock protecting data (or NULL) */
	union {
		Uint32 bitmask;		 /* Bitmask (for AG_VARIABLE_P_FLAG_*) */
		size_t size;		 /* Size (for AG_VARIABLE_STRING_*) */
	} info;
	union ag_variable_fn fn;
	union ag_variable_data data;
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

/* Parse a variable list of AG_Variable arguments. */
#undef  AG_PARSE_VARIABLE_ARGS
#define AG_PARSE_VARIABLE_ARGS(ap, fmtString, L, argSizes)		\
{									\
	const char *fmtSpec;						\
	int nArgs = 0;							\
									\
	for (fmtSpec = &(fmtString)[0]; *fmtSpec != '\0'; ) {		\
		const char *c;						\
		int pFlag = 0, fnFlag = 0, lFlag = 0, isExtended = 0;	\
		int fmtChars, inFmt = 0;				\
		AG_Variable V;						\
									\
		for (c = &fmtSpec[0], fmtChars = 0;			\
		     *c != '\0';					\
		     c++) {						\
			if (*c == '%') { inFmt = 1; }			\
			if (inFmt) { fmtChars++; }			\
			if (*c == '%') {				\
				continue;				\
			} else if (*c == '*' && c[1] != '\0') {	\
				pFlag++;				\
			} else if (*c == 'l' && c[1] != '\0') {	\
				lFlag++;				\
			} else if (*c == 'F' && c[1] != '\0') {	\
				fnFlag++;				\
			} else if (*c == '[' && c[1] != '\0') {	\
				isExtended++;				\
				break;					\
			} else if (inFmt && strchr("*Csdiufgp]", *c)) { \
				break;					\
			} else if (strchr(".0123456789", *c)) {	\
				continue;				\
			} else {					\
				inFmt = 0;				\
			}						\
		}							\
		fmtSpec += fmtChars;					\
		if (*c == '\0') { break; }				\
		if (!inFmt) { continue;	}				\
									\
		(argSizes) = AG_Realloc((argSizes),			\
		    (nArgs+1)*sizeof(int));				\
		(argSizes)[nArgs++] = fmtChars;				\
									\
		V.type = AG_VARIABLE_NULL;				\
		V.name[0] = '\0';					\
		V.mutex = NULL;						\
		V.fn.fnVoid = NULL;					\
		V.info.bitmask = 0;					\
									\
		if (pFlag) {						\
			V.data.p = va_arg(ap, void *);			\
		}							\
		if (isExtended) {					\
			c++;						\
			if (c[0] == 's') {				\
				if (c[1] == '3' && c[2] == '2') {	\
					AG_VARIABLE_SETARG(&V,		\
					    AG_VARIABLE_SINT32,		\
					    AG_VARIABLE_P_SINT32,	\
					    s32, Sint32, int, 0,	\
					    fnSint32, AG_Sint32Fn);	\
				} else if (c[1] == '1' && c[2] == '6') { \
					AG_VARIABLE_SETARG(&V,		\
					    AG_VARIABLE_SINT16,		\
					    AG_VARIABLE_P_SINT16,	\
					    s16, Sint16, int, 0,	\
					    fnSint16, AG_Sint16Fn);	\
				} else if (c[1] == '8') {		\
					AG_VARIABLE_SETARG(&V,		\
					    AG_VARIABLE_SINT8,		\
					    AG_VARIABLE_P_SINT8,	\
					    s8, Sint8, int, 0,		\
					    fnSint8, AG_Sint8Fn);	\
				}					\
			} else if (c[0] == 'u') {			\
				if (c[1] == '3' && c[2] == '2') {	\
					AG_VARIABLE_SETARG(&V,		\
					    AG_VARIABLE_UINT32,		\
					    AG_VARIABLE_P_UINT32,	\
					    u32, Uint32, Uint, 0,	\
					    fnUint32, AG_Uint32Fn);	\
				} else if (c[1] == '1' && c[2] == '6') { \
					AG_VARIABLE_SETARG(&V,		\
					    AG_VARIABLE_UINT16,		\
					    AG_VARIABLE_P_UINT16,	\
					    u16, Uint16, Uint, 0,	\
					    fnUint16, AG_Uint16Fn);	\
				} else if (c[1] == '8') {		\
					AG_VARIABLE_SETARG(&V,		\
					    AG_VARIABLE_UINT8, 		\
					    AG_VARIABLE_P_UINT8,	\
					    u8, Uint8, int, 0,		\
					    fnUint8, AG_Uint8Fn);	\
				}					\
			} else if (c[0] == 'C') {			\
				switch (c[1]) {			\
				case 'p':				\
					AG_VARIABLE_SETARG(&V,		\
					    AG_VARIABLE_CONST_POINTER,	\
					    AG_VARIABLE_P_CONST_POINTER, \
					    Cp, const void *, const void *, \
					    NULL,			\
					    fnConstPointer,		\
					    AG_ConstPointerFn);		\
					break;				\
				case 's':				\
					AG_VARIABLE_SETARG_STRING(&V,	\
					    AG_VARIABLE_CONST_STRING,	\
					    AG_VARIABLE_P_CONST_STRING,	\
					    Cs, const char *, NULL,	\
					    fnString,			\
					    AG_StringFn);		\
					break;				\
				}					\
			} else if (c[0] == 'B') {			\
				AG_VARIABLE_SETARG_STRING_BUFFER(&V,	\
				    AG_VARIABLE_STRING,			\
				    AG_VARIABLE_P_STRING,		\
				    s, char *, NULL,			\
				    fnString, AG_StringFn);		\
			}						\
			break;						\
		}							\
									\
		switch (c[0]) {						\
		case 'p':						\
			AG_VARIABLE_SETARG(&V, AG_VARIABLE_POINTER,	\
			    AG_VARIABLE_P_POINTER,			\
			    p, void *, void *, NULL,			\
			    fnPointer, AG_PointerFn);			\
			break;						\
		case 's':						\
			AG_VARIABLE_SETARG_STRING(&V,			\
			    AG_VARIABLE_STRING,				\
			    AG_VARIABLE_P_STRING,			\
			    s, char *, NULL,				\
			    fnString, AG_StringFn);			\
			break;						\
		case 'd':						\
		case 'i':						\
			if (lFlag == 0) {				\
				AG_VARIABLE_SETARG(&V, AG_VARIABLE_INT,	\
				    AG_VARIABLE_P_INT,			\
				    i, int, int, 0,			\
				    fnInt, AG_IntFn);			\
			} else {					\
				AG_VARIABLE_SETARG(&V,			\
				    AG_VARIABLE_SINT32,			\
				    AG_VARIABLE_P_SINT32,		\
				    s32, Sint32, Sint32, 0,		\
				    fnSint32, AG_Sint32Fn);		\
			}						\
			break;						\
		case 'u':						\
			if (lFlag == 0) {				\
				AG_VARIABLE_SETARG(&V,			\
				    AG_VARIABLE_UINT,			\
				    AG_VARIABLE_P_UINT,			\
				    u, Uint, Uint, 0,			\
				    fnUint, AG_UintFn);			\
			} else {					\
				AG_VARIABLE_SETARG(&V,			\
				    AG_VARIABLE_UINT32,			\
				    AG_VARIABLE_P_UINT32,		\
				    u32, Uint32, Uint32, 0,		\
				    fnUint32, AG_Uint32Fn);		\
			}						\
			break;						\
		case 'f':						\
		case 'g':						\
			if (lFlag == 0) {				\
				AG_VARIABLE_SETARG(&V,			\
				    AG_VARIABLE_FLOAT,			\
				    AG_VARIABLE_P_FLOAT,		\
				    flt, float, double, 0.0f,		\
				    fnFloat, AG_FloatFn);		\
			} else {					\
				AG_VARIABLE_SETARG(&V,			\
				    AG_VARIABLE_DOUBLE,			\
				    AG_VARIABLE_P_DOUBLE,		\
				    dbl, double, double, 0.0,		\
				    fnDouble, AG_DoubleFn);		\
			}						\
			break;						\
		default:						\
			break;						\
		}							\
		AG_ListAppend((L), &V);					\
	}								\
}

__BEGIN_DECLS
struct ag_list;
extern const AG_VariableTypeInfo agVariableTypes[];

int		AG_EvalVariable(void *, AG_Variable *);
void            AG_PrintVariable(char *, size_t, AG_Variable *);
AG_Variable    *AG_GetVariableVFS(void *, const char *)
                    WARN_UNUSED_RESULT_ATTRIBUTE;
AG_Variable    *AG_GetVariable(void *, const char *, ...)
                    WARN_UNUSED_RESULT_ATTRIBUTE;
int             AG_CopyVariable(AG_Variable *, const AG_Variable *);
AG_Variable    *AG_Set(void *, const char *, const char *, ...);
void		AG_Unset(void *, const char *);
void            AG_VariableSubst(void *, const char *, char *, size_t)
	            BOUNDED_ATTRIBUTE(__string__, 3, 4);

Uint         AG_GetUint(void *, const char *);
void         AG_InitUint(AG_Variable *, Uint);
AG_Variable *AG_SetUint(void *, const char *, Uint);
AG_Variable *AG_BindUint(void *, const char *, Uint *);
AG_Variable *AG_BindUintFn(void *, const char *, AG_UintFn, const char *, ...);
AG_Variable *AG_BindUintMp(void *, const char *, Uint *, AG_Mutex *);

int          AG_GetInt(void *, const char *);
AG_Variable *AG_SetInt(void *, const char *, int);
void         AG_InitInt(AG_Variable *, int);
AG_Variable *AG_BindInt(void *, const char *, int *);
AG_Variable *AG_BindIntFn(void *, const char *, AG_IntFn, const char *, ...);
AG_Variable *AG_BindIntMp(void *, const char *, int *, AG_Mutex *);

#define      AG_GetBool		AG_GetInt
#define      AG_SetBool		AG_SetInt
#define      AG_BindBool	AG_BindInt
#define      AG_BindBoolFn	AG_BindIntFn
#define      AG_BindBoolMp	AG_BindIntMp

Uint8        AG_GetUint8(void *, const char *);
AG_Variable *AG_SetUint8(void *, const char *, Uint8);
void         AG_InitUint8(AG_Variable *, Uint8);
AG_Variable *AG_BindUint8(void *, const char *, Uint8 *);
AG_Variable *AG_BindUint8Fn(void *, const char *, AG_Uint8Fn, const char *, ...);
AG_Variable *AG_BindUint8Mp(void *, const char *, Uint8 *, AG_Mutex *);

Sint8        AG_GetSint8(void *, const char *);
AG_Variable *AG_SetSint8(void *, const char *, Sint8);
void         AG_InitSint8(AG_Variable *, Sint8);
AG_Variable *AG_BindSint8(void *, const char *, Sint8 *);
AG_Variable *AG_BindSint8Fn(void *, const char *, AG_Sint8Fn, const char *, ...);
AG_Variable *AG_BindSint8Mp(void *, const char *, Sint8 *, AG_Mutex *);

Uint16       AG_GetUint16(void *, const char *);
AG_Variable *AG_SetUint16(void *, const char *, Uint16);
void         AG_InitUint16(AG_Variable *, Uint16);
AG_Variable *AG_BindUint16(void *, const char *, Uint16 *);
AG_Variable *AG_BindUint16Fn(void *, const char *, AG_Uint16Fn, const char *, ...);
AG_Variable *AG_BindUint16Mp(void *, const char *, Uint16 *, AG_Mutex *);

Sint16       AG_GetSint16(void *, const char *);
AG_Variable *AG_SetSint16(void *, const char *, Sint16);
void         AG_InitSint16(AG_Variable *, Sint16);
AG_Variable *AG_BindSint16Fn(void *, const char *, AG_Sint16Fn, const char *, ...);
AG_Variable *AG_BindSint16(void *, const char *, Sint16 *);
AG_Variable *AG_BindSint16Mp(void *, const char *, Sint16 *, AG_Mutex *);

Uint32       AG_GetUint32(void *, const char *);
AG_Variable *AG_SetUint32(void *, const char *, Uint32);
void         AG_InitUint32(AG_Variable *, Uint32);
AG_Variable *AG_BindUint32Fn(void *, const char *, AG_Uint32Fn, const char *, ...);
AG_Variable *AG_BindUint32(void *, const char *, Uint32 *);
AG_Variable *AG_BindUint32Mp(void *, const char *, Uint32 *, AG_Mutex *);

Sint32       AG_GetSint32(void *, const char *);
AG_Variable *AG_SetSint32(void *, const char *, Sint32);
void         AG_InitSint32(AG_Variable *, Sint32);
AG_Variable *AG_BindSint32Fn(void *, const char *, AG_Sint32Fn, const char *, ...);
AG_Variable *AG_BindSint32(void *, const char *, Sint32 *);
AG_Variable *AG_BindSint32Mp(void *, const char *, Sint32 *, AG_Mutex *);

float        AG_GetFloat(void *, const char *);
AG_Variable *AG_SetFloat(void *, const char *, float);
void         AG_InitFloat(AG_Variable *, float);
AG_Variable *AG_BindFloatFn(void *, const char *, AG_FloatFn, const char *, ...);
AG_Variable *AG_BindFloat(void *, const char *, float *);
AG_Variable *AG_BindFloatMp(void *, const char *, float *, AG_Mutex *);

double       AG_GetDouble(void *, const char *);
AG_Variable *AG_SetDouble(void *, const char *, double);
void         AG_InitDouble(AG_Variable *, double);
AG_Variable *AG_BindDoubleFn(void *, const char *, AG_DoubleFn, const char *, ...);
AG_Variable *AG_BindDouble(void *, const char *, double *);
AG_Variable *AG_BindDoubleMp(void *, const char *, double *, AG_Mutex *);

size_t       AG_GetString(void *, const char *, char *, size_t)
	         BOUNDED_ATTRIBUTE(__string__, 3, 4);
char        *AG_GetStringDup(void *, const char *);
AG_Variable *AG_SetString(void *, const char *, const char *);
AG_Variable *AG_SetStringNODUP(void *, const char *, char *);
AG_Variable *AG_SetStringFixed(void *, const char *, char *, size_t)
                 BOUNDED_ATTRIBUTE(__string__, 3, 4);
void         AG_InitString(AG_Variable *, const char *);
void         AG_InitStringNODUP(AG_Variable *, char *);
void         AG_InitStringFixed(AG_Variable *, char *, size_t)
                 BOUNDED_ATTRIBUTE(__string__, 2, 3);
AG_Variable *AG_PrtString(void *, const char *, const char *, ...);
AG_Variable *AG_BindString(void *, const char *, char *, size_t);
AG_Variable *AG_BindStringFn(void *, const char *, AG_StringFn, const char *, ...);
AG_Variable *AG_BindStringMp(void *, const char *, char *, size_t, AG_Mutex *);
AG_Variable *AG_SetConstString(void *, const char *, const char *);
AG_Variable *AG_BindConstString(void *, const char *, const char **);
AG_Variable *AG_BindConstStringMp(void *, const char *, const char **, AG_Mutex *);

void        *AG_GetPointer(void *, const char *);
AG_Variable *AG_SetPointer(void *, const char *, void *);
void         AG_InitPointer(AG_Variable *, void *);
AG_Variable *AG_BindPointer(void *, const char *, void **);
AG_Variable *AG_BindPointerFn(void *, const char *, AG_PointerFn, const char *, ...);
AG_Variable *AG_BindPointerMp(void *, const char *, void **, AG_Mutex *);

const void  *AG_GetConstPointer(void *, const char *);
AG_Variable *AG_SetConstPointer(void *, const char *, const void *);
void         AG_InitConstPointer(AG_Variable *, const void *);
AG_Variable *AG_BindConstPointer(void *, const char *, const void **);
AG_Variable *AG_BindConstPointerFn(void *, const char *, AG_ConstPointerFn, const char *, ...);
AG_Variable *AG_BindConstPointerMp(void *, const char *, const void **, AG_Mutex *);

AG_Variable *AG_BindFlag(void *, const char *, Uint *, Uint);
AG_Variable *AG_BindFlagMp(void *, const char *, Uint *, Uint, AG_Mutex *);
AG_Variable *AG_BindFlag8(void *, const char *, Uint8 *, Uint8);
AG_Variable *AG_BindFlag8Mp(void *, const char *, Uint8 *, Uint8, AG_Mutex *);
AG_Variable *AG_BindFlag16(void *, const char *, Uint16 *, Uint16);
AG_Variable *AG_BindFlag16Mp(void *, const char *, Uint16 *, Uint16, AG_Mutex *);
AG_Variable *AG_BindFlag32(void *, const char *, Uint32 *, Uint32);
AG_Variable *AG_BindFlag32Mp(void *, const char *, Uint32 *, Uint32, AG_Mutex *);

static __inline__ int          AG_Defined(void *, const char *)
                                   WARN_UNUSED_RESULT_ATTRIBUTE;
static __inline__ AG_Variable *AG_GetVariableLocked(void *, const char *)
                                   WARN_UNUSED_RESULT_ATTRIBUTE;

/* Return 1 if the named variable exists. */
static __inline__ int
AG_Defined(void *pObj, const char *name)
{
	AG_Object *obj = AGOBJECT(pObj);
	Uint i;

	AG_ObjectLock(obj);
	for (i = 0; i < obj->nVars; i++) {
		if (strcmp(name, obj->vars[i].name) == 0) {
			AG_ObjectUnlock(obj);
			return (1);
		}
	}
	AG_ObjectUnlock(obj);
	return (0);
}

/* Acquire any locking device associated with a variable. */
static __inline__ void
AG_LockVariable(AG_Variable *V)
{
	if (V->mutex != NULL) { AG_MutexLock(V->mutex); }
}

/* Release any locking device associated with a variable. */
static __inline__ void
AG_UnlockVariable(AG_Variable *V)
{
	if (V->mutex != NULL) { AG_MutexUnlock(V->mutex); }
}

/* Release all resources associated with a variable. */
static __inline__ void
AG_FreeVariable(AG_Variable *V)
{
	if (V->type == AG_VARIABLE_STRING && V->info.size == 0)
		AG_Free(V->data.s);
}

/*
 * Lookup a variable by name and return it locked.
 * Object must be locked.
 */
static __inline__ AG_Variable *
AG_GetVariableLocked(void *pObj, const char *name)
{
	AG_Object *obj = AGOBJECT(pObj);
	AG_Variable *V;
	Uint i;
	
	for (i = 0; i < obj->nVars; i++) {
		if (strcmp(obj->vars[i].name, name) == 0)
			break;
	}
	if (i == obj->nVars) {
		return (NULL);
	}
	V = &obj->vars[i];
	AG_LockVariable(V);
	return (V);
}

#define AG_VARIABLE_TYPE(V) (agVariableTypes[(V)->type].typeTgt)
#define AG_VARIABLE_TYPE_NAME(V) (agVariableTypes[(V)->type].name)
__END_DECLS

#include <agar/core/close.h>
