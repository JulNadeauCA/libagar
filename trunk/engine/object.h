/*	$Csoft: object.h,v 1.56 2002/11/27 05:11:24 vedge Exp $	*/
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
struct event;

struct object_ops {
	void	(*destroy)(void *);		/* Free resources */
	int	(*load)(void *, int);		/* Load from fd */
	int	(*save)(void *, int);		/* Save to fd */
};

struct object {
	/*
	 * Read-only once attached
	 */
	char	*type;			/* Type of object. */
	char	*name;			/* Key */
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

	struct media_art	*art;	/* Static sprites */
	struct media_audio	*audio;	/* Static samples */

	/*
	 * Read-write
	 */
	struct mappos		*pos;		/* Unique position on a map */
	pthread_mutex_t		 pos_lock;
	TAILQ_HEAD(, event)	 events;	/* Event handlers */
	pthread_mutex_t		 events_lock;
	pthread_mutexattr_t	 events_lockattr;
	TAILQ_HEAD(, prop)	 props;		/* Generic properties */
	pthread_mutex_t		 props_lock;

	SLIST_ENTRY(object) wobjs;		/* Parent's list */
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
		fprintf(stderr, "%s:%d: %s (%s) is not a %s\n",		\
		    __FILE__,						\
		    __LINE__, OBJECT((ob))->name, OBJECT((ob))->type,	\
		    typestr);						\
		abort();						\
	}								\
} while (/*CONSTCOND*/0)
#else
# define OBJECT_ASSERT(ob, type)
#endif

#define OBJECT_ISTYPE(pob, ptype) (strcmp(OBJECT((pob))->type, (ptype)) == 0)

struct object	*object_new(char *, char *, char *, int, const void *);
void		 object_init(struct object *, char *, char *, char *,
		     int, const void *);
int		 object_load(void *);
int		 object_load_from(void *, char *);
int		 object_save(void *);
void		 object_destroy(void *);
char		*object_name(const char *, int);
char		*object_path(char *, const char *);

struct mappos  *object_addpos(void *, Uint32, Uint32, struct input *,
		    struct map *, Uint32, Uint32, Uint32);
struct mappos  *object_movepos(void *, struct map *, Uint32, Uint32);
void		object_delpos(void *);
struct mappos  *object_get_pos(void *);

#endif	/* !_AGAR_OBJECT_H */
