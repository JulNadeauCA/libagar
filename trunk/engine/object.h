/*	$Csoft: object.h,v 1.79 2003/05/24 15:53:39 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

#include <engine/prop.h>
#include <engine/physics.h>
#include <engine/media/art.h>

#include "begin_code.h"

struct map;
struct input;
struct event;

struct object_ops {
	void	(*init)(void *, char *);		/* Initialize */
	void	(*destroy)(void *);			/* Free resources */
	int	(*load)(void *, struct netbuf *);	/* Load from network */
	int	(*save)(void *, struct netbuf *);	/* Save to network */
	void	(*edit)(void *);			/* Edition */
};

struct object_table {
	struct object	**objs;
	Uint32		 nobjs;
	int		 *used;
};

struct object_position {
	struct map	*map;		/* Map (or NULL) */
	int		 x, y;		/* Map coordinates */
	int		 layer;		/* Current layer */
	int		 center;	/* Center the view around this? */
	struct map	*submap;	/* Current submap */
	struct input	*input;		/* Controller (or NULL) */
	struct mapdir	 dir;		/* Map direction (not saved) */
};

#define OBJECT_TYPE_MAX		32
#define OBJECT_NAME_MAX		64

TAILQ_HEAD(objectq, object);

struct object {
	char	 type[OBJECT_TYPE_MAX];	/* Type of object */
	char	 name[OBJECT_NAME_MAX];	/* Identifier */
	const struct object_ops	*ops;	/* Generic operations */
	void	*parent;		/* Parent object (linkage_lock) */

	Uint32	 flags;
#define OBJECT_RELOAD_PROPS	0x01	/* Don't remove props before load */
#define OBJECT_RELOAD_CHILDS	0x02	/* Don't remove childs before load */
#define OBJECT_STATIC		0x04	/* Parent should not call free(3) */

	struct art		*art;	/* Associated set of graphics */
	TAILQ_ENTRY(object)	 cobjs;	/* Child objects */

	pthread_mutex_t		 lock;
	struct object_position	*pos;		/* Position on a map */
	struct objectq		 childs;	/* Descendants (linkage_lock) */
	pthread_mutex_t		 events_lock;	/* XXX use lock? */
	TAILQ_HEAD(,event)	 events;	/* Event handlers */
	pthread_mutex_t		 props_lock;	/* XXX use lock? */
	TAILQ_HEAD(,prop)	 props;		/* Generic properties */
};

#define OBJECT(ob)	((struct object *)(ob))
#define OBJECT_ICON(ob)	(((ob)->art != NULL && (ob)->art->nsprites > 0) ? \
			 SPRITE(ob, 0) : NULL)

#ifdef DEBUG
# define OBJECT_ASSERT(ob, typestr) do {				\
	if (strcmp(OBJECT((ob))->type, typestr) != 0) {			\
		fatal("%s is not a %s", OBJECT((ob))->name, typestr);	\
	}								\
} while (0)
#else
# define OBJECT_ASSERT(ob, type)
#endif

__BEGIN_DECLS
extern DECLSPEC struct object	*object_new(void *, const char *, char *,
				            const void *);
extern DECLSPEC void		 object_init(void *, const char *, char *,
				             const void *);

extern DECLSPEC int	 object_load_art(void *, const char *, int);
extern DECLSPEC void	 object_attach(void *, void *);
extern DECLSPEC void	 object_detach(void *, void *);
extern DECLSPEC void	 object_move(void *, void *, void *);
extern DECLSPEC char	*object_name(void *);
extern DECLSPEC void	*object_find(void *, char *);
extern DECLSPEC int	 object_load(void *, char *);
extern DECLSPEC int	 object_save(void *, char *);
extern DECLSPEC void	 object_destroy(void *);
extern DECLSPEC void	 object_free_childs(struct object *);
extern DECLSPEC void	 object_free_props(struct object *);
extern DECLSPEC void 	 object_free_events(struct object *);
extern DECLSPEC int	 object_path(const char *, const char *, char *,
			             size_t);

extern DECLSPEC int	 object_control(void *, char *);
extern DECLSPEC void	 object_center(void *);
extern DECLSPEC void	 object_load_submap(void *, char *);
extern DECLSPEC int	 object_set_submap(void *, char *);

extern DECLSPEC void	 object_set_position(void *, struct map *, int, int,
			                     int);
extern DECLSPEC void	 object_unset_position(void *);
extern DECLSPEC void	 object_save_position(void *, struct netbuf *);
extern DECLSPEC int	 object_load_position(void *, struct netbuf *);

extern DECLSPEC void	 object_table_init(struct object_table *);
extern DECLSPEC void	 object_table_destroy(struct object_table *);
extern DECLSPEC void	 object_table_insert(struct object_table *,
			                     struct object *);
extern DECLSPEC void	 object_table_save(struct object_table *,
			                   struct netbuf *);
extern DECLSPEC int	 object_table_load(struct object_table *,
			                   struct netbuf *, char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_OBJECT_H */
