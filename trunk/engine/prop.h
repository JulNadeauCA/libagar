/*	$Csoft: prop.h,v 1.1 2002/09/05 12:16:04 vedge Exp $	*/
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
		Uint64	 u64;
		Sint64	 s64;
		char	*s;
		void	*p;
	} data;
	TAILQ_ENTRY(prop) props;
};

struct prop	*prop_set(void *, char *, enum prop_type, ...);
void		 prop_get(void *, char *, enum prop_type, void *);

struct prop	*prop_set_bool(void *, char *, int);
struct prop	*prop_set_int(void *, char *, int);
struct prop	*prop_set_uint8(void *, char *, Uint8);
struct prop	*prop_set_sint8(void *, char *, Sint8);
struct prop	*prop_set_uint16(void *, char *, Uint16);
struct prop	*prop_set_sint16(void *, char *, Sint16);
struct prop	*prop_set_uint32(void *, char *, Uint32);
struct prop	*prop_set_sint32(void *, char *, Sint32);
struct prop	*prop_set_uint64(void *, char *, Uint64);
struct prop	*prop_set_sint64(void *, char *, Sint64);
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
Uint64	 	 prop_uint64(void *, char *);
Sint64		 prop_sint64(void *, char *);
char		*prop_string(void *, char *);
void		*prop_pointer(void *, char *);

int		 prop_load(void *, int);
int		 prop_save(void *, int);

