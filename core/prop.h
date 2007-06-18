/*	$Csoft: prop.h,v 1.26 2005/09/27 00:25:17 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PROP_H_
#define _AGAR_PROP_H_

#ifdef _AGAR_INTERNAL
#include <config/have_long_double.h>
#else
#include <agar/config/have_long_double.h>
#endif

#include "begin_code.h"

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
	AG_PROP_UINT64		= 8,	/* Optional */
	AG_PROP_SINT64		= 9,	/* Optional */
	AG_PROP_FLOAT		= 10,
	AG_PROP_DOUBLE		= 11,
	AG_PROP_LONG_DOUBLE	= 12,	/* Optional */
	AG_PROP_STRING		= 13,
	AG_PROP_POINTER		= 14,
	AG_PROP_BOOL		= 15,
	AG_PROP_EXT_FIRST	= 100,		/* Official extensions */
	AG_PROP_EXT_LAST	= 10000,
	AG_PROP_PVT_START	= 10001,	/* Application extensions */
	AG_PROP_PVT_END		= 65535
};

#if 0
struct ag_prop_ops {
	enum ag_prop_type type;
	const char *descr;
	void (*set_val)(void *obj, AG_Prop *prop, void *arg);
	void *(*get_val)(void *obj, AG_Prop *prop);
	void (*print_val)(char *buf, size_t len, void *obj, AG_Prop *prop);
	int (*compare)(AG_Prop *, AG_Prop *);
	void *(*load)(void *obj, AG_Prop *, AG_Netbuf *);
	void (*save)(void *obj, AG_Prop *, AG_Netbuf *);
} AG_PropOps;
#endif

typedef struct ag_prop {
	char key[AG_PROP_KEY_MAX];
	int type;
	union {
		unsigned u;
		int	 i;
		Uint8	 u8;
		Sint8	 s8;
		Uint16	 u16;
		Sint16	 s16;
		Uint32	 u32;
		Sint32	 s32;
#ifdef SDL_HAS_64BIT_TYPE
		Uint64   u64;
		Sint64   s64;
#endif
		float	 f;
		double	 d;
#ifdef HAVE_LONG_DOUBLE
		long double ld;
#endif
		char	*s;
		void	*p;
	} data;
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
#ifdef SDL_HAS_64BIT_TYPE
		Uint64	(*wUint64)(void *, struct ag_prop *, Uint64);
		Sint64	(*wSint64)(void *, struct ag_prop *, Sint64);
#endif
		float	(*wFloat)(void *, struct ag_prop *, float);
		double	(*wDouble)(void *, struct ag_prop *, double);
#ifdef HAVE_LONG_DOUBLE
		long double (*wLongDouble)(void *, struct ag_prop *,
		                           long double);
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
#ifdef SDL_HAS_64BIT_TYPE
		Uint64	(*rUint64)(void *, struct ag_prop *);
		Sint64	(*rSint64)(void *, struct ag_prop *);
#endif
		float	(*rFloat)(void *, struct ag_prop *);
		double	(*rDouble)(void *, struct ag_prop *);
#ifdef HAVE_LONG_DOUBLE
		long double (*rLongDouble)(void *, struct ag_prop *);
#endif
		char	*(*rString)(void *, struct ag_prop *);
		void	*(*rPointer)(void *, struct ag_prop *);
	} readFn;
	TAILQ_ENTRY(ag_prop) props;
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
__inline__ void	 	 AG_LockProps(void *);
__inline__ void	 	 AG_UnlockProps(void *);
int		 	 AG_PropLoad(void *, AG_Netbuf *);
int		 	 AG_PropSave(void *, AG_Netbuf *);
void		 	 AG_PropDestroy(AG_Prop *);
AG_Prop			*AG_CopyProp(const AG_Prop *);
AG_Prop			*AG_SetProp(void *, const char *, enum ag_prop_type,
			            ...);

