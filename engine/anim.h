/*	$Csoft: anim.h,v 1.11 2002/11/09 02:46:04 vedge Exp $	*/
/*	Public domain	*/

struct anim {
	struct object obj;		/* For error checking */

	SDL_Surface	***frames;
	int		maxframes;
	int		w, h;
	int		frame, nframes, nparts;
	int		delta, delay;	/* For MAPREF_ANIM_DELTA */
};

void	 anim_init(struct anim *, int);
void	 anim_destroy(void *);
int	 anim_add_frame(struct anim *, SDL_Surface *);
void	 anim_insert(struct media_art *, struct anim *, int);

