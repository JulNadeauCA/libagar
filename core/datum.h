/*	Public domain	*/

#include <agar/config/have_64bit.h>
#include <agar/config/have_long_double.h>

#include <agar/core/begin.h>

#define AG_ARGS_MAX	16

typedef enum ag_datum_type {
	/*
	 * Standard types
	 */
	AG_DATUM_NULL		,	/* No data */
	AG_DATUM_UINT		,	/* Unsigned int */
	AG_DATUM_INT		,	/* int */
	AG_DATUM_UINT8		,	/* Unsigned 8-bit */
	AG_DATUM_SINT8		,	/* Signed 8-bit */
	AG_DATUM_UINT16		,	/* Unsigned 16-bit */
	AG_DATUM_SINT16		,	/* Signed 16-bit */
	AG_DATUM_UINT32		,	/* Unsigned 32-bit */
	AG_DATUM_SINT32		,	/* Signed 32-bit */
	AG_DATUM_UINT64		,	/* Unsigned 64-bit (optional) */
	AG_DATUM_SINT64		,	/* Signed 64-bit (optional) */
	AG_DATUM_FLOAT		,	/* Single-precision float */
	AG_DATUM_DOUBLE		,	/* Double-precision float */
	AG_DATUM_LONG_DOUBLE	,	/* Quad-precision float (optional) */
	AG_DATUM_STRING		,	/* C string */
	AG_DATUM_CONST_STRING	,	/* C string (const) */
	AG_DATUM_POINTER	,	/* C pointer */
	AG_DATUM_CONST_POINTER	,	/* C pointer (const) */

	AG_DATUM_P_UINT		,	/* Pointer to Uint */
	AG_DATUM_P_INT		,	/* Pointer to int */
	AG_DATUM_P_UINT8	,	/* Pointer to Uint8 */
	AG_DATUM_P_SINT8	,	/* Pointer to Sint8 */
	AG_DATUM_P_UINT16	,	/* Pointer to Uint16 */
	AG_DATUM_P_SINT16	,	/* Pointer to Sint16 */
	AG_DATUM_P_UINT32	,	/* Pointer to Uint32 */
	AG_DATUM_P_SINT32	,	/* Pointer to Sint32 */
	AG_DATUM_P_UINT64	,	/* Pointer to Uint64 (optional) */
	AG_DATUM_P_SINT64	,	/* Pointer to Sint64 (optional) */
	AG_DATUM_P_FLOAT	,	/* Pointer to float */
	AG_DATUM_P_DOUBLE	,	/* Pointer to double */
	AG_DATUM_P_LONG_DOUBLE	,	/* Pointer to long double (optional) */
	AG_DATUM_P_STRING	,	/* Pointer to C string */
	AG_DATUM_P_CONST_STRING ,	/* Pointer to C string (const) */
	AG_DATUM_P_POINTER	,	/* Pointer to C pointer */
	AG_DATUM_P_CONST_POINTER ,	/* Pointer to C pointer (const) */
	AG_DATUM_P_OBJECT	,	/* Pointer to AG_Object */
	AG_DATUM_P_FLAG		,	/* Bit in int (uses info.mask) */
	AG_DATUM_P_FLAG8	,	/* Bit in int8 (uses info.mask) */
	AG_DATUM_P_FLAG16	,	/* Bit in int16 (uses info.mask) */
	AG_DATUM_P_FLAG32	,	/* Bit in int32 (uses info.mask) */

	/*
	 * Standard types implemented outside of Agar
	 */
	AG_DATUM_REAL		,	/* M: Real number */
	AG_DATUM_P_REAL		,
	AG_DATUM_RANGE		,	/* M: Interval */
	AG_DATUM_P_RANGE	,
	AG_DATUM_COMPLEX	,	/* M: Complex number */
	AG_DATUM_P_COMPLEX	,
	AG_DATUM_QUAT		,	/* M: Quaternion */
	AG_DATUM_P_QUAT		,
	AG_DATUM_RECTANGULAR	,	/* M: Rectangular coordinates */
	AG_DATUM_P_RECTANGULAR	,
	AG_DATUM_POLAR		,	/* M: Polar coordinates */
	AG_DATUM_P_POLAR	,
	AG_DATUM_PARABOLIC	,	/* M: Parabolic coordinates */
	AG_DATUM_P_PARABOLIC	,
	AG_DATUM_SPHERICAL	,	/* M: Spherical coordinates */
	AG_DATUM_P_SPHERICAL	,
	AG_DATUM_CYLINDRICAL	,	/* M: Cylindrical coordinates */
	AG_DATUM_P_CYLINDRICAL	,
	AG_DATUM_COLOR		,	/* M: Vector in RGBA color space */
	AG_DATUM_P_COLOR	,
	AG_DATUM_VECTOR		,	/* M: Vector in Rn */
	AG_DATUM_P_VECTOR	,
	AG_DATUM_VECTOR2	,	/* M: Vector in R2 */
	AG_DATUM_P_VECTOR2	,
	AG_DATUM_VECTOR3	,	/* M: Vector in R3 */
	AG_DATUM_P_VECTOR3	,
	AG_DATUM_VECTOR4	,	/* M: Vector in R4 */
	AG_DATUM_P_VECTOR4	,
	AG_DATUM_MATRIX		,	/* M: mxn matrix */
	AG_DATUM_P_MATRIX	,
	AG_DATUM_MATRIX22	,	/* M: 2x2 matrix */
	AG_DATUM_P_MATRIX22	,
	AG_DATUM_MATRIX33	,	/* M: 3x3 matrix */
	AG_DATUM_P_MATRIX33	,
	AG_DATUM_MATRIX44	,	/* M: 4x4 matrix */
	AG_DATUM_P_MATRIX44	,
	/*
	 * For future standard types
	 */
	AG_DATUM_EXT4		,
	AG_DATUM_EXT5		,
	AG_DATUM_EXT6		,
	AG_DATUM_EXT7		,
	AG_DATUM_EXT8		,
	AG_DATUM_EXT9		,
	AG_DATUM_EXT10		,
	AG_DATUM_EXT11		,
	AG_DATUM_EXT12		,
	AG_DATUM_EXT13		,
	AG_DATUM_EXT14		,
	AG_DATUM_EXT15		,
	AG_DATUM_EXT16		,
	AG_DATUM_EXT17		,
	/*
	 * Application-specific extensions
	 */
	AG_DATUM_PRIVATE	= 10001,
} AG_DatumType;

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
#ifdef HAVE_64BIT
typedef Uint64      (*AG_Uint64Fn)(struct ag_event *);
typedef Sint64      (*AG_Sint64Fn)(struct ag_event *);
#endif
typedef float       (*AG_FloatFn)(struct ag_event *);
typedef double      (*AG_DoubleFn)(struct ag_event *);
#ifdef HAVE_LONG_DOUBLE
typedef long double (*AG_LongDoubleFn)(struct ag_event *);
#endif
typedef char       *(*AG_StringFn)(struct ag_event *, size_t *);
typedef const char *(*AG_ConstStringFn)(struct ag_event *, size_t *);
typedef void       *(*AG_PointerFn)(struct ag_event *);
typedef const void *(*AG_ConstPointerFn)(struct ag_event *);