__inline__ AG_Prop	*AG_SetBool(void *, const char *, int);
__inline__ AG_Prop	*AG_SetUint(void *, const char *, Uint);
__inline__ AG_Prop	*AG_SetInt(void *, const char *, int);
__inline__ AG_Prop	*AG_SetUint8(void *, const char *, Uint8);
__inline__ AG_Prop	*AG_SetSint8(void *, const char *, Sint8);
__inline__ AG_Prop	*AG_SetUint16(void *, const char *, Uint16);
__inline__ AG_Prop	*AG_SetSint16(void *, const char *, Sint16);
__inline__ AG_Prop	*AG_SetUint32(void *, const char *, Uint32);
__inline__ AG_Prop	*AG_SetSint32(void *, const char *, Sint32);
#ifdef SDL_HAS_64BIT_TYPE
__inline__ AG_Prop	*AG_SetUint64(void *, const char *, Uint64);
__inline__ AG_Prop	*AG_SetSint64(void *, const char *, Sint64);
#endif
__inline__ AG_Prop	*AG_SetFloat(void *, const char *, float);
__inline__ AG_Prop	*AG_SetDouble(void *, const char *, double);
__inline__ AG_Prop	*AG_SetPointer(void *, const char *, void *);
AG_Prop		 	*AG_SetString(void *, const char *, const char *, ...)
			 	      FORMAT_ATTRIBUTE(printf, 3, 4)
			 	      NONNULL_ATTRIBUTE(3);

AG_Prop		 *AG_GetProp(void *, const char *, int, void *);
AG_Prop		 *AG_FindProp(const char *, int, void *);
void		  AG_PropPrint(char *, size_t, void *, const char *)
		               BOUNDED_ATTRIBUTE(__string__, 1, 2);
__inline__ int	  AG_PropPath(char *, size_t, const void *, const char *)
		              BOUNDED_ATTRIBUTE(__string__, 1, 2);

__inline__ int	  AG_Bool(void *, const char *);
__inline__ Uint   AG_Uint(void *, const char *);
__inline__ int	  AG_Int(void *, const char *);
__inline__ Uint8  AG_Uint8(void *, const char *);
__inline__ Sint8  AG_Sint8(void *, const char *);
__inline__ Uint16 AG_Uint16(void *, const char *);
__inline__ Sint16 AG_Sint16(void *, const char *);
__inline__ Uint32 AG_Uint32(void *, const char *);
__inline__ Sint32 AG_Sint32(void *, const char *);
__inline__ Uint64 AG_Uint64(void *, const char *);
__inline__ Sint64 AG_Sint64(void *, const char *);
__inline__ float  AG_Float(void *, const char *);
__inline__ double AG_Double(void *, const char *);
__inline__ void	 *AG_Pointer(void *, const char *);
__inline__ char	 *AG_String(void *, const char *);
__inline__ size_t AG_StringCopy(void *, const char *, char *, size_t)
		                BOUNDED_ATTRIBUTE(__string__, 3, 4);

__inline__ int	  AG_FindBool(const char *);
__inline__ Uint   AG_FindUint(const char *);
__inline__ int	  AG_FindInt(const char *);
__inline__ Uint8  AG_FindUint8(const char *);
__inline__ Sint8  AG_FindSint8(const char *);
__inline__ Uint16 AG_FindUint16(const char *);
__inline__ Sint16 AG_FindSint16(const char *);
__inline__ Uint32 AG_FindUint32(const char *);
__inline__ Sint32 AG_FindSint32(const char *);
__inline__ Uint64 AG_FindUint64(const char *);
__inline__ Sint64 AG_FindSint64(const char *);
__inline__ float  AG_FindFloat(const char *);
__inline__ double AG_FindDouble(const char *);
__inline__ void	 *AG_FindPointer(const char *);
__inline__ char	 *AG_FindString(const char *);
__inline__ size_t AG_FindStringCopy(const char *, char *, size_t)
		                    BOUNDED_ATTRIBUTE(__string__, 2, 3);

#ifdef HAVE_LONG_DOUBLE
__inline__ AG_Prop    *AG_SetLongDouble(void *, const char *, long double);
__inline__ long double AG_LongDouble(void *, const char *);
__inline__ long double AG_FindLongDouble(const char *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_PROP_H_ */
