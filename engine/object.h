/*	$Csoft: object.h,v 1.66 2003/02/04 02:23:20 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

#include <engine/prop.h>
#include <engine/media/art.h>

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

struct object_table {
	struct object	**objs;
	Uint32		 nobjs;
	int		 *used;
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
#define OBJECT_ART_CACHE	0x02	/* Keep graphics cached */
#define OBJECT_ART_CAN_FAIL	0x04	/* Graphic load can fail */
#define OBJECT_AUDIO		0x08	/* Load audio */
#define OBJECT_AUDIO_CACHE	0x10	/* Keep audio cached */
#define OBJECT_AUDIO_CAN_FAIL	0x20	/* Audio load can fail */
#define OBJECT_CANNOT_MAP	0x40	/* Don't insert in object tables */
#define OBJECT_SYSTEM		0x80

	enum {
		OBJECT_EMBRYONIC,	/* Inconsistent/Unattached */
		OBJECT_CONSISTENT,	/* Attached */
		OBJECT_DETACHING,	/* Detach in progress */
		OBJECT_DETACHED		/* Inconsistent/Detached */
	} state;

	struct art	*art;		/* Static sprites */
#if 0
	struct audio	*audio;		/* Samples */
#endif
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
	pthread_mutexattr_t	 props_lockattr;

	SLIST_ENTRY(object) wobjs;		/* Parent's list */
};

#define OBJECT(ob)	((struct object *)(ob))
#define OBJECT_OPS(ob)	(((struct object *)(ob))->ops)

#ifdef DEBUG
# define OBJECT_ASSERT(ob, typestr) do {				\
	if (strcmp(OBJECT((ob))->type, typestr) != 0) {			\
		fatal("%s is not a %s\n", OBJECT((ob))->name, typestr);	\
	}								\
} while (/*CONSTCOND*/0)
#else
# define OBJECT_ASSERT(ob, type)
#endif

struct object	*object_new(char *, char *, char *, int, const void *);
void		 object_init(struct object *, char *, char *, char *,
		     int, const void *);
int		 object_load(void *);
int		 object_load_from(void *, char *);
int		 object_save(void *);
void		 object_destroy(void *);
char		*object_name(const char *, int);
char		*object_path(char *, const char *);
void		 object_control(void *, struct input *, int);
void		 object_set_position(void *, struct noderef *, struct map *,
		     int, int);
void		 object_move(void *, struct map *, int, int);
int		 object_vanish(void *);

struct object_table	*object_table_new(void);
void			 object_table_destroy(struct object_table *);
void			 object_table_insert(struct object_table *,
			     struct object *);
void			 object_table_save(struct fobj_buf *,
			     struct object_table *);
struct object_table	*object_table_load(int, char *);

#endif	/* !_AGAR_OBJECT_H */
