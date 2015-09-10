/*	Public domain	*/

#include <agar/core/begin.h>

typedef enum ag_variable_type {
	AG_VARIABLE_NULL,		/* No data */
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
	AG_VARIABLE_P_FLAG,		/* Bit in int (uses info.mask) */
	AG_VARIABLE_P_FLAG8,		/* Bit in int8 (uses info.mask) */
	AG_VARIABLE_P_FLAG16,		/* Bit in int16 (uses info.mask) */
	AG_VARIABLE_P_FLAG32,		/* Bit in int32 (uses info.mask) */
	AG_VARIABLE_P_OBJECT,		/* Pointer to AG_Object(3) */
	AG_VARIABLE_P_TEXT,		/* Pointer to AG_Text(3) */
	AG_VARIABLE_P_VARIABLE,		/* Reference to an AG_Variable(3) */
	AG_VARIABLE_TYPE_LAST
} AG_VariableType;

typedef struct ag_variable_type_info {
	enum ag_variable_type type;		/* Variable type */
	int indirLvl;				/* Indirection level */
	const char *name;			/* Name string */
	enum ag_variable_type typeTgt;		/* Pointer target type (or AG_VARIABLE_NULL) */
	Sint32 code;				/* Numerical code (-1 = non persistent) */
	size_t size;				/* Size in bytes (or 0) */
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
#ifdef AG_HAVE_64BIT
typedef Uint64      (*AG_Uint64Fn)(struct ag_event *);
typedef Sint64      (*AG_Sint64Fn)(struct ag_event *);
#endif
typedef float       (*AG_FloatFn)(struct ag_event *);
typedef double      (*AG_DoubleFn)(struct ag_event *);
#ifdef AG_HAVE_LONG_DOUBLE
typedef long double (*AG_LongDoubleFn)(struct ag_event *);
#endif
typedef size_t      (*AG_StringFn)(struct ag_event *, char *, size_t);
typedef void       *(*AG_PointerFn)(struct ag_event *);
typedef const void *(*AG_ConstPointerFn)(struct ag_event *);
typedef AG_Text    *(*AG_TextFn)(struct ag_event *);

union ag_function {
	AG_VoidFn	fnVoid;
	AG_UintFn	fnUint;
	AG_IntFn	fnInt;
	AG_Uint8Fn	fnUint8;
	AG_Sint8Fn	fnSint8;
	AG_Uint16Fn	fnUint16;
	AG_Sint16Fn	fnSint16;
	AG_Uint32Fn	fnUint32;
	AG_Sint32Fn	fnSint32;
#ifdef AG_HAVE_64BIT
	AG_Uint64Fn	fnUint64;
	AG_Sint64Fn	fnSint64;
#endif
	AG_FloatFn	fnFloat;
	AG_DoubleFn	fnDouble;
#ifdef AG_HAVE_LONG_DOUBLE
	AG_LongDoubleFn	fnLongDouble;
#endif
	AG_StringFn	fnString;
	AG_PointerFn	fnPointer;
	AG_ConstPointerFn fnConstPointer;
	AG_TextFn	fnText;
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
#ifdef AG_HAVE_LONG_DOUBLE
	long double ldbl;
#endif
	Uint8 u8;
	Sint8 s8;
	Uint16 u16;
	Sint16 s16;
	Uint32 u32;
	Sint32 s32;
#ifdef AG_HAVE_64BIT
	Uint64 u64;
	Sint64 s64;
#endif
};

typedef struct ag_variable {
	char name[AG_VARIABLE_NAME_MAX]; /* Variable name */
	AG_VariableType type;	 	 /* Variable type */
	AG_Mutex *mutex;		 /* Lock protecting data (or NULL) */
	union {
		Uint32 bitmask;		/* Bitmask (P_FLAG_*) */
		size_t size;		/* Length / Buffer size (STRING_*) */
		struct {		/* For P_VARIABLE type */
			char *key;
			struct ag_variable *var;
		} ref;
	} info;
	union ag_function fn;		/* Eval function */
	union ag_variable_data data;	/* Variable-stored data */
	AG_TAILQ_ENTRY(ag_variable) vars;
} AG_Variable;

__BEGIN_DECLS
struct ag_list;
extern const AG_VariableTypeInfo agVariableTypes[];

int          AG_EvalVariable(void *, AG_Variable *);
void         AG_PrintVariable(char *, size_t, AG_Variable *);
AG_Variable *AG_GetVariableVFS(void *, const char *)
                               WARN_UNUSED_RESULT_ATTRIBUTE;
AG_Variable *AG_GetVariable(void *, const char *, ...)
                            WARN_UNUSED_RESULT_ATTRIBUTE;
int          AG_CopyVariable(AG_Variable *, const AG_Variable *);
int          AG_DerefVariable(AG_Variable *, const AG_Variable *);
int          AG_CompareVariables(const AG_Variable *, const AG_Variable *);
void         AG_Unset(void *, const char *);
void         AG_VariableSubst(void *, const char *, char *, size_t)
                              BOUNDED_ATTRIBUTE(__string__, 3, 4);

struct ag_list *AG_ListSet(const char *, ...);

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

#ifdef AG_HAVE_64BIT
Uint64       AG_GetUint64(void *, const char *);
AG_Variable *AG_SetUint64(void *, const char *, Uint64);
void         AG_InitUint64(AG_Variable *, Uint64);
AG_Variable *AG_BindUint64Fn(void *, const char *, AG_Uint64Fn, const char *, ...);
AG_Variable *AG_BindUint64(void *, const char *, Uint64 *);
AG_Variable *AG_BindUint64Mp(void *, const char *, Uint64 *, AG_Mutex *);
Sint64       AG_GetSint64(void *, const char *);
AG_Variable *AG_SetSint64(void *, const char *, Sint64);
void         AG_InitSint64(AG_Variable *, Sint64);
AG_Variable *AG_BindSint64Fn(void *, const char *, AG_Sint64Fn, const char *, ...);
AG_Variable *AG_BindSint64(void *, const char *, Sint64 *);
AG_Variable *AG_BindSint64Mp(void *, const char *, Sint64 *, AG_Mutex *);
#endif /* AG_HAVE_64BIT */

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
#ifdef AG_HAVE_LONG_DOUBLE
long double  AG_GetLongDouble(void *, const char *);
AG_Variable *AG_SetLongDouble(void *, const char *, long double);
void         AG_InitLongDouble(AG_Variable *, long double);
AG_Variable *AG_BindLongDoubleFn(void *, const char *, AG_LongDoubleFn, const char *, ...);
AG_Variable *AG_BindLongDouble(void *, const char *, long double *);
AG_Variable *AG_BindLongDoubleMp(void *, const char *, long double *, AG_Mutex *);
#endif

size_t       AG_GetString(void *, const char *, char *, size_t)
	         BOUNDED_ATTRIBUTE(__string__, 3, 4);
char        *AG_GetStringDup(void *, const char *);
char        *AG_GetStringP(void *, const char *);
AG_Variable *AG_SetString(void *, const char *, const char *);
AG_Variable *AG_SetStringNODUP(void *, const char *, char *);
void         AG_InitString(AG_Variable *, const char *);
void         AG_InitStringNODUP(AG_Variable *, char *);
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

AG_Text     *AG_GetText(void *, const char *);
AG_Variable *AG_SetText(void *, const char *, AG_Text *);
void         AG_InitText(AG_Variable *, AG_Text *);
AG_Variable *AG_BindText(void *, const char *, AG_Text *);
AG_Variable *AG_BindTextFn(void *, const char *, AG_TextFn, const char *, ...);
AG_Variable *AG_BindTextMp(void *, const char *, AG_Text *, AG_Mutex *);

AG_Variable *AG_BindFlag(void *, const char *, Uint *, Uint);
AG_Variable *AG_BindFlagMp(void *, const char *, Uint *, Uint, AG_Mutex *);
AG_Variable *AG_BindFlag8(void *, const char *, Uint8 *, Uint8);
AG_Variable *AG_BindFlag8Mp(void *, const char *, Uint8 *, Uint8, AG_Mutex *);
AG_Variable *AG_BindFlag16(void *, const char *, Uint16 *, Uint16);
AG_Variable *AG_BindFlag16Mp(void *, const char *, Uint16 *, Uint16, AG_Mutex *);
AG_Variable *AG_BindFlag32(void *, const char *, Uint32 *, Uint32);
AG_Variable *AG_BindFlag32Mp(void *, const char *, Uint32 *, Uint32, AG_Mutex *);

AG_Variable *AG_BindVariable(void *, const char *, void *, const char *);

/* Initialize an AG_Variable structure. */
static __inline__ void
AG_InitVariable(AG_Variable *V, enum ag_variable_type type)
{
	V->type = type;
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	V->info.size = 0;
	V->info.ref.key = NULL;
	V->info.ref.var = NULL;
	V->data.s = NULL;
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
	switch (V->type) {
	case AG_VARIABLE_STRING:
		if (V->info.size == 0) {
			AG_Free(V->data.s);
		}
		break;
	case AG_VARIABLE_P_VARIABLE:
		AG_Free(V->info.ref.key);
		break;
	default:
		break;
	}
}

#define AG_VARIABLE_TYPE(V) (agVariableTypes[(V)->type].typeTgt)
#define AG_VARIABLE_TYPE_NAME(V) (agVariableTypes[(V)->type].name)
__END_DECLS

#include <agar/core/close.h>
