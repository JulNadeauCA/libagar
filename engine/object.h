/*	$Csoft: object.h,v 1.70 2003/03/22 04:26:02 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

#include <engine/prop.h>
#include <engine/physics.h>
#include <engine/media/art.h>

struct map;
struct noderef;
struct input;
struct event;

/* Generic object operations */
struct object_ops {
	void	(*destroy)(void *);			/* Free resources */
	int	(*load)(void *, struct netbuf *);	/* Load from network */
	int	(*save)(void *, struct netbuf *);	/* Save to network */
};

/* Object dependency table */
struct object_table {
	struct object	**objs;
	Uint32		 nobjs;
	int		 *used;
};

/* Unique position in the world */
struct object_position {
	struct map	*map;		/* Map (or NULL) */
	int		 x, y;		/* Map coordinates */
	int		 layer;		/* Current layer */
	int		 center;	/* Center the view around this */
	struct map	*submap;	/* Current submap */
	struct input	*input;		/* Controller (or NULL) */
	struct mapdir	 dir;		/* Map direction (not saved) */
};

#define OBJECT_TYPE_MAX		128
#define OBJECT_NAME_MAX		256

struct object {
	char			*type;	/* Type of object */
	char			*name;	/* Identification string */
	const struct object_ops	*ops;	/* Generic operations */
	int			 flags;
#define OBJECT_ART		0x001	/* Load graphics */
#define OBJECT_ART_CACHE	0x002	/* Keep graphics cached */
#define OBJECT_ART_CAN_FAIL	0x004	/* Graphic load can fail */
#define OBJECT_AUDIO		0x008	/* Load audio */
#define OBJECT_AUDIO_CACHE	0x010	/* Keep audio cached */
#define OBJECT_AUDIO_CAN_FAIL	0x020	/* Audio load can fail */
#define OBJECT_CANNOT_MAP	0x040	/* Don't insert in object tables */
#define OBJECT_STATIC		0x080	/* Don't destroy (if attached) */
#define OBJECT_RELOAD_PROPS	0x100	/* Don't clear props list before load */
	enum {
		OBJECT_EMBRYONIC,	/* Inconsistent/Unattached */
		OBJECT_CONSISTENT,	/* Attached */
		OBJECT_DETACHING,	/* Detach in progress */
		OBJECT_DETACHED		/* Inconsistent/Detached */
	} state;
	struct art		*art;	/* Graphical data (independent) */
	SLIST_ENTRY(object)	 wobjs;	/* Parent's list */

	pthread_mutex_t		 lock;
	struct object_position	*pos;		/* Position on a map */
	SLIST_HEAD(, object)	 childs;	/* Child objects */

	pthread_mutex_t		 events_lock;
	TAILQ_HEAD(, event)	 events;	/* Event handlers */
	pthread_mutex_t		 props_lock;
	TAILQ_HEAD(, prop)	 props;		/* Generic properties */
};

#define OBJECT(ob)	((struct object *)(ob))

#ifdef DEBUG
# define OBJECT_ASSERT(ob, typestr) do {				\
	if (strcmp(OBJECT((ob))->type, typestr) != 0) {			\
		fatal("%s is not a %s", OBJECT((ob))->name, typestr);	\
	}								\
} while (0)
#else
# define OBJECT_ASSERT(ob, type)
#endif

struct object	*object_new(void *, char *, char *, char *, int, const void *);
void		 object_init(struct object *, char *, char *, char *,
		     int, const void *);
void		 object_attach(void *, void *);
void		 object_detach(void *, void *);
int		 object_load(void *, char *);
int		 object_save(void *, char *);
void		 object_destroy(void *);
void		 object_free_childs(struct object *);
void		 object_free_props(struct object *);
void	 	 object_free_events(struct object *);
int		 object_path(char *, const char *, char *, size_t);

int		 object_control(void *, char *);
void		 object_center(void *);
void		 object_load_submap(void *, char *);
int		 object_set_submap(void *, char *);

void		 object_set_position(void *, struct map *, int, int, int);
void		 object_unset_position(void *);
void		 object_save_position(void *, struct netbuf *);
int		 object_load_position(void *, struct netbuf *);

void	 object_table_init(struct object_table *);
void	 object_table_destroy(struct object_table *);
void	 object_table_insert(struct object_table *, struct object *);
void	 object_table_save(struct object_table *, struct netbuf *);
int	 object_table_load(struct object_table *, struct netbuf *, char *);

#endif	/* _AGAR_OBJECT_H */
