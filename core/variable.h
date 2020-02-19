/*	Public domain	*/

#ifndef _AGAR_CORE_OBJECT_H_
# error "Must be included by object.h"
#endif

#ifndef AG_VARIABLE_NAME_MAX
# if AG_MODEL == AG_SMALL
#  define AG_VARIABLE_NAME_MAX 8
# else
#  define AG_VARIABLE_NAME_MAX 36
# endif
#endif

typedef enum ag_variable_type {
	AG_VARIABLE_NULL,		/* No data */

	AG_VARIABLE_UINT,		/* Unsigned int */
	AG_VARIABLE_P_UINT,		/* Pointer to Uint */
	AG_VARIABLE_INT,		/* Natural int */
	AG_VARIABLE_P_INT,		/* Pointer to int */
#if AG_MODEL != AG_SMALL
	AG_VARIABLE_ULONG,		/* Natural unsigned long integer */
	AG_VARIABLE_P_ULONG,		/* Pointer to unsigned long */
	AG_VARIABLE_LONG,		/* Natural long integer */
	AG_VARIABLE_P_LONG,		/* Pointer to long */
#endif
	AG_VARIABLE_UINT8,		/* Unsigned 8-bit */
	AG_VARIABLE_P_UINT8,		/* Pointer to Uint8 */
	AG_VARIABLE_SINT8,		/* Signed 8-bit */
	AG_VARIABLE_P_SINT8,		/* Pointer to Sint8 */
#if AG_MODEL != AG_SMALL
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
#endif /* !AG_SMALL */
	AG_VARIABLE_STRING,		/* C string */
	AG_VARIABLE_P_STRING,		/* Pointer to C string */
	AG_VARIABLE_POINTER,		/* Generic C pointer */
	AG_VARIABLE_P_POINTER,		/* Reference to a generic pointer */
	AG_VARIABLE_P_FLAG,		/* Bit(s) in an int (per given mask) */
	AG_VARIABLE_P_FLAG8,		/* Bit(s) in an int8 (per given mask) */
#if AG_MODEL != AG_SMALL
	AG_VARIABLE_P_FLAG16,		/* Bit(s) in an int16 (per given mask) */
	AG_VARIABLE_P_FLAG32,		/* Bit(s) in an int32 (per given mask) */
#endif
	AG_VARIABLE_P_OBJECT,		/* Serializable reference to an Object
					   (and hard dependency) */
	AG_VARIABLE_P_VARIABLE,		/* Serializable reference to specific
					   Object Variable (by name) */
	AG_VARIABLE_TYPE_LAST
} AG_VariableType;
#if AG_MODEL == AG_SMALL
#define AG_VariableType Uint8
#endif

#define AG_VARIABLE_BOOL AG_VARIABLE_INT

/* Information about an AG_Variable type */
typedef struct ag_variable_type_info {
	AG_VariableType type;      /* Variable type */
#if AG_MODEL == AG_SMALL
	Uint8 indirLvl;            /* Level of indirection (0 = none) */
	const char *_Nonnull name; /* Name string */
	AG_VariableType typeTgt;   /* Pointer target type (if indirLvl > 0) */
	Sint8 code;                /* Numerical code (-1 = not serializable) */
	Uint8 size;                /* Size in bytes */
#else
	int indirLvl;              /* Level of indirection (0 = none) */
	const char *_Nonnull name; /* Name string */
	AG_VariableType typeTgt;   /* Pointer target type (if indirLvl > 0) */
	int code;                  /* Numerical code (-1 = not serializable) */
	AG_Size size;              /* Size in bytes */
#endif /* !AG_SMALL */
#if AG_MODEL == AG_MEDIUM
	Uint32 _pad;
#endif
} AG_VariableTypeInfo;

/* Variable-stored data */
union ag_variable_data {
	void *_Nullable p;
	char *_Nullable s;
	int             i;
	Uint            u;
	Uint8          u8;
	Sint8          s8;
#if AG_MODEL != AG_SMALL
	Uint16        u16;
	Sint16        s16;
	long           li;
	Ulong         uli;
	Uint32        u32;
	Sint32        s32;
#endif
#ifdef AG_HAVE_64BIT
	Uint64        u64;
	Sint64        s64;
#endif
#ifdef AG_HAVE_FLOAT
	float         flt;
	double        dbl;
#endif
};

