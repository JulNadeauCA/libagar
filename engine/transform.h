/*	$Csoft: transform.h,v 1.3 2003/03/24 12:05:37 vedge Exp $	*/
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

int	transform_load(int, struct transform *);
void	transform_save(void *, struct transform *);
void	transform_destroy(struct transform *);

__inline__ int	transform_compare(struct transform *, struct transform *);

