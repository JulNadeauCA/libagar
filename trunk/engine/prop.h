/*	$Csoft: prop.h,v 1.21 2003/06/15 05:08:39 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PROP_H_
#define _AGAR_PROP_H_
#include "begin_code.h"

#include <config/floating_point.h>
#include <config/have_ieee754.h>
#include <config/have_long_double.h>

#define PROP_KEY_MAX		64
#define PROP_STRING_MAX		65535
#define PROP_UNICODE_MAX	32767

enum prop_type {
	PROP_UINT,
	PROP_INT,
	PROP_UINT8,
	PROP_SINT8,
	PROP_UINT16,
	PROP_SINT16,
	PROP_UINT32,
	PROP_SINT32,
	PROP_UINT64,		/* Unused */
	PROP_SINT64,		/* Unused */
	PROP_FLOAT,		/* IEEE 754 encoding */
	PROP_DOUBLE,		/* IEEE 754 encoding */
	PROP_LONG_DOUBLE,	/* Unused */
	PROP_STRING,
	PROP_POINTER,
	PROP_BOOL,
	PROP_UNICODE,		/* UTF-16 encoding */
	PROP_ANY
};

struct prop {
	char	 key[PROP_KEY_MAX];
	int	 type;
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
		Uint16	*ucs;
	} data;
	TAILQ_ENTRY(prop) props;
};

__BEGIN_DECLS
__inline__ void	 prop_lock(void *);
__inline__ void	 prop_unlock(void *);
int		 prop_load(void *, struct netbuf *);
int		 prop_save(void *, struct netbuf *);
void		 prop_destroy(struct prop *);

struct prop	*prop_copy(const struct prop *);
struct prop	*prop_set(void *, const char *, enum prop_type, ...);
struct prop	*prop_set_bool(void *, const char *, int);
struct prop	*prop_set_uint(void *, const char *, unsigned int);
struct prop	*prop_set_int(void *, const char *, int);
struct prop	*prop_set_uint8(void *, const char *, Uint8);
struct prop	*prop_set_sint8(void *, const char *, Sint8);
struct prop	*prop_set_uint16(void *, const char *, Uint16);
struct prop	*prop_set_sint16(void *, const char *, Sint16);
struct prop	*prop_set_uint32(void *, const char *, Uint32);
struct prop	*prop_set_sint32(void *, const char *, Sint32);
#ifdef FLOATING_POINT
struct prop	*prop_set_float(void *, const char *, float);
struct prop	*prop_set_double(void *, const char *, double);
#endif
struct prop	*prop_set_string(void *, const char *, const char *, ...);
struct prop	*prop_set_unicode(void *, const char *, Uint16 *);
struct prop	*prop_set_pointer(void *, const char *, void *);

struct prop	*prop_get(void *, const char *, enum prop_type, void *);
int		 prop_get_bool(void *, const char *);
unsigned int	 prop_get_uint(void *, const char *);
int		 prop_get_int(void *, const char *);
Uint8	 	 prop_get_uint8(void *, const char *);
Sint8	 	 prop_get_sint8(void *, const char *);
Uint16	 	 prop_get_uint16(void *, const char *);
Sint16	 	 prop_get_sint16(void *, const char *);
Uint32	 	 prop_get_uint32(void *, const char *);
Sint32	 	 prop_get_sint32(void *, const char *);
#ifdef FLOATING_POINT
float		 prop_get_float(void *, const char *);
double		 prop_get_double(void *, const char *);
#endif
char		*prop_get_string(void *, const char *);
Uint16		*prop_get_unicode(void *, const char *);
size_t		 prop_copy_string(void *, const char *, char *, size_t);
size_t		 prop_copy_unicode(void *, const char *, Uint16 *, size_t);
void		*prop_get_pointer(void *, const char *);

void		 prop_print_value(char *, size_t, struct prop *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_PROP_H_ */
