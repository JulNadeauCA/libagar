/*	$Csoft: art.h,v 1.1 2002/12/01 14:41:05 vedge Exp $	*/
/*	Public domain	*/

struct art_anim {
	SDL_Surface	**frames;
	int		maxframes;
	int		frame, nframes;
	int		delta, delay;	/* For MAPREF_ANIM_DELTA */
};

/* Static image or animation */
struct art {
	/* Read-only when attached */
	char		*name;			/* Identifier */
	SDL_Surface	**sprites;		/* Static surfaces */
	struct art_anim	**anims;		/* Animations */
	int		 nsprites;		/* Loaded sprites */
	int		 maxsprites;		/* Allocated sprites */
	int		 nanims;		/* Loaded animations */
	int		 maxanims;		/* Allocated animations */
	struct map	*map;			/* Tile map */
	/* For tile map generation */
	int		 mx, my;		/* Current coordinates */
	Uint32		 cursprite;		/* Current sprite */
	Uint32		 curanim;		/* Current animation */
	struct object	*pobj;

	/* Read-write, thread-safe */
	int		used;			/* Reference count */
	pthread_mutex_t used_lock;

	TAILQ_ENTRY(art) arts;			/* Art pool */
};

#ifdef DEBUG
#define SPRITE(ob, i)	art_get_sprite((struct object *)(ob), (i))
#define ANIM(ob, i)	art_get_anim((struct object *)(ob), (i))
#else
#define SPRITE(ob, i)	((struct object *)(ob))->art->sprites[(i)]
#define ANIM(ob, i)	((struct object *)(ob))->art->anims[(i)]
#endif

struct art	*art_fetch(char *, struct object *);
void		 art_unused(struct art *);

int		 art_insert_sprite(struct art *, SDL_Surface *, int);
void		 art_insert_sprite_tiles(struct art *, SDL_Surface *);

struct art_anim	*art_insert_anim(struct art *, int);
void		 art_insert_anim_frame(struct art_anim *, SDL_Surface *);
void		 art_anim_tick(struct art_anim *, void *);

#ifdef DEBUG
SDL_Surface	*art_get_sprite(struct object *, int);
struct art_anim	*art_get_anim(struct object *, int);
struct window	*art_browser_window(void);
#endif

