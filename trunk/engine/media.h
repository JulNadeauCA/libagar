/*	$Csoft$	*/
/*	Public domain	*/

struct object;

struct media_art {
	/* Read-only when in pool */
	SDL_Surface **sprites;		/* Static surfaces */
	struct	anim **anims;		/* Animations */
	int	nsprites, maxsprites;
	int	nanims, maxanims;
	
	struct	 map *map;		/* Tile map (for edition purposes) */
	int	 mx, my;		/* Current position */
	Uint32	 cursprite, curanim,
		 curflags;

	char	*name;			/* Parent name copy */
	struct	 object *pobj;		/* Parent (for map edition only) */
	LIST_ENTRY(media_art) arts;	/* Art pool */

	/* Read-write, thread-safe */
	int	used;			/* Reference count */
	pthread_mutex_t used_lock;
};

struct media_audio_sample {
	SDL_AudioSpec spec;
	Uint8	*data;
	Uint32	 len;
};

struct media_audio {
	struct	media_audio_sample **samples;
	int	nsamples, maxsamples;
	char	*name;				/* Parent name copy */
	LIST_ENTRY(media_audio) audios;		/* Audio pool */

	/* Read-write, thread-safe */
	int	used;				/* Reference count */
	pthread_mutex_t used_lock;
};

#ifdef DEBUG
#define SPRITE(ob, i)	media_sprite((struct object *)(ob), (i))
#define ANIM(ob, i)	media_anim((struct object *)(ob), (i))
#define SAMPLE(ob, i)	media_sample((struct object *)(ob), (i))
#else
#define SPRITE(ob, i)	((struct object *)(ob))->art->sprites[(i)]
#define ANIM(ob, i)	((struct object *)(ob))->art->anims[(i)]
#define SAMPLE(ob, i)	((struct object *)(ob))->audio->samples[(i)]
#endif

struct media_art	*media_get_art(char *, struct object *);
struct media_audio	*media_get_audio(char *, struct object *);

void	media_add_sprite(struct media_art *, SDL_Surface *, Uint32, int);
void	media_break_sprite(struct media_art *, SDL_Surface *);

#ifdef DEBUG
SDL_Surface			*media_sprite(struct object *, int);
struct anim			*media_anim(struct object *, int);
struct media_audio_sample	*media_sample(struct object *, int);
#endif

void	 media_init_gc(void);
Uint32	 media_start_gc(Uint32, void *);
void	 media_destroy_gc(void);


