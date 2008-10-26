/*	Public domain	*/

#include <agar/config/have_long_double.h>

#include <agar/core/begin.h>

#define AG_PROP_PATH_MAX	(AG_OBJECT_PATH_MAX+1+AG_PROP_KEY_MAX)
#define AG_PROP_KEY_MAX		64
/* #define AG_PROP_STRING_LIMIT	65536 */

enum ag_prop_type {
	AG_PROP_UINT		= 0,
	AG_PROP_INT		= 1,
	AG_PROP_UINT8		= 2,
	AG_PROP_SINT8		= 3,
	AG_PROP_UINT16		= 4,
	AG_PROP_SINT16		= 5,
	AG_PROP_UINT32		= 6,
	AG_PROP_SINT32		= 7,
	AG_PROP_UINT64		= 8,	/* MD */
	AG_PROP_SINT64		= 9,	/* MD */
	AG_PROP_FLOAT		= 10,
	AG_PROP_DOUBLE		= 11,
	AG_PROP_LONG_DOUBLE	= 12,	/* MD */
	AG_PROP_STRING		= 13,
	AG_PROP_POINTER		= 14,
	AG_PROP_BOOL		= 15,
	/*
	 * Extensions implemented outside of Agar
	 */
	AG_PROP_REAL		= 100,	/* M: Real number */
	AG_PROP_RANGE		= 101,	/* M: Interval */
	AG_PROP_COMPLEX		= 102,	/* M: Complex number */
	AG_PROP_QUAT		= 103,	/* M: Quaternion */
	AG_PROP_RECTANGULAR	= 104,	/* M: Rectangular coordinates */
	AG_PROP_POLAR		= 105,	/* M: Polar coordinates */
	AG_PROP_PARABOLIC	= 106,	/* M: Parabolic coordinates */
	AG_PROP_SPHERICAL	= 107,	/* M: Spherical coordinates */
	AG_PROP_CYLINDRICAL	= 108,	/* M: Cylindrical coordinates */
	AG_PROP_COLOR		= 109,	/* M: Vector in RGBA color space */
	AG_PROP_VECTOR		= 120,	/* M: Vector in Rn */
	AG_PROP_VECTOR2		= 121,	/* M: Vector in R2 */
	AG_PROP_VECTOR3		= 122,	/* M: Vector in R3 */
	AG_PROP_VECTOR4		= 123,	/* M: Vector in R4 */
	AG_PROP_MATRIX		= 130,	/* M: mxn matrix */
	AG_PROP_MATRIX22	= 131,	/* M: 2x2 matrix */
	AG_PROP_MATRIX33	= 132,	/* M: 3x3 matrix */
	AG_PROP_MATRIX44	= 133,	/* M: 4x4 matrix */
	/*
	 * Application extensions
	 */
	AG_PROP_PRIVATE		= 10001,
};

struct ag_object;
struct ag_prop;

typedef struct ag_prop_class {
	Uint type;
	size_t size;

	int   (*set)(struct ag_object *obj, struct ag_prop *prop, void *val);
	int   (*get)(struct ag_object *obj, struct ag_prop *prop, void **pVal);
	void  (*print)(char *buf, size_t len, struct ag_object *obj,
	               struct ag_prop *prop);
	int   (*compare)(struct ag_prop *, struct ag_prop *);
	void *(*load)(struct ag_object *obj, struct ag_prop *, AG_DataSource *);
	void  (*save)(struct ag_object *obj, struct ag_prop *, AG_DataSource *);
} AG_PropClass;

