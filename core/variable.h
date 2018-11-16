/*	Public domain	*/

#include <agar/core/begin.h>

#ifndef AG_VARIABLE_NAME_MAX
# if AG_MODEL == AG_SMALL
#  define AG_VARIABLE_NAME_MAX 16
# else
#  define AG_VARIABLE_NAME_MAX 24
# endif
#endif

typedef enum ag_variable_type {
	AG_VARIABLE_NULL,		/* No data */

	AG_VARIABLE_UINT,		/* Unsigned int */
	AG_VARIABLE_P_UINT,		/* Pointer to Uint */
	AG_VARIABLE_INT,		/* Natural int */
	AG_VARIABLE_P_INT,		/* Pointer to int */
	AG_VARIABLE_ULONG,		/* Natural unsigned long integer */
	AG_VARIABLE_P_ULONG,		/* Pointer to unsigned long */
	AG_VARIABLE_LONG,		/* Natural long integer */
	AG_VARIABLE_P_LONG,		/* Pointer to long */

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
	AG_VARIABLE_UINT64,		/* Unsigned 64-bit */
	AG_VARIABLE_P_UINT64,		/* Pointer to Uint64 */
	AG_VARIABLE_SINT64,		/* Signed 64-bit */
	AG_VARIABLE_P_SINT64,		/* Pointer to Sint64 */

	AG_VARIABLE_FLOAT,		/* Single-precision real number */
	AG_VARIABLE_P_FLOAT,		/* Pointer to single-precision real */
	AG_VARIABLE_DOUBLE,		/* Double-precision real number */
	AG_VARIABLE_P_DOUBLE,		/* Pointer to double-precision real */
	AG_VARIABLE_LONG_DOUBLE,	/* Quad-precision float */
	AG_VARIABLE_P_LONG_DOUBLE,	/* Pointer to quad-precision real */

	AG_VARIABLE_STRING,		/* C string */
	AG_VARIABLE_P_STRING,		/* Pointer to C string */
	AG_VARIABLE_POINTER,		/* Generic C pointer */
	AG_VARIABLE_P_POINTER,		/* Reference to a generic pointer */
	AG_VARIABLE_P_FLAG,		/* Bit(s) in an int (per given mask) */
	AG_VARIABLE_P_FLAG8,		/* Bit(s) in an int8 (per given mask) */
	AG_VARIABLE_P_FLAG16,		/* Bit(s) in an int16 (per given mask) */
	AG_VARIABLE_P_FLAG32,		/* Bit(s) in an int32 (per given mask) */
	AG_VARIABLE_P_OBJECT,		/* Serializable reference to an Object
					   (and hard dependency) */
	AG_VARIABLE_P_VARIABLE,		/* Serializable reference to specific
					   Object Variable (by name) */
	AG_VARIABLE_TYPE_LAST
} AG_VariableType;

#ifdef AG_LEGACY
#define AG_VARIABLE_CONST_STRING	AG_VARIABLE_STRING
#define AG_VARIABLE_P_CONST_STRING	AG_VARIABLE_P_STRING
#define AG_VARIABLE_CONST_POINTER	AG_VARIABLE_POINTER
#define AG_VARIABLE_P_CONST_POINTER	AG_VARIABLE_P_POINTER
#define AG_SetConstString(o,n,v)	AG_SetString((o),(n),(char *)(v))
#define AG_BindConstString		AG_BindString
#define AG_BindConstStringMp		AG_BindStringMp
#define AG_BindConstStringFn		AG_BindStringFn
#define AG_GetConstPointer(o,x)		((const void *)AG_GetPointer((o),(x)))
#define AG_SetConstPointer(o,n,v)	AG_SetPointer((o),(n),(void *)(v))
#define AG_InitConstPointer(var,v)	AG_InitPointer((var),(void *)(v))
#define AG_BindConstPointer		AG_BindPointer
#define AG_BindConstPointerFn		AG_BindPointerFn
#define AG_BindConstPointerMp		AG_BindPointerMp
#endif /* AG_LEGACY */

