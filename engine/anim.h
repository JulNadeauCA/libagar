/*	$Csoft: anim.h,v 1.7 2002/06/09 10:08:04 vedge Exp $	*/
/*	Public domain	*/

struct anim {
	SDL_Surface ***frames;
	int	maxframes;
	int	w, h;
	int	frame, nframes, nparts;
	int	delta, delay;	/* For MAPREF_ANIM_DELTA */
};

void	 anim_init(struct anim *, int);
void	 anim_destroy(struct anim *);
int	 anim_addframe(struct anim *, SDL_Surface *);