typedef struct ag_prop {
	char key[AG_PROP_KEY_MAX];		/* Property name */
	Uint type;				/* Property class */
	void *obj;				/* Back pointer to object */
	union {
		Uint	(*wUint)(void *, struct ag_prop *, Uint);
		int	(*wInt)(void *, struct ag_prop *, int);
		int	(*wBool)(void *, struct ag_prop *, int);
		Uint8	(*wUint8)(void *, struct ag_prop *, Uint8);
		Sint8	(*wSint8)(void *, struct ag_prop *, Sint8);
		Uint16	(*wUint16)(void *, struct ag_prop *, Uint16);
		Sint16	(*wSint16)(void *, struct ag_prop *, Sint16);
		Uint32	(*wUint32)(void *, struct ag_prop *, Uint32);
		Sint32	(*wSint32)(void *, struct ag_prop *, Sint32);
#ifdef HAVE_64BIT
		Uint64	(*wUint64)(void *, struct ag_prop *, Uint64);
		Sint64	(*wSint64)(void *, struct ag_prop *, Sint64);
#else /* Padding */
		Uint32  (*wUint64)(void *, struct ag_prop *, void *);
		Uint32  (*wSint64)(void *, struct ag_prop *, void *);
#endif
		float	(*wFloat)(void *, struct ag_prop *, float);
		double	(*wDouble)(void *, struct ag_prop *, double);
#ifdef HAVE_LONG_DOUBLE
		long double (*wLongDouble)(void *, struct ag_prop *,
		                           long double);
#else /* Padding */
		double (*wLongDouble)(void *, struct ag_prop *, double);
#endif
		char	*(*wString)(void *, struct ag_prop *, char *);
		void	*(*wPointer)(void *, struct ag_prop *, void *);
	} writeFn;
	union {
		Uint	(*rUint)(void *, struct ag_prop *);
		int	(*rInt)(void *, struct ag_prop *);
		int	(*rBool)(void *, struct ag_prop *);
		Uint8	(*rUint8)(void *, struct ag_prop *);
		Sint8	(*rSint8)(void *, struct ag_prop *);
		Uint16	(*rUint16)(void *, struct ag_prop *);
		Sint16	(*rSint16)(void *, struct ag_prop *);
		Uint32	(*rUint32)(void *, struct ag_prop *);
		Sint32	(*rSint32)(void *, struct ag_prop *);
#ifdef HAVE_64BIT
		Uint64	(*rUint64)(void *, struct ag_prop *);
		Sint64	(*rSint64)(void *, struct ag_prop *);
#else /* Padding */
		Uint32	(*rUint64)(void *, struct ag_prop *);
		Sint32	(*rSint64)(void *, struct ag_prop *);
#endif
		float	(*rFloat)(void *, struct ag_prop *);
		double	(*rDouble)(void *, struct ag_prop *);
#ifdef HAVE_LONG_DOUBLE
		long double (*rLongDouble)(void *, struct ag_prop *);
#else /* Padding */
		double      (*rLongDouble)(void *, struct ag_prop *);
#endif
		char	*(*rString)(void *, struct ag_prop *);
		void	*(*rPointer)(void *, struct ag_prop *);
	} readFn;
	AG_TAILQ_ENTRY(ag_prop) props;
	
	union {
		unsigned u;
		int	 i;
		Uint8	 u8;
		Sint8	 s8;
		Uint16	 u16;
		Sint16	 s16;
		Uint32	 u32;
		Sint32	 s32;
#ifdef HAVE_64BIT
		Uint64   u64;
		Sint64   s64;
#else
		Uint32   u64[4];	/* Padding */
#endif
		float	 f;
		double	 d;
		char	*s;
		void	*p;
#ifdef HAVE_LONG_DOUBLE
		long double ld;		/* Keep at end of struct (padding) */
#endif
	} data;
} AG_Prop;

#define AG_SetUintWrFn(prop,fn) (prop)->writeFn.wUint = (fn)
#define AG_SetIntWrFn(prop,fn) (prop)->writeFn.wInt = (fn)
#define AG_SetBoolWrFn(prop,fn) (prop)->writeFn.wBool = (fn)
#define AG_SetUint8WrFn(prop,fn) (prop)->writeFn.wUint8 = (fn)
#define AG_SetSint8WrFn(prop,fn) (prop)->writeFn.wSint8 = (fn)
#define AG_SetUint16WrFn(prop,fn) (prop)->writeFn.wUint16 = (fn)
#define AG_SetSint16WrFn(prop,fn) (prop)->writeFn.wSint16 = (fn)
#define AG_SetUint32WrFn(prop,fn) (prop)->writeFn.wUint32 = (fn)
#define AG_SetSint32WrFn(prop,fn) (prop)->writeFn.wSint32 = (fn)
#define AG_SetUint64WrFn(prop,fn) (prop)->writeFn.wUint64 = (fn)
#define AG_SetSint64WrFn(prop,fn) (prop)->writeFn.wSint64 = (fn)
#define AG_SetFloatWrFn(prop,fn) (prop)->writeFn.wFloat = (fn)
#define AG_SetDoubleWrFn(prop,fn) (prop)->writeFn.wDouble = (fn)
#define AG_SetLongDoubleWrFn(prop,fn) (prop)->writeFn.wLongDouble = (fn)
#define AG_SetStringWrFn(prop,fn) (prop)->writeFn.wString = (fn)
#define AG_SetPointerWrFn(prop,fn) (prop)->writeFn.wPointer = (fn)

