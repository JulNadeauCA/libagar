/*	$Csoft: object.h,v 1.90 2003/08/21 04:26:41 vedge Exp $	*/
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

enum object_page_item {
	OBJECT_GFX,		/* Shared graphics */
	OBJECT_AUDIO,		/* Shared audio samples */
	OBJECT_DATA		/* Object data */
};

struct object_ops {
	void	(*init)(void *, const char *);		/* Initialize */
	void	(*reinit)(void *);			/* Reinitialize */
	void	(*destroy)(void *);			/* Free resources */
	int	(*load)(void *, struct netbuf *);	/* Load from network */
	int	(*save)(void *, struct netbuf *);	/* Save to network */
	struct window *(*edit)(void *);			/* Edit object */
};

/*
 * If the object occupies a unique position on a map, this structure
 * describes its coordinates, which submap to display for a given
 * orientation/status, and how the position is controlled.
 */
struct object_position {
	struct map	*map;		/* Map (or NULL) */
	int		 x, y;		/* Map coordinates */
	int		 layer;		/* Current layer */
	int		 center;	/* Center the view around this? */
	struct map	*submap;	/* Current submap */
	struct input	*input;		/* Controller (or NULL) */
	struct mapdir	 dir;		/* Map direction (not saved) */
};

/*
 * This structure describes a dependency of an object toward another object.
 * If the count reaches OBJECT_DEP_MAX, the dependency cannot be removed
 * before the object is destroyed (object_del_dep() becomes a no-op).
 *
 * XXX we could use 64-bit count on 64-bit arches but the saves wouldn't
 * be portable to arches without 64-bit type.
 */
struct object_dep {
	struct object	*obj;		/* Object */
	Uint32		 count;		/* Reference count */
#define OBJECT_DEP_MAX	(0xffffffff-2)	/* Remain resident if reached */
	TAILQ_ENTRY(object_dep) deps;
};

#define OBJECT_TYPE_MAX	32
#define OBJECT_NAME_MAX	64
#define OBJECT_PATH_MAX	1024
#define OBJECT_MAX_USED	(0xffffffff-2)

TAILQ_HEAD(objectq, object);

struct object {
	char	 type[OBJECT_TYPE_MAX];		/* Type of object */
	char	 name[OBJECT_NAME_MAX];		/* Identifier */
	char	*save_pfx;			/* Save dir prefix */

	const struct object_ops	*ops;		/* Generic operations */

	int	 flags;
#define OBJECT_RELOAD_PROPS	0x01	/* Don't free props before load */
#define OBJECT_NON_PERSISTENT	0x02	/* Never include in saves */
#define OBJECT_INDESTRUCTIBLE	0x04	/* Not destructible (advisory) */
#define OBJECT_DATA_RESIDENT	0x08	/* Object data is resident */
#define OBJECT_PRESERVE_DEPS	0x10	/* Don't remove a dependency when its
					   reference count reaches 0. */
#define OBJECT_STATIC		0x20	/* Don't free() after detach. */
#define OBJECT_READONLY		0x40	/* Don't allow edition (advisory) */
#define OBJECT_SAVED_FLAGS	(OBJECT_RELOAD_PROPS|OBJECT_INDESTRUCTIBLE)

	pthread_mutex_t	 lock;
	struct gfx	*gfx;			/* Associated graphics */
	char		*gfx_name;		/* Gfx to fetch */
	Uint32		 gfx_used;		/* Gfx ref count */
	struct audio	*audio;			/* Associated audio samples */
	char		*audio_name;		/* Audio to fetch */
	Uint32		 audio_used;		/* Audio ref count */
	Uint32		 data_used;		/* Derivate data ref count */

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
void		 object_reinit(void *, int);
int		 object_load(void *);
int		 object_load_data(void *);
void		 object_wire_gfx(void *, const char *);
int		 object_save(void *);
int		 object_destroy(void *);
#ifdef EDITION
struct window	*object_edit(void *);
#endif

int	 object_page_in(void *, enum object_page_item);
int	 object_page_out(void *, enum object_page_item);

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
Uint32			  object_dep_index(const void *, const void *);
void			  object_del_dep(void *, const void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_OBJECT_H */