#define AG_VARIABLE_BOOL AG_VARIABLE_INT

typedef struct ag_variable_type_info {
	AG_VariableType type;		/* Variable type */
	int indirLvl;			/* Indirection level */
	const char *_Nonnull name;	/* Name string */
	AG_VariableType typeTgt;	/* Pointer target type (or AG_VARIABLE_NULL) */
	Sint32 code;			/* Numerical code (-1 = non persistent) */
	AG_Size size;			/* Size in bytes (or 0) */
} AG_VariableTypeInfo;

struct ag_event;

typedef void            (*AG_VoidFn)(struct ag_event *_Nonnull);
typedef Uint            (*AG_UintFn)(struct ag_event *_Nonnull);
typedef int             (*AG_IntFn)(struct ag_event *_Nonnull);
typedef Ulong           (*AG_UlongFn)(struct ag_event *_Nonnull);
typedef long            (*AG_LongFn)(struct ag_event *_Nonnull);
typedef Uint8           (*AG_Uint8Fn)(struct ag_event *_Nonnull);
typedef Sint8           (*AG_Sint8Fn)(struct ag_event *_Nonnull);
typedef Uint16          (*AG_Uint16Fn)(struct ag_event *_Nonnull);
typedef Sint16          (*AG_Sint16Fn)(struct ag_event *_Nonnull);
typedef Uint32          (*AG_Uint32Fn)(struct ag_event *_Nonnull);
typedef Sint32          (*AG_Sint32Fn)(struct ag_event *_Nonnull);
#ifdef AG_HAVE_64BIT
typedef Uint64          (*AG_Uint64Fn)(struct ag_event *_Nonnull);
typedef Sint64          (*AG_Sint64Fn)(struct ag_event *_Nonnull);
#endif
#ifdef AG_HAVE_FLOAT
typedef float           (*AG_FloatFn)(struct ag_event *_Nonnull);
typedef double          (*AG_DoubleFn)(struct ag_event *_Nonnull);
# ifdef AG_HAVE_LONG_DOUBLE
typedef long double     (*AG_LongDoubleFn)(struct ag_event *_Nonnull);
# endif
#endif
typedef AG_Size          (*AG_StringFn)(struct ag_event *_Nonnull,
                                        char *_Nonnull, AG_Size);
typedef void *_Nullable (*AG_PointerFn)(struct ag_event *_Nonnull);

union ag_function {
	_Nullable AG_VoidFn   fnVoid;
	_Nullable AG_UintFn   fnUint;
	_Nullable AG_IntFn    fnInt;
	_Nullable AG_UlongFn  fnUlong;
	_Nullable AG_LongFn   fnLong;
	_Nullable AG_Uint8Fn  fnUint8;
	_Nullable AG_Sint8Fn  fnSint8;
	_Nullable AG_Uint16Fn fnUint16;
	_Nullable AG_Sint16Fn fnSint16;
	_Nullable AG_Uint32Fn fnUint32;
	_Nullable AG_Sint32Fn fnSint32;
#ifdef AG_HAVE_64BIT
	_Nullable AG_Uint64Fn fnUint64;
	_Nullable AG_Sint64Fn fnSint64;
#endif
#ifdef AG_HAVE_FLOAT
	_Nullable AG_FloatFn  fnFloat;
	_Nullable AG_DoubleFn fnDouble;
# ifdef AG_HAVE_LONG_DOUBLE
	_Nullable AG_LongDoubleFn fnLongDouble;
# endif
#endif
	_Nullable AG_StringFn  fnString;
	_Nullable AG_PointerFn fnPointer;
};
	
union ag_variable_data {
	void       *_Nullable p;
	const void *_Nullable Cp;
	char       *_Nullable s;
	const char *_Nullable Cs;
	int            i;
	Uint           u;
	long          li;
	Ulong        uli;
	Uint8         u8;
	Sint8         s8;
	Uint16       u16;
	Sint16       s16;
	Uint32       u32;
	Sint32       s32;
#ifdef AG_HAVE_64BIT
	Uint64       u64;
	Sint64       s64;
#endif
#ifdef AG_HAVE_FLOAT
	float        flt;
	double       dbl;
# ifdef AG_HAVE_LONG_DOUBLE
	long double ldbl;
# endif
#endif
};