#define AG_SetUintRdFn(prop,fn) (prop)->readFn.rUint = (fn)
#define AG_SetIntRdFn(prop,fn) (prop)->readFn.rInt = (fn)
#define AG_SetBoolRdFn(prop,fn) (prop)->readFn.rBool = (fn)
#define AG_SetUint8RdFn(prop,fn) (prop)->readFn.rUint8 = (fn)
#define AG_SetSint8RdFn(prop,fn) (prop)->readFn.rSint8 = (fn)
#define AG_SetUint16RdFn(prop,fn) (prop)->readFn.rUint16 = (fn)
#define AG_SetSint16RdFn(prop,fn) (prop)->readFn.rSint16 = (fn)
#define AG_SetUint32RdFn(prop,fn) (prop)->readFn.rUint32 = (fn)
#define AG_SetSint32RdFn(prop,fn) (prop)->readFn.rSint32 = (fn)
#define AG_SetUint64RdFn(prop,fn) (prop)->readFn.rUint64 = (fn)
#define AG_SetSint64RdFn(prop,fn) (prop)->readFn.rSint64 = (fn)
#define AG_SetFloatRdFn(prop,fn) (prop)->readFn.rFloat = (fn)
#define AG_SetDoubleRdFn(prop,fn) (prop)->readFn.rDouble = (fn)
#define AG_SetLongDoubleRdFn(prop,fn) (prop)->readFn.rLongDouble = (fn)
#define AG_SetStringRdFn(prop,fn) (prop)->readFn.rString = (fn)
#define AG_SetPointerRdFn(prop,fn) (prop)->readFn.rPointer = (fn)

__BEGIN_DECLS
extern const AG_PropClass **agPropClasses;
extern Uint                 agPropClassCount ;

void     AG_RegisterPropClass(const AG_PropClass *);
void     AG_UnregisterPropClass(const AG_PropClass *);

int	 AG_PropLoad(void *, AG_DataSource *);
int	 AG_PropSave(void *, AG_DataSource *);
void	 AG_PropDestroy(AG_Prop *);
AG_Prop	*AG_CopyProp(const AG_Prop *);
AG_Prop	*AG_SetProp(void *, const char *, enum ag_prop_type, ...);

AG_Prop	 *AG_GetProp(void *, const char *, int, void *);
AG_Prop	 *AG_FindProp(void *, const char *, int, void *);
void	  AG_PropPrint(char *, size_t, void *, const char *)
		       BOUNDED_ATTRIBUTE(__string__, 1, 2);
int	  AG_PropCopyPath(char *, size_t, void *, const char *)
		          BOUNDED_ATTRIBUTE(__string__, 1, 2);
size_t	  AG_GetStringCopy(void *, const char *, char *, size_t);
AG_Prop  *AG_SetString(void *, const char *, const char *, ...);
int       AG_PropDefined(void *, const char *);

/********************
 * Get/Set Routines *
 ********************/

static __inline__ AG_Prop *
AG_SetUint(void *ob, const char *key, Uint i)
{
	return (AG_SetProp(ob, key, AG_PROP_UINT, i));
}
static __inline__ AG_Prop *
AG_SetInt(void *ob, const char *key, int i)
{
	return (AG_SetProp(ob, key, AG_PROP_INT, i));
}
static __inline__ AG_Prop *
AG_SetUint8(void *ob, const char *key, Uint8 i)
{
	return (AG_SetProp(ob, key, AG_PROP_UINT8, i));
}
static __inline__ AG_Prop *
AG_SetSint8(void *ob, const char *key, Sint8 i)
{
	return (AG_SetProp(ob, key, AG_PROP_SINT8, i));
}
static __inline__ AG_Prop *
AG_SetUint16(void *ob, const char *key, Uint16 i)
{
	return (AG_SetProp(ob, key, AG_PROP_UINT16, i));
}
static __inline__ AG_Prop *
AG_SetSint16(void *ob, const char *key, Sint16 i)
{
	return (AG_SetProp(ob, key, AG_PROP_SINT16, i));
}
static __inline__ AG_Prop *
AG_SetUint32(void *ob, const char *key, Uint32 i)
{
	return (AG_SetProp(ob, key, AG_PROP_UINT32, i));
}
static __inline__ AG_Prop *
AG_SetSint32(void *ob, const char *key, Sint32 i)
{
	return (AG_SetProp(ob, key, AG_PROP_SINT32, i));
}

