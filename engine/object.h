/*	$Csoft: object.h,v 1.85 2003/06/25 06:14:03 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

#include <engine/prop.h>
#include <engine/physics.h>
#include <engine/gfx.h>
#include <engine/audio.h>

#include "begin_code.h"

struct map;
struct input;
struct event;

struct object_ops {
	void	(*init)(void *, const char *);		/* Initialize */
	void	(*destroy)(void *);			/* Free resources */
	int	(*load)(void *, struct netbuf *);	/* Load from network */
	int	(*save)(void *, struct netbuf *);	/* Save to network */
	void	(*edit)(void *);			/* Edition */
};

struct object_table {
	struct object	**objs;
	Uint32		 nobjs;
	Uint32		*used;
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

	struct gfx		*gfx;	/* Associated graphics */
	struct audio		*audio;	/* Associated audio samples */
	TAILQ_ENTRY(object)	 cobjs;	/* Child objects */

	pthread_mutex_t		 lock;
	struct object_position	*pos;		/* Position on a map */
	struct objectq		 childs;	/* Descendants (linkage_lock) */
	pthread_mutex_t		 events_lock;
	TAILQ_HEAD(,event)	 events;	/* Event handlers */
	pthread_mutex_t		 props_lock;
	TAILQ_HEAD(,prop)	 props;		/* Generic properties */
};

#define OBJECT(ob)	((struct object *)(ob))
#define OBJECT_ICON(ob)	(((ob)->gfx != NULL && (ob)->gfx->nsprites > 0) ? \
			 SPRITE(ob, 0) : NULL)

#define OBJECT_NAME(ob, n)	(strcmp(OBJECT(ob)->name, (n)) == 0)
#define OBJECT_TYPE(ob, t)	(strcmp(OBJECT(ob)->type, (t)) == 0)

#define OBJECT_FOREACH_CHILD(var, ob, type)				\
	for((var) = (struct type *)TAILQ_FIRST(&OBJECT(ob)->childs);	\
	    (var) != (struct type *)TAILQ_END(&OBJECT(ob)->childs);	\
	    (var) = (struct type *)TAILQ_NEXT(OBJECT(var), cobjs))

#define OBJECT_FOREACH_CHILD_REVERSE(var, ob, type)			\
	for((var) = (struct type *)TAILQ_LAST(&OBJECT(ob)->childs, objectq);\
	    (var) != (struct type *)TAILQ_END(&OBJECT(ob)->childs);	\
	    (var) = (struct type *)TAILQ_PREV(OBJECT(var), objectq, cobjs))

__BEGIN_DECLS
struct object	*object_new(void *, const char *, const char *, const void *);
void		 object_init(void *, const char *, const char *, const void *);
void		 object_set_ops(void *, const void *);

void	 object_attach(void *, void *);
void	 object_detach(void *, void *);
void	 object_move(void *, void *, void *);
char	*object_name(void *);
void	*object_find(void *, const char *);

int	 object_load(void *);
int	 object_save(void *);
void	 object_destroy(void *);
#ifdef EDITION
void	 object_edit(void *);
#endif

void	 object_free_childs(struct object *);
void	 object_free_props(struct object *);
void 	 object_free_events(struct object *);
int	 object_path(const char *, const char *, char *, size_t);

int	 object_control(void *, const char *);
void	 object_center(void *);
void	 object_load_submap(void *, const char *);
int	 object_set_submap(void *, const char *);

void	 object_set_position(void *, struct map *, int, int, int);
void	 object_unset_position(void *);
void	 object_save_position(void *, struct netbuf *);
int	 object_load_position(void *, struct netbuf *);

void	 object_table_init(struct object_table *);
void	 object_table_destroy(struct object_table *);
void	 object_table_insert(struct object_table *, struct object *);
void	 object_table_save(struct object_table *, struct netbuf *);
int	 object_table_load(struct object_table *, struct netbuf *,
	                   const char *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_OBJECT_H */
