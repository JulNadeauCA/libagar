/*	$Csoft: anim.h,v 1.8 2002/06/09 15:04:29 vedge Exp $	*/
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
int	 anim_add_frame(struct anim *, SDL_Surface *);
void	 anim_insert(struct object_art *, struct anim *, int);

