/*	$Csoft: anim.h,v 1.3 2002/02/08 00:37:40 vedge Exp $	*/

struct anim {
	SDL_Surface **frames;
	Uint32	maxframes;
	Uint32	frame, nframes;
	Uint32	delta, delay;	/* For MAPREF_ANIM_DELTA */
};

struct anim	*anim_create(int);
void		 anim_destroy(struct anim *);

int	anim_addframe(struct anim *, SDL_Surface *);

