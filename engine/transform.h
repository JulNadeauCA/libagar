/*	$Csoft: transform.h,v 1.1 2002/12/13 10:40:26 vedge Exp $	*/
/*	Public domain	*/

enum transform_type {
	TRANSFORM_HFLIP,
	TRANSFORM_VFLIP
};

struct transform {
	enum transform_type	type;
	SDL_Surface		*(*func)(SDL_Surface *, int, Uint32 *);
	Uint32			*args;
	int			nargs;
	SLIST_ENTRY(transform)	transforms;
};

int	transform_init(struct transform *, enum transform_type, int, Uint32 *);
int	transform_load(int, struct transform *);
void	transform_save(void *, struct transform *);
void	transform_destroy(struct transform *);
void	transform_copy(struct transform *, struct transform *);