/* Handy typedefs for AG_Event-accepting functions with typed return values. */
struct ag_event;
typedef void   (*AG_VoidFn)(struct ag_event *_Nonnull);
typedef Uint   (*AG_UintFn)(struct ag_event *_Nonnull);
typedef int    (*AG_IntFn)(struct ag_event *_Nonnull);
typedef Ulong  (*AG_UlongFn)(struct ag_event *_Nonnull);
typedef long   (*AG_LongFn)(struct ag_event *_Nonnull);
typedef Uint8  (*AG_Uint8Fn)(struct ag_event *_Nonnull);
typedef Sint8  (*AG_Sint8Fn)(struct ag_event *_Nonnull);
typedef Uint16 (*AG_Uint16Fn)(struct ag_event *_Nonnull);
typedef Sint16 (*AG_Sint16Fn)(struct ag_event *_Nonnull);
typedef Uint32 (*AG_Uint32Fn)(struct ag_event *_Nonnull);
typedef Sint32 (*AG_Sint32Fn)(struct ag_event *_Nonnull);
#ifdef AG_HAVE_64BIT
typedef Uint64 (*AG_Uint64Fn)(struct ag_event *_Nonnull);
typedef Sint64 (*AG_Sint64Fn)(struct ag_event *_Nonnull);
#endif
#ifdef AG_HAVE_FLOAT
typedef float  (*AG_FloatFn)(struct ag_event *_Nonnull);
typedef double (*AG_DoubleFn)(struct ag_event *_Nonnull);
#endif
typedef AG_Size (*AG_StringFn)(struct ag_event *_Nonnull, char *_Nonnull, AG_Size);
typedef void *_Nullable (*AG_PointerFn)(struct ag_event *_Nonnull);
typedef const void *_Nullable (*AG_ConstPointerFn)(struct ag_event *_Nonnull);

/* Agar variable instance */
typedef struct ag_variable {
	char name[AG_VARIABLE_NAME_MAX];           /* Name string (or "") */
	AG_VariableType type;			   /* Variable type */
#ifdef AG_THREADS
	_Nullable_Mutex AG_Mutex *_Nullable mutex; /* Lock on target data */
#endif
	union {
		Uint pFlags;           /* Pointer flags (for [P_]POINTER) */
#define AG_VARIABLE_P_READONLY 0x01    /* Hard const (need AG_CONST_PTR()) */
#define AG_VARIABLE_P_FREE     0x02    /* Auto free() target on cleanup */
#define AG_VARIABLE_P_SENDER   0x04    /* Is a pointer to a sender Object */
		union {                /* Bitmask (for P_FLAG_*) */
			Uint u;
			Uint8 u8;
#if AG_MODEL != AG_SMALL
			Uint16 u16;
			Uint32 u32;
#endif
		} bitmask;
		AG_Size size;            /* Length/buffer size (for STRING_*) */
		char *_Nullable varName; /* Variable name (for P_VARIABLE) */
		char *_Nullable objName; /* Unresolved path (for P_OBJECT) */
	} info;
	union ag_variable_data data;	/* Variable-stored data */
	AG_TAILQ_ENTRY(ag_variable) vars;
} AG_Variable;

#define AG_VARIABLE_TYPE(V)      (agVariableTypes[(V)->type].typeTgt)
#define AG_VARIABLE_TYPE_NAME(V) (agVariableTypes[(V)->type].name)

__BEGIN_DECLS
extern const AG_VariableTypeInfo agVariableTypes[];

AG_Variable *_Nullable AG_GetVariable(void *_Nonnull, const char *_Nonnull,
                                      void *_Nonnull *_Nonnull)
                                     _Pure_Attribute_If_Unthreaded
                                     _Warn_Unused_Result;
#ifdef AG_ENABLE_STRING
AG_Size AG_PrintVariable(char *_Nonnull, AG_Size, AG_Variable *_Nonnull);
void    AG_VariableSubst(void *_Nonnull, const char *_Nonnull, char *_Nonnull,
                         AG_Size);
#endif

