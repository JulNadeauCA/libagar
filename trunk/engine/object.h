/*	$Csoft: object.h,v 1.104 2004/03/05 15:22:17 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_OBJECT_H_
#define _AGAR_OBJECT_H_

#define OBJECT_TYPE_MAX	32
#define OBJECT_NAME_MAX	128
#define OBJECT_PATH_MAX	1024

#include <engine/prop.h>
#include <engine/gfx.h>
#include <engine/audio.h>
#include <engine/position.h>

#include "begin_code.h"

struct map;
struct input;
struct event;

/* Generic object operation vector */
struct object_ops {
	void	(*init)(void *, const char *);		/* Initialize */
	void	(*reinit)(void *);			/* Reinitialize */
	void	(*destroy)(void *);			/* Free resources */
	int	(*load)(void *, struct netbuf *);	/* Load from network */
	int	(*save)(void *, struct netbuf *);	/* Save to network */
	struct window *(*edit)(void *);			/* Edit object */
};

/* Dependency with respect to another object. */
struct object_dep {
	char		*path;		/* Unresolved object path */
	struct object	*obj;		/* Resolved object */
	Uint32		 count;		/* Reference count */
#define OBJECT_DEP_MAX	(0xffffffff-2)	/* If reached, stay resident */
	TAILQ_ENTRY(object_dep) deps;
};

TAILQ_HEAD(objectq, object);

struct object {
	char	 type[OBJECT_TYPE_MAX];		/* Type of object */
	char	 name[OBJECT_NAME_MAX];		/* Identifier */
	char	*save_pfx;			/* Save dir prefix */
	const struct object_ops *ops;		/* Generic operation vector */
	int	 flags;
#define OBJECT_RELOAD_PROPS	0x01	/* Don't free props before load */
#define OBJECT_NON_PERSISTENT	0x02	/* Never include in saves */
#define OBJECT_INDESTRUCTIBLE	0x04	/* Not destructible (advisory) */
#define OBJECT_DATA_RESIDENT	0x08	/* Object data is resident */
#define OBJECT_PRESERVE_DEPS	0x10	/* Preserve cnt=0 dependencies */
#define OBJECT_STATIC		0x20	/* Don't free() after detach */
#define OBJECT_READONLY		0x40	/* Disallow edition (advisory) */
#define OBJECT_WAS_RESIDENT	0x80	/* Used internally by object_load() */
#define OBJECT_SAVED_FLAGS	(OBJECT_RELOAD_PROPS|OBJECT_INDESTRUCTIBLE|\
				 OBJECT_PRESERVE_DEPS|OBJECT_READONLY)
#define OBJECT_DUPED_FLAGS	(OBJECT_SAVED_FLAGS|OBJECT_NON_PERSISTENT)

	pthread_mutex_t	 lock;
	struct gfx	*gfx;		/* Associated graphics set */
	char		*gfx_name;	/* Graphics set to fetch */
	Uint32		 gfx_used;	/* Referenced sprites/animations */
	struct audio	*audio;		/* Associated audio samples */
	char		*audio_name;	/* Audio sample set to fetch */
	Uint32		 audio_used;	/* Referenced samples */
	Uint32		 data_used;	/* Referenced object derivate data */

	struct position		*pos;		/* Unique position (or NULL) */
	TAILQ_HEAD(,event)	 events;	/* Event handlers */
	TAILQ_HEAD(,prop)	 props;		/* Generic properties */

	/* Uses linkage_lock */
	TAILQ_HEAD(,object_dep)	 deps;		/* Object dependencies */
	struct objectq		 children;	/* Child objects */
	void			*parent;	/* Parent object */
	TAILQ_ENTRY(object)	 cobjs;
};

enum object_page_item {
	OBJECT_GFX,		/* Shared graphics */
	OBJECT_AUDIO,		/* Shared audio samples */
	OBJECT_DATA		/* Object data */
};

#define OBJECT(ob)	((struct object *)(ob))
#define OBJECT_ICON(ob)	(((ob)->gfx != NULL && (ob)->gfx->nsprites > 0) ? \
			 SPRITE(ob, 0) : NULL)

#define OBJECT_TYPE(ob, t)	(strcmp(OBJECT(ob)->type, (t)) == 0)

#define OBJECT_FOREACH_CHILD(var, ob, type)				\
	for((var) = (struct type *)TAILQ_FIRST(&OBJECT(ob)->children);	\
	    (var) != (struct type *)TAILQ_END(&OBJECT(ob)->children);	\
	    (var) = (struct type *)TAILQ_NEXT(OBJECT(var), cobjs))

#define OBJECT_FOREACH_CHILD_REVERSE(var, ob, type)			\
	for((var) = (struct type *)TAILQ_LAST(&OBJECT(ob)->children, objectq);\
	    (var) != (struct type *)TAILQ_END(&OBJECT(ob)->children);	\
	    (var) = (struct type *)TAILQ_PREV(OBJECT(var), objectq, cobjs))

__BEGIN_DECLS
struct object	*object_new(void *, const char *);
void		 object_init(void *, const char *, const char *, const void *);
void		 object_free_data(void *);

int	 object_copy_name(const void *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 object_copy_dirname(const void *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);
int	 object_copy_filename(const void *, char *, size_t)
	     BOUNDED_ATTRIBUTE(__string__, 2, 3);

void		*object_find(const char *);
__inline__ void	*object_root(const void *);
__inline__ void *object_find_parent(void *, const char *, const char *);
__inline__ int	 object_depended(const void *);
int		 object_in_use(const void *);
void		 object_set_type(void *, const char *);
void		 object_set_name(void *, const char *);
void		 object_set_ops(void *, const void *);
void		 object_wire_gfx(void *, const char *);

void	 object_move_up(void *);
void	 object_move_down(void *);
void	*object_duplicate(void *);
void	 object_destroy(void *);

void	 object_free_children(struct object *);
void	 object_free_props(struct object *);
void 	 object_free_events(struct object *);
void	 object_free_deps(struct object *);
void	 object_free_zerodeps(struct object *);

int	 object_page_in(void *, enum object_page_item);
int	 object_page_out(void *, enum object_page_item);
int	 object_save(void *);
int	 object_load(void *);
int	 object_load_generic(void *);
int	 object_reload_data(void *);
int	 object_resolve_deps(void *);
int	 object_resolve_position(void *);
int	 object_load_data(void *);

void	 object_attach(void *, void *);
void	 object_detach(void *);
void	 object_move(void *, void *);

struct object_dep	 *object_add_dep(void *, void *);
__inline__ struct object *object_find_dep(const void *, Uint32);
Uint32			  object_dep_index(const void *, const void *);
void			  object_del_dep(void *, const void *);

#ifdef EDITION
struct window	*object_edit(void *);
#endif
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_OBJECT_H */
