/*	$Csoft: anim.h,v 1.2 2002/02/08 00:18:01 vedge Exp $	*/

struct anim {
	SDL_Surface **frames;	/* Array of surfaces. */
	int	maxframes;	/* Allocated surfaces. */
	int	nframes;	/* Active surfaces. */
	int	delay;		/* Interval in milliseconds */
};

struct anim	*anim_create(int);
void		 anim_destroy(struct anim *);

int	anim_addframe(struct anim *, SDL_Surface *);

