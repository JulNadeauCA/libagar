/*	$Csoft: anim.h,v 1.6 2002/05/11 01:39:19 vedge Exp $	*/
/*	Public domain	*/

struct anim {
	SDL_Surface **frames;
	int	maxframes;
	int	frame, nframes;
	int	delta, delay;	/* For MAPREF_ANIM_DELTA */
};

void	 anim_init(struct anim *, int);
void	 anim_destroy(struct anim *);
int	 anim_addframe(struct anim *, SDL_Surface *);
int	 anim_breakframe(struct anim *, SDL_Surface *);

