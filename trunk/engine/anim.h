/*	$Csoft: anim.h,v 1.4 2002/03/12 13:58:11 vedge Exp $	*/

struct anim {
	SDL_Surface **frames;
	Uint32	maxframes;
	Uint32	frame, nframes;
	Uint32	delta, delay;	/* For MAPREF_ANIM_DELTA */
};

void	 anim_init(struct anim *, int);
void	 anim_destroy(struct anim *);
int	 anim_addframe(struct anim *, SDL_Surface *);