typedef struct ag_variable {
	char name[AG_VARIABLE_NAME_MAX];/* Variable name */
	AG_VariableType type;	 	/* Variable type */
	_Nullable AG_Mutex *_Nullable mutex; /* Lock protecting the data */
	union {
		Uint32 bitmask;		 /* Bitmask (for P_FLAG_*) */
		AG_Size size;		 /* Length / Buffer size (for STRING_*) */
		char *_Nullable varName; /* Variable name (for P_VARIABLE) */
		char *_Nullable objName; /* Unresolved path (for P_OBJECT) */
	} info;
	union ag_function fn;		/* Eval function */
	union ag_variable_data data;	/* Variable-stored data */
	AG_TAILQ_ENTRY(ag_variable) vars;
} AG_Variable;

__BEGIN_DECLS
extern const AG_VariableTypeInfo agVariableTypes[];

int  AG_EvalVariable(void *_Nonnull, AG_Variable *_Nonnull);
void AG_PrintVariable(char *_Nonnull, AG_Size, AG_Variable *_Nonnull);

AG_Variable *_Nullable AG_GetVariableVFS(void *_Nonnull, const char *_Nonnull)
                                        _Warn_Unused_Result;
AG_Variable *_Nullable AG_GetVariable(void *_Nonnull, const char *_Nonnull, ...)
                                     _Warn_Unused_Result;

int AG_CopyVariable(AG_Variable       *_Nonnull _Restrict,
                    const AG_Variable *_Nonnull _Restrict);
int AG_DerefVariable(AG_Variable       *_Nonnull _Restrict,
                     const AG_Variable *_Nonnull _Restrict);
int AG_CompareVariables(const AG_Variable *_Nonnull,
                        const AG_Variable *_Nonnull)
                       _Pure_Attribute;

void AG_Unset(void *_Nonnull, const char *_Nonnull);

void AG_VariableSubst(void *_Nonnull, const char *_Nonnull, char *_Nonnull,
                      AG_Size);
/* Uint */
Uint                  AG_GetUint(void *_Nonnull, const char *_Nonnull);
void                  AG_InitUint(AG_Variable *_Nonnull, Uint);
AG_Variable *_Nonnull AG_SetUint(void *_Nonnull, const char *_Nonnull, Uint);
AG_Variable *_Nonnull AG_BindUint(void *_Nonnull, const char *_Nonnull,
                                  Uint *_Nonnull);
