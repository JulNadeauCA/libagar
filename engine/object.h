/*	$Csoft: object.h,v 1.39 2002/06/08 08:04:05 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

struct map;
struct noderef;
struct mapdir;
struct mappos;
struct input;
struct anim;
struct event;

struct object_ops {
	void	(*destroy)(void *);		/* Free resources */
	int	(*load)(void *, int);		/* Load from fd */
	int	(*save)(void *, int);		/* Save to fd */

	/* Container functions. */
	void	(*attach)(void *, void *);	/* Attach child */
	void	(*detach)(void *, void *);	/* Detach child */
};

struct object_art {
	/* Read-only when in pool */
	SDL_Surface **sprites;		/* Static surfaces */
	struct	anim **anims;		/* Animations */
	int	nsprites, maxsprites;
	int	nanims, maxanims;

	char	*name;			/* Parent name copy */
	LIST_ENTRY(object_art) arts;	/* Art pool */

	/* Read-write, thread-safe */
	int	used;			/* Reference count */
	pthread_mutex_t used_lock;
};

struct object_audio_sample {
	SDL_AudioSpec spec;
	Uint8	*data;
	Uint32	 len;
};

struct object_audio {
	struct	object_audio_sample **samples;
	int	nsamples, maxsamples;
	char	*name;				/* Parent name copy */
	LIST_ENTRY(object_audio) audios;	/* Audio pool */

	/* Read-write, thread-safe */
	int	used;				/* Reference count */
	pthread_mutex_t used_lock;
};

struct obnoderef {
	struct	 object *pobj;
	Uint32	 flags;
#define OBNODEREF_SPRITE	0x0002
#define OBNODEREF_ANIM		0x0010
#define OBNODEREF_ANY		0x00ff
#define OBNODEREF_SAVE		0x0100
	Uint32	 offs;

	TAILQ_ENTRY(obnoderef) nrefs;
};

struct obnode {
	TAILQ_HEAD(, obnoderef) nrefs;	/* Node references */
	Uint32	 nnrefs;

	Uint32	 flags;
#define OBNODE_ORIGIN		0x01	/* Origin of this map */
#define OBNODE_WALKTHROUGH	0x02	/* Can walk through */
#define OBNODE_WALKBEHIND	0x04	/* Can walk behind */

	Uint32	 v1, v2;		/* Extra properties */
};

struct obmap {
	Uint32	 flags;
#define OBMAP_2D		0x0020	/* Two-dimensional */
	Uint32	 mapw, maph;		/* Map geometry */
	Uint32	 defx, defy;		/* Map origin */
	struct	 obnode **map;		/* Array of nodes */
};

struct object {
	/* Read-only once attached */

	char	*type;			/* Type of immediate descendent */
	char	*name;			/* Name string (key) */
	char	*desc;			/* Optional description */
	int	 id;			/* Unique identifier at runtime */
	char	 saveext[4];		/* File extension for state saves */
	const	 struct object_ops *ops; /* Generic functions. */

	int	 flags;
#define OBJ_ART		0x01		/* Load graphics */
#define OBJ_AUDIO	0x02		/* Load audio */
#define OBJ_KEEPMEDIA	0x04		/* Keep graphics/audio cached */
#define OBJ_BLOCK	0x10		/* Map: cannot walk through. XXX */

	struct	 object_art *art;	/* Static sprites */
	struct	 object_audio *audio;	/* Static samples */

	/* Read-write */
	struct	 obmap **maps;		/* Array of maps */
	pthread_mutex_t	maps_lock;
#if 1
	struct	 mappos *pos;		/* Position on the map. XXX array */
	pthread_mutex_t	pos_lock;
#endif

	TAILQ_HEAD(,event) events;	/* Event handlers */
	pthread_mutex_t	events_lock;

	/* Locking policy defined by the parent */
	SLIST_ENTRY(object) wobjs;	/* Attached objects */
};

#define OBJECT(ob)	((struct object *)(ob))
#define OBJECT_OPS(ob)	(((struct object *)(ob))->ops)

#define SPRITE(ob, sp)	OBJECT((ob))->art->sprites[(sp)]
#define ANIM(ob, sp)	OBJECT((ob))->art->anims[(sp)]
#define SAMPLE(ob, sp)	OBJECT((ob))->audio->samples[(sp)]

#define OBJECT_UNUSED(ob, type)	do {			\
	pthread_mutex_lock(&(ob)->type->used_lock);	\
	(ob)->type->used--;				\
	pthread_mutex_unlock(&(ob)->type->used_lock);	\
} while (/*CONSTCOND*/ 0)

#ifdef DEBUG
# define OBJECT_ASSERT(ob, typestr) do {				\
	if (strcmp(OBJECT((ob))->type, typestr) != 0) {			\
		fprintf(stderr, "%s:%d: %s is not a %s\n", __FILE__,	\
		    __LINE__, OBJECT((ob))->name, typestr);		\
		abort();						\
	}								\
} while (/*CONSTCOND*/0)
#else
# define OBJECT_ASSERT(ob, type)
#endif

#define OBJECT_TYPE(pob, ptype)	(strcmp(OBJECT((pob))->type, (ptype)) == 0)

struct object	*object_new(char *, char *, char *, int, const void *);
int		 object_load(void *);
int		 object_save(void *);

void	 object_init(struct object *, char *, char *, char *, int,
	     const void *);
void	 object_destroy(void *);
char	*object_name(char *, int);
int	 object_loadfrom(void *, char *);
int	 object_addanim(struct object_art *, struct anim *);
int	 object_addsprite(struct object_art *, SDL_Surface *);
int	 object_breaksprite(struct object_art *, SDL_Surface *);
void	 object_dump(void *);
char	*object_path(char *, const char *);

void	 object_init_gc(void);
Uint32	 object_start_gc(Uint32 ival, void *);
void	 object_destroy_gc(void);

struct object	*object_strfind(char *);
struct mappos	*object_addpos(void *, Uint32, Uint32, struct input *,
		    struct map *, Uint32, Uint32);
struct mappos	*object_movepos(void *, struct map *, int, int);
void		 object_delpos(void *);

#endif	/* _AGAR_OBJECT_H_ */
