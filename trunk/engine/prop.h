/*	$Csoft: prop.h,v 1.4 2002/12/08 07:57:48 vedge Exp $	*/
/*	Public domain	*/

struct prop {
	char *key;
	enum prop_type {
		PROP_INT,
		PROP_UINT8,
		PROP_SINT8,
		PROP_UINT16,
		PROP_SINT16,
		PROP_UINT32,
		PROP_SINT32,
		PROP_UINT64,
		PROP_SINT64,
		PROP_FLOAT,
		PROP_DOUBLE,
		PROP_LONG_DOUBLE,
		PROP_STRING,
		PROP_POINTER,
		PROP_BOOL
	} type;
	union {
		int	 i;
		Uint8	 u8;
		Sint8	 s8;
		Uint16	 u16;
		Sint16	 s16;
		Uint32	 u32;
		Sint32	 s32;
#ifdef SDL_HAS_64BIT_TYPE
		Uint64	 u64;
		Sint64	 s64;
#endif
#ifdef HAVE_IEEE754
		float	 	f;
		double	 	d;
# ifdef HAVE_LONG_DOUBLE
		long double	ld;
# endif
#endif
		char	*s;
		void	*p;
	} data;
	TAILQ_ENTRY(prop) props;
};

struct prop	*prop_set(void *, char *, enum prop_type, ...);
int		 prop_get(void *, char *, enum prop_type, void *);

struct prop	*prop_set_bool(void *, char *, int);
struct prop	*prop_set_int(void *, char *, int);
struct prop	*prop_set_uint8(void *, char *, Uint8);
struct prop	*prop_set_sint8(void *, char *, Sint8);
struct prop	*prop_set_uint16(void *, char *, Uint16);
struct prop	*prop_set_sint16(void *, char *, Sint16);
struct prop	*prop_set_uint32(void *, char *, Uint32);
struct prop	*prop_set_sint32(void *, char *, Sint32);
#ifdef SDL_HAS_64BIT_TYPE
struct prop	*prop_set_uint64(void *, char *, Uint64);
struct prop	*prop_set_sint64(void *, char *, Sint64);
#endif

#ifdef HAVE_IEEE754
struct prop	*prop_set_float(void *, char *, float);
struct prop	*prop_set_double(void *, char *, double);
# ifdef HAVE_LONG_DOUBLE
struct prop	*prop_set_long_double(void *, char *, long double);
# endif
#endif

struct prop	*prop_set_string(void *, char *, char *, ...);
struct prop	*prop_set_pointer(void *, char *, void *);

int		 prop_bool(void *, char *);
int		 prop_int(void *, char *);
Uint8	 	 prop_uint8(void *, char *);
Sint8	 	 prop_sint8(void *, char *);
Uint16	 	 prop_uint16(void *, char *);
Sint16	 	 prop_sint16(void *, char *);
Uint32	 	 prop_uint32(void *, char *);
Sint32	 	 prop_sint32(void *, char *);
#ifdef SDL_HAS_64BIT_TYPE
Uint64	 	 prop_uint64(void *, char *);
Sint64		 prop_sint64(void *, char *);
#endif
#ifdef HAVE_IEEE754
float		 prop_float(void *, char *);
double		 prop_double(void *, char *);
# ifdef HAVE_LONG_DOUBLE
long double	 prop_long_double(void *, char *);
# endif
#endif /* HAVE_IEEE754 */
char		*prop_string(void *, char *);
void		*prop_pointer(void *, char *);

int		 prop_load(void *, int);
int		 prop_save(void *, int);