AG_Variable *_Nonnull AG_BindUintFn(void *_Nonnull, const char *_Nonnull,
                                    _Nonnull AG_UintFn,
				    const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindUintMp(void *_Nonnull, const char *_Nonnull,
				    Uint *_Nonnull, _Nonnull AG_Mutex *_Nonnull);
/* int */
int                   AG_GetInt(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetInt(void *_Nonnull, const char *_Nonnull, int);
void                  AG_InitInt(AG_Variable *_Nonnull, int);
AG_Variable *_Nonnull AG_BindInt(void *_Nonnull, const char *_Nonnull,
			         int *_Nonnull);
AG_Variable *_Nonnull AG_BindIntFn(void *_Nonnull, const char *_Nonnull,
				   _Nonnull AG_IntFn,
				   const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindIntMp(void *_Nonnull, const char *_Nonnull,
                                   int *_Nonnull,
				   _Nonnull AG_Mutex *_Nonnull);
/* Boolean */
#define AG_GetBool    AG_GetInt
#define AG_SetBool    AG_SetInt
#define AG_BindBool   AG_BindInt
#define AG_BindBoolFn AG_BindIntFn
#define AG_BindBoolMp AG_BindIntMp

/* Ulong */
Ulong                 AG_GetUlong(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetUlong(void *_Nonnull, const char *_Nonnull, Ulong);
void                  AG_InitUlong(AG_Variable *_Nonnull, Ulong);
AG_Variable *_Nonnull AG_BindUlong(void *_Nonnull, const char *_Nonnull,
			           Ulong *_Nonnull);
AG_Variable *_Nonnull AG_BindUlongFn(void *_Nonnull, const char *_Nonnull,
                                     _Nonnull AG_UlongFn,
                                     const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindUlongMp(void *_Nonnull, const char *_Nonnull,
                                     Ulong *_Nonnull,
				     _Nonnull AG_Mutex *_Nonnull);
/* long */
long                  AG_GetLong(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetLong(void *_Nonnull, const char *_Nonnull, long);
void                  AG_InitLong(AG_Variable *_Nonnull, long);
AG_Variable *_Nonnull AG_BindLong(void *_Nonnull, const char *_Nonnull,
			          long *_Nonnull);
AG_Variable *_Nonnull AG_BindLongFn(void *_Nonnull, const char *_Nonnull,
				    _Nonnull AG_LongFn,
                                    const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindLongMp(void *_Nonnull, const char *_Nonnull,
                                    long *_Nonnull,
                                    _Nonnull AG_Mutex *_Nonnull);
/* Uint8 */
Uint8                 AG_GetUint8(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetUint8(void *_Nonnull, const char *_Nonnull, Uint8);
void                  AG_InitUint8(AG_Variable *_Nonnull, Uint8);
AG_Variable *_Nonnull AG_BindUint8(void *_Nonnull, const char *_Nonnull,
                                   Uint8 *_Nonnull);
AG_Variable *_Nonnull AG_BindUint8Fn(void *_Nonnull, const char *_Nonnull,
				     _Nonnull AG_Uint8Fn,
                                     const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindUint8Mp(void *_Nonnull, const char *_Nonnull,
                                     Uint8 *_Nonnull,
                                    _Nonnull AG_Mutex *_Nonnull);
/* Sint8 */
Sint8                 AG_GetSint8(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetSint8(void *_Nonnull, const char *_Nonnull, Sint8);
void                  AG_InitSint8(AG_Variable *_Nonnull, Sint8);
AG_Variable *_Nonnull AG_BindSint8(void *_Nonnull, const char *_Nonnull,
			           Sint8 *_Nonnull);
AG_Variable *_Nonnull AG_BindSint8Fn(void *_Nonnull, const char *_Nonnull,
				     _Nonnull AG_Sint8Fn,
                                     const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindSint8Mp(void *_Nonnull, const char *_Nonnull,
                                     Sint8 *_Nonnull,
                                    _Nonnull AG_Mutex *_Nonnull);
/* Uint16 */
Uint16                AG_GetUint16(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetUint16(void *_Nonnull, const char *_Nonnull, Uint16);
void                  AG_InitUint16(AG_Variable *_Nonnull, Uint16);
AG_Variable *_Nonnull AG_BindUint16(void *_Nonnull, const char *_Nonnull,
			            Uint16 *_Nonnull);
AG_Variable *_Nonnull AG_BindUint16Fn(void *_Nonnull, const char *_Nonnull,
				      _Nonnull AG_Uint16Fn,
                                      const char *_Nullable , ...);
AG_Variable *_Nonnull AG_BindUint16Mp(void *_Nonnull, const char *_Nonnull,
                                      Uint16 *_Nonnull,
                                     _Nonnull AG_Mutex *_Nonnull);
/* Sint16 */
Sint16                AG_GetSint16(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetSint16(void *_Nonnull, const char *_Nonnull, Sint16);
void                  AG_InitSint16(AG_Variable *_Nonnull, Sint16);
AG_Variable *_Nonnull AG_BindSint16(void *_Nonnull, const char *_Nonnull,
			            Sint16 *_Nonnull);
AG_Variable *_Nonnull AG_BindSint16Fn(void *_Nonnull, const char *_Nonnull,
				      _Nonnull AG_Sint16Fn,
                                      const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindSint16Mp(void *_Nonnull, const char *_Nonnull,
                                      Sint16 *_Nonnull,
                                     _Nonnull AG_Mutex *_Nonnull);
/* Uint32 */
Uint32                AG_GetUint32(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetUint32(void *_Nonnull, const char *_Nonnull, Uint32);
void                  AG_InitUint32(AG_Variable *_Nonnull, Uint32);
AG_Variable *_Nonnull AG_BindUint32(void *_Nonnull, const char *_Nonnull,
			            Uint32 *_Nonnull);
AG_Variable *_Nonnull AG_BindUint32Fn(void *_Nonnull, const char *_Nonnull,
				      _Nonnull AG_Uint32Fn,
                                      const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindUint32Mp(void *_Nonnull, const char *_Nonnull,
                                      Uint32 *_Nonnull,
                                     _Nonnull AG_Mutex *_Nonnull);
/* Sint32 */
Sint32                AG_GetSint32(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetSint32(void *_Nonnull, const char *_Nonnull, Sint32);
void                  AG_InitSint32(AG_Variable *_Nonnull, Sint32);
AG_Variable *_Nonnull AG_BindSint32(void *_Nonnull, const char *_Nonnull,
			            Sint32 *_Nonnull);
AG_Variable *_Nonnull AG_BindSint32Fn(void *_Nonnull, const char *_Nonnull,
				      _Nonnull AG_Sint32Fn,
                                      const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindSint32Mp(void *_Nonnull, const char *_Nonnull,
                                      Sint32 *_Nonnull,
                                     _Nonnull AG_Mutex *_Nonnull);

#ifdef AG_HAVE_64BIT
/* Uint64 */
Uint64                AG_GetUint64(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetUint64(void *_Nonnull, const char *_Nonnull, Uint64);
void                  AG_InitUint64(AG_Variable *_Nonnull, Uint64);
AG_Variable *_Nonnull AG_BindUint64(void *_Nonnull, const char *_Nonnull,
			            Uint64 *_Nonnull);
AG_Variable *_Nonnull AG_BindUint64Fn(void *_Nonnull, const char *_Nonnull,
				      _Nonnull AG_Uint64Fn,
                                       const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindUint64Mp(void *_Nonnull, const char *_Nonnull,
                                      Uint64 *_Nonnull,
                                     _Nonnull AG_Mutex *_Nonnull);
/* Sint64 */
Sint64                AG_GetSint64(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetSint64(void *_Nonnull, const char *_Nonnull, Sint64);
void                  AG_InitSint64(AG_Variable *_Nonnull, Sint64);
AG_Variable *_Nonnull AG_BindSint64(void *_Nonnull, const char *_Nonnull,
			            Sint64 *_Nonnull);
AG_Variable *_Nonnull AG_BindSint64Fn(void *_Nonnull, const char *_Nonnull,
				      _Nonnull AG_Sint64Fn,
                                      const char *_Nullable , ...);
AG_Variable *_Nonnull AG_BindSint64Mp(void *_Nonnull, const char *_Nonnull,
                                      Sint64 *_Nonnull,
                                     _Nonnull AG_Mutex *_Nonnull);
#endif /* HAVE_64BIT */

#ifdef AG_HAVE_FLOAT
/* float */
float                 AG_GetFloat(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetFloat(void *_Nonnull, const char *_Nonnull, float);
void                  AG_InitFloat(AG_Variable *_Nonnull, float);
AG_Variable *_Nonnull AG_BindFloat(void *_Nonnull, const char *_Nonnull,
			           float *_Nonnull);
AG_Variable *_Nonnull AG_BindFloatFn(void *_Nonnull , const char *_Nonnull ,
				     _Nonnull AG_FloatFn,
                                     const char *_Nullable , ...);
AG_Variable *_Nonnull AG_BindFloatMp(void *_Nonnull, const char *_Nonnull,
                                     float *_Nonnull,
                                    _Nonnull AG_Mutex *_Nonnull);
/* double */
double                AG_GetDouble(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetDouble(void *_Nonnull, const char *_Nonnull, double);
void                  AG_InitDouble(AG_Variable *_Nonnull, double);
AG_Variable *_Nonnull AG_BindDouble(void *_Nonnull, const char *_Nonnull,
                                    double *_Nonnull);
AG_Variable *_Nonnull AG_BindDoubleFn(void *_Nonnull, const char *_Nonnull ,
				      _Nonnull AG_DoubleFn,
                                      const char *_Nullable , ...);
AG_Variable *_Nonnull AG_BindDoubleMp(void *_Nonnull, const char *_Nonnull,
                                      double *_Nonnull,
                                     _Nonnull AG_Mutex *_Nonnull);
/* long double */
# ifdef AG_HAVE_LONG_DOUBLE
long double           AG_GetLongDouble(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetLongDouble(void *_Nonnull, const char *_Nonnull,
                                       long double);
void                  AG_InitLongDouble(AG_Variable *_Nonnull, long double);
AG_Variable *_Nonnull AG_BindLongDouble(void *_Nonnull, const char  *_Nonnull,
			                long double *_Nonnull);
AG_Variable *_Nonnull AG_BindLongDoubleFn(void *_Nonnull, const char *_Nonnull,
				          _Nonnull AG_LongDoubleFn,
                                          const char *_Nullable , ...);
AG_Variable *_Nonnull AG_BindLongDoubleMp(void *_Nonnull, const char *_Nonnull,
                                          long double *_Nonnull,
                                          _Nonnull AG_Mutex *_Nonnull);
# endif /* HAVE_LONG_DOUBLE */

#endif /* HAVE_FLOAT */

/*
 * Strings
 */
AG_Size AG_GetString(void *_Nonnull, const char *_Nonnull, char *_Nonnull,
                     AG_Size);

char *_Nullable AG_GetStringDup(void *_Nonnull, const char *_Nonnull);
char *_Nullable AG_GetStringP(void *_Nonnull, const char *_Nonnull);

AG_Variable *_Nonnull AG_SetString(void *_Nonnull, const char *_Nonnull,
                                   const char *_Nonnull);
AG_Variable *_Nonnull AG_SetStringNODUP(void *_Nonnull, const char *_Nonnull,
                                        char *_Nonnull);
void                  AG_InitString(AG_Variable *_Nonnull, const char *_Nonnull);
void                  AG_InitStringNODUP(AG_Variable *_Nonnull, char *_Nonnull);

AG_Variable *_Nonnull AG_PrtString(void *_Nonnull, const char *_Nonnull,
                                   const char *_Nonnull, ...)
		                  FORMAT_ATTRIBUTE(printf,3,4);

AG_Variable *_Nonnull AG_BindString(void *_Nonnull, const char *_Nonnull,
                                    char *_Nonnull, AG_Size);

AG_Variable *_Nonnull AG_BindStringFn(void *_Nonnull, const char *_Nonnull,
                                      _Nonnull AG_StringFn,
				      const char *_Nullable, ...);

AG_Variable *_Nonnull AG_BindStringMp(void *_Nonnull, const char *_Nonnull,
                                      char *_Nonnull, AG_Size,
                                     _Nonnull AG_Mutex *_Nonnull);

/* Pointers */
void *_Nullable       AG_GetPointer(void *_Nonnull, const char *_Nonnull);
AG_Variable *_Nonnull AG_SetPointer(void *_Nonnull, const char *_Nonnull,
                                    void *_Nullable);
void                  AG_InitPointer(AG_Variable *_Nonnull, void *_Nullable);
AG_Variable *_Nonnull AG_BindPointer(void *_Nonnull, const char *_Nonnull,
                                     void *_Nonnull *_Nullable);
AG_Variable *_Nonnull AG_BindPointerFn(void *_Nonnull, const char *_Nonnull,
                                       _Nonnull AG_PointerFn,
				       const char *_Nullable, ...);
AG_Variable *_Nonnull AG_BindPointerMp(void *_Nonnull, const char *_Nonnull,
                                       void *_Nonnull *_Nullable,
				       _Nonnull AG_Mutex *_Nonnull);

/* Bits */
AG_Variable *_Nonnull AG_BindFlag(void *_Nonnull, const char *_Nonnull,
                                  Uint *_Nonnull, Uint);
AG_Variable *_Nonnull AG_BindFlagMp(void *_Nonnull, const char *_Nonnull,
				    Uint *_Nonnull, Uint,
				    _Nonnull AG_Mutex *_Nonnull);
AG_Variable *_Nonnull AG_BindFlag8(void *_Nonnull, const char *_Nonnull,
				   Uint8 *_Nonnull, Uint8);
AG_Variable *_Nonnull AG_BindFlag8Mp(void *_Nonnull, const char *_Nonnull,
				     Uint8 *_Nonnull, Uint8,
				     _Nonnull AG_Mutex *_Nonnull);
AG_Variable *_Nonnull AG_BindFlag16(void *_Nonnull, const char *_Nonnull,
				    Uint16 *_Nonnull, Uint16);
AG_Variable *_Nonnull AG_BindFlag16Mp(void *_Nonnull, const char *_Nonnull,
                                      Uint16 *_Nonnull, Uint16,
                                      _Nonnull AG_Mutex *_Nonnull);
AG_Variable *_Nonnull AG_BindFlag32(void *_Nonnull, const char *_Nonnull,
				    Uint32 *_Nonnull, Uint32);
AG_Variable *_Nonnull AG_BindFlag32Mp(void *_Nonnull, const char *_Nonnull,
				      Uint32 *_Nonnull, Uint32,
				      _Nonnull AG_Mutex *_Nonnull);

AG_Variable *_Nonnull AG_BindObject(void *_Nonnull, const char *_Nonnull,
				    void *_Nonnull);
AG_Variable *_Nonnull AG_BindVariable(void *_Nonnull, const char *_Nonnull,
				      void *_Nonnull, const char *_Nonnull);

/* Initialize an AG_Variable structure. */
static __inline__ void
AG_InitVariable(AG_Variable *_Nonnull V, AG_VariableType type,
    const char *_Nonnull name)
{
#ifdef AG_DEBUG
	memset(V->name, '\0', sizeof(V->name));
#endif
	if (name[0] != '\0') {
		AG_Strlcpy(V->name, name, sizeof(V->name));
	} else {
		V->name[0] = '\0';
	}
	V->type = type;
	V->mutex = NULL;
	V->fn.fnVoid = NULL;
	V->info.size = 0;
	V->info.varName = NULL;
	V->data.s = NULL;
}

/* Acquire any locking device associated with a variable. */
static __inline__ void
AG_LockVariable(AG_Variable *_Nonnull V)
{
#ifdef AG_THREADS
	if (V->mutex != NULL) { AG_MutexLock(V->mutex); }
#else
# ifdef __CC65__
	if (V != NULL) { /* Unused */ }
# endif
#endif
}

/* Release any locking device associated with a variable. */
static __inline__ void
AG_UnlockVariable(AG_Variable *_Nonnull V)
{
#ifdef AG_THREADS
	if (V->mutex != NULL) { AG_MutexUnlock(V->mutex); }
#else
# ifdef __CC65__
	if (V != NULL) { /* Unused */ }
# endif
#endif
}

/* Release all resources associated with a variable. */
static __inline__ void
AG_FreeVariable(AG_Variable *_Nonnull V)
{
	switch (V->type) {
	case AG_VARIABLE_STRING:
		if (V->info.size == 0) {
			AG_Free(V->data.s);
		}
		break;
	case AG_VARIABLE_P_VARIABLE:
		AG_Free(V->info.varName);
		break;
	default:
		break;
	}
}

#define AG_VARIABLE_TYPE(V) (agVariableTypes[(V)->type].typeTgt)
#define AG_VARIABLE_TYPE_NAME(V) (agVariableTypes[(V)->type].name)
__END_DECLS

#include <agar/core/close.h>
