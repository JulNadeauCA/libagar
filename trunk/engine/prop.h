/*	$Csoft$	*/
/*	Public domain	*/

struct prop {
	const char *key;
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

struct prop	*prop_set(void *, const char *, enum prop_type, ...);
void		 prop_get(void *, const char *, enum prop_type, void *);

struct prop	*prop_set_bool(void *, const char *, int);
struct prop	*prop_set_int(void *, const char *, int);
struct prop	*prop_set_uint8(void *, const char *, Uint8);
struct prop	*prop_set_sint8(void *, const char *, Sint8);
struct prop	*prop_set_uint16(void *, const char *, Uint16);
struct prop	*prop_set_sint16(void *, const char *, Sint16);
struct prop	*prop_set_uint32(void *, const char *, Uint32);
struct prop	*prop_set_sint32(void *, const char *, Sint32);
struct prop	*prop_set_uint64(void *, const char *, Uint64);
struct prop	*prop_set_sint64(void *, const char *, Sint64);
struct prop	*prop_set_string(void *, const char *, char *, ...);
struct prop	*prop_set_pointer(void *, const char *, void *);

int		 prop_bool(void *, const char *);
int		 prop_int(void *, const char *);
Uint8	 	 prop_uint8(void *, const char *);
Sint8	 	 prop_sint8(void *, const char *);
Uint16	 	 prop_uint16(void *, const char *);
Sint16	 	 prop_sint16(void *, const char *);
Uint32	 	 prop_uint32(void *, const char *);
Sint32	 	 prop_sint32(void *, const char *);
Uint64	 	 prop_uint64(void *, const char *);
Sint64		 prop_sint64(void *, const char *);
char		*prop_string(void *, const char *);
void		*prop_pointer(void *, const char *);

