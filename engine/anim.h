/*	$Csoft: anim.h,v 1.1 2002/02/05 05:47:17 vedge Exp $	*/

struct anim {
	SDL_Surface **frames;	/* Array of surfaces. */
	int	maxframes;	/* Allocated surfaces. */
	int	nframes;	/* Active surfaces. */

	int	delay;		/* Interval in milliseconds */

	/* XXX used by the map editor. */
	int	gframe;
	int	gframedc;
};

struct anim	*anim_create(int);
void		 anim_destroy(struct anim *);

int	anim_addframe(struct anim *, SDL_Surface *);

