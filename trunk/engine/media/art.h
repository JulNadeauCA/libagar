/*	$Csoft: art.h,v 1.3 2002/12/31 00:59:01 vedge Exp $	*/
/*	Public domain	*/

struct object;
struct noderef;

struct art_anim {
	SDL_Surface	 **frames;
	Uint32		  nframes;
	Uint32		maxframes;
	Uint32		   frame;
	int		   delta;
	int		   delay;
};

struct art {
	char		  *name;	/* Identifier */
	struct object	  *pobj;	/* For submap refs */

	SDL_Surface	 **sprites;	/* Static images */
	Uint32		  nsprites;
	Uint32		maxsprites;
	struct art_anim	 **anims;	/* Animations */
	Uint32		  nanims;
	Uint32		maxanims;
	struct map	 **submaps;	/* Fragment maps (for map edition) */
	Uint32		  nsubmaps;
	Uint32		maxsubmaps;

	struct map	*tile_map;	/* User map of source nodes */

	pthread_mutex_t	 used_lock;
	int		 used;		/* Reference count */
	TAILQ_ENTRY(art) arts;		/* Art pool */
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
struct map	*art_insert_fragments(struct art *, SDL_Surface *);
Uint32		 art_insert_submap(struct art *, struct map *);

struct art_anim	*art_insert_anim(struct art *, int);
Uint32		 art_insert_anim_frame(struct art_anim *, SDL_Surface *);
void		 art_anim_tick(struct art_anim *, struct noderef *);

#ifdef DEBUG
SDL_Surface	*art_get_sprite(struct object *, int);
struct art_anim	*art_get_anim(struct object *, int);
struct window	*art_browser_window(void);
#endif

