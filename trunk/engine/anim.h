/*	$Csoft: anim.h,v 1.5 2002/04/24 13:18:38 vedge Exp $	*/

struct anim {
	SDL_Surface **frames;
	int	maxframes;
	int	frame, nframes;
	int	delta, delay;	/* For MAPREF_ANIM_DELTA */
};

void	 anim_init(struct anim *, int);
void	 anim_destroy(struct anim *);
int	 anim_addframe(struct anim *, SDL_Surface *);

