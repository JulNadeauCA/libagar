/*	$Csoft$	*/

struct anim {
	GSList	*frames;
	int	nframes;
	int	delay;		/* Interval in milliseconds */
	int	gframe;		/* Used by the map editor */
	int	gframedc;

	pthread_mutex_t lock;
};

struct anim	*anim_create(int);
void		 anim_destroy(struct anim *);