typedef struct ag_datum {
	enum ag_datum_type type;	/* Datum type */
	const char *name;		/* Key */
	AG_Mutex *mutex;		/* Lock protecting data (or NULL) */
	union {
		Uint32 bitmask;		/* Bitmask (for DATUM_P_FLAG_*) */
		size_t size;		/* Size (for DATUM_STRING_*) */
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
#ifdef HAVE_64BIT
		Uint64 (*fnUint64)(struct ag_event *);
		Sint64 (*fnSint64)(struct ag_event *);
#endif
		float (*fnFloat)(struct ag_event *);
		double (*fnDouble)(struct ag_event *);
#ifdef HAVE_LONG_DOUBLE
		long double (*fnLongDouble)(struct ag_event *);
#endif
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
#ifdef HAVE_64BIT
		Uint64 u64;
		Sint64 s64;
#else
		Uint32 u64[2];		/* Padding */
		Sint32 s64[2];		/* Padding */
#endif
#ifdef HAVE_LONG_DOUBLE
		long double ldbl;	/* Keep at end (padding) */
#endif
	} data;
} AG_Datum;

__BEGIN_DECLS
struct ag_list;

extern const char *agDatumTypeNames[];
struct ag_list *AG_ParseDatumList(const char *, ...);
__END_DECLS

#include <agar/core/close.h>
