/*	$Csoft: prop.h,v 1.17 2003/05/06 01:03:40 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PROP_H_
#define _AGAR_PROP_H_
#include "begin_code.h"

#include <config/floating_point.h>
#include <config/have_ieee754.h>
#include <config/have_long_double.h>

#define PROP_KEY_MAX		256
#define PROP_STRING_MAX		65536

struct prop {
	char *key;
	enum prop_type {		/* Sync monitor/object_browser.c */
		PROP_INT,
		PROP_UINT8,
		PROP_SINT8,
		PROP_UINT16,
		PROP_SINT16,
		PROP_UINT32,
		PROP_SINT32,
		PROP_UINT64_UNUSED,
		PROP_SINT64_UNUSED,
		PROP_FLOAT,
		PROP_DOUBLE,
		PROP_LONG_DOUBLE,
		PROP_STRING,
		PROP_POINTER,
		PROP_BOOL,
		PROP_ANY
	} type;
	union {
		int	 i;
		Uint8	 u8;
		Sint8	 s8;
		Uint16	 u16;
		Sint16	 s16;
		Uint32	 u32;
		Sint32	 s32;
#ifdef FLOATING_POINT
		float	 	f;
		double	 	d;
# ifdef USE_LONG_DOUBLE
		long double	ld;
# endif
#endif
		char	*s;
		void	*p;
	} data;
	TAILQ_ENTRY(prop) props;
};

__BEGIN_DECLS
extern __inline__ void		 prop_lock(void *);
extern __inline__ void		 prop_unlock(void *);
extern DECLSPEC int		 prop_load(void *, struct netbuf *);
extern DECLSPEC int		 prop_save(void *, struct netbuf *);
extern DECLSPEC void		 prop_destroy(struct prop *);
extern DECLSPEC struct prop	*prop_set(void *, const char *, enum prop_type,
				          ...);
extern DECLSPEC struct prop	*prop_set_bool(void *, const char *, int);
extern DECLSPEC struct prop	*prop_set_int(void *, const char *, int);
extern DECLSPEC struct prop	*prop_set_uint8(void *, const char *, Uint8);
extern DECLSPEC struct prop	*prop_set_sint8(void *, const char *, Sint8);
extern DECLSPEC struct prop	*prop_set_uint16(void *, const char *, Uint16);
extern DECLSPEC struct prop	*prop_set_sint16(void *, const char *, Sint16);
extern DECLSPEC struct prop	*prop_set_uint32(void *, const char *, Uint32);
extern DECLSPEC struct prop	*prop_set_sint32(void *, const char *, Sint32);
#ifdef FLOATING_POINT
extern DECLSPEC struct prop	*prop_set_float(void *, const char *, float);
extern DECLSPEC struct prop	*prop_set_double(void *, const char *, double);
# ifdef USE_LONG_DOUBLE
extern DECLSPEC struct prop	*prop_set_long_double(void *, const char *,
				                      long double);
# endif
#endif
extern DECLSPEC struct prop	*prop_set_string(void *, const char *,
				                 const char *, ...);
extern DECLSPEC struct prop	*prop_set_pointer(void *, const char *, void *);

extern DECLSPEC struct prop	*prop_get(void *, const char *, enum prop_type,
				          void *);
extern DECLSPEC int		 prop_get_bool(void *, const char *);
extern DECLSPEC int		 prop_get_int(void *, const char *);
extern DECLSPEC Uint8	 	 prop_get_uint8(void *, const char *);
extern DECLSPEC Sint8	 	 prop_get_sint8(void *, const char *);
extern DECLSPEC Uint16	 	 prop_get_uint16(void *, const char *);
extern DECLSPEC Sint16	 	 prop_get_sint16(void *, const char *);
extern DECLSPEC Uint32	 	 prop_get_uint32(void *, const char *);
extern DECLSPEC Sint32	 	 prop_get_sint32(void *, const char *);
#ifdef FLOATING_POINT
extern DECLSPEC float		 prop_get_float(void *, const char *);
extern DECLSPEC double		 prop_get_double(void *, const char *);
#ifdef USE_LONG_DOUBLE
extern DECLSPEC long double	 prop_get_long_double(void *, const char *);
# endif
#endif
extern DECLSPEC char	*prop_get_string(void *, const char *);
extern DECLSPEC size_t	 prop_copy_string(void *, const char *, char *, size_t);
extern DECLSPEC void	*prop_get_pointer(void *, const char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_PROP_H_ */
