/*	$Csoft: object.h,v 1.50 2002/09/13 11:08:29 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

#include <engine/media.h>
#include <engine/prop.h>

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
	/*
	 * Read-only once attached
	 */
	char	*type;			/* Type of immediate descendent */
	char	*name;			/* Name string (key) */
	char	*desc;			/* Optional description */
	char	 saveext[4];		/* File extension for state saves */
	const struct object_ops	*ops;	/* Generic operations */
	int	 flags;
#define OBJECT_ART		0x01	/* Load graphics */
#define OBJECT_AUDIO		0x02	/* Load audio */
#define OBJECT_KEEP_MEDIA	0x04	/* Keep graphics/audio cached */
#define OBJECT_BLOCK		0x10	/* Map: cannot walk through. XXX */
#define OBJECT_MEDIA_CAN_FAIL	0x20	/* Media load can fail */
#define OBJECT_CANNOT_MAP	0x40	/* Never insert into map tables */
	enum {
		OBJECT_EMBRYONIC,	/* Unattached */
		OBJECT_CONSISTENT,	/* Attached */
		OBJECT_ZOMBIE		/* Detached */
	} state;
	struct	 media_art *art;	/* Static sprites */
	struct	 media_audio *audio;	/* Static samples */

	/*
	 * Read-write
	 */
	struct mappos		*pos;		/* Unique position on a map */
	pthread_mutex_t		 pos_lock;

	TAILQ_HEAD(, event)	 events;	/* Event handlers */
	pthread_mutex_t		 events_lock;
	pthread_mutexattr_t	 events_lockattr;

	TAILQ_HEAD(, prop)	 props;		/* Properties */
	pthread_mutex_t		 props_lock;

	/*
	 * Locking policy defined by the parent
	 */
	SLIST_ENTRY(object) wobjs;	/* Attached objects */
};

#define OBJECT(ob)	((struct object *)(ob))
#define OBJECT_OPS(ob)	(((struct object *)(ob))->ops)

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

struct	object *object_new(char *, char *, char *, int, const void *);
void	object_init(struct object *, char *, char *, char *, int, const void *);
int	object_load(void *);
int	object_load_from(void *, char *);
int	object_save(void *);
void	object_destroy(void *);

char		*object_name(const char *, int);
char		*object_path(char *, const char *);

struct mappos  *object_addpos(void *, Uint32, Uint32, struct input *,
		    struct map *, Uint32, Uint32);
struct mappos  *object_movepos(void *, struct map *, Uint32, Uint32);
void		object_delpos(void *);

#endif	/* !_AGAR_OBJECT_H */