int  AG_CopyVariable(AG_Variable *_Nonnull _Restrict,
                     const AG_Variable *_Nonnull _Restrict);
int  AG_DerefVariable(AG_Variable *_Nonnull _Restrict,
                      const AG_Variable *_Nonnull _Restrict);
int  AG_CompareVariables(const AG_Variable *_Nonnull,
                         const AG_Variable *_Nonnull)
                        _Pure_Attribute;
void AG_Unset(void *_Nonnull, const char *_Nonnull);

/*
 * UINT: Natural unsigned integer
 */
Uint                  AG_GetUint(void *_Nonnull, const char *_Nonnull)
                                _Pure_Attribute_If_Unthreaded;
#if AG_MODEL != AG_SMALL
void                  AG_InitUint(AG_Variable *_Nonnull, Uint);
#endif
AG_Variable *_Nonnull AG_SetUint(void *_Nonnull, const char *_Nonnull, Uint);
AG_Variable *_Nonnull AG_BindUint(void *_Nonnull, const char *_Nonnull,
                                  Uint *_Nonnull);
#ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindUintMp(void *_Nonnull, const char *_Nonnull,
				    Uint *_Nonnull,
				    _Nonnull_Mutex AG_Mutex *_Nonnull);
#endif
/*
 * INT: Natural integer
 */
int                   AG_GetInt(void *_Nonnull, const char *_Nonnull)
                               _Pure_Attribute_If_Unthreaded;
#if AG_MODEL != AG_SMALL
void                  AG_InitInt(AG_Variable *_Nonnull, int);
#endif
AG_Variable *_Nonnull AG_SetInt(void *_Nonnull, const char *_Nonnull, int);
AG_Variable *_Nonnull AG_BindInt(void *_Nonnull, const char *_Nonnull,
			         int *_Nonnull);
#ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindIntMp(void *_Nonnull, const char *_Nonnull,
                                   int *_Nonnull,
				   _Nonnull_Mutex AG_Mutex *_Nonnull);
#endif

/*
 * BOOL: Alias for INT
 */
#define AG_GetBool(o,k)        AG_GetInt((o),(k))
#define AG_SetBool(o,k,v)      AG_SetInt((o),(k),(v))
#define AG_BindBool(o,k,p)     AG_BindInt((o),(k),(p))
#define AG_BindBoolMp(o,k,p,m) AG_BindIntMp((o),(k),(p),(m))

#if AG_MODEL != AG_SMALL
/*
 * ULONG: Natural long unsigned integer.
 */
Ulong                 AG_GetUlong(void *_Nonnull, const char *_Nonnull)
                                 _Pure_Attribute_If_Unthreaded;
