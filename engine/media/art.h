/*	$Csoft: art.h,v 1.9 2003/03/22 04:21:47 vedge Exp $	*/
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

struct art_cached_sprite {
	SDL_Surface	*su;			/* Modified sprite */
	Uint32		 last_drawn;		/* Time last draw occured */
	SLIST_HEAD(,transform)	transforms;	/* Applied transforms */
	SLIST_ENTRY(art_cached_sprite) sprites;
};

struct art_cached_anim {
	struct art_anim	*anim;			/* Modified anim */
	Uint32		 last_drawn;		/* Time last draw occured */
	SLIST_HEAD(,transform)	transforms;	/* Applied transforms */
	SLIST_ENTRY(art_cached_anim) anims;
};

struct art_spritecl {
	SLIST_HEAD(,art_cached_sprite)	sprites;
};

struct art_animcl {
	SLIST_HEAD(,art_cached_anim)	anims;
};

struct art {
	char		  *name;		/* Shared identifier */
	struct object	  *pobj;		/* For submap refs */

	SDL_Surface		 **sprites;	/* Static images */
	struct art_spritecl	 *csprites;	/* Sprite transform cache */
	Uint32			  nsprites;
	Uint32			maxsprites;
	struct art_anim		 **anims;	/* Animations */
	struct art_animcl	 *canims;	/* Anim transform cache */
	Uint32			  nanims;
	Uint32			maxanims;

	struct map	  *tile_map;	/* User map of source nodes */
	struct map	 **submaps;	/* Sprite fragment maps */
	Uint32		  nsubmaps;
	Uint32		maxsubmaps;

	pthread_mutex_t	 used_lock;
	Uint32		 used;		/* Reference count */
#define ART_MAX_USED	 (0xffffffff-1)
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
void		 art_wire(struct art *);
void		 art_scan_alpha(SDL_Surface *);

Uint32		 art_insert_sprite(struct art *, SDL_Surface *, int);
struct map	*art_insert_fragments(struct art *, SDL_Surface *);
Uint32		 art_insert_submap(struct art *, struct map *);

struct art_anim	*art_insert_anim(struct art *, int);
Uint32		 art_insert_anim_frame(struct art_anim *, SDL_Surface *);
void		 art_anim_tick(struct art_anim *, struct noderef *);

#ifdef DEBUG
SDL_Surface	*art_get_sprite(struct object *, Uint32);
struct art_anim	*art_get_anim(struct object *, Uint32);
struct window	*art_browser_window(void);
#endif

