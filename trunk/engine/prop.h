/*	$Csoft: prop.h,v 1.25 2005/03/11 08:59:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PROP_H_
#define _AGAR_PROP_H_
#include "begin_code.h"

#include <config/floating_point.h>
#include <config/have_ieee754.h>
#include <config/have_long_double.h>

#define AG_PROP_KEY_MAX		64
#define AG_PROP_STRING_MAX	65535

enum ag_prop_type {
	AG_PROP_UINT,
	AG_PROP_INT,
	AG_PROP_UINT8,
	AG_PROP_SINT8,
	AG_PROP_UINT16,
	AG_PROP_SINT16,
	AG_PROP_UINT32,
	AG_PROP_SINT32,
	AG_PROP_UINT64,		/* Unused */
	AG_PROP_SINT64,		/* Unused */
	AG_PROP_FLOAT,		/* IEEE 754 encoding */
	AG_PROP_DOUBLE,		/* IEEE 754 encoding */
	AG_PROP_LONG_DOUBLE,	/* Unused */
	AG_PROP_STRING,
	AG_PROP_POINTER,
	AG_PROP_BOOL,
	AG_PROP_ANY
};

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
#ifdef FLOATING_POINT
		float	 f;
		double	 d;
#endif
		char	*s;
		void	*p;
	} data;
	TAILQ_ENTRY(ag_prop) props;
} AG_Prop;

__BEGIN_DECLS
__inline__ void	 AG_LockProps(void *);
__inline__ void	 AG_UnlockProps(void *);
int		 AG_PropLoad(void *, AG_Netbuf *);
int		 AG_PropSave(void *, AG_Netbuf *);
void		 AG_PropDestroy(AG_Prop *);

AG_Prop	*AG_CopyProp(const AG_Prop *);
AG_Prop	*AG_SetProp(void *, const char *, enum ag_prop_type, ...);
AG_Prop	*AG_SetBool(void *, const char *, int);
AG_Prop	*AG_SetUint(void *, const char *, u_int);
AG_Prop	*AG_SetInt(void *, const char *, int);
AG_Prop	*AG_SetUint8(void *, const char *, Uint8);
AG_Prop	*AG_SetSint8(void *, const char *, Sint8);
AG_Prop	*AG_SetUint16(void *, const char *, Uint16);
AG_Prop	*AG_SetSint16(void *, const char *, Sint16);
AG_Prop	*AG_SetUint32(void *, const char *, Uint32);
AG_Prop	*AG_SetSint32(void *, const char *, Sint32);
#ifdef FLOATING_POINT
AG_Prop	*AG_SetFloat(void *, const char *, float);
AG_Prop	*AG_SetDouble(void *, const char *, double);
#endif
AG_Prop	*AG_SetString(void *, const char *, const char *, ...)
		      FORMAT_ATTRIBUTE(printf, 3, 4)
		      NONNULL_ATTRIBUTE(3);
AG_Prop	*AG_SetPointer(void *, const char *, void *);

AG_Prop		 *AG_GetProp(void *, const char *, enum ag_prop_type, void *);
__inline__ int	  AG_Bool(void *, const char *);
__inline__ u_int  AG_Uint(void *, const char *);
__inline__ int	  AG_Int(void *, const char *);
__inline__ Uint8  AG_Uint8(void *, const char *);
__inline__ Sint8  AG_Sint8(void *, const char *);
__inline__ Uint16 AG_Uint16(void *, const char *);
__inline__ Sint16 AG_Sint16(void *, const char *);
__inline__ Uint32 AG_Uint32(void *, const char *);
__inline__ Sint32 AG_Sint32(void *, const char *);
#ifdef FLOATING_POINT
__inline__ float  AG_Float(void *, const char *);
__inline__ double AG_Double(void *, const char *);
#endif
__inline__ void	*AG_Pointer(void *, const char *);
__inline__ char	*AG_String(void *, const char *);
size_t		 AG_StringCopy(void *, const char *, char *, size_t)
		     BOUNDED_ATTRIBUTE(__string__, 3, 4);
void		 AG_PropPrint(char *, size_t, AG_Prop *)
		     BOUNDED_ATTRIBUTE(__string__, 1, 2);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_PROP_H_ */