void                  AG_InitUlong(AG_Variable *_Nonnull, Ulong);
AG_Variable *_Nonnull AG_SetUlong(void *_Nonnull, const char *_Nonnull, Ulong);
AG_Variable *_Nonnull AG_BindUlong(void *_Nonnull, const char *_Nonnull,
			           Ulong *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindUlongMp(void *_Nonnull, const char *_Nonnull,
                                     Ulong *_Nonnull,
				     _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif
/*
 * LONG: Natural long integer.
 */
long                  AG_GetLong(void *_Nonnull, const char *_Nonnull)
                                _Pure_Attribute_If_Unthreaded;
void                  AG_InitLong(AG_Variable *_Nonnull, long);
AG_Variable *_Nonnull AG_SetLong(void *_Nonnull, const char *_Nonnull, long);
AG_Variable *_Nonnull AG_BindLong(void *_Nonnull, const char *_Nonnull,
			          long *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindLongMp(void *_Nonnull, const char *_Nonnull,
                                    long *_Nonnull,
                                    _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif

#endif /* !AG_SMALL */

/*
 * UINT8: Unsigned 8-bit integer
 */
Uint8                 AG_GetUint8(void *_Nonnull, const char *_Nonnull)
                                 _Pure_Attribute_If_Unthreaded;
#if AG_MODEL != AG_SMALL
void                  AG_InitUint8(AG_Variable *_Nonnull, Uint8);
#endif
AG_Variable *_Nonnull AG_SetUint8(void *_Nonnull, const char *_Nonnull, Uint8);
AG_Variable *_Nonnull AG_BindUint8(void *_Nonnull, const char *_Nonnull,
                                   Uint8 *_Nonnull);
#ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindUint8Mp(void *_Nonnull, const char *_Nonnull,
                                     Uint8 *_Nonnull,
                                    _Nonnull_Mutex AG_Mutex *_Nonnull);
#endif
/*
 * UINT8: Signed 8-bit integer
 */
Sint8                 AG_GetSint8(void *_Nonnull, const char *_Nonnull)
                                 _Pure_Attribute_If_Unthreaded;
#if AG_MODEL != AG_SMALL
void                  AG_InitSint8(AG_Variable *_Nonnull, Sint8);
#endif
AG_Variable *_Nonnull AG_SetSint8(void *_Nonnull, const char *_Nonnull, Sint8);
AG_Variable *_Nonnull AG_BindSint8(void *_Nonnull, const char *_Nonnull,
			           Sint8 *_Nonnull);
#ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindSint8Mp(void *_Nonnull, const char *_Nonnull,
                                     Sint8 *_Nonnull,
                                    _Nonnull_Mutex AG_Mutex *_Nonnull);
#endif

#if AG_MODEL != AG_SMALL
/*
 * UINT16: Unsigned 16-bit integer
 */
Uint16                AG_GetUint16(void *_Nonnull, const char *_Nonnull)
                                  _Pure_Attribute_If_Unthreaded;
void                  AG_InitUint16(AG_Variable *_Nonnull, Uint16);
AG_Variable *_Nonnull AG_SetUint16(void *_Nonnull, const char *_Nonnull, Uint16);
AG_Variable *_Nonnull AG_BindUint16(void *_Nonnull, const char *_Nonnull,
			            Uint16 *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindUint16Mp(void *_Nonnull, const char *_Nonnull,
                                      Uint16 *_Nonnull,
                                     _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif
/*
 * SINT16: Signed 16-bit integer
 */
Sint16                AG_GetSint16(void *_Nonnull, const char *_Nonnull)
                                  _Pure_Attribute_If_Unthreaded;
void                  AG_InitSint16(AG_Variable *_Nonnull, Sint16);
AG_Variable *_Nonnull AG_SetSint16(void *_Nonnull, const char *_Nonnull, Sint16);
AG_Variable *_Nonnull AG_BindSint16(void *_Nonnull, const char *_Nonnull,
			            Sint16 *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindSint16Mp(void *_Nonnull, const char *_Nonnull,
                                      Sint16 *_Nonnull,
                                     _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif

/*
 * UINT32: Unsigned 32-bit integer
 */
Uint32                AG_GetUint32(void *_Nonnull, const char *_Nonnull)
                                  _Pure_Attribute_If_Unthreaded;
void                  AG_InitUint32(AG_Variable *_Nonnull, Uint32);
AG_Variable *_Nonnull AG_SetUint32(void *_Nonnull, const char *_Nonnull, Uint32);
AG_Variable *_Nonnull AG_BindUint32(void *_Nonnull, const char *_Nonnull,
			            Uint32 *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindUint32Mp(void *_Nonnull, const char *_Nonnull,
                                      Uint32 *_Nonnull,
                                     _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif
/*
 * SINT32: Signed 32-bit integer
 */
Sint32                AG_GetSint32(void *_Nonnull, const char *_Nonnull)
                                  _Pure_Attribute_If_Unthreaded;
void                  AG_InitSint32(AG_Variable *_Nonnull, Sint32);
AG_Variable *_Nonnull AG_SetSint32(void *_Nonnull, const char *_Nonnull, Sint32);
AG_Variable *_Nonnull AG_BindSint32(void *_Nonnull, const char *_Nonnull,
			            Sint32 *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindSint32Mp(void *_Nonnull, const char *_Nonnull,
                                      Sint32 *_Nonnull,
                                     _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif /* AG_THREADS */

#endif /* !AG_SMALL */

#ifdef AG_HAVE_64BIT
/*
 * UINT64: Unsigned 64-bit integer
 */
Uint64                AG_GetUint64(void *_Nonnull, const char *_Nonnull)
                                  _Pure_Attribute_If_Unthreaded;
void                  AG_InitUint64(AG_Variable *_Nonnull, Uint64);
AG_Variable *_Nonnull AG_SetUint64(void *_Nonnull, const char *_Nonnull, Uint64);
AG_Variable *_Nonnull AG_BindUint64(void *_Nonnull, const char *_Nonnull,
			            Uint64 *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindUint64Mp(void *_Nonnull, const char *_Nonnull,
                                      Uint64 *_Nonnull,
                                     _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif
/*
 * SINT64: Signed 64-bit integer
 */
Sint64                AG_GetSint64(void *_Nonnull, const char *_Nonnull)
                                  _Pure_Attribute_If_Unthreaded;
void                  AG_InitSint64(AG_Variable *_Nonnull, Sint64);
AG_Variable *_Nonnull AG_SetSint64(void *_Nonnull, const char *_Nonnull, Sint64);
AG_Variable *_Nonnull AG_BindSint64(void *_Nonnull, const char *_Nonnull,
			            Sint64 *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindSint64Mp(void *_Nonnull, const char *_Nonnull,
                                      Sint64 *_Nonnull,
                                     _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif
#endif /* AG_HAVE_64BIT */

#ifdef AG_HAVE_FLOAT
/*
 * FLOAT: Single-precision IEEE float.
 */
float                 AG_GetFloat(void *_Nonnull, const char *_Nonnull)
                                 _Pure_Attribute_If_Unthreaded;
void                  AG_InitFloat(AG_Variable *_Nonnull, float);
AG_Variable *_Nonnull AG_SetFloat(void *_Nonnull, const char *_Nonnull, float);
AG_Variable *_Nonnull AG_BindFloat(void *_Nonnull, const char *_Nonnull,
			           float *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindFloatMp(void *_Nonnull, const char *_Nonnull,
                                     float *_Nonnull,
                                    _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif
/*
 * DOUBLE: Double-precision IEEE float.
 */
double                AG_GetDouble(void *_Nonnull, const char *_Nonnull)
                                  _Pure_Attribute_If_Unthreaded;
void                  AG_InitDouble(AG_Variable *_Nonnull, double);
AG_Variable *_Nonnull AG_SetDouble(void *_Nonnull, const char *_Nonnull, double);
AG_Variable *_Nonnull AG_BindDouble(void *_Nonnull, const char *_Nonnull,
                                    double *_Nonnull);
# ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindDoubleMp(void *_Nonnull, const char *_Nonnull,
                                      double *_Nonnull,
                                     _Nonnull_Mutex AG_Mutex *_Nonnull);
# endif
#endif /* HAVE_FLOAT */

/*
 * STRING: C string (which is either contained in a fixed-size buffer,
 * or dynamically allocated internally).
 */
AG_Size               AG_GetString(void *_Nonnull, const char *_Nonnull,
                                   char *_Nonnull, AG_Size);
#if AG_MODEL != AG_SMALL
char *_Nullable       AG_GetStringDup(void *_Nonnull, const char *_Nonnull);
char *_Nullable       AG_GetStringP(void *_Nonnull, const char *_Nonnull);
#endif
AG_Variable *_Nonnull AG_SetString(void *_Nonnull, const char *_Nonnull,
                                   const char *_Nonnull);
#if AG_MODEL != AG_SMALL
void                  AG_InitString(AG_Variable *_Nonnull, const char *_Nonnull);
#endif
AG_Variable *_Nonnull AG_SetStringF(void *_Nonnull, const char *_Nonnull,
                                    const char *_Nonnull, ...)
		                   FORMAT_ATTRIBUTE(printf,3,4);
AG_Variable *_Nonnull AG_SetStringNODUP(void *_Nonnull, const char *_Nonnull,
                                        char *_Nonnull);
void                  AG_InitStringNODUP(AG_Variable *_Nonnull, char *_Nonnull);
AG_Variable *_Nonnull AG_BindString(void *_Nonnull, const char *_Nonnull,
                                    char *_Nonnull, AG_Size);
#ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindStringMp(void *_Nonnull, const char *_Nonnull,
                                      char *_Nonnull, AG_Size,
                                     _Nonnull_Mutex AG_Mutex *_Nonnull);
#endif

/*
 * POINTER: A generic pointer or memory address.
 */
void *_Nullable       AG_GetPointer(void *_Nonnull, const char *_Nonnull)
                                   _Pure_Attribute_If_Unthreaded;
void                  AG_InitPointer(AG_Variable *_Nonnull, void *_Nullable);
AG_Variable *_Nonnull AG_SetPointer(void *_Nonnull, const char *_Nonnull,
                                    void *_Nullable);
AG_Variable *_Nonnull AG_BindPointer(void *_Nonnull, const char *_Nonnull,
                                     void *_Nonnull *_Nullable);
#ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindPointerMp(void *_Nonnull, const char *_Nonnull,
                                       void *_Nonnull *_Nullable,
				       _Nonnull_Mutex AG_Mutex *_Nonnull);
#endif
#if AG_MODEL != AG_SMALL
const void *_Nullable AG_GetConstPointer(void *_Nonnull, const char *_Nonnull)
                                        _Pure_Attribute_If_Unthreaded;
void                  AG_InitConstPointer(AG_Variable *_Nonnull, const void *_Nullable);
AG_Variable *_Nonnull AG_SetConstPointer(void *_Nonnull, const char *_Nonnull,
                                         const void *_Nullable);
#endif

/*
 * FLAG*: One or more bits in an integer, indicated by a constant bitmask.
 */
AG_Variable *_Nonnull AG_BindFlag(void *_Nonnull, const char *_Nonnull,
                                  Uint *_Nonnull, Uint);
#ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindFlagMp(void *_Nonnull, const char *_Nonnull,
				    Uint *_Nonnull, Uint,
				    _Nonnull_Mutex AG_Mutex *_Nonnull);
#endif

AG_Variable *_Nonnull AG_BindFlag8(void *_Nonnull, const char *_Nonnull,
				   Uint8 *_Nonnull, Uint8);
#if AG_MODEL != AG_SMALL
AG_Variable *_Nonnull AG_BindFlag16(void *_Nonnull, const char *_Nonnull,
				    Uint16 *_Nonnull, Uint16);
AG_Variable *_Nonnull AG_BindFlag32(void *_Nonnull, const char *_Nonnull,
				    Uint32 *_Nonnull, Uint32);
#endif
#ifdef AG_THREADS
AG_Variable *_Nonnull AG_BindFlag8Mp(void *_Nonnull, const char *_Nonnull,
				     Uint8 *_Nonnull, Uint8,
				     _Nonnull_Mutex AG_Mutex *_Nonnull);
AG_Variable *_Nonnull AG_BindFlag16Mp(void *_Nonnull, const char *_Nonnull,
                                      Uint16 *_Nonnull, Uint16,
                                      _Nonnull_Mutex AG_Mutex *_Nonnull);
AG_Variable *_Nonnull AG_BindFlag32Mp(void *_Nonnull, const char *_Nonnull,
				      Uint32 *_Nonnull, Uint32,
				      _Nonnull_Mutex AG_Mutex *_Nonnull);
#endif

AG_Variable *_Nonnull AG_BindObject(void *_Nonnull, const char *_Nonnull,
				    void *_Nonnull);
AG_Variable *_Nonnull AG_BindVariable(void *_Nonnull, const char *_Nonnull,
				      void *_Nonnull, const char *_Nonnull);
/*
 * Inlinables
 */
void ag_init_variable(AG_Variable *_Nonnull, AG_VariableType, const char *_Nonnull);
void ag_lock_variable(AG_Variable *_Nonnull);
void ag_unlock_variable(AG_Variable *_Nonnull);
void ag_free_variable(AG_Variable *_Nonnull);

#ifdef AG_INLINE_VARIABLE
# define AG_INLINE_HEADER
# include <agar/core/inline_variable.h>
#else
# define AG_InitVariable(V,t,n) ag_init_variable((V),(t),(n))
# define AG_FreeVariable(V)     ag_free_variable(V)
# ifdef AG_THREADS
#  define AG_LockVariable(V)     ag_lock_variable(V)
#  define AG_UnlockVariable(V)   ag_unlock_variable(V)
# else
#  define AG_LockVariable(V)
#  define AG_UnlockVariable(V)
# endif
#endif
__END_DECLS
