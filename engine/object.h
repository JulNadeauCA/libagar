/*	$Csoft: object.h,v 1.87 2003/06/29 11:33:41 vedge Exp $	*/
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
	void	(*reinit)(void *);			/* Reinitialize */
	void	(*destroy)(void *);			/* Free resources */
	int	(*load)(void *, struct netbuf *);	/* Load from network */
	int	(*save)(void *, struct netbuf *);	/* Save to network */
	struct window *(*edit)(void *);			/* Edit object */
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

struct object_dep {
	struct object	*obj;		/* Object */
	Uint32		 count;		/* Reference count */
#define OBJECT_DEP_MAX	(0xffffffff-2)	/* Remain resident if reached */
	TAILQ_ENTRY(object_dep) deps;
};

#define OBJECT_TYPE_MAX		32
#define OBJECT_NAME_MAX		64
#define OBJECT_PATH_MAX		1024

TAILQ_HEAD(objectq, object);

struct object {
	char	 type[OBJECT_TYPE_MAX];		/* Type of object */
	char	 name[OBJECT_NAME_MAX];		/* Identifier */
	char	*save_pfx;			/* Save dir prefix */

	struct gfx	*gfx;			/* Associated graphics */
	struct audio	*audio;			/* Associated audio samples */
	
	const struct object_ops	*ops;		/* Generic operations */

	int	 flags;
#define OBJECT_RELOAD_PROPS	0x01	/* Don't free props before load */
#define OBJECT_NON_PERSISTENT	0x02	/* Never include in saves */
#define OBJECT_INDESTRUCTIBLE	0x04	/* Not destructible by user
					   (advisory flag only) */
#define OBJECT_SAVED_FLAGS	(OBJECT_RELOAD_PROPS|OBJECT_INDESTRUCTIBLE)

	pthread_mutex_t		 lock;
	struct object_position	*pos;		/* Position on a map */

	pthread_mutex_t		 events_lock;
	TAILQ_HEAD(,event)	 events;	/* Event handlers */
	pthread_mutex_t		 props_lock;
	TAILQ_HEAD(,prop)	 props;		/* Generic properties */

	/* Uses linkage_lock */
	TAILQ_HEAD(,object_dep)	 deps;		/* Object dependencies */
	struct objectq		 childs;	/* Descendants */
	void			*parent;	/* Parent object */
	TAILQ_ENTRY(object)	 cobjs;		/* Child objects */
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
struct object	*object_new(void *, const char *);
void		 object_init(void *, const char *, const char *, const void *);
void		 object_reinit(void *);

void	 object_set_type(void *, const char *);
void	 object_set_name(void *, const char *);
void	 object_set_ops(void *, const void *);

void	 object_attach(void *, void *);
void	 object_detach(void *, void *);
void	 object_move(void *, void *, void *);

int	 object_copy_name(const void *, char *, size_t);
int	 object_copy_filename(const void *, char *, size_t);

void		*object_find(const char *);
__inline__ void	*object_root(void *);
__inline__ int	 object_used(void *);

int	 object_load(void *);
int	 object_save(void *);
int	 object_destroy(void *);

#ifdef EDITION
struct window	*object_edit(void *);
#endif

int	 object_free_childs(struct object *);
void	 object_free_props(struct object *);
void 	 object_free_events(struct object *);
int	 object_path(const char *, const char *, char *, size_t);

int	 object_control(void *, const char *);
void	 object_center(void *);
void	 object_load_submap(void *, const char *);
int	 object_set_submap(void *, const char *);

void	 object_set_position(void *, struct map *, int, int, int);
void	 object_unset_position(void *);
void	 object_save_position(const void *, struct netbuf *);
int	 object_load_position(void *, struct netbuf *);

struct object_dep	 *object_add_dep(void *, void *);
__inline__ struct object *object_find_dep(const void *, Uint32);
Uint32			  object_dep_index(const void *, const struct object *);
void			  object_del_dep(void *, const void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_OBJECT_H */
