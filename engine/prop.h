/*	$Csoft: prop.h,v 1.10 2003/02/25 03:58:07 vedge Exp $	*/
/*	Public domain	*/

#include <config/floating_point.h>
#include <config/have_ieee754.h>
#include <config/have_long_double.h>

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
#endif
#ifdef USE_LONG_DOUBLE
		long double	ld;
#endif
		char	*s;
		void	*p;
	} data;
	TAILQ_ENTRY(prop) props;
};

int	 prop_load(void *, int);
int	 prop_save(void *, int);
void	 prop_destroy(struct prop *);

struct prop	*prop_set(void *, char *, enum prop_type, ...);
struct prop	*prop_set_bool(void *, char *, int);
struct prop	*prop_set_int(void *, char *, int);
struct prop	*prop_set_uint8(void *, char *, Uint8);
struct prop	*prop_set_sint8(void *, char *, Sint8);
struct prop	*prop_set_uint16(void *, char *, Uint16);
struct prop	*prop_set_sint16(void *, char *, Sint16);
struct prop	*prop_set_uint32(void *, char *, Uint32);
struct prop	*prop_set_sint32(void *, char *, Sint32);
#ifdef FLOATING_POINT
struct prop	*prop_set_float(void *, char *, float);
struct prop	*prop_set_double(void *, char *, double);
#endif
#ifdef USE_LONG_DOUBLE
struct prop	*prop_set_long_double(void *, char *, long double);
#endif
struct prop	*prop_set_string(void *, char *, char *, ...);
struct prop	*prop_set_pointer(void *, char *, void *);

struct prop	*prop_get(void *, char *, enum prop_type, void *);
int		 prop_get_bool(void *, char *);
int		 prop_get_int(void *, char *);
Uint8	 	 prop_get_uint8(void *, char *);
Sint8	 	 prop_get_sint8(void *, char *);
Uint16	 	 prop_get_uint16(void *, char *);
Sint16	 	 prop_get_sint16(void *, char *);
Uint32	 	 prop_get_uint32(void *, char *);
Sint32	 	 prop_get_sint32(void *, char *);
#ifdef FLOATING_POINT
float		 prop_get_float(void *, char *);
double		 prop_get_double(void *, char *);
#endif
#ifdef USE_LONG_DOUBLE
long double	 prop_get_long_double(void *, char *);
#endif
char		*prop_get_string(void *, char *);
void		*prop_get_pointer(void *, char *);