#ifdef HAVE_64BIT
static __inline__ AG_Prop *
AG_SetUint64(void *ob, const char *key, Uint64 i)
{
	return (AG_SetProp(ob, key, AG_PROP_UINT64, i));
}
static __inline__ AG_Prop *
AG_SetSint64(void *ob, const char *key, Sint64 i)
{
	return (AG_SetProp(ob, key, AG_PROP_SINT64, i));
}
#endif /* HAVE_64BIT */

static __inline__ AG_Prop *
AG_SetFloat(void *ob, const char *key, float f)
{
	return (AG_SetProp(ob, key, AG_PROP_FLOAT, f));
}
static __inline__ AG_Prop *
AG_SetDouble(void *ob, const char *key, double d)
{
	return (AG_SetProp(ob, key, AG_PROP_DOUBLE, d));
}

#ifdef HAVE_LONG_DOUBLE
static __inline__ AG_Prop *
AG_SetLongDouble(void *ob, const char *key, long double d)
{
	return (AG_SetProp(ob, key, AG_PROP_LONG_DOUBLE, d));
}
#endif /* HAVE_LONG_DOUBLE */

static __inline__ AG_Prop *
AG_SetPointer(void *ob, const char *key, void *p)
{
	return (AG_SetProp(ob, key, AG_PROP_POINTER, p));
}
static __inline__ AG_Prop *
AG_SetBool(void *ob, const char *key, int i)
{
	return (AG_SetProp(ob, key, AG_PROP_BOOL, i));
}
static __inline__ Uint
AG_GetUint(void *p, const char *key)
{
	Uint i;
	if (AG_GetProp(p, key, AG_PROP_UINT, (Uint *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ int
AG_GetInt(void *p, const char *key)
{
	int i;
	if (AG_GetProp(p, key, AG_PROP_INT, (int *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ int
AG_GetBool(void *p, const char *key)
{
	int i;
	if (AG_GetProp(p, key, AG_PROP_BOOL, (int *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Uint8
AG_GetUint8(void *p, const char *key)
{
	Uint8 i;
	if (AG_GetProp(p, key, AG_PROP_UINT8, (Uint8 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Sint8
AG_GetSint8(void *p, const char *key)
{
	Sint8 i;
	if (AG_GetProp(p, key, AG_PROP_SINT8, (Sint8 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Uint16
AG_GetUint16(void *p, const char *key)
{
	Uint16 i;
	if (AG_GetProp(p, key, AG_PROP_UINT16, (Uint16 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Sint16
AG_GetSint16(void *p, const char *key)
{
	Sint16 i;
	if (AG_GetProp(p, key, AG_PROP_SINT16, (Sint16 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Uint32
AG_GetUint32(void *p, const char *key)
{
	Uint32 i;
	if (AG_GetProp(p, key, AG_PROP_UINT32, (Uint32 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Sint32
AG_GetSint32(void *p, const char *key)
{
	Sint32 i;
	if (AG_GetProp(p, key, AG_PROP_SINT32, (Sint32 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}

#ifdef HAVE_64BIT
static __inline__ Uint64
AG_GetUint64(void *p, const char *key)
{
	Uint64 i;
	if (AG_GetProp(p, key, AG_PROP_UINT64, (Uint64 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
static __inline__ Sint64
AG_GetSint64(void *p, const char *key)
{
	Sint64 i;
	if (AG_GetProp(p, key, AG_PROP_SINT64, (Sint64 *)&i) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (i);
}
#endif /* HAVE_64BIT */

static __inline__ float
AG_GetFloat(void *p, const char *key)
{
	float f;
	if (AG_GetProp(p, key, AG_PROP_FLOAT, (float *)&f) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (f);
}
static __inline__ double
AG_GetDouble(void *p, const char *key)
{
	double d;
	if (AG_GetProp(p, key, AG_PROP_DOUBLE, (double *)&d) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (d);
}

#ifdef HAVE_LONG_DOUBLE
static __inline__ long double
AG_GetLongDouble(void *p, const char *key)
{
	long double d;
	if (AG_GetProp(p, key, AG_PROP_LONG_DOUBLE, (long double *)&d)
	    == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (d);
}
#endif /* HAVE_LONG_DOUBLE */

static __inline__ char *
AG_GetString(void *p, const char *key)
{
	char *s;
	if (AG_GetProp(p, key, AG_PROP_STRING, (char *)&s) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (s);
}
static __inline__ void *
AG_GetPointer(void *p, const char *key)
{
	void *np;
	if (AG_GetProp(p, key, AG_PROP_POINTER, (void *)&np) == NULL) {
		AG_FatalError("%s", AG_GetError());
	}
	return (np);
}
__END_DECLS

#include <agar/core/close.h>
