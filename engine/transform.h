/*	$Csoft: transform.h,v 1.4 2003/03/25 13:28:46 vedge Exp $	*/
/*	Public domain	*/

enum transform_type {
	TRANSFORM_HFLIP,
	TRANSFORM_VFLIP
};

struct transform {
	enum transform_type	type;
	void			(*func)(SDL_Surface **, int, Uint32 *);
	Uint32			*args;
	int			nargs;
	SLIST_ENTRY(transform)	transforms;
};

struct transform *transform_new(enum transform_type, int, Uint32 *);
int		  transform_init(struct transform *, enum transform_type,
		      int, Uint32 *);

int	transform_load(struct netbuf *, struct transform *);
void	transform_save(struct netbuf *, struct transform *);
void	transform_destroy(struct transform *);

__inline__ int	transform_compare(struct transform *, struct transform *);

